#include "Logger.h"
#include "GlobalDataTypesIO.h"


CDataTypesIO			gcDataTypesIO;
BOOL					gbDataTypesIO = FALSE;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DataIOInit(void)
{
	if (gbDataTypesIO)
	{
		gcLogger.Error("Global Data Types IO has already been initialised.");
	}

	gcDataTypesIO.Init();
	gbDataTypesIO = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DataIOKill(void)
{
	if (!gbDataTypesIO)
	{
		gcLogger.Error("Global Data Types IO has already been Killed.");
	}

	gcDataTypesIO.Kill();
	gbDataTypesIO = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL DataIOValidate(void)
{
	if (!gbDataTypesIO)
	{
		gcLogger.Error("Global Data Types IO has not been initialised.  CallMemoryInit.");
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


