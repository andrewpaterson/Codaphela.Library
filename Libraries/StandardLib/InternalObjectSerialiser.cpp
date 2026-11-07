#include "BaseLib/DataConnection.h"
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
	CMemoryFile			cMemory;

	cMemory.Init();
	cMemory.Open(EFM_ReadWrite_Create);
	cWriter.Init(&cMemory, NULL);

	bResult = cWriter.Write(pcObject);
	bResult = bResult && cWriter.WriteHeapFroms(pcObject);
	bResult = bResult && Put((CSerialisedObject*)cMemory.GetBufferPointer(), cMemory.GetBufferSize());

	cMemory.Close();
	cWriter.Kill();
	cMemory.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInternalObjectSerialiser::Put(CSerialisedObject* pcSerialised, lsize iSize)
{
	if (pcSerialised->IsNamed())
	{
		return mpcDataConnection->Put(pcSerialised->GetIndex(), pcSerialised->GetName(), pcSerialised, iSize);
	}
	else if (pcSerialised->IsIndexed())
	{
		return mpcDataConnection->Put(pcSerialised->GetIndex(), pcSerialised, iSize);
	}
	else
	{
		return false;
	}
}
