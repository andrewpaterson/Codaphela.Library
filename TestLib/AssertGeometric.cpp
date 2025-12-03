#include "AssertGeometric.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertInt2(int iExpectedX, int iExpectedY, SInt2* psActual, int iLine, char* szFile)
{
	if (!PrivateAssertInt(iExpectedX, psActual->x, "SInt2.x", iLine, szFile))
	{
		return false;
	}
	if (!PrivateAssertInt(iExpectedY, psActual->y, "SInt2.y", iLine, szFile))
	{
		return false;
	}
	return true;
}

