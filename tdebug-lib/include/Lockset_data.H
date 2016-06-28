#ifndef LOCKSET_DATA_H
#define LOCKSET_DATA_H

#include <map>
#include <stack>
#include "Common.H"

struct Lockset_data {
private:
  static int lock_ticker;
  static std::map<ADDRINT, size_t> lockid_map;

  size_t lockset;
  std::stack<size_t> locks;

  void addLockToStack (size_t lockId);

public:
  Lockset_data();
  size_t createLockset();
  void addLockToLockset(ADDRINT lock);
  void removeLockFromLockset();
  static size_t getLockId(ADDRINT lock);
};

#endif
