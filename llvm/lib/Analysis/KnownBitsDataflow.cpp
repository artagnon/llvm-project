//===- KnownBitsDataflow.cpp - Cache and invalidate KnownBits -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/KnownBitsDataflow.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/SimplifyQuery.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/KnownBits.h"

using namespace llvm;

#define DEBUG_TYPE "known-bits-dataflow"

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
struct GraphTraits<const Value *>
    : public NodeGraphTraitsBase<const Value *, Value::const_user_iterator> {
  using NodeRef = const Value *;
  using ChildIteratorType = Value::const_user_iterator;
};
} // namespace llvm

SQCompatibility::SQCompatibility(const SimplifyQuery &SQ)
    : CtxI(const_cast<Instruction *>(SQ.CtxI)), TLI(SQ.TLI), AC(SQ.AC),
      DT(SQ.DT), DC(SQ.DC), CC(SQ.CC), UseInstrInfo(SQ.IIQ.UseInstrInfo) {}

bool SQCompatibility::isRefinementOf(const SQCompatibility &Other) const {
  return (!CtxI || CtxI == Other.CtxI) && (!TLI || TLI == Other.TLI) &&
         (!AC || AC == Other.AC) && (!CtxI || DT == Other.DT) &&
         (!DC || DC == Other.DC) && (!CC || CC == Other.CC) &&
         (!UseInstrInfo || UseInstrInfo == Other.UseInstrInfo);
}

void KnownBitsVH::deleted() {
  // Carefully erase underlying null-pointers from map after nulling out the
  // underlying Value pointer.
  setValPtr(nullptr);
  KBD->cleanup();
}

void KnownBitsVH::allUsesReplacedWith(Value *New) {
  // Invalidate dependent KnownBits information with the current ValueHandle's
  // use-list, before updating the underlying Value pointer.
  KBD->invalidate(*this);
  setValPtr(New);
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
  return make_filter_range(depth_first(V.getValue()),
                           bind_front(&KnownBitsDataflow::contains, this));
}

void KnownBitsDataflow::invalidate(const KnownBitsVH &V) {
  for (const Value *N : forwardDataflow(V))
    at_as(N).resetAll();
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
    Roots.emplace_back(getVH(V));

  // A helper to find out whether a Value is reachable from Roots that computes
  // the reachability information just in time, as Roots are updated.
  auto IsReachableFromRoots = [&](const Value *V) {
    for (const auto &R : Roots)
      for (const Value *N : make_knownbits_range(depth_first(R.getValue())))
        if (N == V)
          return true;
    return false;
  };

  // Now collect all Instructions that aren't reachable from the function's
  // arguments, updating Roots, as we test for unreachability.
  for (const BasicBlock &BB : F)
    for (const Value *V : make_knownbits_range(make_pointer_range(BB)))
      if (!IsReachableFromRoots(V))
        Roots.emplace_back(getVH(V));

  return Roots;
}

void KnownBitsDataflow::initializeEntireGraph(const Function &F) {
  for (const Value *V : make_knownbits_range(make_pointer_range(F.args())))
    insert_or_assign(V, KnownBits(getBitWidth(V->getType(), getDataLayout())));

  // Now collect all Instructions that aren't reachable from the function's
  // arguments, updating Roots, as we test for unreachability.
  for (const BasicBlock &BB : F) {
    for (const Value *V : make_knownbits_range(make_pointer_range(BB))) {
      insert_or_assign(V,
                       KnownBits(getBitWidth(V->getType(), getDataLayout())));
    }
  }
}

template <typename RangeT>
SmallVector<const Value *>
KnownBitsDataflow::forwardDataflow(RangeT &&Roots) const {
  SetVector<const Value *> Collected;
  for (const auto &V : Roots)
    Collected.insert_range(forwardDataflow(V));
  return Collected.takeVector();
}

bool KnownBitsDataflow::isLeaf(const Value *V) const {
  return make_knownbits_range(V->users()).empty();
}

void KnownBitsDataflow::print(const Function &F, raw_ostream &OS) const {
  auto Roots = computeRoots(F);
  for (const Value *V : forwardDataflow(Roots)) {
    if (is_contained(Roots, V))
      OS << "^ ";
    else if (isLeaf(V))
      OS << "$ ";
    else
      OS << "  ";
    V->print(OS);
    OS << " | ";
    at_as(V).print(OS);
    OS << "\n";
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void KnownBitsDataflow::dump(const Function &F) const {
  print(F, dbgs());
}
#endif

std::optional<KnownBits> KnownBitsDataflow::lookup(const Value *V,
                                                   SQCompatibility Info) const {
  auto It = find_as(V);
  if (It == end())
    return std::nullopt;
  // If we had a more refined Info in the cached result, re-using that result
  // will produce a better optimization result.
  const KnownBits &Known = It->second;
  if (Known.isUnknown() || !Info.isRefinementOf(It->second))
    return std::nullopt;
  return Known;
}

void KnownBitsDataflow::insert_or_assign(const Value *V, const KnownBits &Known,
                                         SQCompatibility Info) {
  AugmentedKnownBits ToInsert(Known, Info);
  auto It = find_as(V);
  if (It != end())
    It->second = ToInsert;
  insert_as(std::make_pair(KnownBitsVH(V, this), ToInsert), V);
}

std::optional<KnownBits>
SimplifyQuery::getCachedKnownBits(const Value *V) const {
  // We do not bother with checking compatibility of context-functions.
  if (!KBCache || CtxF)
    return std::nullopt;
  return KBCache->lookup(V, *this);
}

void SimplifyQuery::cacheKnownBits(const Value *V, const KnownBits &Known) {
  if (!KBCache || Known.isUnknown())
    return;
  KBCache->insert_or_assign(V, Known, *this);
}
