#include "AssertGeometric.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertInt2(int iExpectedX, int iExpectedY, SIntVec2* psActual, int iLine, char* szFile)
{
	if (!PrivateAssertInt(iExpectedX, psActual->x, "SIntVec2.x", iLine, szFile))
	{
		return false;
	}
	if (!PrivateAssertInt(iExpectedY, psActual->y, "SIntVec2.y", iLine, szFile))
	{
		return false;
	}
	return true;
}

