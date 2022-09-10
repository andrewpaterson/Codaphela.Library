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
bool CInternalObjectSerialiser::Write(CBaseObject* pcObject)
{
	CObjectWriter		cWriter;
	bool				bResult;
	CSerialisedObject*	pcSerialised;
	CMemoryFile			cMemory;
	unsigned int		iSize;

	cMemory.Init();
	cMemory.Open(EFM_ReadWrite_Create);
	cWriter.Init(&cMemory, NULL);

	bResult = cWriter.Write(pcObject);
	ReturnOnFalse(bResult);

	bResult = cWriter.WriteHeapFroms(pcObject);
	ReturnOnFalse(bResult);

	pcSerialised = (CSerialisedObject*)cMemory.GetBufferPointer();
	iSize = cMemory.GetBufferSize();

	bResult = Put(pcSerialised, iSize);
	ReturnOnFalse(bResult);

	cMemory.Close();
	cWriter.Kill();
	cMemory.Kill();

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInternalObjectSerialiser::Put(CSerialisedObject* pcSerialised, unsigned int iSize)
{
	OIndex	oi;
	bool	bResult;
	char*	szName;

	if (pcSerialised->IsNamed())
	{
		oi = pcSerialised->GetIndex();
		szName = pcSerialised->GetName();
		bResult = mpcDataConnection->Put(oi, szName, pcSerialised, iSize);
	}
	else if (pcSerialised->IsIndexed())
	{
		oi = pcSerialised->GetIndex();
		bResult = mpcDataConnection->Put(oi, pcSerialised, iSize);
	}
	else
	{
		bResult = false;
	}

	return bResult;
}
