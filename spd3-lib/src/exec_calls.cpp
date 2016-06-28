#pragma GCC push_options
#pragma GCC optimize ("O0")

#include "exec_calls.h"
#include "Common.H"
#include "t_debug_task.h"

tbb::atomic<size_t> task_id_ctr;
tbb::atomic<size_t> tid_ctr;
PIN_LOCK lock;
PIN_LOCK tid_map_lock;
std::map<TBB_TID, size_t> tid_map;
std::stack<size_t> tidToTaskIdMap [NUM_THREADS];
AFTaskGraph* taskGraph;

extern "C" {
  __attribute__((noinline)) void __exec_begin__(unsigned long taskId){
    taskGraph->CaptureExecute(get_cur_tid(), taskId);
    CaptureExecute(get_cur_tid());
  }

  __attribute__((noinline)) void __exec_end__(unsigned long taskId){
    taskGraph->CaptureReturn(get_cur_tid());
    CaptureReturn(get_cur_tid());
  }

  __attribute__((noinline))  size_t get_cur_tid() {
    TBB_TID pthd_id = tbb::this_tbb_thread::get_id();
    size_t my_tid;
    PIN_GetLock(&tid_map_lock, 0);
    if (tid_map.count(pthd_id) == 0) {
      my_tid = tid_ctr++;
      tid_map.insert(std::pair<TBB_TID, size_t>(pthd_id, my_tid));
      PIN_ReleaseLock(&tid_map_lock);
    } else {
      PIN_ReleaseLock(&tid_map_lock);
      my_tid = tid_map.at(pthd_id);
    }

    return my_tid;
  }
}


#pragma GCC pop_options
