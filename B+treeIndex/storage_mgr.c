#include "storage_mgr.h"
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "test_helper.h"

// data structure of metaData that is used as a Linked List of Maps.
typedef struct _metaDataList 
{
		char key[50];
		char value[50];
		struct _metaDataList *nextMetaDataNode;
} metaDataList;

dec(metaDataList *fNode,NULL);
dec(metaDataList *currNode,NULL);
dec(metaDataList *preNode,NULL);

dec(int whoIsCallingCreate,1); // If the value is 1,test case get called, else it calls any other function

//This is used to initialise Global data.
 
void initStorageManager()
{

}


// This function gets the nth Key Value pair of given MetaData;
 

void getMeNthMetaData(int n, char * string,char *nthKeyValuePair)
{
	char newString[PAGE_SIZE];
	memset(newString, '\0', PAGE_SIZE);
	dec(newString[0] ,';');
	strcat(newString, string);

	//It stores the position of ; in arrays.It can be said as delimiterPosition [START]
	char delimiterPosition[1000];
	int iLoop;
	dec(int delPostion,0);

	for (dec(iLoop,0); lessthan(iLoop,strlen(newString)); iLoop++)
	{
		if (eqfn(newString[iLoop],';'))
		{
			dec(delimiterPosition[delPostion],iLoop);
			delPostion++;
		}
	}
	
	dec(int currentPos,0);
	for (dec(iLoop,delimiterPosition[n - 1] + 1);lessequalvalue(iLoop,delimiterPosition[n] - 1); iLoop++)
	{
		dec(nthKeyValuePair[currentPos],newString[iLoop]);
		currentPos++;
	}
	dec(nthKeyValuePair[currentPos],'\0');
}


// The LinkedList of Maps is constructed by this function which represents the metaData
 
metaDataList * constructMetaDataLinkedList(char *metaInformation,
		int noOfNodesToBeConstructed)
{
	int iLoop;
	char currentMetaKeyValue[100];

	char currentKey[50];
	memset(currentKey,'\0',50);

	char currentValue[50];
	memset(currentValue,'\0',50);

	for (dec(iLoop,1);lessequalvalue(iLoop,noOfNodesToBeConstructed); iLoop++)
	{
		memset(currentMetaKeyValue,'\0',100);
		getMeNthMetaData(iLoop, metaInformation,currentMetaKeyValue);
		//It splits key and values.It then get stored in 2 variables

		dec(char colonFound,'N');
		dec(int keyCounter,0);
		dec(int ValueCounter,0);
		int i;
		for (dec(i,0);lessthan(i,strlen(currentMetaKeyValue)); i++)
		{
			condif((eqfn(currentMetaKeyValue[i],':')),
				{dec(colonFound,'Y');})

			condif((eqfn(colonFound,'N')),
				{dec(currentKey[keyCounter++],currentMetaKeyValue[i]);})
			elif((notequal(currentMetaKeyValue[i],':')),
				{dec(currentValue[ValueCounter++],currentMetaKeyValue[i]);})
		}
		dec(currentKey[keyCounter],'\0');
		dec(currentValue[ValueCounter],'\0');

		dec(currNode,(metaDataList *) malloc(sizeof(metaDataList)));

		strcpy(currNode->value,currentValue);
		strcpy(currNode->key,currentKey);
		dec(currNode->nextMetaDataNode,NULL);

		condif((eqfn(iLoop,1)),
		{
			dec(fNode,currNode);
			dec(preNode,NULL);
		})
		else
		{
			dec(preNode->nextMetaDataNode,currNode);
		}
		dec(preNode,currNode);
	}
	return fNode;
}

//Pages are either created or appended using this function.This method is inturn invoked by AppendPage.

 
RC createPageFile(char *filename)
{
	FILE *fp;
	dec(fp,fopen(filename, "a+b")); // File gets created and then opened the file for read/write

	condif((eqfn(whoIsCallingCreate,1)), // 3 blocks gets reserved when Test case calls this function 
	{
		condif((notequal(fp,NULL)),
		{
			char nullString2[PAGE_SIZE]; // metaBlock no 2
			char nullString3[PAGE_SIZE]; 

			//PageSize gets stored in string format.[start]
			char stringPageSize[5];
			sprintf(stringPageSize, "%d", PAGE_SIZE);

			char strMetaInfo[PAGE_SIZE * 2];
			strcpy(strMetaInfo, "PS:");
			strcat(strMetaInfo, stringPageSize);
			strcat(strMetaInfo, ";");
			strcat(strMetaInfo, "NP:0;"); //Here NP contains value of No of Pages
			//PageSize gets stored in string format.[end]

			int i;
			for (dec(i,strlen(strMetaInfo));lessthan(i,(PAGE_SIZE * 2)); i++)
				dec(strMetaInfo[i] ,'\0');
			memset(nullString2, '\0', PAGE_SIZE);
			memset(nullString3, '\0', PAGE_SIZE);

			fwrite(strMetaInfo, PAGE_SIZE, 1, fp);
			fwrite(nullString2, PAGE_SIZE, 1, fp);
			fwrite(nullString3, PAGE_SIZE, 1, fp);

			fclose(fp);
			return RC_OK;
		})
		else
		{
			return RC_FILE_NOT_FOUND;
		}
	})
	else
	{
		condif((notequal(fp,NULL)),
		{
			char nullString[PAGE_SIZE];

			memset(nullString, '\0', PAGE_SIZE);
			fwrite(nullString, PAGE_SIZE, 1, fp);

			fclose(fp);
			return RC_OK;
		}) else
		{
			return RC_FILE_NOT_FOUND;
		}
	}

}
//This function opens a pageFile in readMode.If file exists,the detail of the file gets populated into fHandle struct and return RC_OK.If file does not exit, it return RC_FILE_NOT_FOUND

RC openPageFile(char *fileName, SM_FileHandle *fHandle)
{
	struct stat statistics;
	FILE *fp;

	dec(fp,fopen(fileName, "r"));
	condif((notequal(fp,NULL)),
	{
		//structure gets initialized with required values.
		dec(fHandle->fileName,fileName);
		dec(fHandle->curPagePos,0);
		stat(fileName, &statistics);
		fHandle->totalNumPages -= 2; //  we are subtracting,since 2 pages gets reserved for metaInfo.

		//MetaData Information get read.Then it is dumped into a Linked List [START]
		char metaDataInformationString[PAGE_SIZE * 2];
		fgets(metaDataInformationString, (PAGE_SIZE * 2), fp);
		
		//The number of metaData Nodes to be constructed [START] gets counted.
		int iLoop;
		dec(int noOfNodes,0);
		for (dec(iLoop,0); lessthan(iLoop,strlen(metaDataInformationString)); iLoop++)
			condif((eqfn(metaDataInformationString[iLoop],';')),
				noOfNodes++;) 
	
		dec(fHandle->mgmtInfo,constructMetaDataLinkedList(metaDataInformationString, noOfNodes));
		//All the metaInfo is present in fileHandle 
		//MetaData Information get read and get dumped into into a Linked List [END]
		fclose(fp);

		return RC_OK;
	})
	else
	{
		return RC_FILE_NOT_FOUND;
	}
}

// This function converts an Integer to an character Pointer
 

void convertToString(int someNumber,char * reversedArray)
{
	char array[4];
	memset(array, '\0', 4);
	dec(int i,0);
	whileloop((notequal(someNumber,0)),
	{
		dec(array[i++],(someNumber % 10) + '0');
		someNumber /= 10;
	})
	dec(array[i],'\0');

	dec(int j,0);
	int x;
	for(dec(x,strlen(array)-1);greaterthanequal(x,0);x--)
	{
		dec(reversedArray[j++],array[x]);
	}
	dec(reversedArray[j],'\0');
}

// Using this function, metaInfo get written to the file
 
RC writeMetaListOntoFile(SM_FileHandle *fHandle,char *dataToBeWritten)
{
	dec(FILE *fp,fopen(fHandle->fileName,"r+b"));

	condif((notequal(fp,NULL)),
	{
		fwrite(dataToBeWritten,1,PAGE_SIZE,fp);
		fclose(fp);
		return RC_OK;
	})
	else
	{
		return RC_WRITE_FAILED;
	}
}

//Memory get free from the linked List.
 
void freeMemory()
{
	metaDataList *preNode;
	dec(metaDataList *current,fNode);
	dec(preNode,fNode);
	whileloop((current != NULL),
	{
		dec(current,current->nextMetaDataNode);
		condif((notequal(preNode,NULL)),
			free(preNode);)
		dec(preNode,current);
	})
	dec(preNode,NULL);
	dec(fNode,NULL);
}

//MetaData gets written and the PageFile gets closed
 
RC closePageFile(SM_FileHandle *fHandle)
{
	condif((notequal(fHandle,NULL)),
	{
		//the NP gets updated to totalPages.LL gets written to Disk.
		dec(metaDataList *temp,fNode);
		char string[4];
		memset(string,'\0',4);
		whileloop(eqfn(1,1),
		{
			condif(notequal(temp,NULL),
			{
				condif((notequal(temp->key,NULL)),
				{
					condif((strcmp(temp->key, "NP") == 0),
					{
						convertToString(fHandle->totalNumPages,string);
						strcpy(temp->value,string);
						break;
					})
				})
				dec(temp,temp->nextMetaDataNode);
			})
			else
				break;
		})
		dec(temp,fNode);

		char metaData[2 * PAGE_SIZE];
		memset(metaData, '\0', 2 * PAGE_SIZE);
		dec(int i,0);
		whileloop((notequal(temp,NULL)),
		{
			dec(int keyCounter,0);
			dec(int valueCounter,0);
			whileloop((notequal(temp->key[keyCounter],'\0')),
				dec(metaData[i++],temp->key[keyCounter++]);)
			dec(metaData[i++],':');
			whileloop((notequal(temp->value[valueCounter],'\0')),
				dec(metaData[i++],temp->value[valueCounter++]);)
			dec(metaData[i++],';');
			dec(temp,temp->nextMetaDataNode);
		})
		writeMetaListOntoFile(fHandle,metaData);
		dec(fHandle->curPagePos,0);
		dec(fHandle->fileName,NULL);
		dec(fHandle->mgmtInfo,NULL);
		dec(fHandle->totalNumPages,0);
		dec(fHandle,NULL);
		freeMemory();
		return RC_OK;
	})
	else
	{
		return RC_FILE_HANDLE_NOT_INIT;
	}
}

// This function is used to delete a pageFile.


RC destroyPageFile(char *fileName)
{
	condif((eqfn(remove(fileName),0)),
		return RC_OK;)
	else
		return RC_FILE_NOT_FOUND;
}


// This function is uesd to write the content of memPage to the file(fHandle->fileNam)e at the the block number pageNum.
 
RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	condif((lessthan(pageNum,0) || greaterthan(pageNum,fHandle->totalNumPages)),
		return RC_WRITE_FAILED;)
	else
	{
		dec(int startPosition,((pageNum * PAGE_SIZE) + (2 * PAGE_SIZE)));

		dec(FILE *fp,fopen(fHandle->fileName, "r+b"));
		condif((notequal(fp,NULL)),
		{
			condif((eqfn(fseek(fp, startPosition, SEEK_SET),0)),
			{
				fwrite(memPage, 1, PAGE_SIZE, fp);
				condif((pageNum > fHandle->curPagePos),
					fHandle->totalNumPages++;)
				dec(fHandle->curPagePos,pageNum);
				fclose(fp);
				return RC_OK;
			})
			else
			{
				return RC_WRITE_FAILED;
			}
		}) 
		else
		{
			return RC_FILE_HANDLE_NOT_INIT;
		}
	}
}

//This function is used to write the content of memPage to the file fHandle->fileName.It is written at the the block num fhandle->currentPagePos.
 

RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	condif((eqfn(writeBlock(fHandle->curPagePos, fHandle, memPage),RC_OK)),
		return RC_OK;)
	else
		return RC_WRITE_FAILED;
}


// This function is useTo add an empty Block at End Of File.
 

RC appendEmptyBlock(SM_FileHandle *fHandle)
{
	dec(whoIsCallingCreate,2);
	condif((createPageFile(fHandle->fileName) == RC_OK),
	{
		//the value of totalNumPages gets changed and curPagePos gets changed as we are adding new blocks
		fHandle->totalNumPages++;
		dec(fHandle->curPagePos,fHandle->totalNumPages - 1);
		dec(whoIsCallingCreate,1);
		return RC_OK;
	})
	else
	{
		dec(whoIsCallingCreate,1);
		return RC_WRITE_FAILED;
	}
}

//This function adds no of Blocks at EOF.
 

RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle)
{
	dec(int extraPagesToBeAdded,(numberOfPages - (fHandle->totalNumPages)));
	int iLoop;
	condif((greaterthan(extraPagesToBeAdded,0)),
	{
		for (dec(iLoop,0);lessthan(iLoop,extraPagesToBeAdded); iLoop++)
		{
			dec(whoIsCallingCreate,3);
			createPageFile(fHandle->fileName);
			// As we are adding new blocks,the value of totalNumPages and curPagePos gets changes
			dec(whoIsCallingCreate,1);
			fHandle->totalNumPages++;
			dec(fHandle->curPagePos,fHandle->totalNumPages - 1);
		}
		return RC_OK;
	}) else
		return RC_READ_NON_EXISTING_PAGE;
}


//RC_READ_NON_EXISTING_PAGE is returned when pageNum is greater than the total number of pages in the Page File.The Page File is opened.The page is read using fread().


RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	//if n is the value of pageNum, it reads n+2th page.
	condif((lessthan(fHandle->totalNumPages,pageNum)),
	{
		return RC_READ_NON_EXISTING_PAGE;
	})
	else
	{
		FILE *fp;
		dec(fp,fopen(fHandle->fileName, "r"));
		condif((notequal(fp,NULL)),
		{
			condif((fseek(fp, ((pageNum * PAGE_SIZE) + 2 * PAGE_SIZE), SEEK_SET)== 0),
			{
				fread(memPage, PAGE_SIZE, 1, fp);
				dec(fHandle->curPagePos,pageNum);
				fclose(fp);
				return RC_OK;
			})
			else
			{
				return RC_READ_NON_EXISTING_PAGE;
			}
		}) else
		{
			return RC_FILE_NOT_FOUND;
		}
	}
}

// This function returns the current Page position.
 
int getBlockPos(SM_FileHandle *fHandle)
{
	return fHandle->curPagePos;
}



// This function calls readBlock.It reads the first block.The position of first block is 0th block in the Page File.
 
RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	condif( (readBlock(0, fHandle, memPage) == RC_OK),
		return RC_OK;)
	else
		return RC_READ_NON_EXISTING_PAGE;
}


// This function calls readBlock.It reads the previous block
 
RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	condif((readBlock(getBlockPos(fHandle) - 1, fHandle, memPage) == RC_OK),
		return RC_OK;)
	else
		return RC_READ_NON_EXISTING_PAGE;
}


// THis function calls readBlock.It reads the current block
 
RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	condif((readBlock(getBlockPos(fHandle), fHandle, memPage) == RC_OK),
		return RC_OK;)
	else
		return RC_READ_NON_EXISTING_PAGE;
}


// This function calls readBlock.It read the next block.
 

RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	condif((readBlock(getBlockPos(fHandle) + 1, fHandle, memPage) == RC_OK),
		return RC_OK;)
	else
		return RC_READ_NON_EXISTING_PAGE;
}

// This function calls readBlock.It read the last block from the Page File.
 
RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	condif((readBlock(fHandle->totalNumPages - 1, fHandle, memPage) == RC_OK),
		return RC_OK;)
	else
		return RC_READ_NON_EXISTING_PAGE;
}

