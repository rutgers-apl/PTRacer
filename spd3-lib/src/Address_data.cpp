#include "Address_data.H"

void Address_data::printAddressData() {
  std::cout << "******* Task " << task->taskId << " *******\n";
  for (std::vector<struct Lock_version*>::iterator it1 = lockset.begin() ; it1 != lockset.end(); ++it1) {
    (*it1)->printLockVersion();
  }
  if (accessType == READ)
    std::cout << "Accesstype READ\n";
  else
    std::cout << "Accesstype WRITE\n";
std::cout << "*************************************\n";
}

void Address_data::checkForRLock() {
  if (accessType == READ) {
    struct Lock_version* lockset_version = new Lock_version();
    lockset_version->lock = R_LOCK;
    lockset_version->version = R_LOCK_VERSION;
    lockset.push_back(lockset_version);
  }
}

//The lockset of 2 locksets is their intersection
bool Address_data::locksetEmpty(std::vector<struct Lock_version*> l1, std::vector<struct Lock_version*> l2) {
  for (std::vector<struct Lock_version*>::iterator it1 = l1.begin() ; it1 != l1.end(); ++it1) {
    for (std::vector<struct Lock_version*>::iterator it2 = l2.begin() ; it2 != l2.end(); ++it2) {
      if (Lock_version::compareLock(*it1, *it2)) {
	return false;
      }
    }
  }
  return true;
}

bool Address_data::compareTaskId(struct Address_data* a1, struct Address_data* a2) {
  return (a1->task->taskId == a2->task->taskId);
}

bool Address_data::conflictingAccess(struct Address_data* a1, struct Address_data* a2) {
  return (a1->task->taskId != a2->task->taskId && (a1->accessType == WRITE || a2->accessType == WRITE));
}

bool Address_data::bothReads(struct Address_data* a1, struct Address_data* a2) {
  return (a1->accessType == READ && a2->accessType == READ);
}
