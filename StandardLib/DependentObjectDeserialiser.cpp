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

	ClearPointer(pObject);
	bResult = ReadPointerHeader(&cHeader);
	if (!bResult)
	{
		cHeader.Kill();
		return FALSE;
	}
	ppcObjectPtr = pObject->ObjectPtr();
	mpcGraphDeserialiser->AddDependent(&cHeader, ppcObjectPtr);
	
	//cHeader is killed by mpcGraphDeserialiser.
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDependentObjectDeserialiser::ReadDependent(CBaseObject** ppcObjectPtr)
{
	CPointerHeader	cHeader;
	BOOL			bResult;

	*ppcObjectPtr = NULL;
	bResult = ReadPointerHeader(&cHeader);
	mpcGraphDeserialiser->AddDependent(&cHeader, ppcObjectPtr);
	
	//cHeader is killed by mpcGraphDeserialiser.
	return bResult;
}

