#ifndef TRACE_GENERATOR_H
#define TRACE_GENERATOR_H

#include "Common.H"
#include <iostream>
#include <fstream>

enum CheckType {
EQ = 0,
  NEQ  = 1,
  GT = 2,
  LT = 3,
  GTE = 4,
  LTE = 5
  };

extern "C" void TraceGenActivate();
extern "C" void RecordTSpawn(THREADID threadid, void* childId);
extern "C" void RecordTSpawnMain(void* childId);
extern "C" void RecordTLoad(THREADID threadid, void * addr);
extern "C" void RecordTStore(THREADID threadid, void * addr, void* value);
extern "C" void RecordTLoadMain(void * addr);
extern "C" void RecordTStoreMain(void * addr, void* value);
extern "C" void RecordTLockAcq(THREADID threadid, void * lock);
extern "C" void RecordTLockRel(THREADID threadid, void * lock);
extern "C" void RecordTExecute(THREADID threadid);
extern "C" void RecordTReturn(THREADID threadid);
extern "C" void RecordTBrCheck(THREADID threadid, void * addr, void* value, CheckType cType);

#endif
