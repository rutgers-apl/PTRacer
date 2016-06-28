#include<iostream>
#include "t_debug_task.h"
#include "tbb/mutex.h"

using namespace std;
using namespace tbb;

#define NUM_TASKS 22

int shd[NUM_TASKS];
int CHECK_AV shd_var = 0;
bool c;

tbb::mutex x_mutex;
tbb::mutex::scoped_lock myLock;

class Task1: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    RecordTLockAcq(get_cur_tid(), &x_mutex);
    CaptureLockAcquire(get_cur_tid(), (ADDRINT)&x_mutex);
    myLock.acquire(x_mutex);

    int str_1 = 1;
    RecordTStore (get_cur_tid(), &c, &str_1);
    c = true;

    // READ & WRITE
    RecordTLoad (get_cur_tid(), &shd_var);
    int tmp = shd_var + 1;
    RecordTStore (get_cur_tid(), &shd_var, &tmp);
    shd_var++;

    RecordTLockRel(get_cur_tid(), &x_mutex);
    CaptureLockRelease(get_cur_tid(), (ADDRINT)&x_mutex);
    myLock.release();

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task2: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    // READ & WRITE
    RecordTLoad (get_cur_tid(), &shd[getTaskId()]);
    int tmp = shd[getTaskId()] + 1;
    RecordTStore (get_cur_tid(), &shd[getTaskId()], &tmp);
    shd[getTaskId()]++;

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task3: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    // READ & WRITE
    RecordTLoad (get_cur_tid(), &shd[getTaskId()]);
    int tmp = shd[getTaskId()] + 1;
    RecordTStore (get_cur_tid(), &shd[getTaskId()], &tmp);
    shd[getTaskId()]++;

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task4: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    // READ & WRITE
    RecordTLoad (get_cur_tid(), &shd[getTaskId()]);
    int tmp = shd[getTaskId()] + 1;
    RecordTStore (get_cur_tid(), &shd[getTaskId()], &tmp);
    shd[getTaskId()]++;

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task5: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    // READ & WRITE
    RecordTLoad (get_cur_tid(), &shd[getTaskId()]);
    int tmp = shd[getTaskId()] + 1;
    RecordTStore (get_cur_tid(), &shd[getTaskId()], &tmp);
    shd[getTaskId()]++;

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task6: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    // READ & WRITE
    RecordTLoad (get_cur_tid(), &shd[getTaskId()]);
    int tmp = shd[getTaskId()] + 1;
    RecordTStore (get_cur_tid(), &shd[getTaskId()], &tmp);
    shd[getTaskId()]++;

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task7: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    // READ & WRITE
    RecordTLoad (get_cur_tid(), &shd[getTaskId()]);
    int tmp = shd[getTaskId()] + 1;
    RecordTStore (get_cur_tid(), &shd[getTaskId()], &tmp);
    shd[getTaskId()]++;

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task8: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    // READ & WRITE
    RecordTLoad (get_cur_tid(), &shd[getTaskId()]);
    int tmp = shd[getTaskId()] + 1;
    RecordTStore (get_cur_tid(), &shd[getTaskId()], &tmp);
    shd[getTaskId()]++;

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task9: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    // READ & WRITE
    RecordTLoad (get_cur_tid(), &shd[getTaskId()]);
    int tmp = shd[getTaskId()] + 1;
    RecordTStore (get_cur_tid(), &shd[getTaskId()], &tmp);
    shd[getTaskId()]++;

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task10: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    // READ & WRITE
    RecordTLoad (get_cur_tid(), &shd[getTaskId()]);
    int tmp = shd[getTaskId()] + 1;
    RecordTStore (get_cur_tid(), &shd[getTaskId()], &tmp);
    shd[getTaskId()]++;

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task11: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    // READ & WRITE
    RecordTLoad (get_cur_tid(), &shd[getTaskId()]);
    int tmp = shd[getTaskId()] + 1;
    RecordTStore (get_cur_tid(), &shd[getTaskId()], &tmp);
    shd[getTaskId()]++;

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task12: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    // READ & WRITE
    RecordTLoad (get_cur_tid(), &shd[getTaskId()]);
    int tmp = shd[getTaskId()] + 1;
    RecordTStore (get_cur_tid(), &shd[getTaskId()], &tmp);
    shd[getTaskId()]++;

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task13: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    // READ & WRITE
    RecordTLoad (get_cur_tid(), &shd[getTaskId()]);
    int tmp = shd[getTaskId()] + 1;
    RecordTStore (get_cur_tid(), &shd[getTaskId()], &tmp);
    shd[getTaskId()]++;

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task14: public t_debug_task {
public:
  task* execute() {

    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    RecordTLockAcq(get_cur_tid(), &x_mutex);
    CaptureLockAcquire(get_cur_tid(), (ADDRINT)&x_mutex);
    myLock.acquire(x_mutex);

    RecordTLoad (get_cur_tid(), &c);
    if (c == true) {
      int str_2 = 1;
      RecordTBrCheck(get_cur_tid(), &c, &str_2, EQ);

      // READ & WRITE
      RecordTLoad (get_cur_tid(), &shd[getTaskId()]);
      int tmp = shd[getTaskId()] + 1;
      RecordTStore (get_cur_tid(), &shd[getTaskId()], &tmp);
      shd[getTaskId()]++;

      RecordTLockRel(get_cur_tid(), &x_mutex);
      CaptureLockRelease(get_cur_tid(), (ADDRINT)&x_mutex);
      myLock.release();
    } else {
      int str_2 = 1;
      RecordTBrCheck(get_cur_tid(), &c, &str_2, NEQ);

      RecordTLockRel(get_cur_tid(), &x_mutex);
      CaptureLockRelease(get_cur_tid(), (ADDRINT)&x_mutex);
      myLock.release();
      
      // READ & WRITE
      RecordTLoad (get_cur_tid(), &shd_var);
      int tmp = shd_var + 1;
      RecordTStore (get_cur_tid(), &shd_var, &tmp);
      shd_var++;

    }

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task15: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    // READ & WRITE
    RecordTLoad (get_cur_tid(), &shd[getTaskId()]);
    int tmp = shd[getTaskId()] + 1;
    RecordTStore (get_cur_tid(), &shd[getTaskId()], &tmp);
    shd[getTaskId()]++;

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task16: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    // READ & WRITE
    RecordTLoad (get_cur_tid(), &shd[getTaskId()]);
    int tmp = shd[getTaskId()] + 1;
    RecordTStore (get_cur_tid(), &shd[getTaskId()], &tmp);
    shd[getTaskId()]++;

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task17: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    // READ & WRITE
    RecordTLoad (get_cur_tid(), &shd[getTaskId()]);
    int tmp = shd[getTaskId()] + 1;
    RecordTStore (get_cur_tid(), &shd[getTaskId()], &tmp);
    shd[getTaskId()]++;

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task18: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    // READ & WRITE
    RecordTLoad (get_cur_tid(), &shd[getTaskId()]);
    int tmp = shd[getTaskId()] + 1;
    RecordTStore (get_cur_tid(), &shd[getTaskId()], &tmp);
    shd[getTaskId()]++;

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task19: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    // READ & WRITE
    RecordTLoad (get_cur_tid(), &shd[getTaskId()]);
    int tmp = shd[getTaskId()] + 1;
    RecordTStore (get_cur_tid(), &shd[getTaskId()], &tmp);
    shd[getTaskId()]++;

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task20: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    // READ & WRITE
    RecordTLoad (get_cur_tid(), &shd[getTaskId()]);
    int tmp = shd[getTaskId()] + 1;
    RecordTStore (get_cur_tid(), &shd[getTaskId()], &tmp);
    shd[getTaskId()]++;

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task21: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
    RecordTExecute(get_cur_tid());

    // READ & WRITE
    RecordTLoad (get_cur_tid(), &shd[getTaskId()]);
    int tmp = shd[getTaskId()] + 1;
    RecordTStore (get_cur_tid(), &shd[getTaskId()], &tmp);
    shd[getTaskId()]++;

    RecordTReturn(get_cur_tid());
    __exec_end__(getTaskId());
    return NULL;
  }
};
 
int main( int argc, const char *argv[] ) {
  TD_Activate();
  TraceGenActivate();

  int str_1 = 0;
  RecordTStoreMain(&c, &str_1);
  c = false;
  t_debug_task& a = *new(task::allocate_root()) Task1();
  t_debug_task::spawn(a);
  int a_cid = a.getTaskId();
  RecordTSpawnMain(&a_cid);

  t_debug_task& b = *new(task::allocate_root()) Task2();
  t_debug_task::spawn(b);
  int b_cid = b.getTaskId();
  RecordTSpawnMain(&b_cid);

  t_debug_task& c = *new(task::allocate_root()) Task3();
  t_debug_task::spawn(c);
  int c_cid = c.getTaskId();
  RecordTSpawnMain(&c_cid);

  t_debug_task& d = *new(task::allocate_root()) Task4();
  t_debug_task::spawn(d);
  int d_cid = d.getTaskId();
  RecordTSpawnMain(&d_cid);

  t_debug_task& e = *new(task::allocate_root()) Task5();
  t_debug_task::spawn(e);
  int e_cid = e.getTaskId();
  RecordTSpawnMain(&e_cid);

  t_debug_task& f = *new(task::allocate_root()) Task6();
  t_debug_task::spawn(f);
  int f_cid = f.getTaskId();
  RecordTSpawnMain(&f_cid);

  t_debug_task& g = *new(task::allocate_root()) Task7();
  t_debug_task::spawn(g);
  int g_cid = g.getTaskId();
  RecordTSpawnMain(&g_cid);

  t_debug_task& h = *new(task::allocate_root()) Task8();
  t_debug_task::spawn(h);
  int h_cid = h.getTaskId();
  RecordTSpawnMain(&h_cid);

  t_debug_task& i = *new(task::allocate_root()) Task9();
  t_debug_task::spawn(i);
  int i_cid = i.getTaskId();
  RecordTSpawnMain(&i_cid);

  t_debug_task& j = *new(task::allocate_root()) Task10();
  t_debug_task::spawn(j);
  int j_cid = j.getTaskId();
  RecordTSpawnMain(&j_cid);

  t_debug_task& k = *new(task::allocate_root()) Task11();
  t_debug_task::spawn(k);
  int k_cid = k.getTaskId();
  RecordTSpawnMain(&k_cid);

  t_debug_task& l = *new(task::allocate_root()) Task12();
  t_debug_task::spawn(l);
  int l_cid = l.getTaskId();
  RecordTSpawnMain(&l_cid);

  t_debug_task& m = *new(task::allocate_root()) Task13();
  t_debug_task::spawn(m);
  int m_cid = m.getTaskId();
  RecordTSpawnMain(&m_cid);

  t_debug_task& n = *new(task::allocate_root()) Task14();
  t_debug_task::spawn(n);
  int n_cid = n.getTaskId();
  RecordTSpawnMain(&n_cid);

  t_debug_task& o = *new(task::allocate_root()) Task15();
  t_debug_task::spawn(o);
  int o_cid = o.getTaskId();
  RecordTSpawnMain(&o_cid);

  t_debug_task& p = *new(task::allocate_root()) Task16();
  t_debug_task::spawn(p);
  int p_cid = p.getTaskId();
  RecordTSpawnMain(&p_cid);

  t_debug_task& q = *new(task::allocate_root()) Task17();
  t_debug_task::spawn(q);
  int q_cid = q.getTaskId();
  RecordTSpawnMain(&q_cid);

  t_debug_task& s = *new(task::allocate_root()) Task18();
  t_debug_task::spawn(s);
  int s_cid = s.getTaskId();
  RecordTSpawnMain(&s_cid);

  t_debug_task& t = *new(task::allocate_root()) Task19();
  t_debug_task::spawn(t);
  int t_cid = t.getTaskId();
  RecordTSpawnMain(&t_cid);

  t_debug_task& u = *new(task::allocate_root()) Task20();
  t_debug_task::spawn(u);
  int u_cid = u.getTaskId();
  RecordTSpawnMain(&u_cid);

  t_debug_task& v = *new(task::allocate_root()) Task21();
  t_debug_task::spawn_root_and_wait(v);
  int v_cid = v.getTaskId();
  RecordTSpawnMain(&v_cid);

  for (size_t i = 0 ; i < 1000000000 ; i++);
  cout << "Addr of shd_var " << (size_t)&shd_var << std::endl;

  RecordTReturn(0);
  Fini();
}
