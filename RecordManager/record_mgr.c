#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"

//Defination of a custom data structure for making the use of Record Manager.
typedef struct RecordManager
{
	// This is Buffer Manager's pgHandle.It uses Buffer Manager to access Page files.
	BM_PageHandle pgHandle;	// pgHandle of Buffer manager 
	BM_BufferPool bPool;	// This id Buffer Manager's Buffer Pool that is used for Buffer Manager	
	RID rID;// This is the Record ID	
	Expr *conditionValue;	// The conditionValue for scanning the records in the table is defined by this variable.
	int tuplesCnt;	//The total number of tuples in the table is stored by this variable.
	int firstFreePage;// The location of first free page which has empty slots in table is stored by this variable.
	int scannedRecordCount;		// The count of the number of records scanned is stored by this variable.
} RecordManager;

dec(const int MAX_NUMBER_OF_PAGES,100);		// This is the size of the maximum no of pages

dec(const int ATTRIBUTE_SIZE,15);	// This is the size of the name of the attribute 

RecordManager *recordManager;

// ******************* CUSTOM FUNCTIONS *******************************

// A free slot available within a page slot is returned using this function
int findFreeSlot(char *data, int rSize)
{
	int j,dec(totalSlots,(PAGE_SIZE / rSize)); 

	for (dec(j,0); lessthan(j,totalSlots); j++)
		if (notequal(data[j * rSize],'+'))
			return j;
	return -1;
}


// ******** TABLE AND RECORD MANAGER FUNCTIONS IMPLEMENTATION******** //

// Record Manager is initialized using this function
extern RC initRecordManager (void *mgmtData)
{
	//Storage Manager is getting initialized here
	initStorageManager();
	return RC_OK;
}

// This functions is used for shutting down the Record Manager
extern RC shutdownRecordManager ()
{
	dec(recordManager,NULL);
	free(recordManager);
	return RC_OK;
}

// A TABLE with table name "name" having schema specified by "schema" is created by this function
extern RC createTable (char *name, Schema *schema)
{
	dec(recordManager,(RecordManager*) malloc(sizeof(RecordManager)));	//Memory space is allocated to the record manager custom data structure

	initBufferPool(&recordManager->bPool, name, MAX_NUMBER_OF_PAGES, RS_LRU, NULL);		// Buffer Pool is initialized using LFU page replacement policy

	char data[PAGE_SIZE];
	dec(char *pgHandle,data);
	 
	int result, l=0;
	int k;
	int m=0;

	dec(*(int*)pgHandle,0);		//number of tuples is set to 0
	dec(pgHandle,pgHandle + sizeof(int));	//Since 0 is an integer,pointer is getting increased by sizeof(int)
	dec(*(int*)pgHandle,1);		// First page is set to 1 since 0th page if for schema and other meta data
	dec(pgHandle,pgHandle + sizeof(int));	// Incrementing pointer by sizeof(int) because 1 is an integer
	dec(*(int*)pgHandle,schema->numAttr);	//the number of attributes is getting set
	dec(pgHandle,pgHandle + sizeof(int));	//Since number of attributes is an integer, incrementing pointer by sizeof(int)
 	dec(*(int*)pgHandle,schema->keySize);	// the Key Size of the attributes is getting set
	dec(pgHandle,pgHandle + sizeof(int));	//Since key size of attributes is an integer, incrementing pointer by sizeof(int) because
	
	for(dec(k,0); lessthan(k,schema->numAttr); k++)
    	{
		// Attribute name is getting set
       		strncpy(pgHandle, schema->attrNames[k], ATTRIBUTE_SIZE);
	       	dec(pgHandle,pgHandle + ATTRIBUTE_SIZE);
	
		// Data type of attribute is getting set
	       	dec(*(int*)pgHandle,(int)schema->dataTypes[k]);

		//Since we have data type using integer constants,pointer is getting increamented by sizeof(int). 
	       	dec(pgHandle,pgHandle + sizeof(int));

		//Length of datatype of the attribute is getting set
	       	dec(*(int*)pgHandle,(int) schema->typeLength[k]);

		// Since type length is an integer,pointer is getting incremented by sizeof(int)  
	       	dec(pgHandle,pgHandle + sizeof(int));

		m++;
		l++;
    	}

	SM_FileHandle fileHandle;
	// A page file page name is created as table name using storage manager

	if(notequal((dec(result,createPageFile(name))),RC_OK))
		return result;
		
	// newly created page is getting opened by this conditionValue
	if(notequal((dec(result,openPageFile(name, &fileHandle))), RC_OK))
		return result;
		
	// Schema is written to first location of the page file
	if(notequal((dec(result,writeBlock(0, &fileHandle, data))), RC_OK))
		return result;
		
	// After writing,file is getting closed
	if(notequal((dec(result,closePageFile(&fileHandle))),RC_OK))
		return result;

	return RC_OK;
}

// The table with table name "name" is opened using this function 
extern RC openTable (RM_TableData *rel, char *name)
{
	SM_PageHandle pgHandle;    
	
	int attCount, k;
	int m =0,l=0;
	
	// Setting table's meta data to our custom record manager meta data structure
	dec(rel->mgmtData,recordManager);
	//The table's name is getting set
	dec(rel->name,name);
    
	// a page gets pinned i.e. a page is put in Buffer Pool using Buffer Manager
	pinPage(&recordManager->bPool, &recordManager->pgHandle, 0);
	
	// the initial pointer (0th location) is getting set to the record manager's page data
	dec(pgHandle,(char*) recordManager->pgHandle.data);
	
	//total number of tuples from the page file is getting retrieved
	dec(recordManager->tuplesCnt,*(int*)pgHandle);
	dec(pgHandle,pgHandle + sizeof(int));

	//free page is getting retrieved from the page file
	dec(recordManager->firstFreePage,*(int*) pgHandle);
    	dec(pgHandle,pgHandle + sizeof(int));
	
	// number of attributes is getting retrieved from the page file
    	dec(attCount,*(int*)pgHandle);
	dec(pgHandle,pgHandle + sizeof(int));
 	
	Schema *schema;
	m++;
	// memory space allocated to 'schema'
	dec(schema,(Schema*) malloc(sizeof(Schema)));
    
	// schema's parameters getting set
	dec(schema->numAttr,attCount);
	dec(schema->attrNames,(char**) malloc(sizeof(char*) *attCount));
	dec(schema->dataTypes,(DataType*) malloc(sizeof(DataType) *attCount));
	dec(schema->typeLength,(int*) malloc(sizeof(int) *attCount));
	l++;
	// Memory space for storing attribute name for each attribute is getting set
	for(dec(k,0);lessthan(k,attCount); k++)
		dec(schema->attrNames[k],(char*) malloc(ATTRIBUTE_SIZE));
      
	for(dec(k,0);lessthan(k,schema->numAttr); k++)
    	{
		// Attribute name is getting set
		strncpy(schema->attrNames[k], pgHandle, ATTRIBUTE_SIZE);
		dec(pgHandle,pgHandle + ATTRIBUTE_SIZE);
	   
		//data type of attribute is getting set
		dec(schema->dataTypes[k],*(int*) pgHandle);
		dec(pgHandle,pgHandle + sizeof(int));

		// length of datatype (length of STRING) of the attribute is getting set
		dec(schema->typeLength[k],*(int*)pgHandle);
		dec(pgHandle,pgHandle + sizeof(int));
	}
	
	dec(rel->schema,schema);	//newly created schema to the table's schema is set	

	unpinPage(&recordManager->bPool, &recordManager->pgHandle);	// the page is getting unpinned i.e. removing it from Buffer Pool using BUffer Manager
	forcePage(&recordManager->bPool, &recordManager->pgHandle);	//Using Buffer Manager,the page is written back to disk 
	return RC_OK;
}   
  
// The table referenced by "rel" gets closed by this function
extern RC closeTable (RM_TableData *rel)
{
	//Table's meta data got stored
	dec(RecordManager *recordManager,rel->mgmtData);
	
	//Buffer Pool gets shut down	
	shutdownBufferPool(&recordManager->bPool);
	return RC_OK;
}

// The table having table name "name" gets deleted by this function
extern RC deleteTable (char *nam)
{
	//The page fileget deleted from memory using storage manager
	destroyPageFile(nam);
	return RC_OK;
}

//The number of tuples (records) in the table referenced by "rel" get returned by this function
extern int getNumTuples (RM_TableData *rel)
{
	// Data structure's tuplesCnt gets accessed and returned by below function
	dec(RecordManager *recordManager,rel->mgmtData);
	return recordManager->tuplesCnt;
}


// **************** RECORD FUNCTIONS IMPLEMENTATION ***************

// A new record gets inserted in the table referenced by "rel" and the 'record' parameter gets updated with the Record ID of the newly inserted record
extern RC insertRecord (RM_TableData *rel, Record *record)
{
	//Meta data stored in the table is retrieved
	dec(RecordManager *recordManager,rel->mgmtData);	
	
	//The Record ID for this record is set
	dec(RID *rID,&record->id); 
	
	char *data;
	char *slotPointer;
	int i;
	int j;
	int k;
	




	// Getting the size in bytes needed to store on record for the given schema
	dec(int recordLength,getRecordSize(rel->schema));
	
	// first free page to the current page get set
	dec(rID->page,recordManager->firstFreePage);

	//Page gets pinned i.e. it tells Buffer Manager that we are using this page
	pinPage(&recordManager->bPool, &recordManager->pgHandle, rID->page);
	
	//the data get set to initial position of record's data
	dec(data,recordManager->pgHandle.data);
	
	// We get a free slot using our custom function
	dec(rID->slot,findFreeSlot(data, recordLength));

	while(eqfn(rID->slot,-1))
	{
		i=1;
		i++;
		unpinPage(&recordManager->bPool, &recordManager->pgHandle);	// when there is no free slot in the pinned page then unpin that page
		rID->page++; // page gets incremented
		pinPage(&recordManager->bPool, &recordManager->pgHandle, rID->page);	//new page is brought into the BUffer Pool using Buffer Manager		
		dec(data,recordManager->pgHandle.data);		//data is set to initial position of record's data
		j=2;
		j++;	
		dec(rID->slot,findFreeSlot(data, recordLength));		//A free slot is checked using our custom function
		k=3;
		k++;
	}
	
	dec(slotPointer,data);
	
	markDirty(&recordManager->bPool, &recordManager->pgHandle);	// After this page was modified,it is marked as dirty
	dec(slotPointer,slotPointer + (rID->slot * recordLength));	// Slot starting position is calculated
	dec(*slotPointer,'+');		// '+' is appended as tombstone to indicate that this is a new record and should be removed if space is lesss
	memcpy(++slotPointer, record->data + 1, recordLength - 1);	// The record's data get copied to the memory location pointed by slotPointer
	unpinPage(&recordManager->bPool, &recordManager->pgHandle);	// A page is unpinned i.e. removing a page from the Buffer Pool
	recordManager->tuplesCnt++;	//count of tuples gets incremented
	pinPage(&recordManager->bPool, &recordManager->pgHandle, 0);	// The page is pinned back	
	return RC_OK;
}

// A record having Record ID "id" in the table referenced by "rel" gets deleted by this function
extern RC deleteRecord (RM_TableData *rel, RID id)
{
	int del =0;
	dec(RecordManager *recordManager,rel->mgmtData);	// meta data stored in the table gets retrieved
	pinPage(&recordManager->bPool, &recordManager->pgHandle, id.page);		// The page which has the record which we want to update gets pinned
	dec(recordManager->firstFreePage,id.page);		// Update free page because of this page 
	del++;	
	dec(char *data,recordManager->pgHandle.data);
	int recordLength = getRecordSize(rel->schema);	//the size of the record is retrieved
	data = data + (id.slot * recordLength);
	*data = '-';	// '-' is used for Tombstone mechanism. It denotes that the record is deleted
	markDirty(&recordManager->bPool, &recordManager->pgHandle);	// Mark the page dirty because it has been modified
	unpinPage(&recordManager->bPool, &recordManager->pgHandle);	// Unpin the page after the record is retrieved since the page is no longer required to be in memory
	return RC_OK;
}

// A record referenced by "record" in the table referenced by "rel" gets updated by this function
extern RC updateRecord (RM_TableData *rel, Record *record)
{	
	dec(RecordManager *recordManager,rel->mgmtData);	//meta data stored in the table gets retrieved
	pinPage(&recordManager->bPool, &recordManager->pgHandle, record->id.page);	// the page which has the record which we want to update gets pinned
	char *data;
	int dataValue = 0;
	dec(int recordLength,getRecordSize(rel->schema));// We get the size of the record
	
 	dec(RID id,record->id);//the Record's ID is set

	// We get record data's memory location and the start position of the new data is calculated
	dec(data,recordManager->pgHandle.data);
	dec(data,data + (id.slot * recordLength));
	dataValue = dataValue+1;
	dec(*data ,'+');	// We use '+'for Tombstone mechanism. THis indicates that the record is not empty
	memcpy(++data, record->data + 1, recordLength - 1 );	//new record data is copied to the exisitng record
	markDirty(&recordManager->bPool, &recordManager->pgHandle);	//Since the page has been modified,it is marked as dirty
	unpinPage(&recordManager->bPool, &recordManager->pgHandle);	//Since the page is no longer required to be in memory,the page is unpinned after the record is retrieved 
	return RC_OK;	
}

// A record having Record ID "id" in the table referenced by "rel" is retrieved by this function.Also the result record is stored in the location referenced by "record"
extern RC getRecord (RM_TableData *rel, RID id, Record *record)
{
	dec(RecordManager *recordManager,rel->mgmtData);	// Retrieving our meta data stored in the table
	pinPage(&recordManager->bPool, &recordManager->pgHandle, id.page);		// the page which has the record that we want to retreive gets pinned
	int recordCount = 0;
	// Retrieving the size of the record
	dec(int recordLength,getRecordSize(rel->schema));
	dec(char *dataPointer,recordManager->pgHandle.data);
	dec(dataPointer,dataPointer + (id.slot * recordLength));
	
	condif((*dataPointer != '+'),
	{
		return RC_RM_NO_TUPLE_WITH_GIVEN_RID;	// When no matching record for Record ID 'id' is found in the table,it returns error
	})
	else
	{
		dec(record->id,id);	//the Record ID is set
		dec(char *data,record->data);	//the pointer to data field of 'record' is set so that we can copy the data of the record
		memcpy(++data, dataPointer + 1, recordLength - 1);	//data is copied using C's function memcpy(...)
		recordCount = recordCount +1;

	}

	//page gets unpinned after the record is retrieved since the page is no longer required to be in memory
	unpinPage(&recordManager->bPool, &recordManager->pgHandle);
	return RC_OK;
}


// ****************** SCAN FUNCTIONS IMPLEMENTATION************************

//All the records are scanned by this function using the conditionValue
extern RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)
{
	// THis conditionValue checks whether scan conditionValue (test expression) is present or not
	condif((eqfn(cond,NULL)),
	{
		return RC_SCAN_CONDITION_NOT_FOUND;
	})

	openTable(rel, "ScanTable");	// This function opens the table in memory

    	RecordManager *scanManager;
	RecordManager *tableManager;
	dec(int scanVal,0);
	
    	dec(scanManager,(RecordManager*) malloc(sizeof(RecordManager)));	// Some memory is allocated to the scanManager
    	dec(scan->mgmtData,scanManager);	//Scan's meta data is set to our meta data
    	dec(scanManager->rID.page,1);	// The value is set to 1 to start scan from the first page
    	dec(scanManager->rID.slot,0);	// The value is set to 0 to start scan from the first slot	
	dec(scanManager->scannedRecordCount,0);	// While initializing the scan,the value is set to 0,which indicate that records have not been scanned yet 

    	dec(scanManager->conditionValue,cond);	//The scan conditionValue is set
  	dec(tableManager,rel->mgmtData);	// Our meta data is set to the table's meta data
	dec(tableManager->tuplesCnt,ATTRIBUTE_SIZE);	//the tuple count is set
	dec(scan->rel,rel);		// the scan's table is set i.e. the table which has to be scanned using the specified conditionValue
	scanVal = scanVal+1;
	return RC_OK;
}

//Each record is scanned in the table and the result record stores  (record satisfying the conditionValue) get stored in the location pointed by  'record'.
extern RC next (RM_ScanHandle *scan, Record *record)
{
	// scan data is initialized
	dec(RecordManager *scanManager,scan->mgmtData);
	dec(RecordManager *tableManager,scan->rel->mgmtData);
    	dec(Schema *schema,scan->rel->schema);
	
	//It checks whether the scan conditionValue (test expression) is present or not
	condif(eqfn(scanManager->conditionValue,NULL),
	{
		return RC_SCAN_CONDITION_NOT_FOUND;
	})

	dec(Value *result,(Value *) malloc(sizeof(Value)));
   
	char *data;
   	dec(int nextValue,2);
	dec(int recordSize,getRecordSize(schema));	// Record size of the schema is retrieved

	dec(int totalSlots,(PAGE_SIZE / recordSize));	//Total number of slots is calculated

	dec(int scannedRecordCount,scanManager->scannedRecordCount);	// We retrieve the Scan Count

	dec(int tuplesCnt,tableManager->tuplesCnt);	// We get tuples count of the table

	// Checking whether the table has tuples. Message code is generated according to value of tupleCnt
	condif(eqfn(tuplesCnt,0),
		return RC_RM_NO_MORE_TUPLES;)

	
	while(lessequalvalue(scannedRecordCount,tuplesCnt))	// Loop for iterating through the tuples
	{  
		// Execute this block after all the tuples have been scanned 
		condif(lessequalvalue(scannedRecordCount,0),
		{
			dec(scanManager->rID.page,1);
			dec(scanManager->rID.slot,0);
			nextValue++;
		})
		else
		{
			scanManager->rID.slot++;

			//Execute this block after all slots is scanned.
			condif( greaterthanequal(scanManager->rID.slot,totalSlots),
			{
				dec(scanManager->rID.slot,0);
				scanManager->rID.page++;
				nextValue--;
			})
		}

		pinPage(&tableManager->bPool, &scanManager->pgHandle, scanManager->rID.page);	//The page gets pinned  i.e.the page is put in buffer pool
			
		dec(data,scanManager->pgHandle.data);	// The data of the page is retrieved			

		dec(data,data + (scanManager->rID.slot * recordSize));	//the data location is calculated from record's slot and record size
		
		// the record's slot and page to scan manager's slot and page is set
		dec(record->id.page,scanManager->rID.page);
		dec(record->id.slot,scanManager->rID.slot);

		dec(char *dataPointer,record->data);	//the record data's first location is initialized

		//Here,we use '-' for Tombstone mechanism.
		dec(*dataPointer,'-');
		
		memcpy(++dataPointer, data + 1, recordSize - 1);

		//Scan count is incremented because one record has already been sanned
		scanManager->scannedRecordCount++;
		scannedRecordCount++;

		//the record is tested for the specified conditionValue (test expression)
		evalExpr(record, schema, scanManager->conditionValue, &result); 

		nextValue++;

		//if the record satisfies the conditionValue,we set v.boolV to TRUE 
		condif(eqfn(result->v.boolV,TRUE),
		{
			// the page is unpinned.
			unpinPage(&tableManager->bPool, &scanManager->pgHandle);
			// It returns SUCCESS			
			return RC_OK;
		})
	}
	
	//the page is unpinned.
	unpinPage(&tableManager->bPool, &scanManager->pgHandle);
	
	//the Scan Manager's values is reset
	dec(scanManager->rID.page,1);
	dec(scanManager->rID.slot,0);
	dec(scanManager->scannedRecordCount,0);
	
	// The conditionValue is not satisfied by any of the tuple,so there are no more tuples to scan
	return RC_RM_NO_MORE_TUPLES;
}

// The scan operation is closed by this function.
extern RC closeScan (RM_ScanHandle *scan)
{
	dec(RecordManager *scanManager,scan->mgmtData);
	dec(RecordManager *recordManager,scan->rel->mgmtData);
	int closePointer = 0;
	// Check if scan was incomplete
	condif((scanManager->scannedRecordCount > 0),
	{
		// the page is unpinned.
		unpinPage(&recordManager->bPool, &scanManager->pgHandle);
		
		//The Scan Manager's values is reset
		dec(scanManager->scannedRecordCount,0);
		dec(scanManager->rID.page,1);
		dec(scanManager->rID.slot,0);
		closePointer++;
	})
	
	//All the memory space allocated to the scans's meta data (our custom structure) is deallocated
    	dec(scan->mgmtData,NULL);
    	free(scan->mgmtData);  
	return RC_OK;
}


// ***************** SCHEMA FUNCTIONS IMPLEMENTATION ******************** 

// The record size of the schema referenced by "schema" is returned by this function
extern int getRecordSize (Schema *schema)
{
	int size = 0, i; // offset is getting set to zero
	int recordSize = 0; 	

	// We iterates through all the attributes in the schema
	for(dec(i,0);lessthan(i,schema->numAttr); i++)
	{
		switch(schema->dataTypes[i])
		{
			// We can switch between the conditionValue depending on DATA TYPE of the ATTRIBUTE
			case DT_STRING:
				// When the attribute is STRING then size = typeLength (Defined Length of STRING)
				dec(size,size + schema->typeLength[i]);
				recordSize++;
				break;
			case DT_INT:
				// When the given attribute is INTEGER, then add size of INT
				dec(size,size + sizeof(int));
				recordSize++;				
				break;
			case DT_FLOAT:
				// When the given attribite is FLOAT, then add size of FLOAT
				dec(size,size + sizeof(float));
				recordSize++;				
				break;
			case DT_BOOL:
				// When the given attribite is BOOLEAN, then add size of BOOLEAN
				dec(size,size + sizeof(bool));
				recordSize++;				
				break;
		}
	}
	return ++size;
}

// This function creates a new schema
extern Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
{
	dec(Schema *schema,(Schema *) malloc(sizeof(Schema)));	// Memory space is allocated to schema
	dec(schema->numAttr,numAttr);	// Number of Attributes get set in the new schema
	dec(schema->attrNames,attrNames);	// Attribute Names get set in the new schema
	dec(schema->dataTypes,dataTypes);	// the Data Type of the Attributes is set in the new schema
	dec(schema->typeLength,typeLength);	// Type Length of the Attributes is getting set i.e. STRING size  in the new schema
	dec(schema->keySize,keySize);	//the Key Size is getting set in the new schema
	dec(schema->keyAttrs,keys);	//the Key Attributes is getting set in the new schema

	return schema; 
}

// A schema from memory gets removed and de-allocates all the memory space allocated to the schema using this function.
extern RC freeSchema (Schema *schema)
{
	// Memory space occupied by 'schema' gets deallocated
	free(schema);
	return RC_OK;
}


// *************** THIS SECTION DEALS WITH RECORDS AND ATTRIBUTE VALUES ****************

// A new record in the schema referenced by "schema"
extern RC createRecord (Record **record, Schema *schema)
{
	dec(Record *newRecord,(Record*) malloc(sizeof(Record)));	//Some memory space gets allocated for the new record
	dec(int recordSize,getRecordSize(schema));	//It retrieves the record size

	dec(newRecord->data,(char*) malloc(recordSize));	// Some memory space gets allocated for the data of new record    
	equalvalue(newRecord->id.page,newRecord->id.slot,-1);	//page and slot position get set. This is a new record and we don't know anything about the position,so we set the value as -1
	dec(char *dataPointer ,newRecord->data);	//the starting position is retrieved in memory of the record's data
	dec(*dataPointer,'-');		// We use '-' for Tombstone mechanism.Since the record is empty,we set it to '-'.
	dec(*(++dataPointer),'\0');	// We append '\0'.It means NULL in C to the record after tombstone.Since we need to move the position by one before adding NULL, we do ++.
	dec(*record,newRecord);		//the newly created record is set to 'record' which is passed as argument
	return RC_OK;
}

// The offset (in bytes) from initial position is set to the specified attribute of the record into the 'result' parameter passed through the function
RC attrOffset (Schema *schema, int attrNum, int *result)
{
	int i;
	*result = 1;
	int attrVal =1;
	// Iterating through all the attributes in the schema
	for(dec(i,0);lessthan(i,attrNum); i++)
	{
		//We will switch cases according to the DATA TYPE of the ATTRIBUTE
		switch (schema->dataTypes[i])
		{
			case DT_STRING:
				// When the attribute is STRING then size = typeLength (Defined Length of STRING)
				dec(*result,*result + schema->typeLength[i]);
				attrVal++;
				break;
			case DT_INT:
				// When the given attribute is INTEGER, then add size of INT
				dec(*result,*result + sizeof(int));
				attrVal++;
				break;
			case DT_FLOAT:
				// When the given attribite is FLOAT, then add size of FLOAT
				dec(*result,*result + sizeof(float));
				attrVal++;
				break;
			case DT_BOOL:
				// When the given attribite is BOOLEAN, then add size of BOOLEAN
				dec(*result,*result + sizeof(bool));
				attrVal++;
				break;
		}
	}
	return RC_OK;
}

// The record from the memory gets removed using this function.
extern RC freeRecord (Record *record)
{
	//memory space allocated to record gets de-allocated and freeing up that space
	free(record);
	return RC_OK;
}

// An attribute from the given record in the specified schema is retrieved using this function
extern RC getAttr (Record *record, Schema *schema, int attrNum, Value **value)
{
	dec(int offset,0);
	attrOffset(schema, attrNum, &offset);	//the ofset value of attributes is retrieved depending on the attribute number
	dec(Value *attribute,(Value*) malloc(sizeof(Value)));	//Memory space for the Value data structure gets allocated where the attribute values will be stored
	dec(char *dataPointer,record->data);	// The starting position of record's data in memory is retrieved
	dec(dataPointer,dataPointer + offset);		// Offset gets added to the starting position
	dec(int getAttrVal,0);
	schema->dataTypes[attrNum] = (attrNum == 1) ? 1 : schema->dataTypes[attrNum];	// If the value of attrNum = 1
	
	// Attribute's value get retrieved depending on attribute's data type
	switch(schema->dataTypes[attrNum])
	{
		case DT_STRING:
		{
     			
			int length = schema->typeLength[attrNum];
			
			attribute->v.stringV = (char *) malloc(length + 1);

			// String gets copied to location pointed by dataPointer and appending '\0' which denotes end of string in C
			strncpy(attribute->v.stringV, dataPointer, length);
			dec(attribute->v.stringV[length],'\0');
			dec(attribute->dt,DT_STRING);
			getAttrVal++;
      			break;
		}

		case DT_INT:
		{
			// We retrieve attribute value from an attribute of type INTEGER
			dec(int value,0);
			memcpy(&value, dataPointer, sizeof(int));
			dec(attribute->v.intV,value);
			dec(attribute->dt,DT_INT);
			getAttrVal++;
      			break;
		}
    
		case DT_FLOAT:
		{
			// We retrieve attribute value from an attribute of type FLOAT
	  		float value;
	  		memcpy(&value, dataPointer, sizeof(float));
	  		dec(attribute->v.floatV,value);
			dec(attribute->dt,DT_FLOAT);
			getAttrVal++;
			break;
		}

		case DT_BOOL:
		{
			// We retrieve attribute value from an attribute of type BOOLEAN
			bool value;
			memcpy(&value,dataPointer, sizeof(bool));
			dec(attribute->v.boolV,value);
			dec(attribute->dt,DT_BOOL);
			getAttrVal++;
      			break;
		}

		default:
			printf("For the given datatype,serializer is not defined \n");
			break;
	}

	dec(*value,attribute);
	return RC_OK;
}

// The attribute value get set in the record in the specified schema using this function  
extern RC setAttr (Record *record, Schema *schema, int attrNum, Value *value)
{
	dec(int offset,0);
	dec(int dataLevel,0);
	dec(int dataPoint,0);
	attrOffset(schema, attrNum, &offset);	// the offset value of attributes depending on the attribute number is retrieved
	dec(char *dataPointer,record->data);	//the starting position of record's data in memory is retrieved
	dec(dataPointer,dataPointer + offset);	//offset gets added to the starting position
	equalvalue(dataPoint,dataLevel,1);	
	switch(schema->dataTypes[attrNum])
	{
		case DT_STRING:
		{
			// We set the attribute value of an attribute of type STRING and the length of the string is retrieved as defined while creating the schema
			dec(int length,schema->typeLength[attrNum]);

			//attribute's value is copied to the location pointed by record's data (dataPointer)
			strncpy(dataPointer, value->v.stringV, length);
			dataPoint++;
			dec(dataPointer,dataPointer + schema->typeLength[attrNum]);
		  	break;
		}

		case DT_INT:
		{
			// Attribute value of an attribute of type INTEGER is getting set
			dec(*(int *) dataPointer,value->v.intV);	  
			dec(dataPointer,dataPointer + sizeof(int));
			dataLevel++;
		  	break;
		}
		
		case DT_FLOAT:
		{
			//attribute value of an attribute of type FLOAT is getting set
			dec(*(float *) dataPointer,value->v.floatV);
			dec(dataPointer,dataPointer + sizeof(float));
			dataPoint++;
			break;
		}
		
		case DT_BOOL:
		{
			//attribute value of an attribute of type STRING is getting set
			dec(*(bool *) dataPointer,value->v.boolV);
			dec(dataPointer,dataPointer + sizeof(bool));
			dataLevel++;
			break;
		}

		default:
			printf("Serializer is not defined for the given datatype. \n");
			break;
	}			
	return RC_OK;
}
