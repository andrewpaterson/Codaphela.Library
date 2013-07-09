#include "ObjectFileGeneral.h"
#include "DependentObjectAdder.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentObjectAdder::Init(CDependentReadObjects*	pcDependentObjects)
{
	mpcDependentObjects = pcDependentObjects;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentObjectAdder::Kill(void)
{
	mpcDependentObjects = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentObjectAdder::AddContainingPointer(CBaseObject* pcBaseObject, CBaseObject** ppcPointedFrom, CBaseObject* pcContaining)
{
	*ppcPointedFrom = pcBaseObject;

	if (pcContaining)
	{
		pcBaseObject->AddFrom(pcContaining);
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDependentObjectAdder::AddDependent(CObjectIdentifier* pcHeader, CBaseObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated, int iEmbeddedIndex)
{
	if ((pcHeader->mcType == OBJECT_POINTER_NAMED) || (pcHeader->mcType == OBJECT_POINTER_ID))
	{
		mpcDependentObjects->Add(pcHeader, ppcPtrToBeUpdated, pcObjectContainingPtrToBeUpdated, iEmbeddedIndex);
	}
	return TRUE;
}
