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
	CObjectWriter		cWriter;
	BOOL				bResult;
	CSerialisedObject*	pcSerialised;
	CMemoryFile			cMemory;

	cMemory.Init();
	cWriter.Init(&cMemory, NULL);

	bResult = cWriter.Write(pcObject);
	ReturnOnFalse(bResult);

	//bResult = cWriter.WriteHeapFroms(pcObject);
	//ReturnOnFalse(bResult);

	pcSerialised = (CSerialisedObject*)cMemory.GetBufferPointer();

	bResult = Write(pcSerialised);
	ReturnOnFalse(bResult);

	cWriter.Kill();
	cMemory.Kill();

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
