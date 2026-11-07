#include "Pointer.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool FailValidPointerClassAllowNull(const char* szMethod, const char* szParameter, const char* szInstanceClassName, const char* szExpectedClassName)
{
	return gcLogger.Error2(szMethod, " Expected ", szParameter, " to be [Ptr<", szExpectedClassName, ">] but got [Ptr<", szInstanceClassName, ">] instead.", NULL);
}



