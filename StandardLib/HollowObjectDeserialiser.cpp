#include "ObjectHeader.h"
#include "HollowObjectDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObjectDeserialiser::Init(CSerialisedObject* pcSerialised)
{
	CObjectDeserialiser::Init(pcSerialised);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObjectDeserialiser::Kill(void)
{
	CObjectDeserialiser::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CHollowObjectDeserialiser::ReadPointer(CPointerObject* pObject)
{
	CPointerHeader	cHeader;
	BOOL			bResult;
	CBaseObject**	ppcObjectPtr;
	CObject*		pcEmbedding;

	ClearPointer(pObject);
	bResult = ReadPointerHeader(&cHeader);
	if (!bResult)
	{
		cHeader.Kill();
		return FALSE;
	}
	ppcObjectPtr = pObject->ObjectPtr();
	pcEmbedding = pObject->Embedding();

	AddHollowObject(&cHeader, ppcObjectPtr, (CBaseObject*)pcEmbedding);

	cHeader.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CHollowObjectDeserialiser::ReadDependent(CBaseObject** ppcObjectPtr, CBaseObject* pcContaining)
{
	CPointerHeader	cHeader;
	BOOL			bResult;

	*ppcObjectPtr = NULL;
	bResult = ReadPointerHeader(&cHeader);

	AddHollowObject(&cHeader, ppcObjectPtr, (CBaseObject*)pcContaining);

	cHeader.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObjectDeserialiser::AddHollowObject(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr, CBaseObject* pcContaining)
{
}

