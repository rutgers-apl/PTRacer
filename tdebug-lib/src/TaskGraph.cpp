#include "TaskGraph.H"

size_t Task::global_taskId;

bool TaskGraph::areParallel(size_t cur_task, struct Task* remote_task, THREADID threadid) {
  return false;
}

struct Task* TaskGraph::getCurTask(THREADID threadid) {
  return cur;
}

struct Task* TaskGraph::LCA(struct Task* cur_task, struct Task* remote_task) {
  return NULL;
}

void TaskGraph::print() {
  printGraph(head);
}

void TaskGraph::printGraph(struct Task* node) {
  node->printNode();
  //std::cout << "-------------- CHILDREN ------------------" << std::endl;
  //for (std::vector<struct Task*>::iterator it = node->children.begin() ; it != node->children.end(); ++it) {
  //printGraph(*it);
  //}
  //std::cout << "-------------- CHILDREN END ------------------" << std::endl;
}

void TaskGraph::CaptureExecute(THREADID threadid, size_t taskId)
{
  PIN_GetLock(&lock, 0);
  tidToTaskIdMap[threadid].push(cur->taskId);
  PIN_ReleaseLock(&lock);
}

void TaskGraph::CaptureFnNames(std::string* rtn_name)
{
  PIN_GetLock(&lock, 0);
  std::cout << *rtn_name << std::endl;
  PIN_ReleaseLock(&lock);
}

struct Task* TaskGraph::searchTask (struct Task* node, size_t taskId) {
  if (node->taskId == taskId) {
    return node;
  }
  //for (std::vector<struct Task*>::iterator it = node->children.begin() ; it != node->children.end(); ++it) {
  //struct Task* ret = searchTask(*it, taskId);
  // if (ret) return ret;
  //}
  return NULL;
}

struct Task* TaskGraph::getTask(size_t taskId) {
  return searchTask(head, taskId);
}

void TaskGraph::StepCheck(THREADID threadid)
{
  return;
}

void TaskGraph::CaptureWaitOnly(THREADID threadid)
{
  return;
}

void TaskGraph::Pause(THREADID threadid)
{
  return;
}

void TaskGraph::CaptureSetTaskId(THREADID threadid, size_t taskId, int sp_only){
  return;
}
