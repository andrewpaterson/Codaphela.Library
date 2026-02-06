#include "Pointer.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ValidatePointerClass(char* szMethod, char* szParameter, const char* szInstanceClassName, const char* szExpectedClassName)
{
	if (szExpectedClassName && strcmp(szExpectedClassName, szInstanceClassName) != 0)
	{
		return gcLogger.Error2(szMethod, " Expected ", szParameter, " to be [Ptr<", szExpectedClassName, ">] but got [Ptr<", szInstanceClassName, ">] instead.", NULL);
	}
	else
	{
		return true;
	}
}

