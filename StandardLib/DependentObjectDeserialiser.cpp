#include "ObjectGraphDeserialiser.h"
#include "ObjectHeader.h"
#include "DependentObjectDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentObjectDeserialiser::Init(CObjectGraphDeserialiser* pcGraphDeserialiser, CSerialisedObject* pcSerialised)
{
	mpcGraphDeserialiser = pcGraphDeserialiser;
	CObjectDeserialiser::Init(pcSerialised);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentObjectDeserialiser::Kill(void)
{
	mpcGraphDeserialiser = NULL;
	CObjectDeserialiser::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDependentObjectDeserialiser::ReadPointer(CPointerObject* pObject)
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
	mpcGraphDeserialiser->AddDependent(&cHeader, ppcObjectPtr, (CBaseObject*)pcEmbedding);
	
	//cHeader is killed by mpcGraphDeserialiser.
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDependentObjectDeserialiser::ReadDependent(CBaseObject** ppcObjectPtr, CBaseObject* pcContaining)
{
	CPointerHeader	cHeader;
	BOOL			bResult;

	*ppcObjectPtr = NULL;
	bResult = ReadPointerHeader(&cHeader);
	mpcGraphDeserialiser->AddDependent(&cHeader, ppcObjectPtr, pcContaining);
	
	//cHeader is killed by mpcGraphDeserialiser.
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentObjectDeserialiser::AddIndexRemap(OIndex oiNew, OIndex oiOld)
{
	mpcGraphDeserialiser->AddIndexRemap(oiNew, oiOld);
}

