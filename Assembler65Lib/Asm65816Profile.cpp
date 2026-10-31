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
#include "Asm65816Profile.h"


#define ASM65816_PROFILE_NO_FUNCTION		0xFFFFFFFF
#define ASM65816_PROFILE_ADDRESSES		0x1000000
#define ASM65816_PROFILE_NO_LINE			0xFFFFFFFF
#define ASM65816_PROFILE_BANKS			256
#define ASM65816_PROFILE_BANK_SIZE		0x10000


//A function's code: its section in the linker's map.
struct SAsm65816ProfileFunction
{
	uint32					uiStart;
	uint32					uiEnd;
	SAsm65816FunctionProfile*	psProfile;
};


//A function being run: S as it was entered (S is below this while it runs), and which function it is.
struct SAsm65816ProfileCall
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
static bool Asm65816HasDirectPageOperand(uint8 uiOpcode)
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
static bool Asm65816IsCall(uint8 uiOpcode)
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
void CAsm65816Profile::Init(void)
{
	mcFunctions.Init();
	mcLines.Init();
	mcLineEntries.Init();
	mbLineEntries = false;
	mulliCycles = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65816Profile::Kill(void)
{
	mcLineEntries.Kill();
	mcLines.Kill();
	mcFunctions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The function (its index in pasFunctions, in address order) whose code
// uiAddress is in, or ASM65816_PROFILE_NO_FUNCTION.
//
//////////////////////////////////////////////////////////////////////////
static uint32 Asm65816ProfileFind(CArrayTemplate<SAsm65816ProfileFunction>* pasFunctions, uint32 uiAddress)
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
	return ASM65816_PROFILE_NO_FUNCTION;
}


//////////////////////////////////////////////////////////////////////////
//
// Runs the program (loaded in pcMemory, pcFast reset) up to uiMaxSteps
// steps, adding what each function did to its profile.  The functions are
// the text sections in the linker's map (szMap): "  $00012345 $0000021C
// text._Label".  A call is getting to the start of a function by a call
// or a jump (Asm65816IsCall); the calls being run are kept by S, so those
// entered below S are over after a return, and before a call at or above
// where they were entered.  A direct page access is counted for the function that last set
// D to D's value (tcd): runtime routines run on their caller's frame.  With
// the program's line table (pcLines, NULL for none) each instruction's
// cycles go to its source line in its function, and the functions get the
// table's fingerprints (a function compiled from other code since forgets
// its profile).  Returns the steps run.
//
//////////////////////////////////////////////////////////////////////////
uint64 CAsm65816Profile::Run(CAsm65Memory* pcMemory, CFast65C816* pcFast, const char* szMap, uint64 uiMaxSteps, CAsm65LineTable* pcLines)
{
	CArrayTemplate<SAsm65816ProfileFunction>	asFunctions;
	CArrayTemplate<SAsm65816ProfileCall>		asCalls;
	SAsm65816ProfileFunction*					psFunction;
	SAsm65816ProfileCall*						psCall;
	SAsm65816FunctionProfile*					psProfile;
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
	uint64									ulliBefore;
	uint16									uiS;
	uint8									uiOpcode;
	uint8									uiPreviousOpcode;
	uint64									uiSteps;
	size									ui;
	CArrayChars								aszLabels;		//Per function in asFunctions.
	uint32*									apuiBanks[ASM65816_PROFILE_BANKS];	//Per address in a bank with lines: its line, or ASM65816_PROFILE_NO_LINE.
	CArrayTemplate<SAsm65816LineProfile>		asRun;			//Per line in pcLines, this run.
	SAsm65816LineProfile*						psLine;
	SAsm65LineEntry*						psEntry;
	uint32									uiLine;
	uint32									uiLastLine;
	uint32*									puiBank;
	CMapStringTemplate<SAsm65816LineProfile>	cRun;
	SAsm65816LineProfile*						psTotal;
	CChars									szKey;
	char*									szKeyText;
	SMapIterator							sIterator;
	size									uiKeySize;
	size									uiDataSize;
	bool									bMore;

	//The functions' profiles are put first (a put may move those already there), then found for each function.
	for (szLine = strstr(szMap, "Sections\n"); szLine && (strncmp(szLine, "\nDropped", 8) != 0) && (strncmp(szLine, "\nSymbols", 8) != 0); szLine = strchr(szLine + 1, '\n'))
	{
		if ((sscanf(szLine + 1, " 0x%x 0x%x text.%511s", &uiStart, &uiSize, szLabel) == 3) && (uiSize > 0) && (uiStart < ASM65816_PROFILE_ADDRESSES))
		{
			PutFunction(szLabel);
		}
	}
	if (pcLines)
	{
		Fingerprints(pcLines);
	}
	asFunctions.Init();
	aszLabels.Init();
	auiStarts = (uint8*)malloc(ASM65816_PROFILE_ADDRESSES);
	memset(auiStarts, 0, ASM65816_PROFILE_ADDRESSES);
	for (szLine = strstr(szMap, "Sections\n"); szLine && (strncmp(szLine, "\nDropped", 8) != 0) && (strncmp(szLine, "\nSymbols", 8) != 0); szLine = strchr(szLine + 1, '\n'))
	{
		if ((sscanf(szLine + 1, " 0x%x 0x%x text.%511s", &uiStart, &uiSize, szLabel) == 3) && (uiSize > 0) && (uiStart < ASM65816_PROFILE_ADDRESSES))
		{
			psFunction = asFunctions.Add();
			psFunction->uiStart = uiStart;
			psFunction->uiEnd = uiStart + uiSize;
			psFunction->psProfile = mcFunctions.Get(szLabel);
			aszLabels.Add(szLabel);
			auiStarts[uiStart] = 1;
		}
	}

	//Each address's line: the lines' code runs from their address to the next line's.
	memset(apuiBanks, 0, sizeof(apuiBanks));
	asRun.Init();
	if (pcLines)
	{
		asRun.GrowToAtLeastNumElements(pcLines->NumLines(), true, 0);
		for (ui = 0; ui < pcLines->NumLines(); ui++)
		{
			psEntry = pcLines->GetLine(ui);
			uiStart = psEntry->uiAddress & (ASM65816_PROFILE_ADDRESSES - 1);
			uiSize = (ui + 1 < pcLines->NumLines()) ? (pcLines->GetLine(ui + 1)->uiAddress & (ASM65816_PROFILE_ADDRESSES - 1)) : uiStart;
			for (uiAddress = uiStart; (psEntry->uiLine != 0) && (uiAddress < uiSize); uiAddress++)
			{
				puiBank = apuiBanks[uiAddress / ASM65816_PROFILE_BANK_SIZE];
				if (!puiBank)
				{
					puiBank = (uint32*)malloc(ASM65816_PROFILE_BANK_SIZE * sizeof(uint32));
					memset(puiBank, 0xFF, ASM65816_PROFILE_BANK_SIZE * sizeof(uint32));
					apuiBanks[uiAddress / ASM65816_PROFILE_BANK_SIZE] = puiBank;
				}
				puiBank[uiAddress % ASM65816_PROFILE_BANK_SIZE] = (uint32)ui;
			}
		}
	}

	auiDirectPageOwners = (uint32*)malloc(0x10000 * sizeof(uint32));
	memset(auiDirectPageOwners, 0xFF, 0x10000 * sizeof(uint32));
	asCalls.Init();
	ulliCycles = pcFast->NumCycles();
	uiPreviousOpcode = 0x22;
	uiLastLine = ASM65816_PROFILE_NO_LINE;
	for (uiSteps = 0; (uiSteps < uiMaxSteps) && !pcFast->IsStopped(); uiSteps++)
	{
		uiAddress = pcFast->GetProgramAddress() & (ASM65816_PROFILE_ADDRESSES - 1);
		uiOpcode = pcMemory->GetByte(uiAddress);
		if (auiStarts[uiAddress] && Asm65816IsCall(uiPreviousOpcode))
		{
			//A call: those that have returned are above S.
			uiS = pcFast->GetS();
			while ((asCalls.NumElements() > 0) && (asCalls.Tail()->uiS <= uiS))
			{
				asCalls.RemoveTail();
			}
			uiFunction = Asm65816ProfileFind(&asFunctions, uiAddress);
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
		if (Asm65816HasDirectPageOperand(uiOpcode))
		{
			uiOwner = auiDirectPageOwners[pcFast->GetD()];
			if (uiOwner == ASM65816_PROFILE_NO_FUNCTION)
			{
				uiOwner = Asm65816ProfileFind(&asFunctions, uiAddress);
			}
			if (uiOwner != ASM65816_PROFILE_NO_FUNCTION)
			{
				asFunctions.Get(uiOwner)->psProfile->ulliDirectPage++;
			}
		}
		puiBank = apuiBanks[uiAddress / ASM65816_PROFILE_BANK_SIZE];
		uiLine = puiBank ? puiBank[uiAddress % ASM65816_PROFILE_BANK_SIZE] : ASM65816_PROFILE_NO_LINE;
		if (uiLine != ASM65816_PROFILE_NO_LINE)
		{
			psLine = asRun.Get(uiLine);
			psLine->ulliInstructions++;
			if (uiAddress == (pcLines->GetLine(uiLine)->uiAddress & (ASM65816_PROFILE_ADDRESSES - 1)))
			{
				psLine->ulliEntries++;
			}
			ulliBefore = pcFast->NumCycles();
			pcFast->Step();
			psLine->ulliCycles += pcFast->NumCycles() - ulliBefore;
			uiLastLine = uiLine;
		}
		else if (uiLastLine != ASM65816_PROFILE_NO_LINE)
		{
			//Code with no line (a runtime routine) is the last line's.
			ulliBefore = pcFast->NumCycles();
			pcFast->Step();
			asRun.Get(uiLastLine)->ulliRuntimeCycles += pcFast->NumCycles() - ulliBefore;
		}
		else
		{
			pcFast->Step();
		}
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
			auiDirectPageOwners[pcFast->GetD()] = Asm65816ProfileFind(&asFunctions, uiAddress);
		}
	}
	mulliCycles += pcFast->NumCycles() - ulliCycles;

	//Each line's run goes to its function's line: in this run the most entries of its pieces' and their instructions and
	//cycles added, then added to the runs before.
	cRun.Init();
	for (ui = 0; pcLines && (ui < pcLines->NumLines()); ui++)
	{
		psLine = asRun.Get(ui);
		psEntry = pcLines->GetLine(ui);
		if (psLine->ulliInstructions == 0)
		{
			continue;
		}
		uiFunction = Asm65816ProfileFind(&asFunctions, psEntry->uiAddress & (ASM65816_PROFILE_ADDRESSES - 1));
		if (uiFunction == ASM65816_PROFILE_NO_FUNCTION)
		{
			continue;
		}
		asFunctions.Get(uiFunction)->psProfile->ulliCycles += psLine->ulliCycles + psLine->ulliRuntimeCycles;
		LineKey(&szKey, aszLabels.GetText(uiFunction), pcLines->GetFile(psEntry->uiFile), psEntry->uiLine);
		psTotal = cRun.Get(szKey.Text());
		if (!psTotal)
		{
			psTotal = cRun.Put(szKey.Text());
			memset(psTotal, 0, sizeof(SAsm65816LineProfile));
		}
		szKey.Kill();
		psTotal->ulliEntries = (psLine->ulliEntries > psTotal->ulliEntries) ? psLine->ulliEntries : psTotal->ulliEntries;
		psTotal->ulliInstructions += psLine->ulliInstructions;
		psTotal->ulliCycles += psLine->ulliCycles;
		psTotal->ulliRuntimeCycles += psLine->ulliRuntimeCycles;
	}
	for (bMore = cRun.StartIteration(&sIterator, (void**)&szKeyText, &uiKeySize, (void**)&psTotal, &uiDataSize); bMore; bMore = cRun.Iterate(&sIterator, (void**)&szKeyText, &uiKeySize, (void**)&psTotal, &uiDataSize))
	{
		mbLineEntries = false;
		psLine = mcLines.Get(szKeyText);
		if (!psLine)
		{
			psLine = mcLines.Put(szKeyText);
			memset(psLine, 0, sizeof(SAsm65816LineProfile));
		}
		psLine->ulliEntries += psTotal->ulliEntries;
		psLine->ulliInstructions += psTotal->ulliInstructions;
		psLine->ulliCycles += psTotal->ulliCycles;
		psLine->ulliRuntimeCycles += psTotal->ulliRuntimeCycles;
	}
	cRun.Kill();

	for (ui = 0; ui < ASM65816_PROFILE_BANKS; ui++)
	{
		if (apuiBanks[ui])
		{
			free(apuiBanks[ui]);
		}
	}
	asRun.Kill();
	aszLabels.Kill();
	asCalls.Kill();
	free(auiDirectPageOwners);
	free(auiStarts);
	asFunctions.Kill();
	return uiSteps;
}


//////////////////////////////////////////////////////////////////////////
//
// A line's key: "label\tline\tfile".
//
//////////////////////////////////////////////////////////////////////////
void CAsm65816Profile::LineKey(CChars* pszKey, const char* szLabel, const char* szFile, uint32 uiLine)
{
	pszKey->Init(szLabel);
	pszKey->Append('\t');
	pszKey->Append((int64)uiLine);
	pszKey->Append('\t');
	pszKey->Append(szFile);
}


//////////////////////////////////////////////////////////////////////////
//
// The function's profile, an empty one if it has none.
//
//////////////////////////////////////////////////////////////////////////
SAsm65816FunctionProfile* CAsm65816Profile::PutFunction(const char* szLabel)
{
	SAsm65816FunctionProfile*	psProfile;

	psProfile = mcFunctions.Get(szLabel);
	if (!psProfile)
	{
		psProfile = mcFunctions.Put(szLabel);
		memset(psProfile, 0, sizeof(SAsm65816FunctionProfile));
	}
	return psProfile;
}


//////////////////////////////////////////////////////////////////////////
//
// The profile of a line of szFile in the function, an empty one if it has
// none.
//
//////////////////////////////////////////////////////////////////////////
SAsm65816LineProfile* CAsm65816Profile::PutLine(const char* szLabel, const char* szFile, uint32 uiLine)
{
	SAsm65816LineProfile*	psLine;
	CChars				szKey;

	mbLineEntries = false;
	LineKey(&szKey, szLabel, szFile, uiLine);
	psLine = mcLines.Get(szKey.Text());
	if (!psLine)
	{
		psLine = mcLines.Put(szKey.Text());
		memset(psLine, 0, sizeof(SAsm65816LineProfile));
	}
	szKey.Kill();
	return psLine;
}


//////////////////////////////////////////////////////////////////////////
//
// How often the line's code was entered in all the functions it is in
// (code inlined into others is a line of its callee), 0 for none.
//
//////////////////////////////////////////////////////////////////////////
uint64 CAsm65816Profile::LineEntries(const char* szFile, uint32 uiLine)
{
	SMapIterator		sIterator;
	char*				szKey;
	const char*			szTab;
	SAsm65816LineProfile*	psLine;
	uint64*				pulliEntries;
	size				uiKeySize;
	size				uiDataSize;
	bool				bMore;
	CChars				szLineKey;

	if (!mbLineEntries)
	{
		mcLineEntries.Kill();
		mcLineEntries.Init();
		for (bMore = mcLines.StartIteration(&sIterator, (void**)&szKey, &uiKeySize, (void**)&psLine, &uiDataSize); bMore; bMore = mcLines.Iterate(&sIterator, (void**)&szKey, &uiKeySize, (void**)&psLine, &uiDataSize))
		{
			szTab = strchr(szKey, '\t');
			if (!szTab)
			{
				continue;
			}
			pulliEntries = mcLineEntries.Get(szTab + 1);
			if (!pulliEntries)
			{
				pulliEntries = mcLineEntries.Put(szTab + 1);
				*pulliEntries = 0;
			}
			*pulliEntries += psLine->ulliEntries;
		}
		mbLineEntries = true;
	}
	szLineKey.Init();
	szLineKey.Append((int64)uiLine);
	szLineKey.Append('\t');
	szLineKey.Append(szFile);
	pulliEntries = mcLineEntries.Get(szLineKey.Text());
	szLineKey.Kill();
	return pulliEntries ? *pulliEntries : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The profile of a line of szFile in the function, or NULL.
//
//////////////////////////////////////////////////////////////////////////
SAsm65816LineProfile* CAsm65816Profile::GetLine(const char* szLabel, const char* szFile, uint32 uiLine)
{
	SAsm65816LineProfile*	psLine;
	CChars				szKey;

	LineKey(&szKey, szLabel, szFile, uiLine);
	psLine = mcLines.Get(szKey.Text());
	szKey.Kill();
	return psLine;
}


//////////////////////////////////////////////////////////////////////////
//
// Removes the function's lines (its code changed).
//
//////////////////////////////////////////////////////////////////////////
void CAsm65816Profile::ForgetLines(const char* szLabel)
{
	CArrayChars		aszKeys;
	SMapIterator	sIterator;
	char*			szKey;
	void*			pvData;
	size			uiKeySize;
	size			uiDataSize;
	size			uiLabel;
	size			ui;
	bool			bMore;

	aszKeys.Init();
	uiLabel = strlen(szLabel);
	for (bMore = mcLines.StartIteration(&sIterator, (void**)&szKey, &uiKeySize, &pvData, &uiDataSize); bMore; bMore = mcLines.Iterate(&sIterator, (void**)&szKey, &uiKeySize, &pvData, &uiDataSize))
	{
		if ((strncmp(szKey, szLabel, uiLabel) == 0) && (szKey[uiLabel] == '\t'))
		{
			aszKeys.Add(szKey);
		}
	}
	for (ui = 0; ui < aszKeys.NumElements(); ui++)
	{
		mcLines.Remove(aszKeys.GetText(ui));
	}
	mbLineEntries = false;
	aszKeys.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The functions get the line table's fingerprints.  One whose profile was
// of other code (another hash) starts again.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65816Profile::Fingerprints(CAsm65LineTable* pcLines)
{
	SAsm65LineFingerprint*	psFingerprint;
	SAsm65816FunctionProfile*	psProfile;
	size					ui;

	for (ui = 0; ui < pcLines->NumFingerprints(); ui++)
	{
		psFingerprint = pcLines->GetFingerprint(ui);
		psProfile = mcFunctions.Get(psFingerprint->szLabel.Text());
		if (!psProfile)
		{
			continue;
		}
		if ((psProfile->uiHash != 0) && (psProfile->uiHash != psFingerprint->uiHash))
		{
			memset(psProfile, 0, sizeof(SAsm65816FunctionProfile));
			ForgetLines(psFingerprint->szLabel.Text());
		}
		psProfile->uiHash = psFingerprint->uiHash;
		psProfile->iTime = psFingerprint->iTime;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the profiles in szFilename (as Write writes them) to these.  A
// function whose hash differs from the one here (other code) replaces it.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65816Profile::Read(const char* szFilename)
{
	CTextFile				cFile;
	const char*				szLine;
	char					szLabel[512];
	unsigned long long		ulliDirectPage;
	unsigned long long		ulliCalls;
	unsigned long long		ulliCycles;
	unsigned long long		ulliEntries;
	unsigned long long		ulliInstructions;
	unsigned long long		ulliRuntimeCycles;
	unsigned int			uiHash;
	long long				lliTime;
	unsigned int			uiLine;
	int						iRecursive;
	int						iFile;
	const char*				szEnd;
	CChars					szFile;
	CChars					szFunction;
	SAsm65816FunctionProfile*	psProfile;
	SAsm65816LineProfile*		psLine;

	cFile.Init();
	if (!cFile.Read(szFilename))
	{
		cFile.Kill();
		return false;
	}
	cFile.PassifyNewlines();
	szFunction.Init();
	for (szLine = cFile.Text(); szLine && *szLine; szLine = strchr(szLine, '\n'), szLine = szLine ? szLine + 1 : NULL)
	{
		if (sscanf(szLine, "cycles %llu", &ulliCycles) == 1)
		{
			mulliCycles += ulliCycles;
		}
		else if (sscanf(szLine, "function %511s %llu %llu %d %x %lld %llu", szLabel, &ulliDirectPage, &ulliCalls, &iRecursive, &uiHash, &lliTime, &ulliCycles) == 7)
		{
			psProfile = PutFunction(szLabel);
			if ((uiHash != 0) && (psProfile->uiHash != 0) && (psProfile->uiHash != uiHash))
			{
				memset(psProfile, 0, sizeof(SAsm65816FunctionProfile));
				ForgetLines(szLabel);
				psProfile = mcFunctions.Get(szLabel);
			}
			psProfile->ulliDirectPage += ulliDirectPage;
			psProfile->ulliCalls += ulliCalls;
			psProfile->bRecursive |= iRecursive != 0;
			psProfile->ulliCycles += ulliCycles;
			psProfile->uiHash = uiHash;
			psProfile->iTime = lliTime;
			szFunction.Kill();
			szFunction.Init(szLabel);
		}
		else if ((sscanf(szLine, "line %u %llu %llu %llu %llu %n", &uiLine, &ulliEntries, &ulliInstructions, &ulliCycles, &ulliRuntimeCycles, &iFile) == 5) && !szFunction.Empty())
		{
			szEnd = strchr(szLine, '\n');
			szFile.Init();
			szFile.Append(szLine + iFile, szEnd ? (size)(szEnd - (szLine + iFile)) : strlen(szLine + iFile));
			psLine = PutLine(szFunction.Text(), szFile.Text(), uiLine);
			psLine->ulliEntries += ulliEntries;
			psLine->ulliInstructions += ulliInstructions;
			psLine->ulliCycles += ulliCycles;
			psLine->ulliRuntimeCycles += ulliRuntimeCycles;
			szFile.Kill();
		}
		else if (sscanf(szLine, "%511s %llu %llu %d", szLabel, &ulliDirectPage, &ulliCalls, &iRecursive) == 4)
		{
			//A profile from before line tables: "label directpage calls recursive".
			psProfile = PutFunction(szLabel);
			psProfile->ulliDirectPage += ulliDirectPage;
			psProfile->ulliCalls += ulliCalls;
			psProfile->bRecursive |= iRecursive != 0;
		}
	}
	szFunction.Kill();
	cFile.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// "cycles n", then for each function that ran "function label directpage
// calls recursive hash time cycles" (hash in hex, time in seconds since
// 1970, cycles its own code's and the runtime's it called) and its lines,
// "line number entries instructions cycles runtimecycles file", in label
// and line order.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65816Profile::Write(const char* szFilename)
{
	CTextFile				cFile;
	CChars					sz;
	CArrayChars				aszLabels;
	CArrayChars				aszKeys;
	SMapIterator			sIterator;
	char*					szKey;
	SAsm65816FunctionProfile*	psProfile;
	SAsm65816LineProfile*		psLine;
	size					uiKeySize;
	size					uiDataSize;
	size					ui;
	size					uiKey;
	size					uiLabel;
	const char*				szTab;
	bool					bMore;
	char					szNumbers[160];
	bool					bResult;

	aszLabels.Init();
	for (bMore = mcFunctions.StartIteration(&sIterator, (void**)&szKey, &uiKeySize, (void**)&psProfile, &uiDataSize); bMore; bMore = mcFunctions.Iterate(&sIterator, (void**)&szKey, &uiKeySize, (void**)&psProfile, &uiDataSize))
	{
		if ((psProfile->ulliCalls > 0) || (psProfile->ulliCycles > 0))
		{
			aszLabels.Add(szKey);
		}
	}
	aszLabels.QuickSort(true);
	aszKeys.Init();
	for (bMore = mcLines.StartIteration(&sIterator, (void**)&szKey, &uiKeySize, (void**)&psLine, &uiDataSize); bMore; bMore = mcLines.Iterate(&sIterator, (void**)&szKey, &uiKeySize, (void**)&psLine, &uiDataSize))
	{
		aszKeys.Add(szKey);
	}
	aszKeys.QuickSort(true);

	sz.Init();
	snprintf(szNumbers, sizeof(szNumbers), "cycles %llu\n", (unsigned long long)mulliCycles);
	sz.Append(szNumbers);
	uiKey = 0;
	for (ui = 0; ui < aszLabels.NumElements(); ui++)
	{
		psProfile = mcFunctions.Get(aszLabels.GetText(ui));
		snprintf(szNumbers, sizeof(szNumbers), " %llu %llu %d %x %lld %llu\n", (unsigned long long)psProfile->ulliDirectPage, (unsigned long long)psProfile->ulliCalls, psProfile->bRecursive ? 1 : 0,
				 (unsigned int)psProfile->uiHash, (long long)psProfile->iTime, (unsigned long long)psProfile->ulliCycles);
		sz.Append("function ");
		sz.Append(aszLabels.GetText(ui));
		sz.Append(szNumbers);

		//Its lines are the keys that start with its label and a tab.
		uiLabel = aszLabels.Get(ui)->Length();
		for (uiKey = 0; uiKey < aszKeys.NumElements(); uiKey++)
		{
			szKey = aszKeys.GetText(uiKey);
			if ((strncmp(szKey, aszLabels.GetText(ui), uiLabel) != 0) || (szKey[uiLabel] != '\t'))
			{
				continue;
			}
			psLine = mcLines.Get(szKey);
			szTab = strchr(szKey + uiLabel + 1, '\t');
			snprintf(szNumbers, sizeof(szNumbers), "line %d %llu %llu %llu %llu ", atoi(szKey + uiLabel + 1), (unsigned long long)psLine->ulliEntries, (unsigned long long)psLine->ulliInstructions, (unsigned long long)psLine->ulliCycles, (unsigned long long)psLine->ulliRuntimeCycles);
			sz.Append(szNumbers);
			sz.Append(szTab ? szTab + 1 : "");
			sz.AppendNewLine();
		}
	}
	cFile.Init(sz.Text());
	bResult = cFile.Write(szFilename);
	cFile.Kill();
	sz.Kill();
	aszKeys.Kill();
	aszLabels.Kill();
	return bResult;
}


//A source line's run in all the functions its code is in, for WriteReport.
struct SAsm65816ReportLine
{
	CChars	szFile;
	uint32	uiLine;
	uint64	ulliEntries;
	uint64	ulliCycles;
	uint64	ulliRuntimeCycles;
	CChars	szFunction;		//The function its code ran the most cycles in.
	uint64	ulliFunctionCycles;
};


//////////////////////////////////////////////////////////////////////////
//
// The most cycles first.
//
//////////////////////////////////////////////////////////////////////////
static int Asm65816CompareReportLines(const void* pv1, const void* pv2)
{
	SAsm65816ReportLine*	ps1;
	SAsm65816ReportLine*	ps2;

	ps1 = *(SAsm65816ReportLine**)pv1;
	ps2 = *(SAsm65816ReportLine**)pv2;
	if (ps1->ulliCycles + ps1->ulliRuntimeCycles != ps2->ulliCycles + ps2->ulliRuntimeCycles)
	{
		return (ps1->ulliCycles + ps1->ulliRuntimeCycles > ps2->ulliCycles + ps2->ulliRuntimeCycles) ? -1 : 1;
	}
	return 0;
}


//A function's cycles, for WriteReport.
struct SAsm65816ReportFunction
{
	const char*				szLabel;
	SAsm65816FunctionProfile*	psProfile;
};


//////////////////////////////////////////////////////////////////////////
//
// The most cycles first.
//
//////////////////////////////////////////////////////////////////////////
static int Asm65816CompareReportFunctions(const void* pv1, const void* pv2)
{
	SAsm65816ReportFunction*	ps1;
	SAsm65816ReportFunction*	ps2;

	ps1 = (SAsm65816ReportFunction*)pv1;
	ps2 = (SAsm65816ReportFunction*)pv2;
	if (ps1->psProfile->ulliCycles != ps2->psProfile->ulliCycles)
	{
		return (ps1->psProfile->ulliCycles > ps2->psProfile->ulliCycles) ? -1 : 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The text of line uiLine (1 based) of the file, without its indent, or
// empty if the file can't be read.  Files read are kept in pcFiles.
//
//////////////////////////////////////////////////////////////////////////
static void Asm65816SourceText(CMapStringTemplate<CChars>* pcFiles, const char* szFile, uint32 uiLine, CChars* pszText)
{
	CChars*		pszContents;
	CTextFile	cFile;
	const char*	sz;
	uint32		ui;

	pszContents = pcFiles->Get(szFile);
	if (!pszContents)
	{
		pszContents = pcFiles->Put(szFile);
		cFile.Init();
		if (cFile.Read(szFile))
		{
			cFile.PassifyNewlines();
			pszContents->Init(cFile.Text());
		}
		else
		{
			pszContents->Init();
		}
		cFile.Kill();
	}
	sz = pszContents->Text();
	for (ui = 1; (ui < uiLine) && sz && *sz; ui++)
	{
		sz = strchr(sz, '\n');
		sz = sz ? sz + 1 : NULL;
	}
	if (!sz)
	{
		return;
	}
	while ((*sz == ' ') || (*sz == '\t'))
	{
		sz++;
	}
	for (; (*sz != '\0') && (*sz != '\n'); sz++)
	{
		pszText->Append(*sz);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A report of where the cycles went: the uiMost source lines that ran the
// most cycles (in all the functions their code is in: inlined code is a
// line of its callee), each with its share of the run, the times its code
// was entered, the function it ran the most in and its text, then the
// uiMost functions whose own code ran the most cycles.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65816Profile::WriteReport(const char* szFilename, size uiMost)
{
	CMapStringTemplate<SAsm65816ReportLine>	cByLine;		//By "line\tfile".
	CMapStringTemplate<CChars>				cFiles;
	CArrayTemplate<SAsm65816ReportLine*>		apsLines;
	CArrayTemplate<SAsm65816ReportFunction>	asFunctions;
	SAsm65816ReportFunction*					psFunction;
	SAsm65816ReportLine*						psReport;
	SAsm65816LineProfile*						psLine;
	SAsm65816FunctionProfile*					psProfile;
	SMapIterator							sIterator;
	char*									szKey;
	const char*								szTab;
	CChars									szLineKey;
	CChars									sz;
	CChars*									psz;
	const char*								szName;
	CTextFile								cFile;
	size									uiKeySize;
	size									uiDataSize;
	size									uiLabel;
	size									ui;
	bool									bMore;
	char									szNumbers[160];
	bool									bResult;

	cByLine.Init();
	for (bMore = mcLines.StartIteration(&sIterator, (void**)&szKey, &uiKeySize, (void**)&psLine, &uiDataSize); bMore; bMore = mcLines.Iterate(&sIterator, (void**)&szKey, &uiKeySize, (void**)&psLine, &uiDataSize))
	{
		szTab = strchr(szKey, '\t');
		if (!szTab)
		{
			continue;
		}
		uiLabel = (size)(szTab - szKey);
		szLineKey.Init(szTab + 1);
		psReport = cByLine.Get(szLineKey.Text());
		if (!psReport)
		{
			psReport = cByLine.Put(szLineKey.Text());
			memset(psReport, 0, sizeof(SAsm65816ReportLine));
			psReport->szFile.Init(strchr(szTab + 1, '\t') ? strchr(szTab + 1, '\t') + 1 : "");
			psReport->uiLine = (uint32)atoi(szTab + 1);
			psReport->szFunction.Init();
		}
		psReport->ulliEntries += psLine->ulliEntries;
		psReport->ulliCycles += psLine->ulliCycles;
		psReport->ulliRuntimeCycles += psLine->ulliRuntimeCycles;
		if (psLine->ulliCycles > psReport->ulliFunctionCycles)
		{
			psReport->ulliFunctionCycles = psLine->ulliCycles;
			psReport->szFunction.Kill();
			psReport->szFunction.Init();
			psReport->szFunction.Append(szKey, uiLabel);
		}
		szLineKey.Kill();
	}

	apsLines.Init();
	for (bMore = cByLine.StartIteration(&sIterator, (void**)&szKey, &uiKeySize, (void**)&psReport, &uiDataSize); bMore; bMore = cByLine.Iterate(&sIterator, (void**)&szKey, &uiKeySize, (void**)&psReport, &uiDataSize))
	{
		apsLines.Add(&psReport);
	}
	apsLines.Sort(Asm65816CompareReportLines);

	sz.Init();
	cFiles.Init();
	snprintf(szNumbers, sizeof(szNumbers), "%llu cycles\n\nLines (cycles with the runtime's it called, %% of all, its own code's, times entered, file(line) in function: text)\n", (unsigned long long)mulliCycles);
	sz.Append(szNumbers);
	for (ui = 0; (ui < apsLines.NumElements()) && (ui < uiMost); ui++)
	{
		psReport = *apsLines.Get(ui);
		snprintf(szNumbers, sizeof(szNumbers), "%12llu %5.2f%% %12llu %10llu  ", (unsigned long long)(psReport->ulliCycles + psReport->ulliRuntimeCycles), (mulliCycles > 0) ? (100.0 * (double)(psReport->ulliCycles + psReport->ulliRuntimeCycles) / (double)mulliCycles) : 0.0, (unsigned long long)psReport->ulliCycles, (unsigned long long)psReport->ulliEntries);
		sz.Append(szNumbers);
		szName = strrchr(psReport->szFile.Text(), '/');
		szName = szName ? szName : strrchr(psReport->szFile.Text(), '\\');
		sz.Append(szName ? szName + 1 : psReport->szFile.Text());
		sz.Append('(');
		sz.Append((int64)psReport->uiLine);
		sz.Append(") in ");
		sz.Append(psReport->szFunction);
		sz.Append(": ");
		Asm65816SourceText(&cFiles, psReport->szFile.Text(), psReport->uiLine, &sz);
		sz.AppendNewLine();
	}

	asFunctions.Init();
	for (bMore = mcFunctions.StartIteration(&sIterator, (void**)&szKey, &uiKeySize, (void**)&psProfile, &uiDataSize); bMore; bMore = mcFunctions.Iterate(&sIterator, (void**)&szKey, &uiKeySize, (void**)&psProfile, &uiDataSize))
	{
		if (psProfile->ulliCycles > 0)
		{
			psFunction = asFunctions.Add();
			psFunction->szLabel = szKey;
			psFunction->psProfile = psProfile;
		}
	}
	asFunctions.Sort(Asm65816CompareReportFunctions);
	sz.Append("\nFunctions (cycles of its own code and the runtime's it called, % of all, calls, function)\n");
	for (ui = 0; (ui < asFunctions.NumElements()) && (ui < uiMost); ui++)
	{
		psFunction = asFunctions.Get(ui);
		snprintf(szNumbers, sizeof(szNumbers), "%12llu %5.2f%% %10llu  ", (unsigned long long)psFunction->psProfile->ulliCycles, (mulliCycles > 0) ? (100.0 * (double)psFunction->psProfile->ulliCycles / (double)mulliCycles) : 0.0, (unsigned long long)psFunction->psProfile->ulliCalls);
		sz.Append(szNumbers);
		sz.Append(psFunction->szLabel);
		sz.AppendNewLine();
	}

	cFile.Init(sz.Text());
	bResult = cFile.Write(szFilename);
	cFile.Kill();
	sz.Kill();
	asFunctions.Kill();
	apsLines.Kill();
	for (bMore = cFiles.StartIteration(&sIterator, (void**)&szKey, &uiKeySize, (void**)&psz, &uiDataSize); bMore; bMore = cFiles.Iterate(&sIterator, (void**)&szKey, &uiKeySize, (void**)&psz, &uiDataSize))
	{
		psz->Kill();
	}
	cFiles.Kill();
	for (bMore = cByLine.StartIteration(&sIterator, (void**)&szKey, &uiKeySize, (void**)&psReport, &uiDataSize); bMore; bMore = cByLine.Iterate(&sIterator, (void**)&szKey, &uiKeySize, (void**)&psReport, &uiDataSize))
	{
		psReport->szFile.Kill();
		psReport->szFunction.Kill();
	}
	cByLine.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65816FunctionProfile* CAsm65816Profile::Get(const char* szLabel)
{
	return mcFunctions.Get(szLabel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 CAsm65816Profile::NumCycles(void)
{
	return mulliCycles;
}

