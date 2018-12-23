#include "tables.h"
#include "storage_mgr.h"
#include "record_mgr.h"
#include <stdlib.h>
#include <string.h>


//Record Mgr Global variable declaration 
SM_FileHandle fh_rec_mgr;
int tombStonedRIDsList[10000];//The biggest numInserts Value is 10000
dec(int currentScannedRecord,0);


// This function initializes the record manager
 RC initRecordManager (void *mgmtData)
{
	dec(int i,0);
	for(;lessthan(i,10000);i++)
		dec(tombStonedRIDsList[i],-99);
    return RC_OK;
}


// This function shutdowns the record manager
RC shutdownRecordManager ()
{
    return RC_OK;
}


// This function creates a table.Underlying page file is then created.Then information about the schema, free-space and so on get stored in the table information pages
 
RC createTable (char *name, Schema *schema)
{
    SM_PageHandle ph;
    
    dec(ph,((SM_PageHandle) malloc(PAGE_SIZE)));
    
    createPageFile (name);                      //Create a new file with file name as table name.
    openPageFile (name, &fh_rec_mgr);           //Open the file.

    memset(ph,'\0',PAGE_SIZE);
    strcpy(ph,serializeSchema(schema)); 
    
    writeBlock (0, &fh_rec_mgr, ph);

    memset(ph,'\0',PAGE_SIZE);
    writeBlock (1, &fh_rec_mgr, ph);

    free(ph);
    
    return RC_OK;
}


// This function opens a table
 
RC openTable (RM_TableData *rel, char *name)
{
    dec(rel->schema,((Schema *)malloc(sizeof(Schema))));
    dec(rel->name,((char *)malloc(sizeof(char)*100)));
    SM_PageHandle ph;
    
    dec(ph,((SM_PageHandle) malloc(PAGE_SIZE)));

    condif((eqfn(fh_rec_mgr.fileName,NULL)),
    	openPageFile(rel->name,&fh_rec_mgr);)

    strcpy(rel->name,name);
    readBlock(0, &fh_rec_mgr, ph);
    deSerializeSchema(ph, rel->schema);
    rel->mgmtData = NULL; 
    
    free(ph);

    condif((notequal(rel->schema,NULL) && notequal(rel,NULL)),
        return RC_OK;)
    else
    	return RC_OPEN_TABLE_FAILED;

    closePageFile(&fh_rec_mgr);

    return RC_OK;
}



//This function closes a table.
RC closeTable (RM_TableData *rel)
{
    	char freePagesList[PAGE_SIZE];
	memset(freePagesList,'\0',PAGE_SIZE);

	dec(SM_PageHandle ph,((SM_PageHandle)malloc(PAGE_SIZE)));
	readBlock(1,&fh_rec_mgr,ph);

	strcpy(freePagesList,ph);

	dec(int x,0);

	char FreePageNumber[10];
	memset(FreePageNumber,'\0',10);

	char nullString[PAGE_SIZE];
	memset(nullString,'\0',PAGE_SIZE);
	for(;notequal(tombStonedRIDsList[x],-99) ;x++)
	{
		sprintf(FreePageNumber,"%d",(tombStonedRIDsList[x]));
		strcat(freePagesList,FreePageNumber);
		strcat(freePagesList,";");

		writeBlock(1,&fh_rec_mgr,freePagesList);// After this call ,FSM block will get updated
		writeBlock(tombStonedRIDsList[x]+2,&fh_rec_mgr,nullString);//deleted block get written

		memset(FreePageNumber,'\0',10);
	}

    closePageFile(&fh_rec_mgr);

    free(rel->schema);
    free(rel->name);
    return RC_OK;
}



// This function deletes the table
RC deleteTable (char *name)
{
    destroyPageFile(name);
    return RC_OK;
}


// This function returns the number of tuples in the table
 
int getNumTuples (RM_TableData *rel)
{
   dec(int freePageCount,0);
   dec(int i,0);
    
    SM_PageHandle ph;
    
    dec(ph,((SM_PageHandle) malloc(PAGE_SIZE)));
    
    readBlock(1, &fh_rec_mgr, ph);
    
    for (;(ph[i]!=NULL); i++) {
        condif((eqfn(ph[i],';')),
            freePageCount ++;)
    }
    
    closePageFile(&fh_rec_mgr);
    openPageFile(rel->name, &fh_rec_mgr);
    return (fh_rec_mgr.totalNumPages - freePageCount);
}



// This function is used for inserting a new record.RID will be assigned to the newly inserted record.Then the record parameter passed is updated.
 

RC insertRecord (RM_TableData *rel, Record *record)
{
	condif((eqfn(fh_rec_mgr.fileName,NULL)),
	{
		openPageFile(rel->name,&fh_rec_mgr);
	})

	int recordId;
    dec(char *stringToBeWritten,((char *)malloc(sizeof(char) *PAGE_SIZE)));
    memset(stringToBeWritten,'\0',PAGE_SIZE);
    strcpy(stringToBeWritten,record->data);

    char freePagesList[PAGE_SIZE];
	memset(freePagesList,'\0',PAGE_SIZE);

	SM_PageHandle ph;
	dec(ph,(SM_PageHandle) malloc(PAGE_SIZE));
	readBlock(1, &fh_rec_mgr, ph);
	strcpy(freePagesList,ph); //read the 1st block content to fpl here

	free(ph);
	dec(char areFreePagesPresent,'N');
	int x;
	condif((notequal(freePagesList,NULL)),
	{
		for(dec(x,0); lessthan(x,strlen(freePagesList));x++)
		{
			condif((eqfn(freePagesList[x],';')),
			{
				dec(areFreePagesPresent,'Y');
				break;
			})
		}
	})
	condif((eqfn(areFreePagesPresent,'Y')),
	{
		char firstFreePageNumber[10];
		memset(firstFreePageNumber,'\0',10);
		for(dec(x,0);notequal(freePagesList[x],';');x++)
		{
			dec(firstFreePageNumber[x],freePagesList[x]);
		}

		writeBlock(atoi(firstFreePageNumber) + 2,&fh_rec_mgr,stringToBeWritten);
		fh_rec_mgr.totalNumPages--;
		dec(int i,0);
		dec(char *newFreePagesList,((SM_PageHandle) malloc(PAGE_SIZE)));
		memset(newFreePagesList,'\0',PAGE_SIZE);
		for(dec(x,(strlen(firstFreePageNumber) + 1)) ;lessthan(x,strlen(freePagesList));x++)
		{
			dec(newFreePagesList[i++],freePagesList[x]);
		}
		writeBlock(1,&fh_rec_mgr,newFreePagesList);
		free(newFreePagesList);
		dec(recordId,atoi(firstFreePageNumber));
	})
	else
	{
		writeBlock(fh_rec_mgr.totalNumPages,&fh_rec_mgr,stringToBeWritten);
		dec(recordId,fh_rec_mgr.totalNumPages - 3);
	}

	dec(record->id.page,recordId);
	dec(record->id.slot,-99); 
  	free(stringToBeWritten);
    	return RC_OK;
}



// This function deletes the record having RID that is passed as an parameter
RC deleteRecord (RM_TableData *rel, RID id)
{
	dec(int i,0) ;
	whileloop((notequal(tombStonedRIDsList[i],-99)),
		i++;)
	dec(tombStonedRIDsList[i],id.page);

    return RC_OK;
}


// This function updates an existing record.THe record get updated with new values
 
RC updateRecord (RM_TableData *rel, Record *record)
{
	openPageFile (rel->name, &fh_rec_mgr);
	writeBlock(record->id.page + 2,&fh_rec_mgr,record->data);
	closePageFile(&fh_rec_mgr);
        return RC_OK;
}


//This function retrieves a record.THe RID that is passed as an parameter is used for retrieving the record
 
RC getRecord (RM_TableData *rel, RID id, Record *record)
{

    SM_PageHandle ph;

    SM_FileHandle fh;

    openPageFile (rel->name, &fh);

    dec(ph,((SM_PageHandle) malloc(PAGE_SIZE)));

    memset(ph,'\0',PAGE_SIZE);

    readBlock(id.page + 2, &fh, ph);

    dec(record->id.page,id.page);

    strcpy(record->data, ph);
    closePageFile(&fh);
    free(ph);

    return RC_OK;
}



// This function initializes the RM_ScanHandle data structure.This data structure is passed as an argument along with it.
 
RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)
{
	dec(scan->rel,rel);
	dec(scan->mgmtData,cond);
    	return RC_OK;
}


// This function stores the position of semicolon. 
void storeSemiColonPostion(char * record,int *p)
{
	int i;
	dec(int j,0);
	for(dec(i,0) ;lessthan(i,strlen(record)); i++)
		condif((eqfn(record[i],';')),
			dec(p[j++],i);)
}


// This function gets the value of columndata.
 
void getColumnData(int columnNum,char * record,int *semiColonPosition,char * cellValue)
{
	int cellStartIndex;

	condif((eqfn(columnNum,0)),
		dec(cellStartIndex,0);)
	else
		dec(cellStartIndex,semiColonPosition[columnNum - 1] + 1);

	int i;
	dec(int j,0);
	for(dec(i,cellStartIndex) ;lessthan(i,semiColonPosition[columnNum]);i++)
		dec(cellValue[j++],record[i]);

}



/* This function returns the next tuple.This tuple should fulfill the scan condition.
if the scan condition passed is NULL,then we return all tuples of the table.
After the scan condition is completed for the first time,RC_RM_NO_MORE_TUPLES is returned and RC_OK otherwise
unless an error occurs*/

RC next (RM_ScanHandle *scan, Record *record)
{
	condif((eqfn(fh_rec_mgr.fileName,NULL)),
		openPageFile(scan->rel->name,&fh_rec_mgr);)

	dec(Expr *expression,(Expr *) scan->mgmtData);
	whileloop((lessthan(currentScannedRecord,fh_rec_mgr.totalNumPages - 2)),
	{
		dec(SM_PageHandle ph,(SM_PageHandle) malloc(PAGE_SIZE));
		memset(ph,'\0',PAGE_SIZE);

		readBlock(currentScannedRecord+2,&fh_rec_mgr,ph);

		char cellValue[PAGE_SIZE];
		memset(cellValue,'\0',PAGE_SIZE);

		int semiColonPosition[3];  
		storeSemiColonPostion(ph,semiColonPosition);

		condif((eqfn(expression->expr.op->type,OP_COMP_EQUAL)),
		{
			getColumnData(expression->expr.op->args[1]->expr.attrRef,ph,semiColonPosition,cellValue);
			condif((eqfn(expression->expr.op->args[0]->expr.cons->dt,DT_INT)),
			{
				condif((eqfn(atoi(cellValue),expression->expr.op->args[0]->expr.cons->v.intV)),
				{
					strcpy(record->data,ph);
					currentScannedRecord++;
					free(ph);
					return RC_OK;
				})
			})
			elif((eqfn(expression->expr.op->args[0]->expr.cons->dt,DT_STRING)),
			{
				condif((eqfn(strcmp(cellValue,expression->expr.op->args[0]->expr.cons->v.stringV),0)),
				{
					strcpy(record->data,ph);
					currentScannedRecord++;
					free(ph);
					return RC_OK;
				})
			})
		})
		elif((eqfn(expression->expr.op->type,OP_BOOL_NOT)),
		{
			getColumnData(expression->expr.op->args[0]->expr.op->args[0]->expr.attrRef,ph,semiColonPosition,cellValue);
			condif((lessequalvalue(expression->expr.op->args[0]->expr.op[0].args[1]->expr.cons->v.intV,atoi(cellValue))),
			{
				strcpy(record->data,ph);
				currentScannedRecord++;
				free(ph);
				return RC_OK;
			})
		})
		currentScannedRecord++;
		free(ph);
	})
	dec(currentScannedRecord,0); // This global variable get reset for next scan
	return RC_RM_NO_MORE_TUPLES;
}


//This function indicates that all associated resources can get cleaned up by record manager
 
RC closeScan (RM_ScanHandle *scan)
{
    return RC_OK;
}


// This function returns the size of record  in bytes for a given schema

int getRecordSize (Schema *schema)
{
    dec(int size,0);
    int i;
    for (dec(i,0);lessthan(i,schema->numAttr); i ++) {
        switch(schema->dataTypes[i]) {
            case DT_INT:
                plusequal(size,sizeof(int));
                break;
            case DT_FLOAT:
                plusequal(size,sizeof(float));
                break;
            case DT_BOOL:
                plusequal(size,sizeof(bool));
                break;
            case DT_STRING:
                plusequal(size,schema->typeLength[i]);
                break;
        }
    }
    dec(size,8);
    return size;
}



// A new schema is created by this function
Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
{
	dec(Schema *schema,(Schema *)malloc(sizeof(Schema)));
	dec(schema->numAttr,numAttr);
	dec(schema->attrNames,attrNames);
	dec(schema->dataTypes,dataTypes);
	dec(schema->typeLength,typeLength);
	dec(schema->keyAttrs,keys);
	dec(schema->keySize,keySize);

	return schema;
}



//This function is used to free the schema
 
RC freeSchema (Schema *schema)
{
    free(schema);
    return RC_OK;
}



/*  For a given schema,this function creates a new record.
Enough memory gets allocated to the data field.It hold the binary representation for all attributes
of this record.*/

RC createRecord (Record **record, Schema *schema)
{
	dec(*record,((Record *)malloc(sizeof(Record))));
	dec((*record)->data,((char *)malloc(PAGE_SIZE)));
	memset((*record)->data,'\0',PAGE_SIZE);

	 condif((notequal(record,NULL)),
		 return RC_OK;)
	 else
		 return RC_CREATE_FAILED;
}


//A specified record gets free using this function
 
RC freeRecord (Record *record)
{
    free(record);
    return RC_OK;
}


// This function is used to get the attribute values of a record
 
RC getAttr (Record *record, Schema *schema, int attrNum, Value **value)
{
    char temp[PAGE_SIZE + 1];
    dec(int tempCounter,1);
    dec(int dataTypeCounter,0);

    memset(temp,'\0',PAGE_SIZE + 1);

    dec(*value,((Value *)malloc(sizeof(Value) * schema->numAttr)));
    
    int i;
    for (dec(i,0); lessthan(i,PAGE_SIZE); i ++)
    {
        condif (((eqfn(record->data[i],';')) || (eqfn(record->data[i],'\0'))),
        {
            condif((eqfn(attrNum,dataTypeCounter)), {

                switch (schema->dataTypes[dataTypeCounter]) {
                    case DT_INT:
                        dec(temp[0],'i');
                        break;
                    case DT_FLOAT:
                        dec(temp[0],'f');
                        break;
                    case DT_BOOL:
                        dec(temp[0],'b');
                        break;
                    case DT_STRING:
                        dec(temp[0],'s');
                        break;
                }

                dec(*value,stringToValue(temp));
                break;
            })
            dataTypeCounter ++;
            dec(tempCounter,1);
            memset(temp,'\0',PAGE_SIZE + 1);
        })
        else {
            dec(temp[tempCounter ++],record->data[i]);
        }
    }
    return RC_OK;
}



/* Description:It sets the attribute values of a record
 */
RC setAttr (Record *record, Schema *schema, int attrNum, Value *value)
{
	 dec(char * serializedColumnValue,serializeValue(value));

	 dec(int noOfSemiColons,0);
	 int i;
	 for(dec(i,0); lessthan(i,strlen(record->data)) ; i++)
		 condif((eqfn(record->data[i],';')),
			 noOfSemiColons++;)

	 condif((noOfSemiColons == schema->numAttr),
	 {
		 int semiColonPosition[schema->numAttr];

		 dec(int j,0);
		 for(dec(i,0); lessthan(i,strlen(record->data)) ; i++)
			 condif((eqfn(record->data[i],';')),
				 dec(semiColonPosition[j++],i);)

		 condif((eqfn( attrNum,0)),
		 {
			 char endString[PAGE_SIZE];
			 memset(endString,'\0',PAGE_SIZE);
			 dec(i,0);
			 int j;
			 for(dec(j,semiColonPosition[attrNum]) ; lessthan(j,strlen(record->data)) ; j++ )
			 {
				 dec(endString[i++],record->data[j]);
			 }
			 dec(endString[i],'\0');

			 memset(record->data,'\0',PAGE_SIZE);

			 strcpy(record->data,serializedColumnValue);
			 strcpy(record->data,endString);
		 })
		 else
		 {
			 char firstString[PAGE_SIZE];
			 char endString[PAGE_SIZE];

			 memset(firstString,'\0',PAGE_SIZE);
			 memset(endString,'\0',PAGE_SIZE);

			 for(dec(i,0) ;lessequalvalue(i,semiColonPosition[attrNum - 1]); i++)
			 {
				 dec(firstString[i],record->data[i]);
			 }
			 dec(firstString[i],'\0');

			 dec(int j,0);
			 for(dec(i,semiColonPosition[attrNum]) ;lessthan(i,strlen(record->data)); i++)
			 {
				 dec(endString[j++],record->data[i]);
			 }
			 dec(endString[j],'\0');

			 strcat(firstString,serializedColumnValue);
			 strcat(firstString,endString);

			 memset(record->data,'\0',PAGE_SIZE);

			 strcpy(record->data,firstString);
		 }
	 })
	 else
	 {
		 strcat(record->data,serializedColumnValue);
		 strcat(record->data,";");
	 }

	 condif((notequal(serializedColumnValue,NULL)),
		 return RC_OK;)
	 else
		 return RC_SET_ATTR_FAILED;
}

