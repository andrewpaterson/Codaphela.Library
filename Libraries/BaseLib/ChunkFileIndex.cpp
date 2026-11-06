/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "ChunkFileIndex.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SChunkIndex::Init(filePos iChunkDataPos, chunkName iName, filePos iChunkSize)
{
	memset(this, 0xEE, sizeof(SChunkIndex));

	miChunkDataPos = iChunkDataPos;
	miName = iName;
	miChunkSize = iChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChunkIndex::ReadInit(void)
{
	mcChunkIndices.Init();
	macNames.Init();
	miSearchName = 0;
	muiNextName = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChunkIndex::WriteInit(void)
{
	ReadInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChunkIndex::Kill(void)
{
	macNames.Kill();
	mcChunkIndices.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CompareChunkNamePosition(const void* pvLeft, const void* pvRight)
{
	SChunkNamePosition*	psLeft;
	SChunkNamePosition*	psRight;

	psLeft = (SChunkNamePosition*)pvLeft;
	psRight = (SChunkNamePosition*)pvRight;
	if (psLeft->miName != psRight->miName)
	{
		return (psLeft->miName < psRight->miName) ? -1 : 1;
	}
	if (psLeft->muiPosition != psRight->muiPosition)
	{
		return (psLeft->muiPosition < psRight->muiPosition) ? -1 : 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Sorts the names unless they already are (chunks are only ever added).
//
//////////////////////////////////////////////////////////////////////////
void CChunkIndex::SortNames(void)
{
	lsize				i;
	lsize				uiNumChunks;
	SChunkNamePosition*	psName;

	uiNumChunks = mcChunkIndices.NumElements();
	if (macNames.NumElements() == uiNumChunks)
	{
		return;
	}

	macNames.ReInit();
	for (i = 0; i < uiNumChunks; i++)
	{
		psName = macNames.Add();
		psName->miName = mcChunkIndices.Get(i)->miName;
		psName->muiPosition = i;
	}
	macNames.QuickSort(CompareChunkNamePosition);
}


//////////////////////////////////////////////////////////////////////////
//
// Where in macNames the first chunk named iName is, or would be.
//
//////////////////////////////////////////////////////////////////////////
lsize CChunkIndex::FindFirstName(chunkName iName)
{
	lsize	uiLow;
	lsize	uiHigh;
	lsize	uiMiddle;

	SortNames();
	uiLow = 0;
	uiHigh = macNames.NumElements();
	while (uiLow < uiHigh)
	{
		uiMiddle = uiLow + (uiHigh - uiLow) / 2;
		if (macNames.Get(uiMiddle)->miName < iName)
		{
			uiLow = uiMiddle + 1;
		}
		else
		{
			uiHigh = uiMiddle;
		}
	}
	return uiLow;
}


//////////////////////////////////////////////////////////////////////////
//
// The position of the first chunk named iName (CHUNK_FILE_NOT_FOUND if
// there isn't one).  FindNextChunkWithName gives the others, in order.
//
//////////////////////////////////////////////////////////////////////////
lsize CChunkIndex::FindFirstChunkWithName(chunkName iName)
{
	miSearchName = iName;
	muiNextName = FindFirstName(iName);
	return FindNextChunkWithName();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CChunkIndex::FindNextChunkWithName(void)
{
	SChunkNamePosition*	psName;

	psName = macNames.SafeGet(muiNextName);
	if ((psName == NULL) || (psName->miName != miSearchName))
	{
		return CHUNK_FILE_NOT_FOUND;
	}
	muiNextName++;
	return psName->muiPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CChunkIndex::GetNumChunksWithName(chunkName iName)
{
	lsize	uiFirst;
	lsize	i;
	lsize	uiNumNames;

	uiFirst = FindFirstName(iName);
	uiNumNames = macNames.NumElements();
	for (i = uiFirst; (i < uiNumNames) && (macNames.Get(i)->miName == iName); i++)
	{
	}
	return i - uiFirst;
}

