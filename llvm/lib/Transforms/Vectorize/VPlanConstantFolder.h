//===- VPlanConstantFolder.h - ConstantFolder for VPlan -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "VPlanValue.h"
#include "llvm/IR/ConstantFold.h"
#include "llvm/IR/Constants.h"

namespace llvm {
class VPConstantFolder {
private:
  Constant *getIRConstant(VPValue *V) const {
    return dyn_cast_or_null<Constant>(V->getUnderlyingValue());
  }

  Value *foldBinOp(Instruction::BinaryOps Opcode, VPValue *LHS,
                   VPValue *RHS) const {
    auto *LC = getIRConstant(LHS);
    auto *RC = getIRConstant(RHS);
    if (LC && RC) {
      if (ConstantExpr::isDesirableBinOp(Opcode))
        return ConstantExpr::get(Opcode, LC, RC);
      return ConstantFoldBinaryInstruction(Opcode, LC, RC);
    }
    return nullptr;
  }

public:
  Value *foldAnd(VPValue *LHS, VPValue *RHS) const {
    return foldBinOp(Instruction::BinaryOps::And, LHS, RHS);
  }

  Value *foldOr(VPValue *LHS, VPValue *RHS) const {
    return foldBinOp(Instruction::BinaryOps::Or, LHS, RHS);
  }

  Value *foldNot(VPValue *Op) const {
    auto *C = getIRConstant(Op);
    if (C)
      return ConstantExpr::get(Instruction::BinaryOps::Xor, C,
                               Constant::getAllOnesValue(C->getType()));
    return nullptr;
  }

  Value *foldLogicalAnd(VPValue *LHS, VPValue *RHS) const {
    auto *LC = getIRConstant(LHS);
    auto *RC = getIRConstant(RHS);
    if (LC && RC)
      return ConstantFoldSelectInstruction(
          LC, RC, ConstantInt::getNullValue(RC->getType()));
    return nullptr;
  }

  Value *foldSelect(VPValue *Cond, VPValue *TrueVal, VPValue *FalseVal) const {
    auto *CC = getIRConstant(Cond);
    auto *TV = getIRConstant(TrueVal);
    auto *FV = getIRConstant(FalseVal);
    if (CC && TV && FV)
      return ConstantFoldSelectInstruction(CC, TV, FV);
    return nullptr;
  }

  Value *foldCmp(CmpInst::Predicate Pred, VPValue *LHS, VPValue *RHS) const {
    auto *LC = getIRConstant(LHS);
    auto *RC = getIRConstant(RHS);
    if (LC && RC)
      return ConstantFoldCompareInstruction(Pred, LC, RC);
    return nullptr;
  }

  Value *foldPtrAdd(VPValue *Base, VPValue *Offset, GEPNoWrapFlags NW) const {
    auto *BC = getIRConstant(Base);
    auto *OC = getIRConstant(Offset);
    if (BC && OC) {
      auto &Ctx = BC->getType()->getContext();
      return ConstantExpr::getGetElementPtr(Type::getInt8Ty(Ctx), BC, OC, NW);
    }
    return nullptr;
  }

  Value *foldCast(Instruction::CastOps Opcode, VPValue *Op,
                  Type *DestTy) const {
    auto *C = getIRConstant(Op);
    if (C) {
      if (ConstantExpr::isDesirableCastOp(Opcode))
        return ConstantExpr::getCast(Opcode, C, DestTy);
      return ConstantFoldCastInstruction(Opcode, C, DestTy);
    }
    return nullptr;
  }
};
} // namespace llvm
