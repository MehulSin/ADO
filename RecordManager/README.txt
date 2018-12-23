Assignment 1 - Record manager
Submitted by Group No. 7 (msingh36@hawk.iit.edu, kshankar1@hawk.iit.edu ,rkothakapu@hawk.iit.edu)

RUNNING THE SCRIPT
=======================================

1) Using terminal,go at Project root (assign3) directory,then execute command ls to list the files and verify the correct location of directory.

3) Execute "make clean" command to remove old compiled .o files.

4) Execute "make" to compile project files.It will comple all the project files including "test_assign3_1.c" file. 

5) Execute "make run" command to compile "test+assign3_1.c".

6) For compiling test expression and related files, execute "make test_expr".It will compile all file including "test_expr.c"

7) Run "test_expr.c" file by "make run_expr".


SOLUTION DESCRIPTION
=======================================
By minimizing the use of local variables and freeing reserved space wherever possible,we have ensured proper memory management.
We have also implemented Tombstone mechanism in addition to the functions required as specified in record_mgr.h file. 

1. TABLE AND RECORD MANAGER FUNCTIONS
=======================================

Below functions are written for handling record manager (initialization and shutting down).We have used table related functions for creating,opening, closing and deleting a table
For accessing pages using Buffer Pool, we make use Buffer Manager(Assignment 2).Also,for performing operations on page file on disk, we use code developed in Assigment 1 for Storage Manager indirectly.

initRecordManager (...)
- Initializes recordManager.
- For initializing the storage manager,we call initStorageManager(...) function of storage manager.

shutdownRecordManager(...)
- Shuts down the record manager.
- Free all the resources/memory allocated to the record manager.
- All the resources/memory space that is being used by the Record Manager gets free after executing this function.
-RecordManager data structure pointer is set to NULL and free() function of C file is called to deallocate memory space.
 

Below function are for creating, opening, closing, and deleting a table:

createTable(...)
- Input argument is the name of the table that is to be opened by this function. 
- This opens the table specified by the argument.
- Buffer Pool is initialized by calling initBufferPool(...). Implements LRU page replacment policy.
- Initialize table values
- Sets the attributes such as name, datatype, and size.
- Also creates a page file, uses the the page file to write the block containing the table and properly closes the page file.

openTable(...)
- Input argument is the name of the table.
- Opens the table as specified in the parameter using the metadata provided in schema.

closeTable(...)
- Input argument is the name of the table.
- Using shutdownBufferPool(...) function in the Buffer Manager, this function closes the table.
- Changes are written in the page file prior to closing down the buffer pool.

deleteTable(...)
- Input argument is the name of the table.
- Internally uses function destroyPageFile(...) from Storage Manager Assignment.
- Using this function we can delete a table by providing table name as input argument.


2. RECORD FUNCTIONS
=======================================

Following functions are created for retrieving, deleting, inserting, and updating (CRUD operations) a record.

insertRecord(...)
- Input argument is the Record ID of the record.
- insertRecord(...) is used for inserting a new record into the table and updates the record paramenter.
- After selecting the page which has space sufficient for the record, we pin the page. Then, we identify the data pointer and add a '+' to indicate record is new.
- Page is marked as updated which triggers Buffer Manager to write the content back to the disk.
- Using memcpy() function, we copy the record's data into the new record.
- Pinned page is then unpinned.

deleteRecord(...)
- Input argument is the Record ID of the record.
- Page ID of the page containing the record is tracked in table's meta-data so that it is made aware of free space which can eb used for a new record.
- As with the insert, we pin the page and locate the data pointer; however, update the first char to '-' instead of '+' with the insert. '-' indicates that record is no longer available.
- Page is marked as updated which triggers Buffer Manager to write the content back.

updateRecord(...)
- Table is referenced by parameter "rel" and using this function we update the record.
- Identify the page where the record is positioned using table's meta-data. Page is then pinned in the buffer pool.
- It sets the Record ID, navigates to the location where the record's data is stored.
- Using memcpy() function, we copy the record's data into the new record. 
- Page is marked as updated which triggers Buffer Manager to write the content back.
- Page is then unpinned.

getRecord(....)
- This function retrieves a record having Record ID "id" passed in the paramater in the table referenced by "rel" which is also passed in the parameter. The result record is stored in the location referenced by the parameter "record".
- It finds the page where the record is located by table's meta-data and using the 'id' of the record, it pins that page in the buffer pool.
- It sets the Record ID of the 'record' parameter with the id of the record which exists in the page and copies the data too.
- It then unpins the page.


3. SCAN FUNCTIONS
=======================================

All tuples from a table are retrieved using scan function.Certain condition (represented as an Expr) should be fulfilled by tuple.When a scan is started,the RM_ScanHandle data structure get initialized and is passed as an argument to the function startScan. Then ,the next method is called and it returns the next tuple that fulfills the scan condition. If NULL is passed as a scan condition, it returns RC_SCAN_CONDITION_NOT_FOUND .Then RC_RM_NO_MORE_TUPLES is returned after completeing the scan and RC_OK otherwise (unless an error occurs).

startScan(...)
- In this function,a scan is started using data that RM_ScanHandle data structure gives us.startScan() function gets RM_ScanHandle data structure as an argument.
- Custom data structure's scan related variables then get initialized.
- We return error code RC_SCAN_CONDITION_NOT_FOUND,if condition is NULL. 

next(...)
- The next tuple which satisfies the condition (test expression) is returned by this function.
- We return error code RC_SCAN_CONDITION_NOT_FOUND,if condition iS NULL. 
- We return error code RC_RM_NO_MORE_TUPLES when there are no tuples in the table.
- Then iteration through the tuples in the table is done.Then we pin the page that contains the given tuple and navigate to the location where data is stored.Data is copied into a temporary buffer.Finally we evaluate the given test expression by calling eval(....)
- If we get the result(v.boolV) of test expression as TRUE,then it means the tuple that we obtained fulfilled the given condition.THe page then get unpinned.We return RC_OK.
- If the condition is not fulfilled by any of the tuple, then error code RC_RM_NO_MORE_TUPLES is returned.

closeScan(...) 
- The scan operation is closed by this operation.
- In this function,we check the condition whether the scan was complete or not by checking the value of scanCount of the table's metadata.The scan was incomplete,if we get the scanCount value greater than 0.
- We unpin the page  if the scan was incomplete, and all scan mechanism related variables get reset in our table's meta-data (custom data structure).
- Then the space occupied by the metadata get free.

4. SCHEMA FUNCTIONS
=========================================

In this functions we return the size in bytes of records for a given schema.New schema is then created. 

getRecordSize(...)
- The size of a record in the specified schema is returned by this function.
- Then iteration through the attributes of the schema is done. We iteratively add the size (space in bytes) required by each attribute to the variable 'size'. 
- The value of the variable 'size' is the size of the record.

freeSchema(...)
- The schema specified by the parameter 'schema' from the memory get removed by this function .
- This purpose is served by the variable (field) refNum in each page frame.A count of the page frames being accessed by the client is kept by refNum.
- C function free(...) is used by us to de-allocate the memory space occupied by the schema.Hence,it get removed from the memory.

createSchema(...)
- A new schema with the specified parameters in memory is created by this function.
- The number of parameters is specified by numAttr. The name of the attributes is specified by attrNames.The datatype of the attributes is specified by datatypes.The length of the attribute (example: length of STRING) is specified by  typeLength.
-  A schema object is created and memory space gets allocated to the object. Finally the schema's parameters is set to the parameters passed in the createSchema(...).


5. ATTRIBUTE FUNCTIONS
=========================================

The attribute values of a record get or set by this function and a new record for a given schema is created.Enough memory should get allocated to the data field after a new record is created to hold the binary representations for all attributes of this record as determined by the schema.  

createRecord(...)
- A new record is created by this function in the schema passed by parameter 'schema' and it is passed to the 'record' paramater of the createRecord() function.
- Proper memory space gets allocated to the new record. Also Memory space is given for the data of the record which is the record size.
- Also, '-' is added to the first position and '\0' is appended which represents NULL in C.A new blank record is denoted by '-'.
- Finally,this new record gets assign to the 'record' passed through the parameter.

attrOffset(...)
- The offset (in bytes) get set by this function from initial position to the specified attribute of the record into the 'result' parameter passed through the function.
- Iteration is done through the attributes of the schema untill the specified attribute number is reached. The size (space in bytes) required by each attribute is added iteratively to the pointer *result. 

freeRecord(...)
-The memory space allocated to the 'record' passed through the parameter gets deallocated using this function.
-The C function free() is used to de-allocate the memory space (free up space) that was used by the record.

getAttr(...)
- An attribute get retrieved from the function from the given record in the specified schema.
- We pass through parameter the record,schema and attribute number of record whose attribute is to be retrieved.We store attribute details back to the location that is referenced by 'value'.'value' is passed through the parameter.
- Using the attrOffset(...) function,we go to the location of the attribute. Then depending on the datatype of the attribute,we copy the attribute's datatype and value to the '*value' parameter.

setAttr(...)
- This function sets the attribute value in the record in the specified schema. The record, schema and attribute number whose data is to be retrieved is passed through the parameter.
- We use 'value' parameter to pass the data that is to be stored in the attribute.
- Using the attrOffset(...) function,we go to the location of the attribute. Then depending on the datatype of the attribute, the data gets copied in the '*value' parameter to the attributes datatype and value both.
