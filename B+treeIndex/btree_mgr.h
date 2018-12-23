#ifndef BTREE_MGR_H
#define BTREE_MGR_H

#include "dberror.h"
#include "tables.h"

// structure for accessing btrees
typedef struct BTreeHandle {
  DataType keyType;
  char *idxId;
  void *mgmtData;
} BTreeHandle;

typedef struct BT_ScanHandle {
  BTreeHandle *tree;
  void *mgmtData;
} BT_ScanHandle;

// init and shutdown index manager
extern RC initIndexManager (void *mgmtData);
extern RC shutdownIndexManager ();

// create, destroy, open, and close an btree index
extern RC createBtree (char *idxId, DataType keyType, int n);
extern RC openBtree (BTreeHandle **tree, char *idxId);
extern RC closeBtree (BTreeHandle *tree);
extern RC deleteBtree (char *idxId);

// access information about a b-tree
extern RC getNumNodes (BTreeHandle *tree, int *result);
extern RC getNumEntries (BTreeHandle *tree, int *result);
extern RC getKeyType (BTreeHandle *tree, DataType *result);

// index access
extern RC findKey (BTreeHandle *tree, Value *key, RID *result);
extern RC insertKey (BTreeHandle *tree, Value *key, RID rid);
extern RC deleteKey (BTreeHandle *tree, Value *key);
extern RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle);
extern RC nextEntry (BT_ScanHandle *handle, RID *result);
extern RC closeTreeScan (BT_ScanHandle *handle);

// debug and test functions
extern char *printTree (BTreeHandle *tree);


/*Condition Defination*/
#define forloop(start,cond,do) for(int start = 0;cond;start++){do}
#define condif(a,b) if(a){b}
#define elif(a,b) else if(a){b}
#define dec(a,b) a=b
#define unequal(a,b) a>=b
#define fof(a,b) for(a){b}
#define eqfn(a,b) a==b
#define lessthan(a,b) a<b
#define notequal(a,b) a!=b
#define greaterthan(a,b) a>b
#define greaterthanequal(a,b) a>=b
#define lessequalvalue(a,b) a<=b
#define whileloop(a,b) while(a){b}
#define equalvalue(a,b,c) a=b=c
#define plusequal(a,b) a+=b

#endif // BTREE_MGR_H
