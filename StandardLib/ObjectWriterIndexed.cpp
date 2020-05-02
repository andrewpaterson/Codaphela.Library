#include "CoreLib/NamedIndexedData.h"
#include "SerialisedObject.h"
#include "ObjectWriterIndexed.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectWriterIndexed::Init(CNamedIndexedData* pcIndexedData, unsigned int	uiTimeStamp)
{
	mpcIndexedData = pcIndexedData;
	muiTimeStamp = uiTimeStamp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectWriterIndexed::Kill(void)
{
	mpcIndexedData = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectWriterIndexed::Write(CSerialisedObject* pcSerialised)
{
	OIndex	oi;
	BOOL	bResult;

	oi = pcSerialised->GetIndex();
	if (pcSerialised->IsNamed())
	{
		bResult = mpcIndexedData->Put(oi, pcSerialised->GetName(), pcSerialised, pcSerialised->GetLength());
	}
	else if (pcSerialised->IsIndexed())
	{
		bResult = mpcIndexedData->Put(oi, pcSerialised, pcSerialised->GetLength());
	}
	else
	{
		bResult = FALSE;
	}

	return bResult;
}

