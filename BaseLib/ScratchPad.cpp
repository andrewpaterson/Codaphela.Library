/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "ScratchPad.h"
#include "MemoryStack.h"
#include "DataMacro.h"
#include "Numbers.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScratchPad::Init(void)
{
	Init(6 MB);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScratchPad::Init(int iChunkSize)
{
	mcScratchPad.Init(iChunkSize);
	miSourceChunkSize = iChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScratchPad::Kill(void)
{
	mcScratchPad.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CScratchPad::Add(size_t tSize)
{
	SSPNode*	psNode;
	void*		pvData;

	psNode = (SSPNode*)mcScratchPad.Add(tSize + sizeof(SSPNode));
	psNode->iSize = tSize;
	psNode->bUsed = TRUE;
	pvData = HeaderGetData<SSPNode, void>(psNode);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScratchPad::Reset(void)
{
	mcScratchPad.Clear();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CScratchPad::GetMemorySize(void)
{
	return mcScratchPad.GetTotalMemory();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CScratchPad::GetUsedSize(void)
{
	return mcScratchPad.GetUsedMemory();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScratchPad::GetParams(SScratchPadParams* psParams)
{
	psParams->iChunkSize = miSourceChunkSize;
}

