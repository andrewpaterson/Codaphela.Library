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
#include <string.h>
#include "IRW65C816BackendInternal.h"
#include "IRW65C816Code.h"


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
	maiOffsets.Init(CIRW65C816Backend::GetScratch());
	masCopies.Init(CIRW65C816Backend::GetScratch());
	masCopySteps.Init(CIRW65C816Backend::GetScratch());
	maiSlots.Init(CIRW65C816Backend::GetScratch());
	maiParameters.Init(CIRW65C816Backend::GetScratch());
	mauiPendingArguments.Init(CIRW65C816Backend::GetScratch());
	masFoldBase.Init(CIRW65C816Backend::GetScratch());
	maiFoldOffset.Init(CIRW65C816Backend::GetScratch());
	masFoldIndex.Init(CIRW65C816Backend::GetScratch());
	mcDominators.Init();
	mcLoops.Init();
	mcBackendPasses.Init();
	mpcDominators = NULL;
	maiUses.Init(CIRW65C816Backend::GetScratch());
	maiCountUses.Init(CIRW65C816Backend::GetScratch());
	maiUseBlock.Init(CIRW65C816Backend::GetScratch());
	mauiZeroWords.Init(CIRW65C816Backend::GetScratch());
	mabLowByteOnly.Init(CIRW65C816Backend::GetScratch());
	mbUnmasked = false;
	mabInX.Init(CIRW65C816Backend::GetScratch());
	masArguments.Init(CIRW65C816Backend::GetScratch());
	msFrame.Init(0, false);
	miFused = 0;
	mpsWritten = NULL;
	mpsDivided = NULL;
	mbMathTables = mpcOptions->mbMathTables;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::Kill(void)
{
	masArguments.Kill();
	mauiPendingArguments.Kill();
	maiParameters.Kill();
	maiSlots.Kill();
	masCopySteps.Kill();
	masCopies.Kill();
	mabInX.Kill();
	mabLowByteOnly.Kill();
	mauiZeroWords.Kill();
	maiUseBlock.Kill();
	maiCountUses.Kill();
	maiUses.Kill();
	masFoldIndex.Kill();
	maiFoldOffset.Kill();
	masFoldBase.Kill();
	maiOffsets.Kill();
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
	size		uiStart;

	mpcFunction = pcFunction;
	mpszOut = pszDest;
	mpcErrors = pcErrors;
	mbResult = true;
	uiStart = pszDest->Length();
	muiSource = 0;

	ExpandMultiplies();
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
	if (Forwarder(&szJump))
	{
		GenerateForwarder(szLabel, &szJump);
		szJump.Kill();
		pszDest->AppendNewLine();
		return mbResult;
	}
	szJump.Kill();

	Layout();
	msFrame.bNoResult = ReturnsNothingInA();
	if (msFrame.iFrameBytes > IR816_MAX_FRAME_BYTES)
	{
		CChars	szBytes;

		szBytes.Init(CIRW65C816Backend::GetScratch());
		szBytes.Append(msFrame.iFrameBytes);
		Error("The frame is too big for the W65C816's stack (bytes)", szBytes.Text());
		szBytes.Kill();
		return false;
	}
	GenerateBody(szLabel, uiStart);
	DropFrame(uiStart);
	if (mbSourceMarkers)
	{
		mcSourceMarkers.Put(pszDest, uiStart);
	}
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
	pcWritten->msFrame = msFrame;
	return pcWritten;
}


//////////////////////////////////////////////////////////////////////////
//
// The function's frame is put on a page if that is worth it, now that the
// unit's calls are known (AlignFrame; a listing's source markers are taken
// out while it looks at the code), and then its exits keep A in Y only when
// they must (IR816TidyExits).  A function nothing was kept for has an empty
// frame.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::FinishFunction(CChars* pszCode, const char* szLabel, CIRWrittenFunction* pcWritten, bool bRecursive, CAsm65Profile* pcProfile)
{
	CIRSourceMarkers	cMarkers;
	SIRW65C816Frame			sFrame;

	if (pcWritten)
	{
		sFrame = ((CIRW65C816Written*)pcWritten)->msFrame;
	}
	else
	{
		memset(&sFrame, 0, sizeof(sFrame));
	}
	cMarkers.Init();
	cMarkers.Take(pszCode, 0);
	AlignFrame(pszCode, szLabel, &sFrame, bRecursive, pcProfile);
	IR816TidyExits(pszCode, sFrame.bNoResult);
	cMarkers.Put(pszCode, 0);
	cMarkers.Kill();
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


//The cycles a multiply by a constant costs as a runtime call (with its operands' stores), and roughly what a constant
//shift (by whole bytes, then bits), an add or sub and a neg cost, for 1, 2 and 4 word values (ShiftAddCost).
#define IR816_MULTIPLY_CALL_16	150
#define IR816_MULTIPLY_CALL_32	230
#define IR816_MULTIPLY_CALL_64	3000


//////////////////////////////////////////////////////////////////////////
//
// Roughly the cycles of shifts and adds in a value of iWords words: a shift
// of iShift bits, then iAdds adds (or subs).
//
//////////////////////////////////////////////////////////////////////////
static int IR816ShiftCycles(int iWords, int iShift, int iAdds)
{
	int	iCost;

	iCost = 0;
	if (iShift > 0)
	{
		iCost = 5 * iWords + 12 * iWords * (iShift / 8) + 7 * iWords * (iShift % 8);
	}
	return iCost + 12 * iWords * iAdds;
}


//////////////////////////////////////////////////////////////////////////
//
// A mul by a constant is a runtime call (ExpandMultiplies).
//
//////////////////////////////////////////////////////////////////////////
int CIRW65C816Backend::MultiplyCost(SIRType sType, uint64 ulliMultiplier)
{
	int	iWords;

	iWords = Words(sType);
	return (iWords == 1) ? IR816_MULTIPLY_CALL_16 : ((iWords == 2) ? IR816_MULTIPLY_CALL_32 : IR816_MULTIPLY_CALL_64);
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
	int	iWords;

	iWords = Words(sType);
	return IR816ShiftCycles(iWords, aiPlaces[iDigits - 1], iDigits - 1) + 5 * iWords * (iDigits - 1) + ((aiSigns[iDigits - 1] < 0) ? 12 * iWords : 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The direct page operands ([D + n]) in szCode.
//
//////////////////////////////////////////////////////////////////////////
static int32 IR816CountDirectPage(const char* szCode)
{
	int32	iCount;

	iCount = 0;
	for (szCode = strstr(szCode, "D + "); szCode; szCode = strstr(szCode + 4, "D + "))
	{
		iCount++;
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
// The function's code after Layout: the prologue, each block's
// instructions (those CanFuse finds read by the next as it goes) and jump,
// then Peephole from uiStart.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::GenerateBody(const char* szLabel, size uiStart)
{
	CIRW65C816Code		cCode;
	size			ui;
	size			uiInstruction;
	CIRBlock*		pcBlock;
	CIRBlock*		pcNext;
	int32			iLabel;
	size			uiBlockStart;
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
	Prologue(szLabel);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		pcNext = (ui + 1 < mpcFunction->NumBlocks()) ? mpcFunction->GetBlock(ui + 1) : NULL;
		miBlock = (int32)ui;
		uiBlockStart = mpszOut->Length();
		Label(*maiBlockLabels.Get(ui));
		mauiPendingArguments.FakeSetUsedElements(0);
		miFused = 0;
		mpsWritten = NULL;
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			//Folded instructions write nothing: what is fused goes on past them.
			if (IsFolded(pcBlock->masInstructions.Get(uiInstruction)))
			{
				continue;
			}
			if (CanFuse(pcBlock, uiInstruction))
			{
				mapsFused[miFused] = pcBlock->masInstructions.Get(uiInstruction);
				miFused++;
				mpsWritten = NULL;
				continue;
			}
			Source(pcBlock->masInstructions.Get(uiInstruction)->uiSource);
			mbUnmasked = IsLowByteOnly(pcBlock->masInstructions.Get(uiInstruction)->sResult);
			Instruction(pcBlock, uiInstruction);
			mbUnmasked = false;
			mpsWritten = pcBlock->masInstructions.Get(uiInstruction);
			miFused = 0;
		}
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
		msFrame.iDirectPageWeight += iWeight * IR816CountDirectPage(mpszOut->Text() + uiBlockStart);
	}
	miBlock = -1;
	mpcDominators = NULL;
	if (mbSourceMarkers)
	{
		mcSourceMarkers.Take(mpszOut, uiStart);
	}

	//The passes over the code, decoded once (CIRW65C816Code).
	mpcModule->Begin(CPPCP_CodePasses);
	cCode.Init();
	cCode.Load(mpszOut, uiStart);
	Peephole(&cCode);
	PeepholeMasks(&cCode);
	PeepholeStores(&cCode);
	PeepholeTransfers(&cCode);
	PeepholeBranches(&cCode);
	PeepholeRegisters(&cCode);
	RemoveUnreadStores(&cCode);

	//The passes after Peephole leave stores and loads it would have taken out.
	RemoveDeadStores(&cCode);

	//Then those dead on every way on from them (the lda that fed one may be loaded over unread now).
	if (RemoveFlowDeadStores(&cCode))
	{
		RemoveDeadStores(&cCode);
	}

	//The loads of what a register holds, known across labels.
	PeepholeFlowRegisters(&cCode);
	cCode.Write(mpszOut, uiStart);
	cCode.Kill();
	mpcModule->End(CPPCP_CodePasses);
}


//////////////////////////////////////////////////////////////////////////
//
// The words of each temporary that are 0 whatever it is (mauiZeroWords, a
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
	size			uiBlock;
	size			ui;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	int				iWords;
	int				iNarrow;
	uint8			uiZero;
	uint8			uiAll;
	uint64			ulliConstant;

	mauiZeroWords.FakeSetUsedElements(0);
	mauiZeroWords.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0);
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
					uiZero = (uint8)(uiAll & ~((1 << iNarrow) - 1)) | ZeroWords(psInstruction->asArguments[0]);
				}
				break;
			case IRO_And:
				uiZero = ZeroWords(psInstruction->asArguments[0]) | ZeroWords(psInstruction->asArguments[1]);
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
			*mauiZeroWords.Get(psInstruction->sResult.Index()) = (uint8)(uiZero & uiAll);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The i8 values an instruction works out whose every use reads the low
// byte of their word alone (mabLowByteOnly): an operand of an i8 add, sub,
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
	size			uiBlock;
	size			ui;
	size			uiArgument;
	int				i;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRReference	sValue;
	bool			bLowByte;

	mabLowByteOnly.FakeSetUsedElements(0);
	mabLowByteOnly.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0);

	//The i8s instructions work out, until a use says otherwise.
	for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
		{
			psInstruction = pcBlock->masInstructions.Get(ui);
			if (psInstruction->sResult.IsTemporary() && psInstruction->sType.Is(IRTK_I8) && mpcFunction->GetTemporaryType(psInstruction->sResult).Is(IRTK_I8))
			{
				*mabLowByteOnly.Get(psInstruction->sResult.Index()) = 1;
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
					*mabLowByteOnly.Get(sValue.Index()) = 0;
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
					*mabLowByteOnly.Get(sValue.Index()) = 0;
				}
			}
		}
		if (pcBlock->msJump.sArgument.IsTemporary())
		{
			*mabLowByteOnly.Get(pcBlock->msJump.sArgument.Index()) = 0;
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
	return sValue.IsTemporary() && (sValue.Index() < mabLowByteOnly.NumElements()) && (*mabLowByteOnly.Get(sValue.Index()) != 0);
}


//////////////////////////////////////////////////////////////////////////
//
// A bit for each word of sValue that is always 0: a constant's zero words,
// a temporary's as FindZeroWords found them.
//
//////////////////////////////////////////////////////////////////////////
uint8 CIRW65C816Backend::ZeroWords(SIRReference sValue)
{
	uint64	ulliBits;
	uint8	uiZero;
	int		i;

	if (sValue.IsConstant())
	{
		ulliBits = mpcFunction->GetConstant(sValue)->ulliBits;
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
	if (sValue.IsTemporary() && (sValue.Index() < mauiZeroWords.NumElements()))
	{
		return *mauiZeroWords.Get(sValue.Index());
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
	return sValue.IsTemporary() && (iWord >= 1) && (iWord < 4) && ((ZeroWords(sValue) >> iWord) & 1);
}


//////////////////////////////////////////////////////////////////////////
//
// The temporaries used once, by an instruction or a jump (not a phi): the
// block of that use (maiUseBlock).  A folded load's or store's index counts
// as a use.
//
//////////////////////////////////////////////////////////////////////////
static void IR816UseIn(CArrayTemplate<int32>* paiUseBlock, CArrayTemplate<int32>* paiCount, SIRReference sValue, int32 iBlock)
{
	if (!sValue.IsTemporary())
	{
		return;
	}
	(*paiCount->Get(sValue.Index()))++;
	*paiUseBlock->Get(sValue.Index()) = iBlock;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::FindSingleUses(void)
{
	CArrayTemplate<int32>	aiCount;
	CIRBlock*				pcBlock;
	SIRPhi*					psPhi;
	SIRInstruction*			psInstruction;
	size					ui;
	size					uiIndex;
	size					uiArgument;

	aiCount.Init(CIRW65C816Backend::GetScratch());
	aiCount.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0);
	maiUseBlock.FakeSetUsedElements(0);
	maiUseBlock.GrowToAtLeastNumElements(mpcFunction->NumTemporaries() + 1, true, 0xFF);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiIndex = 0; uiIndex < pcBlock->masPhis.NumElements(); uiIndex++)
		{
			psPhi = pcBlock->masPhis.Get(uiIndex);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				//Twice: a phi's use isn't in a block's straight line code.
				IR816UseIn(&maiUseBlock, &aiCount, psPhi->asArguments.Get(uiArgument)->sValue, -1);
				IR816UseIn(&maiUseBlock, &aiCount, psPhi->asArguments.Get(uiArgument)->sValue, -1);
			}
		}
		for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements(); uiIndex++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiIndex);
			IR816UseIn(&maiUseBlock, &aiCount, psInstruction->asArguments[0], (int32)ui);
			IR816UseIn(&maiUseBlock, &aiCount, psInstruction->asArguments[1], (int32)ui);

			//A folded index (FoldIndices) is read by the load or store too (ldy), not only by its zext.
			if (psInstruction->uiOpcode == IRO_Load)
			{
				IR816UseIn(&maiUseBlock, &aiCount, FoldedIndex(psInstruction->asArguments[0]), (int32)ui);
			}
			else if (psInstruction->uiOpcode == IRO_Store)
			{
				IR816UseIn(&maiUseBlock, &aiCount, FoldedIndex(psInstruction->asArguments[1]), (int32)ui);
			}
		}
		IR816UseIn(&maiUseBlock, &aiCount, pcBlock->msJump.sArgument, (int32)ui);
	}
	for (ui = 0; ui < mpcFunction->NumTemporaries(); ui++)
	{
		if (*aiCount.Get(ui) != 1)
		{
			*maiUseBlock.Get(ui) = -1;
		}
	}
	aiCount.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the function only calls another with its own arguments and
// returns what that returns (a wrapper): one block of args, a call of a
// symbol and a return of its result (or of nothing).  Each of the callee's
// arguments must already be where the callee wants it: the hidden result
// pointer and each parameter in the same register or in the same place
// among the pushed words (the pushed words the same, in the same order).
// A constant the callee takes in a register is loaded into it.  One
// argument in A (or A and X) may be a parameter in the same registers plus
// (or minus) a constant that leaves its high word alone (a member's address: 'this'
// moved to it), added before the jump.  Then pszJump gets that add, those
// loads and a jml to the callee: it returns to this function's caller, the
// pushed words where its caller left them.  False, with nothing in pszJump,
// for any other function.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Backend::Forwarder(CChars* pszJump)
{
	static const char*		aszLoads[IR816_ARGUMENT_REGISTERS] = { "lda", "ldx", "ldy" };
	CIRBlock*				pcBlock;
	SIRInstruction*			psCall;
	SIRInstruction*			psArgument;
	SIRReference			sCallee;
	CArrayIRRegisterArguments	asCallee;
	CArrayTemplate<int>		aiSource;		//Per callee argument: -1 the hidden result pointer, -2 a constant, else the parameter's index.
	CArrayTemplate<int>		aiPushed;		//The callee's pushed arguments' sources, in order.
	CArrayTemplate<SIRInstruction*>	apsArguments;	//The block's args, in order.
	SIRInstruction*			psInstruction;
	SIRReference			sAdjusted;		//The temporary that is a parameter plus a constant, or none.
	size					uiAdjustedFrom;	//The parameter it adds to.
	uint64					ulliAdjust;
	size					uiAdjustedArgument;	//The callee's argument it is, or ARRAY_ELEMENT_NOT_FOUND.
	CChars					szAdjust;
	int32					iLabel;
	SIRRegisterArgument*			psPlaced;
	SIRRegisterArgument*			psOwn;
	size					ui;
	size					uj;
	size					uiFirst;
	size					uiOwnFirst;
	size					uiPushed;
	int						iSource;
	int						iRegisters;
	int						i;
	uint64					ulliBits;
	bool					bResult;
	CChars					szLoads;

	if ((mpcFunction->NumBlocks() != 1) || (mpcFunction->muiAttributes & IRA_Variadic))
	{
		return false;
	}
	pcBlock = mpcFunction->GetBlock(0);
	if ((pcBlock->masPhis.NumElements() != 0) || (pcBlock->masInstructions.NumElements() == 0) || (pcBlock->msJump.uiKind != IRJ_Return))
	{
		return false;
	}
	psCall = pcBlock->masInstructions.Get(pcBlock->masInstructions.NumElements() - 1);
	sCallee = psCall->asArguments[0];
	if ((psCall->uiOpcode != IRO_Call) || !sCallee.IsSymbol() || (sCallee.Index() == mpcFunction->muiSymbol) || (psCall->uiFlags & IRF_Variadic))
	{
		return false;
	}

	//It returns the call's result, of its own type, or nothing.
	if (mpcFunction->msReturnType.IsNone())
	{
		if (!pcBlock->msJump.sArgument.IsNone() || (!psCall->sResult.IsNone() && IsHiddenResult(psCall->sType)))
		{
			return false;
		}
	}
	else if (psCall->sResult.IsNone() || !pcBlock->msJump.sArgument.Equals(psCall->sResult) || !psCall->sType.Equals(mpcFunction->msReturnType))
	{
		return false;
	}

	//The args, and at most one parameter plus a constant ahead of the arg that passes it.
	apsArguments.Init(CIRW65C816Backend::GetScratch());
	sAdjusted.uiValue = 0;
	uiAdjustedFrom = 0;
	ulliAdjust = 0;
	bResult = true;
	for (ui = 0; bResult && (ui + 1 < pcBlock->masInstructions.NumElements()); ui++)
	{
		psInstruction = pcBlock->masInstructions.Get(ui);
		if (psInstruction->uiOpcode == IRO_Arg)
		{
			apsArguments.Add(&psInstruction);
			continue;
		}
		bResult = ((psInstruction->uiOpcode == IRO_PtrAdd) || (psInstruction->uiOpcode == IRO_Add) || (psInstruction->uiOpcode == IRO_Sub)) &&
				  sAdjusted.IsNone() && psInstruction->sResult.IsTemporary() && psInstruction->asArguments[1].IsConstant();
		for (uj = 0; bResult && (uj < mpcFunction->masParameters.NumElements()); uj++)
		{
			if (psInstruction->asArguments[0].Equals(*mpcFunction->masParameters.Get(uj)))
			{
				sAdjusted = psInstruction->sResult;
				uiAdjustedFrom = uj;
				ulliAdjust = mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits;
				if (psInstruction->uiOpcode == IRO_Sub)
				{
					//x - c is x + -c (at the argument's width).
					ulliAdjust = 0 - ulliAdjust;
				}
				break;
			}
		}
		bResult = bResult && !sAdjusted.IsNone();
	}
	if (!bResult)
	{
		apsArguments.Kill();
		return false;
	}

	//The callee's arguments as Call places them, and where each comes from.
	asCallee.Init(CIRW65C816Backend::GetScratch());
	aiSource.Init(CIRW65C816Backend::GetScratch());
	uiAdjustedArgument = ARRAY_ELEMENT_NOT_FOUND;
	if (!psCall->sResult.IsNone() && IsHiddenResult(psCall->sType))
	{
		psPlaced = asCallee.Add();
		psPlaced->iUnits = 2;
		psPlaced->iRegister = -1;
		iSource = -1;
		aiSource.Add(&iSource);
	}
	for (ui = 0; bResult && (ui < apsArguments.NumElements()); ui++)
	{
		psArgument = *apsArguments.Get(ui);
		psPlaced = asCallee.Add();
		psPlaced->iUnits = Words(psArgument->sType);
		psPlaced->iRegister = -1;
		iSource = -3;
		if (psArgument->asArguments[0].IsConstant())
		{
			iSource = -2;
		}
		for (uj = 0; uj < mpcFunction->masParameters.NumElements(); uj++)
		{
			if (psArgument->asArguments[0].Equals(*mpcFunction->masParameters.Get(uj)) &&
				(Words(mpcFunction->GetTemporaryType(*mpcFunction->masParameters.Get(uj))) == psPlaced->iUnits))
			{
				iSource = (int)uj;
			}
		}
		if (!sAdjusted.IsNone() && psArgument->asArguments[0].Equals(sAdjusted))
		{
			if ((uiAdjustedArgument != ARRAY_ELEMENT_NOT_FOUND) || (Words(mpcFunction->GetTemporaryType(*mpcFunction->masParameters.Get(uiAdjustedFrom))) != psPlaced->iUnits))
			{
				bResult = false;
				break;
			}
			iSource = (int)uiAdjustedFrom;
			uiAdjustedArgument = asCallee.NumElements() - 1;
		}
		bResult = iSource != -3;
		aiSource.Add(&iSource);
	}
	iRegisters = (bResult && IsRegisterCall(sCallee)) ? AssignRegisters(&asCallee, IR816_ARGUMENT_REGISTERS, IR816_ARGUMENT_REGISTERS) : 0;

	//The pushed words line up: the callee's pushed arguments are this function's pushed ones, in order.
	uiOwnFirst = IsHiddenResult(mpcFunction->msReturnType) ? 1 : 0;
	uiFirst = asCallee.NumElements() - apsArguments.NumElements();
	aiPushed.Init(CIRW65C816Backend::GetScratch());
	for (ui = 0; bResult && (ui < asCallee.NumElements()); ui++)
	{
		if (asCallee.Get(ui)->iRegister < 0)
		{
			if ((*aiSource.Get(ui) == -2) || (ui == uiAdjustedArgument))
			{
				bResult = false;
			}
			aiPushed.Add(aiSource.Get(ui));
		}
	}
	uiPushed = 0;
	for (ui = 0; bResult && (ui < masArguments.NumElements()); ui++)
	{
		psOwn = masArguments.Get(ui);
		if (psOwn->iRegister >= 0)
		{
			continue;
		}
		iSource = (ui < uiOwnFirst) ? -1 : (int)(ui - uiOwnFirst);
		bResult = (uiPushed < aiPushed.NumElements()) && (*aiPushed.Get(uiPushed) == iSource);
		uiPushed++;
	}
	bResult = bResult && (uiPushed == aiPushed.NumElements());

	//The register ones: in the same register here, or a constant loaded.
	szLoads.Init(CIRW65C816Backend::GetScratch());
	for (ui = 0; bResult && (ui < asCallee.NumElements()); ui++)
	{
		psPlaced = asCallee.Get(ui);
		if (psPlaced->iRegister < 0)
		{
			continue;
		}
		iSource = *aiSource.Get(ui);
		if (iSource == -2)
		{
			ulliBits = mpcFunction->GetConstant((*apsArguments.Get(ui - uiFirst))->asArguments[0])->ulliBits;
			for (i = 0; i < psPlaced->iUnits; i++)
			{
				szLoads.Append("\t\t");
				szLoads.Append(aszLoads[psPlaced->iRegister + i]);
				szLoads.Append(' ');
				szLoads.Append((int)((ulliBits >> (16 * i)) & 0xFFFF));
				szLoads.AppendNewLine();
			}
			continue;
		}
		if ((iSource == -1) && (uiOwnFirst == 0))
		{
			bResult = false;
			break;
		}
		psOwn = masArguments.Get((iSource == -1) ? 0 : (uiOwnFirst + (size)iSource));
		bResult = (psOwn->iRegister == psPlaced->iRegister) && (psOwn->iUnits == psPlaced->iUnits);
	}

	//The adjusted argument is in A (and X): the constant, at the argument's width, added to its low word and the
	//carry into X.  One that would change X otherwise (a pointer moved back) isn't a jump.
	szAdjust.Init(CIRW65C816Backend::GetScratch());
	if (bResult && (uiAdjustedArgument != ARRAY_ELEMENT_NOT_FOUND))
	{
		psPlaced = asCallee.Get(uiAdjustedArgument);
		ulliAdjust &= (psPlaced->iUnits == 1) ? 0xFFFFULL : 0xFFFFFFFFULL;
		bResult = (psPlaced->iRegister == 0) && (psPlaced->iUnits <= 2) && ((ulliAdjust >> 16) == 0);
		if (bResult && ((ulliAdjust & 0xFFFF) != 0))
		{
			szAdjust.Append("\t\tclc\n\t\tadc ");
			szAdjust.Append((int)(ulliAdjust & 0xFFFF));
			szAdjust.AppendNewLine();
			if (psPlaced->iUnits == 2)
			{
				iLabel = NewLabel();
				szAdjust.Append("\t\tbcc L");
				szAdjust.Append(iLabel);
				szAdjust.Append("\n\t\tinx\nL");
				szAdjust.Append(iLabel);
				szAdjust.Append(':');
				szAdjust.AppendNewLine();
			}
		}
	}

	if (bResult)
	{
		pszJump->Append(szAdjust);
		pszJump->Append(szLoads);
		pszJump->Append("\t\tjml ");
		pszJump->Append(mpcModule->GetSymbolName(sCallee.Index()));
		if (iRegisters > 0)
		{
			AppendRegisterSuffix(pszJump, &asCallee, IR816_REGISTER_LETTERS);
		}
		pszJump->AppendNewLine();
	}
	szAdjust.Kill();
	szLoads.Kill();
	aiPushed.Kill();
	aiSource.Kill();
	asCallee.Kill();
	apsArguments.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A wrapper (Forwarder): its label, a register function's stub and body
// label as Prologue writes them, then the jump (pszJump) instead of a
// frame and body.  It has no frame (FunctionArguments' empty one, which
// AlignFrame leaves).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::GenerateForwarder(const char* szLabel, CChars* pszJump)
{
	CChars	szBody;

	mpszOut->Append(szLabel);
	mpszOut->Append(':');
	mpszOut->AppendNewLine();
	Line(".width (A, X), 16");
	Line(".relax");
	if (msFrame.iRegisterWords > 0)
	{
		szBody.Init(szLabel);
		AppendRegisterSuffix(&szBody, &masArguments, IR816_REGISTER_LETTERS);
		RegisterStub(&szBody);
		if (!(mpcFunction->muiAttributes & IRA_Local) && !mpcModule->IsLocal(mpcFunction->muiSymbol))
		{
			mpszOut->Append("\t\t.global ");
			mpszOut->Append(szBody);
			mpszOut->AppendNewLine();
		}
		mpszOut->Append(szBody);
		mpszOut->Append(':');
		mpszOut->AppendNewLine();
		szBody.Kill();
	}
	mpszOut->Append(*pszJump);
}


//////////////////////////////////////////////////////////////////////////
//
// D is pointed at the frame: the frame is D + 1 to D + frame size.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::Prologue(const char* szLabel)
{
	CChars	szBody;

	mpszOut->Append(szLabel);
	mpszOut->Append(':');
	mpszOut->AppendNewLine();
	Line(".width (A, X), 16");
	Line(".relax");
	if (msFrame.iRegisterWords == 0)
	{
		IR816AppendFrameEntry(mpszOut, msFrame.iFrameBytes);
		return;
	}

	//A register function: the plain label is the stub for callers that push everything, then the body's own label
	//(visible to other objects when the plain one is).
	szBody.Init(szLabel);
	AppendRegisterSuffix(&szBody, &masArguments, IR816_REGISTER_LETTERS);
	RegisterStub(&szBody);
	if (!(mpcFunction->muiAttributes & IRA_Local) && !mpcModule->IsLocal(mpcFunction->muiSymbol))
	{
		mpszOut->Append("\t\t.global ");
		mpszOut->Append(szBody);
		mpszOut->AppendNewLine();
	}
	mpszOut->Append(szBody);
	mpszOut->Append(':');
	mpszOut->AppendNewLine();
	szBody.Kill();

	//D saved, the register words pushed into the frame's top (A's lowest), then the rest of the frame below them.
	IR816AppendRegisterFrameEntry(mpszOut, msFrame.iRegisterWords, msFrame.iFrameBytes);
}


//////////////////////////////////////////////////////////////////////////
//
// A register function's plain label, for callers that push every argument
// (function pointers, vtables, thunks, assembly): on entry argument word k
// is at S + 4 + 2k (after the jsl's return address).  The body wants its
// pushed words just above its return address (S + 4 on), in order, so they
// are moved down there (over the register words, read first) and it goes
// straight on to the body; the words left above them are the caller's to
// remove, as they always are.  Y's and X's words are loaded through A, then
// A's (kept in Y while the words move when its place is moved over).  When
// all three registers hold arguments and A's place is moved over there is
// nowhere to keep it: the pushed words are pushed again instead, the body
// is called, they are removed (A kept in Y, X kept) and it returns.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::RegisterStub(CChars* pszBodyLabel)
{
	int				aiWordOf[IR816_ARGUMENT_REGISTERS];
	int				iWords;
	int				iStacked;
	int				iMoved;
	int				i;
	size			ui;
	SIRRegisterArgument*	psArgument;
	bool			bKeepA;

	//Each register's word, the pushed words, and how many of those aren't already where the body wants them.
	iWords = 0;
	iStacked = 0;
	iMoved = 0;
	for (ui = 0; ui < masArguments.NumElements(); ui++)
	{
		psArgument = masArguments.Get(ui);
		for (i = 0; i < psArgument->iUnits; i++)
		{
			if (psArgument->iRegister >= 0)
			{
				aiWordOf[psArgument->iRegister + i] = iWords;
			}
			else
			{
				if (iWords != iStacked)
				{
					iMoved++;
				}
				iStacked++;
			}
			iWords++;
		}
	}

	//A's word is moved over when its place is one the pushed words go to.
	bKeepA = (iMoved > 0) && (aiWordOf[0] < iStacked);
	if (bKeepA && (msFrame.iRegisterWords == IR816_ARGUMENT_REGISTERS))
	{
		RegisterStubCall(pszBodyLabel, aiWordOf, iWords, iStacked);
		return;
	}

	//Y's and X's words, and A's into Y when the moves go over it.
	RegisterStubLoad(aiWordOf, msFrame.iRegisterWords - 1, 1, 0);
	if (bKeepA)
	{
		RegisterStubLoad(aiWordOf, 0, 0, 0);
		Line("tay");
	}

	//The pushed words down to S + 4 on, first first: each goes to a lower place than any still to be read.
	iStacked = 0;
	iWords = 0;
	for (ui = 0; ui < masArguments.NumElements(); ui++)
	{
		psArgument = masArguments.Get(ui);
		for (i = 0; i < psArgument->iUnits; i++)
		{
			if (psArgument->iRegister < 0)
			{
				if (iWords != iStacked)
				{
					StackLine("lda", 4 + 2 * iWords);
					StackLine("sta", 4 + 2 * iStacked);
				}
				iStacked++;
			}
			iWords++;
		}
	}

	//A's word, then on to the body (its label is next).
	if (bKeepA)
	{
		Line("tya");
	}
	else
	{
		RegisterStubLoad(aiWordOf, 0, 0, 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Loads the register words iHigh down to iLow (2 Y, 1 X, 0 A) from the
// plain label's arguments, iPushed words further from S than on entry: Y's
// and X's through A.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::RegisterStubLoad(int* aiWordOf, int iHigh, int iLow, int iPushed)
{
	int		k;

	for (k = iHigh; k >= iLow; k--)
	{
		StackLine("lda", 4 + 2 * aiWordOf[k] + 2 * iPushed);
		if (k == 2)
		{
			Line("tay");
		}
		else if (k == 1)
		{
			Line("tax");
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// RegisterStub when the pushed words can't be moved down (all three
// registers hold arguments and A's place is moved over): they are pushed
// again, last first (each push moves the stack's words 2 further from S),
// the registers are loaded, the body is called, they are removed (A kept
// in Y, X kept) and it returns.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::RegisterStubCall(CChars* pszBodyLabel, int* aiWordOf, int iWords, int iStacked)
{
	CChars			sz;
	int				iPushed;
	int				iWord;
	int				i;
	size			ui;
	SIRRegisterArgument*	psArgument;

	iPushed = 0;
	iWord = iWords;
	for (ui = masArguments.NumElements(); ui > 0; ui--)
	{
		psArgument = masArguments.Get(ui - 1);
		for (i = psArgument->iUnits - 1; i >= 0; i--)
		{
			iWord--;
			if (psArgument->iRegister < 0)
			{
				StackLine("lda", 4 + 2 * iWord + 2 * iPushed);
				Line("pha");
				iPushed++;
			}
		}
	}

	RegisterStubLoad(aiWordOf, msFrame.iRegisterWords - 1, 0, iStacked);

	sz.Init("jsl ");
	sz.Append(pszBodyLabel);
	Line(sz.Text());
	Line("tay");
	Line("tsc");
	Line("clc");
	Line("adc ", 2 * iStacked);
	Line("tcs");
	Line("tya");
	Line("rtl");
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// szInstruction [S + iOffset]: a stack relative load or store.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::StackLine(const char* szInstruction, int iOffset)
{
	CChars	sz;

	sz.Init(szInstruction);
	sz.Append(" [S + ");
	sz.Append(iOffset);
	sz.Append("]");
	Line(sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The result is in A (and X); Y keeps A while S is moved back.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::Epilogue(void)
{
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
			Frame("lda", msFrame.iHiddenResult);
			Frame("sta", IR816_SCRATCH_POINTER);
			Frame("lda", msFrame.iHiddenResult + 2);
			Frame("sta", IR816_SCRATCH_POINTER + 2);
			for (i = 0; i < iWords; i++)
			{
				Line("ldy ", 2 * i);
				Operand("lda", psJump->sArgument, i);
				mpszOut->Append("\t\tsta [[D + ")->Append(IR816_SCRATCH_POINTER)->Append("]:24 + Y]")->AppendNewLine();
			}
		}
		else if (Fused(psJump->sArgument))
		{
			//The high word first (CanFuseReturn).
			Operand("lda", psJump->sArgument, 1);
			Line("tax");
			Operand("lda", psJump->sArgument, 0);
		}
		else if (!psJump->sArgument.IsTemporary() || IR816OnDirectPage(OffsetOf(psJump->sArgument) + 2 * iWords))
		{
			//Straight into A and X.
			if (iWords > 1)
			{
				Operand("ldx", psJump->sArgument, 1);
			}
			Operand("lda", psJump->sArgument, 0);
		}
		else
		{
			CopyWords(IR816_SCRATCH_RESULT, psJump->sArgument, iWords);
			Frame("lda", IR816_SCRATCH_RESULT);
			if (iWords > 1)
			{
				Frame("ldx", IR816_SCRATCH_RESULT + 2);
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
			if (InX(psPhi->sResult))
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
				Operand("lda", psStep->sSource, i);
				Frame("sta", IR816_SCRATCH_RESULT + 2 * i);
			}
		}
		else if (InX(psStep->sDestination))
		{
			if (psStep->bSaved)
			{
				Frame("ldx", IR816_SCRATCH_RESULT);
			}
			else
			{
				Operand("ldx", psStep->sSource, 0);
			}
		}
		else if (psStep->bSaved)
		{
			for (i = 0; i < Words(psStep->sType); i++)
			{
				Frame("lda", IR816_SCRATCH_RESULT + 2 * i);
				Frame("sta", OffsetOf(psStep->sDestination) + 2 * i);
			}
		}
		else
		{
			CopyWords(OffsetOf(psStep->sDestination), psStep->sSource, Words(psStep->sType));
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

	if (!sSource.IsTemporary() || !FoldedBase(sSource).Equals(sSource))
	{
		return false;
	}
	if (InX(sDestination) || InX(sSource))
	{
		return InX(sDestination) && InX(sSource);
	}
	iSource = OffsetOf(sSource);
	iDestination = OffsetOf(sDestination);
	return (iSource < iDestination + 2 * Words(mpcFunction->GetTemporaryType(sDestination))) && (iDestination < iSource + 2 * Words(mpcFunction->GetTemporaryType(sSource)));
}


//////////////////////////////////////////////////////////////////////////
//
// True if the two values are at one place (ParallelCopy).
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Backend::CopySamePlace(SIRReference sValue1, SIRReference sValue2)
{
	if (!sValue1.IsTemporary() || !sValue2.IsTemporary() || !FoldedBase(sValue1).Equals(sValue1) || !FoldedBase(sValue2).Equals(sValue2))
	{
		return false;
	}
	if (InX(sValue1) || InX(sValue2))
	{
		return InX(sValue1) && InX(sValue2);
	}
	return OffsetOf(sValue1) == OffsetOf(sValue2);
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
// comparison's flags (FusedBranch) or the condition's word.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::BranchWhen(CIRBlock* pcBlock, bool bWhen, int32 iLabel)
{
	if ((pcBlock->masInstructions.NumElements() > 0) && IsFusedCompare(pcBlock, pcBlock->masInstructions.NumElements() - 1))
	{
		FusedBranch(pcBlock->masInstructions.Get(pcBlock->masInstructions.NumElements() - 1), bWhen, iLabel, FlagsKnown(pcBlock), SubtractsSigned(pcBlock));
	}
	else
	{
		Operand("lda", pcBlock->msJump.sArgument, 0);
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
	default:
		Line("stp");
		break;
	}
}

