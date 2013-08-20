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
void CDependentObjectAdder::AddContainingPointer(CEmbeddedObject* pcBaseObject, CEmbeddedObject** ppcPointedFrom, CBaseObject* pcContaining)
{
	*ppcPointedFrom = pcBaseObject;

	if (pcContaining)
	{
		pcBaseObject->AddHeapFrom(pcContaining);
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDependentObjectAdder::AddDependent(CObjectIdentifier* pcHeader, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated, unsigned short iNumEmbedded, unsigned short iEmbeddedIndex)
{
	if ((pcHeader->mcType == OBJECT_POINTER_NAMED) || (pcHeader->mcType == OBJECT_POINTER_ID))
	{
		mpcDependentObjects->Add(pcHeader, ppcPtrToBeUpdated, pcObjectContainingPtrToBeUpdated, iNumEmbedded, iEmbeddedIndex);
	}
	return TRUE;
}
