#include "llvm/Transforms/TaskDebug/TaskDebugPass.h"
#include "llvm/Analysis/AliasAnalysis.h"

void getAnalysisUsage(AnalysisUsage &Info) {
  Info.addRequired<AliasAnalysis>();
}

bool TaskDebug::hasAnnotation(Instruction* i, Value *V, StringRef Ann, uint8_t level) {
  // Check instruction metadata.
  //errs() << "0th If\n";
  if (Instruction *I = dyn_cast<Instruction>(V)) {

    if (I->getOpcode() == Instruction::GetElementPtr) {
      MDNode *MD = i->getMetadata("tyann");
      if (MD) {
        MDString *MDS = cast<MDString>(MD->getOperand(0));
        if (MDS->getString().equals(Ann)) {
          return true;
        } else
          return false;
      } else
        return false;
    }

    MDNode *MD = I->getMetadata("tyann");
    //errs() << *V << " 1st If\n";
    if (MD) {
      //errs() << "2nd If\n";
      MDString *MDS = cast<MDString>(MD->getOperand(0));
      if (MDS->getString().equals(Ann)) {
	//errs() << "3rd If\n";
	//return true;
	#if 1
        ConstantAsMetadata *CAM = cast<ConstantAsMetadata>(MD->getOperand(1));
        ConstantInt *CI = cast<ConstantInt>(CAM->getValue());
        if (CI->getValue() == level) {
	  //errs() << CI->getValue() << "Level true\n";
          return true;
        } else {
	  //errs() << CI->getValue() << "Level false\n";
          return false;
        }
	#endif
      }
    }
  } else if (GlobalValue *G = dyn_cast<GlobalValue>(V)) {
    MDNode *MD = i->getMetadata("tyann");
    if (MD) {
      //errs() << "2nd If\n";
      MDString *MDS = cast<MDString>(MD->getOperand(0));
      if (MDS->getString().equals(Ann)) {
	//errs() << "3rd If\n";
	return true;
	#if 0
        ConstantAsMetadata *CAM = cast<ConstantAsMetadata>(MD->getOperand(1));
        ConstantInt *CI = cast<ConstantInt>(CAM->getValue());
        if (CI->getValue() == level) {
	  //errs() << CI->getValue() << "Level true\n";
          return true;
        } else {
	  //errs() << CI->getValue() << "Level false\n";
          return false;
        }
	#endif
      }
    }    
  }

  // TODO: Check for annotations on globals, parameters.

  return false;
}

TaskDebug::TaskDebug():ModulePass(ID) {
  num_accesses = 0;
}

void TaskDebug::instrument_access(Value* op, Instruction* inst, Constant* rdWr, Module& module) {
  SmallVector<Value*, 8> args;
  Instruction* get_tid = CallInst::Create(getThreadId, args, "", inst);
  //errs() << *get_tid << "\n";
  BitCastInst* bitcast = new BitCastInst(op, 
					 PointerType::getUnqual(Type::getInt8Ty(module.getContext())),
  					 "", inst);
  //errs() << *bitcast << "\n";
  //errs() << "READ WRITE = " << *rdWr << "\n";

  args.push_back(get_tid);
  args.push_back(bitcast);
  args.push_back(rdWr);
  
  Instruction* recordMemCall = CallInst::Create(recordMem, args, "", inst);
  //errs() << *recordMemCall << "\n";
}

void TaskDebug::parseBB(BasicBlock& blk, Module& module) {
  for (BasicBlock::iterator i = blk.begin(), e = blk.end(); i != e; ++i) {

    /*Value *Ptr = nullptr;
    if (auto *LI = dyn_cast<LoadInst>(i)) {
      Ptr = LI->getPointerOperand();
    } else if (auto *SI = dyn_cast<StoreInst>(i)) {
      Ptr = SI->getPointerOperand();
    }

    // Dereferencing a pointer (either for a load or a store). Insert a
    // check if the pointer is nullable.
    if (Ptr) {
      errs() << "Check PTR\n";
      if (hasAnnotation(Ptr, "nullable", 0)) {
	errs() << "FOUNDDDDDD\n";
      }
      }*/

    switch (i->getOpcode()) {
    case Instruction::Load:
      {
	//errs() << "Load\n";
	Value* op_l = i->getOperand(0);
	if (hasAnnotation(i, op_l, "check_av", 1)) {
	  //if (const GlobalValue* G = dyn_cast<GlobalValue>(op_l)) {
	  //errs() << "Load inst with Glbal var\n";
	  Constant* read =
	    ConstantInt::get(Type::getInt32Ty(module.getContext()), 0);
	  //errs() << " - INSTRUMENT - \n";
	  instrument_access(op_l, &(*i), read, module);
	  //errs() << " - INSTRUMENTED - \n";
	  num_accesses++;
	}
	break;
      }
    case Instruction::Store:
      {
	//errs() << "Store\n";
	Value* op_s = i->getOperand(1);
	//errs() << * op_s << "\n";
	if (hasAnnotation(i, op_s, "check_av", 1)) {
	  //if (const GlobalValue* G1 = dyn_cast<GlobalValue>(op_s)) {
	  //errs() << "Store inst with Glbal var\n";
	  Constant* write =
	    ConstantInt::get(Type::getInt32Ty(module.getContext()), 1);
	  //errs() << " - INSTRUMENT - \n";
	  instrument_access(op_s, &(*i), write, module);
	  //errs() << " - INSTRUMENTED - \n";
	  num_accesses++;
	}
	break;
      }
    }
    //errs() << *i << "\n";
    //errs() << "\n";
  }
}

void TaskDebug::intializeTaskDebugVariables(Module& module) {
  getThreadId = module.getFunction("get_cur_tid");
  assert(getThreadId &&
	 "get_cur_tid function type null?");

  recordMem = module.getFunction("RecordMem");
  assert(recordMem &&
	 "RecordMem function type null?");
}

void TaskDebug::intializeTaskDebugFunctions(Module& module) {
  Type* VoidTy = Type::getVoidTy(module.getContext());
  Type* void_ptr_ty = PointerType::getUnqual(Type::getInt8Ty(module.getContext()));
  Type* size_ty = Type::getInt64Ty(module.getContext());
  Type* int_ty = Type::getInt32Ty(module.getContext());

  module.getOrInsertFunction("get_cur_tid", size_ty, NULL);
  module.getOrInsertFunction("RecordMem", VoidTy, size_ty, void_ptr_ty, int_ty, NULL);
}

bool TaskDebug::instru_fn(StringRef fn_name) {
  std::ifstream infile("functions.txt");
  std::string line;
  while (std::getline(infile, line)) {
    if(fn_name.find(line) != StringRef::npos)
      return true;
  }
  return false;
}

bool TaskDebug::runOnModule(Module &module) {
  //errs() << "TaskDebugPass:\n";

  intializeTaskDebugFunctions(module);
  intializeTaskDebugVariables(module);

#if 0
  iplist<GlobalVariable>& globalList = module.getGlobalList();
  errs() << "Global Size = " << globalList.size() << "\n";
  for (ilist_iterator<GlobalVariable> i = globalList.begin(); i != globalList.end(); ++i) {
    errs() << "***********************************************************\n";
    errs() << "var name : " << i->getName() << "\n";
  }
#endif

  //AliasAnalysis &AA = getAnalysis<AliasAnalysis>();
  
  iplist<Function>& functionList = module.getFunctionList();
  for (ilist_iterator<Function> i = functionList.begin(); i != functionList.end(); ++i) {
    if (!instru_fn(i->getName())) continue;
    //errs() << "***********************************************************\n";
    //errs() << "function name : " << i->getName() << "\n";
    //if ((i->getName()).find("_ZNK8mainWorkclERKN3tbb13blocked_rangeIiEEm") == StringRef::npos) continue;
    iplist<BasicBlock>& bbList = i->getBasicBlockList();
    for (ilist_iterator<BasicBlock> j = bbList.begin(), f = bbList.end(); j != f; ++j) {
      parseBB(*j, module);
    }
  }

  //errs() << "******* Total number of accesses instrumented = " << num_accesses << " *******\n";
  return true;
}

char TaskDebug::ID = 0;
static RegisterPass<TaskDebug> X("task_debug", "TaskDebug Pass");
