/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/Chars.h"
#include "BaseLib/FastFunctions.h"
#include "IndexedData.h"
#include "IndexedHuge.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedHuge::Init(CDurableFile* pcFile, BOOL bDirtyTesting, CIndexedData* pcIndexedData, int iSecondLevelWidth, int iThirdLevelWidth, int iNumSecondLevelChunks, int iNumThirdLevelChunks)
{
	miLastOi = INVALID_O_INDEX;
	mpcFile = pcFile;
	mbDirtyTesting = bDirtyTesting;
	mpcIndexedData = pcIndexedData;
	mpvSecondLevel = NULL;
	mpvThirdLevel = NULL;
	mapFirstLevel.Init();

	ClearCounters();
	ChangeStrategy(iSecondLevelWidth, iThirdLevelWidth, iNumSecondLevelChunks, iNumThirdLevelChunks);

	mbSecondLevelCacheFull = FALSE;
	mpsFirstThirdLevelChunk = NULL;
	mpsLastThirdLevelChunk = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedHuge::Kill(void)
{
	mapFirstLevel.Kill();
	SafeFree(mpvSecondLevel);
	SafeFree(mpvThirdLevel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedHuge::ClearCounters(void)
{
	miDiskReads = 0;
	miDiskWrites = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedHuge::ChangeStrategy(int iSecondLevelWidth, int iThirdLevelWidth, int iNumSecondLevelChunks, int iNumThirdLevelChunks)
{
	int		iSecondLevelCacheSize;
	int		iThirdLevelCacheSize;

	if (miLastOi != INVALID_O_INDEX)
	{
		return FALSE;
	}
	if ((iSecondLevelWidth % iThirdLevelWidth) != 0)
	{
		return FALSE;
	}

	miSecondLevelChunkWidth = iSecondLevelWidth;
	miThirdLevelChunkWidth = iThirdLevelWidth;

	miNumSecondLevelChunks = iNumSecondLevelChunks;
	miNumThirdLevelChunks = iNumThirdLevelChunks;

	SafeFree(mpvSecondLevel);
	SafeFree(mpvThirdLevel);

	iSecondLevelCacheSize = GetSecondLevelCacheByteSize();
	iThirdLevelCacheSize = GetThirdLevelCacheByteSize();

	mpvSecondLevel = malloc(iSecondLevelCacheSize);
	memset_fast(mpvSecondLevel, 0, iSecondLevelCacheSize);
	mpvThirdLevel = malloc(iThirdLevelCacheSize);
	memset_fast(mpvThirdLevel, 0, iThirdLevelCacheSize);

	miLastSecondLevelChunk = -1;

	mapFirstLevel.Kill();
	mapFirstLevel.Allocate(iNumSecondLevelChunks);
	mapFirstLevel.Zero();

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedHuge::NumPossibleInMemoryIndexDescriptors(void)
{
	return miNumThirdLevelChunks * miThirdLevelChunkWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedHuge::GetThirdLevelCacheByteSize(void)
{
	return miNumThirdLevelChunks * (sizeof(SIndexedThirdLevelSearch) + (sizeof(CIndexDescriptor) * miThirdLevelChunkWidth));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedHuge::GetSecondLevelCacheByteSize(void)
{
	return miNumSecondLevelChunks * (sizeof(SIndexedSecondLevelSearch) + (sizeof(SIndexedThirdLevelSearch*) * miSecondLevelChunkWidth));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIndexedSecondLevelSearch* CIndexedHuge::GetSecondLevelSearch(OIndex oi)
{
	int								i;
	SIndexedSecondLevelSearch*		psSecondLevel;

	//The second level chunks should be sorted.
	for (i = 0; i < miNumSecondLevelChunks; i++)
	{
		psSecondLevel = *mapFirstLevel.Get(i);
		if (psSecondLevel)
		{
			if ((oi >= psSecondLevel->iFirst) && (oi <= psSecondLevel->iLast))
			{
				return psSecondLevel;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexDescriptor* CIndexedHuge::PrivateGetDescriptor(OIndex oi)
{
	SIndexedSecondLevelSearch*	psIndexedSecondLevelSearch;
	SIndexedThirdLevelSearch*	psIndexedThirdLevelSearch;
	int							iSecondLevelIndex;
	int							iThirdLevelOffset;
	CIndexDescriptor*			pcDescriptorInCache;
	int							iSecondLevelOffset;

	psIndexedSecondLevelSearch = GetSecondLevelSearch(oi);
	if (psIndexedSecondLevelSearch == NULL)
	{
		iSecondLevelIndex = (int)(oi / miSecondLevelChunkWidth);
		psIndexedSecondLevelSearch = LoadSecondLevelChunk(iSecondLevelIndex);
	}

	iSecondLevelOffset = (int)(oi - psIndexedSecondLevelSearch->iFirst);

	psIndexedThirdLevelSearch = *GetIndexedThirdLevelChunk(psIndexedSecondLevelSearch, iSecondLevelOffset);
	if (psIndexedThirdLevelSearch == NULL)
	{
		psIndexedThirdLevelSearch = LoadThirdLevelChunk(psIndexedSecondLevelSearch, iSecondLevelOffset);
	}

	iThirdLevelOffset = iSecondLevelOffset % miThirdLevelChunkWidth;
	pcDescriptorInCache = GetCachedDescriptor(psIndexedThirdLevelSearch, iThirdLevelOffset);
	
	return pcDescriptorInCache;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedHuge::Get(CIndexDescriptor* pcDescriptor, OIndex oi)
{
	CIndexDescriptor*			pcDescriptorInCache;

	if (oi > miLastOi)
	{
		return FALSE;
	}

	pcDescriptorInCache = PrivateGetDescriptor(oi);
	if (!pcDescriptorInCache->IsAllocated())
	{
		return FALSE;
	}

	memcpy(pcDescriptor, pcDescriptorInCache, sizeof(CIndexDescriptor));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedHuge::Set(CIndexDescriptor* pcDescriptor)
{
	CIndexDescriptor*			pcDescriptorInCache;
	OIndex						oi;

	oi = pcDescriptor->GetIndex();

	pcDescriptorInCache = PrivateGetDescriptor(oi);

	if (mbDirtyTesting)
	{
		if (!pcDescriptorInCache->IsAllocated())
		{
			pcDescriptor->Dirty(TRUE);
		}
		else
		{
			if (pcDescriptorInCache->IsDirty())
			{
				pcDescriptor->Dirty(TRUE);
			}
			else if (!pcDescriptor->IsDirty())
			{
				if (memcmp(pcDescriptor, pcDescriptorInCache, sizeof(CIndexDescriptor)) != 0)
				{
					pcDescriptor->Dirty(TRUE);
				}
			}
		}

		if (pcDescriptor->IsDirty())
		{
			memcpy_fast(pcDescriptorInCache, pcDescriptor, sizeof(CIndexDescriptor));
		}
	}
	else
	{
		memcpy_fast(pcDescriptorInCache, pcDescriptor, sizeof(CIndexDescriptor));
	}


	if (miLastOi < pcDescriptor->GetIndex())
	{
		miLastOi = pcDescriptor->GetIndex();
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedHuge::Remove(OIndex oi)
{
	CIndexDescriptor	cIndexedDataDescriptor;
	BOOL				bResult;

	bResult = Get(&cIndexedDataDescriptor, oi);
	if (!bResult)
	{
		return FALSE;
	}

	cIndexedDataDescriptor.Init(oi, 0);
	return Set(&cIndexedDataDescriptor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexedHuge::Length(void)
{
	return miLastOi+1;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedHuge::Load(void)
{
	filePos				iFileSize;
	CIndexDescriptor*	pacDescriptors;
	filePos				iNumToRead;
	OIndex				iRemaining;
	int					iNumInChunk;
	int					i;
	filePos				iActualRead;

	miLastOi = INVALID_O_INDEX;
	iFileSize = mpcFile->Size();
	if (iFileSize == 0)
	{
		return;
	}

	iNumInChunk = HUGE_DESRIPTOR_READ_SIZE / sizeof(CIndexDescriptor);
	if (iFileSize <= HUGE_DESRIPTOR_READ_SIZE)
	{
		pacDescriptors = (CIndexDescriptor*)malloc((int)iFileSize);
		iNumToRead = iFileSize / sizeof(CIndexDescriptor);
		iRemaining = 0;
	}
	else
	{
		pacDescriptors = (CIndexDescriptor*)malloc(HUGE_DESRIPTOR_READ_SIZE);
		iNumToRead = iNumInChunk;
		iRemaining = (iFileSize / sizeof(CIndexDescriptor)) - iNumToRead;
	}

	while (iNumToRead != 0)
	{
		iActualRead = mpcFile->Read(pacDescriptors, sizeof(CIndexDescriptor), iNumToRead);
		miDiskReads += iActualRead;

		for (i = 0; i < iNumToRead; i++)
		{
			if (pacDescriptors[i].IsAllocated())
			{
				if (miLastOi < pacDescriptors[i].GetIndex())
				{
					miLastOi = pacDescriptors[i].GetIndex();
				}
			}
		}

		if (iRemaining > iNumInChunk)
		{
			iNumToRead = iNumInChunk;
			iRemaining -= iNumInChunk;
		}
		else
		{
			iNumToRead = (int)iRemaining;
			iRemaining = 0;
		}
	}

	free(pacDescriptors);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedHuge::Save(void)
{
	int							i;
	SIndexedThirdLevelSearch*	ps;

	//You need to optimise this to write sequentially and in contiguous third level chunks.
	for (i = 0; i < miNumThirdLevelChunks; i++)
	{
		ps = GetCachedThirdLevelChunk(i);
		if (ps->bValid)
		{
			SaveThirdLevelChunk(ps);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int	CIndexedHuge::IncrementSecondLevelNumber(int iInput)
{
	if (iInput < miNumSecondLevelChunks-1)
	{
		iInput++;
	}
	else
	{
		iInput = 0;
	}
	return iInput;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIndexedSecondLevelSearch* CIndexedHuge::LoadSecondLevelChunk(int iSecondLevelIndex)
{
	SIndexedSecondLevelSearch*		psIndexedSecondLevelSearch;
	int								iPrevLast;
	int								iSize;

	iSize = (sizeof(SIndexedSecondLevelSearch) + (sizeof(SIndexedThirdLevelSearch*) * miSecondLevelChunkWidth));
	if (!mbSecondLevelCacheFull)
	{
		iPrevLast = miLastSecondLevelChunk;
		miLastSecondLevelChunk = IncrementSecondLevelNumber(miLastSecondLevelChunk);
		if ((miLastSecondLevelChunk != 0) || (iPrevLast == -1))
		{
			psIndexedSecondLevelSearch = (SIndexedSecondLevelSearch*)RemapSinglePointer(mpvSecondLevel, miLastSecondLevelChunk * iSize);
			memset_fast(psIndexedSecondLevelSearch, 0, iSize);

			SetSecondLevelChunk(psIndexedSecondLevelSearch, iSecondLevelIndex);
			return psIndexedSecondLevelSearch;
		}

		mbSecondLevelCacheFull = TRUE;
		miLastSecondLevelChunk--;
	}
	miLastSecondLevelChunk = IncrementSecondLevelNumber(miLastSecondLevelChunk);

	psIndexedSecondLevelSearch = (SIndexedSecondLevelSearch*)RemapSinglePointer(mpvSecondLevel, miLastSecondLevelChunk * iSize);
	EvictSecondLevelChunk(psIndexedSecondLevelSearch);

	SetSecondLevelChunk(psIndexedSecondLevelSearch, iSecondLevelIndex);
	return psIndexedSecondLevelSearch;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedHuge::SetSecondLevelChunk(SIndexedSecondLevelSearch* psIndexedSecondLevelSearch, int iSecondLevelIndex)
{
	psIndexedSecondLevelSearch->iFirst = miSecondLevelChunkWidth * iSecondLevelIndex;
	psIndexedSecondLevelSearch->iLast = (miSecondLevelChunkWidth * (iSecondLevelIndex + 1)) - 1;
	psIndexedSecondLevelSearch->iArrayIndex = miLastSecondLevelChunk;
	mapFirstLevel.Set(miLastSecondLevelChunk, &psIndexedSecondLevelSearch);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIndexedThirdLevelSearch* CIndexedHuge::LoadThirdLevelChunk(SIndexedSecondLevelSearch* psIndexedSecondLevelSearch, int iSecondLevelOffset)
{
	SIndexedThirdLevelSearch**	ppsIndexedThirdLevelSearch;
	SIndexedThirdLevelSearch*	psIndexedThirdLevelSearch;
	CIndexDescriptor*			psDescriptor;
	filePos						iResult;
	OIndex						iOffset;
	filePos						iLength;
	int							iNumToRead;
	int							iNumToClear;
	filePos						iDescriptorsInFile;
	OIndex						iFirstInThirdLevelChunk;
	OIndex						iLastInThirdLevelChunk;

	if (mpsFirstThirdLevelChunk == NULL)
	{
		psIndexedThirdLevelSearch = GetCachedThirdLevelChunk(0);
		mpsFirstThirdLevelChunk = psIndexedThirdLevelSearch;
	}
	else
	{
		psIndexedThirdLevelSearch = FindUnallocatedThirdLevelChunk();
		if (psIndexedThirdLevelSearch == NULL)
		{
			psIndexedThirdLevelSearch = mpsFirstThirdLevelChunk;
			EvictThirdLevelChunk(mpsFirstThirdLevelChunk);
		}
	}

	psIndexedThirdLevelSearch->psNext = NULL;
	psIndexedThirdLevelSearch->bValid = TRUE;
	psIndexedThirdLevelSearch->psParent = psIndexedSecondLevelSearch;
	psIndexedThirdLevelSearch->iSecondLevelOffset = iSecondLevelOffset;

	iLength = mpcFile->Size();
	iDescriptorsInFile = iLength / sizeof(CIndexDescriptor);

	iFirstInThirdLevelChunk = psIndexedSecondLevelSearch->iFirst + ((iSecondLevelOffset / miThirdLevelChunkWidth) * miThirdLevelChunkWidth);
	iLastInThirdLevelChunk = iFirstInThirdLevelChunk + miThirdLevelChunkWidth-1;
	if (iDescriptorsInFile > iLastInThirdLevelChunk)
	{
		//If the file completely contains this chunk then read the whole chunk.
		iOffset = iFirstInThirdLevelChunk * sizeof(CIndexDescriptor);
		psDescriptor = GetCachedDescriptor(psIndexedThirdLevelSearch, 0);
		iResult = mpcFile->Read(EFSO_SET, (unsigned int)iOffset, psDescriptor, sizeof(CIndexDescriptor), miThirdLevelChunkWidth);
		miDiskReads += iResult;
	}
	else if (iDescriptorsInFile <= iFirstInThirdLevelChunk)
	{
		//If the file doesn't contain this chunk (or part) then clear the whole chunk.
		psDescriptor = GetCachedDescriptor(psIndexedThirdLevelSearch, 0);
		memset_fast(psDescriptor, 0, sizeof(CIndexDescriptor) * miThirdLevelChunkWidth);
	}
	else
	{
		//The file partially contains this chunk.  Read some, clear the rest.
		iNumToClear = (int)((iLastInThirdLevelChunk - iDescriptorsInFile) + 1);
		iNumToRead = (miThirdLevelChunkWidth - (iNumToClear));

		iOffset = iFirstInThirdLevelChunk * sizeof(CIndexDescriptor);
		psDescriptor = GetCachedDescriptor(psIndexedThirdLevelSearch, 0);
		iResult = mpcFile->Read(EFSO_SET, (unsigned int)iOffset, psDescriptor, sizeof(CIndexDescriptor), iNumToRead);
		miDiskReads += iResult;
		psDescriptor = GetCachedDescriptor(psIndexedThirdLevelSearch, iNumToRead);
		memset_fast(psDescriptor, 0, sizeof(CIndexDescriptor) * iNumToClear);
	}

	ppsIndexedThirdLevelSearch = GetIndexedThirdLevelChunk(psIndexedSecondLevelSearch, iSecondLevelOffset);
	*ppsIndexedThirdLevelSearch = psIndexedThirdLevelSearch;

	if (mpsLastThirdLevelChunk)
	{
		mpsLastThirdLevelChunk->psNext = psIndexedThirdLevelSearch;
	}
	mpsLastThirdLevelChunk = psIndexedThirdLevelSearch;

	return psIndexedThirdLevelSearch;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedHuge::EvictSecondLevelChunk(SIndexedSecondLevelSearch* psIndexedSecondLevelSearch)
{
	OIndex						ulliFirstLevelIndex;
	int							iFirstLevelIndex;
	SIndexedSecondLevelSearch*	psTemp;
	int							i;
	SIndexedThirdLevelSearch*	psIndexedThirdLevelSearch;

	ulliFirstLevelIndex = psIndexedSecondLevelSearch->iFirst / miSecondLevelChunkWidth;
	iFirstLevelIndex = (int)ulliFirstLevelIndex;
	psTemp = NULL;
	mapFirstLevel.Set(psIndexedSecondLevelSearch->iArrayIndex, &psTemp);

	for (i = 0; i < miSecondLevelChunkWidth; i++)
	{
		psIndexedThirdLevelSearch = *GetIndexedThirdLevelChunk(psIndexedSecondLevelSearch, i);
		if ((psIndexedThirdLevelSearch) && (psIndexedThirdLevelSearch->bValid))
		{
			EvictThirdLevelChunk(psIndexedThirdLevelSearch);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedHuge::EvictThirdLevelChunk(SIndexedThirdLevelSearch* psIndexedThirdLevelSearch)
{
	SIndexedThirdLevelSearch**		ppsIndexedThirdLevelSearch;
	SIndexedSecondLevelSearch*		psIndexedSecondLevelSearch;
	int								iSecondLevelOffset;

	psIndexedSecondLevelSearch = psIndexedThirdLevelSearch->psParent;

	//Clear the second level chunk pointer pointing to this third level chunk.
	iSecondLevelOffset = psIndexedThirdLevelSearch->iSecondLevelOffset;
	ppsIndexedThirdLevelSearch = GetIndexedThirdLevelChunk(psIndexedSecondLevelSearch, iSecondLevelOffset);
	(*ppsIndexedThirdLevelSearch) = NULL;

	//Roll to the next third level chunk if the chunk being evicted was this one.
	if (mpsFirstThirdLevelChunk == psIndexedThirdLevelSearch)
	{
		mpsFirstThirdLevelChunk = psIndexedThirdLevelSearch->psNext;
	}

	SaveThirdLevelChunk(psIndexedThirdLevelSearch);

	EvictCachedObjects(psIndexedThirdLevelSearch);

	memset_fast(psIndexedThirdLevelSearch, 0, sizeof(SIndexedThirdLevelSearch) + (miThirdLevelChunkWidth*sizeof(CIndexDescriptor)));
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedHuge::EvictCachedObjects(SIndexedThirdLevelSearch* psIndexedThirdLevelSearch)
{
	int					i;
	CIndexDescriptor*	pcDescriptor;

	if ((mpcIndexedData) && (mpcIndexedData->IsCaching()))
	{
		for (i = 0; i < miThirdLevelChunkWidth; i++)
		{
			pcDescriptor = GetCachedDescriptor(psIndexedThirdLevelSearch, i);
			if (pcDescriptor->IsAllocated())
			{
				mpcIndexedData->EvictFromCache(pcDescriptor);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedHuge::SaveThirdLevelChunk(SIndexedThirdLevelSearch* psIndexedThirdLevelSearch)
{
	OIndex				iFirstChunkOI;
	filePos				iLength;
	filePos				iDescriptorsInFile;
	int					i;
	CIndexDescriptor*	pcDescriptor;
	OIndex				iLastChunkOI;
	long				iOffset;
	filePos				iResult;
	filePos				iNumToWrite;
	BOOL				bResult;

	iFirstChunkOI = GetThirdLevelChunkOI(psIndexedThirdLevelSearch);

	iLength = mpcFile->Size();
	iDescriptorsInFile = iLength / sizeof(CIndexDescriptor);

	iLastChunkOI = -1;
	for (i = miThirdLevelChunkWidth-1; i >= 0; i--)
	{
		pcDescriptor = GetCachedDescriptor(psIndexedThirdLevelSearch, i);
		if (pcDescriptor->IsAllocated())
		{
			if ((!mbDirtyTesting) || (mbDirtyTesting && pcDescriptor->IsDirty()))
			{
				iLastChunkOI = pcDescriptor->GetIndex();
				break;
			}
		}
	}

	for (i = 0; i < miThirdLevelChunkWidth; i++)
	{
		pcDescriptor = GetCachedDescriptor(psIndexedThirdLevelSearch, i);
		pcDescriptor->Dirty(FALSE);
	}

	if (iLastChunkOI != -1)
	{
		iOffset = (int)(iFirstChunkOI * sizeof(CIndexDescriptor));
		pcDescriptor = GetCachedDescriptor(psIndexedThirdLevelSearch, 0);
		iNumToWrite = (filePos)((iLastChunkOI-iFirstChunkOI)+1);

		if (iLength < iOffset)
		{
			bResult = PadFile(iLength, iOffset);
		}

		iResult = mpcFile->Write(EFSO_SET, (unsigned int)iOffset, pcDescriptor, sizeof(CIndexDescriptor), iNumToWrite);
		miDiskWrites += iResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedHuge::PadFile(filePos iLength, filePos iOffset)
{
	CIndexDescriptor	cZero;
	filePos				iDiff;
	filePos				i;
	filePos				iResult;

	iDiff = (iOffset - iLength) / sizeof(CIndexDescriptor);
	cZero.Init(INVALID_O_INDEX, 0);
	for (i = 0; i < iDiff; i++)
	{
		iResult = mpcFile->Write(EFSO_SET, ((iLength/sizeof(CIndexDescriptor)) + i) * sizeof(CIndexDescriptor), &cZero, sizeof(CIndexDescriptor), 1);
		miDiskWrites += iResult;
		if (iResult != 1)
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexedHuge::GetThirdLevelChunkOI(SIndexedThirdLevelSearch* psIndexedThirdLevelSearch)
{
	return psIndexedThirdLevelSearch->psParent->iFirst + (psIndexedThirdLevelSearch->iSecondLevelOffset / miThirdLevelChunkWidth) * miThirdLevelChunkWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIndexedThirdLevelSearch* CIndexedHuge::FindUnallocatedThirdLevelChunk(void)
{
	int							i;
	SIndexedThirdLevelSearch*	psIndexedThirdLevelSearch;

	for (i = 0; i < miNumThirdLevelChunks; i++)
	{
		psIndexedThirdLevelSearch = GetCachedThirdLevelChunk(i);
		if (!psIndexedThirdLevelSearch->bValid)
		{
			return psIndexedThirdLevelSearch;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIndexedThirdLevelSearch* CIndexedHuge::GetCachedThirdLevelChunk(int iIndex)
{
	SIndexedThirdLevelSearch*	ps;

	ps = (SIndexedThirdLevelSearch*)RemapSinglePointer(mpvThirdLevel, ((sizeof(SIndexedThirdLevelSearch) + (sizeof(CIndexDescriptor)*miThirdLevelChunkWidth))) * iIndex);
	return ps;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexDescriptor* CIndexedHuge::GetCachedDescriptor(SIndexedThirdLevelSearch* psIndexedThirdLevelSearch, int iIndex)
{
	CIndexDescriptor*		pc;

	pc = (CIndexDescriptor*)RemapSinglePointer(psIndexedThirdLevelSearch, sizeof(SIndexedThirdLevelSearch) + iIndex * sizeof(CIndexDescriptor));
	return pc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIndexedThirdLevelSearch** CIndexedHuge::GetIndexedThirdLevelChunk(SIndexedSecondLevelSearch* pSIndexedSecondLevelSearch, int iSecondLevelOffset)
{
	SIndexedThirdLevelSearch**	ppsIndexedThirdLevelSearch;
	int							iThirdLevelIndex;

	iThirdLevelIndex = (iSecondLevelOffset / miThirdLevelChunkWidth) * miThirdLevelChunkWidth;
	ppsIndexedThirdLevelSearch = (SIndexedThirdLevelSearch**)RemapSinglePointer(pSIndexedSecondLevelSearch, sizeof(SIndexedSecondLevelSearch) + (sizeof(CIndexDescriptor*) * iThirdLevelIndex));
	return ppsIndexedThirdLevelSearch;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedHuge::UpdateFile(void)
{
	Save();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedHuge::Set(CIndexDescriptor* pacDescriptors, int iNumDescriptors)
{
	int					i;
	BOOL				bDirtyTesting;
	CIndexDescriptor*	pcDescriptor;

	bDirtyTesting = mbDirtyTesting;
	mbDirtyTesting = FALSE;
	for (i = 0; i < iNumDescriptors; i++)
	{
		pcDescriptor = &pacDescriptors[i];
		if (pcDescriptor->IsAllocated())
		{
			Set(pcDescriptor);
		}
	}
	mbDirtyTesting = bDirtyTesting;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedHuge::DumpThirdLevelCache(void)
{
	int 						iThirdLevelCacheSize;
	int							i;
	CChars						sz;
	SIndexedThirdLevelSearch*	ps;
	int							j;
	CIndexDescriptor*		pc;

	sz.Init();

	iThirdLevelCacheSize = miNumThirdLevelChunks * (sizeof(SIndexedThirdLevelSearch) + (sizeof(CIndexDescriptor)*miThirdLevelChunkWidth));
	sz.Append("Third Level Cache (");
	sz.Append(iThirdLevelCacheSize);
	sz.Append(" bytes)\n--------------------\n");
	sz.Append("Chunk Width: ");
	sz.Append(miThirdLevelChunkWidth);
	sz.Append("\nChunk Count: ");
	sz.Append(miNumThirdLevelChunks);
	sz.Append("\nChunk Size: ");
	sz.Append((int)(sizeof(SIndexedThirdLevelSearch) + (sizeof(CIndexDescriptor)*miThirdLevelChunkWidth)));
	sz.AppendNewLine();
	for (i = 0; i < miNumThirdLevelChunks; i++)
	{
		ps = GetCachedThirdLevelChunk(i);
		sz.Append("Valid: ");
		sz.Append(ps->bValid);
		sz.AppendNewLine();
		if (ps->bValid)
		{
			sz.Append("Parent: ");
			sz.Append((int)ps->psParent->iFirst);
			sz.Append(" - ");
			sz.Append((int)ps->psParent->iLast);
			sz.Append(" (");
			sz.Append(ps->iSecondLevelOffset);
			sz.Append(")\n");

			for (j = 0; j < miThirdLevelChunkWidth; j++)
			{
				pc = GetCachedDescriptor(ps, j);
				if (pc->IsAllocated())
				{
					sz.Append("    OI: ");
					sz.Append((int)pc->GetIndex());
					sz.Append("\n");
				}
				else
				{
					sz.Append("    Unallocated\n");
				}
			}
		}
		sz.AppendNewLine();
	}
	sz.AppendNewLine();

	sz.Dump();
	sz.Kill();
}

