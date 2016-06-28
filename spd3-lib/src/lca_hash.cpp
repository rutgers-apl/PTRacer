#include "lca_hash.H"
#include <iostream>

struct lca_hash** lca_hash::lcaHashTable;
size_t lca_hash::tableSize;

size_t lca_hash::getHTIndex (size_t arg){
  size_t ret_val = arg%tableSize;
  return ret_val;
}

struct lca_hash* lca_hash::getHTElement(size_t arg){
  struct lca_hash* ret_val = lcaHashTable[getHTIndex(arg)];
  if (ret_val != NULL && ret_val->curAddr == arg) {
    return ret_val;
  } else {
    return NULL;
  }
}

void lca_hash::setHTElement(size_t arg, struct lca_hash* entry){
  lcaHashTable[getHTIndex(arg)] = entry;
}

int lca_hash::createHashTable() {
  tableSize =  LCA_TABLE_SIZE;
  lcaHashTable = (struct lca_hash **) malloc(sizeof(struct lca_hash*) * tableSize);
  if (lcaHashTable == NULL) {
      return 0;
  } else {
    for(size_t i=0;i<tableSize;i++) {
      lcaHashTable[i] = NULL;
    }
  }
  return 1;
}

void lca_hash::updateEntry(size_t cur_addr, size_t rem_addr, bool lca_result, struct AFTask* lca)
{    
  struct lca_hash* cur = getHTElement(cur_addr);
  if (cur == NULL) {//then create
    //initialize values
    cur = new lca_hash(cur_addr, rem_addr,lca_result, lca);
    setHTElement(cur_addr, cur);
  } else {
    cur->remAddr = rem_addr;
    cur->lca_result = lca_result;
    cur->lca = lca;
  }
}

ParallelStatus lca_hash::checkParallel(size_t cur_addr, size_t rem_addr) {
  struct lca_hash* cur = getHTElement(cur_addr);
  if (cur != NULL) {
    if  (cur->remAddr == rem_addr) {
      if(cur->lca_result == true)
	return TRUE;
      else
	return FALSE;
    } else {
      struct lca_hash* rem = getHTElement(rem_addr);
      if (rem != NULL) {
	if  (rem->remAddr == cur_addr) {
	  if(rem->lca_result == true)
	    return TRUE;
	  else
	    return FALSE;	  
	} else {
	  return NA;
	}
      } else {
	return NA;
      }
    }
  } else { //cur == NULL
    struct lca_hash* rem = getHTElement(rem_addr);
    if (rem != NULL) {
      if  (rem->remAddr == cur_addr) {
	if(rem->lca_result == true)
	  return TRUE;
	else
	  return FALSE;	  
      } else {
	return NA;
      }
    } else {
      return NA;
    }    
  }
}

struct AFTask* lca_hash::getLCA(size_t cur_addr, size_t rem_addr) {
  struct lca_hash* cur = getHTElement(cur_addr);
  if (cur != NULL) {
    if  (cur->remAddr == rem_addr) {
      return cur->lca;
    } else {
      struct lca_hash* rem = getHTElement(rem_addr);
      if (rem != NULL) {
	if  (rem->remAddr == cur_addr) {
	  return rem->lca;
	} else {
	  return NULL;
	}
      } else {
	return NULL;
      }
    }
  } else { //cur == NULL
    struct lca_hash* rem = getHTElement(rem_addr);
    if (rem != NULL) {
      if  (rem->remAddr == cur_addr) {
	return rem->lca;
      } else {
	return NULL;
      }
    } else {
      return NULL;
    }    
  }
}
