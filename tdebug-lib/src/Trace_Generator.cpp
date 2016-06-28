#include "Trace_Generator.H"
#include <dirent.h>
#include <sstream>
#include <sys/stat.h>

std::stack<std::ofstream*> traces[NUM_THREADS];
std::ofstream br_check;

size_t thd_ins_ctr[NUM_THREADS];

extern "C" void TraceGenActivate() {
  DIR *dp = NULL;
  if(NULL == (dp = opendir("traces")) ) {
    mkdir("traces", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  }
  br_check.open("traces/br_checks");

  traces[0].push(new std::ofstream());
  std::string file_name = "traces/Trace_0";
  (traces[0].top())->open(file_name.c_str());
}

extern "C" void RecordTExecute(THREADID threadid) {
  traces[threadid].push(new std::ofstream());
  std::string file_name = "traces/Trace_"; 

  std::stringstream ss;
  ss << tidToTaskIdMap[threadid].top();
  file_name += ss.str();

  (traces[threadid].top())->open(file_name.c_str());
}

extern "C" void RecordTReturn(THREADID threadid) {
  std::ofstream* curStream = traces[threadid].top();
  traces[threadid].pop();
  curStream->close();
  delete(curStream);
}


extern "C" void RecordTLoad(THREADID threadid, void * addr) {
  *(traces[threadid].top()) << "R" 
			    << (ADDRINT) addr 
			    << "@" 
			    << tidToTaskIdMap[threadid].top()
			    << "." << thd_ins_ctr[threadid]++ << "\n";
}

extern "C" void RecordTStore(THREADID threadid, void * addr, void* value) {
  *(traces[threadid].top()) << "W" 
			    << (ADDRINT) addr 
			    << "@" 
			    << tidToTaskIdMap[threadid].top()
			    << "." << thd_ins_ctr[threadid]++
			    << "=" << *(int*)value << "\n";
}

extern "C" void RecordTLoadMain(void * addr) {
  *(traces[0].top()) << "R" 
			    << (ADDRINT) addr 
			    << "@" 
			    << 0
			    << "." << thd_ins_ctr[0]++ << "\n";
}

extern "C" void RecordTStoreMain(void * addr, void* value) {
  *(traces[0].top()) << "W" 
			    << (ADDRINT) addr 
			    << "@" 
			    << 0
			    << "." << thd_ins_ctr[0]++
			    << "=" << *(int*)value << "\n";
}

extern "C" void RecordTSpawn(THREADID threadid, void* childId) {
  *(traces[threadid].top()) << "S" 
			    << "@" 
			    << tidToTaskIdMap[threadid].top()
			    << "." << thd_ins_ctr[0]++
			    << "=" << *(int*)childId << "\n";
}

extern "C" void RecordTSpawnMain(void* childId) {
  *(traces[0].top()) << "S" 
			    << "@" 
			    << 0
			    << "." << thd_ins_ctr[0]++
			    << "=" << *(int*)childId << "\n";
}

extern "C" void RecordTLockAcq(THREADID threadid, void * lock) {
  *(traces[threadid].top()) << "L" 
			    << (ADDRINT) lock 
			    << "@" 
			    << tidToTaskIdMap[threadid].top()
			    << "." << thd_ins_ctr[threadid]++ << "\n";
}

extern "C" void RecordTLockRel(THREADID threadid, void * lock) {
  *(traces[threadid].top()) << "U" 
			    << (ADDRINT) lock 
			    << "@" 
			    << tidToTaskIdMap[threadid].top()
			    << "." << thd_ins_ctr[threadid]++ << "\n";
}

extern "C" void RecordTBrCheck(THREADID threadid, void * addr, void* value, CheckType cType) {
  if(cType == EQ) {
    br_check << "R" 
	     << (ADDRINT) addr
	     << "@"
	     << tidToTaskIdMap[threadid].top()
	     << "." << --thd_ins_ctr[threadid]
	     << " != " << *(int*) value << "\n";
  } else if(cType == NEQ) {
    br_check << "R" 
	     << (ADDRINT) addr
	     << "@"
	     << tidToTaskIdMap[threadid].top()
	     << "." << --thd_ins_ctr[threadid]
	     << " == " << *(int*) value << "\n";
  } else if(cType == GT) {
    br_check << "R" 
	     << (ADDRINT) addr
	     << "@"
	     << tidToTaskIdMap[threadid].top()
	     << "." << --thd_ins_ctr[threadid]
	     << " <= " << *(int*) value << "\n";
  } else if(cType == LT) {
    br_check << "R" 
	     << (ADDRINT) addr
	     << "@"
	     << tidToTaskIdMap[threadid].top()
	     << "." << --thd_ins_ctr[threadid]
	     << " >= " << *(int*) value << "\n";
  } else if(cType == GTE) {
    br_check << "R" 
	     << (ADDRINT) addr
	     << "@"
	     << tidToTaskIdMap[threadid].top()
	     << "." << --thd_ins_ctr[threadid]
	     << " < " << *(int*) value << "\n";
  } else if(cType == LTE) {
    br_check << "R" 
	     << (ADDRINT) addr
	     << "@"
	     << tidToTaskIdMap[threadid].top()
	     << "." << --thd_ins_ctr[threadid]
	     << " > " << *(int*) value << "\n";
  }
  thd_ins_ctr[threadid]++;
}
