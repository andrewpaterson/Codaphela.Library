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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BaseLib/TextFile.h"
#include "Asm816Profile.h"


#define ASM816_PROFILE_NO_FUNCTION		0xFFFFFFFF
#define ASM816_PROFILE_ADDRESSES		0x1000000


//A function's code: its section in the linker's map.
struct SAsm816ProfileFunction
{
	uint32					uiStart;
	uint32					uiEnd;
	SAsm816FunctionProfile*	psProfile;
};


//A function being run: S as it was entered (S is below this while it runs), and which function it is.
struct SAsm816ProfileCall
{
	uint16	uiS;
	uint32	uiFunction;
};


//////////////////////////////////////////////////////////////////////////
//
// Whether a W65C816 opcode has a direct page operand (dp, dp,x, dp,y,
// (dp), (dp),y, (dp,x), [dp], [dp],y or pei): a cycle more when D's low byte
// isn't 0.
//
//////////////////////////////////////////////////////////////////////////
static bool Asm816HasDirectPageOperand(uint8 uiOpcode)
{
	uint8	uiLow;

	uiLow = uiOpcode & 0x1F;
	if ((uiOpcode & 0x03) == 0x01)
	{
		//ora and eor adc sta lda cmp sbc: (dp,x), dp, [dp], (dp),y, dp,x, [dp],y.
		return (uiLow == 0x01) || (uiLow == 0x05) || (uiLow == 0x07) || (uiLow == 0x11) || (uiLow == 0x15) || (uiLow == 0x17);
	}
	if (uiLow == 0x12)
	{
		//(dp): ora and eor adc sta lda cmp sbc.
		return true;
	}
	switch (uiOpcode)
	{
	case 0x04: case 0x14: case 0x24: case 0x34: case 0x64: case 0x74:
	case 0x06: case 0x16: case 0x26: case 0x36: case 0x46: case 0x56: case 0x66: case 0x76:
	case 0x84: case 0x94: case 0x86: case 0x96: case 0xA4: case 0xB4: case 0xA6: case 0xB6:
	case 0xC4: case 0xC6: case 0xD6: case 0xE4: case 0xE6: case 0xF6: case 0xD4:
		return true;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an instruction gets to a function's start by calling it: jsl, jsr,
// jsr (a,x), a jump (a tail call: jml, jmp, jml [a], jmp (a), jmp (a,x)) or
// the rtl of an indirect call (the address pushed, then rtl).  A branch to it
// is a loop.
//
//////////////////////////////////////////////////////////////////////////
static bool Asm816IsCall(uint8 uiOpcode)
{
	switch (uiOpcode)
	{
	case 0x22: case 0x20: case 0xFC: case 0x5C: case 0x4C: case 0xDC: case 0x6C: case 0x7C: case 0x6B:
		return true;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm816Profile::Init(void)
{
	mcFunctions.Init();
	mulliCycles = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm816Profile::Kill(void)
{
	mcFunctions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The function (its index in pasFunctions, in address order) whose code
// uiAddress is in, or ASM816_PROFILE_NO_FUNCTION.
//
//////////////////////////////////////////////////////////////////////////
static uint32 Asm816ProfileFind(CArrayTemplate<SAsm816ProfileFunction>* pasFunctions, uint32 uiAddress)
{
	size	uiLow;
	size	uiHigh;
	size	uiMiddle;

	uiLow = 0;
	uiHigh = pasFunctions->NumElements();
	while (uiLow < uiHigh)
	{
		uiMiddle = (uiLow + uiHigh) / 2;
		if (pasFunctions->Get(uiMiddle)->uiEnd <= uiAddress)
		{
			uiLow = uiMiddle + 1;
		}
		else
		{
			uiHigh = uiMiddle;
		}
	}
	if ((uiLow < pasFunctions->NumElements()) && (pasFunctions->Get(uiLow)->uiStart <= uiAddress))
	{
		return (uint32)uiLow;
	}
	return ASM816_PROFILE_NO_FUNCTION;
}


//////////////////////////////////////////////////////////////////////////
//
// Runs the program (loaded in pcMemory, pcFast reset) up to uiMaxSteps
// steps, adding what each function did to its profile.  The functions are
// the text sections in the linker's map (szMap): "  $00012345 $0000021C
// text._Label".  A call is getting to the start of a function by a call
// or a jump (Asm816IsCall); the calls being run are kept by S, so those
// entered below S are over after a return, and before a call at or above
// where they were entered.  A direct page access is counted for the function that last set
// D to D's value (tcd): runtime routines run on their caller's frame.
// Returns the steps run.
//
//////////////////////////////////////////////////////////////////////////
size CAsm816Profile::Run(CAsm65Memory* pcMemory, CFast65C816* pcFast, const char* szMap, size uiMaxSteps)
{
	CArrayTemplate<SAsm816ProfileFunction>	asFunctions;
	CArrayTemplate<SAsm816ProfileCall>		asCalls;
	SAsm816ProfileFunction*					psFunction;
	SAsm816ProfileCall*						psCall;
	SAsm816FunctionProfile*					psProfile;
	uint32*									auiDirectPageOwners;
	uint8*									auiStarts;
	const char*								szLine;
	char									szLabel[512];
	uint32									uiStart;
	uint32									uiSize;
	uint32									uiAddress;
	uint32									uiFunction;
	uint32									uiOwner;
	uint64									ulliCycles;
	uint16									uiS;
	uint8									uiOpcode;
	uint8									uiPreviousOpcode;
	size									uiSteps;
	size									ui;

	//The functions' profiles are put first (a put may move those already there), then found for each function.
	for (szLine = strstr(szMap, "Sections\n"); szLine && (strncmp(szLine, "\nDropped", 8) != 0) && (strncmp(szLine, "\nSymbols", 8) != 0); szLine = strchr(szLine + 1, '\n'))
	{
		if ((sscanf(szLine + 1, " $%x $%x text.%511s", &uiStart, &uiSize, szLabel) == 3) && (uiSize > 0) && (uiStart < ASM816_PROFILE_ADDRESSES) && !mcFunctions.Get(szLabel))
		{
			memset(mcFunctions.Put(szLabel), 0, sizeof(SAsm816FunctionProfile));
		}
	}
	asFunctions.Init();
	auiStarts = (uint8*)malloc(ASM816_PROFILE_ADDRESSES);
	memset(auiStarts, 0, ASM816_PROFILE_ADDRESSES);
	for (szLine = strstr(szMap, "Sections\n"); szLine && (strncmp(szLine, "\nDropped", 8) != 0) && (strncmp(szLine, "\nSymbols", 8) != 0); szLine = strchr(szLine + 1, '\n'))
	{
		if ((sscanf(szLine + 1, " $%x $%x text.%511s", &uiStart, &uiSize, szLabel) == 3) && (uiSize > 0) && (uiStart < ASM816_PROFILE_ADDRESSES))
		{
			psFunction = asFunctions.Add();
			psFunction->uiStart = uiStart;
			psFunction->uiEnd = uiStart + uiSize;
			psFunction->psProfile = mcFunctions.Get(szLabel);
			auiStarts[uiStart] = 1;
		}
	}

	auiDirectPageOwners = (uint32*)malloc(0x10000 * sizeof(uint32));
	memset(auiDirectPageOwners, 0xFF, 0x10000 * sizeof(uint32));
	asCalls.Init();
	ulliCycles = pcFast->NumCycles();
	uiPreviousOpcode = 0x22;
	for (uiSteps = 0; (uiSteps < uiMaxSteps) && !pcFast->IsStopped(); uiSteps++)
	{
		uiAddress = pcFast->GetProgramAddress() & (ASM816_PROFILE_ADDRESSES - 1);
		uiOpcode = pcMemory->GetByte(uiAddress);
		if (auiStarts[uiAddress] && Asm816IsCall(uiPreviousOpcode))
		{
			//A call: those that have returned are above S.
			uiS = pcFast->GetS();
			while ((asCalls.NumElements() > 0) && (asCalls.Tail()->uiS <= uiS))
			{
				asCalls.RemoveTail();
			}
			uiFunction = Asm816ProfileFind(&asFunctions, uiAddress);
			psProfile = asFunctions.Get(uiFunction)->psProfile;
			psProfile->ulliCalls++;
			for (ui = 0; ui < asCalls.NumElements(); ui++)
			{
				if (asCalls.Get(ui)->uiFunction == uiFunction)
				{
					psProfile->bRecursive = true;
					break;
				}
			}
			psCall = asCalls.Add();
			psCall->uiS = uiS;
			psCall->uiFunction = uiFunction;
		}
		if (Asm816HasDirectPageOperand(uiOpcode))
		{
			uiOwner = auiDirectPageOwners[pcFast->GetD()];
			if (uiOwner == ASM816_PROFILE_NO_FUNCTION)
			{
				uiOwner = Asm816ProfileFind(&asFunctions, uiAddress);
			}
			if (uiOwner != ASM816_PROFILE_NO_FUNCTION)
			{
				asFunctions.Get(uiOwner)->psProfile->ulliDirectPage++;
			}
		}
		pcFast->Step();
		uiPreviousOpcode = uiOpcode;
		if ((uiOpcode == 0x6B) || (uiOpcode == 0x60) || (uiOpcode == 0x40))
		{
			//A return: the calls entered below S now are over.
			uiS = pcFast->GetS();
			while ((asCalls.NumElements() > 0) && (asCalls.Tail()->uiS < uiS))
			{
				asCalls.RemoveTail();
			}
		}
		if (uiOpcode == 0x5B)
		{
			//tcd: this function's frame.
			auiDirectPageOwners[pcFast->GetD()] = Asm816ProfileFind(&asFunctions, uiAddress);
		}
	}
	mulliCycles += pcFast->NumCycles() - ulliCycles;
	asCalls.Kill();
	free(auiDirectPageOwners);
	free(auiStarts);
	asFunctions.Kill();
	return uiSteps;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the profiles in szFilename (as Write writes them) to these.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm816Profile::Read(const char* szFilename)
{
	CTextFile				cFile;
	const char*				szLine;
	char					szLabel[512];
	unsigned long long		ulliDirectPage;
	unsigned long long		ulliCalls;
	unsigned long long		ulliCycles;
	int						iRecursive;
	SAsm816FunctionProfile*	psProfile;

	cFile.Init();
	if (!cFile.Read(szFilename))
	{
		cFile.Kill();
		return false;
	}
	cFile.PassifyNewlines();
	for (szLine = cFile.Text(); szLine && *szLine; szLine = strchr(szLine, '\n'), szLine = szLine ? szLine + 1 : NULL)
	{
		if (sscanf(szLine, "cycles %llu", &ulliCycles) == 1)
		{
			mulliCycles += ulliCycles;
		}
		else if (sscanf(szLine, "%511s %llu %llu %d", szLabel, &ulliDirectPage, &ulliCalls, &iRecursive) == 4)
		{
			psProfile = mcFunctions.Get(szLabel);
			if (!psProfile)
			{
				psProfile = mcFunctions.Put(szLabel);
				memset(psProfile, 0, sizeof(SAsm816FunctionProfile));
			}
			psProfile->ulliDirectPage += ulliDirectPage;
			psProfile->ulliCalls += ulliCalls;
			psProfile->bRecursive |= iRecursive != 0;
		}
	}
	cFile.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// "cycles n", then "label directpage calls recursive" for each function
// that was called.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm816Profile::Write(const char* szFilename)
{
	CTextFile				cFile;
	CChars					sz;
	SMapIterator			sIterator;
	char*					szLabel;
	SAsm816FunctionProfile*	psProfile;
	size					uiKeySize;
	size					uiDataSize;
	bool					bMore;
	char					szNumbers[80];
	bool					bResult;

	sz.Init();
	snprintf(szNumbers, sizeof(szNumbers), "cycles %llu\n", (unsigned long long)mulliCycles);
	sz.Append(szNumbers);
	for (bMore = mcFunctions.StartIteration(&sIterator, (void**)&szLabel, &uiKeySize, (void**)&psProfile, &uiDataSize); bMore; bMore = mcFunctions.Iterate(&sIterator, (void**)&szLabel, &uiKeySize, (void**)&psProfile, &uiDataSize))
	{
		if (psProfile->ulliCalls > 0)
		{
			sz.Append(szLabel);
			snprintf(szNumbers, sizeof(szNumbers), " %llu %llu %d\n", (unsigned long long)psProfile->ulliDirectPage, (unsigned long long)psProfile->ulliCalls, psProfile->bRecursive ? 1 : 0);
			sz.Append(szNumbers);
		}
	}
	cFile.Init(sz.Text());
	bResult = cFile.Write(szFilename);
	cFile.Kill();
	sz.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm816FunctionProfile* CAsm816Profile::Get(const char* szLabel)
{
	return mcFunctions.Get(szLabel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 CAsm816Profile::NumCycles(void)
{
	return mulliCycles;
}

