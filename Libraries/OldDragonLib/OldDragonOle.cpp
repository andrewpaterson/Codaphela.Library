/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela OldDragonLib

Codaphela OldDragonLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela OldDragonLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela OldDragonLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include <string.h>
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/Logger.h"
#include "OldDragonOle.h"


#define OLE_HEADER_SIZE				512
#define OLE_HEADER_DIFAT_ENTRIES	109
#define OLE_DIRECTORY_ENTRY_SIZE	128
#define OLE_END_OF_CHAIN			0xfffffffe
#define OLE_FREE_SECTOR				0xffffffff
#define OLE_TYPE_STREAM				2
#define OLE_TYPE_ROOT				5


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COldDragonOle::_Init(void)
{
	mpuiData = NULL;
	muiSize = 0;
	muiSectorSize = 0;
	muiMiniSectorSize = 0;
	muiMiniCutoff = 0;
	mpuiFat = NULL;
	muiFatEntries = 0;
	mpuiMiniFat = NULL;
	muiMiniFatEntries = 0;
	mpuiDirectory = NULL;
	muiDirectorySize = 0;
	mpuiMiniStream = NULL;
	muiMiniStreamSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COldDragonOle::Init(const char* szFilename)
{
	static const uint8	auiSignature[] = { 0xd0, 0xcf, 0x11, 0xe0, 0xa1, 0xb1, 0x1a, 0xe1 };
	uint32				uiDirectory;
	uint32				uiMiniFat;
	uint8*				puiMiniFat;
	size				uiMiniFatBytes;
	size				uiPos;
	uint32				uiRootStart;

	_Init();
	ReturnOnFalse(ReadFile(szFilename));

	if ((muiSize < OLE_HEADER_SIZE) || (memcmp(mpuiData, auiSignature, sizeof(auiSignature)) != 0))
	{
		return gcLogger.Error2(__METHOD__, " File [", szFilename, "] isn't an OLE compound file.", NULL);
	}

	muiSectorSize = (size)1 << (mpuiData[30] & 0x1f);
	muiMiniSectorSize = (size)1 << (mpuiData[32] & 0x1f);
	if ((muiSectorSize < 512) || (muiSectorSize > 65536) || (muiMiniSectorSize == 0) || (muiMiniSectorSize > muiSectorSize))
	{
		return gcLogger.Error2(__METHOD__, " File [", szFilename, "] has unusual sector sizes.", NULL);
	}
	muiMiniCutoff = Uint32At(56);
	ReturnOnFalse(ReadFat());

	uiDirectory = Uint32At(48);
	if (!ReadChain(uiDirectory, &mpuiDirectory, &muiDirectorySize))
	{
		return gcLogger.Error2(__METHOD__, " File [", szFilename, "]'s directory couldn't be read.", NULL);
	}

	uiMiniFat = Uint32At(60);
	if (Uint32At(64) != 0)
	{
		if (!ReadChain(uiMiniFat, &puiMiniFat, &uiMiniFatBytes))
		{
			return gcLogger.Error2(__METHOD__, " File [", szFilename, "]'s mini FAT couldn't be read.", NULL);
		}
		mpuiMiniFat = (uint32*)puiMiniFat;
		muiMiniFatEntries = uiMiniFatBytes / sizeof(uint32);
	}

	for (uiPos = 0; uiPos + OLE_DIRECTORY_ENTRY_SIZE <= muiDirectorySize; uiPos += OLE_DIRECTORY_ENTRY_SIZE)
	{
		if (mpuiDirectory[uiPos + 66] == OLE_TYPE_ROOT)
		{
			memcpy(&uiRootStart, &mpuiDirectory[uiPos + 116], sizeof(uint32));
			if (uiRootStart != OLE_END_OF_CHAIN)
			{
				if (!ReadChain(uiRootStart, &mpuiMiniStream, &muiMiniStreamSize))
				{
					return gcLogger.Error2(__METHOD__, " File [", szFilename, "]'s mini stream couldn't be read.", NULL);
				}
			}
			break;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COldDragonOle::Kill(void)
{
	free(mpuiMiniStream);
	free(mpuiDirectory);
	free(mpuiMiniFat);
	free(mpuiFat);
	free(mpuiData);
	_Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COldDragonOle::ReadFile(const char* szFilename)
{
	CFileBasic	cFile;
	bool		bResult;

	cFile.Init(DiskFile(szFilename));
	if (!cFile.Open(EFM_Read))
	{
		cFile.Kill();
		return gcLogger.Error2(__METHOD__, " Could not open file [", szFilename, "].", NULL);
	}

	muiSize = (size)cFile.Size();
	mpuiData = (uint8*)malloc(muiSize + 1);
	bResult = (mpuiData != NULL);
	if (bResult && (muiSize > 0))
	{
		bResult = cFile.Read(mpuiData, muiSize, 1) == 1;
	}
	cFile.Close();
	cFile.Kill();

	if (!bResult)
	{
		return gcLogger.Error2(__METHOD__, " Could not read file [", szFilename, "].", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The FAT's own sectors are listed in the header, and then in a chain of
// sectors if there are too many for the header.
//
//////////////////////////////////////////////////////////////////////////
bool COldDragonOle::ReadFat(void)
{
	uint32	uiNumFatSectors;
	uint32	uiDifat;
	uint32	uiNumDifat;
	size	uiEntriesPerSector;
	size	uiFound;
	size	i;
	uint32	uiSector;
	size	uiPos;
	size	uiDifatIndex;

	uiNumFatSectors = Uint32At(44);
	uiDifat = Uint32At(68);
	uiNumDifat = Uint32At(72);
	uiEntriesPerSector = muiSectorSize / sizeof(uint32);

	if ((uiNumFatSectors == 0) || ((size)uiNumFatSectors > muiSize / muiSectorSize))
	{
		return gcLogger.Error2(__METHOD__, " The file's FAT has an unreasonable size.", NULL);
	}

	muiFatEntries = (size)uiNumFatSectors * uiEntriesPerSector;
	mpuiFat = (uint32*)malloc(muiFatEntries * sizeof(uint32));
	if (mpuiFat == NULL)
	{
		return false;
	}

	uiFound = 0;
	for (i = 0; (i < OLE_HEADER_DIFAT_ENTRIES) && (uiFound < uiNumFatSectors); i++)
	{
		uiSector = Uint32At(76 + i * sizeof(uint32));
		uiPos = ((size)uiSector + 1) * muiSectorSize;
		if ((uiSector >= OLE_END_OF_CHAIN) || (uiPos > muiSize) || (muiSize - uiPos < muiSectorSize))
		{
			return gcLogger.Error2(__METHOD__, " The file's FAT sector is outside the file.", NULL);
		}
		memcpy(&mpuiFat[uiFound * uiEntriesPerSector], &mpuiData[uiPos], muiSectorSize);
		uiFound++;
	}

	for (uiDifatIndex = 0; (uiDifatIndex < uiNumDifat) && (uiFound < uiNumFatSectors); uiDifatIndex++)
	{
		uiPos = ((size)uiDifat + 1) * muiSectorSize;
		if ((uiDifat >= OLE_END_OF_CHAIN) || (uiPos > muiSize) || (muiSize - uiPos < muiSectorSize))
		{
			return gcLogger.Error2(__METHOD__, " The file's extended FAT sector is outside the file.", NULL);
		}
		for (i = 0; (i + 1 < uiEntriesPerSector) && (uiFound < uiNumFatSectors); i++)
		{
			uiSector = Uint32At(uiPos + i * sizeof(uint32));
			if ((uiSector >= OLE_END_OF_CHAIN) || (((size)uiSector + 1) * muiSectorSize > muiSize) || (muiSize - ((size)uiSector + 1) * muiSectorSize < muiSectorSize))
			{
				return gcLogger.Error2(__METHOD__, " The file's FAT sector is outside the file.", NULL);
			}
			memcpy(&mpuiFat[uiFound * uiEntriesPerSector], &mpuiData[((size)uiSector + 1) * muiSectorSize], muiSectorSize);
			uiFound++;
		}
		uiDifat = Uint32At(uiPos + (uiEntriesPerSector - 1) * sizeof(uint32));
	}

	if (uiFound != uiNumFatSectors)
	{
		return gcLogger.Error2(__METHOD__, " The file's FAT is incomplete.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A chain can't have more sectors than the FAT has, so a chain that loops
// runs out of room and fails rather than running forever.
//
//////////////////////////////////////////////////////////////////////////
bool COldDragonOle::ReadChain(uint32 uiStart, uint8** ppuiDest, size* puiSize)
{
	size	uiNumSectors;
	uint32	uiSector;
	size	uiPos;
	size	uiDone;

	*ppuiDest = NULL;
	*puiSize = 0;

	uiNumSectors = 0;
	uiSector = uiStart;
	while (uiSector != OLE_END_OF_CHAIN)
	{
		if (((size)uiSector >= muiFatEntries) || (uiNumSectors >= muiFatEntries))
		{
			return false;
		}
		uiSector = mpuiFat[uiSector];
		uiNumSectors++;
	}

	*ppuiDest = (uint8*)malloc(uiNumSectors * muiSectorSize + 1);
	if (*ppuiDest == NULL)
	{
		return false;
	}

	uiDone = 0;
	uiSector = uiStart;
	while (uiSector != OLE_END_OF_CHAIN)
	{
		uiPos = ((size)uiSector + 1) * muiSectorSize;
		if ((uiPos > muiSize) || (muiSize - uiPos < muiSectorSize))
		{
			free(*ppuiDest);
			*ppuiDest = NULL;
			return false;
		}
		memcpy(&(*ppuiDest)[uiDone], &mpuiData[uiPos], muiSectorSize);
		uiDone += muiSectorSize;
		uiSector = mpuiFat[uiSector];
	}
	*puiSize = uiDone;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COldDragonOle::ReadMiniChain(uint32 uiStart, uint8* puiDest, size uiSize)
{
	uint32	uiSector;
	size	uiDone;
	size	uiPos;
	size	uiLength;
	size	uiSteps;

	uiSector = uiStart;
	uiDone = 0;
	uiSteps = 0;
	while (uiDone < uiSize)
	{
		if (((size)uiSector >= muiMiniFatEntries) || (uiSteps > muiMiniFatEntries))
		{
			return false;
		}
		uiPos = (size)uiSector * muiMiniSectorSize;
		uiLength = uiSize - uiDone;
		if (uiLength > muiMiniSectorSize)
		{
			uiLength = muiMiniSectorSize;
		}
		if ((uiPos > muiMiniStreamSize) || (muiMiniStreamSize - uiPos < uiLength))
		{
			return false;
		}
		memcpy(&puiDest[uiDone], &mpuiMiniStream[uiPos], uiLength);
		uiDone += uiLength;
		uiSector = mpuiMiniFat[uiSector];
		uiSteps++;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COldDragonOle::ReadStream(const char* szPrefix, uint8** ppuiStream, size* puiSize)
{
	size			uiPos;
	const uint8*	puiEntry;
	uint32			uiStart;
	uint32			uiSize;
	size			uiChainSize;

	*ppuiStream = NULL;
	*puiSize = 0;

	for (uiPos = 0; uiPos + OLE_DIRECTORY_ENTRY_SIZE <= muiDirectorySize; uiPos += OLE_DIRECTORY_ENTRY_SIZE)
	{
		puiEntry = &mpuiDirectory[uiPos];
		if ((puiEntry[66] != OLE_TYPE_STREAM) || !DirectoryNameStartsWith(puiEntry, szPrefix))
		{
			continue;
		}

		memcpy(&uiStart, &puiEntry[116], sizeof(uint32));
		memcpy(&uiSize, &puiEntry[120], sizeof(uint32));

		if ((size)uiSize < muiMiniCutoff)
		{
			if ((size)uiSize > muiMiniStreamSize)
			{
				return false;
			}
			*ppuiStream = (uint8*)malloc((size)uiSize + 1);
			if ((*ppuiStream == NULL) || !ReadMiniChain(uiStart, *ppuiStream, uiSize))
			{
				free(*ppuiStream);
				*ppuiStream = NULL;
				return false;
			}
			*puiSize = uiSize;
			return true;
		}

		if (!ReadChain(uiStart, ppuiStream, &uiChainSize))
		{
			return false;
		}
		if ((size)uiSize > uiChainSize)
		{
			free(*ppuiStream);
			*ppuiStream = NULL;
			return false;
		}
		*puiSize = uiSize;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 COldDragonOle::Uint32At(size uiPos)
{
	uint32	uiValue;

	if ((uiPos > muiSize) || (muiSize - uiPos < sizeof(uint32)))
	{
		return OLE_FREE_SECTOR;
	}
	memcpy(&uiValue, &mpuiData[uiPos], sizeof(uint32));
	return uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
// A directory entry's name is UTF-16.  Every stream asked for is ASCII.
//
//////////////////////////////////////////////////////////////////////////
bool COldDragonOle::DirectoryNameStartsWith(const uint8* puiEntry, const char* szPrefix)
{
	size	i;

	for (i = 0; szPrefix[i] != 0; i++)
	{
		if ((i >= 31) || (puiEntry[i * 2] != (uint8)szPrefix[i]) || (puiEntry[i * 2 + 1] != 0))
		{
			return false;
		}
	}
	return true;
}

