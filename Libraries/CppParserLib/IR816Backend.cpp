/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "IR816BackendInternal.h"


//The W65C816 back end: a function's code from its IR (GenerateFunction, GenerateBody), its arguments' places
//(the register rule, FunctionArguments), its entry and exit (Prologue, RegisterStub, Forwarder, Epilogue) and its
//blocks' jumps and phis.  The rest is in IR816Places, IR816Instructions (with IR816Floats, IR816Shifts,
//IR816Multiplies, IR816Compares, IR816Fusion and IR816Calls), IR816Frames, IR816Lines, IR816Branches, IR816Peephole,
//IR816DeadStores, IR816Flow and IR816Runtime, and what they share in IR816BackendInternal.h.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Init(CIRModule* pcModule, int* piLabels)
{
	CIRBackend::Init(pcModule, piLabels, CIR816Backend::GetScratch());
	maiOffsets.Init(CIR816Backend::GetScratch());
	masCopies.Init(CIR816Backend::GetScratch());
	masCopySteps.Init(CIR816Backend::GetScratch());
	maiSlots.Init(CIR816Backend::GetScratch());
	maiParameters.Init(CIR816Backend::GetScratch());
	mauiPendingArguments.Init(CIR816Backend::GetScratch());
	masFoldBase.Init(CIR816Backend::GetScratch());
	maiFoldOffset.Init(CIR816Backend::GetScratch());
	masFoldIndex.Init(CIR816Backend::GetScratch());
	mcDominators.Init();
	mcLoops.Init();
	mcBackendPasses.Init();
	mpcDominators = NULL;
	maiUses.Init(CIR816Backend::GetScratch());
	maiCountUses.Init(CIR816Backend::GetScratch());
	maiUseBlock.Init(CIR816Backend::GetScratch());
	mabInX.Init(CIR816Backend::GetScratch());
	masArguments.Init(CIR816Backend::GetScratch());
	msFrame.Init(0, false);
	miFused = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Kill(void)
{
	masArguments.Kill();
	mauiPendingArguments.Kill();
	maiParameters.Kill();
	maiSlots.Kill();
	masCopySteps.Kill();
	masCopies.Kill();
	mabInX.Kill();
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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Line(const char* szText)
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
void CIR816Backend::Line(const char* szText, int64 lliValue)
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
int CIR816Backend::ValueBytes(SIRType sType)
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
		if (mpcModule->GetPointerSize(sType.uiSpace) != 4)
		{
			Error("Only 4 byte pointers are done yet");
		}
		return 4;
	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIR816Backend::Words(SIRType sType)
{
	return ValueBytes(sType) / 2;
}


//////////////////////////////////////////////////////////////////////////
//
// A 64 bit result is written through a hidden pointer.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::IsHiddenResult(SIRType sType)
{
	return ValueBytes(sType) == 8;
}


bool	gbIR816RegisterArguments = true;		//SetRegisterArguments.
bool	gbIR816SoftFloatInFrame = true;			//SetSoftFloatInFrame.
thread_local CMallocator*	gpcIR816Scratch = NULL;	//SetScratch: this thread's, else NULL.


//////////////////////////////////////////////////////////////////////////
//
// Whether the soft float is softfloat816.asm (true, the default), whose
// frame operand entries (FrameOperandCall) the float instructions call, or
// softfloat.cpp compiled, called as a function with its arguments pushed.
// CCPP65Generator::SetW65C816Assembly sets it.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::SetSoftFloatInFrame(bool bInFrame)
{
	gbIR816SoftFloatInFrame = bInFrame;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether register functions (CIRModule::SetRegisterCall) take their
// arguments in registers (true, the default), or every call pushes them as
// before, to compare the two.  Every unit of a program must be compiled
// the same way.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::SetRegisterArguments(bool bRegisterArguments)
{
	gbIR816RegisterArguments = bRegisterArguments;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::IsRegisterArguments(void)
{
	return gbIR816RegisterArguments;
}


//////////////////////////////////////////////////////////////////////////
//
// The back end's passes on this thread allocate their working arrays and
// strings with pcScratch (an arena, CCPP65IRFrontEnd::GenerateCode's for
// one function), or with the system allocator when it is NULL.  Nothing
// they allocate with it outlives the back end.  Returns the one it was, to
// set again after.
//
//////////////////////////////////////////////////////////////////////////
CMallocator* CIR816Backend::SetScratch(CMallocator* pcScratch)
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
CMallocator* CIR816Backend::GetScratch(void)
{
	return gpcIR816Scratch ? gpcIR816Scratch : &gcSystemAllocator;
}


//////////////////////////////////////////////////////////////////////////
//
// The register arguments' fixed rule (W65C816.txt): the arguments in order
// (the hidden result pointer first) each take the next of A, X and Y when
// all their words fit, else they are pushed; a later, smaller one may still
// fit.  Sets each one's iRegister and returns the register words used
// (always A, then X, then Y).
//
//////////////////////////////////////////////////////////////////////////
int IR816AssignRegisters(CArrayIR816Arguments* pasArguments)
{
	size			ui;
	int				iNext;
	SIR816Argument*	psArgument;

	iNext = 0;
	for (ui = 0; ui < pasArguments->NumElements(); ui++)
	{
		psArgument = pasArguments->Get(ui);
		if ((psArgument->iWords > 0) && (iNext + psArgument->iWords <= IR816_ARGUMENT_REGISTERS))
		{
			psArgument->iRegister = iNext;
			iNext += psArgument->iWords;
		}
		else
		{
			psArgument->iRegister = -1;
		}
	}
	return iNext;
}


//////////////////////////////////////////////////////////////////////////
//
// Appends a register function's label suffix: a dot, then for each argument
// word in order (the hidden result pointer's first) the register it is in
// (a, x or y) or s when it is pushed, without the pushed words at the end.
// Nothing when no word is in a register.  C++ names have no dot, so it
// can't clash: _Add__Fii.ax, _Fill__FPcic.axy.
//
//////////////////////////////////////////////////////////////////////////
void IR816AppendRegisterSuffix(CChars* psz, CArrayIR816Arguments* pasArguments)
{
	size			ui;
	size			uiStart;
	size			uiEnd;
	int				i;
	SIR816Argument*	psArgument;

	uiStart = psz->Length();
	uiEnd = uiStart;
	psz->Append('.');
	for (ui = 0; ui < pasArguments->NumElements(); ui++)
	{
		psArgument = pasArguments->Get(ui);
		for (i = 0; i < psArgument->iWords; i++)
		{
			if (psArgument->iRegister >= 0)
			{
				psz->Append(IR816_REGISTER_LETTERS[psArgument->iRegister + i]);
				uiEnd = psz->Length();
			}
			else
			{
				psz->Append('s');
			}
		}
	}
	psz->SetLength(uiEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// The function's hidden result pointer (when it returns 64 bits) and
// parameters as its callers pass them (masArguments, the frame's register
// words): in registers when its symbol is marked for them and it isn't
// variadic.  The frame starts empty.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::FunctionArguments(void)
{
	SIR816Argument*	psArgument;
	size			ui;
	int32			iRegisterWords;
	bool			bVariadic;

	masArguments.FakeSetUsedElements(0);
	if (IsHiddenResult(mpcFunction->msReturnType))
	{
		psArgument = masArguments.Add();
		psArgument->iWords = 2;
		psArgument->iRegister = -1;
	}
	for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
	{
		psArgument = masArguments.Add();
		psArgument->iWords = Words(mpcFunction->GetTemporaryType(*mpcFunction->masParameters.Get(ui)));
		psArgument->iRegister = -1;
	}
	bVariadic = (mpcFunction->muiAttributes & IRA_Variadic) != 0;
	iRegisterWords = (IsRegisterCall(IRReference(IRRK_Symbol, mpcFunction->muiSymbol)) && !bVariadic) ? IR816AssignRegisters(&masArguments) : 0;
	msFrame.Init(iRegisterWords, bVariadic);
}


//////////////////////////////////////////////////////////////////////////
//
// An empty frame: none of its parts placed.
//
//////////////////////////////////////////////////////////////////////////
void SIR816Frame::Init(int32 iRegisterWordsIn, bool bVariadicIn)
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
}


//////////////////////////////////////////////////////////////////////////
//
// A part of iBytes (in whole words) on the frame's top.  Returns its place.
//
//////////////////////////////////////////////////////////////////////////
int32 SIR816Frame::Add(int32 iBytes)
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
int32 CIR816Backend::ArgumentPlace(SIR816Argument* psArgument, int32* piPushed)
{
	int32	iPlace;

	if (psArgument->iRegister >= 0)
	{
		return msFrame.RegisterWord(psArgument->iRegister);
	}
	iPlace = *piPushed;
	*piPushed += 2 * psArgument->iWords;
	return iPlace;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::GenerateFunction(CIRFunction* pcFunction, const char* szLabel, CChars* pszDest, CCPPErrors* pcErrors)
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
	mcDominators.Compute(pcFunction);
	mcLoops.Compute(pcFunction, &mcDominators);
	FunctionArguments();

	//A function that only passes its arguments on to another (a wrapper) jumps to it.
	CChars	szJump;

	szJump.Init(CIR816Backend::GetScratch());
	if (Forwarder(&szJump))
	{
		GenerateForwarder(szLabel, &szJump);
		szJump.Kill();
		pszDest->AppendNewLine();
		return mbResult;
	}
	szJump.Kill();

	Layout();
	if (msFrame.iFrameBytes > IR816_MAX_FRAME_BYTES)
	{
		CChars	szBytes;

		szBytes.Init(CIR816Backend::GetScratch());
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
// What AlignFrame needs of the function GenerateFunction just wrote.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::GetFrame(SIR816Frame* psFrame)
{
	*psFrame = msFrame;
}


//The cycles a multiply by a constant costs as a runtime call (with its operands' stores), and roughly what a constant
//shift (by whole bytes, then bits), an add or sub and a neg cost, for 1, 2 and 4 word values (IR816MultiplyCost).
#define IR816_MULTIPLY_CALL_16	150
#define IR816_MULTIPLY_CALL_32	230
#define IR816_MULTIPLY_CALL_64	3000
#define IR816_MAX_MULTIPLY_DIGITS	8


//////////////////////////////////////////////////////////////////////////
//
// Roughly the cycles of shifts and adds in a value of iWords words: a shift
// of iShift bits, then iAdds adds (or subs).
//
//////////////////////////////////////////////////////////////////////////
static int IR816ShiftAddCost(int iWords, int iShift, int iAdds)
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
// A mul by a constant (either side) costs a runtime call: when shifts and
// adds cost less, it is written as them, x times the constant's signed
// digits (its non-adjacent form, top first) Horner's way: acc = x (or -x),
// then for each digit acc = (acc << the gap) + x (or - x), and the last
// acc << the lowest digit's place.  Multiplies by powers of 2, 0 and 1 were
// already made shifts by the optimiser (FoldConstants).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::ExpandMultiplies(void)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRReference	sValue;
	SIRReference	sAcc;
	SIRType			sType;
	uint64			ulliConstant;
	int				aiPlaces[IR816_MAX_MULTIPLY_DIGITS];
	int				aiSigns[IR816_MAX_MULTIPLY_DIGITS];
	int				iDigits;
	int				iBits;
	int				iWords;
	int				iCost;
	int				iCall;
	int				iPlace;
	int				i;
	size			ui;
	size			uiIndex;
	bool			bTooMany;
	bool			bDone;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements(); uiIndex++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiIndex);
			sType = psInstruction->sType;
			if ((psInstruction->uiOpcode != IRO_Mul) || !psInstruction->sResult.IsTemporary() || !sType.IsInteger() || sType.Is(IRTK_I1) ||
				(psInstruction->asArguments[0].IsConstant() == psInstruction->asArguments[1].IsConstant()))
			{
				continue;
			}
			i = psInstruction->asArguments[0].IsConstant() ? 0 : 1;
			ulliConstant = IRTruncate(sType, mpcFunction->GetConstant(psInstruction->asArguments[i])->ulliBits);
			sValue = psInstruction->asArguments[1 - i];
			iBits = sType.IntegerBits();
			iWords = Words(sType);

			//The non-adjacent form: digits of +1 and -1, never two next to each other.  Digits past the top are 0.
			iDigits = 0;
			bTooMany = false;
			for (iPlace = 0; (ulliConstant != 0) && (iPlace < iBits); iPlace++)
			{
				if (ulliConstant & 1)
				{
					if (iDigits == IR816_MAX_MULTIPLY_DIGITS)
					{
						bTooMany = true;
						break;
					}
					aiPlaces[iDigits] = iPlace;
					aiSigns[iDigits] = ((ulliConstant & 3) == 3) ? -1 : 1;
					ulliConstant = (aiSigns[iDigits] < 0) ? ulliConstant + 1 : ulliConstant - 1;
					iDigits++;
				}
				ulliConstant >>= 1;
			}
			if (bTooMany || (iDigits < 2))
			{
				continue;
			}

			//Horner's way: the shifts add up to the top digit's place.
			iCost = IR816ShiftAddCost(iWords, aiPlaces[iDigits - 1], iDigits - 1) + 5 * iWords * (iDigits - 1) + ((aiSigns[iDigits - 1] < 0) ? 12 * iWords : 0);
			iCall = (iWords == 1) ? IR816_MULTIPLY_CALL_16 : ((iWords == 2) ? IR816_MULTIPLY_CALL_32 : IR816_MULTIPLY_CALL_64);
			if (iCost >= iCall)
			{
				continue;
			}

			//The new instructions go before the mul, which becomes the last of them (keeping its result).
			sAcc = sValue;
			if (aiSigns[iDigits - 1] < 0)
			{
				sAcc = InsertBefore(pcBlock, &uiIndex, IRO_Neg, sType, sAcc, IRNone());
			}
			bDone = false;
			for (i = iDigits - 1; i > 0; i--)
			{
				sAcc = InsertBefore(pcBlock, &uiIndex, IRO_Shl, sType, sAcc, mpcFunction->AddInteger(sType, aiPlaces[i] - aiPlaces[i - 1]));
				if ((i == 1) && (aiPlaces[0] == 0))
				{
					//The last digit is at bit 0: its add is the mul's place.
					psInstruction = pcBlock->masInstructions.Get(uiIndex);
					psInstruction->uiOpcode = (aiSigns[0] > 0) ? IRO_Add : IRO_Sub;
					psInstruction->asArguments[0] = sAcc;
					psInstruction->asArguments[1] = sValue;
					bDone = true;
					break;
				}
				sAcc = InsertBefore(pcBlock, &uiIndex, (aiSigns[i - 1] > 0) ? IRO_Add : IRO_Sub, sType, sAcc, sValue);
			}
			if (!bDone)
			{
				//acc << the lowest digit's place is the mul's place.
				psInstruction = pcBlock->masInstructions.Get(uiIndex);
				psInstruction->uiOpcode = IRO_Shl;
				psInstruction->asArguments[0] = sAcc;
				psInstruction->asArguments[1] = mpcFunction->AddInteger(sType, aiPlaces[0]);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A new instruction before pcBlock's instruction *puiIndex, which moves up
// one (*puiIndex follows it).  Returns its result.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIR816Backend::InsertBefore(CIRBlock* pcBlock, size* puiIndex, uint16 uiOpcode, SIRType sType, SIRReference sFirst, SIRReference sSecond)
{
	SIRInstruction	sMoved;
	SIRReference	sResult;
	size			uiLast;

	sResult = mpcFunction->Add(pcBlock, uiOpcode, sType, sFirst, sSecond);
	uiLast = pcBlock->masInstructions.NumElements() - 1;
	sMoved = *pcBlock->masInstructions.Get(uiLast);
	pcBlock->masInstructions.RemoveAt(uiLast, true);
	pcBlock->masInstructions.InsertAt(&sMoved, *puiIndex);
	(*puiIndex)++;
	return sResult;
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
void CIR816Backend::GenerateBody(const char* szLabel, size uiStart)
{
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
				continue;
			}
			Source(pcBlock->masInstructions.Get(uiInstruction)->uiSource);
			Instruction(pcBlock, uiInstruction);
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
	Peephole(mpszOut, uiStart);
	PeepholeMasks(mpszOut, uiStart);
	PeepholeStores(mpszOut, uiStart);
	PeepholeTransfers(mpszOut, uiStart);
	PeepholeBranches(mpszOut, uiStart);
	PeepholeRegisters(mpszOut, uiStart);
	RemoveUnreadStores(mpszOut, uiStart);

	//The passes after Peephole leave stores and loads it would have taken out.
	RemoveDeadStores(mpszOut, uiStart);

	//Then those dead on every way on from them (the lda that fed one may be loaded over unread now).
	if (RemoveFlowDeadStores(mpszOut, uiStart))
	{
		RemoveDeadStores(mpszOut, uiStart);
	}

	//The loads of what a register holds, known across labels.
	PeepholeFlowRegisters(mpszOut, uiStart);
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
void CIR816Backend::FindSingleUses(void)
{
	CArrayTemplate<int32>	aiCount;
	CIRBlock*				pcBlock;
	SIRPhi*					psPhi;
	SIRInstruction*			psInstruction;
	size					ui;
	size					uiIndex;
	size					uiArgument;

	aiCount.Init(CIR816Backend::GetScratch());
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
// A constant the callee takes in a register is loaded into it.  Then
// pszJump gets those loads and a jml to the callee: it returns to this
// function's caller, the pushed words where its caller left them.  False,
// with nothing in pszJump, for any other function.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::Forwarder(CChars* pszJump)
{
	static const char*		aszLoads[IR816_ARGUMENT_REGISTERS] = { "lda", "ldx", "ldy" };
	CIRBlock*				pcBlock;
	SIRInstruction*			psCall;
	SIRInstruction*			psArgument;
	SIRReference			sCallee;
	CArrayIR816Arguments	asCallee;
	CArrayTemplate<int>		aiSource;		//Per callee argument: -1 the hidden result pointer, -2 a constant, else the parameter's index.
	CArrayTemplate<int>		aiPushed;		//The callee's pushed arguments' sources, in order.
	SIR816Argument*			psPlaced;
	SIR816Argument*			psOwn;
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

	//The callee's arguments as Call places them, and where each comes from.
	asCallee.Init(CIR816Backend::GetScratch());
	aiSource.Init(CIR816Backend::GetScratch());
	bResult = true;
	if (!psCall->sResult.IsNone() && IsHiddenResult(psCall->sType))
	{
		psPlaced = asCallee.Add();
		psPlaced->iWords = 2;
		psPlaced->iRegister = -1;
		iSource = -1;
		aiSource.Add(&iSource);
	}
	for (ui = 0; bResult && (ui + 1 < pcBlock->masInstructions.NumElements()); ui++)
	{
		psArgument = pcBlock->masInstructions.Get(ui);
		if (psArgument->uiOpcode != IRO_Arg)
		{
			bResult = false;
			break;
		}
		psPlaced = asCallee.Add();
		psPlaced->iWords = Words(psArgument->sType);
		psPlaced->iRegister = -1;
		iSource = -3;
		if (psArgument->asArguments[0].IsConstant())
		{
			iSource = -2;
		}
		for (uj = 0; uj < mpcFunction->masParameters.NumElements(); uj++)
		{
			if (psArgument->asArguments[0].Equals(*mpcFunction->masParameters.Get(uj)) &&
				(Words(mpcFunction->GetTemporaryType(*mpcFunction->masParameters.Get(uj))) == psPlaced->iWords))
			{
				iSource = (int)uj;
			}
		}
		bResult = iSource != -3;
		aiSource.Add(&iSource);
	}
	iRegisters = (bResult && IsRegisterCall(sCallee)) ? IR816AssignRegisters(&asCallee) : 0;

	//The pushed words line up: the callee's pushed arguments are this function's pushed ones, in order.
	uiOwnFirst = IsHiddenResult(mpcFunction->msReturnType) ? 1 : 0;
	uiFirst = asCallee.NumElements() - (pcBlock->masInstructions.NumElements() - 1);
	aiPushed.Init(CIR816Backend::GetScratch());
	for (ui = 0; bResult && (ui < asCallee.NumElements()); ui++)
	{
		if (asCallee.Get(ui)->iRegister < 0)
		{
			if (*aiSource.Get(ui) == -2)
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
	szLoads.Init(CIR816Backend::GetScratch());
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
			ulliBits = mpcFunction->GetConstant(pcBlock->masInstructions.Get(ui - uiFirst)->asArguments[0])->ulliBits;
			for (i = 0; i < psPlaced->iWords; i++)
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
		bResult = (psOwn->iRegister == psPlaced->iRegister) && (psOwn->iWords == psPlaced->iWords);
	}

	if (bResult)
	{
		pszJump->Append(szLoads);
		pszJump->Append("\t\tjml ");
		pszJump->Append(mpcModule->GetSymbolName(sCallee.Index()));
		if (iRegisters > 0)
		{
			IR816AppendRegisterSuffix(pszJump, &asCallee);
		}
		pszJump->AppendNewLine();
	}
	szLoads.Kill();
	aiPushed.Kill();
	aiSource.Kill();
	asCallee.Kill();
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
void CIR816Backend::GenerateForwarder(const char* szLabel, CChars* pszJump)
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
		IR816AppendRegisterSuffix(&szBody, &masArguments);
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
void CIR816Backend::Prologue(const char* szLabel)
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
	IR816AppendRegisterSuffix(&szBody, &masArguments);
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
void CIR816Backend::RegisterStub(CChars* pszBodyLabel)
{
	int				aiWordOf[IR816_ARGUMENT_REGISTERS];
	int				iWords;
	int				iStacked;
	int				iMoved;
	int				i;
	size			ui;
	SIR816Argument*	psArgument;
	bool			bKeepA;

	//Each register's word, the pushed words, and how many of those aren't already where the body wants them.
	iWords = 0;
	iStacked = 0;
	iMoved = 0;
	for (ui = 0; ui < masArguments.NumElements(); ui++)
	{
		psArgument = masArguments.Get(ui);
		for (i = 0; i < psArgument->iWords; i++)
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
		for (i = 0; i < psArgument->iWords; i++)
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
void CIR816Backend::RegisterStubLoad(int* aiWordOf, int iHigh, int iLow, int iPushed)
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
void CIR816Backend::RegisterStubCall(CChars* pszBodyLabel, int* aiWordOf, int iWords, int iStacked)
{
	CChars			sz;
	int				iPushed;
	int				iWord;
	int				i;
	size			ui;
	SIR816Argument*	psArgument;

	iPushed = 0;
	iWord = iWords;
	for (ui = masArguments.NumElements(); ui > 0; ui--)
	{
		psArgument = masArguments.Get(ui - 1);
		for (i = psArgument->iWords - 1; i >= 0; i--)
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
void CIR816Backend::StackLine(const char* szInstruction, int iOffset)
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
void CIR816Backend::Epilogue(void)
{
	IR816AppendFrameExit(mpszOut, msFrame.iFrameBytes);
	Line("rtl");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Return(CIRBlock* pcBlock)
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
		else if (!psJump->sArgument.IsTemporary() || (OffsetOf(psJump->sArgument) + 2 * iWords <= 255))
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
void CIR816Backend::EdgeCopies(CIRBlock* pcFrom, CIRBlock* pcTo)
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
void CIR816Backend::AddCopy(SIRPhi* psPhi, SIRPhiArgument* psArgument)
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
bool CIR816Backend::CopyOverlaps(SIRReference sSource, SIRReference sDestination)
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
bool CIR816Backend::CopySamePlace(SIRReference sValue1, SIRReference sValue2)
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
void CIR816Backend::JumpTo(CIRBlock* pcFrom, CIRBlock* pcTo, CIRBlock* pcNext)
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
void CIR816Backend::Branch(const char* szMnemonic, int32 iLabel)
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
bool CIR816Backend::HasEdgeCopies(CIRBlock* pcFrom, CIRBlock* pcTo)
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
void CIR816Backend::BranchWhen(CIRBlock* pcBlock, bool bWhen, int32 iLabel)
{
	if ((pcBlock->masInstructions.NumElements() > 0) && IsFusedCompare(pcBlock, pcBlock->masInstructions.NumElements() - 1))
	{
		FusedBranch(pcBlock->masInstructions.Get(pcBlock->masInstructions.NumElements() - 1), bWhen, iLabel, FlagsKnown(pcBlock));
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
void CIR816Backend::Jump(CIRBlock* pcBlock, CIRBlock* pcNext)
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

