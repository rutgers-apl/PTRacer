#ifndef __TBB_t_debug_task_H
#define __TBB_t_debug_task_H

#pragma GCC push_options
#pragma GCC optimize ("O0")

#include <map>
#include "tbb/task.h"
#include "exec_calls.h"

#define CHECK_AV __attribute__((type_annotate("check_av")))

namespace tbb {

  class t_debug_task: public task {
  private:
    size_t taskId;
    void setTaskId(size_t taskId, int sp_only) { this->taskId = taskId; }

  public:
    //static void __TBB_EXPORTED_METHOD spawn( task& t, size_t taskId );__attribute__((optimize(0)));
    static void __TBB_EXPORTED_METHOD spawn( task& t );//__attribute__((optimize(0)));
    static void __TBB_EXPORTED_METHOD spawn_root_and_wait( task& root );//__attribute__((optimize(0)));
    void spawn_and_wait_for_all( task& child );//__attribute__((optimize(0)));
    void wait_for_all( );//__attribute__((optimize(0)));
    size_t getTaskId() { return taskId; }
  };

  inline void t_debug_task::spawn(task& t) {
    (static_cast<t_debug_task&>(t)).setTaskId(++task_id_ctr, 1);
    taskGraph->CaptureSetTaskId(get_cur_tid(), (static_cast<t_debug_task&>(t)).getTaskId(), true);
    task::spawn(t);
  }

#if 0
  inline void t_debug_task::spawn(task& t) {
    (static_cast<t_debug_task&>(t)).setTaskId(++task_id_ctr, 1);
    task::spawn(t);
  }
#endif

  inline void t_debug_task::spawn_root_and_wait( task& root ) {
    (static_cast<t_debug_task&>(root)).setTaskId(++task_id_ctr, 0);
    taskGraph->CaptureSetTaskId(get_cur_tid(), (static_cast<t_debug_task&>(root)).getTaskId(), false);
    task::spawn_root_and_wait(root);
  }

  inline void t_debug_task::spawn_and_wait_for_all( task& child ) {
    (static_cast<t_debug_task&>(child)).setTaskId(++task_id_ctr, 1);
    taskGraph->CaptureSetTaskId(get_cur_tid(), (static_cast<t_debug_task&>(child)).getTaskId(), true);
    task::spawn_and_wait_for_all(child);
    taskGraph->CaptureWaitOnly(get_cur_tid());
  }

  inline void t_debug_task::wait_for_all( ) {
    task::wait_for_all();
    taskGraph->CaptureWaitOnly(get_cur_tid());
  }

}

#pragma GCC pop_options
#endif
