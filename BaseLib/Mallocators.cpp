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
	mmszClasses.Init(true, false);
	mmShortNames.Init(true, false);

	Add(&gcNullAllocator);
	Add(&gcSystemAllocator);
	Add(&gcMemoryAllocator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMallocators::Kill(void)
{
	mmShortNames.Kill();
	mmszClasses.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMallocators::Add(CMallocator* pcMalloc)
{
	char*	sz;

	sz = (char*)(pcMalloc->ClassName());
	if (mmszClasses.Get(sz))
	{
		gcLogger.Error2(__METHOD__, " A mallocator named [", sz, "] already exists.", NULL);
		return false;
	}

	mmszClasses.Put(sz, &pcMalloc);
	mmShortNames.Put((char*)pcMalloc->ShortName(), sz);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMallocator* CMallocators::Read(CFileReader* pcFileReader)
{
	uint32				iLength;
	char				szShortName[1024];
	CMallocator**		ppcMallocator;
	CLocalMallocator*	pcLocalMallocator;
	char*				szClassName;

	if (!MemoryValidate())
	{
		return NULL;
	}

	if (!pcFileReader->ReadStringLength(&iLength))
	{
		gcLogger.Error2(__METHOD__, " Could not read mallocator name length.", NULL);
		return false;
	}

	if ((iLength < 0 || iLength >= 1024))
	{
		gcLogger.Error2(__METHOD__, " Could not read mallocator name, too long [", IntToString(iLength), "].", NULL);
	}

	if (!pcFileReader->ReadStringChars(szShortName, iLength))
	{
		gcLogger.Error2(__METHOD__, " Could not read mallocator name.", NULL);
		return false;
	}

	szClassName =  mmShortNames.Get(szShortName);
	if (!szClassName)
	{
		mmShortNames.Dump();
		gcLogger.Error2(__METHOD__, " Could not find mallocator for int16 name [", szShortName, "].", NULL);
		return false;
	}

	ppcMallocator = mmszClasses.Get(szClassName);
	if (!ppcMallocator)
	{
		gcLogger.Error2(__METHOD__, " Could not find mallocator named [", szClassName, "].", NULL);
		return false;
	}

	if ((*ppcMallocator)->IsLocal())
	{
		pcLocalMallocator = (CLocalMallocator*)(*ppcMallocator);
		pcLocalMallocator = (CLocalMallocator*)gcConstructors.Construct(pcLocalMallocator->ClassName(), &gcSystemAllocator);
		if (!pcLocalMallocator)
		{
			gcLogger.Error2(__METHOD__, " Could not construct local mallocator [", (*ppcMallocator)->ClassName(), "].", NULL);
			return NULL;
		}
		if (!pcLocalMallocator->Read(pcFileReader))
		{
			gcLogger.Error2(__METHOD__, " Could not read local mallocator [", pcLocalMallocator->ClassName(), "].", NULL);
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
bool CMallocators::Write(CFileWriter* pcFileWriter, CMallocator* pcMalloc)
{
	CLocalMallocator*	pcLocal;

	if (!MemoryValidate())
	{
		return false;
	}

	if (pcMalloc == NULL)
	{
		gcLogger.Error2(__METHOD__, " Could not write NULL mallocator.", NULL);
		return false;
	}

	if (!pcFileWriter->WriteString(pcMalloc->ShortName()))
	{
		gcLogger.Error2(__METHOD__, " Could not write mallocator name [", pcMalloc->ClassName(), "].", NULL);
		return false;
	}

	if (pcMalloc->IsLocal())
	{
		pcLocal = (CLocalMallocator*)pcMalloc;
		return pcLocal->Write(pcFileWriter);
	}
	else
	{
		return true;
	}
}

