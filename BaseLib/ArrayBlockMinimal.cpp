#include "ArrayBlockMinimal.h"
#include "SystemAllocator.h"
#include "GlobalMemory.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockMinimal::Init(void)
{
	Init(&gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockMinimal::Init(CMallocator* pcMalloc)
{
	CAlloc::Init(pcMalloc);
	miUsedElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockMinimal::Kill(void)
{
	miUsedElements = 0;
	CAlloc::Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlockMinimal::WriteHeader(CFileWriter* pcFileWriter)
{
	if (!pcFileWriter->WriteInt(miUsedElements))
	{
		return FALSE;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlockMinimal::WriteAllocatorAndHeader(CFileWriter* pcFileWriter)
{
	BOOL	bResult;

	bResult = gcMallocators.WriteMallocator(pcFileWriter, mpcMalloc);
	if (!bResult)
	{
		return FALSE;
	}

	bResult = WriteHeader(pcFileWriter);
	if (!bResult)
	{
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlockMinimal::ReadAllocator(CFileReader* pcFileReader)
{
	CMallocator*	pcMalloc;

	pcMalloc = gcMallocators.ReadMallocator(pcFileReader);
	if (pcMalloc == NULL)
	{
		return FALSE;
	}
	CAlloc::Init(pcMalloc);
	return TRUE;
}

