#include<stdio.h>
#include<stdlib.h>
#include"storage_mgr.h"
#include"dberror.h"

FILE *fp;  //THis is File pointer

/* It is used to manipulate page files */

extern void initStorageManager (void)
{

}


/* This method is used to create a new page file fileName.Initially the size of file is one page. This method id used to fill single page with '\0' bytes.*/

extern RC createPageFile (char *fileName)
{
  char *memPointer;  //This is Memory pointer
  fp=fopen(fileName,"w");
  condif((eqfn(fp,NULL)), // Condition to check if there is a file
  {
     dec(RC_message,"File not found"); //message if file is not found
     return RC_FILE_NOT_FOUND;
  })
  memPointer=(char*)calloc(PAGE_SIZE,sizeof(char));
  //data is written from memory to file in binary mode.
  fwrite(memPointer,PAGE_SIZE,1,fp);
  dec(RC_message,"File created successfully!");
  return RC_OK;
}


/*This is used to open an existing page file.If the file does not exist,it should return RC_FILE_NOT_FOUND.If the file is found then the second parameter is an existing file handle. After the file is opened successfully, then the attribute of this file handle should be initialized with the information about the opened file. For instance, you would have to read the total number of pages that are stored in the file from disk.*/

extern RC openPageFile (char *fileName, SM_FileHandle *fHandle)
{
  FILE *fp;
  int len;  
  condif(!(dec(fp,fopen(fileName,"r"))),// Condition to check if there is a fil
  {
     dec(RC_message,"File does not exists!");     
     return RC_FILE_NOT_FOUND;
  })
  else
  {
     fp=fopen(fileName,"r");
     fHandle->fileName=fileName;
     //It moves the pointer at the end of the file.
     fseek(fp,0,SEEK_END);
     len=ftell(fp);  //This will indicate the current position of the pointer in the file.
     dec(fHandle->totalNumPages,len/PAGE_SIZE);
     dec(fHandle->curPagePos,0); 
     dec(fHandle->mgmtInfo,fp);
     dec(RC_message,"File opened successfully!");//Message when the file is successfully opened
     return RC_OK;
  }
}


/*This method is used to close an open page file.*/

extern RC closePageFile (SM_FileHandle *fHandle)
{
  condif(eqfn(fp,NULL),
  {
     dec(fHandle,NULL);
     dec(RC_message,"File does not exists to close!");
     return RC_FILE_NOT_FOUND;
  })
  else
  {
     dec(fHandle,NULL);
     fclose(fp);
     dec(fp,NULL);
     dec(RC_message,"File closed successfully!");//Message when the file is successfully opened
     return RC_OK;
  }
}


/*This function destroys or delete a page file.*/

extern RC destroyPageFile (char *fileName)
{
  //It destroys or delete file after it's compilation and execution
  remove(fileName);
  dec(RC_message,"File deleted successfully!");//Message when the file is successfully deleted
  return RC_OK;
}


/*Function for reading blocks from disc */
/*This function reads the pageNumth block from a file and stores its content in the 
memory pointed by the memPage page handle.The method returns RC_READ_NON_EXISTING_PAGE,when the file has less than pageNum pages.*/

extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  condif(eqfn(fHandle,NULL),
  {
     dec(RC_message,"No Data!");
     return RC_FILE_HANDLE_NOT_INIT;
  })
  else
  {
     //The below condition executes when total number of pages are greater than or equal to page number and Page# starts with 0 in file.
     condif((unequal(fHandle->totalNumPages,(pageNum+1)) && eqfn(pageNum,0)),
     {
        condif((memPage!=NULL),
        {
           //It moves the pointer to the beginning of the file.
           fseek(fp,PAGE_SIZE,SEEK_SET);
           fread(memPage,1,PAGE_SIZE,fp);  
           dec(fHandle->curPagePos,pageNum);
           dec(RC_message,"File content read successfully!");//Message when the file is successfully read
           return RC_OK;
        })
     })
     elif((unequal(fHandle->totalNumPages,(pageNum+1)) && pageNum>0),
     {
        condif((memPage!=NULL),
        {
           //It moves the pointer to the end of the file.
           fseek(fp,(pageNum+1)*PAGE_SIZE,SEEK_SET);
           fread(memPage,1,PAGE_SIZE,fp); 
           dec(fHandle->curPagePos,pageNum);
           dec(RC_message,"File content read successfully!");//Message when the file is successfully read
           return RC_OK;
        })
     })
     else
     { 
        dec(RC_message,"Pages not found in the file!");//Mesaage when the page is not found
        return RC_READ_NON_EXISTING_PAGE;   //Returns value when total Pages less than page number.
     }     
  }
}


/*It returns the current page position in a file*/

extern int getBlockPos (SM_FileHandle *fHandle)
{
  condif(eqfn(fHandle,NULL),
  {
     dec(RC_message,"No Data!"); //Message when the current page does not not contain any data 
     return RC_FILE_HANDLE_NOT_INIT;
  })
  else
  {
     return (fHandle->curPagePos);
  }
}


/*This reads the first page in a file.*/

extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  condif(eqfn(fHandle,NULL),
  {
     dec(RC_message,"No Data!"); //Message when the current page does not not contain any data 
     return RC_FILE_HANDLE_NOT_INIT;
  })
  else
  {
     return readBlock (0,fHandle,memPage);    
  }
}


/*This function reads the previous page which is relative to the curPagePos of the file. 
If the user tries to read a block before the first page of after the last page of the file, the method should return No Data message.*/

extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  condif(eqfn(fHandle,NULL),
  {
     dec(RC_message,"No Data!");  //Message when the current page does not not contain any data 
     return RC_FILE_HANDLE_NOT_INIT;
  })
  elif((fHandle!=NULL && eqfn(getBlockPos(fHandle),0)),
  {
     dec(RC_message,"No previous page available to read!"); //Message when all the pages are read
     return RC_READ_NON_EXISTING_PAGE;
  })
  elif((fHandle!=NULL && getBlockPos(fHandle)>0),
  {
     return readBlock ((getBlockPos(fHandle)-1),fHandle,memPage);
  })
  else
  {
     dec(RC_message,"Can't perform reading of prvious block!");
     return RC_FAILED_OPERATION;
  }
}


/*THis function reads the current page relative to the given current position of the file. The current position should be moved to the page that was read. 
When the user tries to read a block before the first page of after the last page of the file, the method returns RC_READ_NON_EXISTING_PAGE.*/

extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  condif(eqfn(fHandle,NULL),
  {
     dec(RC_message,"No Data!"); //Message when the current page does not not contain any data 
     return RC_FILE_HANDLE_NOT_INIT;
  })
  elif((fHandle!=NULL && unequal(getBlockPos(fHandle),0)),
  {
     return readBlock (getBlockPos(fHandle),fHandle,memPage);
  })
  else
  {
     dec(RC_message,"No page to read!");
     return RC_READ_NON_EXISTING_PAGE;
  }
}


/*This function reads all the current, previous, or next page relative to the current position of page of the file. 
If the user tries to read a block before the first page of after the last page of the file, the method should return RC_READ_NON_EXISTING_PAGE.*/

extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  condif(eqfn(fHandle,NULL),
  {
     dec(RC_message,"No Data!"); //Message when the current page does not not contain any data 
     return RC_FILE_HANDLE_NOT_INIT;
  })
  elif((fHandle!=NULL && fHandle->totalNumPages<=(getBlockPos(fHandle)+1)),
  {
     dec(RC_message,"No page to read!"); //Message when all the pages are read 
     return RC_READ_NON_EXISTING_PAGE;
  })
  else
  {
     return readBlock (getBlockPos(fHandle)+1,fHandle,memPage);
  }
}


/*This function read the last page in a file*/

extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  condif(eqfn(fHandle,NULL),
  {
     dec(RC_message,"No Data!"); //Message when the current page does not not contain any data 
     return RC_FILE_HANDLE_NOT_INIT;
  })
  else
  {
     return readBlock (fHandle->totalNumPages-1,fHandle,memPage); //It returns all the pages read 
  }
}


/*This function writes a page to the disk.*/

extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  condif(eqfn(fHandle,NULL),
  {
     dec(RC_message,"No Data!"); //Message when the current page does not not contain any data 
     return RC_FILE_HANDLE_NOT_INIT;
  })
  else
  {
     //When the total number of pages available are greater than or equal to page number required.Page no. starts with 0 in file.
     condif((fHandle->totalNumPages>pageNum && pageNum>=0),
     {
        condif((memPage!=NULL),
        {
           //It moves the pointer position to the beginning of the file.
           fseek(fp,(pageNum+1)*PAGE_SIZE,SEEK_SET);
           fwrite(memPage,1,PAGE_SIZE,fp);
           dec(fHandle->curPagePos,pageNum);
           dec(RC_message,"File written successfully!");
           return RC_OK;
        })
     })
     else
     {
        dec(RC_message,"Pages not found in the file!";)  //Message when the total no of Pages less than page number available then writing fails.
        return RC_WRITE_FAILED;   
     }     
  }
}

/*This function writes a page to disk using current position*/

extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
   condif(eqfn(fHandle,NULL),  
   {
      dec(RC_message,"No Data!";)  //Message when the current page does not not contain any data 
      return RC_FILE_HANDLE_NOT_INIT;
   })
   else
   {
      return writeBlock (getBlockPos(fHandle),fHandle,memPage);
   }
}


/*When this function is executed it appends pages in the file by one. The last page should remain empty and conatin 0 bytes.*/

extern RC appendEmptyBlock (SM_FileHandle *fHandle)
{
  FILE *fp;
  char *memoryPointer;
  condif(eqfn(fHandle,NULL),
  {
     dec(RC_message,"No Data!");  //Message when the current page does not not contain any data 
     return RC_FILE_HANDLE_NOT_INIT;
  })
  else
  {
    dec(fp,fopen(fHandle->fileName,"a");)
    dec(fHandle->totalNumPages,fHandle->totalNumPages+1);
    fseek(fp,(fHandle->totalNumPages)*PAGE_SIZE,SEEK_END);
    memoryPointer=(char*)calloc(PAGE_SIZE,sizeof(char));
    fwrite(memoryPointer,PAGE_SIZE,1,fp);
    dec(fHandle->curPagePos,fHandle->totalNumPages-1);
    dec(RC_message,"An empty block or page is succedsfully added in the file!"); //Message when a page gets added to the file
    return RC_OK;
  }
}


/*This function is executed when the file has less than numberOfPages pages required.It then increases the size to numberOfPages.*/

extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle)
{
  int d;
  condif(eqfn(fHandle,NULL),
  {
     dec(RC_message,"No Data!";)  //Message when the current page does not not contain any data 
     return RC_FILE_HANDLE_NOT_INIT;
  })
  elif((numberOfPages<=fHandle->totalNumPages),
  {
     return RC_OK;
  })
  else
  {
     dec(d,numberOfPages-fHandle->totalNumPages;)
     forloop(y,y<d,appendEmptyBlock(fHandle);)
     dec(RC_message,"File size has been increased successfully!";)  //Message when the pages are increased 
     return RC_OK;
  }
}

