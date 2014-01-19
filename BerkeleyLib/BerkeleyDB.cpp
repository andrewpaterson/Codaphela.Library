#include "BerkeleyDB.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBerkeleyDB::Init(void)
{
	mpDb = new Db(NULL, 0);               // Instantiate the Db object

	// Open the database
	mpDb->open(NULL,                // Transaction pointer 
		"my_db.db",          // Database file name 
		NULL,                // Optional logical database name
		DB_BTREE,            // Database access method
		DB_CREATE,              // Open flags
		0);                  // File mode (using defaults)
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBerkeleyDB::Kill(void)
{
	mpDb->close(0);
}

