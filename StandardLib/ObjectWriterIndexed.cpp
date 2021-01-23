#include "CoreLib/DataConnection.h"
#include "SerialisedObject.h"
#include "ObjectWriterIndexed.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectWriterIndexed::Init(CDataConnection* pcDataConnection, unsigned int	uiTimeStamp)
{
	mpcDataConnection = pcDataConnection;
	muiTimeStamp = uiTimeStamp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectWriterIndexed::Kill(void)
{
	mpcDataConnection = NULL;
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
		bResult = mpcDataConnection->Put(oi, pcSerialised->GetName(), pcSerialised, pcSerialised->GetLength());
	}
	else if (pcSerialised->IsIndexed())
	{
		bResult = mpcDataConnection->Put(oi, pcSerialised, pcSerialised->GetLength());
	}
	else
	{
		bResult = FALSE;
	}

	return bResult;
}

