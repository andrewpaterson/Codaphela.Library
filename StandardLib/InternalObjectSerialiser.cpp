#include "CoreLib/DataConnection.h"
#include "ObjectWriter.h"
#include "SerialisedObject.h"
#include "InternalObjectSerialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInternalObjectSerialiser::Init(CDataConnection* pcDataConnection)
{
	mpcDataConnection = pcDataConnection;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInternalObjectSerialiser::Kill(void)
{
	mpcDataConnection = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInternalObjectSerialiser::Write(CBaseObject* pcObject)
{
	CObjectWriter	cSerialiser;
	BOOL				bResult;
	CSerialisedObject*	pcSerialised;

	cSerialiser.Init(NULL);

	bResult = cSerialiser.Write(pcObject);
	ReturnOnFalse(bResult);

	pcSerialised = (CSerialisedObject*)cSerialiser.GetData();

	bResult = Write(pcSerialised);
	ReturnOnFalse(bResult);

	cSerialiser.Kill();

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInternalObjectSerialiser::Write(CSerialisedObject* pcSerialised)
{
	OIndex	oi;
	BOOL	bResult;
	int		iLength;
	char* szName;

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
