#include <stdlib.h>
#include <string.h>
#include "PointerFunctions.h"
#include "PointerRemapper.h"
#include "MemoryDrive.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMemoryDrive::Init(uint64 uiSizeInBytes, uint32 uiSectorSize)
{
	uint64	uiSectorCount;
	uint64	uiSectorRemainder;

	CFileDrive::Init(uiSectorSize);

	uiSectorCount = uiSizeInBytes / uiSectorSize;
	uiSectorRemainder = uiSizeInBytes % uiSectorSize;
	mpvData = NULL;

	if (uiSectorRemainder != 0)
	{
		uiSectorCount++;
	}

	muiDataSize = uiSectorCount * muiSectorSize;
	mpvData = malloc((lsize)muiDataSize);
	if (mpvData)
	{
		memset(mpvData, 0, (lsize)muiDataSize);
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
		memcpy(pvData, RemapSinglePointer(mpvData, (lsize)(uiSector * muiSectorSize)), muiSectorSize);
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
bool CMemoryDrive::Read(uint64 uiSector, uint32 uiNumSectors, void* pvData)
{
	uint64	uiEnd;
	void*	pvSource;

	uiEnd = uiSector + uiNumSectors - 1;
	if (uiSector >= muiMaxSector)
	{
		return false;
	}

	if (uiNumSectors == 0)
	{
		return true;
	}

	if (uiEnd >= muiMaxSector)
	{
		return false;
	}

	pvSource = RemapSinglePointer(mpvData, (lsize)(uiSector * muiSectorSize));
	memcpy(pvData, pvSource, muiSectorSize * uiNumSectors);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMemoryDrive::Write(uint64 uiSector, void* pvData)
{
	if (uiSector < muiMaxSector)
	{
		memcpy(RemapSinglePointer(mpvData, (lsize)(uiSector * muiSectorSize)), pvData, muiSectorSize);
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
bool CMemoryDrive::Write(uint64 uiSector, uint32 uiNumSectors, void* pvData)
{
	uint64 uiEnd;

	uiEnd = uiSector + uiNumSectors - 1;
	if (uiSector >= muiMaxSector)
	{
		return false;
	}

	if (uiNumSectors == 0)
	{
		return true;
	}

	if (uiEnd >= muiMaxSector)
	{
		return false;
	}

	memcpy(RemapSinglePointer(mpvData, (lsize)(uiSector * muiSectorSize)), pvData, muiSectorSize * uiNumSectors);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMemoryDrive::Erase(uint64 uiStartSector, uint64 uiStopSectorInclusive)
{
	if ((uiStartSector < muiMaxSector) && (uiStopSectorInclusive < muiMaxSector) && (uiStopSectorInclusive >= uiStartSector))
	{
		memset(RemapSinglePointer(mpvData, (ptrdiff_t)(uiStartSector * muiSectorSize)), 0, (lsize)(((uiStopSectorInclusive - uiStartSector) + 1) * muiSectorSize));
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
bool CMemoryDrive::Erase(void)
{
	memset(mpvData, 0, (lsize)muiMaxSector * muiSectorSize);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryDrive::GetMemory(void)
{
	return mpvData;
}

