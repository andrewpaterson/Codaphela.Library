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
#include "OldDragonCursor.h"
#include "OldDragonFile.h"


#define OLD_DRAGON_MAGIC			0xb4d85f9a
#define OLD_DRAGON_NO_NAME			0xffffffff
#define OLD_DRAGON_FILE_HEADER		24
#define OLD_DRAGON_HEADER_SHORT		12
#define OLD_DRAGON_HEADER_LONG		32
#define OLD_DRAGON_INDEX_ENTRY		12
#define OLD_DRAGON_MAX_DEPTH		32


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COldDragonFile::_Init(void)
{
	mpuiData = NULL;
	muiSize = 0;
	meVersion = ODV_Unknown;
	mbDamaged = false;
	macChunks.Init();
	macNames.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// Fails on a file that can't be read, or isn't one of the old layouts.
//
//////////////////////////////////////////////////////////////////////////
bool COldDragonFile::Init(const char* szFilename)
{
	bool	bResult;

	_Init();
	if (!ReadWholeFile(szFilename))
	{
		return false;
	}

	meVersion = Identify();
	switch (meVersion)
	{
	case ODV_IOX:
		bResult = WalkIOX();
		break;
	case ODV_August2009:
		bResult = WalkIndexed(0, OLD_DRAGON_HEADER_SHORT, -1, 0);
		break;
	case ODV_September2009:
		bResult = WalkIndexed(0, OLD_DRAGON_HEADER_LONG, -1, 0);
		break;
	case ODV_Max:
		bResult = ReadNames(Uint32At(0));
		if (bResult)
		{
			//WriteClose can only finish the innermost open chunk: a root with no index means others were left open.
			if ((int32)Uint32At(OLD_DRAGON_FILE_HEADER + 8) == -1)
			{
				bResult = SalvageMax(Uint32At(0));
			}
			else
			{
				bResult = WalkIndexed(OLD_DRAGON_FILE_HEADER, OLD_DRAGON_HEADER_LONG, -1, 0);
			}
		}
		break;
	default:
		bResult = false;
		break;
	}

	if (!bResult)
	{
		return gcLogger.Error2(__METHOD__, " Could not read old dragon file [", szFilename, "].", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COldDragonFile::Kill(void)
{
	macNames.Kill();
	macChunks.Kill();
	free(mpuiData);
	mpuiData = NULL;
	muiSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EOldDragonVersion COldDragonFile::GetVersion(void)
{
	return meVersion;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COldDragonFile::IsDamaged(void)
{
	return mbDamaged;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size COldDragonFile::NumChunks(void)
{
	return macChunks.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SOldDragonChunk* COldDragonFile::GetChunk(size uiIndex)
{
	return macChunks.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// Only Max files have names for their chunks.  NULL for the others, and
// for a chunk left unclosed, as the name is only written when it closes.
//
//////////////////////////////////////////////////////////////////////////
const char* COldDragonFile::GetChunkName(SOldDragonChunk* psChunk)
{
	return FindName(psChunk->uiName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const uint8* COldDragonFile::GetChunkData(SOldDragonChunk* psChunk)
{
	return &mpuiData[psChunk->uiDataPos];
}


//////////////////////////////////////////////////////////////////////////
//
// NumChunks if the parent has no child with that name.
//
//////////////////////////////////////////////////////////////////////////
size COldDragonFile::FindChild(size uiParent, uint32 uiName)
{
	size				i;
	size				uiNumChunks;
	SOldDragonChunk*	psChunk;

	uiNumChunks = macChunks.NumElements();
	for (i = uiParent + 1; i < uiNumChunks; i++)
	{
		psChunk = macChunks.Get(i);
		if ((psChunk->iParent == (int32)uiParent) && (psChunk->uiName == uiName))
		{
			return i;
		}
	}
	return uiNumChunks;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COldDragonFile::ReadWholeFile(const char* szFilename)
{
	CFileBasic	cFile;
	filePos		iSize;
	bool		bResult;

	cFile.Init(DiskFile(szFilename));
	if (!cFile.Open(EFM_Read))
	{
		cFile.Kill();
		return gcLogger.Error2(__METHOD__, " Could not open file [", szFilename, "].", NULL);
	}

	iSize = cFile.Size();
	muiSize = (size)iSize;
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
//
//////////////////////////////////////////////////////////////////////////
EOldDragonVersion COldDragonFile::Identify(void)
{
	size	uiPos;
	uint32	uiNumChunks;
	uint32	i;
	uint32	uiNext;

	if (muiSize < OLD_DRAGON_HEADER_SHORT)
	{
		return ODV_Unknown;
	}

	if ((memcmp(mpuiData, "CHK", 4) == 0) || (memcmp(mpuiData, "DRG", 4) == 0))
	{
		return ODV_Current;
	}

	if (Contains(20, 4) && (Uint32At(20) == OLD_DRAGON_MAGIC))
	{
		return ODV_Max;
	}

	if (Uint32At(0) == OLD_DRAGON_NO_NAME)
	{
		if (Contains(28, 4) && (Uint32At(28) == OLD_DRAGON_MAGIC))
		{
			return ODV_September2009;
		}
		return ODV_August2009;
	}

	//An IOX file is only recognised by its list of chunks running exactly to the end of the file.
	uiNumChunks = Uint32At(0);
	if ((uiNumChunks == 0) || (uiNumChunks > muiSize / OLD_DRAGON_HEADER_SHORT))
	{
		return ODV_Unknown;
	}
	uiPos = sizeof(uint32);
	for (i = 0; i < uiNumChunks; i++)
	{
		if (!Contains(uiPos, OLD_DRAGON_HEADER_SHORT))
		{
			return ODV_Unknown;
		}
		uiNext = Uint32At(uiPos + 4);
		if (uiNext <= uiPos)
		{
			return ODV_Unknown;
		}
		uiPos = uiNext;
	}
	if (uiPos == muiSize)
	{
		return ODV_IOX;
	}
	return ODV_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
// A root is added so every version looks the same to a reader.
//
//////////////////////////////////////////////////////////////////////////
bool COldDragonFile::WalkIOX(void)
{
	uint32	uiNumChunks;
	uint32	i;
	size	uiPos;
	uint32	uiName;
	uint32	uiNext;
	uint32	uiSize;

	uiNumChunks = Uint32At(0);
	ReturnOnFalse(AddChunk(OLD_DRAGON_NO_NAME, -1, 0, sizeof(uint32), muiSize - sizeof(uint32), false));

	uiPos = sizeof(uint32);
	for (i = 0; i < uiNumChunks; i++)
	{
		uiName = Uint32At(uiPos);
		uiNext = Uint32At(uiPos + 4);
		uiSize = Uint32At(uiPos + 8);
		if (uiPos + OLD_DRAGON_HEADER_SHORT + uiSize != uiNext)
		{
			return gcLogger.Error2(__METHOD__, " IOX chunk size doesn't reach the next chunk.", NULL);
		}
		ReturnOnFalse(AddChunk(uiName, 0, uiPos, OLD_DRAGON_HEADER_SHORT, uiSize, false));
		uiPos = uiNext;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Both index forms start with a tag and the entry size.  August's then
// repeats the entry size and September's goes straight to the count, so
// the two are told apart by version rather than by content.
//
//////////////////////////////////////////////////////////////////////////
bool COldDragonFile::WalkIndexed(size uiHeaderPos, size uiHeaderSize, int32 iParent, size uiDepth)
{
	uint32				uiName;
	uint32				uiSize;
	int32				iIndexPos;
	int32				iThis;
	uint32				uiNumEntries;
	size				uiEntries;
	uint32				i;
	size				uiEntry;
	uint32				uiChildName;
	uint32				uiChildSize;
	size				uiChildData;
	size				uiChildHeader;

	if ((uiDepth > OLD_DRAGON_MAX_DEPTH) || !Contains(uiHeaderPos, uiHeaderSize))
	{
		return gcLogger.Error2(__METHOD__, " Chunk header is outside the file.", NULL);
	}
	uiName = Uint32At(uiHeaderPos);
	uiSize = Uint32At(uiHeaderPos + 4);
	iIndexPos = (int32)Uint32At(uiHeaderPos + 8);

	iThis = (int32)macChunks.NumElements();
	ReturnOnFalse(AddChunk(uiName, iParent, uiHeaderPos, uiHeaderSize, uiSize, false));
	if (iIndexPos == -1)
	{
		return true;
	}

	if (!Contains((size)iIndexPos, 28))
	{
		return gcLogger.Error2(__METHOD__, " Chunk index is outside the file.", NULL);
	}
	if ((Uint32At((size)iIndexPos) != OLD_DRAGON_TAG_ARRAY) || (Uint32At((size)iIndexPos + 4) != OLD_DRAGON_INDEX_ENTRY))
	{
		return gcLogger.Error2(__METHOD__, " Chunk index isn't an array of index entries.", NULL);
	}
	if (meVersion == ODV_August2009)
	{
		uiNumEntries = Uint32At((size)iIndexPos + 16);
		uiEntries = (size)iIndexPos + 28;
	}
	else
	{
		uiNumEntries = Uint32At((size)iIndexPos + 8);
		uiEntries = (size)iIndexPos + 16;
	}

	for (i = 0; i < uiNumEntries; i++)
	{
		uiEntry = uiEntries + (size)i * OLD_DRAGON_INDEX_ENTRY;
		if (!Contains(uiEntry, OLD_DRAGON_INDEX_ENTRY))
		{
			return gcLogger.Error2(__METHOD__, " Chunk index runs off the end of the file.", NULL);
		}
		uiChildName = Uint32At(uiEntry);
		uiChildSize = Uint32At(uiEntry + 4);
		uiChildData = Uint32At(uiEntry + 8);
		if (uiChildData < uiHeaderSize)
		{
			return gcLogger.Error2(__METHOD__, " Chunk index points before the file.", NULL);
		}

		uiChildHeader = uiChildData - uiHeaderSize;
		if (!Contains(uiChildHeader, uiHeaderSize) || (Uint32At(uiChildHeader) != uiChildName) || (Uint32At(uiChildHeader + 4) != uiChildSize))
		{
			return gcLogger.Error2(__METHOD__, " Chunk index doesn't match the chunk it points to.", NULL);
		}
		ReturnOnFalse(WalkIndexed(uiChildHeader, uiHeaderSize, iThis, uiDepth + 1));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COldDragonFile::ReadNames(size uiPos)
{
	COldDragonCursor		cCursor;
	int32					iNumNames;
	int32					i;
	SOldDragonChunkName*	psName;

	if (!Contains(uiPos, 0))
	{
		return gcLogger.Error2(__METHOD__, " Chunk name table is outside the file.", NULL);
	}

	cCursor.Init(&mpuiData[uiPos], muiSize - uiPos);
	iNumNames = cCursor.ReadTaggedInt();
	for (i = 0; (i < iNumNames) && !cCursor.Failed(); i++)
	{
		psName = macNames.Add();
		cCursor.ReadTaggedChars(psName->szName, sizeof(psName->szName));
		psName->uiName = (uint32)cCursor.ReadTaggedInt();
	}
	if (cCursor.Failed())
	{
		return gcLogger.Error2(__METHOD__, " Chunk name table couldn't be read.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Melee.DRG has 64 meshes that were opened and never closed, so nothing
// indexes the chunks around them.  Chunks are found by their magic
// numbers instead, and all become children of the root.  A chunk with no
// name was left open and is only known to end where the next one starts.
//
//////////////////////////////////////////////////////////////////////////
bool COldDragonFile::SalvageMax(size uiEnd)
{
	size	uiPos;
	size	uiHeaderPos;
	uint32	uiName;
	uint32	uiSize;
	int32	iIndexPos;
	size	uiLast;
	size	uiNext;

	mbDamaged = true;
	if (uiEnd < OLD_DRAGON_FILE_HEADER + OLD_DRAGON_HEADER_LONG)
	{
		return gcLogger.Error2(__METHOD__, " Chunk name table is inside the root's header.", NULL);
	}
	ReturnOnFalse(AddChunk(OLD_DRAGON_NO_NAME, -1, OLD_DRAGON_FILE_HEADER, OLD_DRAGON_HEADER_LONG, uiEnd - OLD_DRAGON_FILE_HEADER - OLD_DRAGON_HEADER_LONG, true));

	for (uiPos = OLD_DRAGON_FILE_HEADER + OLD_DRAGON_HEADER_LONG + 28; uiPos + sizeof(uint32) <= uiEnd; uiPos++)
	{
		if (Uint32At(uiPos) != OLD_DRAGON_MAGIC)
		{
			continue;
		}

		uiHeaderPos = uiPos - 28;
		uiName = Uint32At(uiHeaderPos);
		uiSize = Uint32At(uiHeaderPos + 4);
		iIndexPos = (int32)Uint32At(uiHeaderPos + 8);
		if ((uiName != OLD_DRAGON_NO_NAME) && (FindName(uiName) == NULL))
		{
			continue;
		}
		if ((uiSize > uiEnd) || ((iIndexPos != -1) && ((size)iIndexPos > uiEnd)))
		{
			continue;
		}

		if (macChunks.NumElements() > 1)
		{
			//The chunk before this one, if it was left open, ends here.
			uiLast = macChunks.NumElements() - 1;
			if (macChunks.Get(uiLast)->bUnclosed)
			{
				macChunks.Get(uiLast)->uiDataSize = uiHeaderPos - macChunks.Get(uiLast)->uiDataPos;
			}
		}
		ReturnOnFalse(AddChunk(uiName, 0, uiHeaderPos, OLD_DRAGON_HEADER_LONG, uiSize, uiName == OLD_DRAGON_NO_NAME));

		//Don't look for magic numbers in a finished chunk's data.  An image could hold anything.
		if (uiName != OLD_DRAGON_NO_NAME)
		{
			uiNext = uiHeaderPos + OLD_DRAGON_HEADER_LONG + uiSize;
			uiPos = uiNext - 1;
		}
	}

	uiLast = macChunks.NumElements() - 1;
	if (uiLast > 0)
	{
		if (macChunks.Get(uiLast)->bUnclosed)
		{
			macChunks.Get(uiLast)->uiDataSize = uiEnd - macChunks.Get(uiLast)->uiDataPos;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COldDragonFile::Contains(size uiPos, size uiLength)
{
	return (uiPos <= muiSize) && (uiLength <= muiSize - uiPos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 COldDragonFile::Uint32At(size uiPos)
{
	uint32	uiValue;

	if (!Contains(uiPos, sizeof(uint32)))
	{
		return 0;
	}
	memcpy(&uiValue, &mpuiData[uiPos], sizeof(uint32));
	return uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* COldDragonFile::FindName(uint32 uiName)
{
	size					i;
	SOldDragonChunkName*	psName;

	for (i = 0; i < macNames.NumElements(); i++)
	{
		psName = macNames.Get(i);
		if (psName->uiName == uiName)
		{
			return psName->szName;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COldDragonFile::AddChunk(uint32 uiName, int32 iParent, size uiHeaderPos, size uiHeaderSize, size uiDataSize, bool bUnclosed)
{
	SOldDragonChunk*	psChunk;

	if (!Contains(uiHeaderPos + uiHeaderSize, uiDataSize))
	{
		if (!bUnclosed)
		{
			return gcLogger.Error2(__METHOD__, " Chunk runs off the end of the file.", NULL);
		}
	}

	//More chunks than could fit means an index has looped back on itself.
	if (macChunks.NumElements() > muiSize / OLD_DRAGON_HEADER_SHORT)
	{
		return gcLogger.Error2(__METHOD__, " File has more chunks than it has room for.", NULL);
	}

	psChunk = macChunks.Add();
	psChunk->uiName = uiName;
	psChunk->iParent = iParent;
	psChunk->uiHeaderPos = uiHeaderPos;
	psChunk->uiDataPos = uiHeaderPos + uiHeaderSize;
	psChunk->uiDataSize = uiDataSize;
	psChunk->bUnclosed = bUnclosed;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* OldDragonVersionName(EOldDragonVersion eVersion)
{
	switch (eVersion)
	{
	case ODV_Current:
		return "Current";
	case ODV_IOX:
		return "Into Oblivion";
	case ODV_August2009:
		return "August 2009";
	case ODV_September2009:
		return "September 2009";
	case ODV_Max:
		return "Max";
	default:
		return "Unknown";
	}
}

