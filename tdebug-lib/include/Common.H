#ifndef COMMON_H
#define COMMON_H

#define NUM_THREADS 64

#include <stack>
#include <pthread.h>

typedef unsigned long THREADID;
typedef unsigned long ADDRINT;
typedef pthread_mutex_t PIN_LOCK;

enum AccessType {
  READ = 0,
  WRITE = 1
};

extern PIN_LOCK lock;
extern std::stack<size_t> tidToTaskIdMap [NUM_THREADS];

inline void init_common() {
  tidToTaskIdMap[0].push(0);
}

inline void PIN_GetLock(PIN_LOCK* lock, int tid) {
  pthread_mutex_lock(lock);
}

inline void PIN_ReleaseLock(PIN_LOCK* lock) {
  pthread_mutex_unlock(lock);
}

#endif
