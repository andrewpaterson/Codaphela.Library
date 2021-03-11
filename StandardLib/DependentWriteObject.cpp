#include "DependentWriteObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentWriteObject::Init(CBaseObject* pcObject, BOOL bWritten)
{
	mpcObject = pcObject;
	mbWritten = bWritten;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CompareDependentWriteObject(const void* ps1, const void* ps2)
{
	CDependentWriteObject*	psObj1;
	CDependentWriteObject*	psObj2;

	psObj1 = (CDependentWriteObject*)ps1;
	psObj2 = (CDependentWriteObject*)ps2;

	if (psObj1->mpcObject < psObj2->mpcObject)
	{
		return -1;
	}
	else if (psObj1->mpcObject > psObj2->mpcObject)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

