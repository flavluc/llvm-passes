#define DEBUG_TYPE "dead-code-elimination"

#include <utility>
#include <vector>

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/ADT/Statistic.h"

#include "RangeAnalysis.h"

STATISTIC(InstructionsEliminated, "Number of instructions eliminated");
STATISTIC(BasicBlocksEliminated, "Number of basic blocks entirely eliminated");

using namespace llvm;
using namespace std;

namespace {

  bool slt(Range &up, Range &lo) {return up.getUpper().slt(lo.getLower());}
  bool ult(Range &up, Range &lo) {return up.getUpper().ult(lo.getLower());}
  bool sle(Range &up, Range &lo) {return up.getUpper().sle(lo.getLower());}
  bool ule(Range &up, Range &lo) {return up.getUpper().ule(lo.getLower());}
  bool sgt(Range &up, Range &lo) {return up.getLower().sgt(lo.getUpper());}
  bool ugt(Range &up, Range &lo) {return up.getLower().ugt(lo.getUpper());}
  bool sge(Range &up, Range &lo) {return up.getLower().sge(lo.getUpper());}
  bool uge(Range &up, Range &lo) {return up.getLower().uge(lo.getUpper());}

  pair<int, int> shouldReplaceByConst(ICmpInst *IC, InterProceduralRA<Cousot> &Ra) {
    Range r1 = Ra.getRange(IC->getOperand(0));
    Range r2 = Ra.getRange(IC->getOperand(1));

    switch (IC->getPredicate()) {
      case CmpInst::ICMP_EQ:
        return make_pair(slt(r1, r2) || sgt(r1, r2), 0);
      case CmpInst::ICMP_NE:
        return make_pair(slt(r1, r2) || sgt(r1, r2), 1);
      case CmpInst::ICMP_UGT:
        return make_pair(ugt(r1, r2) || ult(r1, r2), ugt(r1, r2) ? 1 : 0);
      case CmpInst::ICMP_UGE:
        return make_pair(uge(r1, r2) || ult(r1, r2), uge(r1, r2) ? 1 : 0);
      case CmpInst::ICMP_ULT:
        return make_pair(ult(r1, r2) || ugt(r1, r2), ult(r1, r2) ? 1 : 0);
      case CmpInst::ICMP_ULE:
        return make_pair(ule(r1, r2) || ugt(r1, r2), ule(r1, r2) ? 1 : 0);
      case CmpInst::ICMP_SGT:
        return make_pair(sgt(r1, r2) || slt(r1, r2), sgt(r1, r2) ? 1 : 0);
      case CmpInst::ICMP_SGE:
        return make_pair(sge(r1, r2) || slt(r1, r2), sge(r1, r2) ? 1 : 0);
      case CmpInst::ICMP_SLT:
        return make_pair(slt(r1, r2) || ugt(r1, r2), slt(r1, r2) ? 1 : 0);
      case CmpInst::ICMP_SLE:
        return make_pair(sle(r1, r2) || ugt(r1, r2), sle(r1, r2) ? 1 : 0);
      default:
        return make_pair(false, 0);
    }
  }

  void replaceInstBy(ICmpInst *CI, int _const) {
    Value *C = ConstantInt::get(CI->getType(), _const);
    CI->replaceAllUsesWith(C);
  }

  void removeInstructions(vector<Instruction*> Is) {
    for (Instruction* I : Is)
      if(I != NULL)
        RecursivelyDeleteTriviallyDeadInstructions(I);
  }

  pair<int, int> amountOfBBAndI(Function &Fn) {
    unsigned int amountBB = 0, amountI = 0;

    for (BasicBlock &BB : Fn){
      amountBB += 1;
      amountI += distance(BB.begin(), BB.end());
    }

    return make_pair(amountBB, amountI);
  }


  void generateStatistics(Function &Fn, pair<int, int> oldAmount, pair<int, int> newAmount) {

    auto [oldAmountBB, oldAmountI] = oldAmount;
    auto [newAmountBB, newAmountI] = newAmount;

    BasicBlocksEliminated = oldAmountBB - newAmountBB;
    InstructionsEliminated = oldAmountI - newAmountI;
  }

  bool removeDeadCode(Function &Fn, InterProceduralRA<Cousot> &Ra) {

    vector<Instruction*> instructions;
    auto oldAmount = amountOfBBAndI(Fn);

    for (BasicBlock &BB : Fn)
      for (Instruction &I : BB)
        if (ICmpInst* CI = dyn_cast<ICmpInst>(&I)){
          auto [replace, _const] = shouldReplaceByConst(CI, Ra);
          if (replace){
            replaceInstBy(CI, _const);
            instructions.push_back(&I);
          }
        }

    removeInstructions(instructions);
    removeUnreachableBlocks(Fn);

    auto newAmount = amountOfBBAndI(Fn);
    generateStatistics(Fn, oldAmount, newAmount);

    return oldAmount != newAmount;
  }

  struct LegacyDeadCodeElimination : public FunctionPass {
    static char ID;
    LegacyDeadCodeElimination() : FunctionPass(ID) {}

    bool runOnFunction(Function &Fn) override {

      InterProceduralRA<Cousot> &Ra = getAnalysis<InterProceduralRA<Cousot>>();

      bool modified = removeDeadCode(Fn, Ra);
      return modified;
    }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
      AU.addRequired<InterProceduralRA<Cousot>>();
    }
  };

}

char LegacyDeadCodeElimination::ID = 0;

static RegisterPass<LegacyDeadCodeElimination>
    X("my-dce", "Dead Code Elimination Pass", true, false);