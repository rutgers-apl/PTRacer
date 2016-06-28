#include "DR_Detector.H"
#include "AFTask.H"

// 2^10 entries each will be 8 bytes each
const size_t SS_PRIMARY_TABLE_ENTRIES = ((size_t) 1024);

// each secondary entry has 2^ 22 entries (64 bytes each)
const size_t SS_SEC_TABLE_ENTRIES = ((size_t) 4*(size_t) 1024 * (size_t) 1024);

struct Dr_Address_Data** shadow_space;

std::ofstream report;

std::map<ADDRINT, struct violation*> all_violations;

extern "C" void TD_Activate() {
  taskGraph = new AFTaskGraph();

  size_t primary_length = (SS_PRIMARY_TABLE_ENTRIES) * sizeof(struct Dr_Address_Data*);
  shadow_space = (struct Dr_Address_Data**)mmap(0, primary_length, PROT_READ| PROT_WRITE,
		      MAP_PRIVATE|MAP_ANONYMOUS|MAP_NORESERVE, -1, 0);
  assert(shadow_space != (void *)-1);
}

bool exceptions (THREADID threadid, ADDRINT addr) {
  return (tidToTaskIdMap[threadid].empty() || 
	  tidToTaskIdMap[threadid].top() == 0 ||
	  all_violations.count(addr) != 0
	  );
}

extern "C" void RecordMem(THREADID threadid, void * addr, AccessType accessType) {
  //PIN_GetLock(&lock, 0);

  ADDRINT addr_addr = (ADDRINT) addr;

  // Exceptions
  if(exceptions(threadid, addr_addr)){ 
    //PIN_ReleaseLock(&lock);
    return;
  }

  //check for DR with previous accesses
  size_t primary_index = (addr_addr >> 22) & 0x3ff;
  struct Dr_Address_Data* primary_ptr = shadow_space[primary_index];
  if (primary_ptr == NULL) {
    //mmap secondary table
    size_t sec_length = (SS_SEC_TABLE_ENTRIES) * sizeof(struct Dr_Address_Data);
    primary_ptr = (struct Dr_Address_Data*)mmap(0, sec_length, PROT_READ| PROT_WRITE,
		      MAP_PRIVATE|MAP_ANONYMOUS|MAP_NORESERVE, -1, 0);
    shadow_space[primary_index] = primary_ptr;
  }

  size_t offset = (addr_addr) & 0x3fffff;
  struct Dr_Address_Data* addr_vec = primary_ptr + offset;

  PIN_GetLock(&addr_vec->addr_lock, 0);
  
  if (addr_vec != NULL) {
    //if the tasks can execute in parallel
    if (addr_vec->wr_task != NULL && taskGraph->areParallel(tidToTaskIdMap[threadid].top(), addr_vec->wr_task, threadid)) {
      //report race
      all_violations.insert( std::pair<ADDRINT, 
      			     struct violation* >(addr_addr,
      						 new violation(new violation_data(taskGraph->getCurTask(threadid), accessType),
      							       new violation_data(addr_vec->wr_task, WRITE))) );
    }
    if (accessType == WRITE) {
      if (addr_vec->r1_task != NULL && taskGraph->areParallel(tidToTaskIdMap[threadid].top(), addr_vec->r1_task, threadid)) {
	//report race
	all_violations.insert( std::pair<ADDRINT,
			       struct violation* >(addr_addr,
						   new violation(new violation_data(taskGraph->getCurTask(threadid), accessType),
								 new violation_data(addr_vec->r1_task, READ))) );	  
      } else if (addr_vec->r2_task != NULL && taskGraph->areParallel(tidToTaskIdMap[threadid].top(), addr_vec->r2_task, threadid)) {
	//report race
	all_violations.insert( std::pair<ADDRINT,
			       struct violation* >(addr_addr,
						   new violation(new violation_data(taskGraph->getCurTask(threadid), accessType),
								 new violation_data(addr_vec->r2_task, READ))) );
      }
    }
  }

  //update shadow space
  if (accessType == WRITE) {
    addr_vec->wr_task = taskGraph->getCurTask(threadid);
  } else {
    //Update read
    if (addr_vec->r1_task == NULL) {
      addr_vec->r1_task = taskGraph->getCurTask(threadid);
      PIN_ReleaseLock(&addr_vec->addr_lock);
      return;
    } else if (addr_vec->r2_task == NULL) {
      addr_vec->r2_task = taskGraph->getCurTask(threadid);
      PIN_ReleaseLock(&addr_vec->addr_lock);
      return;
    }
    
    bool cur_r1 = taskGraph->areParallel(tidToTaskIdMap[threadid].top(),addr_vec->r1_task, threadid);
    bool cur_r2 = taskGraph->areParallel(tidToTaskIdMap[threadid].top(),addr_vec->r2_task, threadid);
    if (!cur_r1 && !cur_r2) {
      addr_vec->r1_task = taskGraph->getCurTask(threadid);
      addr_vec->r2_task = NULL;
    } else if (cur_r1 && cur_r2) {
      struct AFTask* curTask = taskGraph->getCurTask(threadid);
      struct AFTask* lca12 = taskGraph->LCA(addr_vec->r1_task, addr_vec->r2_task);
      struct AFTask* lca1s = taskGraph->LCA(addr_vec->r1_task, curTask);
      //struct AFTask* lca2s = static_cast<struct AFTask*>(taskGraph->LCA(addr_vec->r2_task, curTask));
      if (lca1s->depth < lca12->depth /*|| lca2s->depth < lca12->depth*/)
	addr_vec->r1_task = curTask;
    }
  }

  PIN_ReleaseLock(&addr_vec->addr_lock);
}

extern "C" void RecordAccess(THREADID threadid, void * addr, ADDRINT* locks_acq, 
			     size_t locks_acq_size, ADDRINT* locks_rel, 
			     size_t locks_rel_size, AccessType accessType) {}
void CaptureExecute(THREADID threadid) {}

void CaptureReturn(THREADID threadid) {}

extern "C" void CaptureLockAcquire(THREADID threadid, ADDRINT lock_addr) {}

extern "C" void CaptureLockRelease(THREADID threadid, ADDRINT lock_addr) {}

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
