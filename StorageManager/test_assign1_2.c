#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

// variable test name
char *testName;

/* link to test output files */
#define TESTPF "test_pagefile.bin"

/* prototypes */
static void testCreateOpenClose(void);
static void testSinglePageContent(void);

/* main function*/
int
main (void)
{
  testName = "";
  
  initStorageManager();
  testSinglePageContent();
  testCreateOpenClose();

  return 0;
}


/* It checks a return code.If output is correct,RC_OK will be true.If it is not RC_OK then output a message, error description, and exit */
// This function test the create, open, and close of a page file functionality
void
testCreateOpenClose(void)
{
  SM_FileHandle fh;

  testName = "test create open and close methods";

  TEST_CHECK(createPageFile (TESTPF));
  
  TEST_CHECK(openPageFile (TESTPF, &fh));
  ASSERT_TRUE(strcmp(fh.fileName, TESTPF) == 0, "filename correct");
  ASSERT_TRUE((fh.totalNumPages == 1), "expect 1 page in new file");
  ASSERT_TRUE((fh.curPagePos == 0), "freshly opened file's page position should be 0");

  TEST_CHECK(closePageFile (&fh));
  TEST_CHECK(destroyPageFile (TESTPF));

  // It tries to open file after destruction.It should give an error
  ASSERT_TRUE((openPageFile(TESTPF, &fh) != RC_OK), "opening non-existing file should return an error.");

  TEST_DONE();
}

/* This function create and open file and perform testing of various scenario.After all the test id pass, it closes the page file */
void
testSinglePageContent(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;

  testName = "test single page content";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

	  //It creates a new page file
	  TEST_CHECK(createPageFile (TESTPF));
	  TEST_CHECK(openPageFile (TESTPF, &fh));
	  printf("created and opened file\n");
	  
	  // It reads first page into handle
	  TEST_CHECK(readFirstBlock (&fh, ph));
	  // This checks if the page is empty (zero bytes) or not
	  for (i=0; i < PAGE_SIZE; i++)
	    ASSERT_TRUE((ph[i] == 0), "expected zero byte in first page of freshly initialized page");
	  printf("first block was empty\n");
	    
	  // This checks the writing of block on disk.It change ph to a string and write it to one of the disk
	  for (i=0; i < PAGE_SIZE; i++)
	    ph[i] = (i % 10) + '0';
	  TEST_CHECK(writeBlock (0, &fh, ph));
	  printf("writing first block\n");

	  // This condition reads back the page containing the string and check if it is correct
	 TEST_CHECK(readBlock (0,&fh, ph));
	  for (i=0; i < PAGE_SIZE; i++)
	    ASSERT_TRUE((ph[i] == (i % 10) + '0'), "character in page read from disk is the one we expected.");
	  printf("reading first block\n");
	  
	  //This condition checks appending an empty block to the page of file.
	  TEST_CHECK(appendEmptyBlock(&fh));
          printf("Appending empty block\n");

	  //This condition changes ph to a string and checks it's writing that one to disk
	  for (i=0; i < PAGE_SIZE; i++)
	    ph[i] = (i % 10) + '0';
	  TEST_CHECK(writeCurrentBlock (&fh, ph));
	  printf("Writing current block\n");

	  //This condition reads back the page containing the string and check if it is correct
	 TEST_CHECK(readCurrentBlock (&fh, ph));
	  for (i=0; i < PAGE_SIZE; i++)
	    ASSERT_TRUE((ph[i] == (i % 10) + '0'), "character in page read from disk is the one we expected.");
	  printf("Reading current block\n");

	  //This condition checks the functionality of reading the first page.
	  TEST_CHECK(readFirstBlock (&fh, ph));
	  printf("Reading first block\n");

        //This condition checks the functionality of reading the current page.
	 TEST_CHECK(readCurrentBlock(&fh,ph));
	 printf("Reading current block\n");

        //This condition checks the functionality of reading the next page.
	 TEST_CHECK(readNextBlock(&fh,ph));
	 printf("Reading next block\n");

	 //This condition checks the functionality of reading the previous page.
	 TEST_CHECK(readPreviousBlock(&fh,ph));
	 printf("Reading previous block\n");

	 //This condition checks the functionality of reading the last page.
	 TEST_CHECK(readLastBlock(&fh,ph));
	 printf("Reading last block\n");

         //This condition checks the functionality of increasing file capacity or size.
         TEST_CHECK(ensureCapacity (7,&fh))
         printf("File capacity increasesd.\n");
	 // THis condition checks the functionality of destroying new page file
	 TEST_CHECK(destroyPageFile (TESTPF));  
  
  TEST_DONE();
}
