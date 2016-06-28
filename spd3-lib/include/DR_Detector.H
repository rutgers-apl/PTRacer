#ifndef DR_DETECTOR_H
#define DR_DETECTOR_H

#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sys/mman.h>

#include "Common.H"
#include "AFTask.H"
#include "AFTaskGraph.H"

struct violation_data {
  struct AFTask* task;
  AccessType accessType;

  violation_data(struct AFTask* task, AccessType accessType) {
    this-> task = task;
    this->accessType = accessType;
  }
};

struct violation {
  struct violation_data* a1;
  struct violation_data* a2;

  violation(violation_data* a1,
	    violation_data* a2) {
    this->a1 = a1;
    this->a2 = a2;
  }
};

struct Dr_Address_Data {
  PIN_LOCK addr_lock;
  struct AFTask* wr_task;
  struct AFTask* r1_task;
  struct AFTask* r2_task;
};

extern AFTaskGraph* taskGraph;

//private functions
// static bool exceptions(THREADID threadid, ADDRINT addr);
// static void report_access(size_t a, AccessType a_type);
// static void report_race(ADDRINT addr, size_t a1, AccessType a1_type, size_t a2, AccessType a2_type);

//public functions
extern "C" void TD_Activate();
extern "C" void RecordMem(THREADID threadid, void * addr, AccessType accessType);
extern "C" void Fini();
void CaptureExecute(THREADID threadid);
void CaptureReturn(THREADID threadid);
extern "C" void CaptureLockAcquire(THREADID threadid, ADDRINT lock_addr);
extern "C" void CaptureLockRelease(THREADID threadid, ADDRINT lock_addr);
extern "C" void RecordAccess(THREADID threadid, void * addr, ADDRINT* locks_acq, 
			     size_t locks_acq_size, ADDRINT* locks_rel, 
			     size_t locks_rel_size, AccessType accessType);


#endif
