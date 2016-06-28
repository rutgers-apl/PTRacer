#ifndef ADDRESS_DATA_H
#define ADDRESS_DATA_H

#include <vector>
#include "Lock_version.H"
#include "AFTask.H"

#define R_LOCK 0
#define R_LOCK_VERSION 0

struct Address_data {
private:
public:
  //size_t taskId;
  struct AFTask* task;
  std::vector<struct Lock_version*> lockset;
  AccessType accessType;

  Address_data(struct AFTask* task, AccessType accessType) {
    this->task = task;
    this->accessType = accessType;
  }

  void checkForRLock();

  void printAddressData();

  static bool locksetEmpty(std::vector<struct Lock_version*> l1, std::vector<struct Lock_version*> l2);

  static bool compareTaskId(struct Address_data* a1, struct Address_data* a2);

  static bool conflictingAccess(struct Address_data* a1, struct Address_data* a2);

  static bool bothReads(struct Address_data* a1, struct Address_data* a2);
};

#endif
