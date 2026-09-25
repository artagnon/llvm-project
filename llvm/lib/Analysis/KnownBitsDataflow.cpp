//===- KnownBitsDataflow.cpp - Cache and invalidate KnownBits -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/KnownBitsDataflow.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/KnownBits.h"

using namespace llvm;

#define DEBUG_TYPE "known-bits-dataflow"

STATISTIC(KnownBitsCacheHits, "Number of hits in the KnownBits cache");

namespace llvm {
template <typename NodeRef, typename ChildIteratorType>
struct NodeGraphTraitsBase {
  static NodeRef getEntryNode(NodeRef N) { return N; }
  static ChildIteratorType child_begin(NodeRef N) { // NOLINT
    return N->user_begin();
  }
  static ChildIteratorType child_end(NodeRef N) { // NOLINT
    return N->user_end();
  }
};

template <>
struct GraphTraits<Value *>
    : public NodeGraphTraitsBase<Value *, Value::user_iterator> {
  using NodeRef = Value *;
  using ChildIteratorType = Value::user_iterator;
};
} // namespace llvm

// Pin the vtable.
void KnownBitsVH::anchor() {}

void KnownBitsVH::deleted() {
  // This is called in the destructor of ValueHandleBase. Carefully avoid
  // constructing a new ValueHandle, and avoid calling virtual functions.
  [[maybe_unused]] bool Removed = KBD->remove_if(
      [&](const auto &It) { return It.first.getValPtr() == getValPtr(); });
  assert(Removed && "Expected to find ValPtr in map");
  clearValPtr();
}

void KnownBitsVH::allUsesReplacedWith(Value *New) {
  // This is called in ValueHandleBase before any uses are replaced.
  KBD->invalidate(*this);
  setValPtr(New);
}

KnownBitsWithCtxI::KnownBitsWithCtxI(const KnownBits &Known,
                                     const Instruction *CtxI)
    : KnownBits(Known), CtxI(const_cast<Instruction *>(CtxI)) {}

bool KnownBitsWithCtxI::canUseWith(const Instruction *Other) const {
  return Other == CtxI;
}

/// A wrapper around make_filter_range, that filters \p R on scalar types that
/// are either integer or pointer type, as these are the only types handled by
/// computeKnownBits.
template <typename RangeT>
static auto make_knownbits_range(RangeT &&R) { // NOLINT
  return make_filter_range(R, [](const auto &V) {
    return V->getType()->getScalarType()->isIntOrPtrTy();
  });
}

auto KnownBitsDataflow::forwardDataflow(const KnownBitsVH &V) const {
  return make_filter_range(depth_first(V.getValPtr()),
                           bind_front(&KnownBitsDataflow::contains, this));
}

void KnownBitsDataflow::invalidate(const KnownBitsVH &V) {
  for (const Value *N : forwardDataflow(V))
    value_as(N).resetAll();
}

unsigned KnownBitsDataflow::getBitWidth(Type *Ty, const DataLayout &DL) {
  if (unsigned BitWidth = Ty->getScalarSizeInBits())
    return BitWidth;
  return DL.getPointerTypeSizeInBits(Ty);
}

SmallVector<KnownBitsVH>
KnownBitsDataflow::computeRoots(const Function &F) const {
  SmallVector<KnownBitsVH> Roots;

  // First, collect function arguments.
  for (const Value *V : make_knownbits_range(make_pointer_range(F.args())))
    if (contains(V))
      Roots.emplace_back(key_as(V));

  // A helper to find out whether a Value is reachable from Roots that computes
  // the reachability information just in time, as Roots are updated.
  auto IsReachableFromRoots = [&](const Value *V) {
    for (const KnownBitsVH &R : Roots)
      for (const Value *N : make_knownbits_range(depth_first(R.getValPtr())))
        if (N == V)
          return true;
    return false;
  };

  // Now collect all Instructions that aren't reachable from the function's
  // arguments, updating Roots, as we test for unreachability.
  for (const BasicBlock &BB : F)
    for (const Value *V : make_knownbits_range(make_pointer_range(BB)))
      if (!IsReachableFromRoots(V) && contains(V))
        Roots.emplace_back(key_as(V));

  return Roots;
}

/// For testing, we emplace all-conflict as a sentinel value.
static KnownBits getAllConflict(unsigned BitWidth) {
  KnownBits Known(BitWidth);
  Known.setAllConflict();
  return Known;
}

void KnownBitsDataflow::initializeEntireGraph(const Function &F) {
  for (const Value *V : make_knownbits_range(make_pointer_range(F.args())))
    emplace_as(V, getAllConflict(getBitWidth(V->getType(), getDataLayout())));

  // Now collect all Instructions that aren't reachable from the function's
  // arguments, updating Roots, as we test for unreachability.
  for (const BasicBlock &BB : F) {
    for (const Value *V : make_knownbits_range(make_pointer_range(BB))) {
      emplace_as(V, getAllConflict(getBitWidth(V->getType(), getDataLayout())));
    }
  }
}

template <typename RangeT>
SmallVector<const Value *>
KnownBitsDataflow::forwardDataflow(RangeT &&Roots) const {
  SetVector<const Value *> Collected;
  for (const KnownBitsVH &V : Roots)
    Collected.insert_range(forwardDataflow(V));
  return Collected.takeVector();
}

bool KnownBitsDataflow::isLeaf(const Value *V) const {
  return make_knownbits_range(V->users()).empty();
}

void KnownBitsDataflow::print(const Function &F, raw_ostream &OS) const {
  SmallVector<KnownBitsVH> Roots = computeRoots(F);
  for (const Value *V : forwardDataflow(Roots)) {
    if (is_contained(Roots, V))
      OS << "^ ";
    else if (isLeaf(V))
      OS << "$ ";
    else
      OS << "  ";
    V->print(OS);
    OS << " | ";
    value_as(V).print(OS);
    OS << "\n";
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void KnownBitsDataflow::dump(const Function &F) const {
  print(F, dbgs());
}
#endif

std::optional<KnownBits>
KnownBitsDataflow::lookup(const Value *V, const Instruction *CtxI) const {
  auto It = find_as(V);
  if (It == end())
    return std::nullopt;
  const KnownBitsWithCtxI &Known = It->second;
  if (Known.isUnknown() || !Known.canUseWith(CtxI))
    return std::nullopt;
  ++KnownBitsCacheHits;
  return Known;
}

void KnownBitsDataflow::emplace_as(const Value *V, const KnownBits &Known,
                                   const Instruction *CtxI) {
  if (Known.isUnknown())
    return;
  emplace_or_assign({V, this}, KnownBitsWithCtxI(Known, CtxI));
}

AnalysisKey KnownBitsDataflowAnalysis::Key;

KnownBitsDataflow KnownBitsDataflowAnalysis::run(Function &F,
                                                 FunctionAnalysisManager &) {
  return F.getDataLayout();
}

bool KnownBitsDataflow::invalidate(Function &, const PreservedAnalyses &PA,
                                   FunctionAnalysisManager::Invalidator &) {
  auto PAC = PA.getChecker<KnownBitsDataflowAnalysis>();
  return !PAC.preserved();
}

// Legacy PM wrapper pass.
char KnownBitsDataflowAnalysisWrapperPass::ID = 0;

KnownBitsDataflowAnalysisWrapperPass::KnownBitsDataflowAnalysisWrapperPass()
    : FunctionPass(ID) {}

void KnownBitsDataflowAnalysisWrapperPass::getAnalysisUsage(
    AnalysisUsage &AU) const {
  AU.setPreservesAll();
}

bool KnownBitsDataflowAnalysisWrapperPass::runOnFunction(Function &F) {
  Result.reset(new KnownBitsDataflow(F.getDataLayout()));
  return false;
}

INITIALIZE_PASS(KnownBitsDataflowAnalysisWrapperPass, "known-bits-dataflow",
                "KnownBits Dataflow", false, true)
