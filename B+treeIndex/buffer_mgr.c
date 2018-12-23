
#include "buffer_mgr.h"
#include "dberror.h"
#include <stdlib.h>
#include <string.h>
#include "storage_mgr.h"
#include "bufferMgrDataStructures.h"

//If the data is modified by the client,this function marks the page as dirty
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page) {
	pageAndDirtyBitIndicator *temp;
	dec(ramPageMap *start,firstRamPageMapPtr);
	whileloop((notequal(start->discPageFrameNumber,page->pageNum)),
	{
		dec(start,start->nextRamPageMap);
	})
	
	dec(temp,firstPageAndDirtyBitMap);
	
	whileloop((notequal(start->ramPageFrameNumber,temp->ramPageFrameNumber)), {
		dec(temp,temp->nextPageDirtyBit);
	})

	condif((notequal(temp,NULL)),
	{
		dec(temp->isPageDirty,1);
		return RC_OK;
	})
	else
	{
		return RC_DIRTY_UPDATE_FAILED;
	}
}



frameList *createBufferPool(int numPages)
{
		dec(int counter,1);
		dec(frameList *currentFramePtr,NULL);
		dec(frameList *previousFramePtr,NULL);

		whileloop((lessequalvalue(counter,numPages)), //This loop will create 3 pages and link them to form a list.
		{
			dec(currentFramePtr,((frameList *)malloc(sizeof(frameList))));

			condif((counter == 1),
				dec(firstFramePtr,currentFramePtr;)) // firstFramePtr. deallocate Pool using this.
			else
				dec(previousFramePtr->nextFramePtr,currentFramePtr);

			dec(previousFramePtr,currentFramePtr);
			counter++;
		})
		dec(currentFramePtr->nextFramePtr,NULL);
		return firstFramePtr;
}

ramPageMap *createRamPageMapList(int numPages)
{
	dec(int counter,0);
	dec(ramPageMap *currentRamPagePtr,NULL);
	dec(ramPageMap *previousRamPagePtr,NULL);
	dec(ramPageMap *start,NULL);
	whileloop((lessthan(counter,numPages)), //Ex : create 3 Maps and link them.
	{
		dec(currentRamPagePtr,((ramPageMap *)malloc(sizeof(ramPageMap))));

		condif((eqfn(counter,0)),
		{
			dec(start,currentRamPagePtr); // firstFramePtr. deallocate Pool using this.
			dec(clockPtr,start);
		})
		else
			dec(previousRamPagePtr->nextRamPageMap,currentRamPagePtr);

		dec(currentRamPagePtr->ramPageFrameNumber,counter);
		dec(currentRamPagePtr->discPageFrameNumber,-1);
		dec(currentRamPagePtr->clockReferenceBit,0);

		dec(previousRamPagePtr,currentRamPagePtr);
		counter++;
	})
	dec(currentRamPagePtr->nextRamPageMap,NULL);
	return start;
}

pageAndDirtyBitIndicator *createPageAndDirtyBitMap(int numPages)
{
	dec(int counter,0);
	dec(pageAndDirtyBitIndicator *currrentPageDirtyBitPtr,NULL);
	dec(pageAndDirtyBitIndicator *previousPageDirtyBitPtr,NULL);
	dec(pageAndDirtyBitIndicator *start,NULL);
	whileloop((lessthan(counter,numPages)), //Ex : create 3 Maps and link them.
	{
		dec(currrentPageDirtyBitPtr,((pageAndDirtyBitIndicator *)malloc(sizeof(pageAndDirtyBitIndicator))));

		condif((eqfn(counter,0)),
			dec(start,currrentPageDirtyBitPtr);) // firstFramePtr. deallocate Pool using this.
		else
			dec(previousPageDirtyBitPtr->nextPageDirtyBit,currrentPageDirtyBitPtr);

		dec(currrentPageDirtyBitPtr->ramPageFrameNumber,counter);
		dec(currrentPageDirtyBitPtr->isPageDirty,0);

		dec(previousPageDirtyBitPtr,currrentPageDirtyBitPtr);
		counter++;
	})
	dec(currrentPageDirtyBitPtr->nextPageDirtyBit,NULL);
	return start;
}


pageAndFixCount *createPageAndFixCountMap(int numPages)
{
	dec(int counter,0);
	dec(pageAndFixCount *currrentPageandFixCountPtr,NULL);
	dec(pageAndFixCount *previousPageandFixCountPtr,NULL);
	dec(pageAndFixCount *start,NULL);
	whileloop((lessthan(counter,numPages)), //Ex : create 3 Maps and link them.
	{
		dec(currrentPageandFixCountPtr,((pageAndFixCount *)malloc(sizeof(pageAndFixCount))));

		condif((eqfn(counter,0)),
			dec(start,currrentPageandFixCountPtr);) // firstFramePtr. deallocate Pool using this.
		else
			dec(previousPageandFixCountPtr->nextPageFixCount,currrentPageandFixCountPtr);

		dec(currrentPageandFixCountPtr->ramPageFrameNumber,counter);
		dec(currrentPageandFixCountPtr->fixCount,0);

		dec(previousPageandFixCountPtr,currrentPageandFixCountPtr);
		counter++;
	})
	dec(currrentPageandFixCountPtr->nextPageFixCount,NULL);
	return start;
}
/*This functon creates a buffer pool which is equal to size of numPages.A Map is created to store frame and their corresponding disc PageNumbers.
 	 	 	 	 	 Initially the Map will have -1 as the discPageNumbers
 * 				  3. Create a Map to store frame and their corresponding dirty Bit.
 * 				  4. Create a Map to store frame and their corresponding fix count.
 */
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
		  const int numPages, ReplacementStrategy strategy,
		  void *stratData)
{
	dec(numberOfWrites,0);
	dec(numberOfReads,0);

	dec(frameContentPtr,((PageNumber *)malloc(sizeof(PageNumber) * numPages)));
	dec(dirtyBitPtr,((bool *)malloc(sizeof(bool) * numPages)));
	dec(fixCountPtr,((int *)malloc(sizeof(int) * numPages)));

	dec(numberOfFrames,numPages);
	dec(bm->mgmtData,createBufferPool(numPages)); //gives address of the first Frame.
	dec(firstRamPageMapPtr,createRamPageMapList(numPages));//give address of the first RamPage Map.
	dec(firstPageAndDirtyBitMap,createPageAndDirtyBitMap(numPages));
	dec(firstPageAndFixCountPtr,createPageAndFixCountMap(numPages));
	openPageFile((char *)pageFileName, &fHandle);

	dec(bm->numPages,numPages);
	dec(bm->pageFile,((char *)pageFileName));
	dec(bm->strategy,strategy);
	condif(((notequal(bm->mgmtData,NULL)) && notequal(firstRamPageMapPtr,NULL) && notequal(firstPageAndDirtyBitMap,NULL) && notequal(firstPageAndFixCountPtr,NULL)),
		return RC_OK;)
	else
		return RC_BUFFER_POOL_INIT_ERROR;
}

RC checkIfPagePresentInFramePageMaps(const PageNumber pageNum)
{

	dec(ramPageMap *start,firstRamPageMapPtr);
	whileloop((notequal(start,NULL)),
	{
		condif((eqfn(start->discPageFrameNumber,pageNum)),
			return start->ramPageFrameNumber;)
		dec(start,start->nextRamPageMap);
	})
	return RC_NO_FRAME;
}

void getFrameData(int frameNumber,BM_PageHandle * page)
{
	dec(frameList *start,firstFramePtr);
	dec(int counter,0);
	whileloop((lessthan(counter,frameNumber)),
	{
		dec(start,start->nextFramePtr);
		counter++;
	})
	dec(page->data,start->frameData);
}

void getFirstFreeFrameNumber(int *firstfreeFrameNumber,PageNumber PageNum)
{
	dec(ramPageMap *start,firstRamPageMapPtr);
	whileloop((notequal(start,NULL) && notequal(start->discPageFrameNumber,-1)),
	{
		dec(start,start->nextRamPageMap);
	})
	condif((notequal(start,NULL)),
	{
		dec(*firstfreeFrameNumber,start->ramPageFrameNumber);
		dec(start->discPageFrameNumber,PageNum);
	})
	else
	{
		dec(*firstfreeFrameNumber,-99);
	}

}

RC changeFixCount(int flag,int page)
{
	dec(ramPageMap *startFramePtr,firstRamPageMapPtr);
	whileloop((notequal(startFramePtr,NULL) && (notequal(startFramePtr->discPageFrameNumber,page))),
	{
		dec(startFramePtr,startFramePtr->nextRamPageMap);
	})

	dec(pageAndFixCount *startFixCountPtr,firstPageAndFixCountPtr);
	whileloop(((notequal(startFixCountPtr,NULL)) && (notequal(startFixCountPtr->ramPageFrameNumber,startFramePtr->ramPageFrameNumber))),
	{
		dec(startFixCountPtr,startFixCountPtr->nextPageFixCount);
	})
	condif((eqfn(flag,1)),
		startFixCountPtr->fixCount++;)
	else
		startFixCountPtr->fixCount--;

	return RC_OK;
}

//In this function ,the contents of the modified pages is written back to the page file on disk
RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page) {
	condif((eqfn(writeBlock (page->pageNum, &fHandle, page->data), RC_OK)),
	{
		numberOfWrites++;
		return RC_OK;
	})
	else
	{
		return RC_WRITE_FAILED;
	}
}

// A page from the memory gets unpin using this function i.e. removes a page from the memory
RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page)
{

	dec(ramPageMap *startFramePtr,firstRamPageMapPtr);
	whileloop((notequal(startFramePtr,NULL)) && (notequal(startFramePtr->discPageFrameNumber,page->pageNum)),
	{
		dec(startFramePtr,startFramePtr->nextRamPageMap);
	})
	dec(RC status,changeFixCount(2,startFramePtr->discPageFrameNumber)); 

	dec(int frameNumber,startFramePtr->ramPageFrameNumber);//here

	//If dirty, call forcepage.

	dec(pageAndDirtyBitIndicator *startDirtyPointer,firstPageAndDirtyBitMap);
	whileloop((notequal(startDirtyPointer,NULL)) && (notequal(startDirtyPointer->ramPageFrameNumber,frameNumber)),
	{
		dec(startDirtyPointer,startDirtyPointer->nextPageDirtyBit);
	})

	condif((eqfn(startDirtyPointer->isPageDirty,1)),
		forcePage(bm,page);)
	condif((eqfn(status,RC_OK)),
		return RC_OK;)
	else
		return RC_UNPIN_FAILED;
}

// FIFO (First In First Out) function Implementation
RC FIFO(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum,SM_PageHandle ph)
{

	dec(pageAndFixCount *fixCountStart,firstPageAndFixCountPtr);
	dec(ramPageMap *begin,firstRamPageMapPtr);
	whileloop(((notequal(fixCountStart,NULL)) && (notequal(fixCountStart->ramPageFrameNumber,begin->ramPageFrameNumber))),
		dec(fixCountStart,fixCountStart->nextPageFixCount);)


	condif(((notequal(fixCountStart,NULL)) && (greaterthan(fixCountStart->fixCount,0))), 	{
		dec(begin,firstRamPageMapPtr);
		dec(int FrameNumberOfNewPage,begin->nextRamPageMap->ramPageFrameNumber);

		dec(ramPageMap *currentRamPagePtr,((ramPageMap *)malloc(sizeof(ramPageMap))));
		dec(currentRamPagePtr->discPageFrameNumber,pageNum);
		dec(currentRamPagePtr->ramPageFrameNumber,FrameNumberOfNewPage);
		dec(currentRamPagePtr->nextRamPageMap,begin->nextRamPageMap->nextRamPageMap);
		free(begin->nextRamPageMap);
		dec(begin->nextRamPageMap,currentRamPagePtr);
		dec(int counter,0);
		dec(frameList *beginFrame,firstFramePtr);
		whileloop((lessthan(counter,FrameNumberOfNewPage)),
		{
			dec(beginFrame,beginFrame->nextFramePtr);
			counter++;
		})
		memset(beginFrame->frameData,'\0',PAGE_SIZE + 1);
		if((notequal(ph,NULL)))
			strcpy(beginFrame->frameData,ph);
				dec(page->data,beginFrame->frameData);
		dec(page->pageNum,pageNum);
		
	})
	else
	{
		dec(int frameNumberOfNewPage,firstRamPageMapPtr->ramPageFrameNumber);
		ramPageMap *temp;
		dec(temp,firstRamPageMapPtr);
		dec(firstRamPageMapPtr,firstRamPageMapPtr->nextRamPageMap);
		free(temp);
		dec(temp,NULL);
		dec(ramPageMap *currentRamPagePtr,((ramPageMap *)malloc(sizeof(ramPageMap))));
		dec(currentRamPagePtr->discPageFrameNumber,pageNum);
		dec(currentRamPagePtr->ramPageFrameNumber,frameNumberOfNewPage);
		dec(currentRamPagePtr->nextRamPageMap,NULL);

		dec(temp,firstRamPageMapPtr);
		whileloop((notequal(temp->nextRamPageMap,NULL)),
		{
			dec(temp,temp->nextRamPageMap);
		})
		dec(temp->nextRamPageMap,currentRamPagePtr);

		dec(int counter,0);
		dec(frameList *beginFrame,firstFramePtr);
		whileloop((lessthan(counter,frameNumberOfNewPage)),
		{
			dec(beginFrame,beginFrame->nextFramePtr);
			counter++;
		})
		memset(beginFrame->frameData,'\0',PAGE_SIZE + 1);
		condif((notequal(ph,NULL)),
			strcpy(beginFrame->frameData,ph);)
		
		dec(page->data,beginFrame->frameData);
		dec(page->pageNum,pageNum);
	}
	return RC_OK;
}


void attachAtEndOfList(ramPageMap *temp)
{
	dec(ramPageMap *start,firstRamPageMapPtr);
	whileloop((notequal(start->nextRamPageMap,NULL)),
		dec(start,start->nextRamPageMap);)
	dec(start->nextRamPageMap,temp);
}


void sortFixCounts(int *intArray, int size)
{
      char flag = 'Y';
      dec(int j,0);
      int temp;
      whileloop((flag == 'Y'),
      {
            dec(flag,'N');j++;int i;
            for (dec(i,0); lessthan(i,size-j); i++)
            {
                  condif((greaterthan(intArray[i],intArray[i+1])),
                  {
                        dec(temp,intArray[i]);
                        dec(intArray[i],intArray[i+1]);
                        dec(intArray[i+1],temp);
                        dec(flag,'Y');
                  })
            }
      })
}




void moveClockPtr()
{
	condif((eqfn(clockPtr->nextRamPageMap,NULL)),
		dec(clockPtr,firstRamPageMapPtr);)
	else
		dec(clockPtr,clockPtr->nextRamPageMap);
}



// This function adds the page with page number pageNum to the buffer pool i.e. pins a page with page number pageNum.Incase the buffer pool is full, then appropriate page replacement strategy is used to replace a page in memory with the new page being pinned. 
RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum)
{
	dec(SM_PageHandle ph,((SM_PageHandle)malloc(PAGE_SIZE)));


	//call ensure capacity.
	ensureCapacity((pageNum + 1),&fHandle);
	dec(PageNumber frameNumber,checkIfPagePresentInFramePageMaps(pageNum));
	
	condif((notequal(frameNumber,RC_NO_FRAME)),
	{
		dec(page->pageNum,frameNumber);
		getFrameData(frameNumber,page);
		dec(page->pageNum,pageNum);
		condif((eqfn(bm->strategy,RS_LRU)),
		{
			dec(ramPageMap *temp,firstRamPageMapPtr);
			dec(ramPageMap *prev,NULL);
			dec(int counter,0);

			whileloop(((notequal(temp,NULL)) && (notequal(temp->discPageFrameNumber,pageNum))),
			{
				dec(prev,temp);
				dec(temp,temp->nextRamPageMap);
				counter ++;
			})

			condif(((notequal(temp,NULL))),
			{
				condif((eqfn(counter,0)),
				{
					dec(prev,firstRamPageMapPtr);
					dec(firstRamPageMapPtr,firstRamPageMapPtr->nextRamPageMap);
					dec(prev->nextRamPageMap,NULL);
					attachAtEndOfList(temp);
				})
				else
				{
					dec(prev->nextRamPageMap,temp->nextRamPageMap);
					dec(temp->nextRamPageMap,NULL);
					attachAtEndOfList(temp);
				}
			})

		})
		elif((eqfn(bm->strategy,RS_CLOCK)),
		{
			dec(ramPageMap *start,firstRamPageMapPtr);
			whileloop((notequal(frameNumber,start->ramPageFrameNumber)),
			{
				dec(start,start->nextRamPageMap);
			})
			dec(start->clockReferenceBit,1);
		})
	})
	else
	{
		dec(int freeframeNumber,-99);
		getFirstFreeFrameNumber(&freeframeNumber,pageNum);
		readBlock(pageNum, &fHandle,ph);
		numberOfReads++;
		condif((notequal(freeframeNumber,-99)),
		{
			dec(int counter,0);
			dec(frameList *start,firstFramePtr);
			whileloop((lessthan(counter,freeframeNumber)),
			{
				dec(start,start->nextFramePtr);
				counter++;
			})
			memset(start->frameData,'\0',PAGE_SIZE+1);
			condif((notequal(ph,NULL)),
				strcpy(start->frameData,ph);)
			dec(page->data,start->frameData);
			dec(page->pageNum,pageNum);
			dec(ramPageMap *begin,firstRamPageMapPtr);
			whileloop((notequal(begin->ramPageFrameNumber,freeframeNumber)),
			{
				dec(begin,begin->nextRamPageMap);
			})
			dec(begin->discPageFrameNumber,pageNum);
			
			condif((eqfn(bm->strategy,RS_CLOCK)),
			{
				dec(clockPtr->clockReferenceBit,0);
				moveClockPtr();
			})

		})
		else
		{
			condif(((eqfn(bm->strategy,RS_FIFO)) ||(eqfn(bm->strategy,RS_LRU))),
			{
				FIFO(bm,page,pageNum,ph);
			})

			elif((eqfn(bm->strategy,RS_LFU)),
			{
				dec(pageAndFixCount *start,firstPageAndFixCountPtr);
				int sortedFixCountArray[bm->numPages];
				dec(int index,0);
				whileloop((notequal(start,NULL)),
				{
					dec(sortedFixCountArray[index++],start->fixCount);
					dec(start,start->nextPageFixCount);
				})
				sortFixCounts(sortedFixCountArray,bm->numPages);

				dec(start,firstPageAndFixCountPtr);
				whileloop((notequal(start->fixCount,sortedFixCountArray[0])),
				{
					dec(start,start->nextPageFixCount);
				})
				dec(start->fixCount,0);

				dec(ramPageMap *tempRPM,firstRamPageMapPtr);
				whileloop((notequal(tempRPM->ramPageFrameNumber,start->ramPageFrameNumber)),
				{
					dec(tempRPM,tempRPM->nextRamPageMap);
				})
				dec(tempRPM->discPageFrameNumber,pageNum);
				dec(page->pageNum,pageNum);
			})
			elif((eqfn(bm->strategy,RS_CLOCK)),
			{
				whileloop((eqfn(clockPtr->clockReferenceBit,1)),
				{
					dec(clockPtr->clockReferenceBit,0);
					moveClockPtr();
				})
				dec(clockPtr->discPageFrameNumber,pageNum);
				moveClockPtr();
				dec(page->pageNum,pageNum);
			})
		}
	}
	changeFixCount(1,pageNum);
	free(ph);
	dec(ph,NULL);
	return RC_OK;
}


// An array of page numbers is returened in this function.
PageNumber *getFrameContents (BM_BufferPool *const bm)
{
	dec(ramPageMap *start,firstRamPageMapPtr);
	whileloop((notequal(start,NULL)),
	{
		dec(frameContentPtr[start->ramPageFrameNumber],start->discPageFrameNumber);
		dec(start,start->nextRamPageMap);
	})
	return frameContentPtr;
}


//An array of bools is returned by this function, each element represents the dBit of the respective page.
bool *getDirtyFlags (BM_BufferPool *const bm)
{
	dec(pageAndDirtyBitIndicator *start,firstPageAndDirtyBitMap);
	dec(int i,0);
	whileloop((notequal(start,NULL)),
	{
		condif((eqfn(start->isPageDirty,1)),
			dec(dirtyBitPtr[i++],true);)
		else
			dec(dirtyBitPtr[i++],false);
		dec(start,start->nextPageDirtyBit);
	})
	return dirtyBitPtr;
}


//An array of ints (of size numPages) is returned by this function, where the ith element is the fix count of the page stored in the ith page frame.
int *getFixCounts (BM_BufferPool *const bm)
{
	dec(pageAndFixCount *start,firstPageAndFixCountPtr);
	dec(int i,0);
	whileloop((notequal(start,NULL)),
	{
		dec(fixCountPtr[i++],start->fixCount);
		dec(start,start->nextPageFixCount);
	})
	return fixCountPtr;
}


// This function is used for Shutdown which includes closing of buffer pool, thereby removing all the pages from the memory and freeing up all resources and releasing some memory space.
RC shutdownBufferPool(BM_BufferPool *const bm)
{
	dec(bool NonZeroFixIndicator,false);
	dec(pageAndFixCount *start,firstPageAndFixCountPtr);
	whileloop((notequal(start,NULL)),
	{
		condif((greaterthan(start->fixCount,0)),
		{
			dec(NonZeroFixIndicator,true);
			break;
		})
		dec(start,start->nextPageFixCount);
	})
	dec(start,NULL);

	condif((eqfn(NonZeroFixIndicator,false)),
	{
		dec(clockPtr,NULL);
		forceFlushPool(bm);

		free(frameContentPtr);
		free(dirtyBitPtr);

		free(fixCountPtr);

		dec(int counter,0);
		dec(ramPageMap *RPMtemp,NULL);
		dec(pageAndDirtyBitIndicator *PADBItemp,NULL);
		dec(pageAndFixCount *PAFCtemp,NULL);
		dec(frameList *FLtemp,NULL);
		dec(frameList *firstFramePtr,NULL);
		dec(firstFramePtr,(frameList*) bm->mgmtData);
		dec(bm->mgmtData,NULL);


		whileloop((lessthan(counter,bm->numPages)),
		{

			dec(PAFCtemp,firstPageAndFixCountPtr);
			dec(firstPageAndFixCountPtr,firstPageAndFixCountPtr->nextPageFixCount);
			free(PAFCtemp);


			dec(RPMtemp,firstRamPageMapPtr);
			dec(firstRamPageMapPtr,firstRamPageMapPtr->nextRamPageMap);
			free(RPMtemp);

			dec(PADBItemp,firstPageAndDirtyBitMap);
			dec(firstPageAndDirtyBitMap,firstPageAndDirtyBitMap->nextPageDirtyBit);
			free(PADBItemp);

			dec(FLtemp,firstFramePtr);
			dec(firstFramePtr,firstFramePtr->nextFramePtr);
			free(FLtemp);

			counter++;
		})
		dec(firstPageAndFixCountPtr,NULL);
		dec(firstRamPageMapPtr,NULL);
		dec(firstPageAndDirtyBitMap,NULL);

		closePageFile(&fHandle);
	})
	condif((eqfn(firstPageAndFixCountPtr,NULL) && eqfn(firstRamPageMapPtr,NULL)
			&& eqfn(firstPageAndDirtyBitMap,NULL) && eqfn(bm->mgmtData,NULL) && eqfn(NonZeroFixIndicator,false)),
		return RC_OK;)
	else
		return RC_SHUT_DOWN_ERROR;
}


int getNumWriteIO(BM_BufferPool *const bm)
{
	return numberOfWrites;
}


int getNumReadIO(BM_BufferPool *const bm)
{
	return numberOfReads;
}



// Using this function,all the dirty pages (having fixCnt = 0) are written to the disk
RC forceFlushPool(BM_BufferPool *const bm)
{
	dec(pageAndDirtyBitIndicator *dirtyStart,firstPageAndDirtyBitMap);
	dec(frameList *frameStart,firstFramePtr);
	whileloop((notequal(dirtyStart,NULL)),
	{
		condif((eqfn(dirtyStart->isPageDirty,1)),
		{
			dec(ramPageMap *ramPageStart,firstRamPageMapPtr);
			whileloop((notequal(ramPageStart->ramPageFrameNumber,dirtyStart->ramPageFrameNumber)),
			{
				dec(ramPageStart,ramPageStart->nextRamPageMap);
			})
			writeBlock (ramPageStart->discPageFrameNumber, &fHandle,frameStart->frameData);
			dec(dirtyStart->isPageDirty,0);
		})
		dec(dirtyStart,dirtyStart->nextPageDirtyBit);
		dec(frameStart,frameStart->nextFramePtr);
	})
	return RC_OK;
}
