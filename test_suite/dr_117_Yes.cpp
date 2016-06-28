#include<iostream>
#include "t_debug_task.h"

using namespace std;
using namespace tbb;

#define NUM_TASKS 22

int CHECK_AV *shd;

class Task1: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[getTaskId()]++;

    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task2: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[getTaskId()]++;
    
    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task3: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[getTaskId()]++;

    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task4: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[getTaskId()]++;

    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task5: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[getTaskId()]++;

    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task6: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[getTaskId()]++;

    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task7: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[getTaskId()]++;

    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task8: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[getTaskId()]++;

    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task9: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[getTaskId()]++;

    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task10: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[getTaskId()]++;

    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task11: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[getTaskId()]++;

    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task12: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[getTaskId()]++;

    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task13: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[getTaskId()]++;

    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task14: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[getTaskId()]++;

    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task15: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[getTaskId()]++;

    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task16: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[getTaskId()]++;

    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task17: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[2]++;

    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task18: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[getTaskId()]++;

    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task19: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[getTaskId()]++;

    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task20: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[getTaskId()]++;

    __exec_end__(getTaskId());
    return NULL;
  }
};

class Task21: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());

    // READ & WRITE
    shd[getTaskId()]++;

    __exec_end__(getTaskId());
    return NULL;
  }
};

class Driver: public t_debug_task {
public:
  task* execute() {
    __exec_begin__(getTaskId());
 
    set_ref_count(22);

    task& a = *new(task::allocate_child()) Task1();
    t_debug_task::spawn(a);

    task& b = *new(task::allocate_child()) Task2();
    t_debug_task::spawn(b);

    task& c = *new(task::allocate_child()) Task3();
    t_debug_task::spawn(c);

    task& d = *new(task::allocate_child()) Task4();
    t_debug_task::spawn(d);

    task& e = *new(task::allocate_child()) Task5();
    t_debug_task::spawn(e);

    task& f = *new(task::allocate_child()) Task6();
    t_debug_task::spawn(f);

    task& g = *new(task::allocate_child()) Task7();
    t_debug_task::spawn(g);

    task& h = *new(task::allocate_child()) Task8();
    t_debug_task::spawn(h);

    task& i = *new(task::allocate_child()) Task9();
    t_debug_task::spawn(i);

    task& j = *new(task::allocate_child()) Task10();
    t_debug_task::spawn(j);

    task& k = *new(task::allocate_child()) Task11();
    t_debug_task::spawn(k);

    task& l = *new(task::allocate_child()) Task12();
    t_debug_task::spawn(l);

    task& m = *new(task::allocate_child()) Task13();
    t_debug_task::spawn(m);

    task& n = *new(task::allocate_child()) Task14();
    t_debug_task::spawn(n);

    task& o = *new(task::allocate_child()) Task15();
    t_debug_task::spawn(o);

    task& p = *new(task::allocate_child()) Task16();
    t_debug_task::spawn(p);

    task& q = *new(task::allocate_child()) Task17();
    t_debug_task::spawn(q);

    task& s = *new(task::allocate_child()) Task18();
    t_debug_task::spawn(s);

    task& t = *new(task::allocate_child()) Task19();
    t_debug_task::spawn(t);

    task& u = *new(task::allocate_child()) Task20();
    t_debug_task::spawn(u);

    task& v = *new(task::allocate_child()) Task21();
    t_debug_task::spawn(v);

    t_debug_task::wait_for_all();
  
    __exec_end__(getTaskId());
    return NULL;
  }
};
 
int main( int argc, const char *argv[] ) {
  TD_Activate();

  shd = (int*)malloc(NUM_TASKS * sizeof(int));

  task& v = *new(task::allocate_root()) Driver();
  t_debug_task::spawn_root_and_wait(v);

  cout << "Addr of shd[2] " << (size_t)&shd[2] << std::endl;
  Fini();
}
