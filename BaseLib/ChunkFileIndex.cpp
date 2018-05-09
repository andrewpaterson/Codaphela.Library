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
void SChunkIndex::Init(filePos iChunkDataPos, int iName, filePos iChunkSize)
{
	memset(this, 0xEE, sizeof(SChunkIndex));

	miChunkDataPos = iChunkDataPos;
	miName = iName;
	miChunkSize = iChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CChunkIndex::ReadInit(void)
{
	mcChunkIndices.Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CChunkIndex::WriteInit(void)
{
	mcChunkIndices.Init();
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

	miCurrChunkNum = 0;
	miSearchName = iName;

	for (i = 0; i < mcChunkIndices.NumElements(); i++)
	{
		iTestName = mcChunkIndices.Get(i)->miName;
		if (iTestName == iName)
		{
			miCurrChunkNum = i+1;
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

	for (i = miCurrChunkNum; i < mcChunkIndices.NumElements(); i++)
	{
		if (mcChunkIndices.Get(i)->miName == miSearchName)
		{
			miCurrChunkNum = i+1;
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
		if (mcChunkIndices.Get(i)->miName == iName)
		{
			iNumWithName++;
		}
	}
	return iNumWithName;
}

