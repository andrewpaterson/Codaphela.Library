#include "Pointer.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool FailValidPointerClassAllowNull(char* szMethod, char* szParameter, const char* szInstanceClassName, const char* szExpectedClassName)
{
	return gcLogger.Error2(szMethod, " Expected ", szParameter, " to be [Ptr<", szExpectedClassName, ">] but got [Ptr<", szInstanceClassName, ">] instead.", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsValidPointerClassAllowNull(char* szParameter, const char* szInstanceClassName, const char* szExpectedClassName)
{
	//AllowNull is on the szInstanceClassName.
	//Not sure why szExpectedClassName is checked.

	if (szExpectedClassName && szInstanceClassName && StringCompare(szExpectedClassName, szInstanceClassName) != 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

