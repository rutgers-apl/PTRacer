#ifndef AFTASK_GRAPH_H
#define AFTASK_GRAPH_H

#include <assert.h>
#include <iostream>
#include <fstream>
#include <map>
#include <sys/mman.h>

#include "AFTask.H"
#include "Common.H"
#include "lca_hash.H"

#define MMAP_FLAGS (MAP_PRIVATE| MAP_ANONYMOUS| MAP_NORESERVE)
#define NUM_GRAPH_NODES 134217728

class AFTaskGraph {
private:
  struct AFTask* tgraph_nodes;
  size_t last_allocated_node;
  std::stack<size_t> cur[NUM_THREADS];
  std::map<size_t, size_t> temp_cur_map;

  bool parallel(struct AFTask* cur_task, struct AFTask* remote_task);
  void CaptureSpawnOnly(THREADID threadid, size_t taskId);
  void CaptureSpawnAndWait(THREADID threadid, size_t taskId);
  bool checkForStep (struct AFTask* cur_node);
  size_t create_node(NodeType node_type, size_t task_id);
  void initialize_task (size_t index,NodeType node_type, size_t val);

public:
  AFTaskGraph() {
    last_allocated_node = 0;
    size_t length = NUM_GRAPH_NODES * sizeof(struct AFTask);
    tgraph_nodes = (struct AFTask*) mmap(0, length, PROT_READ|PROT_WRITE, MMAP_FLAGS, -1, 0);
    assert (tgraph_nodes != (void*)-1);

    size_t head_node = create_node(FINISH, 0);
    cur[0].push(head_node);
    tgraph_nodes[head_node].depth = 0;
    tgraph_nodes[head_node].sp_root_n_wt_flag = true;

    lca_hash::createHashTable();
  }

  void printGraph(struct AFTask* node);
  void CaptureExecute(THREADID threadid, size_t taskId);
  void CaptureReturn(THREADID threadid);
  void CaptureWaitOnly(THREADID threadid);
  void CaptureWait(THREADID threadid);
  void CaptureSetTaskId(THREADID threadid, size_t taskId, int sp_only);
  struct AFTask* LCA(struct AFTask* cur_task, struct AFTask* remote_task);

  bool areParallel(size_t cur_task, struct AFTask* remote_task, THREADID threadid);
  struct AFTask* getCurTask(THREADID threadid);
  void Fini();
};

#endif
