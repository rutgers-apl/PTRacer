#include "llvm/Transforms/TaskDebug/TaskDebugBranchCheckPass.h"

#include<vector>
#include<stack>

TaskDebugBranchCheck::TaskDebugBranchCheck():ModulePass(ID) {
}

bool TaskDebugBranchCheck::hasAnnotation(Instruction* i, Value *V, StringRef Ann, uint8_t level) {
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

bool TaskDebugBranchCheck::instru_fn(StringRef fn_name) {
  std::ifstream infile("functions.txt");
  std::string line;
  while (std::getline(infile, line)) {
    if(fn_name.find(line) != StringRef::npos)
      return true;
  }
  return false;
}

bool TaskDebugBranchCheck::successor_visited(BasicBlock* blk, std::map<BasicBlock*, int>& bbVisitedMap) {
  if(bbVisitedMap.count(blk) == 0) {
    bbVisitedMap.insert(std::pair<BasicBlock*, int>(blk, 1));
    return false;
  }
  return true;
}

void TaskDebugBranchCheck::instrument_access(Instruction* inst, 
					     Value* addr,
					     Constant* rdWr, 
					     std::vector<Value*>& locks_acq, 
					     std::vector<Value*>& locks_rel) {
  SmallVector<Value*, 8> args;

  //call instruction get_cur_tid()
  Instruction* get_tid = CallInst::Create(getThreadId, args, "", inst);
  args.push_back(get_tid);

  //Address of shared variable
  BitCastInst* bitcast = new BitCastInst(addr, 
					 PointerType::getUnqual(Type::getInt8Ty(inst->getContext())),
  					 "", inst);

  args.push_back(bitcast);

  //array of locks acquired

  if (locks_acq.size() == 0) {
    ConstantPointerNull* null_ptr = ConstantPointerNull::get(PointerType::getUnqual(Type::getInt64Ty(inst->getContext())));
    args.push_back(null_ptr);
  } else {
    AllocaInst* lock_aq_alloca = new AllocaInst(ArrayType::get(Type::getInt64Ty(inst->getContext()), locks_acq.size()), 
						"", 
						inst);
    //errs() << "Alloca = " << *lock_aq_alloca << "\n";

    int j = 0;
    Constant* base_idx =
      ConstantInt::get(Type::getInt32Ty(inst->getContext()), 0);
    for (std::vector<Value*>::iterator it = locks_acq.begin() ; it != locks_acq.end(); ++it, ++j) {
      SmallVector<Value*, 8> idx;
      idx.push_back(base_idx);
      idx.push_back(ConstantInt::get(Type::getInt32Ty(inst->getContext()), j));
      GetElementPtrInst * lock_elem = GetElementPtrInst::CreateInBounds(ArrayType::get(Type::getInt64Ty(inst->getContext()), locks_acq.size()), 
									lock_aq_alloca, 
									idx, 
									"", 
									inst);
      StoreInst* str_inst = new StoreInst(*it, lock_elem, inst);
      //errs() << "STORE INST = " << *str_inst << "\n";
    }
    args.push_back(lock_aq_alloca);
  }

  //size of locks acquired array
  Constant* aq_size =
    ConstantInt::get(Type::getInt64Ty(inst->getContext()), locks_acq.size());
  args.push_back(aq_size);

  //array of locks released  
  if (locks_rel.size() == 0) {
    ConstantPointerNull* null_ptr = ConstantPointerNull::get(PointerType::getUnqual(Type::getInt64Ty(inst->getContext())));
    args.push_back(null_ptr);
  } else {
    AllocaInst* lock_rel_alloca = new AllocaInst(ArrayType::get(Type::getInt64Ty(inst->getContext()), locks_rel.size()), 
						 "", 
						 inst);
    //errs() << "Alloca = " << *lock_rel_alloca << "\n";

    int j = 0;
    Constant* base_idx =
      ConstantInt::get(Type::getInt32Ty(inst->getContext()), 0);
    for (std::vector<Value*>::iterator it = locks_rel.begin() ; it != locks_rel.end(); ++it, ++j) {
      SmallVector<Value*, 8> idx;
      idx.push_back(base_idx);
      idx.push_back(ConstantInt::get(Type::getInt32Ty(inst->getContext()), j));
      GetElementPtrInst * lock_elem = GetElementPtrInst::CreateInBounds(ArrayType::get(Type::getInt64Ty(inst->getContext()), locks_rel.size()), 
									lock_rel_alloca, 
									idx, 
									"", 
									inst);
      StoreInst* str_inst = new StoreInst(*it, lock_elem, inst);
      //errs() << "STORE INST = " << *str_inst << "\n";
    }
    args.push_back(lock_rel_alloca);
  }

  //size of locks released array
  Constant* rel_size =
    ConstantInt::get(Type::getInt64Ty(inst->getContext()), locks_rel.size());
  args.push_back(rel_size);

  //Read/Write argument
  args.push_back(rdWr);
  
  CallInst* recordAccCall = CallInst::Create(recordAccess, args, "", inst);
  //errs() << *recordAccCall << "\n";
}

void TaskDebugBranchCheck::addFunctionSummaries(BasicBlock* from_bb, BasicBlock* to_bb, Instruction* first_inst) {
  //errs() << "++++++++++++++++++++DETECTED BRANCHES++++++++++++++++++++++\n";
  //errs() << "BB 1 First inst: " << *(from_bb->getFirstNonPHI()) << "\n";
  //TerminatorInst *TInst = from_bb->getTerminator();
  //errs() << "BB 1 Last inst: " << *TInst << "\n\n";
  //errs() << "BB 2 First inst: " << *(to_bb->getFirstNonPHI()) << "\n";
  //TInst = to_bb->getTerminator();
  //errs() << "BB 2 Last inst: " << *TInst << "\n\n";
  //errs() << "++++++++++++++++++++DETECTED BRANCHES++++++++++++++++++++++\n";
  
  std::vector<Value*> locks_acq;
  std::vector<Value*> locks_rel;

  bool startInst = false;

  for (BasicBlock::iterator i = from_bb->begin(); i != from_bb->end(); ++i) {

    if (startInst == false) {
      if (first_inst == dyn_cast<Instruction>(i)) {
	startInst = true;
      } else {
	continue;
      }
    }

    switch (i->getOpcode()) {
    case Instruction::Call:
      {
	CallInst* callInst = dyn_cast<CallInst>(i);
	if(callInst->getCalledFunction() != NULL) {
	  if(callInst->getCalledFunction() == lockAcquire) {
	    locks_acq.push_back(callInst->getArgOperand(1));
	  } else if (callInst->getCalledFunction() == lockRelease) {
	    locks_rel.push_back(callInst->getArgOperand(1));
	  }
	}
	break;
      }
    case Instruction::Load:
      {
	//errs() << "LOAD INST " << *i << "\n";
	Value* op_l = i->getOperand(0);
	if (hasAnnotation(i, op_l, "check_av", 1)) {
	  Constant* read =
	    ConstantInt::get(Type::getInt32Ty(to_bb->getContext()), 0);
	  instrument_access(to_bb->getFirstNonPHI(), op_l, read, locks_acq, locks_rel);
	}
	break;
      }
    case Instruction::Store:
      {
	//errs() << "STR INST " << *i << "\n";
	Value* op_s = i->getOperand(1);
	if (hasAnnotation(i, op_s, "check_av", 1)) {
	  Constant* write =
	    ConstantInt::get(Type::getInt32Ty(to_bb->getContext()), 1);
	  instrument_access(to_bb->getFirstNonPHI(), op_s, write, locks_acq, locks_rel);
	}
	break;
      }
    }
  }
  
}

void TaskDebugBranchCheck::checkForBranch(TerminatorInst *TInst) {
  unsigned NSucc = TInst->getNumSuccessors();

  
  if(NSucc == 0 || NSucc == 1) //Not a branch for sure
    return;

  BasicBlock* Succ_1 = TInst->getSuccessor(0);
  BasicBlock* Succ_2 = TInst->getSuccessor(1);

  assert(Succ_1 != NULL && Succ_2 != NULL);
  TerminatorInst *TInst_Succ_1 = Succ_1->getTerminator();
  TerminatorInst *TInst_Succ_2 = Succ_2->getTerminator();

  Instruction* succ_1_first = Succ_1->getFirstNonPHI();
  Instruction* succ_2_first = Succ_2->getFirstNonPHI();

  if(TInst_Succ_1->getNumSuccessors() == 0 && TInst_Succ_2->getNumSuccessors() == 0) {
    //errs() << "FOUND DIRECT RETURN BRANCH\n";
    addFunctionSummaries(Succ_1, Succ_2, succ_1_first);
    addFunctionSummaries(Succ_2, Succ_1, succ_2_first);
  } else if (TInst_Succ_1->getNumSuccessors() == 1 && 
	     TInst_Succ_2->getNumSuccessors() == 1 && 
	     TInst_Succ_1->getSuccessor(0) == TInst_Succ_2->getSuccessor(0)) {
    //Found if-then-else branch
    //errs() << "FOUND IF_THEN_ELSE BRANCH\n";
    addFunctionSummaries(Succ_1, Succ_2, succ_1_first);
    //errs() << "ELSE IN IF\n";
    addFunctionSummaries(Succ_2, Succ_1, succ_2_first);
  } else if(TInst_Succ_1->getNumSuccessors() == 1 && 
	     TInst_Succ_1->getSuccessor(0) == Succ_2) {
    addFunctionSummaries(Succ_1, Succ_2, succ_1_first);
  } else if(TInst_Succ_2->getNumSuccessors() == 1 && 
	     TInst_Succ_2->getSuccessor(0) == Succ_1) {
    //Found if-then branch
    //errs() << "FOUND IF_THEN BRANCH\n";
    addFunctionSummaries(Succ_2, Succ_1, succ_2_first);
  }
  //errs() << "EXIT CHECK FOR BRANCH\n";
}

void TaskDebugBranchCheck::parseBB(BasicBlock& blk, std::map<BasicBlock*, int>& bbVisitedMap) {
  //errs() << "***********NEW BASIC BLOCK*********\n";
  //errs() << "First inst: " << *(blk.getFirstNonPHI()) << "\n";
  TerminatorInst *TInst = blk.getTerminator();
  if (!TInst) return;
  checkForBranch(TInst);

  for (unsigned I = 0, NSucc = TInst->getNumSuccessors(); I < NSucc; ++I) {
    BasicBlock *Succ = TInst->getSuccessor(I);
    if (!successor_visited(Succ, bbVisitedMap))
      parseBB(*Succ, bbVisitedMap);
  }
}

void TaskDebugBranchCheck::intializeBranchCheckVariables(Module& module) {
  getThreadId = module.getFunction("get_cur_tid");
  assert(getThreadId &&
	 "get_cur_tid function type null?");

  recordAccess = module.getFunction("RecordAccess");
  assert(recordAccess &&
	 "RecordAccess function type null?");

  lockAcquire = module.getFunction("CaptureLockAcquire");
  assert(lockAcquire &&
	 "CaptureLockAcquire function type null?");

  lockRelease = module.getFunction("CaptureLockRelease");
  assert(lockRelease &&
	 "CaptureLockRelease function type null?");
}

void TaskDebugBranchCheck::intializeBranchCheckFunctions(Module& module) {
  Type* VoidTy = Type::getVoidTy(module.getContext());
  Type* void_ptr_ty = PointerType::getUnqual(Type::getInt8Ty(module.getContext()));
  Type* uint_ptr_ty = PointerType::getUnqual(Type::getInt64Ty(module.getContext()));
  Type* size_ty = Type::getInt64Ty(module.getContext());
  Type* int_ty = Type::getInt32Ty(module.getContext());

  module.getOrInsertFunction("get_cur_tid", size_ty, NULL);
  module.getOrInsertFunction("RecordAccess", VoidTy, size_ty, void_ptr_ty, uint_ptr_ty, size_ty, uint_ptr_ty, size_ty, int_ty, NULL);
  module.getOrInsertFunction("CaptureLockAcquire", VoidTy, size_ty, size_ty, NULL);
  module.getOrInsertFunction("CaptureLockRelease", VoidTy, size_ty, size_ty, NULL);
}

bool TaskDebugBranchCheck::runOnModule(Module &module) {
#if 1
  //errs() << "TaskDebugBranchCheckPass:\n";

  intializeBranchCheckFunctions(module);
  intializeBranchCheckVariables(module);

  iplist<Function>& functionList = module.getFunctionList();
  for (ilist_iterator<Function> i = functionList.begin(); i != functionList.end(); ++i) {
    if (!instru_fn(i->getName())) continue;
    //errs() << i->getName() << "\n";
    std::map<BasicBlock*, int> bbVisitedMap;
    bbVisitedMap.insert(std::pair<BasicBlock*, int>(&i->getEntryBlock(), 1));
    parseBB(i->getEntryBlock(), bbVisitedMap);
  }
#endif
  //errs() << "EXIT PASS\n";
  return false;
}

char TaskDebugBranchCheck::ID = 0;
static RegisterPass<TaskDebugBranchCheck> X("task_debug_branch_check", "TaskDebugBranchCheck Pass");
