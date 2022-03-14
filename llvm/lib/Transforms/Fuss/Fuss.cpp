#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace {
struct Fuss : public FunctionPass {
  static char ID;
  Fuss() : FunctionPass(ID) {}

  // Possible improvement: check the prototype of the function
  // Q: What if there is more than one variable ret?
  const Instruction &findVariableRet(const Function &F) {
    for (auto &BB : *F.getReverseIterator()) {
      for (auto &I : *BB.getReverseIterator()) {
        if (I.getOpcode() == Instruction::Ret &&
            !isa<Constant>(I.getOperandUse(0))) {
          return I;
        }
      }
    }
    llvm_unreachable("Given function does not have a variable ret");
  }

  // Q: What if there is more than one variable store?
  const Instruction *findVariableStore(iterator_range<Value::use_iterator> It) {
    for (auto &U : It) {
      auto *I = dyn_cast<Instruction>(U.getUser());
      if (I->getOpcode() == Instruction::Store && !isa<Constant>(U)) {
        return I;
      }
    }
    llvm_unreachable("Could not find a variable store");
  }

  // Q: What if there is more than one use of ret's operand?
  // Q: What if the only use of ret isn't a load?
  const Instruction *findTarget(const Instruction &Ret) {
    auto *Op = Ret.getOperand(0);
    assert(Op->getNumUses() == 1 && "More than one use of ret's operand");
    auto *I = dyn_cast<Instruction>(*Op->uses().begin());
    assert(I->getOpcode() == Instruction::Load &&
           "use of ret's operand is not a load");
    auto *T = findVariableStore(I->getOperand(0)->uses());
    return T;
  }

  // Possible improvement: inject srand(time(NULL)) in the root BB
  FunctionCallee randPrototype(LLVMContext &ctx, Module *mod) {
    auto *ty = FunctionType::get(Type::getInt32Ty(ctx), false);
    auto func = mod->getOrInsertFunction("rand", ty);
    return func;
  }

  Value *getInt32Const(IRBuilder<> &Builder, int v) {
    return Constant::getIntegerValue(Builder.getInt32Ty(), APInt(32, v));
  }

  void transformTarget(const Instruction *I, const BasicBlock *RetBB) {
    auto *T = const_cast<BasicBlock *>(I->getParent());
    auto *F = T->getParent();

    // Create two new BBs, and replace T's uses with brBB
    auto *brBB = BasicBlock::Create(F->getContext(), "brBB", F, T);
    auto *deadBB =
        BasicBlock::Create(F->getContext(), "deadBB", F, T->getNextNode());
    T->replaceAllUsesWith(brBB);

    // Append to brBB
    IRBuilder<> Builder(brBB);
    auto *callV =
        Builder.CreateCall(randPrototype(F->getContext(), F->getParent()));
    auto *brV = Builder.CreateAdd(callV, getInt32Const(Builder, 1));
    auto *boolV = Builder.CreateCmp(CmpInst::Predicate::ICMP_UGT, brV,
                                    getInt32Const(Builder, 0));
    Builder.CreateCondBr(boolV, T, deadBB);

    // Append to deadBB
    IRBuilder<> DBuilder(deadBB);
    DBuilder.CreateStore(getInt32Const(DBuilder, 0), I->getOperand(1));
    DBuilder.CreateBr(const_cast<BasicBlock *>(RetBB));
  }

  bool runOnFunction(Function &F) override {
    auto &Ret = findVariableRet(F);
    auto *T = findTarget(Ret);
    transformTarget(T, Ret.getParent());
    return false;
  }
};
} // namespace

// Possible improvement: use the new PassManager
char Fuss::ID = 0;
static RegisterPass<Fuss> X("fuss", "Obfuscation pass",
                            false /* Only looks at CFG */,
                            false /* Analysis Pass */);

static RegisterStandardPasses Y(PassManagerBuilder::EP_OptimizerLast,
                                [](const PassManagerBuilder &Builder,
                                   legacy::PassManagerBase &PM) {
                                  PM.add(new Fuss());
                                });
