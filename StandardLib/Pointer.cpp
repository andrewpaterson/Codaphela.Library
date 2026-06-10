#include "Pointer.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ValidatePointerClassAllowNull(char* szMethod, char* szParameter, const char* szInstanceClassName, const char* szExpectedClassName)
{
	//AllowNull is on the szInstanceClassName.
	//Not sure why szExpectedClassName is checked.

	if (szExpectedClassName && szInstanceClassName && StringCompare(szExpectedClassName, szInstanceClassName) != 0)
	{
		return gcLogger.Error2(szMethod, " Expected ", szParameter, " to be [Ptr<", szExpectedClassName, ">] but got [Ptr<", szInstanceClassName, ">] instead.", NULL);
	}
	else
	{
		return true;
	}
}

