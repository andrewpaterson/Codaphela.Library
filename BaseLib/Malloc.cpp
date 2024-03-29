#include "IntegerHelper.h"
#include "Logger.h"
#include "Malloc.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMalloc::Init(CMallocator* pcMalloc)
{
	mpcMalloc = pcMalloc;
	if (!mpcMalloc)
	{
		gcLogger.Error2(__METHOD__, " Mallocator is NULL.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMalloc::Kill(void)
{
	mpcMalloc = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMalloc::Malloc(size_t tSize)
{
	return mpcMalloc->Malloc(tSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMalloc::Free(void* pv)
{
	mpcMalloc->Free(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMalloc::Realloc(void* pv, size_t tSize)
{
	return mpcMalloc->Realloc(pv, tSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMalloc::IsMallocInitialised(void)
{
	return FixBool(mpcMalloc);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMalloc::IsMalloc(CMallocator* pcMalloc)
{
	return mpcMalloc == pcMalloc;
}

