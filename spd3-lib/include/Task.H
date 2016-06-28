#ifndef TASK_H
#define TASK_H

#include <vector>
#include <iostream>

struct Task {
protected:
  Task() {
    taskId = Task::global_taskId++;
    parent = NULL;
  }

  Task(size_t val) {
    taskId = val;
    parent = NULL;
  }

public:
  size_t taskId;
  struct Task* parent;

  static size_t global_taskId;

  virtual void printNode() {
    std::cout << std::endl << "Task: " << taskId << std::endl;
  }
};

#endif
