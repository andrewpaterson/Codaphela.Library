#include "SystemAllocator.h"
#include "GlobalMemory.h"
#include "NullAllocator.h"
#include "Logger.h"
#include "LogString.h"
#include "Mallocators.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMallocators::Init(void)
{
	mmszcMallocators.Init(8, FALSE);

	AddMallocator(&gcNullAllocator);
	AddMallocator(&gcSystemAllocator);
	AddMallocator(&gcMemoryAllocator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMallocators::Kill(void)
{
	mmszcMallocators.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMallocators::AddMallocator(CMallocator* pcMallocator)
{
	char*	sz;

	sz = (char*)(pcMallocator->GetName());
	if (mmszcMallocators.Get(sz))
	{
		gcLogger.Error2(__METHOD__, " A mallocator named [", sz, "] already exists.", NULL);
		return FALSE;
	}

	mmszcMallocators.Put(sz, &pcMallocator);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMallocator* CMallocators::ReadMallocator(CFileReader* pcFileReader)
{
	int					iLength;
	char				szName[1024];
	CMallocator**		ppcMallocator;
	CLocalMallocator*	pcLocalMallocator;

	if (!MemoryValidate())
	{
		return NULL;
	}

	if (!pcFileReader->ReadStringLength(&iLength))
	{
		gcLogger.Error("Could not read mallocator name length.");
		return FALSE;
	}

	if ((iLength < 0 || iLength >= 1024))
	{
		gcLogger.Error2(__METHOD__, " Could not read mallocator name, too long [", IntToString(iLength), "].", NULL);
	}

	if (!pcFileReader->ReadStringChars(szName, iLength))
	{
		gcLogger.Error2(__METHOD__, " Could not read mallocator name.", NULL);
		return FALSE;
	}

	ppcMallocator = mmszcMallocators.Get(szName);
	if (!ppcMallocator)
	{
		gcLogger.Error2(__METHOD__, " Could not find mallocator named [", szName, "].", NULL);
		return FALSE;
	}

	if ((*ppcMallocator)->IsLocal())
	{
		pcLocalMallocator = (CLocalMallocator*)(*ppcMallocator);
		pcLocalMallocator = (CLocalMallocator*)gcConstructors.Construct(pcLocalMallocator->GetName(), &gcSystemAllocator);
		if (!pcLocalMallocator)
		{
			gcLogger.Error2(__METHOD__, " Could not construct local mallocator [", (*ppcMallocator)->GetName(), "].", NULL);
			return NULL;
		}
		if (!pcLocalMallocator->Read(pcFileReader))
		{
			gcLogger.Error2(__METHOD__, " Could not read local mallocator [", pcLocalMallocator->GetName(), "].", NULL);
			return NULL;
		}
		else
		{
			return pcLocalMallocator;
		}
	}
	else
	{
		return *ppcMallocator;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMallocators::WriteMallocator(CFileWriter* pcFileWriter, CMallocator* pcMallocator)
{
	CLocalMallocator*	pcLocal;

	if (!MemoryValidate())
	{
		return FALSE;
	}

	if (pcMallocator == NULL)
	{
		gcLogger.Error2(__METHOD__, " Could not write NULL mallocator.", NULL);
		return FALSE;
	}

	if (!pcFileWriter->WriteString(pcMallocator->GetName()))
	{
		gcLogger.Error2(__METHOD__, " Could not write mallocator name [", pcMallocator->GetName(), "].", NULL);
		return FALSE;
	}

	if (pcMallocator->IsLocal())
	{
		pcLocal = (CLocalMallocator*)pcMallocator;
		return pcLocal->Write(pcFileWriter);
	}
	else
	{
		return TRUE;
	}
}

