#include <stdlib.h>
#include <string.h>
#include "TestFat32Common.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* AllocateStringBuffer(lsize uiSize)
{
	char*	 szSource;
	char	c;
	lsize	i;

	szSource = (char*)malloc(uiSize);
	c = 'A';
	for (i = 0; i < uiSize; i++)
	{
		szSource[i] = c;
		c++;
		if (c == 127)
		{
			c = 'A';
		}
		if ((c == ' ') || (c == '\\') || (c == '"'))
		{
			c++;
		}
	}
	i--;
	szSource[i] = '\0';

	return szSource;
}



//////////////////////////////////////////////////////////////////////////
//
// uiSectors sectors of 512 bytes, all zeros.
//
//////////////////////////////////////////////////////////////////////////
void CFatTestDrive::Init(uint64 uiSectors)
{
	CFileDrive::Init(512);
	muiMaxSector = uiSectors;
	muiPages = (uint32)((uiSectors + FAT_TEST_PAGE_SECTORS - 1) / FAT_TEST_PAGE_SECTORS);
	mapuiPages = (uint8**)malloc(muiPages * sizeof(uint8*));
	memset(mapuiPages, 0, muiPages * sizeof(uint8*));
	mbImage = false;
	muiImageSectors = 0;
	muiWrites = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// As CMemoryDrive::Init: uiBytes (rounded up to sectors) of zeros.  Only
// 512 byte sectors.
//
//////////////////////////////////////////////////////////////////////////
void CFatTestDrive::Init(uint64 uiBytes, uint32 uiSectorSize)
{
	Init((uiBytes + uiSectorSize - 1) / uiSectorSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The disk image szFilename's sectors, read from it as they are first used
// (it stays open until Kill or Erase).
//
//////////////////////////////////////////////////////////////////////////
bool CFatTestDrive::InitImage(const char* szFilename)
{
	uint64	uiSectors;

	mcImage.Init(szFilename);
	if (!mcImage.Open(EFM_Read))
	{
		mcImage.Kill();
		Init((uint64)0);
		return false;
	}
	uiSectors = (uint64)mcImage.Size() / 512;
	Init(uiSectors);
	mbImage = true;
	muiImageSectors = uiSectors;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatTestDrive::Kill(void)
{
	uint32	i;

	for (i = 0; i < muiPages; i++)
	{
		free(mapuiPages[i]);
	}
	free(mapuiPages);
	if (mbImage)
	{
		mcImage.Close();
		mcImage.Kill();
		mbImage = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The page uiPage: read from the image the first time (if it has it).  A
// page that is all zeros (not written, not in the image) is NULL unless
// bWrite.  The page table grows (SetMaxSectorForTesting can make the drive
// bigger than Init).
//
//////////////////////////////////////////////////////////////////////////
uint8* CFatTestDrive::GetPage(uint32 uiPage, bool bWrite)
{
	uint8*	puiPage;
	uint64	uiFirstSector;
	uint64	uiSectors;

	if (uiPage >= muiPages)
	{
		if (!bWrite)
		{
			return NULL;
		}
		mapuiPages = (uint8**)realloc(mapuiPages, (uiPage + 1) * sizeof(uint8*));
		memset(&mapuiPages[muiPages], 0, (uiPage + 1 - muiPages) * sizeof(uint8*));
		muiPages = uiPage + 1;
	}

	puiPage = mapuiPages[uiPage];
	if (puiPage)
	{
		return puiPage;
	}

	uiFirstSector = (uint64)uiPage * FAT_TEST_PAGE_SECTORS;
	if (!bWrite && (!mbImage || (uiFirstSector >= muiImageSectors)))
	{
		return NULL;
	}

	puiPage = (uint8*)malloc(FAT_TEST_PAGE_BYTES);
	memset(puiPage, 0, FAT_TEST_PAGE_BYTES);
	if (mbImage && (uiFirstSector < muiImageSectors))
	{
		uiSectors = muiImageSectors - uiFirstSector;
		if (uiSectors > FAT_TEST_PAGE_SECTORS)
		{
			uiSectors = FAT_TEST_PAGE_SECTORS;
		}
		mcImage.Seek((filePos)(uiFirstSector * 512), EFSO_SET);
		mcImage.Read(puiPage, (lsize)uiSectors * 512, 1);
	}
	mapuiPages[uiPage] = puiPage;
	return puiPage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatTestDrive::Read(uint64 uiSector, void* pvData)
{
	return Read(uiSector, 1, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatTestDrive::Read(uint64 uiSector, uint32 uiNumSectors, void* pvData)
{
	uint8*	puiPage;
	uint32	i;

	if (uiSector + uiNumSectors > muiMaxSector)
	{
		return false;
	}
	for (i = 0; i < uiNumSectors; i++)
	{
		puiPage = GetPage((uint32)((uiSector + i) / FAT_TEST_PAGE_SECTORS), false);
		if (puiPage)
		{
			memcpy((uint8*)pvData + i * 512, &puiPage[(uint32)((uiSector + i) % FAT_TEST_PAGE_SECTORS) * 512], 512);
		}
		else
		{
			memset((uint8*)pvData + i * 512, 0, 512);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatTestDrive::Write(uint64 uiSector, void* pvData)
{
	return Write(uiSector, 1, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatTestDrive::Write(uint64 uiSector, uint32 uiNumSectors, void* pvData)
{
	uint8*	puiPage;
	uint32	i;

	if (uiSector + uiNumSectors > muiMaxSector)
	{
		return false;
	}
	for (i = 0; i < uiNumSectors; i++)
	{
		puiPage = GetPage((uint32)((uiSector + i) / FAT_TEST_PAGE_SECTORS), true);
		memcpy(&puiPage[(uint32)((uiSector + i) % FAT_TEST_PAGE_SECTORS) * 512], (uint8*)pvData + i * 512, 512);
		muiWrites++;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Sectors set to zeros: only those in pages that aren't already all zeros.
//
//////////////////////////////////////////////////////////////////////////
bool CFatTestDrive::Erase(uint64 uiStartSector, uint64 uiStopSectorInclusive)
{
	uint8	auiZero[512];
	uint64	uiSector;

	memset(auiZero, 0, 512);
	for (uiSector = uiStartSector; uiSector <= uiStopSectorInclusive; uiSector++)
	{
		if (GetPage((uint32)(uiSector / FAT_TEST_PAGE_SECTORS), false) && !Write(uiSector, auiZero))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The whole drive zeros (as CMemoryDrive::Erase), the image forgotten.
//
//////////////////////////////////////////////////////////////////////////
bool CFatTestDrive::Erase(void)
{
	uint32	i;

	for (i = 0; i < muiPages; i++)
	{
		free(mapuiPages[i]);
		mapuiPages[i] = NULL;
	}
	if (mbImage)
	{
		mcImage.Close();
		mcImage.Kill();
		mbImage = false;
	}
	muiImageSectors = 0;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The sectors written since Init.
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatTestDrive::NumWrites(void)
{
	return muiWrites;
}
