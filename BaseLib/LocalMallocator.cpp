#include "LocalMallocator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLocalMallocator::IsLocal(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CLocalMallocator::ShortName(void)
{
	return ClassName();
}

