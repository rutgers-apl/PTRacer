#include "Lockset_data.H"
//#include "Common.H"

PIN_LOCK lockmap_lock;

int Lockset_data::lock_ticker = 0;
std::map<ADDRINT, size_t> Lockset_data::lockid_map;

Lockset_data::Lockset_data() {
  lockset = 0xffffffffffffffff;
}

size_t Lockset_data::createLockset() {
  return lockset;
}

void Lockset_data::addLockToStack (size_t lockId) {
  locks.push(lockId);
}

void Lockset_data::addLockToLockset(ADDRINT lock_held) {
  size_t lockId = getLockId(lock_held);
  addLockToStack(lockId);
  lockset = lockset & lockId;
}

void Lockset_data::removeLockFromLockset() {
  size_t lockId = locks.top();
  lockset = lockset | ~(lockId);
  locks.pop();
}

size_t Lockset_data::getLockId(ADDRINT lock_held) {
  size_t lockId;
  PIN_GetLock(&lockmap_lock, 0);
  if (lockid_map.count(lock_held) == 0) {
    lockId = ~((size_t)1 << lock_ticker++);
    lockid_map.insert(std::pair<ADDRINT, size_t>(lock_held, lockId));
  } else {
    lockId = lockid_map.at(lock_held);
  }
  PIN_ReleaseLock(&lockmap_lock);
  return lockId;
}
