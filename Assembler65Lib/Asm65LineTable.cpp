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
#include "BaseLib/StringHelper.h"
#include "Asm65LineTable.h"


//////////////////////////////////////////////////////////////////////////
//
// Lines by address; at one address the one added last (a later .line at
// the same place) last.
//
//////////////////////////////////////////////////////////////////////////
static int CompareAsm65Lines(const void* pvLeft, const void* pvRight)
{
	const SAsm65LineEntry*	psLeft;
	const SAsm65LineEntry*	psRight;

	psLeft = (const SAsm65LineEntry*)pvLeft;
	psRight = (const SAsm65LineEntry*)pvRight;
	if (psLeft->uiAddress != psRight->uiAddress)
	{
		return (psLeft->uiAddress < psRight->uiAddress) ? -1 : 1;
	}
	return (psLeft->uiOrder < psRight->uiOrder) ? -1 : ((psLeft->uiOrder > psRight->uiOrder) ? 1 : 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65LineTable::Init(void)
{
	maszFiles.Init();
	mcFileIndices.Init(true, true);
	masLines.Init();
	masFingerprints.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65LineTable::Kill(void)
{
	size	ui;

	for (ui = 0; ui < masFingerprints.NumElements(); ui++)
	{
		masFingerprints.Get(ui)->szLabel.Kill();
	}
	masFingerprints.Kill();
	masLines.Kill();
	mcFileIndices.Kill();
	maszFiles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65LineTable::Clear(void)
{
	Kill();
	Init();
}


//////////////////////////////////////////////////////////////////////////
//
// The code at uiAddress is for line uiLine (1 based; 0 for none) of szFile.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65LineTable::AddLine(uint32 uiAddress, const char* szFile, uint32 uiLine)
{
	SAsm65LineEntry*	psLine;
	uint32*				puiFile;
	uint32				uiFile;

	puiFile = mcFileIndices.Get(szFile);
	if (puiFile)
	{
		uiFile = *puiFile;
	}
	else
	{
		uiFile = (uint32)maszFiles.NumElements();
		maszFiles.Add(szFile);
		mcFileIndices.Put(szFile, &uiFile);
	}
	psLine = masLines.Add();
	psLine->uiAddress = uiAddress;
	psLine->uiFile = uiFile;
	psLine->uiLine = uiLine;
	psLine->uiOrder = (uint32)(masLines.NumElements() - 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65LineTable::AddFingerprint(const char* szLabel, uint32 uiHash, int64 iTime)
{
	SAsm65LineFingerprint*	psFingerprint;

	psFingerprint = masFingerprints.Add();
	psFingerprint->szLabel.Init(szLabel);
	psFingerprint->uiHash = uiHash;
	psFingerprint->iTime = iTime;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65LineTable::Sort(void)
{
	masLines.Sort(CompareAsm65Lines);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65LineTable::NumLines(void)
{
	return masLines.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65LineEntry* CAsm65LineTable::GetLine(size uiIndex)
{
	return masLines.SafeGet(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// The line the code at uiAddress is for: the last at or before it (after
// Sort).  NULL if there is none, or it is for no line.
//
//////////////////////////////////////////////////////////////////////////
SAsm65LineEntry* CAsm65LineTable::FindLine(uint32 uiAddress)
{
	size				uiLow;
	size				uiHigh;
	size				uiMiddle;
	SAsm65LineEntry*	psLine;

	uiLow = 0;
	uiHigh = masLines.NumElements();
	while (uiLow < uiHigh)
	{
		uiMiddle = (uiLow + uiHigh) / 2;
		if (masLines.Get(uiMiddle)->uiAddress <= uiAddress)
		{
			uiLow = uiMiddle + 1;
		}
		else
		{
			uiHigh = uiMiddle;
		}
	}
	if (uiLow == 0)
	{
		return NULL;
	}
	psLine = masLines.Get(uiLow - 1);
	return (psLine->uiLine != 0) ? psLine : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65LineTable::NumFiles(void)
{
	return maszFiles.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CAsm65LineTable::GetFile(size uiIndex)
{
	return (uiIndex < maszFiles.NumElements()) ? maszFiles.Get(uiIndex)->Text() : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65LineTable::NumFingerprints(void)
{
	return masFingerprints.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65LineFingerprint* CAsm65LineTable::GetFingerprint(size uiIndex)
{
	return masFingerprints.SafeGet(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65LineFingerprint* CAsm65LineTable::FindFingerprint(const char* szLabel)
{
	size	ui;

	for (ui = 0; ui < masFingerprints.NumElements(); ui++)
	{
		if (masFingerprints.Get(ui)->szLabel.Equals(szLabel))
		{
			return masFingerprints.Get(ui);
		}
	}
	return NULL;
}

