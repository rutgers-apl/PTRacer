#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/AliasAnalysis.h"

//#include "llvm/Transforms/TaskDebug/AnnotationInfo.h"

#include <fstream>

using namespace llvm;

//#define DEBUG_TYPE "task_debug"

class TaskDebug : public ModulePass {
 private:
  Function* recordMem;
  Function* getThreadId;
  Function* activate;
  Function* Fini;
  void intializeTaskDebugVariables(Module&);
  void intializeTaskDebugFunctions(Module&);
  void instrument_access(Value* op, Instruction* inst, Constant* rdWr, Module& module);
  bool instru_fn(StringRef fn_name);
  bool hasAnnotation(Instruction* i,Value *V, StringRef Ann, uint8_t level = 0);

 public:
  size_t num_accesses;
  static char ID; // Pass identification, replacement for typeid
  TaskDebug();

  void parseBB(BasicBlock& blk, Module&);
  bool runOnModule(Module&);
  void getAnalysisUsage(AnalysisUsage &Info);
};
