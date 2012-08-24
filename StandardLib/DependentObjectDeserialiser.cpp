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

	ClearPointer(pObject);
	bResult = ReadPointerHeader(&cHeader);
	mpcGraphDeserialiser->DoShitWith(&cHeader);
	cHeader.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDependentObjectDeserialiser::ReadDependent(CUnknown** ppcUnknown)
{
	CPointerHeader	cHeader;
	BOOL			bResult;

	*ppcUnknown = NULL;
	bResult = ReadPointerHeader(&cHeader);
	mpcGraphDeserialiser->DoShitWith(&cHeader);
	cHeader.Kill();
	return bResult;
}