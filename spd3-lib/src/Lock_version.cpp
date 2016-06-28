#include "Lock_version.H"

bool Lock_version::compareLock(struct Lock_version* l1, struct Lock_version* l2) {
  return ((l1->lock == l2->lock) && (l1->version == l2->version));
}
