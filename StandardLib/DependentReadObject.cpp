#include "ObjectFileGeneral.h"
#include "DependentReadObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadObject::Init(CPointerHeader* pcObjectPtr)
{
	CPointerHeader*	pcThis;

	pcThis = this;
	memcpy_fast(pcThis, pcObjectPtr, sizeof(CPointerHeader));
	mbRead = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadObject::Kill(void)
{
	CPointerHeader::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDependentReadObject::IsNamed(void)
{
	return mcType == OBJECT_POINTER_NAMED;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CDependentReadObject::GetName(void)
{
	return mszObjectName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CDependentReadObject::GetIndex(void)
{
	return moi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CompareDependentReadObject(const void* ps1, const void* ps2)
{
	CDependentReadObject*	psObj1;
	CDependentReadObject*	psObj2;

	psObj1 = (CDependentReadObject*)ps1;
	psObj2 = (CDependentReadObject*)ps2;

	if (psObj1->moi < psObj2->moi)
	{
		return -1;
	}
	else if (psObj1->moi > psObj2->moi)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
