Assignment-2:
	The goal of this assignment is to accomplish a buffer manager of a database. 
	
Submitted by Group No. 7 (msingh36@hawk.iit.edu, kshankar1@hawk.iit.edu ,rkothakapu@hawk.iit.edu)


RUNNING THE SCRIPT
=========================

1) Go to Project root (assign2) using Terminal.

2) Type ls to list the files and check that we are in the correct directory.

3) Type "make clean" to delete old compiled .o files.

4) Type "make" to compile all project files including "test_assign2_1.c" and "test_assign2_2.c" file 

5) Type "./test1" to run "test_assign2_1.c" file.

6) Type "./test2" to run "test_assign2_2.c" file.

==========================================================================
This function consisit of below files :
	Makefile
	README.txt
	buffer_mgr.c
	buffer_mgr.h
	buffer_mgr_stat.c
	buffer_mgr_stat.h
	dberror.c
	dberror.h
        dt.h 
	storage_mgr.c
	storage_mgr.h
	test_assign2_1.c
	test_assign2_2.c
	test_helper.h

=========================================================================================================================

1. BUFFER POOL FUNCTIONS:

The functions related to buffer pool are used to create a buffer pool for an existing page file on disk.The page file is present on disk while the buffer pool is created in memory. We have used Storage Manager (Assignment 1) to perform operations on page file on disk.

initBufferPool(...)
--> A new buffer pool is created in memory by this function.
--> The size of buffer is defined by the parameter numPages i.e. number of page frames that can be stored in the buffer.
--> The name of the page file are stored in pageFileName ,whose pages are cached in memory.
--> Various page replacement strategy (FIFO, LRU, LFU, CLOCK)  are represented by the strategy that will be used by this buffer pool
--> Parameters are passed using stratData if any to the page replacement strategy. 

shutdownBufferPool(...)
--> This function destroys the buffer pool i.e. it shutdowns the bufferpool.
--> All resources/memory space that is being used by the Buffer Manager for the buffer pool gets free.
--> We call forceFlushPool(...) before destroying the buffer pool. It writes all the dirty pages (modified pages) to the disk.
--> It will throw RC_PINNED_PAGES_IN_BUFFER error,if any page is being used by any client.

forceFlushPool(...)
--> All the dirty pages (modified pages whose dirtyBit = 1) are written to the disk using this function.
--> All the page frames in buffer pool is verified by this function and whether the dirtyBit = 1 (it indicates that some client modified content of the page frame) and fixCount = 0 (it indicates that page frame is not being used by any user).If both the conditions are satisfied then it writes the page frame to the page file on disk.


2. PAGE MANAGEMENT FUNCTIONS
==========================

The page management related functions are used to perform below functionality :
		a)load pages from disk into the buffer pool (pin pages), 
		b)remove a page frame from buffer pool (unpin page),
		c)mark page as dirty ,and 
		d)force a page fram to be written to the disk.

pinPage(...)
--> The page number pageNun gets pin by this function i.e, it reads the page from the page file present on disk and stores it in the buffer pool.
--> It checks if the buffer pool has an empty space or not ,before pinning a page . If there is an empty space, then the page frame gets stored in the buffer pool else a page in the buffer pool gets replaced by a page replacement strategy.
--> We have implemented FIFO, LRU, LFU and CLOCK page replacement strategies which are used while pinning a page.
--> The page has to be replaced gets decided by the page replacement algorithms.It checks whether that respective page is dirty or not. It it has dirtyBit = 1, then the contents of the page frame is written to the page file on disk and the new page is placed at that location where the old page was.

unpinPage(...)
--> The specified page gets unpinned using this function.Page's pageNum decide whether it has to be unpinned or not.
--> Once the page is located using a loop,the fixCount of that page gets decremented by 1 which means that the client is no longer using this page.

makeDirty(...)
--> This function is used to set the dirtyBit of the specified page frame to 1.
--> The page frame gets located through pageNum by iteratively checking each page in the buffer pool and when the page id found, it set the dirtyBit = 1 for that page.

forcePage(....)
--> The content of the specified page frame is written to the page file present on disk using.
--> It locates the specified page using pageNum by checking all the pages in the buffer loop using a loop construct.
--> When the page is found, it uses the Storage Manager functions to write the content of the page frame to the page file on disk. After writing, it sets dirtyBit = 0 for that page.


3. STATISTICS FUNCTIONS
===========================

The statistics related functions gathers some information about the buffer pool. Various statistical information about the buffer pool are provided by these function.

getFrameContents(...)
--> An array of PageNumbers is returned by this function. Here, array size = buffer size (numPages).
--> We get value of pageNum by iterating over all the page frames present in the buffer pool. 

getDirtyFlags(...)
--> An array of bools is returned by this function.Here,array size = buffer size (numPages).
--> We retrieve the value of the dirtyBit of the page frame present in the buffer pool by iterating over all the page frame in the buffer pool
--> If the page stored in the "n"th page frame is dirty,the "n"th element is the TRUE .

getFixCounts(...) 
--> An array of ints is returned by this function.Here,array size = buffer size (numPages).
--> We retrieve the value of the fixCount of the page frame present in the buffer pool by iterating over all the page frame in the buffer pool
--> The "n"th element is the fixCount of the page stored in the "n"th page frame.

getNumReadIO(...)
--> The count of total number of IO reads performed by the buffer pool i.e. number of pages read from the disk is returned by this function.
--> The rearIndex variable is used to maintain this data.

getNumWriteIO(...)
--> The count of total number of IO writes performed by the buffer pool i.e. number of pages written to the disk is returned by this function.
--> This data is maintained using the writeCount variable. writeCount gets initialized to 0 when buffer pool is initialized and gets incremented whenever a page frame is written to disk.


4. PAGE REPLACEMENT ALGORITHM FUNCTIONS
=========================================

The page replacement strategy functions implements FIFO, LRU, LFU, CLOCK algorithms.It is used while pinning a page. If the buffer pool is full and there is a new page has that need to be pinned, then a page should be replaced from the buffer pool. The page that has to be replaced from the buffer pool get determined using replacement strategies.

FIFO(...)
--> First In First Out (FIFO) is the most basic page replacement strategy used and is generally like a queue where the page which comes first in the buffer pool is in front and that page will be replaced first when the buffer pool is full.
-->We write the content of the page frame to the page file on disk after the page is located and then add the new page at that location.

LFU(...)
--> Least Frequently Used (LFU) looks for the page that is used least times comapred to other page frames in the buffer pool and removes it from the page frame.
--> The variable (field) refNum in each page frame is used for this purpose. It keeps a count of of the page frames that is being accessed by the client.
--> So while using LFU, we need to find the position of the page frame that has the lowest value of refNum.
--> The content of the page frame is written to the page file on disk and then the new page is added at that location.
--> Also,the position of the least frequently used page frame is stored in a variable "lfuPointer" so that we can use it next time while replacing a page in the buffer pool. It reduces the number of iterations from 2nd page replacement onwards.

LRU(...)
--> Least Recently Used (LRU)looks for the page frame that hasn't been used for a long time (least recent) amongst the other page frames in the buffer pool and removes it.
--> The variable (field) hitNum in each page frame is used for this purpose. It keeps count of the page frames that has being accessed and pinned by the client. We also use a global variable "hit" for this purpose.
--> So while using LRU, we just need to find the position of the page frame with the lowest value of hitNum.
--> The content of the page frame is written to the page file on disk and then new page is added at that location.

CLOCK(...)
--> This algorithm is used to keep track of the last added page frame in the buffer pool. Also,clockPointer is used as a counter to point the page frames in the buffer pool.
--> When we need to replace a page we check for the "clockPointer"s position. If that position's page's hitNo is not 1 (i.e. it wasn't the last page added), then it gets replaced with the new page.
--> If hitNo = 1, then we set it's hitNo = 0 and increment clockPointer. We repeat this process until we find a position to replace the page. We set hitNo = 0 to avoid entering into an infinite loop.

 
TEST CASES 2
===============
We have added additional test cases in source file test_assign2_2.c. The instructions to run these test cases is mentioned in this README file. These test cases test LFU and CLOCK page replacement strategies.


