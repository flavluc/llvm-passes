#include <iostream>
#include <fstream>
#include <algorithm>

#include "DotGraph.cpp"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

namespace {

std::string genOpStr(Value *operand) {
        
  if(isa<Function>(operand))
      return "@" + operand->getName().str();
  
  if (isa<BasicBlock>(operand))
      return operand->getName().str();

  if(ConstantInt *CI = dyn_cast<ConstantInt>(operand))
      return std::to_string(CI->getZExtValue());

  if (ConstantExpr *CE = dyn_cast<ConstantExpr>(operand)){
    std::string rep = " (";
    for (int j=0; j < CE->getNumOperands(); j++)
        rep += genOpStr(CE->getOperand(j));
    return rep + ")";
  }
  
  if (operand->hasName())
    return "%" + operand->getName().str();

  return "";
}

std::string getInstStr(Instruction &I) {

  string result;

  if (!I.getType()->isVoidTy()){
    result += "%" + I.getName().str() + " = ";
  }

  result += I.getOpcodeName();

  if (PHINode *PN = dyn_cast<PHINode>(&I)) {

    for (int i = 0; i < PN->getNumIncomingValues(); i++) 
     result += std::string(i ? ", " : "") + " [ " + genOpStr(PN->getIncomingValue(i)) + ", " + genOpStr(PN->getIncomingBlock(i)) + " ]";

    return result + "\\l";
  }

  for(Use &operand : I.operands())
    result += " " + genOpStr(operand.get());

  return result + "\\l";
}

std::string getBBStr(BasicBlock &BB) {
  std::string bbStr = BB.getName().str() + ":\\l\\l";

  for (Instruction &I : BB) 
    bbStr += getInstStr(I);

  return bbStr;
}

void genInstNames(Function &Fn){
  int pc = 0;
  for (BasicBlock &BB : Fn){
    BB.setName("BB"+to_string(pc++));

    for (Instruction &I : BB)
      if (!I.hasName() && !I.getType()->isVoidTy())
        I.setName(to_string(pc++));
  }
}

std::string getFnString(Function &Fn) {

  genInstNames(Fn);

  std::string title = "CFG for '"+Fn.getName().str()+"' function";
  dot::Graph graph(title);

  for (BasicBlock &BB : Fn)
    graph.node(BB.getName(), getBBStr(BB));
  

  for (BasicBlock &BB : Fn)
    for (BasicBlock *BBSucc : successors(&BB))
      graph.edge(BB.getName(), BBSucc->getName());

  return graph.genDot();
}

void generateCFGFor(Function &Fn) {
  std::string filename = (Fn.getName() + ".dot").str();
  errs() << "writing '" << filename << "'...";

  std::ofstream file(filename);
  
  if (file.is_open())
    file << getFnString(Fn);
  else
    errs() << "  error opening file for writing!";
  errs() << "\n";
 }

// New PM implementation
struct ByteCodeToDot : PassInfoMixin<ByteCodeToDot> {
  // Main entry point, takes IR unit to run the pass on (&F) and the
  // corresponding pass manager (to be queried if need be)
  PreservedAnalyses run(Function &Fn, FunctionAnalysisManager &) {
    generateCFGFor(Fn);
    return PreservedAnalyses::all();
  }
};

// Legacy PM implementation
struct LegacyByteCodeToDot : public FunctionPass {
  static char ID;
  LegacyByteCodeToDot() : FunctionPass(ID) {}
  // Main entry point - the name conveys what unit of IR this is to be run on.
  bool runOnFunction(Function &Fn) override {
    generateCFGFor(Fn);
    // Doesn't modify the input unit of IR, hence 'false'
    return false;
  }
};
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getByteCodeToDotPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "ByteCodeToDot", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "bc-to-dot") {
                    FPM.addPass(ByteCodeToDot());
                    return true;
                  }
                  return false;
                });
          }};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize ByteCodeToDot when added to the pass pipeline on the
// command line, i.e. via '-passes=bc-to-dot'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getByteCodeToDotPluginInfo();
}

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------
// The address of this variable is used to uniquely identify the pass. The
// actual value doesn't matter.
char LegacyByteCodeToDot::ID = 0;

// This is the core interface for pass plugins. It guarantees that 'opt' will
// recognize LegacyByteCodeToDot when added to the pass pipeline on the command
// line, i.e.  via '--legacy-bc-to-dot'
static RegisterPass<LegacyByteCodeToDot>
    X("legacy-bc-to-dot", "Byte Code To Dot Pass",
      true, // This pass doesn't modify the CFG => true
      false // This pass is not a pure analysis pass => false
    );
