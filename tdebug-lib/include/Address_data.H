#ifndef ADDRESS_DATA_H
#define ADDRESS_DATA_H

#include <vector>
#include "AFTask.H"

struct Address_data {
private:
public:
  size_t lockset;
  struct AFTask* w1_task;
  struct AFTask* w2_task;
  struct AFTask* r1_task;
  struct AFTask* r2_task;

  // Address_data(size_t lockset, 
  // 	       struct AFTask* wr_task, 
  // 	       struct AFTask* r1_task, 
  // 	       struct AFTask* r2_task) {
  //   this->lockset = lockset;
  //   this->wr_task = wr_task;
  //   this->r1_task = r1_task;
  //   this->r2_task = r2_task;
  // }
};

#endif
