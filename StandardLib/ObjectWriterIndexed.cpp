#include "NamedIndexedData.h"
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
		bResult = mpcIndexedData->SetOrAdd(oi, pcSerialised->GetName(), pcSerialised, pcSerialised->GetLength(), muiTimeStamp);
	}
	else
	{
		bResult = mpcIndexedData->SetOrAdd(oi, pcSerialised, pcSerialised->GetLength(), muiTimeStamp);
	}

	return bResult;
}

