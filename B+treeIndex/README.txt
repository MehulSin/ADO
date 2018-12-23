
Assignment 4: B+ - Tree Index
Submitted by Group No. 7 (msingh36@hawk.iit.edu, kshankar@hawk.iit.edu,rkothakapu@hawk.iit.edu)
*************************************************************************************************

RUNNING THE SCRIPT
=================================================================================================================

1) Using terminal,go at Project root (assignment4) directory,then execute command ls to list the files and verify the correct location of directory.

3) Execute "make clean" command to remove old compiled .o files.

4) Execute "make" to compile project files.It will comple all the project files including "test_assign4_1.c" file. 

5) Execute "test_assign4.c" file by using "./test_assign4" command.

6) Execute "test_expr.c" file using "./test_expr" command.

=================================================================================================================
Description of functions used
==================================================================================================================
1)B+-Tree Index Functions
=======================================================================================
  
Below function are for creating, deleting,opening, and closing B+ tree index:	

createBtree(...)
This function is used to create a B+ tree index.
		
deleteBtree(...)
This function is used to delete a B+ tree index.It also removes the corresponding page file.

openBtree(...)
This function is used to open a B+ tree index created.

closeBtree(...)
This function is used to close a B+ tree index which is already opened
The index manager ensures that all new or modified pages of the index are flushed back to disk. 

====================================================================================================
2. ACCESSING B+ TREE FUNCTIONS
====================================================================================================

We can find, insert, and delete keys in/from a given B+ Tree from the below function.Also,openTreeScan, nextEntry, and closeTreeScan methods is used to scan all entries of a B+ Tree in sorted way.


findKey(...)
This method searches the B+ Tree for the key provided in the parameter.
If it found that the key does not exist,it returns RC_IM_KEY_NOT_FOUND.

insertKey(...)
This function is used to insert a new key and a record pointer pair into the index using key and RID provided in the parameter.
Incase if that key is already stored in the B+ tree,it returns the error code RC_IM_KEY_ALREADY_EXISTS .

deleteKey(...)
This function is used to remove a key and the corresponding record pointer from the index. 
If the key is not present in the index then RC_IM_KEY_NOT_FOUND is returned.

openTreeScan(...)
This function is used to open the tree  and scan through all entries of a B+ tree . 

nextEntry(...)
This function is used to read the next entry that is to be done in the B+ tree.

closeTreeScan(...)
This function is used to close the tree after all the elements of the B+ tree is scanned.

=======================================================================================================
3. ACCESS INFORMATION ABOUT OUR B+ TREE
=======================================================================================================

We can get information of our B+ tree using these function.Various information that we can retrieve using this function are number of nodes and keys in tree 

getNumNodes(...)
This function is used to calculate the total number of nodes in the B+ tree.

getNumEntries(...)
This function calculates the total number of entries present in the B+ tree.

getKeyType(...)
The datatype of the keys being stored in our B+ Tree get returned by this function.
This information is stored in our TreeManager structure in "keyType" variable. Then,this data is returned.


==========================================================================================================
4.DEBUGGING AND TEST FUNCTIONS
==========================================================================================================
We debug B+ tree using this function

printTree(...)
This Function is used to print b+ tree. 

============================================================================================================
5.Changes in test_assign4_1.c
============================================================================================================

Line number: 253 ASSERT_EQUALS_INT(numInserts, i, "have seen all entries");

numInserts and i will not be equal. We need to increment i by 1.

Please check line numbers from 253 to 256 in test_assign4_1.c


6. INITIALIZE AND SHUTDOWN INDEX MANAGER
=================================================

Index manager is initialized and shut down using this function.It frees all the resources used.

initIndexManager(...)
Index manager is initialized using this function.
initStorageManager(...) function is used for initializing the storage manager. 

shutdownIndexManager(...)
Index manager shut down using this function.It de-allocates all the resources allocated to the index manager.




