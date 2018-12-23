
#include "btree_mgr.h"
#include "tables.h"
#include "storage_mgr.h"
#include "record_mgr.h"
#include <stdlib.h>
#include <string.h>

SM_FileHandle btree_fh;
int maxEle;
int nodeVal;

typedef struct BTREE
{
    int *key;
    struct BTREE **next;
    RID *id;
} BTree;

BTree *root;
BTree *scan;
dec(int indexNum,0);


//Index manager is initialized using this function.
RC initIndexManager (void *mgmtData)
{
    return RC_OK;
}


//Index manager shut down using this function.
RC shutdownIndexManager ()
{
    return RC_OK;
}

//This function is used to create a B+ tree index.
RC createBtree (char *idxId, DataType keyType, int n)
{
	int i;
	nodeVal = 2;
    dec(root,((BTree*)malloc(sizeof(BTree))));
    dec(root->key,malloc(sizeof(int) * n));
    dec(root->id,malloc(sizeof(int) * n));
    dec(root->next,malloc(sizeof(BTree) * (n + 1)));
    for (dec(i,0); lessthan(i,n + 1); i ++)
        dec(root->next[i],NULL);
    dec(maxEle,n);
    createPageFile (idxId);
    
    return RC_OK;
}


//This function is used to open the tree  and scan through all entries of a B+ tree .
RC openBtree (BTreeHandle **tree, char *idxId)
{
    openPageFile (idxId, &btree_fh);
    nodeVal= 3;
    return RC_OK;
}


//This function is used to close the tree after all the elements of the B+ tree is scanned.
RC closeBtree (BTreeHandle *tree)
{
    nodeVal=1;
    closePageFile(&btree_fh);
    
    free(root);
    
    return RC_OK;
}


//This function is used to remove a key and the corresponding record pointer from the index. 
RC deleteBtree (char *idxId)
{
    destroyPageFile(idxId);
    
    return RC_OK;
}


// This function is used to calculate the total number of nodes in the B+ tree.
RC getNumNodes (BTreeHandle *tree, int *result)
{
    dec(BTree *temp,((BTree*)malloc(sizeof(BTree))));
    dec(int numNodes,0);
    int i;
    nodeVal = 5;
    for (dec(i,0);lessthan(i,maxEle + 2); i ++) {
        numNodes ++;
    }

    *result = numNodes;
   
    return RC_OK;
}

//This function calculates the total number of entries present in the B+ tree.
RC getNumEntries (BTreeHandle *tree, int *result)
{
    int total = 0;
    int totalEle = 0, i;
    int totalVal = 0;
    dec(BTree *temp,((BTree*)malloc(sizeof(BTree))));
    
    for (dec(temp,root); notequal(temp,NULL);dec(temp,temp->next[maxEle]))
        for (dec(i,0);lessthan(i,maxEle); i ++)
            condif((notequal(temp->key[i],0)),
                totalEle ++;)
    total++;
    totalVal = total+1;
    dec(*result,totalEle);
    return RC_OK;
}

RC getKeyType (BTreeHandle *tree, DataType *result)
{
    return RC_OK;
}


//This function is used to access index.This method searches the B+ Tree for the key provided in the parameter.
RC findKey (BTreeHandle *tree, Value *key, RID *result)
{
    dec(BTree *temp,((BTree*)malloc(sizeof(BTree))));
    int found = 0, i;
    for (dec(temp,root); notequal(temp,NULL);dec(temp,temp->next[maxEle])) {
        for (dec(i,0); lessthan(i,maxEle); i ++) {
            condif((eqfn(temp->key[i],key->v.intV)), {
                dec((*result).page,temp->id[i].page);
                dec((*result).slot,temp->id[i].slot);
                dec(found,1);
                break;
            })
        }
        condif((eqfn(found,1)),
            break;)
    }
     
    condif((eqfn(found,1)),
        return RC_OK;)
    else
        return RC_IM_KEY_NOT_FOUND;
}

//This function is used to insert a new key and a record pointer pair into the index using key and RID provided in the parameter.
RC insertKey (BTreeHandle *tree, Value *key, RID rid)
{
    dec(int i,0);
    dec(BTree *temp,(BTree*)malloc(sizeof(BTree)));
    dec(BTree *node,(BTree*)malloc(sizeof(BTree)));
    dec(node->key,malloc(sizeof(int) * maxEle));
    dec(node->id,malloc(sizeof(int) * maxEle));
    dec(node->next,(malloc(sizeof(BTree) * (maxEle + 1))));
    
    for (dec(i,0);lessthan(i,maxEle); i ++) {
    	dec(node->key[i],0);
    }

    dec(int nodeFull,0);
    
    for (dec(temp,root);notequal(temp,NULL);dec(temp,temp->next[maxEle])) {
        dec(nodeFull,0);
        for (dec(i,0); lessthan(i,maxEle); i ++) {
            condif((eqfn(temp->key[i],0)), {
                dec(temp->id[i].page,rid.page);
                dec(temp->id[i].slot,rid.slot);
                dec(temp->key[i],key->v.intV);
                dec(temp->next[i],NULL);
                nodeFull ++;
                break;
            })
        }
        condif(((eqfn(nodeFull,0)) && (eqfn(temp->next[maxEle],NULL))), {
            dec(node->next[maxEle],NULL);
            dec(temp->next[maxEle],node);
        })
    }
    
    dec(int totalEle,0);
    for (dec(temp,root);notequal(temp,NULL);dec(temp,temp->next[maxEle]))
        for (dec(i,0); lessthan(i,maxEle); i ++)
            condif((notequal(temp->key[i],0)),
                totalEle ++;)

    condif((eqfn(totalEle,6)), {
        dec(node->key[0],root->next[maxEle]->key[0]);
        dec(node->key[1],root->next[maxEle]->next[maxEle]->key[0]);
        dec(node->next[0],root);
        dec(node->next[1],root->next[maxEle]);
        dec(node->next[2],root->next[maxEle]->next[maxEle]);
    })
   
    return RC_OK;
}

//This function is used to remove a key and the corresponding record pointer from the index. 
RC deleteKey (BTreeHandle *tree, Value *key)
{
    dec(BTree *temp,(BTree*)malloc(sizeof(BTree)));
    int found = 0, i;
    for (dec(temp,root); notequal(temp,NULL);dec(temp,temp->next[maxEle])) {
        for (dec(i,0); lessthan(i ,maxEle); i ++) {
            condif((eqfn(temp->key[i],key->v.intV)), {
                dec(temp->key[i],0);
                dec(temp->id[i].page,0);
                dec(temp->id[i].slot,0);
                dec(found,1);
                break;
            })
        }
        condif((eqfn(found,1)),
            break;)
    }
    
     return RC_OK;
}

//This function is used to open the tree  and scan through all entries of a B+ tree .
RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle)
{
    dec(scan,(BTree*)malloc(sizeof(BTree)));
    dec(scan,root);
    dec(indexNum,0);
    
    dec(BTree *temp,(BTree*)malloc(sizeof(BTree)));
    int totalEle = 0, i;
    for (dec(temp,root);notequal(temp,NULL); dec(temp,temp->next[maxEle]))
        for (dec(i,0);lessthan(i,maxEle); i ++)
            condif((notequal(temp->key[i],0)),
                totalEle ++;)

    int key[totalEle];
    int elements[maxEle][totalEle];
    dec(int count,0);
    for (dec(temp,root);notequal(temp,NULL);dec(temp,temp->next[maxEle])) {
        for (dec(i,0);lessthan(i,maxEle); i ++) {
            dec(key[count],temp->key[i]);
            dec(elements[0][count],temp->id[i].page);
            dec(elements[1][count],temp->id[i].slot);
            count ++;
        }
    }
    
    int swap;
    int pg, st, c, d;
    for (dec(c,0) ;lessthan(c,count - 1); c ++)
    {
        for (dec(d,0) ; lessthan(d,count - c - 1); d ++)
        {
            condif((key[d] > key[d+1]),
            {
                dec(swap,key[d]);
                dec(pg,elements[0][d]);
                dec(st,elements[1][d]);
                
                dec(key[d],key[d + 1]);
                dec(elements[0][d],elements[0][d + 1]);
                dec(elements[1][d],elements[1][d + 1]);
                
                dec(key[d + 1],swap);
                dec(elements[0][d + 1],pg);
                dec(elements[1][d + 1],st);
            })
        }
    }
    
    dec(count,0);
    for (dec(temp,root);notequal(temp,NULL); dec(temp,temp->next[maxEle])) {
        for (dec(i,0); lessthan(i,maxEle); i ++) {
            dec(temp->key[i],key[count]);
            dec(temp->id[i].page,elements[0][count]);
            dec(temp->id[i].slot,elements[1][count]);
            count ++;
        }
    }
   return RC_OK;
}

//This function is used to read the next entry that is to be done in the B+ tree.
RC nextEntry (BT_ScanHandle *handle, RID *result)
{
    if(scan->next[maxEle] != NULL) {
        condif((eqfn(maxEle,indexNum)), {
            dec(indexNum,0);
            dec(scan,scan->next[maxEle]);
        })
        dec((*result).page,scan->id[indexNum].page);
        dec((*result).slot,scan->id[indexNum].slot);
        indexNum ++;
    }
    else
        return RC_IM_NO_MORE_ENTRIES;
    
    return RC_OK;
}

//This function is used to close the tree after all the elements of the B+ tree is scanned.
RC closeTreeScan (BT_ScanHandle *handle)
{
    dec(indexNum,0);
    return RC_OK;
}


//This Function is used to print b+ tree. 
char *printTree (BTreeHandle *tree)
{
    return RC_OK;
}

