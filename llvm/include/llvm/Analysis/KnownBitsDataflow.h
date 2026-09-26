//===- KnownBitsDataflow.h - Cache and invalidate KnownBits ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_KNOWNBITSDATAFLOW_H
#define LLVM_ANALYSIS_KNOWNBITSDATAFLOW_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/KnownBits.h"

namespace llvm {
class Value;
class User;
class Function;
class DataLayout;
class raw_ostream;
class KnownBitsDataflow;
class TargetLibraryInfo;
class AssumptionCache;
class DominatorTree;
class DomConditionCache;
struct CondContext;
struct SimplifyQuery;

/// A custom ValueHandle with callback to erase KnownBits in the cache when a
/// Value is deleted, or invalidate dependent KnownBits when RAUW'ed.
class KnownBitsVH final : public CallbackVH {
  friend class KnownBitsDataflow;
  KnownBitsDataflow *KBD;

  virtual void deleted() override;
  virtual void allUsesReplacedWith(Value *New) override;

public:
  KnownBitsVH(const Value *V, const KnownBitsDataflow *KBD)
      : CallbackVH(V), KBD(const_cast<KnownBitsDataflow *>(KBD)) {}
  const Value *getValue() const { return getValPtr(); }
  operator const Value *() const { return getValPtr(); }
  bool operator==(const KnownBitsVH &Other) const {
    return getValue() == Other.getValue();
  }
  bool operator<(const KnownBitsVH &Other) const {
    return getValue() < Other.getValue();
  }
};

// The DenseMapInfo for our KnownBits ValueHandle is just the DenseMapInfo on
// the Value pointer: there is no other information in the ValueHandle that's
// relevant for a DenseMap.
template <> struct DenseMapInfo<KnownBitsVH> {
  static unsigned getHashValue(const KnownBitsVH &Val) {
    return DenseMapInfo<const Value *>::getHashValue(Val.getValue());
  }

  static bool isEqual(const KnownBitsVH &LHS, const KnownBitsVH &RHS) {
    return DenseMapInfo<const Value *>::isEqual(LHS.getValue(), RHS.getValue());
  }
};

/// A DenseMap holding a ValueHandle, that performs lookups based on the
/// underlying Value.
template <typename ValueT>
using DenseMapForVH =
    DenseMap<KnownBitsVH, ValueT, DenseMapInfo<const Value *>>;

/// Tracks CxtI from SimplifyQuery.
struct CtxITracking : public WeakVH {
  CtxITracking() = default;
  CtxITracking(const SimplifyQuery &SQ);
  bool isRefinementOf(const CtxITracking &Other) const;
};

/// The ValueT of our DenseMap is actually a KnownBits augmented with
/// context-instruction information.
struct AugmentedKnownBits : public KnownBits, public CtxITracking {
  AugmentedKnownBits() = default;
  AugmentedKnownBits(const KnownBits &Known, CtxITracking Info = {})
      : KnownBits(Known), CtxITracking(Info) {}
};

/// A structure keeps a mapping between a custom ValueHandle and
/// AugmentedKnownBits, with core functionality to cache KnownBits with
/// automatic invalidation on IR manipulation. Given its usecase, keeping a
/// deterministically-ordered structure would be wasteful, and we can compute a
/// deterministic ordering for testing and debugging purposes.
class LLVM_ABI KnownBitsDataflow : protected DenseMapForVH<AugmentedKnownBits> {
  friend class KnownBitsVH;

  /// The cache is valid for exactly one DL.
  const DataLayout &DL;

  /// Used to clean up after an IR Value is erased, carefully written not to use
  /// ValueHandle lookups.
  void cleanup() {
    remove_if([&](const auto &KV) { return KV.first.getValue() == nullptr; });
  }

  /// Do a forward data-flow walk, and find all ValueHandles whose KnownBits
  /// depeends on the KnownBits of \p V. Returns a range of Values.
  auto forwardDataflow(const KnownBitsVH &V) const;

  /// Range-based variant.
  template <typename RangeT>
  SmallVector<const Value *> forwardDataflow(RangeT &&Roots) const;

protected:
  using BaseT = DenseMapForVH<AugmentedKnownBits>;

  LLVM_ABI_FOR_TEST KnownBitsVH key_as(const Value *V) const { // NOLINT
    auto It = find_as(V);
    assert(It != end() && "Expected to find ValueHandle");
    return It->first;
  }
  LLVM_ABI_FOR_TEST AugmentedKnownBits &value_as(const Value *V) { // NOLINT
    auto It = find_as(V);
    assert(It != end() && "Expected to find ValueHandle");
    return It->second;
  }
  LLVM_ABI_FOR_TEST AugmentedKnownBits
  value_as(const Value *V) const { // NOLINT
    auto It = find_as(V);
    assert(It != end() && "Expected to find ValueHandle");
    return It->second;
  }

  /// Invalidates KnownBits in the entire subgraph found from the
  /// forwardDataflow walk starting from \p V. Used on IR manipulation.
  LLVM_ABI_FOR_TEST void invalidate(const KnownBitsVH &V);

  /// A leaf is a Value whose users filtered on a KnownBits range is empty. Used
  /// in print.
  LLVM_ABI_FOR_TEST bool isLeaf(const Value *V) const;

  /// Roots are the function \p F's arguments, along with Instructions that
  /// expose a new root like phis and fptosi. This is used in print, skipping
  /// any nodes not in the map.
  LLVM_ABI_FOR_TEST SmallVector<KnownBitsVH>
  computeRoots(const Function &F) const;

  /// Initializing the entire graph for Function \p F with unknown known-bits.
  /// It is expensive, and is used only for testing purposes.
  LLVM_ABI_FOR_TEST void initializeEntireGraph(const Function &F);

public:
  LLVM_ABI KnownBitsDataflow(const DataLayout &DL) : DL(DL) {}
  LLVM_ABI KnownBitsDataflow(const KnownBitsDataflow &) = delete;
  LLVM_ABI KnownBitsDataflow &operator=(const KnownBitsDataflow &) = delete;
  LLVM_ABI const DataLayout &getDataLayout() const { return DL; }

  /// A small helper extracted from ValueTracking.
  LLVM_ABI static unsigned getBitWidth(Type *Ty, const DataLayout &DL);

  LLVM_ABI bool empty() const { return BaseT::empty(); }
  LLVM_ABI size_t size() const { return BaseT::size(); }
  LLVM_ABI bool contains(const Value *V) const { return find_as(V) != end(); }

  /// Looks up \p V if it is present in the map, and returns a previously cached
  /// KnownBits that is not unknown, or std::nullopt. Pass \p Info to filter on
  /// compatibility of context-instructions.
  LLVM_ABI std::optional<KnownBits> lookup(const Value *V,
                                           CtxITracking Info = {}) const;

  /// Registers that \p V has KnownBits information \p Known, with
  /// conext-instruction \p Info, overwriting any existing value.
  LLVM_ABI void insert_or_assign(const Value *V, const KnownBits &Known,
                                 CtxITracking Info = {});

  /// This routine prints in determinstic order, at the cost of being expensive.
  LLVM_ABI void print(const Function &F, raw_ostream &OS) const;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD void dump(const Function &F) const;
#endif
};
} // end namespace llvm

#endif // LLVM_ANALYSIS_KNOWNBITSDATAFLOW_H
