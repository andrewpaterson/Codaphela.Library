/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include <string.h>
#include "IRW65C816BackendInternal.h"
#include "IRW65C816Code.h"
#include "IRW65C816Costs.h"


//The W65C816 back end: a function's code from its IR (GenerateFunction, GenerateBody), its arguments' places
//(the register rule, FunctionArguments), its entry and exit (Prologue, RegisterStub, Forwarder, Epilogue) and its
//blocks' jumps and phis.  The rest is in IR816Places, IR816Instructions (with IR816Floats, IR816Shifts,
//IR816Multiplies, IR816Compares, IR816Fusion and IR816Calls), IR816Frames, IR816Lines, IR816Branches, IR816Peephole,
//IR816DeadStores, IR816Flow and IR816Runtime, and what they share in IRW65C816BackendInternal.h.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::Init(CIRModule* pcModule, int* piLabels, CScratchPadAllocator* pcArena)
{
	//The working arrays and strings are in the arena, which is this thread's scratch until Kill.
	mpcArena = pcArena;
	mpcOldScratch = pcArena ? SetScratch(pcArena) : NULL;
	InitBackend(pcModule, piLabels, CIRW65C816Backend::GetScratch());
	masTemporaries.Init(CIRW65C816Backend::GetScratch());
	mcPlaces.Init(this);
	mcSelector.Init(this);
	mcCalls.Init(this);
	mcFusion.Init(this);
	mcFolds.Init(this);
	mpcTemporariesOf = NULL;
	masCopies.Init(CIRW65C816Backend::GetScratch());
	masCopySteps.Init(CIRW65C816Backend::GetScratch());
	maiParameters.Init(CIRW65C816Backend::GetScratch());
	mauiPendingArguments.Init(CIRW65C816Backend::GetScratch());
	mcDominators.Init();
	mcLoops.Init();
	mcBackendPasses.Init();
	mpcDominators = NULL;
	mbUnmasked = false;
	masArguments.Init(CIRW65C816Backend::GetScratch());
	msFrame.Init(0, false);
	miFused = 0;
	mpsWritten = NULL;
	mpsDivided = NULL;
	mbMathTables = mpcOptions->mbMathTables;
	mpcCode = NULL;
	mszWritten.Init(CIRW65C816Backend::GetScratch());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::Kill(void)
{
	mszWritten.Kill();
	masArguments.Kill();
	mauiPendingArguments.Kill();
	maiParameters.Kill();
	masCopySteps.Kill();
	masCopies.Kill();
	mcFolds.Kill();
	mcFusion.Kill();
	mcCalls.Kill();
	mcSelector.Kill();
	mcPlaces.Kill();
	masTemporaries.Kill();
	mcLoops.Kill();
	mcBackendPasses.Kill();
	mcDominators.Kill();
	CIRBackend::Kill();

	//The thread's scratch is what it was, and the arena is emptied (unless the thread had it already: then it is
	//whoever gave it to the thread who empties it).
	if (mpcArena)
	{
		SetScratch(mpcOldScratch);
		if (mpcOldScratch != mpcArena)
		{
			mpcArena->GetScratchPad()->Recycle((lsize)IR816_ARENA_KEPT);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::Line(const char* szText)
{
	mpszOut->Append("\t\t");
	mpszOut->Append(szText);
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// szText followed by a number.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::Line(const char* szText, int64 lliValue)
{
	mpszOut->Append("\t\t");
	mpszOut->Append(szText);
	mpszOut->Append(lliValue);
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// A value's place in the frame: a word for i1 to i16 (zero extended), two for 32 bits, four for 64.
//
//////////////////////////////////////////////////////////////////////////
int CIRW65C816Backend::ValueBytes(SIRType sType)
{
	switch (sType.uiKind)
	{
	case IRTK_I1:
	case IRTK_I8:
	case IRTK_I16:
		return 2;
	case IRTK_I32:
	case IRTK_F32:
		return 4;
	case IRTK_I64:
	case IRTK_F64:
		return 8;
	case IRTK_Pointer:
		if (mpcModule->GetPointerSize(sType.uiSpace) != IR816_POINTER_BYTES)
		{
			Error("Only 4 byte pointers are done yet");
		}
		return IR816_POINTER_BYTES;
	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIRW65C816Backend::Words(SIRType sType)
{
	return ValueBytes(sType) / 2;
}


//////////////////////////////////////////////////////////////////////////
//
// A 64 bit result is written through a hidden pointer.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Backend::IsHiddenResult(SIRType sType)
{
	return ValueBytes(sType) == 8;
}


thread_local CMallocator*	gpcIR816Scratch = NULL;	//SetScratch: this thread's, else NULL.


//////////////////////////////////////////////////////////////////////////
//
// The back end's passes on this thread allocate their working arrays and
// strings with pcScratch (an arena, CCPP65IRFrontEnd::GenerateCode's for
// one function), or with the system allocator when it is NULL.  Nothing
// they allocate with it outlives the back end.  Returns the one it was, to
// set again after.
//
//////////////////////////////////////////////////////////////////////////
CMallocator* CIRW65C816Backend::SetScratch(CMallocator* pcScratch)
{
	CMallocator*	pcOld;

	pcOld = gpcIR816Scratch;
	gpcIR816Scratch = pcScratch;
	return pcOld;
}


//////////////////////////////////////////////////////////////////////////
//
// The mallocator for the back end's working arrays and strings (SetScratch).
//
//////////////////////////////////////////////////////////////////////////
CMallocator* CIRW65C816Backend::GetScratch(void)
{
	return gpcIR816Scratch ? gpcIR816Scratch : &gcSystemAllocator;
}


//////////////////////////////////////////////////////////////////////////
//
// A parameter is read from its arrival place (PlaceTemporaries), which is
// also where an argaddr of it points.  A store through that address would
// change what the parameter reads after it, where the parameter is the
// value passed.  So a parameter both addressed and used is copied first
// thing in the entry block, and its uses read the copy, which has a place
// of its own (AddRegions shares no arrival place in a function with an
// argaddr).  The front end reads an addressed parameter only through its
// address, so this is for IR that mixes the two.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::CopyAddressedParameters(void)
{
	CIRBlock*		pcBlock;
	CIRBlock*		pcEntry;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRReference	sParameter;
	SIRReference	sCopy;
	size			uiParameter;
	size			ui;
	size			uj;
	size			uk;
	size			uiIndex;
	bool			bAddressed;
	bool			bUsed;

	pcEntry = mpcFunction->GetEntry();
	for (uiParameter = 0; uiParameter < mpcFunction->masParameters.NumElements(); uiParameter++)
	{
		sParameter = *mpcFunction->masParameters.Get(uiParameter);
		bAddressed = false;
		bUsed = false;
		for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
		{
			pcBlock = mpcFunction->GetBlock(ui);
			for (uj = 0; uj < pcBlock->masPhis.NumElements(); uj++)
			{
				psPhi = pcBlock->masPhis.Get(uj);
				for (uk = 0; uk < psPhi->asArguments.NumElements(); uk++)
				{
					bUsed |= psPhi->asArguments.Get(uk)->sValue.Equals(sParameter);
				}
			}
			for (uj = 0; uj < pcBlock->masInstructions.NumElements(); uj++)
			{
				psInstruction = pcBlock->masInstructions.Get(uj);
				if ((psInstruction->uiOpcode == IRO_ArgAddress) && ((size)mpcFunction->GetSigned(psInstruction->asArguments[0]) == uiParameter))
				{
					bAddressed = true;
				}
				bUsed |= psInstruction->asArguments[0].Equals(sParameter) || psInstruction->asArguments[1].Equals(sParameter);
			}
			bUsed |= pcBlock->msJump.sArgument.Equals(sParameter);
		}
		if (!bAddressed || !bUsed)
		{
			continue;
		}

		uiIndex = 0;
		sCopy = InsertBefore(pcEntry, &uiIndex, IRO_Copy, mpcFunction->GetTemporaryType(sParameter), sParameter, IRNone());
		for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
		{
			pcBlock = mpcFunction->GetBlock(ui);
			for (uj = 0; uj < pcBlock->masPhis.NumElements(); uj++)
			{
				psPhi = pcBlock->masPhis.Get(uj);
				for (uk = 0; uk < psPhi->asArguments.NumElements(); uk++)
				{
					if (psPhi->asArguments.Get(uk)->sValue.Equals(sParameter))
					{
						psPhi->asArguments.Get(uk)->sValue = sCopy;
					}
				}
			}
			for (uj = ((pcBlock == pcEntry) ? 1 : 0); uj < pcBlock->masInstructions.NumElements(); uj++)
			{
				psInstruction = pcBlock->masInstructions.Get(uj);
				for (uk = 0; uk < IR_INSTRUCTION_ARGUMENTS; uk++)
				{
					if (psInstruction->asArguments[uk].Equals(sParameter))
					{
						psInstruction->asArguments[uk] = sCopy;
					}
				}
			}
			if (pcBlock->msJump.sArgument.Equals(sParameter))
			{
				pcBlock->msJump.sArgument = sCopy;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The function's hidden result pointer (when it returns 64 bits) and
// parameters as its callers pass them (masArguments, the frame's register
// words): in registers when its symbol is marked for them and it isn't
// variadic.  The frame starts empty.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::FunctionArguments(void)
{
	SIRRegisterArgument*	psArgument;
	size			ui;
	int32			iRegisterWords;
	bool			bVariadic;

	masArguments.FakeSetUsedElements(0);
	if (IsHiddenResult(mpcFunction->msReturnType))
	{
		psArgument = masArguments.Add();
		psArgument->iUnits = 2;
		psArgument->iRegister = -1;
	}
	for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
	{
		psArgument = masArguments.Add();
		psArgument->iUnits = Words(mpcFunction->GetTemporaryType(*mpcFunction->masParameters.Get(ui)));
		psArgument->iRegister = -1;
	}
	bVariadic = (mpcFunction->muiAttributes & IRA_Variadic) != 0;
	iRegisterWords = (IsRegisterCall(IRReference(IRRK_Symbol, mpcFunction->muiSymbol)) && !bVariadic) ? AssignRegisters(&masArguments, IR816_ARGUMENT_REGISTERS, IR816_ARGUMENT_REGISTERS) : 0;
	msFrame.Init(iRegisterWords, bVariadic);
}


//////////////////////////////////////////////////////////////////////////
//
// An empty frame: none of its parts placed.
//
//////////////////////////////////////////////////////////////////////////
void SIRW65C816Frame::Init(int32 iRegisterWordsIn, bool bVariadicIn)
{
	iFrameBytes = 0;
	iSavedS = -1;
	iRegisterWords = iRegisterWordsIn;
	iRegisterBase = -1;
	iArgumentBytes = 0;
	iHiddenResult = -1;
	iDirectPageWeight = 0;
	bLoops = false;
	bVariadic = bVariadicIn;
	bNoResult = false;
}


//////////////////////////////////////////////////////////////////////////
//
// A part of iBytes (in whole words) on the frame's top.  Returns its place.
//
//////////////////////////////////////////////////////////////////////////
int32 SIRW65C816Frame::Add(int32 iBytes)
{
	int32	iPlace;

	iPlace = iFrameBytes + 1;
	iFrameBytes += (iBytes + 1) & ~1;
	return iPlace;
}


//////////////////////////////////////////////////////////////////////////
//
// An argument's place (D + offset): in the frame's top when it came in
// registers (once Layout has placed the register words), else the next
// pushed one (*piPushed, moved past it).
//
//////////////////////////////////////////////////////////////////////////
int32 CIRW65C816Backend::ArgumentPlace(SIRRegisterArgument* psArgument, int32* piPushed)
{
	int32	iPlace;

	if (psArgument->iRegister >= 0)
	{
		return msFrame.RegisterWord(psArgument->iRegister);
	}
	iPlace = *piPushed;
	*piPushed += 2 * psArgument->iUnits;
	return iPlace;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Backend::GenerateFunction(CIRFunction* pcFunction, const char* szLabel, CChars* pszDest, CCPPErrors* pcErrors)
{
	size			uiStart;
	CIRW65C816Code	cCode;

	mpcFunction = pcFunction;
	mpcErrors = pcErrors;
	mbResult = true;
	uiStart = pszDest->Length();
	muiSource = 0;

	//The code is written into cCode as it is made, then printed into pszDest.
	cCode.Init();
	mpcCode = &cCode;
	mszWritten.Clear();
	mpszOut = &mszWritten;
	mcMarkers.Begin();

	ExpandMultiplies();
	CopyAddressedParameters();
	pcFunction->Renumber();
	pcFunction->ComputePredecessors();
	mcBackendPasses.OrderBlocks(pcFunction, true);
	mcBackendPasses.SinkSteps(pcFunction);
	pcFunction->Renumber();

	//The blocks' dominators and loops, for Layout (the loop depths PackTemporaries and AllocateX weigh by) and
	//GenerateBody (the jumps round loops, AlignFrame's estimate).  Nothing after changes the blocks or their jumps.
	pcFunction->ComputePredecessors();
	mpcModule->Count(CPPCC_Dominators);
	mcDominators.Compute(pcFunction);
	mcLoops.Compute(pcFunction, &mcDominators);
	FunctionArguments();

	//A function that only passes its arguments on to another (a wrapper) jumps to it.
	CChars	szJump;

	szJump.Init(CIRW65C816Backend::GetScratch());
	if (!mpcOptions->mbDebuggable && mcCalls.Forwarder(&szJump))
	{
		mcCalls.GenerateForwarder(szLabel, &szJump);
		szJump.Kill();
		Flush();
		cCode.Write(pszDest, uiStart);
		cCode.Kill();
		mpcCode = NULL;
		mpszOut = pszDest;
		pszDest->AppendNewLine();
		return mbResult;
	}
	szJump.Kill();

	mcPlaces.Layout();
	msFrame.bNoResult = ReturnsNothingInA();
	if (msFrame.iFrameBytes > IR816_MAX_FRAME_BYTES)
	{
		CChars	szBytes;

		szBytes.Init(CIRW65C816Backend::GetScratch());
		szBytes.Append(msFrame.iFrameBytes);
		Error("The frame is too big for the W65C816's stack (bytes)", szBytes.Text());
		szBytes.Kill();
		cCode.Kill();
		mpcCode = NULL;
		mpszOut = pszDest;
		return false;
	}
	GenerateBody(szLabel);
	cCode.Write(pszDest, uiStart);
	cCode.Kill();
	mpcCode = NULL;
	mpszOut = pszDest;
	if (!mpcOptions->mbDebuggable)
	{
		DropFrame(uiStart);
	}
	if (mpcOptions->mbExceptions)
	{
		IR816ClearExitCarries(pszDest, uiStart);
	}
	PutMarkers(pszDest, uiStart);
	pszDest->AppendNewLine();
	return mbResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether no return leaves a result in A: the function returns nothing, or
// its result through the hidden result pointer.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Backend::ReturnsNothingInA(void)
{
	size		ui;
	SIRJump*	psJump;

	if (msFrame.iHiddenResult >= 0)
	{
		return true;
	}
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		psJump = &mpcFunction->GetBlock(ui)->msJump;
		if ((psJump->uiKind == IRJ_Return) && !psJump->sType.IsNone())
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether Garth Wilson's product table (MathMult) is in memory: an i8
// multiply is then a lookup in place (ByteMultiply).  Without it the
// program is linked with IRW65C816Runtime-MathSafe.asm, which doesn't read it
// (CCPP65Builder::SetMathTables).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::SetMathTables(bool bMathTables)
{
	mbMathTables = bMathTables;
}


//////////////////////////////////////////////////////////////////////////
//
// What AlignFrame needs of the function GenerateFunction just wrote.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::GetFrame(SIRW65C816Frame* psFrame)
{
	*psFrame = msFrame;
}


//////////////////////////////////////////////////////////////////////////
//
// The function's frame, for FinishFunction.
//
//////////////////////////////////////////////////////////////////////////
CIRWrittenFunction* CIRW65C816Backend::TakeWritten(void)
{
	CIRW65C816Written*	pcWritten;

	pcWritten = NewMalloc<CIRW65C816Written>();
	pcWritten->Init();
	pcWritten->msFrame = msFrame;
	memcpy(pcWritten->mauiPlaceBytes, mauiPlaceBytes, sizeof(mauiPlaceBytes));
	AddVariableSlots(pcWritten);
	return pcWritten;
}


//////////////////////////////////////////////////////////////////////////
//
// A variable's slot is at D + its slot, a parameter read where it was
// passed (its argaddr) at D + the parameter's place.
//
//////////////////////////////////////////////////////////////////////////
int32 CIRW65C816Backend::VariableSlotPlace(SIRInstruction* psSlot)
{
	if (psSlot->uiOpcode == IRO_ArgAddress)
	{
		return *maiParameters.Get((size)mpcFunction->GetConstant(psSlot->asArguments[0])->ulliBits);
	}
	return masTemporaries.Get(psSlot->sResult.Index())->iSlot;
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes of a variable's slot: a slot's size, a parameter's type's.
//
//////////////////////////////////////////////////////////////////////////
int32 CIRW65C816Backend::VariableSlotBytes(SIRInstruction* psSlot)
{
	size	uiParameter;

	if (psSlot->uiOpcode == IRO_ArgAddress)
	{
		uiParameter = (size)mpcFunction->GetConstant(psSlot->asArguments[0])->ulliBits;
		return (int32)mpcModule->GetSize(mpcFunction->GetTemporaryType(*mpcFunction->masParameters.Get(uiParameter)));
	}
	return (int32)mpcFunction->GetConstant(psSlot->asArguments[0])->ulliBits;
}


//////////////////////////////////////////////////////////////////////////
//
// The function's frame is put on a page if that is worth it, now that the
// unit's calls are known (AlignFrame; a listing's source markers and a
// debugger's place markers are taken out while it looks at the code), and
// then its exits keep A in Y only when they must (IR816TidyExits).  A
// function nothing was kept for has an empty frame.  Then a place marker
// at frame bytes the code never reads says none (UnreadPlaces).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::FinishFunction(CChars* pszCode, const char* szLabel, CIRWrittenFunction* pcWritten, bool bRecursive, CAsm65Profile* pcProfile)
{
	CIRSourceMarkers	cMarkers;
	SIRW65C816Frame		sFrame;
	CIRW65C816Code		cCode;
	uint8				abRead[IR816_DIRECT_PAGE];
	bool				bPlaces;

	if (pcWritten)
	{
		sFrame = ((CIRW65C816Written*)pcWritten)->msFrame;
	}
	else
	{
		memset(&sFrame, 0, sizeof(sFrame));
	}
	bPlaces = pcWritten && (strstr(pszCode->Text(), IR_PLACE_MARKER) != NULL);
	cMarkers.Init();
	cMarkers.Take(pszCode, 0);
	if (!mpcOptions->mbDebuggable)
	{
		AlignFrame(pszCode, szLabel, &sFrame, bRecursive, pcProfile);
	}
	IR816TidyExits(pszCode, sFrame.bNoResult);
	if (mpcOptions->mbExceptions)
	{
		//An exit AlignFrame rewrote no longer adds to S, which is what left the carry clear.
		IR816ClearExitCarries(pszCode, 0);
	}
	if (bPlaces)
	{
		cCode.Init();
		cCode.Load(pszCode, 0);
		if (IR816FindReads(&cCode, 0, abRead))
		{
			memset(abRead, 1, sizeof(abRead));
		}
		cCode.Kill();
	}
	cMarkers.Put(pszCode, 0);
	cMarkers.Kill();
	if (bPlaces)
	{
		UnreadPlaces(pszCode, (CIRW65C816Written*)pcWritten, abRead);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Where a value is kept, for a debugger (CIRBackend::DebugPlace): X, or
// its frame place (D + offset; the function's base is D); a slot's
// address its value.  A value with no place (only in A, folded) has none.  The bytes of the place are noted
// for UnreadPlaces.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Backend::DebugPlace(SIRReference sValue, CChars* pszPlace)
{
	int32	iOffset;
	int32	iBytes;

	if (mcPlaces.InX(sValue))
	{
		pszPlace->Append("register, X");
		return true;
	}
	if (masTemporaries.Get(sValue.Index())->iSlot >= 0)
	{
		pszPlace->Append("frameaddress, ")->Append(masTemporaries.Get(sValue.Index())->iSlot);
		return true;
	}
	iOffset = mcPlaces.OffsetOf(sValue);
	if (iOffset < 0)
	{
		return false;
	}
	if (iOffset < IR816_DIRECT_PAGE)
	{
		iBytes = mcPlaces.PlaceBytes((int32)sValue.Index());
		if (iBytes > mauiPlaceBytes[iOffset])
		{
			mauiPlaceBytes[iOffset] = (uint8)iBytes;
		}
	}
	pszPlace->Append("frame, ")->Append(iOffset);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The text passes take out stores to direct page words nothing reads
// (RemoveUnreadStores: neither byte; AlignFrame's argument copies), so a
// value with a word of its place (pcWritten's mauiPlaceBytes) not read
// (abRead) in the finished code may never be there: its place markers say
// none.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::UnreadPlaces(CChars* pszCode, CIRW65C816Written* pcWritten, uint8* abRead)
{
	CChars		szOut;
	const char*	szText;
	const char*	szEnd;
	const char*	szPlace;
	size		uiMarker;
	int32		iOffset;
	int32		iBytes;
	int32		iByte;
	bool		bRead;

	uiMarker = strlen(IR_PLACE_MARKER);
	szOut.Init();
	for (szText = pszCode->Text(); *szText != '\0'; szText = (*szEnd == '\n') ? szEnd + 1 : szEnd)
	{
		szEnd = strchr(szText, '\n');
		if (!szEnd)
		{
			szEnd = szText + strlen(szText);
		}
		szPlace = (strncmp(szText, IR_PLACE_MARKER, uiMarker) == 0) ? strchr(szText + uiMarker, ' ') : NULL;
		if (szPlace && (szPlace < szEnd) && (strncmp(szPlace, " frame, ", 8) == 0))
		{
			iOffset = (int32)strtol(szPlace + 8, NULL, 10);
			iBytes = ((iOffset >= 0) && (iOffset < IR816_DIRECT_PAGE)) ? pcWritten->mauiPlaceBytes[iOffset] : 0;
			bRead = true;
			for (iByte = iOffset; (iByte < iOffset + iBytes) && (iByte + 1 < IR816_DIRECT_PAGE); iByte += 2)
			{
				bRead &= (abRead[iByte] != 0) || (abRead[iByte + 1] != 0);
			}
			if (!bRead)
			{
				szOut.AppendSubString(szText, szPlace);
				szOut.Append(" none");
				szOut.AppendNewLine();
				continue;
			}
		}
		szOut.AppendSubString(szText, (*szEnd == '\n') ? szEnd + 1 : szEnd);
	}
	pszCode->Set(szOut.Text());
	szOut.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The options, and from them whether the code may read the maths tables
// (SetMathTables changes it for this back end alone).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::SetOptions(CCPPCompileOptions* pcOptions)
{
	CIRBackend::SetOptions(pcOptions);
	mbMathTables = mpcOptions->mbMathTables;
}


//////////////////////////////////////////////////////////////////////////
//
// A mul by a constant is a runtime call (ExpandMultiplies); the cycles are
// IRW65C816Costs', which the cost of a divide's reciprocal reads too.
//
//////////////////////////////////////////////////////////////////////////
int CIRW65C816Backend::MultiplyCost(SIRType sType, uint64 ulliMultiplier)
{
	return IR816MultiplyCallCycles(Words(sType));
}


//////////////////////////////////////////////////////////////////////////
//
// ExpandMultiplies' shifts and adds for iDigits signed digits: the shifts
// add up to the top digit's place, each costs its setting up, and a top
// digit of -1 is a neg.
//
//////////////////////////////////////////////////////////////////////////
int CIRW65C816Backend::ShiftAddCost(SIRType sType, int* aiPlaces, int* aiSigns, int iDigits)
{
	return IR816ShiftAddCycles(Words(sType), aiPlaces, aiSigns, iDigits);
}


//////////////////////////////////////////////////////////////////////////
//
// The lines written so far (mszWritten) added to the code, and to the
// source markers' lines (as Take would find them); a line not yet ended
// waits.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::Flush(void)
{
	const char*	szText;
	const char*	szLine;
	const char*	szEnd;
	size		uiTaken;

	szText = mszWritten.Text();
	uiTaken = 0;
	for (szLine = szText; (szEnd = strchr(szLine, '\n')) != NULL; szLine = szEnd + 1)
	{
		if (strncmp(szLine, IR_PLACE_MARKER, StrLen(IR_PLACE_MARKER)) == 0)
		{
			//A debugger's place marker (CIRBackend::DebugPlaceMarker): kept apart from the code, as Take would.
			mcMarkers.AddMarker(szLine, (size)(szEnd - szLine));
		}
		else
		{
			mpcCode->Append(szLine, (size)(szEnd - szLine));
			if (mbSourceMarkers || mbDebugPlaces)
			{
				mcMarkers.AddLine(szLine, (size)(szEnd - szLine));
			}
		}
		uiTaken = (size)(szEnd + 1 - szText);
	}
	if (uiTaken > 0)
	{
		mszWritten.RemoveFromStart((int)uiTaken);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A source marker before what is written next (for a listing): kept
// apart from the code, as CIRSourceMarkers::Take would take it out, so
// the passes see the code they would without it.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::Source(uint32 uiSource)
{
	CChars	szMarker;

	if (!mbSourceMarkers || (uiSource == 0) || (uiSource == muiSource))
	{
		return;
	}
	muiSource = uiSource;
	Flush();
	szMarker.Init(CIRW65C816Backend::GetScratch());
	szMarker.Append(IR_SOURCE_MARKER);
	szMarker.Append((int64)muiSource);
	mcMarkers.AddMarker(szMarker.Text(), szMarker.Length());
	szMarker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The function's code after Layout, into mpcCode: the prologue, each
// block's instructions (those CanFuse finds read by the next as it goes)
// and jump, then the passes over it.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::GenerateBody(const char* szLabel)
{
	size			ui;
	size			uiInstruction;
	CIRBlock*		pcBlock;
	CIRBlock*		pcNext;
	int32			iLabel;
	size			uiBlockStart;
	size			uiFirst;
	int32			iWeight;
	int32			iDepth;

	maiBlockLabels.FakeSetUsedElements(0);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		iLabel = NewLabel();
		maiBlockLabels.Add(&iLabel);
	}

	FindSingleUses();
	FindZeroWords();
	FindLowByteUses();
	mpcDominators = &mcDominators;

	//Whether the function has place markers is known before anything is written: the lines between markers are
	//kept from the first (Flush, CIRSourceMarkers::AddLine).
	memset(mauiPlaceBytes, 0, sizeof(mauiPlaceBytes));
	BeginDebugPlaces(&mcDominators);
	Prologue(szLabel);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		pcNext = (ui + 1 < mpcFunction->NumBlocks()) ? mpcFunction->GetBlock(ui + 1) : NULL;
		miBlock = (int32)ui;
		Flush();
		uiBlockStart = mpcCode->NumLines();
		Label(*maiBlockLabels.Get(ui));
		mauiPendingArguments.FakeSetUsedElements(0);
		miFused = 0;
		mpsWritten = NULL;
		uiFirst = 0;
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			//Folded instructions write nothing: what is fused goes on past them.
			if (mcFolds.IsFolded(pcBlock->masInstructions.Get(uiInstruction)))
			{
				continue;
			}
			if (mcFusion.CanFuse(pcBlock, uiInstruction))
			{
				mapsFused[miFused] = pcBlock->masInstructions.Get(uiInstruction);
				miFused++;
				mpsWritten = NULL;
				continue;
			}
			DebugPlaces(pcBlock, uiFirst, uiInstruction);
			Source(pcBlock->masInstructions.Get(uiInstruction)->uiSource);
			mbUnmasked = IsLowByteOnly(pcBlock->masInstructions.Get(uiInstruction)->sResult);
			mcSelector.Instruction(pcBlock, uiInstruction);
			mbUnmasked = false;
			mpsWritten = pcBlock->masInstructions.Get(uiInstruction);
			miFused = 0;
			uiFirst = uiInstruction + 1;
		}
		DebugPlaces(pcBlock, uiFirst, pcBlock->masInstructions.NumElements());
		Source(pcBlock->msJump.uiSource);
		Jump(pcBlock, pcNext);
		miFused = 0;

		//The block's direct page accesses, weighted by its loops (AlignFrame's estimate).
		iDepth = mcLoops.Depth(pcBlock);
		if (iDepth > 0)
		{
			msFrame.bLoops = true;
		}
		iWeight = (int32)CIRLoops::Weight(iDepth, IR816_ALIGN_MAX_DEPTH);
		Flush();
		msFrame.iDirectPageWeight += iWeight * mpcCode->CountMentions(uiBlockStart, "D + ");
	}
	EndDebugPlaces();
	miBlock = -1;
	mpcDominators = NULL;
	Flush();

	//The passes over the code.
	mpcModule->Begin(CPPCP_CodePasses);
	KeepSlots(mpcCode);
	Peephole(mpcCode);
	PeepholeMasks(mpcCode);
	PeepholeStores(mpcCode);
	PeepholeTransfers(mpcCode);
	PeepholeBranches(mpcCode);
	PeepholeRegisters(mpcCode);
	RemoveUnreadStores(mpcCode);

	//The passes after Peephole leave stores and loads it would have taken out.
	RemoveDeadStores(mpcCode);

	//Then those dead on every way on from them (the lda that fed one may be loaded over unread now).
	if (RemoveFlowDeadStores(mpcCode))
	{
		RemoveDeadStores(mpcCode);
	}

	//The loads of what a register holds, known across labels.
	PeepholeFlowRegisters(mpcCode);
	mpcModule->End(CPPCP_CodePasses);
}


//////////////////////////////////////////////////////////////////////////
//
// The variables' slots' bytes (VariableSlotName: a debugger reads them,
// Debug or Release) told to the passes over the code, which then leave
// their stores.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::KeepSlots(CIRW65C816Code* pcCode)
{
	size			uiBlock;
	size			uiInstruction;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;

	for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcFunction->GetBlock(uiBlock);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (VariableSlotName(psInstruction))
			{
				pcCode->SetKept(VariableSlotPlace(psInstruction), VariableSlotBytes(psInstruction));
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The words of each temporary that are 0 whatever it is (a temporary's uiZeroWords, a
// bit a word): those a zext adds, those an and with a constant or with a
// value whose word is 0 clears, and those a shift right by a constant
// empties.  The instructions read them as the constant (Operand,
// ConstantWord), not from the frame: adc 0, not adc of a word stz put
// there, which then is often not stored at all (RemoveUnreadStores).  A
// value worked out after its use in block order (round a loop) has none.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::FindZeroWords(void)
{
	size				uiTemporary;
	size			uiBlock;
	size			ui;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	int				iWords;
	int				iNarrow;
	uint8			uiZero;
	uint8			uiAll;
	uint64			ulliConstant;

	GrowTemporaries();
	for (uiTemporary = 0; uiTemporary < masTemporaries.NumElements(); uiTemporary++)
	{
		masTemporaries.Get(uiTemporary)->uiZeroWords = 0;
	}
	for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
		{
			psInstruction = pcBlock->masInstructions.Get(ui);
			if (!psInstruction->sResult.IsTemporary() || !psInstruction->sType.IsInteger())
			{
				continue;
			}
			iWords = Words(psInstruction->sType);
			if ((iWords < 2) || (iWords > 4))
			{
				continue;
			}
			uiAll = (uint8)((1 << iWords) - 1);
			uiZero = 0;
			switch (psInstruction->uiOpcode)
			{
			case IRO_ZExt:
				if (psInstruction->asArguments[0].IsTemporary())
				{
					iNarrow = Words(mpcFunction->GetTemporaryType(psInstruction->asArguments[0]));
					uiZero = (uint8)(uiAll & ~((1 << iNarrow) - 1)) | mcSelector.ZeroWords(psInstruction->asArguments[0]);
				}
				break;
			case IRO_And:
				uiZero = mcSelector.ZeroWords(psInstruction->asArguments[0]) | mcSelector.ZeroWords(psInstruction->asArguments[1]);
				break;
			case IRO_LShr:
				if (psInstruction->asArguments[1].IsConstant())
				{
					//The top words a shift of 16 or more empties.
					ulliConstant = mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits;
					if (ulliConstant < (uint64)(16 * iWords))
					{
						uiZero = (uint8)(uiAll & ~((1 << (iWords - (int)(ulliConstant / 16))) - 1));
					}
				}
				break;
			default:
				break;
			}
			masTemporaries.Get(psInstruction->sResult.Index())->uiZeroWords = (uint8)(uiZero & uiAll);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The i8 values an instruction works out whose every use reads the low
// byte of their word alone (a temporary's bLowByteOnly): an operand of an i8 add, sub,
// and, or, xor or not (16 bit operations whose low byte depends on the
// operands' low bytes only), or the value of a byte's store (written under
// sep m).  Such a value isn't masked to its byte when it is worked out
// (Mask, mbUnmasked), so its word's high byte may be anything; the add,
// sub and not mask their own results as ever, and the logic ones do when an
// operand is one of these (Logic).  A compare, an extension, a shift, a
// multiply or divide, an argument, a phi, a branch and a return all read
// the whole word, so a value any of them uses is masked.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::FindLowByteUses(void)
{
	size				uiTemporary;
	size			uiBlock;
	size			ui;
	size			uiArgument;
	int				i;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRReference	sValue;
	bool			bLowByte;

	GrowTemporaries();
	for (uiTemporary = 0; uiTemporary < masTemporaries.NumElements(); uiTemporary++)
	{
		masTemporaries.Get(uiTemporary)->bLowByteOnly = false;
	}

	//The i8s instructions work out, until a use says otherwise.
	for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
		{
			psInstruction = pcBlock->masInstructions.Get(ui);
			if (psInstruction->sResult.IsTemporary() && psInstruction->sType.Is(IRTK_I8) && mpcFunction->GetTemporaryType(psInstruction->sResult).Is(IRTK_I8))
			{
				masTemporaries.Get(psInstruction->sResult.Index())->bLowByteOnly = 1;
			}
		}
	}
	for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
		{
			psPhi = pcBlock->masPhis.Get(ui);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				sValue = psPhi->asArguments.Get(uiArgument)->sValue;
				if (sValue.IsTemporary())
				{
					masTemporaries.Get(sValue.Index())->bLowByteOnly = 0;
				}
			}
		}
		for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
		{
			psInstruction = pcBlock->masInstructions.Get(ui);
			for (i = 0; i < IR_INSTRUCTION_ARGUMENTS; i++)
			{
				sValue = psInstruction->asArguments[i];
				if (!sValue.IsTemporary())
				{
					continue;
				}
				switch (psInstruction->uiOpcode)
				{
				case IRO_Add:
				case IRO_Sub:
				case IRO_And:
				case IRO_Or:
				case IRO_Xor:
				case IRO_Not:
					bLowByte = psInstruction->sType.Is(IRTK_I8);
					break;
				case IRO_Store:
					bLowByte = (i == 0) && psInstruction->sType.Is(IRTK_I8) && !(psInstruction->uiFlags & IRF_Volatile);
					break;
				default:
					bLowByte = false;
					break;
				}
				if (!bLowByte)
				{
					masTemporaries.Get(sValue.Index())->bLowByteOnly = 0;
				}
			}
		}
		if (pcBlock->msJump.sArgument.IsTemporary())
		{
			masTemporaries.Get(pcBlock->msJump.sArgument.Index())->bLowByteOnly = 0;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether sValue is an i8 left unmasked (FindLowByteUses).
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Backend::IsLowByteOnly(SIRReference sValue)
{
	return sValue.IsTemporary() && (sValue.Index() < masTemporaries.NumElements()) && (masTemporaries.Get(sValue.Index())->bLowByteOnly != 0);
}


//////////////////////////////////////////////////////////////////////////
//
// A bit for each word of sValue that is always 0: a constant's zero words,
// a temporary's as FindZeroWords found them.
//
//////////////////////////////////////////////////////////////////////////
uint8 CIRW65C816Selector::ZeroWords(SIRReference sValue)
{
	uint64	ulliBits;
	uint8	uiZero;
	int		i;

	if (sValue.IsConstant())
	{
		ulliBits = mpcBackend->mpcFunction->GetConstant(sValue)->ulliBits;
		uiZero = 0;
		for (i = 0; i < 4; i++)
		{
			if (((ulliBits >> (16 * i)) & 0xFFFF) == 0)
			{
				uiZero |= (uint8)(1 << i);
			}
		}
		return uiZero;
	}
	if (sValue.IsTemporary() && (sValue.Index() < mpcBackend->masTemporaries.NumElements()))
	{
		return mpcBackend->masTemporaries.Get(sValue.Index())->uiZeroWords;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether word iWord of the temporary sValue (not its first) is always 0.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Backend::IsZeroWord(SIRReference sValue, int iWord)
{
	return sValue.IsTemporary() && (iWord >= 1) && (iWord < 4) && ((mcSelector.ZeroWords(sValue) >> iWord) & 1);
}


//////////////////////////////////////////////////////////////////////////
//
// The temporaries used once, by an instruction or a jump (not a phi): the
// block of that use (a temporary's iUseBlock).  A folded load's or store's index counts
// as a use.
//
//////////////////////////////////////////////////////////////////////////
static void IR816UseIn(CArrayIRW65C816Temporaries* pasTemporaries, CArrayTemplate<int32>* paiCount, SIRReference sValue, int32 iBlock)
{
	if (!sValue.IsTemporary())
	{
		return;
	}
	(*paiCount->Get(sValue.Index()))++;
	pasTemporaries->Get(sValue.Index())->iUseBlock = iBlock;
}


//////////////////////////////////////////////////////////////////////////
//
// A record for each of the function's temporaries, and one more.  For a function
// it hasn't been asked about, every one starts as nothing is known of it; for the
// same function again, only the ones made since are new.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::GrowTemporaries(void)
{
	SIRW65C816Temporary	sNew;

	if (mpcTemporariesOf != mpcFunction)
	{
		masTemporaries.FakeSetUsedElements(0);
		mpcTemporariesOf = mpcFunction;
	}
	sNew.iOffset = -1;
	sNew.iSlot = -1;
	sNew.sFoldBase = IRNone();
	sNew.iFoldOffset = 0;
	sNew.sFoldIndex = IRNone();
	sNew.iUses = 0;
	sNew.iCountUses = 0;
	sNew.iUseBlock = -1;
	sNew.uiZeroWords = 0;
	sNew.bLowByteOnly = false;
	sNew.bInX = false;
	sNew.bDirectSlot = false;
	while (masTemporaries.NumElements() < mpcFunction->NumTemporaries() + 1)
	{
		masTemporaries.Add(&sNew);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::FindSingleUses(void)
{
	size				uiTemporary;
	CArrayTemplate<int32>	aiCount;
	CIRBlock*				pcBlock;
	SIRPhi*					psPhi;
	SIRInstruction*			psInstruction;
	size					ui;
	size					uiIndex;
	size					uiArgument;

	aiCount.Init(CIRW65C816Backend::GetScratch());
	aiCount.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0);
	GrowTemporaries();
	for (uiTemporary = 0; uiTemporary < masTemporaries.NumElements(); uiTemporary++)
	{
		masTemporaries.Get(uiTemporary)->iUseBlock = -1;
	}
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiIndex = 0; uiIndex < pcBlock->masPhis.NumElements(); uiIndex++)
		{
			psPhi = pcBlock->masPhis.Get(uiIndex);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				//Twice: a phi's use isn't in a block's straight line code.
				IR816UseIn(&masTemporaries, &aiCount, psPhi->asArguments.Get(uiArgument)->sValue, -1);
				IR816UseIn(&masTemporaries, &aiCount, psPhi->asArguments.Get(uiArgument)->sValue, -1);
			}
		}
		for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements(); uiIndex++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiIndex);
			IR816UseIn(&masTemporaries, &aiCount, psInstruction->asArguments[0], (int32)ui);
			IR816UseIn(&masTemporaries, &aiCount, psInstruction->asArguments[1], (int32)ui);

			//A folded index (FoldIndices) is read by the load or store too (ldy), not only by its zext.
			if (psInstruction->uiOpcode == IRO_Load)
			{
				IR816UseIn(&masTemporaries, &aiCount, mcFolds.FoldedIndex(psInstruction->asArguments[0]), (int32)ui);
			}
			else if (psInstruction->uiOpcode == IRO_Store)
			{
				IR816UseIn(&masTemporaries, &aiCount, mcFolds.FoldedIndex(psInstruction->asArguments[1]), (int32)ui);
			}
		}
		IR816UseIn(&masTemporaries, &aiCount, pcBlock->msJump.sArgument, (int32)ui);
	}
	for (ui = 0; ui < mpcFunction->NumTemporaries(); ui++)
	{
		if (*aiCount.Get(ui) != 1)
		{
			masTemporaries.Get(ui)->iUseBlock = -1;
		}
	}
	aiCount.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// D is pointed at the frame: the frame is D + 1 to D + frame size.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::Prologue(const char* szLabel)
{
	mcCalls.Entry(szLabel);
	if (msFrame.iRegisterWords == 0)
	{
		IR816AppendFrameEntry(mpszOut, msFrame.iFrameBytes);
	}
	else
	{
		//D saved, the register words pushed into the frame's top (A's lowest), then the rest of the frame below them.
		IR816AppendRegisterFrameEntry(mpszOut, msFrame.iRegisterWords, msFrame.iFrameBytes);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The result is in A (and X); Y keeps A while S is moved back.  The
// variables are nowhere from here on: D becomes the caller's.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::Epilogue(void)
{
	EndDebugPlaces();
	IR816AppendFrameExit(mpszOut, msFrame.iFrameBytes);
	Line("rtl");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::Return(CIRBlock* pcBlock)
{
	SIRJump*	psJump;
	int			i;
	int			iWords;

	psJump = &pcBlock->msJump;
	if (!psJump->sType.IsNone())
	{
		iWords = Words(psJump->sType);
		if (msFrame.iHiddenResult >= 0)
		{
			mcSelector.Frame("lda", msFrame.iHiddenResult);
			mcSelector.Frame("sta", IR816_SCRATCH_POINTER);
			mcSelector.Frame("lda", msFrame.iHiddenResult + 2);
			mcSelector.Frame("sta", IR816_SCRATCH_POINTER + 2);
			for (i = 0; i < iWords; i++)
			{
				Line("ldy ", 2 * i);
				mcSelector.Operand("lda", psJump->sArgument, i);
				mpszOut->Append("\t\tsta [[D + ")->Append(IR816_SCRATCH_POINTER)->Append("]:24 + Y]")->AppendNewLine();
			}
		}
		else if (mcFusion.Fused(psJump->sArgument))
		{
			//The high word first (CanFuseReturn).
			mcSelector.Operand("lda", psJump->sArgument, 1);
			Line("tax");
			mcSelector.Operand("lda", psJump->sArgument, 0);
		}
		else if (!psJump->sArgument.IsTemporary() || IR816OnDirectPage(mcPlaces.OffsetOf(psJump->sArgument) + 2 * iWords))
		{
			//Straight into A and X.
			if (iWords > 1)
			{
				mcSelector.Operand("ldx", psJump->sArgument, 1);
			}
			mcSelector.Operand("lda", psJump->sArgument, 0);
		}
		else
		{
			mcSelector.CopyWords(IR816_SCRATCH_RESULT, psJump->sArgument, iWords);
			mcSelector.Frame("lda", IR816_SCRATCH_RESULT);
			if (iWords > 1)
			{
				mcSelector.Frame("ldx", IR816_SCRATCH_RESULT + 2);
			}
		}
	}
	Epilogue();
}


//////////////////////////////////////////////////////////////////////////
//
// A jump into a block with phis puts each phi's value in its place, as one
// parallel copy (ParallelCopy: phis that take each other's values, a swap,
// are all read before any is written, a cycle broken through
// IR816_SCRATCH_RESULT).  A phi kept in X is given its value last, so the
// copies before it can still read a value in X.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::EdgeCopies(CIRBlock* pcFrom, CIRBlock* pcTo)
{
	size			ui;
	int				i;
	SIRPhi*			psPhi;
	SIRPhi*			psInX;
	SIRCopyStep*	psStep;

	masCopies.FakeSetUsedElements(0);
	psInX = NULL;
	for (ui = 0; ui < pcTo->masPhis.NumElements(); ui++)
	{
		psPhi = pcTo->masPhis.Get(ui);
		if (pcTo->FindPhiArgument(psPhi, pcFrom))
		{
			if (mcPlaces.InX(psPhi->sResult))
			{
				psInX = psPhi;
			}
			else
			{
				AddCopy(psPhi, pcTo->FindPhiArgument(psPhi, pcFrom));
			}
		}
	}
	if (psInX)
	{
		AddCopy(psInX, pcTo->FindPhiArgument(psInX, pcFrom));
	}

	ParallelCopy(&masCopies, &masCopySteps);
	for (ui = 0; ui < masCopySteps.NumElements(); ui++)
	{
		psStep = masCopySteps.Get(ui);
		if (psStep->bSave)
		{
			for (i = 0; i < Words(psStep->sType); i++)
			{
				mcSelector.Operand("lda", psStep->sSource, i);
				mcSelector.Frame("sta", IR816_SCRATCH_RESULT + 2 * i);
			}
		}
		else if (mcPlaces.InX(psStep->sDestination))
		{
			if (psStep->bSaved)
			{
				mcSelector.Frame("ldx", IR816_SCRATCH_RESULT);
			}
			else
			{
				mcSelector.Operand("ldx", psStep->sSource, 0);
			}
		}
		else if (psStep->bSaved)
		{
			for (i = 0; i < Words(psStep->sType); i++)
			{
				mcSelector.Frame("lda", IR816_SCRATCH_RESULT + 2 * i);
				mcSelector.Frame("sta", mcPlaces.OffsetOf(psStep->sDestination) + 2 * i);
			}
		}
		else
		{
			mcSelector.CopyWords(mcPlaces.OffsetOf(psStep->sDestination), psStep->sSource, Words(psStep->sType));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A phi's copy on a jump, for EdgeCopies.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::AddCopy(SIRPhi* psPhi, SIRPhiArgument* psArgument)
{
	SIRCopy*	psCopy;

	psCopy = masCopies.Add();
	psCopy->sDestination = psPhi->sResult;
	psCopy->sSource = psArgument->sValue;
	psCopy->sType = psPhi->sType;
	psCopy->bSaved = false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if writing sDestination's place changes sSource's (ParallelCopy): X
// for a value kept in X, else the frame words.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Backend::CopyOverlaps(SIRReference sSource, SIRReference sDestination)
{
	int32	iSource;
	int32	iDestination;

	if (!sSource.IsTemporary() || !mcFolds.FoldedBase(sSource).Equals(sSource))
	{
		return false;
	}
	if (mcPlaces.InX(sDestination) || mcPlaces.InX(sSource))
	{
		return mcPlaces.InX(sDestination) && mcPlaces.InX(sSource);
	}
	iSource = mcPlaces.OffsetOf(sSource);
	iDestination = mcPlaces.OffsetOf(sDestination);
	return (iSource < iDestination + 2 * Words(mpcFunction->GetTemporaryType(sDestination))) && (iDestination < iSource + 2 * Words(mpcFunction->GetTemporaryType(sSource)));
}


//////////////////////////////////////////////////////////////////////////
//
// True if the two values are at one place (ParallelCopy).
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Backend::CopySamePlace(SIRReference sValue1, SIRReference sValue2)
{
	if (!sValue1.IsTemporary() || !sValue2.IsTemporary() || !mcFolds.FoldedBase(sValue1).Equals(sValue1) || !mcFolds.FoldedBase(sValue2).Equals(sValue2))
	{
		return false;
	}
	if (mcPlaces.InX(sValue1) || mcPlaces.InX(sValue2))
	{
		return mcPlaces.InX(sValue1) && mcPlaces.InX(sValue2);
	}
	return mcPlaces.OffsetOf(sValue1) == mcPlaces.OffsetOf(sValue2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::JumpTo(CIRBlock* pcFrom, CIRBlock* pcTo, CIRBlock* pcNext)
{
	CChars	sz;

	if (pcTo == pcNext)
	{
		return;
	}
	if (mpcDominators && mpcDominators->IsBackEdge(pcFrom, pcTo))
	{
		//Going round a loop: marked for PeepholeBranches.
		sz.Init("bra L");
		sz.Append(*maiBlockLabels.Get(pcTo->muiIndex));
		sz.Append(IR816_ROUND);
		Line(sz.Text());
		sz.Kill();
		return;
	}
	Branch("bra", *maiBlockLabels.Get(pcTo->muiIndex));
}


//////////////////////////////////////////////////////////////////////////
//
// szMnemonic to label iLabel; the assembler makes it reach (.relax).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::Branch(const char* szMnemonic, int32 iLabel)
{
	CChars	sz;

	sz.Init(szMnemonic);
	sz.Append(" L");
	sz.Append(iLabel);
	Line(sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the jump from pcFrom to pcTo has phi values to copy.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Backend::HasEdgeCopies(CIRBlock* pcFrom, CIRBlock* pcTo)
{
	size	ui;

	for (ui = 0; ui < pcTo->masPhis.NumElements(); ui++)
	{
		if (pcTo->FindPhiArgument(pcTo->masPhis.Get(ui), pcFrom))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Branches to iLabel when the block's brif condition is bWhen: the fused
// comparison's flags (FusedBranch), the carry a tested call left, or the
// condition's word.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::BranchWhen(CIRBlock* pcBlock, bool bWhen, int32 iLabel)
{
	if ((pcBlock->masInstructions.NumElements() > 0) && (pcBlock->masInstructions.Tail()->uiOpcode == IRO_Thrown) && mcCalls.mbThrownInCarry &&
		pcBlock->masInstructions.Tail()->sResult.Equals(pcBlock->msJump.sArgument))
	{
		//The call left its thrown in the carry (CIRW65C816Calls::Call).
		Branch(bWhen ? "bcs" : "bcc", iLabel);
	}
	else if ((pcBlock->masInstructions.NumElements() > 0) && mcSelector.IsFusedCompare(pcBlock, pcBlock->masInstructions.NumElements() - 1))
	{
		mcSelector.FusedBranch(pcBlock->masInstructions.Get(pcBlock->masInstructions.NumElements() - 1), bWhen, iLabel, mcSelector.FlagsKnown(pcBlock), mcSelector.SubtractsSigned(pcBlock));
	}
	else
	{
		mcSelector.Operand("lda", pcBlock->msJump.sArgument, 0);
		Branch(bWhen ? "bne" : "beq", iLabel);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A brif skips a brl to its else edge when the condition isn't 0, so neither edge is limited to a short branch.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::Jump(CIRBlock* pcBlock, CIRBlock* pcNext)
{
	SIRJump*	psJump;
	int32		iThen;
	bool		bThenCopies;
	bool		bElseCopies;

	psJump = &pcBlock->msJump;
	switch (psJump->uiKind)
	{
	case IRJ_Jump:
		EdgeCopies(pcBlock, psJump->pcThen);
		JumpTo(pcBlock, psJump->pcThen, pcNext);
		break;
	case IRJ_Branch:
		//Straight to a block whose edge has no copies, falling through to the next block where it can.
		bThenCopies = HasEdgeCopies(pcBlock, psJump->pcThen);
		bElseCopies = HasEdgeCopies(pcBlock, psJump->pcElse);
		if (!bElseCopies && ((psJump->pcThen == pcNext) || bThenCopies || (psJump->pcElse != pcNext)))
		{
			BranchWhen(pcBlock, false, *maiBlockLabels.Get(psJump->pcElse->muiIndex));
			EdgeCopies(pcBlock, psJump->pcThen);
			JumpTo(pcBlock, psJump->pcThen, pcNext);
		}
		else if (!bThenCopies)
		{
			BranchWhen(pcBlock, true, *maiBlockLabels.Get(psJump->pcThen->muiIndex));
			EdgeCopies(pcBlock, psJump->pcElse);
			JumpTo(pcBlock, psJump->pcElse, pcNext);
		}
		else
		{
			iThen = NewLabel();
			BranchWhen(pcBlock, true, iThen);
			EdgeCopies(pcBlock, psJump->pcElse);
			JumpTo(pcBlock, psJump->pcElse, NULL);
			Label(iThen);
			EdgeCopies(pcBlock, psJump->pcThen);
			JumpTo(pcBlock, psJump->pcThen, pcNext);
		}
		break;
	case IRJ_Return:
		Return(pcBlock);
		break;
	case IRJ_Throw:
		//The function's exit with no value and the carry set: the caller's thrown (CIRW65C816Calls::Call).
		IR816AppendFrameExit(mpszOut, msFrame.iFrameBytes);
		Line("sec");
		Line("rtl");
		break;
	default:
		Line("stp");
		break;
	}
}

