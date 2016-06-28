#include "Lockset_data.H"

void Lockset_data::printLockset() {
  std::cout << "******* LOCKSET *******\n";
  for (std::vector<ADDRINT>::iterator it = lockset.begin() ; it != lockset.end(); ++it) {
    std::cout << *it << std::endl;
  }
  std::cout << "***********************\n";
}

std::vector<struct Lock_version*> Lockset_data::createLockset() {
  std::vector<struct Lock_version*> ret_lockset;
  for (std::vector<ADDRINT>::iterator it = lockset.begin() ; it != lockset.end(); ++it) {
    struct Lock_version* lockset_version = new Lock_version();
    lockset_version->lock = *it;
    lockset_version->version = lock_version.at(*it);
    ret_lockset.push_back(lockset_version);
  }
  return ret_lockset;
}

bool Lockset_data::isLockPresent (ADDRINT lock) {
  for (std::vector<ADDRINT>::iterator it = lockset.begin() ; it != lockset.end(); ++it) {
    if (*it == lock)
      return true;
  }
  return false;
}

void Lockset_data::removeLock (ADDRINT lock) {
  for (std::vector<ADDRINT>::iterator it = lockset.begin() ; it != lockset.end(); ++it) {
    if (*it == lock) {
      lockset.erase(it);
      return;
    }
  }
}

void Lockset_data::addLockToStack (ADDRINT lock) {
  locks.push(lock);
}

void Lockset_data::addLockToLockset(ADDRINT lock) {
  addLockToStack(lock);
  lockset.push_back(lock);
}

void Lockset_data::removeLockFromLockset() {
  ADDRINT lock = locks.top();
  removeLock(lock);
  locks.pop();
}

void Lockset_data::updateVersionNumber (ADDRINT lock) {
  if (lock_version.count(lock) == 0) {
    lock_version.insert( std::pair<ADDRINT, size_t>(lock,0) );
  } else {
    lock_version.at(lock)++;
  }
}
