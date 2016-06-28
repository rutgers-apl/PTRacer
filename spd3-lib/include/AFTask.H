#ifndef AFTASK_H
#define AFTASK_H

enum NodeType {
  NO_TYPE = 0,
  ASYNC = 1,
  FINISH = 2,
  STEP = 3
};

struct AFTask {
  size_t taskId;
  size_t parent;
  size_t cur_step;
  NodeType type;
  size_t depth;
  size_t seq_num;
  size_t num_children;
  NodeType young_ns_child; //NodeType of youngest non-step child
};

#endif
