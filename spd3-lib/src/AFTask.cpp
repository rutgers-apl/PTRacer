#if 0
#include "AFTask.H"

//size_t AFTask::num_nodes;

void AFTask::setDepth() {
#if 0
  if (parent == 0)
    depth = 0;
  else
    depth = (static_cast<struct AFTask*>(parent))->depth + 1;
#endif
}

void AFTask::printNode () {
  std::cout << std::endl << "Task: " << taskId << " Type " << type << std::endl;
}

struct AFTask* AFTask::createAsyncNode() {
  //num_nodes++;
  return new AFTask(ASYNC);
}

struct AFTask* AFTask::createFinishNode() {
  //num_nodes++;
  return new AFTask(FINISH);
}

#if 0
struct AFTask* AFTask::createStepNode() {
    return new AFTask(STEP);
}
#endif

struct AFTask* AFTask::createAsyncNode(size_t taskId) {
  //num_nodes++;
  return new AFTask(ASYNC, taskId);
}

struct AFTask* AFTask::createFinishNode(size_t taskId) {
  //num_nodes++;
  return new AFTask(FINISH, taskId);
}

struct AFTask* AFTask::createStepNode(size_t taskId) {
  //num_nodes++;
  return new AFTask(STEP, taskId);
}
#endif
