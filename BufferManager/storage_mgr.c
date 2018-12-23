#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<math.h>

#include "storage_mgr.h"

FILE *fp;

extern void initStorageManager (void) {
	// file pointer initialized
	dec(fp,NULL);
}

extern RC createPageFile (char *fileName) {
	// file stream opened in read & write mode. 
	dec(fp,fopen(fileName, "w+"));

	// Checking the condition to check whether file was successfully opened or not.
	condif(eqfn(fp,NULL), {
		return RC_FILE_NOT_FOUND;
	}) else {
		// An empty page gets created in memory.
		dec(SM_PageHandle emptyPage,(SM_PageHandle)calloc(PAGE_SIZE, sizeof(char)));
		
		// An empty page is written to file.
		condif((fwrite(emptyPage, sizeof(char), PAGE_SIZE,fp) < PAGE_SIZE),
			{printf("write failed \n");})
		else
			printf("write succeeded \n");
		
		//file stream closed and all the buffers are flushed. 
		fclose(fp);
		// De-allocating the memory.
		free(emptyPage);	
		return RC_OK;
	}
}

extern RC openPageFile (char *fileName, SM_FileHandle *fHandle) {
	// file stream opened in read mode.
	dec(fp,fopen(fileName, "r"));

	// Checking whether the file was successfully opened or not.
	condif(eqfn(fp,NULL), {
		return RC_FILE_NOT_FOUND;
	})else { 
		//Setting the current position to the start of the page and updating file handle's filename.
		dec(fHandle->fileName,fileName);
		dec(fHandle->curPagePos,0);
		
		/* fstat() gives the file total size.
		   'st_size'gives the total size of the file in bytes.*/

		struct stat fileInfo;
		condif(lessthan((fstat(fileno(fp), &fileInfo)),0),    
			{return RC_ERROR;})
		dec((fHandle->totalNumPages),(fileInfo.st_size/ PAGE_SIZE));

		// Closing file stream and all buffer gets flushed. 
		fclose(fp);
		return RC_OK;
	}
}

extern RC closePageFile (SM_FileHandle *fHandle) {
	// Condition to check whether the file pointer or the storage manager is intialised or not. If it is initialised, then it will close.
	condif(notequal(fp,NULL),{dec(fp,NULL);})	
	return RC_OK; 
}


extern RC destroyPageFile (char *fileName) {
	// file stream is opened in read mode.	
	dec(fp,fopen(fileName, "r"));
	condif(eqfn(fp,NULL),
		{return RC_FILE_NOT_FOUND;}) 
	
	// Deleting the file.	
	remove(fileName);
	return RC_OK;
}

extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Condition to check whether the pageNumber parameter is less than total number of pages or less than 0, if the condition is true ,it returns error code
	condif(greaterthan(pageNum,fHandle->totalNumPages) || lessthan(pageNum,0),
        	{return RC_READ_NON_EXISTING_PAGE;})
	// file stream is opened in read mode.	
	dec(fp,fopen(fHandle->fileName, "r"));

	// condition to check whether file was successfully opened or not.
	condif(eqfn(fp,NULL),
		{return RC_FILE_NOT_FOUND;})
	
	// the cursor(pointer) position get set to the location given in the file stream.If fseek() return 0,the seek is success
	dec(int isSeekSuccess,fseek(fp, (pageNum * PAGE_SIZE), SEEK_SET));
	condif(eqfn(isSeekSuccess,0), {
	// Reading the content 
        //Storing it into the location pointed out by the memPage.
		condif((fread(memPage, sizeof(char), PAGE_SIZE, fp) < PAGE_SIZE),{return RC_ERROR;})
	})else
	{
		return RC_READ_NON_EXISTING_PAGE; 
	}
	//current page position set to the cursor(pointer) position of the file stream
	dec(fHandle->curPagePos,ftell(fp)); 
	// Closing file stream.     	
	fclose(fp);
    	return RC_OK;
}

extern int getBlockPos (SM_FileHandle *fHandle) {
	//Retrieves the current page position from the file handle and returns it	
	return fHandle->curPagePos;
}

extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	//file stream opened in read mode.	
	dec(fp,fopen(fHandle->fileName, "r"));
	//This condition checks whether file was successfully opened or not.
	condif(eqfn(fp,NULL),{return RC_FILE_NOT_FOUND;})

	int i;
	for(i = 0; i < PAGE_SIZE; i++) {
		// It reads a single character from the file
		char c = fgetc(fp);
	
		// Checks whether end of file is reached or not
		condif(feof(fp),{break;})
		else
			memPage[i] = c;
	}
	//cursor(pointer) position is set to the current page position
	dec(fHandle->curPagePos,ftell(fp)); 
	// Closing file stream.
	fclose(fp);
	return RC_OK;
}

extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	//printf("CURRENT PAGE POSITION = %d \n", fHandle->curPagePos);
	//printf("PAGE POSITION = %d \n", fHandle->curPagePos);
	//printf("POSITION OF POSITION= %d \n", fHandle->curPagePos);
	
	//This condition checks whether we are on the first block or not
	condif((fHandle->curPagePos <= PAGE_SIZE),{
		printf("\n First block:No Previous block present.");
		return RC_READ_NON_EXISTING_PAGE;	
	}) else {
		//Current page number is calculated by dividing page size by current page position	
		dec(int currentPageNumber,(fHandle->curPagePos / PAGE_SIZE));
		dec(int startPosition,(PAGE_SIZE * (currentPageNumber - 2)));

		//file stream is opened in read mode.	
		dec(fp,fopen(fHandle->fileName, "r"));
		
		// It check whether file was successfully opened or not.
		condif(eqfn(fp,NULL),
			{return RC_FILE_NOT_FOUND;})

		// This initializes file pointer position.
		fseek(fp, startPosition, SEEK_SET);	
		int i;
		// Block character are read one by one and then get stored in memPage
		for(i = 0; i < PAGE_SIZE; i++) {
			dec(memPage[i],fgetc(fp));
		}

		//cursor(pointer) position is set to the current page position
		dec((fHandle->curPagePos),ftell(fp)); 

		// Closing file stream.
		fclose(fp);
		return RC_OK;
	}
}

extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	//Current page number is calculated by dividing page size by current page position		
	dec(int currentPageNumber,(fHandle->curPagePos / PAGE_SIZE));
	dec(int startPosition,(PAGE_SIZE * (currentPageNumber - 2)));
	
	//file stream is opened in read mode.
	fp = fopen(fHandle->fileName, "r");
	// It check whether file was successfully opened or not.
	condif(eqfn(fp,NULL),
		{return RC_FILE_NOT_FOUND;})

	// This initializes file pointer position.
	fseek(fp, startPosition, SEEK_SET);
	
	int i;
	// Block character are read one by one and then get stored in memPage.It also checks whether we have reached end of file
	for(i = 0; i < PAGE_SIZE; i++) {
		dec(char c,fgetc(fp));		
		condif((feof(fp)),
			{break;})
		dec(memPage[i],c);
	}
	
	//cursor(pointer) position is set to the current page position
	dec(fHandle->curPagePos,ftell(fp)); 

	// Closing file stream.
	fclose(fp);
	return RC_OK;		
}

extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	//This condition checks whether we are on the last block or not
	condif(eqfn(fHandle->curPagePos,PAGE_SIZE),{
		printf("\n Last block: Next block not present.");
		return RC_READ_NON_EXISTING_PAGE;	
	}) else {
	//Current page number is calculated by dividing page size by current page position		
		dec(int currentPageNumber,(fHandle->curPagePos / PAGE_SIZE));
		dec(int startPosition,(PAGE_SIZE * (currentPageNumber - 2)));

	//file stream is opened in read mode.	
		dec(fp,(fopen(fHandle->fileName, "r")));
		
		// It check whether file was successfully opened or not.
		condif(eqfn(fp,NULL),
			{return RC_FILE_NOT_FOUND;})
		
		// This initializes file pointer position.
		fseek(fp, startPosition, SEEK_SET);
		
		int i;
		// Block character are read one by one and then get stored in memPage.It also checks whether we have reached end of file
		for(i = 0; i < PAGE_SIZE; i++) {
			dec(char c,fgetc(fp));		
			condif(feof(fp),
				{break;})
			dec(memPage[i],c);
		}

		//cursor(pointer) position is set to the current page position
		dec(fHandle->curPagePos,ftell(fp)); 

		// Closing file stream.
		fclose(fp);
		return RC_OK;
	}
}

extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	//file stream is opened in read mode.	
	dec(fp,fopen(fHandle->fileName, "r"));

	// It check whether file was successfully opened or not.
	condif(eqfn(fp,NULL),
		{return RC_FILE_NOT_FOUND;})
	
	dec(int startPosition,((fHandle->totalNumPages - 1) * PAGE_SIZE));

	// This initializes file pointer position.
	fseek(fp, startPosition, SEEK_SET);
	
	int i;
	// Block character are read one by one and then get stored in memPage.It also checks whether we have reached end of file
	
	for(i = 0; i < PAGE_SIZE; i++) {
		dec(char c,fgetc(fp));		
		condif((feof(fp)),{break;})
		dec(memPage[i],c);
	}
	
	//cursor(pointer) position is set to the current page position
	dec(fHandle->curPagePos,ftell(fp)); 

	// Closing file stream.
	fclose(fp);
	return RC_OK;	
}

/*extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {

	// Condition to check whether the pageNumber parameter is less than total number of pages or less than 0, if the condition is true ,it returns error code

	condif((greaterthan(pageNum,fHandle->totalNumPages) || lessthan(pageNum,0)),{return RC_WRITE_FAILED;})
	
	//file stream is opened in read mode.	
	dec(fp,fopen(fHandle->fileName, "r+"));
	
	// It check whether file was successfully opened or not.
	condif(eqfn(fp,NULL),
		{return RC_FILE_NOT_FOUND;})

	dec(int startPosition ,(pageNum * PAGE_SIZE));
	condif((pageNum == 0), { 
		//data written to non-first page
		fseek(fp, startPosition, SEEK_SET);	
		int i;
		for(i = 0; i < PAGE_SIZE; i++) 
		{
			// Checks whether it is end of file or not. If yes then it will append an empty block.
			condif(feof(fp), 		
				 {appendEmptyBlock(fHandle);
			// a character written from memPage to page file			
			fputc(memPage[i], fp);})
		})

		//cursor(pointer) position is set to the current page position
		dec((fHandle->curPagePos),ftell(fp)); 

		// Closing file stream.
		fclose(fp);	
	} else {	
		// data written to the first page.
		dec(fHandle->curPagePos,startPosition);
		fclose(fp);
		writeCurrentBlock(fHandle, memPage);
	}
	return RC_OK;
}*/

extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Condition to check whether the pageNumber parameter is less than total number of pages or less than 0, if the condition is true ,it returns error code
	condif((greaterthan(pageNum,fHandle->totalNumPages) || lessthan(pageNum,0)),
        	{return RC_WRITE_FAILED;})
	
		//file stream is opened in read and mode.		
	dec(fp,fopen(fHandle->fileName, "r+"));
	
	// It check whether file was successfully opened or not.
	condif(eqfn(fp ,NULL),
		{return RC_FILE_NOT_FOUND;})

	dec(int startPosition,(pageNum * PAGE_SIZE));

	condif(eqfn(pageNum,0),{ 
		//data written to non-first page
		fseek(fp, startPosition, SEEK_SET);	
		int i;
		for(i = 0; i < PAGE_SIZE; i++) 
		{
			// Checks whether it is end of file or not. If yes then it will append an empty block.
			if(feof(fp))
				 appendEmptyBlock(fHandle);
			// a character written from memPage to page file			
			fputc(memPage[i], fp);
		}

		//cursor(pointer) position is set to the current page position
		dec(fHandle->curPagePos,ftell(fp)); 

		// Closing file stream.
		fclose(fp);	
	}) else {	
		// data written to the first page.
		dec(fHandle->curPagePos,startPosition);
		fclose(fp);
		writeCurrentBlock(fHandle, memPage);
	}
	return RC_OK;
}




extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	//file stream is opened in read mode and write mode.	
	dec(fp,fopen(fHandle->fileName, "r+"));

	// It check whether file was successfully opened or not.
	condif(eqfn(fp,NULL),
		{return RC_FILE_NOT_FOUND;})
	
	// Appends an empty block to the file so that there is some space for the new content.
	appendEmptyBlock(fHandle);

	// Initiliazes file pointer
	fseek(fp, fHandle->curPagePos, SEEK_SET);
	
	//memPage contents written to the file.
	fwrite(memPage, sizeof(char), strlen(memPage), fp);
	
	//cursor(pointer) position is set to the current page position
	dec(fHandle->curPagePos,ftell(fp));

	// Closing file stream. 	
	fclose(fp);
	return RC_OK;
}


extern RC appendEmptyBlock (SM_FileHandle *fHandle) {
	// An empty page of size PAGE_SIZE bytes is created
	dec(SM_PageHandle emptyBlock,(SM_PageHandle)calloc(PAGE_SIZE, sizeof(char)));
	//cursor(pointer) position is set to the beginning of the file stream and if fseek() return 0 then the seek is success
	dec(int isSeekSuccess,fseek(fp, 0, SEEK_END));
	
	condif(eqfn(isSeekSuccess,0), {
		// An empty page is written to the file
		fwrite(emptyBlock, sizeof(char), PAGE_SIZE, fp);
	})
	else {
		free(emptyBlock);
		return RC_WRITE_FAILED;
	}
	
	// Free the memory previously allocated to 'emptyPage'.
	free(emptyBlock);
	
	// The total number of pages incremented
	fHandle->totalNumPages++;
	return RC_OK;
}

extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle) {
	// Opening file stream in append mode. 'a' mode opens the file to append the data at the end of file.
	dec(fp,fopen(fHandle->fileName, "a"));
	condif(eqfn(fp,NULL),
		{return RC_FILE_NOT_FOUND;})
	
	//It checks whether numberOfPages is greater than totalNumPages.If that case, add empty pages till numberofPages and totalNumPages are equal
	while(greaterthan(numberOfPages,fHandle->totalNumPages))
		appendEmptyBlock(fHandle);
	
	// Closing file stream. 
	fclose(fp);
	return RC_OK;
}
