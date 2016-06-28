#ifndef LOCKSET_DATA_H
#define LOCKSET_DATA_H

#include <map>
#include <stack>
#include <vector>
#include "Lock_version.H"

struct Lockset_data {
private:
  std::vector<ADDRINT> lockset;
  std::map<ADDRINT, size_t> lock_version;
  std::stack<ADDRINT> locks;

  void addLockToStack (ADDRINT lock);
  void removeLock (ADDRINT lock);

public:
  void printLockset();
  std::vector<struct Lock_version*> createLockset();
  bool isLockPresent (ADDRINT lock);
  void addLockToLockset(ADDRINT lock);
  void removeLockFromLockset();
  void updateVersionNumber (ADDRINT lock);
};

#endif
