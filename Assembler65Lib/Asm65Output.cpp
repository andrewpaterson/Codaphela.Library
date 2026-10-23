/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela Assembler65Lib

Codaphela Assembler65Lib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela Assembler65Lib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela Assembler65Lib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Asm65Output.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Output::Init(void)
{
	mcSegments.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Output::Kill(void)
{
	Clear();
	mcSegments.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Output::Clear(void)
{
	size	i;

	for (i = 0; i < mcSegments.NumElements(); i++)
	{
		mcSegments.Get(i)->cBytes.Kill();
	}
	mcSegments.Kill();
	mcSegments.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Output::Write(uint32 uiAddress, uint8 uiByte)
{
	SAsm65Segment*	psSegment;

	psSegment = mcSegments.Tail();
	if ((psSegment == NULL) || (psSegment->uiAddress + (uint32)psSegment->cBytes.NumElements() != uiAddress))
	{
		psSegment = mcSegments.Add();
		psSegment->uiAddress = uiAddress;
		psSegment->cBytes.Init();
	}
	psSegment->cBytes.Add(uiByte);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Output::NumSegments(void)
{
	return mcSegments.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65Segment* CAsm65Output::GetSegment(size uiIndex)
{
	return mcSegments.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Output::NumBytes(void)
{
	size	i;
	size	uiBytes;

	uiBytes = 0;
	for (i = 0; i < mcSegments.NumElements(); i++)
	{
		uiBytes += mcSegments.Get(i)->cBytes.NumElements();
	}
	return uiBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// The last segment written at the address wins.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Output::Get(uint32 uiAddress, uint8* puiByte)
{
	size			i;
	SAsm65Segment*	psSegment;
	bool			bFound;

	bFound = false;
	for (i = 0; i < mcSegments.NumElements(); i++)
	{
		psSegment = mcSegments.Get(i);
		if ((uiAddress >= psSegment->uiAddress) && (uiAddress - psSegment->uiAddress < psSegment->cBytes.NumElements()))
		{
			*puiByte = psSegment->cBytes.GetValue(uiAddress - psSegment->uiAddress);
			bFound = true;
		}
	}
	return bFound;
}


//////////////////////////////////////////////////////////////////////////
//
// Bytes that were not written read as 0.
//
//////////////////////////////////////////////////////////////////////////
uint16 CAsm65Output::GetWord(uint32 uiAddress)
{
	uint8	uiLow;
	uint8	uiHigh;

	uiLow = 0;
	uiHigh = 0;
	Get(uiAddress, &uiLow);
	Get(uiAddress + 1, &uiHigh);
	return (uint16)(uiLow | (uiHigh << 8));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CAsm65Output::GetLong(uint32 uiAddress)
{
	return (uint32)GetWord(uiAddress) | ((uint32)GetWord(uiAddress + 2) << 16);
}
