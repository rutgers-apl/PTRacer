#ifndef LCA_HASH_H
#define LCA_HASH_H

#define LCA_TABLE_SIZE 16000

#include <stdlib.h>
#include "AFTask.H"

enum ParallelStatus {
  NA = 0,
  TRUE = 1,
  FALSE = 2
};

struct lca_hash {
private:
  size_t curAddr;
  size_t remAddr;
  bool lca_result;
  struct AFTask* lca;

  static size_t getHTIndex(size_t arg);
  static struct lca_hash* getHTElement(size_t arg);
  static void setHTElement(size_t arg, struct lca_hash* entry);
public:
  lca_hash(size_t curAddr, size_t remAddr, bool lca_result, struct AFTask* lca) {
    this->curAddr = curAddr;
    this->remAddr = remAddr;
    this->lca_result = lca_result;
    this->lca = lca;
  }

  static struct lca_hash** lcaHashTable;
  static size_t tableSize;

  static int createHashTable();
  static void updateEntry(size_t cur_addr, size_t rem_addr, bool lca_result, struct AFTask* lca);
  static ParallelStatus checkParallel(size_t cur_addr, size_t rem_addr);
  static struct AFTask* getLCA(size_t cur_addr, size_t rem_addr);
};

#endif
