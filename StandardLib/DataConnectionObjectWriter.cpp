#include "CoreLib/DataConnection.h"
#include "SerialisedObject.h"
#include "DataConnectionObjectWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDataConnectionObjectWriter::Init(CDataConnection* pcDataConnection, unsigned int	uiTimeStamp)
{
	mpcDataConnection = pcDataConnection;
	muiTimeStamp = uiTimeStamp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDataConnectionObjectWriter::Kill(void)
{
	mpcDataConnection = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDataConnectionObjectWriter::Write(CSerialisedObject* pcSerialised)
{
	OIndex	oi;
	BOOL	bResult;
	int		iLength;
	char*	szName;

	iLength = pcSerialised->GetLength();
	if (pcSerialised->IsNamed())
	{
		oi = pcSerialised->GetIndex();
		szName = pcSerialised->GetName();
		bResult = mpcDataConnection->Put(oi, szName, pcSerialised, iLength);
	}
	else if (pcSerialised->IsIndexed())
	{
		oi = pcSerialised->GetIndex();
		bResult = mpcDataConnection->Put(oi, pcSerialised, iLength);
	}
	else
	{
		bResult = FALSE;
	}

	return bResult;
}

