/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
void CChunkIndex::ReadInit(void)
{
	mcChunkIndices.Init(CHUNK_INDEX_CHUNK_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CChunkIndex::WriteInit(void)
{
	mcChunkIndices.Init(CHUNK_INDEX_CHUNK_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CChunkIndex::Kill(void)
{
	mcChunkIndices.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChunkIndex::FindFirstChunkWithName(int iName)
{
	int i;
	int	iTestName;

	iCurrChunkNum = 0;
	miSearchName = iName;

	for (i = 0; i < mcChunkIndices.NumElements(); i++)
	{
		iTestName = mcChunkIndices.Get(i)->iName;
		if (iTestName == iName)
		{
			iCurrChunkNum = i+1;
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChunkIndex::FindNextChunkWithName(void)
{
	int i;

	for (i = iCurrChunkNum; i < mcChunkIndices.NumElements(); i++)
	{
		if (mcChunkIndices.Get(i)->iName == miSearchName)
		{
			iCurrChunkNum = i+1;
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChunkIndex::GetNumChunksWithName(int iName)
{
	int i;
	int iNumWithName;

	iNumWithName = 0;

	for (i = 0; i < mcChunkIndices.NumElements(); i++)
	{
		if (mcChunkIndices.Get(i)->iName == iName)
		{
			iNumWithName++;
		}
	}
	return iNumWithName;
}

