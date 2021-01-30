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
	int		iLength;
	char*	szName;

	oi = pcSerialised->GetIndex();
	iLength = pcSerialised->GetLength();
	if (pcSerialised->IsNamed())
	{
		szName = pcSerialised->GetName();
		bResult = mpcDataConnection->Put(oi, szName, pcSerialised, iLength);
	}
	else if (pcSerialised->IsIndexed())
	{
		bResult = mpcDataConnection->Put(oi, pcSerialised, iLength);
	}
	else
	{
		bResult = FALSE;
	}

	return bResult;
}

