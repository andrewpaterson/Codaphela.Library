#include <stdlib.h>
#include <string.h>
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/PointerRemapper.h"
#include "MemoryDrive.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMemoryDrive::Init(size_t uiSizeInBytes, uint32 uiSectorSize)
{
	size_t	uiSectorCount;
	uint64	uiSectorRemainder;

	CFileDrive::Init(uiSectorSize);

	uiSectorCount = uiSizeInBytes / uiSectorSize;
	uiSectorRemainder = uiSizeInBytes % uiSectorSize;
	mpvData = NULL;

	if (uiSectorRemainder != 0)
	{
		uiSectorCount++;
	}

	mpvData = malloc(uiSectorCount * muiSectorSize);
	if (mpvData)
	{
		memset(mpvData, 0, uiSectorCount * muiSectorSize);
		muiMaxSector = uiSectorCount;
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryDrive::Kill(void)
{
	SafeFree(mpvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMemoryDrive::Read(uint64 uiSector, void* pvData)
{
	if (uiSector < muiMaxSector)
	{
		memcpy(pvData, RemapSinglePointer(mpvData, (size_t)(uiSector * muiSectorSize)), muiSectorSize);
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMemoryDrive::Write(uint64 uiSector, void* pvData)
{
	if (uiSector < muiMaxSector)
	{
		memcpy(RemapSinglePointer(mpvData, (size_t)(uiSector * muiSectorSize)), pvData, muiSectorSize);
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMemoryDrive::Erase(uint64 uiStartSector, uint64 uiStopSectorInclusive)
{
	if ((uiStartSector < muiMaxSector) && (uiStopSectorInclusive < muiMaxSector) && (uiStartSector >= uiStartSector))
	{
		memset(RemapSinglePointer(mpvData, (ptrdiff_t)(uiStartSector* muiSectorSize)), 0, (size_t)(((uiStopSectorInclusive - uiStartSector) + 1)* muiSectorSize));
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryDrive::GetMemory(void)
{
	return mpvData;
}

