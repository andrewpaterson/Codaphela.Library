#include "CoreLib/NamedIndexedData.h"
#include "ObjectReaderIndexed.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectReaderIndexed::Init(CNamedIndexedData* pcIndexedData)
{
	mpcIndexedData = pcIndexedData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectReaderIndexed::Kill(void)
{
	mpcIndexedData = NULL;
	CObjectReader::Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CObjectReaderIndexed::Read(OIndex oi, void* pvBuffer)
{
	CSerialisedObject*		pcSerialised;
	BOOL					bExists;

	bExists = mpcIndexedData->Get(oi, pvBuffer);
	if (bExists)
	{
		pcSerialised = (CSerialisedObject*)pvBuffer;
		return pcSerialised;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CObjectReaderIndexed::Read(char* szObjectName, void* pvBuffer)
{
	CSerialisedObject*		pcSerialised;
	BOOL					bExists;

	bExists = mpcIndexedData->Get(szObjectName, pvBuffer);
	if (bExists)
	{
		pcSerialised = (CSerialisedObject*)pvBuffer;
		return pcSerialised;
	}
	else
	{
		return NULL;
	}
}


