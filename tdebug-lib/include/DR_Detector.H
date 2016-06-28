#ifndef DR_DETECTOR_H
#define DR_DETECTOR_H

#include <assert.h>
#include <iostream>
#include <fstream>

#include "Lockset_data.H"
#include "Address_data.H"
#include "Common.H"
#include "AFTaskGraph.H"
#include <stdlib.h>
#include <cstring>

#define NUM_FIXED_ENTRIES 3

extern AFTaskGraph* taskGraph;

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
  struct Address_data f_entries[NUM_FIXED_ENTRIES];
  std::vector<struct Address_data>* access_list;
};

//public functions
extern "C" void TD_Activate();

void CaptureExecute(THREADID threadid);
void CaptureReturn(THREADID threadid);
extern "C" void RecordMem(THREADID threadid, void * addr, AccessType accessType);
extern "C" void RecordAccess(THREADID threadid, void * addr, ADDRINT* locks_acq, 
			     size_t locks_acq_size, ADDRINT* locks_rel, 
			     size_t locks_rel_size, AccessType accessType);
  
extern "C" void CaptureLockAcquire(THREADID threadid, ADDRINT lock_addr);
extern "C" void CaptureLockRelease(THREADID threadid, ADDRINT lock_addr);
extern "C" void Fini();

#endif
