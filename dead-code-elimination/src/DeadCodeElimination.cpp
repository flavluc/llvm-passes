#include <utility>
#include <vector>

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Utils/Local.h"

#include "RangeAnalysis.hpp"

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

  void removeDeadCode(Function &Fn, InterProceduralRA<Cousot> &Ra) {

    for (BasicBlock &BB : Fn)
      for (Instruction &I : BB)
        if (ICmpInst* CI = dyn_cast<ICmpInst>(&I)){
          
          auto [replace, _const] = shouldReplaceByConst(CI, Ra);
          if (replace){
            replaceInstBy(CI, _const);
          }
        }
  }

  struct LegacyDeadCodeElimination : public FunctionPass {
    static char ID;
    LegacyDeadCodeElimination() : FunctionPass(ID) {}

    bool runOnFunction(Function &Fn) override {

      InterProceduralRA<Cousot> &Ra = getAnalysis<InterProceduralRA<Cousot>>();

      removeDeadCode(Fn, Ra);
      return true;
    }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
      AU.addRequired<InterProceduralRA<Cousot>>();
    }
  };

}

char LegacyDeadCodeElimination::ID = 0;

static RegisterPass<LegacyDeadCodeElimination>
    X("my-dce", "Dead Code Elimination Pass", true, false);
