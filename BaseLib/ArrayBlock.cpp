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
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "ArrayBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Init(int iSize)
{
	miElementSize = iSize;
	miNumElements = 0;
	miUsedElements = 0;
	miChunkSize = 1;
	mpvArray = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Init(int iSize, int iChunkSize)
{
	Init(iSize);
	miChunkSize = iChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Allocate(int iSize, int iChunkSize)
{
	Init(iSize, iChunkSize);
	GrowByChunk();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Allocate(int iSize, int iChunkSize, int iNumElements)
{
	miElementSize = iSize;
	miChunkSize = iChunkSize;
	mpvArray = NULL;
	SetUsedElements(iNumElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::ReInit(void)
{
	Kill();
	Init(miElementSize, miChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Fake(void* pvData, int iSize, int iNum, int iChunkSize)
{
	mpvArray = pvData;
	miElementSize = iSize;
	miNumElements = iChunkSize;
	miUsedElements = iNum;
	miChunkSize = iChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::FakeSetUsedElements(int iUsedElements)
{
	miUsedElements = iUsedElements;
}

