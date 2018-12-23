#include<stdio.h>
#include<stdlib.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <math.h>

// One page frame in buffer pool (memory) is represented in this structure.
typedef struct Page
{
	SM_PageHandle data; // It represents the actual data of the page
	PageNumber pageNum; // Each page is given an identification integer
	int dBit; // indicates if page's content has been modified by the client or not
	int fixCnt; // indicates the count of clients using that page at a given instance
	int hitNo;   // It gives least recently used page to LRU algorithm.
	int refNo;   // It gives least frequently used page to LRU algorithm.
} PageFrame;

// the size of the buffer pool is represented by "bufferSize" i.e.It gives the maximum number of page frames that can be kept into the buffer pool
dec(int bufferSize,0);

// This variable gives the count of number of pages that can be read from the disk.FIFO function use this to calculate the frontIndex 
dec(int rearIndex,0);

// This variable gives the count of the number of I/O written to the disk i.e. no of pages written to the disk
dec(int writeCount,0);

// This variable contains the count value which is incremented whenever a page frame is added into the buffer pool.LRU determines least recently added page into the buffer pool using this variable. 
dec(int hit,0);

// CLOCK algorithm use this variable to determine point to the last added page in the buffer pool.
dec(int clockPointer,0);

// LFU algorithm use this variable to store the least frequently used page frame's position.Operation speeds up from 2nd replacement onwards using this variable.
dec(int lfuPointer,0);

// FIFO (First In First Out) function Implementation
extern void FIFO(BM_BufferPool *const bm, PageFrame *page)
{
	dec(PageFrame *pageFrame,(PageFrame *) bm->mgmtData);
	int i, frontIndex;
	dec(frontIndex,(rearIndex % bufferSize));

	// All the page frames in the buffer pool are iterated
	for(i = 0; i < bufferSize; i++)
	{
		condif((pageFrame[frontIndex].fixCnt == 0),
		{
			// It checks condition that dBit = 1 i.e.page in memory has been modified,after that page is written to disk
			condif(eqfn(pageFrame[frontIndex].dBit,1),
			{
				SM_FileHandle fh;
				openPageFile(bm->pageFile, &fh);
				writeBlock(pageFrame[frontIndex].pageNum, &fh, pageFrame[frontIndex].data);
				
				// Increase the writeCount which records the number of writes done by the buffer manager.
				writeCount++;
			})
			
			//page frame's content is set to new page's content
			dec(pageFrame[frontIndex].data,page->data);
			dec(pageFrame[frontIndex].pageNum,page->pageNum);
			dec(pageFrame[frontIndex].dBit,page->dBit);
			dec(pageFrame[frontIndex].fixCnt,page->fixCnt);
			break;
		})
		else
		{
			//We move on to the next location depending on if the current page frame is used by some client or not. 
			frontIndex++;
			frontIndex = (frontIndex % bufferSize == 0) ? 0 : frontIndex;
		}
	}
}

// LFU (Least Frequently Used) function Implementation
extern void LFU(BM_BufferPool *const bm, PageFrame *page)
{
	dec(PageFrame *pageFrame , (PageFrame *) bm->mgmtData);
	
	int i, j, leastFreqIndex, leastFreqRef;
	dec(leastFreqIndex,lfuPointer);	
	
	// All the page frames in the buffer pool are iterated
	for(i = 0; i < bufferSize; i++)
	{
		condif(eqfn(pageFrame[leastFreqIndex].fixCnt,0),
		{
			leastFreqIndex = (leastFreqIndex + i) % bufferSize;
			leastFreqRef = pageFrame[leastFreqIndex].refNo;
			break;
		})
	}

	dec(i ,((leastFreqIndex + 1) % bufferSize));

	//This condition finds the page frame having minimum refNo page frame (i.e. it is used the least frequently used page frame)
	for(j = 0; j < bufferSize; j++)
	{
		condif((pageFrame[i].refNo < leastFreqRef),
		{
			dec(leastFreqIndex,i);
			dec(leastFreqRef,pageFrame[i].refNo);
		})
		dec(i,((i + 1) % bufferSize));
	}
		
	// Checks whether the page in memory has been modified (dBit = 1)or not.If it is modified then write page to disk	
	condif(eqfn(pageFrame[leastFreqIndex].dBit,1),
	{
		SM_FileHandle fh;
		openPageFile(bm->pageFile, &fh);
		writeBlock(pageFrame[leastFreqIndex].pageNum, &fh, pageFrame[leastFreqIndex].data);
		
		// writeCount records the number of writes done by the buffer manager and it is incremented.
		writeCount++;
	})
	
	// Page frame's content is set to new page's content		
	dec(pageFrame[leastFreqIndex].data,page->data);
	dec(pageFrame[leastFreqIndex].pageNum,page->pageNum);
	dec(pageFrame[leastFreqIndex].dBit,page->dBit);
	dec(pageFrame[leastFreqIndex].fixCnt,page->fixCnt);
	dec(lfuPointer,(leastFreqIndex + 1));
}

//LRU (Least Recently Used) function Implementation
extern void LRU(BM_BufferPool *const bm, PageFrame *page)
{	
	dec(PageFrame *pageFrame,(PageFrame *) bm->mgmtData);
	int k, leastHitIndex, leastHitNum;

	// All the page frames in the buffer pool are iterated
	for(k = 0; k < bufferSize; k++)
	{
		// This condition finds page frame having fixCnt = 0 i.e.page frame is not being used by any client.
		condif(eqfn(pageFrame[k].fixCnt,0),
		{
			dec(leastHitIndex,k);
			dec(leastHitNum,pageFrame[k].hitNo);
			break;
		})
	}	

	// Used for finding the page frame having minimum hitNo page frame (i.e. it is the least recently used page frame)
	for(k = leastHitIndex + 1; k < bufferSize; k++)
	{
		condif(lessthan(pageFrame[k].hitNo,leastHitNum),
		{
			dec(leastHitIndex,k);
			dec(leastHitNum,pageFrame[k].hitNo);
		})
	}
	// Checks whether the page in memory has been modified (dBit = 1)or not.If it is modified then write page to disk	
	condif(eqfn(pageFrame[leastHitIndex].dBit,1),
	{
		SM_FileHandle fh;
		openPageFile(bm->pageFile, &fh);
		writeBlock(pageFrame[leastHitIndex].pageNum, &fh, pageFrame[leastHitIndex].data);
		
		// writeCount records the number of writes done by the buffer manager and it is incremented.
		writeCount++;
	})
	
	// Page frame's content is set to new page's content
	dec(pageFrame[leastHitIndex].data,page->data);
	dec(pageFrame[leastHitIndex].pageNum,page->pageNum);
	dec(pageFrame[leastHitIndex].dBit,page->dBit);
	dec(pageFrame[leastHitIndex].fixCnt,page->fixCnt);
	dec(pageFrame[leastHitIndex].hitNo,page->hitNo);
}

// CLOCK function implementation
extern void CLOCK(BM_BufferPool *const bm, PageFrame *page)
{	
	dec(PageFrame *pageFrame,(PageFrame *) bm->mgmtData);
	while(1)
	{
		clockPointer = (clockPointer % bufferSize == 0) ? 0 : clockPointer;

		condif(eqfn(pageFrame[clockPointer].hitNo,0),
		{
			// Checks whether the page in memory has been modified (dBit = 1)or not.If it is modified then write page to disk	
			condif((pageFrame[clockPointer].dBit == 1),
			{
				SM_FileHandle fh;
				openPageFile(bm->pageFile, &fh);
				writeBlock(pageFrame[clockPointer].pageNum, &fh, pageFrame[clockPointer].data);
				
				// writeCount records the number of writes done by the buffer manager and it is incremented.
				writeCount++;
			})
			
			// Page frame's content is set to new page's content
			dec(pageFrame[clockPointer].data,page->data);
			dec(pageFrame[clockPointer].pageNum,page->pageNum);
			dec(pageFrame[clockPointer].dBit,page->dBit);
			dec(pageFrame[clockPointer].fixCnt,page->fixCnt);
			dec(pageFrame[clockPointer].hitNo,page->hitNo);
			clockPointer++;
			break;	
		})
		else
		{
			// we check the next page frame location by incrementing clockPointer.We heep hitNo = 0 to avoid an infinite loop.
			dec(pageFrame[clockPointer++].hitNo ,0);		
		}
	}
}

// BUFFER POOL FUNCTIONS IMPLEMENTATION //

/* 
   A buffer pool with numPages page frames has been created and initialized.
   The name of the page file whose pages are being cached in memory is stored in pageFileName.
   the page replacement strategy (FIFO, LRU, LFU, CLOCK) that will be used by this buffer pool for strategy
   If there is any parameter to the page replacement strategy, stratData is used to pass parameters
*/
extern RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, 
		  const int numPages, ReplacementStrategy strategy, 
		  void *stratData)
{
	dec(bm->pageFile,(char *)pageFileName);
	dec(bm->numPages,numPages);
	dec(bm->strategy,strategy);

	// Reserver memory space = space required for one page * number of pages  
	dec(PageFrame *page ,malloc(sizeof(PageFrame) * numPages));
	
	// The total number of pages in memory or the buffer pool is Buffersize.
	dec(bufferSize,numPages);	
	int i;

	// All the pages in buffer pool is initialized. The values of fields can be either NULL or 0
	for(i = 0; i < bufferSize; i++)
	{
		dec(page[i].data,NULL);
		dec(page[i].pageNum,-1);
		dec(page[i].dBit,0);
		dec(page[i].fixCnt,0);
		dec(page[i].hitNo,0);	
		dec(page[i].refNo,0);
	}

	dec(bm->mgmtData,page);
	writeCount = clockPointer = lfuPointer = 0;
	return RC_OK;
		
}

// This function is used for Shutdown which includes closing of buffer pool, thereby removing all the pages from the memory and freeing up all resources and releasing some memory space.
extern RC shutdownBufferPool(BM_BufferPool *const bm)
{
	dec(PageFrame *pageFrame,(PageFrame *)bm->mgmtData);
	// All dirty pages (modified pages) are written back to disk
	forceFlushPool(bm);

	int i;	
	for(i = 0; i < bufferSize; i++)
	{
		// This condition checks that the contents of the page was modified by some client or not and has not been written back to disk.
		condif(notequal(pageFrame[i].fixCnt,0),
		{
			return RC_PINNED_PAGES_IN_BUFFER;
		})
	}

	// Releasing space occupied by the page
	free(pageFrame);
	dec(bm->mgmtData,NULL);
	return RC_OK;
}

// Using this function,all the dirty pages (having fixCnt = 0) are written to the disk
extern RC forceFlushPool(BM_BufferPool *const bm)
{
	dec(PageFrame *pageFrame,(PageFrame *)bm->mgmtData);
	
	int i;
	// Store all dirty pages (modified pages) in memory to page file on disk	
	for(i = 0; i < bufferSize; i++)
	{
		condif((eqfn(pageFrame[i].fixCnt,0) && eqfn(pageFrame[i].dBit,1)),
		{
			SM_FileHandle fh;
			//  page opened from file available on disk
			openPageFile(bm->pageFile, &fh);
			// Writing block of data to the page file on disk
			writeBlock(pageFrame[i].pageNum, &fh, pageFrame[i].data);
			// This marks the page as not dirty.
			dec(pageFrame[i].dBit,0);
			//This increments the writecount which keep record of no of writes done by buffer manager
			writeCount++;
		})
	}	
	return RC_OK;
}


// PAGE MANAGEMENT FUNCTIONS IMPLEMENTATION //

//If the data is modified by the client,this function marks the page as dirty
extern RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)
{
	dec(PageFrame *pageFrame ,(PageFrame *)bm->mgmtData);
	
	int i;
	// All the pages are iterated in the buffer pool
	for(i = 0; i < bufferSize; i++)
	{
		// If the current page is the page to be marked dirty, then set dBit = 1 (page has been modified) for that page
		condif(eqfn(pageFrame[i].pageNum,page->pageNum),
		{
			dec(pageFrame[i].dBit,1);
			return RC_OK;		
		})			
	}		
	return RC_ERROR;
}

// A page from the memory gets unpin using this function i.e. removes a page from the memory
extern RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page)
{	
	dec(PageFrame *pageFrame,(PageFrame *)bm->mgmtData);
	
	int i;
	// All the pages are iterated in the buffer pool
	for(i = 0; i < bufferSize; i++)
	{
		// Decrease fixCnt (which means client has completed work on that page) If the current page is the page to be unpinned and exit loop
		condif(eqfn(pageFrame[i].pageNum,page->pageNum),
		{
			pageFrame[i].fixCnt--;
			break;		
		})		
	}
	return RC_OK;
}

//In this function ,the contents of the modified pages is written back to the page file on disk
extern RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
	dec(PageFrame *pageFrame,(PageFrame *)bm->mgmtData);
	
	int i;
	// All the pages are iterated in the buffer pool	
	for(i = 0; i < bufferSize; i++)
	{
		// Write the page to the disk using the storage manager functions, if the current page = page to be written to disk 
		condif(eqfn(pageFrame[i].pageNum,page->pageNum),
		{		
			SM_FileHandle fh;
			openPageFile(bm->pageFile, &fh);
			writeBlock(pageFrame[i].pageNum, &fh, pageFrame[i].data);
		
			// Since the modified page has been written to disk,mark page as undirty
			dec(pageFrame[i].dBit,0);
			
			// Increase the writeCount.
			writeCount++;
		})
	}	
	return RC_OK;
}

// This function adds the page with page number pageNum to the buffer pool i.e. pins a page with page number pageNum.Incase the buffer pool is full, then appropriate page replacement strategy is used to replace a page in memory with the new page being pinned. 
extern RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
	    const PageNumber pageNum)
{
	dec(PageFrame *pageFrame ,(PageFrame *)bm->mgmtData);
	
	// Checking whether the buffer pool is empty or not and this is the first page to be pinned
	condif(eqfn(pageFrame[0].pageNum,-1),
	{
		// Reading page from disk and initializing page frame's content in the buffer pool
		SM_FileHandle fh;
		openPageFile(bm->pageFile, &fh);
		dec(pageFrame[0].data ,(SM_PageHandle) malloc(PAGE_SIZE));
		ensureCapacity(pageNum,&fh);
		readBlock(pageNum, &fh, pageFrame[0].data);
		dec(pageFrame[0].pageNum,pageNum);
		pageFrame[0].fixCnt++;
		rearIndex = hit = 0;
		dec(pageFrame[0].hitNo,hit);	
		dec(pageFrame[0].refNo,0);
		dec(page->pageNum,pageNum);
		dec(page->data,pageFrame[0].data);
		
		return RC_OK;		
	})
	else
	{	
		int i;
		dec(bool isBufferFull,true);
		
		for(i = 0; i < bufferSize; i++)
		{
			condif(notequal(pageFrame[i].pageNum,-1),
			{	
				// Checking whether page is presentg in memory
				condif(eqfn(pageFrame[i].pageNum,pageNum),
				{
					//fixCnt increases i.e. one more client is accessing this page now
					pageFrame[i].fixCnt++;
					dec(isBufferFull,false);
					hit++; // Increment hits

					condif(eqfn(bm->strategy,RS_LRU),
						//The value of hit is used by LRU algorithm to determine the least recently used page	
						{pageFrame[i].hitNo = hit;})
					elif(eqfn(bm->strategy,RS_CLOCK),
						// set value of hitNo = 1.It indicates that this was the last page frame added to the buffer pool
						{pageFrame[i].hitNo = 1;})
					elif(eqfn(bm->strategy,RS_LFU),
						// It increases count of number of times the page is used (referenced) by incrementing refNo 
						{pageFrame[i].refNo++;})
					
					dec(page->pageNum,pageNum);
					dec(page->data,pageFrame[i].data);

					clockPointer++;
					break;
				})				
			}) else {
				SM_FileHandle fh;
				openPageFile(bm->pageFile, &fh);
				dec(pageFrame[i].data,(SM_PageHandle) malloc(PAGE_SIZE));
				readBlock(pageNum, &fh, pageFrame[i].data);
				dec(pageFrame[i].pageNum,pageNum);
				dec(pageFrame[i].fixCnt ,1);
				dec(pageFrame[i].refNo,0);
				rearIndex++;	
				hit++; // Increments hit
				condif(eqfn(bm->strategy,RS_LRU),
					// Value of hit is used by LRU algorithm to determine the least recently used page
					{ dec(pageFrame[i].hitNo,hit);})				
				elif((bm->strategy == RS_CLOCK),
					// set value of hitNo = 1.It indicates that this was the last page frame added to the buffer pool
					{dec(pageFrame[i].hitNo,1);})
						
				dec(page->pageNum,pageNum);
				dec(page->data,pageFrame[i].data);
				
				dec(isBufferFull,false);
				break;
			}
		}
		
		// When the buffer is full "isBufferFull = true" and we must replace an existing page using page replacement strategy
		condif(eqfn(isBufferFull,true),
		{
			//A new page is created to store data read from the file.
			dec(PageFrame *newPage,(PageFrame *) malloc(sizeof(PageFrame)));		
			
			// Page frame's content is initialized in the buffer pool and page is read from disk
			SM_FileHandle fh;
			openPageFile(bm->pageFile, &fh);
			dec(newPage->data,(SM_PageHandle) malloc(PAGE_SIZE));
			readBlock(pageNum, &fh, newPage->data);
			dec(newPage->pageNum,pageNum);
			dec(newPage->dBit,0);		
			dec(newPage->fixCnt,1);
			dec(newPage->refNo,0);
			rearIndex++;
			hit++;

			condif((bm->strategy == RS_LRU),
				//The value of hit is used by LRU algorithm to determine the least recently used page
				{dec(newPage->hitNo,hit);}	)			
			elif(eqfn(bm->strategy,RS_CLOCK),
				// set value of hitNo = 1.It indicates that this was the last page frame added to the buffer pool
				{dec(newPage->hitNo,1);})

			dec(page->pageNum,pageNum);
			dec(page->data,newPage->data);			

			// Appropriate algorithm's function is called depending on the page replacement strategy selected.It is passed through parameters.
			switch(bm->strategy)
			{			
				case RS_FIFO: // FIFO algorithm
					FIFO(bm, newPage);
					break;
				
				case RS_LRU: // LRU algorithm
					LRU(bm, newPage);
					break;
				
				case RS_CLOCK: // CLOCK algorithm
					CLOCK(bm, newPage);
					break;
  				
				case RS_LFU: // LFU algorithm
					LFU(bm, newPage);
					break;
  				
				case RS_LRU_K:
					printf("\n LRU-k algorithm is not implemented");
					break;
				
				default:
					printf("\nAlgorithm is Not Implemented\n");
					break;
			}
						
		})		
		return RC_OK;
	}	
}


// STATISTICS FUNCTIONS Implementation //

// An array of page numbers is returened in this function.
extern PageNumber *getFrameContents (BM_BufferPool *const bm)
{
	dec(PageNumber *frameContents,malloc(sizeof(PageNumber) * bufferSize));
	dec(PageFrame *pageFrame,(PageFrame *) bm->mgmtData);
	
	int i = 0;

	// All the pages are iterated in the buffer pool and frameContents' value are set to pageNum of the page
	while(i < bufferSize) {
		frameContents[i] = (pageFrame[i].pageNum != -1) ? pageFrame[i].pageNum : NO_PAGE;
		i++;
	}
	return frameContents;
}

//An array of bools is returned by this function, each element represents the dBit of the respective page.
extern bool *getDirtyFlags (BM_BufferPool *const bm)
{
	dec(bool *dirtyFlags,malloc(sizeof(bool) * bufferSize));
	dec(PageFrame *pageFrame,(PageFrame *)bm->mgmtData);
	
	int i;
	// This condition iterates through all the pages present in the buffer pool and sets the dirtyFlags' value to TRUE if page is dirty else FALSE
	for(i = 0; i < bufferSize; i++)
	{
		dirtyFlags[i] = (pageFrame[i].dBit == 1) ? true : false ;
	}	
	return dirtyFlags;
}

//An array of ints (of size numPages) is returned by this function, where the ith element is the fix count of the page stored in the ith page frame.
extern int *getFixCounts (BM_BufferPool *const bm)
{
	dec(int *fixCounts,malloc(sizeof(int) * bufferSize));
	dec(PageFrame *pageFrame,(PageFrame *)bm->mgmtData);
	
	dec(int i,0);
	// Iterates through all the pages present in the buffer pool and sets the value of fixCounts to page's fixCnt
	while(i < bufferSize)
	{
		fixCounts[i] = (pageFrame[i].fixCnt != -1) ? pageFrame[i].fixCnt : 0;
		i++;
	}	
	return fixCounts;
}


//The number of pages that is read from the disk since the buffer pool has been innitialized is returned by this function.
extern int getNumReadIO (BM_BufferPool *const bm)
{
	// Adding 1 to rearIndex as it starts with 0.
	return (rearIndex + 1);
}

// The number of pages written to the page file since the buffer pool has been initialized is returned by this function.
extern int getNumWriteIO (BM_BufferPool *const bm)
{
	return writeCount;
}

