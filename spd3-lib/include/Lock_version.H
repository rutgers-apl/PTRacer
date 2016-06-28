#ifndef LOCK_VERSION_H
#define LOCK_VERSION_H

#include "Common.H"
#include <iostream>

struct Lock_version {
private:
public:
  ADDRINT lock;
  size_t version;

  void printLockVersion() {
    std::cout << "Lock: " << lock << " Version: " << version << std::endl;
  }
  
  static bool compareLock(struct Lock_version* l1, struct Lock_version* l2);
};

#endif
