#include <sys/mman.h>
#include "DR_Detector.H"

#include <bitset>

//array of stacks of lockset_data
std::stack<struct Lockset_data*> thd_lockset[NUM_THREADS];

// 2^10 entries each will be 8 bytes each
const size_t SS_PRIMARY_TABLE_ENTRIES = ((size_t) 1024);

// each secondary entry has 2^ 22 entries (64 bytes each)
const size_t SS_SEC_TABLE_ENTRIES = ((size_t) 4*(size_t) 1024 * (size_t) 1024);
struct Dr_Address_Data** shadow_space;

std::ofstream report;

std::map<ADDRINT, struct violation*> all_violations;

extern "C" void TD_Activate() {
  //std::cout << "Address data size = " << sizeof(struct Dr_Address_Data) << " addr_data = " << sizeof(struct Address_data) << std::endl;
  taskGraph = new AFTaskGraph();
  thd_lockset[0].push(new Lockset_data());
  size_t primary_length = (SS_PRIMARY_TABLE_ENTRIES) * sizeof(struct Dr_Address_Data*);
  shadow_space = (struct Dr_Address_Data**)mmap(0, primary_length, PROT_READ| PROT_WRITE,
						MMAP_FLAGS, -1, 0);
  assert(shadow_space != (void *)-1);
}

extern "C" void CaptureLockAcquire(THREADID threadid, ADDRINT lock_addr) {
  //PIN_GetLock(&lock, 0);

  struct Lockset_data* curLockset = thd_lockset[threadid].top();
  curLockset->addLockToLockset(lock_addr);

  //PIN_ReleaseLock(&lock);
}

extern "C" void CaptureLockRelease(THREADID threadid, ADDRINT lock_addr) {
  //PIN_GetLock(&lock, 0);

  struct Lockset_data* curLockset = thd_lockset[threadid].top();
  curLockset->removeLockFromLockset();

  //PIN_ReleaseLock(&lock);
}

void CaptureExecute(THREADID threadid) {
  //PIN_GetLock(&lock, 0);

  thd_lockset[threadid].push(new Lockset_data());

  //PIN_ReleaseLock(&lock);
}

void CaptureReturn(THREADID threadid) {
  //PIN_GetLock(&lock, 0);

  struct Lockset_data* curLockset = thd_lockset[threadid].top();
  thd_lockset[threadid].pop();
  delete(curLockset);

  //PIN_ReleaseLock(&lock);
}

static bool exceptions (THREADID threadid, ADDRINT addr) {
  return (tidToTaskIdMap[threadid].empty() || 
	  tidToTaskIdMap[threadid].top() == 0 || 
	  thd_lockset[threadid].empty() ||
	  all_violations.count(addr) != 0
	  ); 
}

extern "C" void RecordAccess(THREADID threadid, void * access_addr, ADDRINT* locks_acq, 
			     size_t locks_acq_size, ADDRINT* locks_rel, 
			     size_t locks_rel_size, AccessType accessType) {
  //PIN_GetLock(&lock, 0);

  // Exceptions
  ADDRINT addr = (ADDRINT) access_addr;
  if(exceptions(threadid, addr)){ 
    //PIN_ReleaseLock(&lock);
    return;
  }

  //get lockset and current step node
  size_t curLockset = (thd_lockset[threadid].top())->createLockset();

  /////////// extra for adding lockset /////////////////////////

  for (size_t i = 0; i < locks_acq_size; i++) {
    curLockset = curLockset & Lockset_data::getLockId(locks_acq[i]);
  }

  for (size_t i = 0; i < locks_rel_size; i++) {
    curLockset = curLockset | ~(Lockset_data::getLockId(locks_rel[i]));
  }

  /////////////////////////////////////////////////////////////

  struct AFTask* curStepNode = taskGraph->getCurTask(threadid);

  /////////////////////////////////////////////////////
  // check for access pattern and update shadow space

  size_t primary_index = (addr >> 22) & 0x3ff;
  struct Dr_Address_Data* primary_entry = shadow_space[primary_index];

  if (primary_entry == NULL) {
    size_t sec_length = (SS_SEC_TABLE_ENTRIES) * sizeof(struct Dr_Address_Data);
    primary_entry = (struct Dr_Address_Data*)mmap(0, sec_length, PROT_READ| PROT_WRITE,
						     MMAP_FLAGS, -1, 0);
    shadow_space[primary_index] = primary_entry;
    
    //initialize all locksets to 0xffffffff
    // for (size_t i = 0; i< SS_SEC_TABLE_ENTRIES;i++) {
    //   struct Dr_Address_Data& dr_address_data = primary_entry[i];
    //   for (int j = 0 ; j < NUM_FIXED_ENTRIES ; j++) {
    // 	(dr_address_data.f_entries[j]).lockset = 0xffffffff;
    //   }
    // }
  }

  size_t offset = (addr) & 0x3fffff;
  struct Dr_Address_Data* dr_address_data = primary_entry + offset;

  PIN_GetLock(&dr_address_data->addr_lock, 0);

  if (dr_address_data == NULL) {
    // first access to the location. add access history to shadow space
    if (accessType == READ) {
      (dr_address_data->f_entries[0]).lockset = curLockset;
      (dr_address_data->f_entries[0]).r1_task = curStepNode;
    } else {
      (dr_address_data->f_entries[0]).lockset = curLockset;
      (dr_address_data->f_entries[0]).w1_task = curStepNode;
    }
  } else {
    // check for data race with each access history entry

    bool race_detected = false;
    int f_insert_index = -1;

    for(int i = 0 ; i < NUM_FIXED_ENTRIES; i++) {
      if(race_detected && f_insert_index != -1) break;

      struct Address_data& f_entry = dr_address_data->f_entries[i];

      if (f_insert_index == -1 && f_entry.lockset == 0) {
	f_insert_index = i;
	break;
      }
      
      if ((f_entry.lockset != 0) && (((~curLockset) & (~f_entry.lockset)) == 0)) {
	//check for data race
	if (f_entry.w1_task != NULL && 
	    taskGraph->areParallel(tidToTaskIdMap[threadid].top(), f_entry.w1_task, threadid)) {
	  all_violations.insert( std::pair<ADDRINT, 
				 struct violation* >(addr, 
						     new violation(new violation_data(curStepNode, accessType), 
								   new violation_data(f_entry.w1_task, WRITE))) );
	  race_detected = true;
	  break;
	}
	if (f_entry.w2_task != NULL && 
	    taskGraph->areParallel(tidToTaskIdMap[threadid].top(), f_entry.w2_task, threadid)) {
	  all_violations.insert( std::pair<ADDRINT, 
				 struct violation* >(addr, 
						     new violation(new violation_data(curStepNode, accessType), 
								   new violation_data(f_entry.w2_task, WRITE))) );
	  race_detected = true;
	  break;
	}
	if (accessType == WRITE) {
	  if (f_entry.r1_task != NULL && taskGraph->areParallel(tidToTaskIdMap[threadid].top(), f_entry.r1_task, threadid)) {
	    all_violations.insert( std::pair<ADDRINT, 
				   struct violation* >(addr, 
						       new violation(new violation_data(curStepNode, accessType), 
								     new violation_data(f_entry.r1_task, READ))) );
	    race_detected = true;
	    break;
	  } 
	  if (f_entry.r2_task != NULL && taskGraph->areParallel(tidToTaskIdMap[threadid].top(), f_entry.r2_task, threadid)) {
	    all_violations.insert( std::pair<ADDRINT, 
				   struct violation* >(addr, 
						       new violation(new violation_data(curStepNode, accessType), 
								     new violation_data(f_entry.r2_task, READ))) );
	    race_detected = true;
	    break;
	  }
	}
      } 
      if (f_entry.lockset != 0 && curLockset == f_entry.lockset){
	f_insert_index = i;
      }      
      
    }

    if (!race_detected) {
      int insert_index = -1;

      std::vector<struct Address_data>* access_list = dr_address_data->access_list;

      if (access_list != NULL) {
	for (std::vector<struct Address_data>::iterator it=access_list->begin();
	     it!=access_list->end(); ++it) {
	  struct Address_data& add_data = *it;
	  //check if intersection of lockset is empty
	  if (((~curLockset) & (~add_data.lockset)) == 0) {
	    //check for data race
	    if (add_data.w1_task != NULL && 
		taskGraph->areParallel(tidToTaskIdMap[threadid].top(), add_data.w1_task, threadid)) {
	  
	      all_violations.insert( std::pair<ADDRINT, 
				     struct violation* >(addr, 
							 new violation(new violation_data(curStepNode, accessType), 
								       new violation_data(add_data.w1_task, WRITE))) );
	      race_detected = true;
	      break;
	    }
	    if (add_data.w2_task != NULL && 
		taskGraph->areParallel(tidToTaskIdMap[threadid].top(), add_data.w2_task, threadid)) {
	  
	      all_violations.insert( std::pair<ADDRINT, 
				     struct violation* >(addr, 
							 new violation(new violation_data(curStepNode, accessType), 
								       new violation_data(add_data.w2_task, WRITE))) );
	      race_detected = true;
	      break;
	    }
	    if (accessType == WRITE) {
	      if (add_data.r1_task != NULL && taskGraph->areParallel(tidToTaskIdMap[threadid].top(), add_data.r1_task, threadid)) {
		all_violations.insert( std::pair<ADDRINT, 
				       struct violation* >(addr, 
							   new violation(new violation_data(curStepNode, accessType), 
									 new violation_data(add_data.r1_task, READ))) );
		race_detected = true;
		break;
	      } 
	      if (add_data.r2_task != NULL && taskGraph->areParallel(tidToTaskIdMap[threadid].top(), add_data.r2_task, threadid)) {
		all_violations.insert( std::pair<ADDRINT, 
				       struct violation* >(addr, 
							   new violation(new violation_data(curStepNode, accessType), 
									 new violation_data(add_data.r2_task, READ))) );
		race_detected = true;
		break;
	      }
	    }
	  } 
	  if (curLockset == add_data.lockset){
	    insert_index = it - access_list->begin();
	  }
	}
      }

      if(!race_detected) {
	if(f_insert_index != -1) {
	  struct Address_data& f_entry = dr_address_data->f_entries[f_insert_index];
	  if (f_entry.lockset == 0) {
	    f_entry.lockset = curLockset;
	    if (accessType == READ) {
	      f_entry.r1_task = curStepNode;
	    } else {
	      f_entry.w1_task = curStepNode;;
	    }
	  } else {
	    if (accessType == WRITE) {
	      //f_entry.wr_task = taskGraph->rightmostNode(curStepNode, f_entry.wr_task);
	      bool par_w1 = false, par_w2 = false;
	      if (f_entry.w1_task)
		par_w1 = taskGraph->areParallel(tidToTaskIdMap[threadid].top(),f_entry.w1_task, threadid);
	      if (f_entry.w2_task)
		par_w2 = taskGraph->areParallel(tidToTaskIdMap[threadid].top(),f_entry.w2_task, threadid);

	      if ((f_entry.w1_task == NULL && f_entry.w2_task == NULL) ||
		  (f_entry.w1_task == NULL && !par_w2) ||
		  (f_entry.w2_task == NULL && !par_w1) ||
		  (!par_w1 && !par_w2)) {
		f_entry.w1_task = curStepNode;
		f_entry.w2_task = NULL;
	      } else if (par_w1 && par_w2) {
		struct AFTask* lca12 = taskGraph->LCA(f_entry.w1_task, f_entry.w2_task);
		struct AFTask* lca1s = taskGraph->LCA(f_entry.w1_task, curStepNode);
		//struct AFTask* lca2s = static_cast<struct AFTask*>(taskGraph->LCA(addr_vec->r2_task, curTask));
		if (lca1s->depth < lca12->depth /*|| lca2s->depth < lca12->depth*/)
		  f_entry.w1_task = curStepNode;
	      } else if (f_entry.w2_task == NULL && par_w1) {
		f_entry.w2_task = curStepNode;
	      }
	    } else { //accessType == READ
	      bool par_r1 = false, par_r2 = false;
	      if (f_entry.r1_task)
		par_r1 = taskGraph->areParallel(tidToTaskIdMap[threadid].top(),f_entry.r1_task, threadid);
	      if (f_entry.r2_task)
		par_r2 = taskGraph->areParallel(tidToTaskIdMap[threadid].top(),f_entry.r2_task, threadid);

	      if ((f_entry.r1_task == NULL && f_entry.r2_task == NULL) ||
		  (f_entry.r1_task == NULL && !par_r2) ||
		  (f_entry.r2_task == NULL && !par_r1) ||
		  (!par_r1 && !par_r2)) {
		f_entry.r1_task = curStepNode;
		f_entry.r2_task = NULL;
	      } else if (par_r1 && par_r2) {
		struct AFTask* lca12 = taskGraph->LCA(f_entry.r1_task, f_entry.r2_task);
		struct AFTask* lca1s = taskGraph->LCA(f_entry.r1_task, curStepNode);
		//struct AFTask* lca2s = static_cast<struct AFTask*>(taskGraph->LCA(addr_vec->r2_task, curTask));
		if (lca1s->depth < lca12->depth /*|| lca2s->depth < lca12->depth*/)
		  f_entry.r1_task = curStepNode;
	      } else if (f_entry.r2_task == NULL && par_r1) {
		f_entry.r2_task = curStepNode;
	      }
	    }
	  }
	} else {
	  if (insert_index == -1) {
	    std::vector<struct Address_data>* access_list = dr_address_data->access_list;

	    if (access_list == NULL) {
	      dr_address_data->access_list = new std::vector<struct Address_data>();
	      access_list = dr_address_data->access_list;
	    }
	    // form address data
	    struct Address_data address_data;
	    address_data.lockset = curLockset;
	    if (accessType == READ) {
	      address_data.r1_task = curStepNode;	      
	    } else {
	      address_data.w1_task = curStepNode;
	    }
	    // add to access history
	    access_list->push_back(address_data);
	  } else {
	    // update access history
	    struct Address_data& update_data = access_list->at(insert_index);

	    if (accessType == WRITE) {
	      //update_data.wr_task = taskGraph->rightmostNode(curStepNode, update_data.wr_task);
	      bool par_w1 = false, par_w2 = false;
	      if (update_data.w1_task)
		par_w1 = taskGraph->areParallel(tidToTaskIdMap[threadid].top(),update_data.w1_task, threadid);
	      if (update_data.w2_task)
		par_w2 = taskGraph->areParallel(tidToTaskIdMap[threadid].top(),update_data.w2_task, threadid);

	      if ((update_data.w1_task == NULL && update_data.w2_task == NULL) ||
		  (update_data.w1_task == NULL && !par_w2) ||
		  (update_data.w2_task == NULL && !par_w1) ||
		  (!par_w1 && !par_w2)) {
		update_data.w1_task = curStepNode;
		update_data.w2_task = NULL;
	      } else if (par_w1 && par_w2) {
		struct AFTask* lca12 = taskGraph->LCA(update_data.w1_task, update_data.w2_task);
		struct AFTask* lca1s = taskGraph->LCA(update_data.w1_task, curStepNode);
		//struct AFTask* lca2s = static_cast<struct AFTask*>(taskGraph->LCA(addr_vec->r2_task, curTask));
		if (lca1s->depth < lca12->depth /*|| lca2s->depth < lca12->depth*/)
		  update_data.w1_task = curStepNode;
	      } else if (update_data.w2_task == NULL && par_w1) {
		update_data.w2_task = curStepNode;
	      }
	    } else { //accessType == READ
	      bool par_r1 = false, par_r2 = false;
	      if (update_data.r1_task)
		par_r1 = taskGraph->areParallel(tidToTaskIdMap[threadid].top(),update_data.r1_task, threadid);
	      if (update_data.r2_task)
		par_r2 = taskGraph->areParallel(tidToTaskIdMap[threadid].top(),update_data.r2_task, threadid);

	      if ((update_data.r1_task == NULL && update_data.r2_task == NULL) ||
		  (update_data.r1_task == NULL && !par_r2) ||
		  (update_data.r2_task == NULL && !par_r1) ||
		  (!par_r1 && !par_r2)) {
		update_data.r1_task = curStepNode;
		update_data.r2_task = NULL;
	      } else if (par_r1 && par_r2) {
		struct AFTask* lca12 = taskGraph->LCA(update_data.r1_task, update_data.r2_task);
		struct AFTask* lca1s = taskGraph->LCA(update_data.r1_task, curStepNode);
		//struct AFTask* lca2s = static_cast<struct AFTask*>(taskGraph->LCA(addr_vec->r2_task, curTask));
		if (lca1s->depth < lca12->depth /*|| lca2s->depth < lca12->depth*/)
		  update_data.r1_task = curStepNode;
	      } else if (update_data.r2_task == NULL && par_r1) {
		update_data.r2_task = curStepNode;
	      }
	    }
	  }
	}
      }
    }  
  }
  /////////////////////////////////////////////////////

  PIN_ReleaseLock(&dr_address_data->addr_lock);
  //PIN_ReleaseLock(&lock);
}

extern "C" void RecordMem(THREADID threadid, void * access_addr, AccessType accessType) {

  //PIN_GetLock(&lock, 0);

  // Exceptions
  ADDRINT addr = (ADDRINT) access_addr;
  if(exceptions(threadid, addr)){ 
    //PIN_ReleaseLock(&lock);
    return;
  }

  //get lockset and current step node
  size_t curLockset = (thd_lockset[threadid].top())->createLockset();
  struct AFTask* curStepNode = taskGraph->getCurTask(threadid);

  /////////////////////////////////////////////////////
  // check for access pattern and update shadow space

  size_t primary_index = (addr >> 22) & 0x3ff;
  struct Dr_Address_Data* primary_entry = shadow_space[primary_index];

  if (primary_entry == NULL) {
    size_t sec_length = (SS_SEC_TABLE_ENTRIES) * sizeof(struct Dr_Address_Data);
    primary_entry = (struct Dr_Address_Data*)mmap(0, sec_length, PROT_READ| PROT_WRITE,
						     MMAP_FLAGS, -1, 0);
    shadow_space[primary_index] = primary_entry;
    
    //initialize all locksets to 0xffffffff
    // for (size_t i = 0; i< SS_SEC_TABLE_ENTRIES;i++) {
    //   struct Dr_Address_Data& dr_address_data = primary_entry[i];
    //   for (int j = 0 ; j < NUM_FIXED_ENTRIES ; j++) {
    // 	(dr_address_data.f_entries[j]).lockset = 0xffffffff;
    //   }
    //}
  }

  size_t offset = (addr) & 0x3fffff;
  struct Dr_Address_Data* dr_address_data = primary_entry + offset;

  PIN_GetLock(&dr_address_data->addr_lock, 0);

  if (dr_address_data == NULL) {
    // first access to the location. add access history to shadow space
    //std::cout << "FIRST ACCESS\n";
    if (accessType == READ) {
      (dr_address_data->f_entries[0]).lockset = curLockset;
      (dr_address_data->f_entries[0]).r1_task = curStepNode;
    } else {
      (dr_address_data->f_entries[0]).lockset = curLockset;
      (dr_address_data->f_entries[0]).w1_task = curStepNode;
    }
  } else {
    // check for data race with each access history entry

    bool race_detected = false;
    int f_insert_index = -1;

    for(int i = 0 ; i < NUM_FIXED_ENTRIES; i++) {
      if(race_detected && f_insert_index != -1) break;

      struct Address_data& f_entry = dr_address_data->f_entries[i];

      if (f_insert_index == -1 && f_entry.lockset == 0) {
	f_insert_index = i;
	break;
      }
      
      //std::bitset<64> x(curLockset);
      //std::bitset<64> y(f_entry.lockset);
      //std::cout << "Cur = " << x << " fentry = " << y << std::endl;

      // if (addr == /*6325048*/6329176) {
      // 	std::cout << "ADDR INTEREST - ACC TYPE = " << accessType << "tid = " << tidToTaskIdMap[threadid].top() << std::endl;
      // 	//std::cout << "lockset = " << curLockset;
      // 	std::bitset<64> x(curLockset);
      // 	std::bitset<64> y(f_entry.lockset);
      // 	std::cout << "Cur = " << x << " fentry = " << y << std::endl;
      // }

      if ((f_entry.lockset != 0) && (((~curLockset) & (~f_entry.lockset)) == 0)) {
	//check for data race
	if (f_entry.w1_task != NULL && 
	    taskGraph->areParallel(tidToTaskIdMap[threadid].top(), f_entry.w1_task, threadid)) {
	  all_violations.insert( std::pair<ADDRINT, 
				 struct violation* >(addr, 
						     new violation(new violation_data(curStepNode, accessType), 
								   new violation_data(f_entry.w1_task, WRITE))) );
	  race_detected = true;
	  break;
	}
	if (f_entry.w2_task != NULL && 
	    taskGraph->areParallel(tidToTaskIdMap[threadid].top(), f_entry.w2_task, threadid)) {
	  all_violations.insert( std::pair<ADDRINT, 
				 struct violation* >(addr, 
						     new violation(new violation_data(curStepNode, accessType), 
								   new violation_data(f_entry.w2_task, WRITE))) );
	  race_detected = true;
	  break;
	}

	if (accessType == WRITE) {
	  if (f_entry.r1_task != NULL && taskGraph->areParallel(tidToTaskIdMap[threadid].top(), f_entry.r1_task, threadid)) {
	    all_violations.insert( std::pair<ADDRINT, 
				   struct violation* >(addr, 
						       new violation(new violation_data(curStepNode, accessType), 
								     new violation_data(f_entry.r1_task, READ))) );
	    race_detected = true;
	    break;
	  } 
	  if (f_entry.r2_task != NULL && taskGraph->areParallel(tidToTaskIdMap[threadid].top(), f_entry.r2_task, threadid)) {
	    all_violations.insert( std::pair<ADDRINT, 
				   struct violation* >(addr, 
						       new violation(new violation_data(curStepNode, accessType), 
								     new violation_data(f_entry.r2_task, READ))) );
	    race_detected = true;
	    break;
	  }
	}
      } 
      if (f_entry.lockset != 0 && curLockset == f_entry.lockset){
	f_insert_index = i;
      }      
      
    }

    if (!race_detected) {
      int insert_index = -1;

      std::vector<struct Address_data>* access_list = dr_address_data->access_list;

      if (access_list != NULL) {
	for (std::vector<struct Address_data>::iterator it=access_list->begin();
	     it!=access_list->end(); ++it) {
	  struct Address_data& add_data = *it;
	  //check if intersection of lockset is empty
	  if (((~curLockset) & (~add_data.lockset)) == 0) {
	    //check for data race
	    if (add_data.w1_task != NULL && 
		taskGraph->areParallel(tidToTaskIdMap[threadid].top(), add_data.w1_task, threadid)) {
	  
	      all_violations.insert( std::pair<ADDRINT, 
				     struct violation* >(addr, 
							 new violation(new violation_data(curStepNode, accessType), 
								       new violation_data(add_data.w1_task, WRITE))) );
	      race_detected = true;
	      break;
	    }
	    if (add_data.w2_task != NULL && 
		taskGraph->areParallel(tidToTaskIdMap[threadid].top(), add_data.w2_task, threadid)) {
	  
	      all_violations.insert( std::pair<ADDRINT, 
				     struct violation* >(addr, 
							 new violation(new violation_data(curStepNode, accessType), 
								       new violation_data(add_data.w2_task, WRITE))) );
	      race_detected = true;
	      break;
	    }
	    if (accessType == WRITE) {
	      if (add_data.r1_task != NULL && taskGraph->areParallel(tidToTaskIdMap[threadid].top(), add_data.r1_task, threadid)) {
		all_violations.insert( std::pair<ADDRINT, 
				       struct violation* >(addr, 
							   new violation(new violation_data(curStepNode, accessType), 
									 new violation_data(add_data.r1_task, READ))) );
		race_detected = true;
		break;
	      } 
	      if (add_data.r2_task != NULL && taskGraph->areParallel(tidToTaskIdMap[threadid].top(), add_data.r2_task, threadid)) {
		all_violations.insert( std::pair<ADDRINT, 
				       struct violation* >(addr, 
							   new violation(new violation_data(curStepNode, accessType), 
									 new violation_data(add_data.r2_task, READ))) );
		race_detected = true;
		break;
	      }
	    }
	  } 
	  if (curLockset == add_data.lockset){
	    insert_index = it - access_list->begin();
	  }
	}
      }

      if(!race_detected) {
	if(f_insert_index != -1) {
	  struct Address_data& f_entry = dr_address_data->f_entries[f_insert_index];
	  if (f_entry.lockset == 0) {
	    f_entry.lockset = curLockset;
	    if (accessType == READ) {
	      f_entry.r1_task = curStepNode;
	    } else {
	      f_entry.w1_task = curStepNode;;
	    }
	  } else {
	    if (accessType == WRITE) {
	      //f_entry.wr_task = taskGraph->rightmostNode(curStepNode, f_entry.wr_task);
	      bool par_w1 = false, par_w2 = false;
	      if (f_entry.w1_task)
		par_w1 = taskGraph->areParallel(tidToTaskIdMap[threadid].top(),f_entry.w1_task, threadid);
	      if (f_entry.w2_task)
		par_w2 = taskGraph->areParallel(tidToTaskIdMap[threadid].top(),f_entry.w2_task, threadid);

	      if ((f_entry.w1_task == NULL && f_entry.w2_task == NULL) ||
		  (f_entry.w1_task == NULL && !par_w2) ||
		  (f_entry.w2_task == NULL && !par_w1) ||
		  (!par_w1 && !par_w2)) {
		f_entry.w1_task = curStepNode;
		f_entry.w2_task = NULL;
	      } else if (par_w1 && par_w2) {
		struct AFTask* lca12 = taskGraph->LCA(f_entry.w1_task, f_entry.w2_task);
		struct AFTask* lca1s = taskGraph->LCA(f_entry.w1_task, curStepNode);
		//struct AFTask* lca2s = static_cast<struct AFTask*>(taskGraph->LCA(addr_vec->r2_task, curTask));
		if (lca1s->depth < lca12->depth /*|| lca2s->depth < lca12->depth*/)
		  f_entry.w1_task = curStepNode;
	      } else if (f_entry.w2_task == NULL && par_w1) {
		f_entry.w2_task = curStepNode;
	      }
	    } else { //accessType == READ
	      bool par_r1 = false, par_r2 = false;
	      if (f_entry.r1_task)
		par_r1 = taskGraph->areParallel(tidToTaskIdMap[threadid].top(),f_entry.r1_task, threadid);
	      if (f_entry.r2_task)
		par_r2 = taskGraph->areParallel(tidToTaskIdMap[threadid].top(),f_entry.r2_task, threadid);

	      if ((f_entry.r1_task == NULL && f_entry.r2_task == NULL) ||
		  (f_entry.r1_task == NULL && !par_r2) ||
		  (f_entry.r2_task == NULL && !par_r1) ||
		  (!par_r1 && !par_r2)) {
		f_entry.r1_task = curStepNode;
		f_entry.r2_task = NULL;
	      } else if (par_r1 && par_r2) {
		struct AFTask* lca12 = taskGraph->LCA(f_entry.r1_task, f_entry.r2_task);
		struct AFTask* lca1s = taskGraph->LCA(f_entry.r1_task, curStepNode);
		//struct AFTask* lca2s = static_cast<struct AFTask*>(taskGraph->LCA(addr_vec->r2_task, curTask));
		if (lca1s->depth < lca12->depth /*|| lca2s->depth < lca12->depth*/)
		  f_entry.r1_task = curStepNode;
	      } else if (f_entry.r2_task == NULL && par_r1) {
		f_entry.r2_task = curStepNode;
	      }
	    }
	  }
	} else {
	  if (insert_index == -1) {
	    std::vector<struct Address_data>* access_list = dr_address_data->access_list;

	    if (access_list == NULL) {
	      dr_address_data->access_list = new std::vector<struct Address_data>();
	      access_list = dr_address_data->access_list;
	    }
	    // form address data
	    struct Address_data address_data;
	    address_data.lockset = curLockset;
	    if (accessType == READ) {
	      address_data.r1_task = curStepNode;	      
	    } else {
	      address_data.w1_task = curStepNode;
	    }
	    // add to access history
	    access_list->push_back(address_data);
	  } else {
	    // update access history
	    struct Address_data& update_data = access_list->at(insert_index);

	    if (accessType == WRITE) {
	      //update_data.wr_task = taskGraph->rightmostNode(curStepNode, update_data.wr_task);
	      bool par_w1 = false, par_w2 = false;
	      if (update_data.w1_task)
		par_w1 = taskGraph->areParallel(tidToTaskIdMap[threadid].top(),update_data.w1_task, threadid);
	      if (update_data.w2_task)
		par_w2 = taskGraph->areParallel(tidToTaskIdMap[threadid].top(),update_data.w2_task, threadid);

	      if ((update_data.w1_task == NULL && update_data.w2_task == NULL) ||
		  (update_data.w1_task == NULL && !par_w2) ||
		  (update_data.w2_task == NULL && !par_w1) ||
		  (!par_w1 && !par_w2)) {
		update_data.w1_task = curStepNode;
		update_data.w2_task = NULL;
	      } else if (par_w1 && par_w2) {
		struct AFTask* lca12 = taskGraph->LCA(update_data.w1_task, update_data.w2_task);
		struct AFTask* lca1s = taskGraph->LCA(update_data.w1_task, curStepNode);
		//struct AFTask* lca2s = static_cast<struct AFTask*>(taskGraph->LCA(addr_vec->r2_task, curTask));
		if (lca1s->depth < lca12->depth /*|| lca2s->depth < lca12->depth*/)
		  update_data.w1_task = curStepNode;
	      } else if (update_data.w2_task == NULL && par_w1) {
		update_data.w2_task = curStepNode;
	      }
	    } else { //accessType == READ
	      bool par_r1 = false, par_r2 = false;
	      if (update_data.r1_task)
		par_r1 = taskGraph->areParallel(tidToTaskIdMap[threadid].top(),update_data.r1_task, threadid);
	      if (update_data.r2_task)
		par_r2 = taskGraph->areParallel(tidToTaskIdMap[threadid].top(),update_data.r2_task, threadid);

	      if ((update_data.r1_task == NULL && update_data.r2_task == NULL) ||
		  (update_data.r1_task == NULL && !par_r2) ||
		  (update_data.r2_task == NULL && !par_r1) ||
		  (!par_r1 && !par_r2)) {
		update_data.r1_task = curStepNode;
		update_data.r2_task = NULL;
	      } else if (par_r1 && par_r2) {
		struct AFTask* lca12 = taskGraph->LCA(update_data.r1_task, update_data.r2_task);
		struct AFTask* lca1s = taskGraph->LCA(update_data.r1_task, curStepNode);
		//struct AFTask* lca2s = static_cast<struct AFTask*>(taskGraph->LCA(addr_vec->r2_task, curTask));
		if (lca1s->depth < lca12->depth /*|| lca2s->depth < lca12->depth*/)
		  update_data.r1_task = curStepNode;
	      } else if (update_data.r2_task == NULL && par_r1) {
		update_data.r2_task = curStepNode;
	      }
	    }
	  }
	}
      }
    }  
  }

  /////////////////////////////////////////////////////
  PIN_ReleaseLock(&dr_address_data->addr_lock);
  //PIN_ReleaseLock(&lock);
}

static void report_access(struct violation_data* a) {
  report << a->task->taskId << "          ";
  if (a->accessType == READ)
    report << "READ\n";
  else
    report << "WRITE\n";
}

static void report_DR(ADDRINT addr, struct violation_data* a1, struct violation_data* a2) {
  report << "** Data race detected at " << addr << " **\n";
  report << "Accesses:\n";
  report << "TaskId    AccessType\n";
  report_access(a1);
  report_access(a2);
  report << "*******************************\n";
}

extern "C" void Fini()
{
  report.open("violations.out");

  for (std::map<ADDRINT,struct violation*>::iterator it=all_violations.begin();
       it!=all_violations.end(); ++it) {
    struct violation* viol = it->second;
    report_DR(it->first, viol->a1, viol->a2);
  }
  std::cout << "Number of violations = " << all_violations.size() << std::endl;
  report.close();
}
