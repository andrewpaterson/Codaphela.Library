#include "BaseLib/Logger.h"
#include "BaseLib/ErrorTypes.h"
#include "Objects.h"
#include "ObjectFileGeneral.h"
#include "DependentReadObjects.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadObjects::Init(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadObjects::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Points a pointer that was read (*ppcPtrToBeUpdated, in pcContaining) at
// pcObject's embedded object iEmbeddedIndex: the object itself for 0, or
// if it's hollow one of its embedded hollows.
//
//////////////////////////////////////////////////////////////////////////
bool CDependentReadObjects::PointTo(CBaseObject* pcObject, uint16 iEmbeddedIndex, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcContaining)
{
	CEmbeddedObject*	pcEmbeddedObject;

	pcEmbeddedObject = pcObject->GetEmbeddedObject(iEmbeddedIndex);
	if (pcEmbeddedObject == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Object {", ObjectToString(pcObject), "} has no embedded object [", IntToString(iEmbeddedIndex), "].", NULL);
	}

	*ppcPtrToBeUpdated = pcEmbeddedObject;
	if (pcContaining)
	{
		pcEmbeddedObject->AddHeapFrom(pcContaining, false);
	}
	return true;
}

