#ifndef EXEC_CALLS_H
#define EXEC_CALLS_H

#pragma GCC push_options
#pragma GCC optimize ("O0")

#include "tbb/atomic.h"
#include "AFTaskGraph.H"
#include "DR_Detector.H"
#include "Trace_Generator.H"
#include "tbb/tbb_thread.h"

typedef tbb::internal::tbb_thread_v3::id TBB_TID;

extern std::map<TBB_TID, size_t> tid_map;

extern PIN_LOCK tid_map_lock;

extern tbb::atomic<size_t> task_id_ctr;

extern tbb::atomic<size_t> tid_ctr;

extern AFTaskGraph* taskGraph;

extern "C" {
  __attribute__((noinline)) void __exec_begin__(unsigned long taskId);
  
  __attribute__((noinline)) void __exec_end__(unsigned long taskId);

  size_t __TBB_EXPORTED_METHOD get_cur_tid( );
}


#pragma GCC pop_options

#endif
