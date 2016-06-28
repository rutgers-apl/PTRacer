#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include <fstream>

//#include "llvm/Transforms/TaskDebug/AnnotationInfo.h"

#include<map>

using namespace llvm;

class TaskDebugBranchCheck : public ModulePass {
 private:
  Function* getThreadId;
  Function* recordAccess;
  Function* lockAcquire;
  Function* lockRelease;

  void instrument_access(Instruction* inst, 
			 Value* addr,
			 Constant* rdWr, 
			 std::vector<Value*>& locks_acq, 
			 std::vector<Value*>& locks_rel);
  void intializeBranchCheckFunctions(Module& module);
  void intializeBranchCheckVariables(Module& module);
  void addFunctionSummaries(BasicBlock* bb1, BasicBlock* bb2, Instruction* first_inst);
  void checkForBranch(TerminatorInst *TInst);
  bool successor_visited(BasicBlock* blk, std::map<BasicBlock*, int>& bbVisitedMap);
  void parseBB(BasicBlock& blk, std::map<BasicBlock*, int>&);
  bool instru_fn(StringRef fn_name);
  bool hasAnnotation(Instruction* i,Value *V, StringRef Ann, uint8_t level = 0);

 public:
  static char ID; // Pass identification, replacement for typeid
  TaskDebugBranchCheck();
  bool runOnModule(Module&);
};
