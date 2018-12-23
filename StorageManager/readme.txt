
Assignment 1 - Storage Manager
Submitted by Group No. 7 (msingh36@hawk.iit.edu, kshankar1@hawk.iit.edu ,rkothakapu@hawk.iit.edu)

=====================================================================================
The assignment consists of 3 C files and 3 header files , a Makefile and a README: 
1. storage_mgr.c
2. storage_mgr.h
3. test_assign1.c
4. test_helper.h
5. dberror.c
6. dberror.h
7. Makefile
8. README

======================================Makefile=======================================

All the files are present in the Bitbucket repository: ()

Any of the below commands can be used to run the file :
1. make 
2. make all

After executing the above command,two output file "test_assign1_1" and "test_assign1_2" is created. We can execute the created file by running the command "./test_assign1_1" and "./test_assign1_2" on terminal

Command used to clean the created file:"make clean"

======================================storage_mgr.c========================================

storage_mgr.c contains the definition of interface declared in storage_mgr.h.Below is the description of implementation of each function in storage_mgr.c :

void initStorageManager (void): 
1. All the global variables used in this assignment should be declared in this block.
2. However,we have not initialized any variable in it as it is not required to initialize any variable in "init" block

RC createPageFile (char *fileName): 
1. This function creates a new file in write mode with empty block written in it.

RC openPageFile (char *fileName, SM_FileHandle *fHandle):
1. This function opens the file in read and write mode
2. It stores the value of totalNumPages, curPagePos, fileName.Also,It store the POSIX file pointer in mgmtInfo

int getBlockPos (SM_FileHandle *fHandle):
1.This function returns the value of current block position.It uses curPagePos variable of the file handler for returning the position value. 

RC closePageFile (SM_FileHandle *fHandle):
1. This function closes the file using fclose() and de-initialize the file handler

RC destroyPageFile (char *fileName):
1. This function deletes the file from the directory handled by file handler.

RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage):
1. This function checks if the page exists using the totalNumPages.If the page exists,it seeks the file pointer until that page using built-in function "fseek"
2. It then reads the file into PageHandler from the seeked location for the next 4096 bytes using built-in function "fread". If the page is not present, it returns "NON_EXISTING_PAGE" error

RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage):
1.This function is used to calls the readBlock function.The readBlock function have pageNum=0

RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage):
1.This function is used to calls the readBlock function with the pageNum value which is "pageNum=curPagePos-1".

RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage):
1.This function calls the readBlock function with the pageNum value which is "pageNum=curPagePos+1"

RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage):
1.This function calls the readBlock functions with the pageNum value which is "pageNum=curPagePos"

RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage):
1.This function calls the readBlock function with the pageNum value which is pageNum=totalNumPages-1

RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage):
1.This function calls the writeBlock function with the pageNum value which is pageNum=curPagePos

RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage):
This function performs below functionality:
1. It checks whether the page number is greater than or equal to 0.
2. If it is greater than or equal to 0 then, it seeks the file pointer using built-in function "fseek" until that page.
3. If the page to be written on file in greater than totalNumPages,then it increases the pages of the file with value totalNumPages=pageNum+1.
4. Then it writes the data to the seeked location from the PageHandler for the next 4096 bytes using built-in function "fwite".
5. If the page does not exists,it return "INVALID_PAGE_NUMBER" error.

RC appendEmptyBlock (SM_FileHandle *fHandle):
This function calls the writeBlock function with the pageNum and memPage value which is pageNum=totalNumPages and memPage=emptyblock

RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle):
This function calls the writeBlock function with the pageNum and memPage value which is pageNum=numberOfPages-1 and memPage=emptyblock

===============================================test_assign1_2==================================================

In addition to the given test cases, we have also implemented some other testcases in additional "test_assign1_2" file 
Each of the test cases process the file "test_pagefile.bin"
