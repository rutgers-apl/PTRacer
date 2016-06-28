#ifndef TASK_GRAPH_H
#define TASK_GRAPH_H

#include <iostream>
#include <vector>
#include <string>

#include "Task.H"
#include "Common.H"

class TaskGraph {
private:
  struct Task* searchTask (struct Task* node, size_t taskId);

protected:
  struct Task* head;
  struct Task* cur;
  struct Task* getTask(size_t taskId);
  void printGraph(struct Task* node);

public:
  void print();
  virtual void CaptureSetTaskId(THREADID threadid, size_t taskId, int sp_only);
  virtual void CaptureSpawn(THREADID threadid) = 0;
  virtual void CaptureSpawnWait(THREADID threadid) = 0;
  virtual void CaptureExecute(THREADID threadid, size_t taskId);
  virtual void CaptureReturn(THREADID threadid) = 0;
  virtual void CaptureWait(THREADID threadid) = 0;
  virtual void CaptureWaitOnly(THREADID threadid);
  virtual void Pause(THREADID threadid);
  virtual bool areParallel(size_t cur_task, size_t remote_task, THREADID threadid) = 0;
  virtual bool areParallel(size_t cur_task, struct Task* remote_task, THREADID threadid);
  virtual void StepCheck(THREADID threadid);
  virtual struct Task* getCurTask(THREADID threadid);
  virtual struct Task* LCA(struct Task* cur_task, struct Task* remote_task);

  void CaptureFnNames(std::string* rtn_name);
  virtual void Fini() {}
};
#endif
