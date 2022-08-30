#include <stdlib.h>
#include <string.h>
#include "MemoryDrive.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMemoryDrive::Init(size_t uiSizeInBytes)
{
	uint64	uiSectorCount;
	uint64	uiSectorRemainder;

	CFileDrive::Init();

	uiSectorCount = uiSizeInBytes / DRIVE_SECTOR_SIZE;
	uiSectorRemainder = uiSizeInBytes % DRIVE_SECTOR_SIZE;
	mpasSectors = NULL;

	if (uiSectorRemainder != 0)
	{
		return false;
	}

	mpasSectors = (SDriveSector*)malloc(uiSizeInBytes);
	if (mpasSectors)
	{
		memset(mpasSectors, 0, uiSizeInBytes);
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
	if (mpasSectors)
	{
		free(mpasSectors);
		mpasSectors = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMemoryDrive::Read(uint64 uiSector, SDriveSector* psSector)
{
	if (uiSector < muiMaxSector)
	{
		memcpy(psSector, &mpasSectors[uiSector], sizeof(SDriveSector));
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
bool CMemoryDrive::Write(uint64 uiSector, SDriveSector* psSector)
{
	if (uiSector < muiMaxSector)
	{
		memcpy(&mpasSectors[uiSector], psSector, sizeof(SDriveSector));
		return true;
	}
	else
	{
		return false;
	}
}

