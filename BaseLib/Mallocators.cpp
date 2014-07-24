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

	sz = pcMallocator->GetName();
	if (mmszcMallocators.Get(sz))
	{
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
	int				iLength;
	char			szName[1024];
	CMallocator**	ppcMallocator;

	if (!pcFileReader->ReadStringLength(&iLength))
	{
		gcLogger.Error("Could not read mallocator name length.");
		return FALSE;
	}

	if ((iLength < 0 || iLength >= 1024))
	{
		gcLogger.Error2("Could not read mallocator name, too long [", IntToString(iLength), "].", NULL);
	}

	if (!pcFileReader->ReadStringChars(szName, iLength))
	{
		gcLogger.Error("Could not read mallocator name.");
		return FALSE;
	}

	ppcMallocator = mmszcMallocators.Get(szName);
	if (!ppcMallocator)
	{
		gcLogger.Error2("Could not find mallocator named [", szName, "].");
		return FALSE;
	}

	return *ppcMallocator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMallocators::WriteMallocator(CFileWriter* pcFileWriter, CMallocator* pcMallocator)
{
	CLocalMallocator*	pcLocal;

	if (pcMallocator == NULL)
	{
		gcLogger.Error("Could not write NULL mallocator.");
		return FALSE;
	}

	if (!pcFileWriter->WriteString(pcMallocator->GetName()))
	{
		gcLogger.Error("Could not write mallocator name.");
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

