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


//The W65C816 back end's instructions: each IR instruction's code (Instruction), from their operands and places:
//arithmetic and logic, extensions, loads and stores.  Shifts, soft float, multiplies and divides, compares,
//fused instructions and calls are in IR816Shifts, IR816Floats, IR816Multiplies, IR816Compares, IR816Fusion and
//IR816Calls.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::Init(CIRW65C816Backend* pcBackend)
{
	mpcBackend = pcBackend;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::Kill(void)
{
	mpcBackend = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// szMnemonic on the frame's word at iOffset: [D + offset], or through X past the direct page's 256 bytes.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::Frame(const char* szMnemonic, int32 iOffset)
{
	mpcBackend->mpszOut->Append("\t\t");
	if (IR816OnDirectPage(iOffset))
	{
		mpcBackend->mpszOut->Append(szMnemonic);
		mpcBackend->mpszOut->Append(" [D + ");
		mpcBackend->mpszOut->Append(iOffset);
		mpcBackend->mpszOut->Append("]");
	}
	else
	{
		mpcBackend->mpszOut->Append("ldx ");
		mpcBackend->mpszOut->Append(iOffset);
		mpcBackend->mpszOut->AppendNewLine();
		mpcBackend->mpszOut->Append("\t\t");
		mpcBackend->mpszOut->Append(szMnemonic);
		mpcBackend->mpszOut->Append(" [D + X]");
	}
	mpcBackend->mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// szMnemonic with word iWord of a temporary, constant or symbol.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::Operand(const char* szMnemonic, SIRReference sValue, int iWord)
{
	uint16	uiWord;
	char*	szName;

	if ((mpcBackend->miFused > 0) && (strcmp(szMnemonic, "lda") == 0) && mpcBackend->mcFusion.Fused(sValue))
	{
		WordInA(mpcBackend->mcFusion.Fused(sValue), iWord);
		return;
	}
	if (sValue.IsTemporary())
	{
		if ((iWord == 0) && mpcBackend->mcPlaces.InX(sValue))
		{
			//Kept in X (AllocateX): read through A or a push, or else the scratch word.
			if (strcmp(szMnemonic, "lda") == 0)
			{
				mpcBackend->Line("txa");
			}
			else if (strcmp(szMnemonic, "ldy") == 0)
			{
				mpcBackend->Line("txy");
			}
			else if (strcmp(szMnemonic, "pei") == 0)
			{
				mpcBackend->Line("phx");
			}
			else if (strcmp(szMnemonic, "ldx") != 0)
			{
				Frame("stx", IR816_SCRATCH_COUNT);
				Frame(szMnemonic, IR816_SCRATCH_COUNT);
			}
			return;
		}
		if (mpcBackend->IsZeroWord(sValue, iWord))
		{
			//A word that is 0 whatever the value is (FindZeroWords): the constant, not what was stored.
			if (strcmp(szMnemonic, "pei") == 0)
			{
				mpcBackend->Line("pea 0");
				return;
			}
		}
		else if (iWord < mpcBackend->Words(mpcBackend->mpcFunction->GetTemporaryType(sValue)))
		{
			Frame(szMnemonic, mpcBackend->mcPlaces.OffsetOf(sValue) + 2 * iWord);
			return;
		}
		uiWord = 0;
	}
	else if (sValue.IsConstant())
	{
		uiWord = (iWord < 4) ? (uint16)(mpcBackend->mpcFunction->GetConstant(sValue)->ulliBits >> (16 * iWord)) : 0;
	}
	else if (sValue.IsSymbol())
	{
		szName = mpcBackend->mpcModule->GetSymbolName(sValue.Index());
		mpcBackend->mpszOut->Append("\t\t");
		mpcBackend->mpszOut->Append(szMnemonic);
		if (iWord == 0)
		{
			mpcBackend->mpszOut->Append(" ");
			mpcBackend->mpszOut->Append(szName);
			mpcBackend->mpszOut->Append(" & 0xFFFF");
		}
		else if (iWord == 1)
		{
			mpcBackend->mpszOut->Append(" ");
			mpcBackend->mpszOut->Append(szName);
			mpcBackend->mpszOut->Append(" >> 16");
		}
		else
		{
			mpcBackend->mpszOut->Append(" 0");
		}
		mpcBackend->mpszOut->AppendNewLine();
		return;
	}
	else
	{
		uiWord = 0;
	}
	mpcBackend->mpszOut->Append("\t\t");
	mpcBackend->mpszOut->Append(szMnemonic);
	mpcBackend->mpszOut->Append(" ");
	mpcBackend->mpszOut->Append((int32)uiWord);
	mpcBackend->mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::Store(SIRReference sResult, int iWord)
{
	if (mpcBackend->mcPlaces.InX(sResult))
	{
		mpcBackend->Line("tax");
		return;
	}
	if ((mpcBackend->miBlock >= 0) && (mpcBackend->Words(mpcBackend->mpcFunction->GetTemporaryType(sResult)) == 1) && IR816OnDirectPage(mpcBackend->mcPlaces.OffsetOf(sResult)) &&
		(sResult.Index() < mpcBackend->masTemporaries.NumElements()) && (mpcBackend->masTemporaries.Get(sResult.Index())->iUseBlock == mpcBackend->miBlock))
	{
		//Its one use is later in the block: the store is marked for Peephole, which leaves the value in A when that
		//use is the next lda of it with A not changed before it.
		mpcBackend->mpszOut->Append("\t\tsta [D + ");
		mpcBackend->mpszOut->Append(mpcBackend->mcPlaces.OffsetOf(sResult));
		mpcBackend->mpszOut->Append("]" IR816_KEEP_IN_A);
		mpcBackend->mpszOut->AppendNewLine();
		return;
	}
	Frame("sta", mpcBackend->mcPlaces.OffsetOf(sResult) + 2 * iWord);
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps an i8 or i1 in A zero extended.  Not an i8 nothing reads but the
// low byte of (mbUnmasked, FindLowByteUses).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::Mask(SIRType sType)
{
	if (sType.Is(IRTK_I8))
	{
		if (!mpcBackend->mbUnmasked)
		{
			mpcBackend->Line("and 0xFF");
		}
	}
	else if (sType.Is(IRTK_I1))
	{
		mpcBackend->Line("and 1");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::CopyWords(int32 iDestination, SIRReference sValue, int iWords)
{
	int		i;
	uint16	uiWord;

	for (i = 0; i < iWords; i++)
	{
		if (ConstantWord(sValue, i, &uiWord) && (uiWord == 0))
		{
			Frame("stz", iDestination + 2 * i);
			continue;
		}
		Operand("lda", sValue, i);
		Frame("sta", iDestination + 2 * i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether word iWord of sValue is a constant known now (*puiWord): a
// constant's word, 0 past the end of a temporary or a constant, or a
// temporary's word that is always 0 (IsZeroWord).
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Selector::ConstantWord(SIRReference sValue, int iWord, uint16* puiWord)
{
	if (sValue.IsConstant())
	{
		*puiWord = (iWord < 4) ? (uint16)(mpcBackend->mpcFunction->GetConstant(sValue)->ulliBits >> (16 * iWord)) : 0;
		return true;
	}
	if (sValue.IsTemporary() && ((iWord >= mpcBackend->Words(mpcBackend->mpcFunction->GetTemporaryType(sValue))) || mpcBackend->IsZeroWord(sValue, iWord)))
	{
		*puiWord = 0;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::ZeroWords(int32 iDestination, int iFirst, int iWords)
{
	int		i;

	for (i = 0; i < iWords; i++)
	{
		Frame("stz", iDestination + 2 * (iFirst + i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Fills iWords words from iFirst with the sign of the word before them.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::FillSign(int32 iDestination, int iFirst, int iWords)
{
	int		i;

	if (iWords <= 0)
	{
		return;
	}
	Frame("lda", iDestination + 2 * (iFirst - 1));
	SignWordInA();
	for (i = 0; i < iWords; i++)
	{
		Frame("sta", iDestination + 2 * (iFirst + i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes a value's words, the high word first so it is little endian on the stack.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::PushValue(SIRReference sValue, SIRType sType)
{
	int		i;

	for (i = mpcBackend->Words(sType) - 1; i >= 0; i--)
	{
		PushWord(sValue, i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes word iWord of a temporary, constant or symbol.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::PushWord(SIRReference sValue, int iWord)
{
	if (sValue.IsTemporary() && (iWord < mpcBackend->Words(mpcBackend->mpcFunction->GetTemporaryType(sValue))) && (IR816OnDirectPage(mpcBackend->mcPlaces.OffsetOf(sValue) + 2 * iWord) || mpcBackend->mcPlaces.InX(sValue)))
	{
		//pei pushes a direct page word without A (Peephole makes it pha when A holds it).
		Operand("pei", sValue, iWord);
	}
	else if (sValue.IsTemporary())
	{
		Operand("lda", sValue, iWord);
		mpcBackend->Line("pha");
	}
	else
	{
		Operand("pea", sValue, iWord);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes the 24 bit address (in bank 0) of the frame's byte at iOffset.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::PushFrameAddress(int32 iOffset)
{
	mpcBackend->Line("pea 0");
	mpcBackend->Line("tdc");
	mpcBackend->Line("clc");
	mpcBackend->Line("adc ", iOffset);
	mpcBackend->Line("pha");
}


//////////////////////////////////////////////////////////////////////////
//
// Where a load or store's pointer is: a symbol (long addresses) or a place in the direct page ([d],y needs it within
// the first 256 bytes, else it is copied to the scratch pointer), and the constant a folded pointer adds to it.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::PointerPlace(SIRReference sPointer, int32* piPlace, char** pszSymbol, int32* piOffset)
{
	*piOffset = 0;
	if (!mpcBackend->mcFolds.FoldedBase(sPointer).Equals(sPointer))
	{
		*piOffset = mpcBackend->masTemporaries.Get(sPointer.Index())->iFoldOffset;
		sPointer = mpcBackend->mcFolds.FoldedBase(sPointer);
	}
	*pszSymbol = NULL;
	*piPlace = IR816_SCRATCH_POINTER;
	if (sPointer.IsSymbol())
	{
		*pszSymbol = mpcBackend->mpcModule->GetSymbolName(sPointer.Index());
		return;
	}
	if (sPointer.IsTemporary() && IR816PointerOnDirectPage(mpcBackend->mcPlaces.OffsetOf(sPointer)))
	{
		*piPlace = mpcBackend->mcPlaces.OffsetOf(sPointer);
		return;
	}
	CopyWords(IR816_SCRATCH_POINTER, sPointer, 2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Selector::Instruction(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;
	int				i;

	psInstruction = pcBlock->masInstructions.Get(uiIndex);
	switch (psInstruction->uiOpcode)
	{
	case IRO_Add:
	case IRO_PtrAdd:
		if (mpcBackend->mcFolds.FoldedBase(psInstruction->sResult).Equals(psInstruction->sResult))
		{
			Arithmetic(psInstruction);
		}
		return true;
	case IRO_Sub:
		Arithmetic(psInstruction);
		return true;
	case IRO_And:
		Logic(psInstruction);
		return true;
	case IRO_Or:
		Logic(psInstruction);
		return true;
	case IRO_Xor:
		Logic(psInstruction);
		return true;
	case IRO_Not:
		Logic(psInstruction);
		return true;
	case IRO_Neg:
		mpcBackend->Line("sec");
		for (i = 0; i < mpcBackend->Words(psInstruction->sType); i++)
		{
			mpcBackend->Line("lda 0");
			Operand("sbc", psInstruction->asArguments[0], i);
			if (mpcBackend->Words(psInstruction->sType) == 1)
			{
				Mask(psInstruction->sType);
			}
			Store(psInstruction->sResult, i);
		}
		return true;
	case IRO_Copy:
		if (mpcBackend->mcPlaces.InX(psInstruction->sResult))
		{
			Operand("ldx", psInstruction->asArguments[0], 0);
			return true;
		}
		CopyWords(mpcBackend->mcPlaces.OffsetOf(psInstruction->sResult), psInstruction->asArguments[0], mpcBackend->Words(psInstruction->sType));
		return true;
	case IRO_Shl:
	case IRO_LShr:
	case IRO_AShr:
		Shift(psInstruction);
		return true;
	case IRO_Mul:
	case IRO_UDiv:
	case IRO_SDiv:
	case IRO_URem:
	case IRO_SRem:
		return RuntimeCall(psInstruction);
	case IRO_Eq:
	case IRO_Ne:
	case IRO_SLt:
	case IRO_SLe:
	case IRO_SGt:
	case IRO_SGe:
	case IRO_ULt:
	case IRO_ULe:
	case IRO_UGt:
	case IRO_UGe:
		if (IsFusedCompare(pcBlock, uiIndex))
		{
			//Worked out by the block's branch (Jump).
			return true;
		}
		Compare(psInstruction);
		return true;
	case IRO_SExt:
	case IRO_ZExt:
	case IRO_Trunc:
	case IRO_PtrToI:
	case IRO_IToPtr:
	case IRO_SpaceCast:
		//A zext folded into its ptradds' loads and stores (FoldIndices) isn't worked out.
		if (mpcBackend->mcFolds.FoldedBase(psInstruction->sResult).Equals(psInstruction->sResult))
		{
			Extend(psInstruction);
		}
		return true;
	case IRO_Slot:
		if (!mpcBackend->mcFolds.IsDirectSlot(psInstruction->sResult))
		{
			Address(psInstruction, mpcBackend->masTemporaries.Get(psInstruction->sResult.Index())->iSlot);
		}
		return true;
	case IRO_ArgAddress:
		Address(psInstruction, *mpcBackend->maiParameters.Get((size)mpcBackend->mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits));
		return true;
	case IRO_Load:
		Load(psInstruction);
		return true;
	case IRO_Store:
		StoreMemory(psInstruction);
		return true;
	case IRO_Arg:
		mpcBackend->mauiPendingArguments.Add(&uiIndex);
		return true;
	case IRO_Call:
		mpcBackend->mcCalls.Call(pcBlock, psInstruction);
		return true;
	case IRO_Thrown:
		mpcBackend->mcCalls.Thrown(psInstruction);
		return true;
	case IRO_FAdd:
	case IRO_FSub:
	case IRO_FMul:
	case IRO_FDiv:
		FloatArithmetic(psInstruction);
		return true;
	case IRO_FNeg:
		CopyWords(mpcBackend->mcPlaces.OffsetOf(psInstruction->sResult), psInstruction->asArguments[0], mpcBackend->Words(psInstruction->sType));
		Frame("lda", mpcBackend->mcPlaces.OffsetOf(psInstruction->sResult) + 2 * (mpcBackend->Words(psInstruction->sType) - 1));
		mpcBackend->Line("eor 0x8000");
		Frame("sta", mpcBackend->mcPlaces.OffsetOf(psInstruction->sResult) + 2 * (mpcBackend->Words(psInstruction->sType) - 1));
		return true;
	case IRO_FEq:
	case IRO_FNe:
	case IRO_FLt:
	case IRO_FLe:
	case IRO_FGt:
	case IRO_FGe:
	case IRO_FOrdered:
	case IRO_FUnordered:
		FloatCompare(psInstruction);
		return true;
	case IRO_SIToF:
	case IRO_UIToF:
	case IRO_FToSI:
	case IRO_FToUI:
	case IRO_FExt:
	case IRO_FTrunc:
		FloatConversion(psInstruction);
		return true;
	default:
		return mpcBackend->Error("The W65C816 back end doesn't do this instruction yet", GetIROpcode(psInstruction->uiOpcode)->szName);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// add, sub and ptradd: a word at a time with the carry between them
// (WordInA), or an i16 kept in X stepped by 1 or 2.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::Arithmetic(SIRInstruction* psInstruction)
{
	int		i;
	int		iWords;
	uint16	uiStep;

	iWords = mpcBackend->Words(psInstruction->sType);
	if ((iWords == 1) && psInstruction->sType.Is(IRTK_I16) && mpcBackend->mcPlaces.InX(psInstruction->sResult) && mpcBackend->mcPlaces.InX(psInstruction->asArguments[0]) &&
		ConstantWord(psInstruction->asArguments[1], 0, &uiStep) && (uiStep >= 1) && (uiStep <= 2))
	{
		for (i = 0; i < uiStep; i++)
		{
			mpcBackend->Line((psInstruction->uiOpcode == IRO_Sub) ? "dex" : "inx");
		}
		return;
	}
	if (StepInPlace(psInstruction))
	{
		return;
	}
	for (i = 0; i < iWords; i++)
	{
		WordInA(psInstruction, i);
		if (iWords == 1)
		{
			Mask(psInstruction->sType);
		}
		Store(psInstruction->sResult, i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A two word value stepped by 1 or -1 into its own place (the result shares
// the operand's) with inc and dec on the direct page: inc low, and high when
// low wrapped to 0; or dec high when low is 0, then dec low.  An i16 is one
// inc or dec.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Selector::StepInPlace(SIRInstruction* psInstruction)
{
	SIRReference	sValue;
	uint32			uiStep;
	int32			iOffset;
	int32			iSkip;
	bool			bWord;

	sValue = psInstruction->asArguments[0];
	bWord = psInstruction->sType.Is(IRTK_I16);
	if (((mpcBackend->Words(psInstruction->sType) != 2) && !bWord) || !psInstruction->sResult.IsTemporary() || !sValue.IsTemporary() || mpcBackend->mcFusion.Fused(sValue) ||
		!psInstruction->asArguments[1].IsConstant() || mpcBackend->mcPlaces.InX(sValue) || mpcBackend->mcPlaces.InX(psInstruction->sResult) ||
		(mpcBackend->mcPlaces.OffsetOf(psInstruction->sResult) != mpcBackend->mcPlaces.OffsetOf(sValue)) || !IR816PointerOnDirectPage(mpcBackend->mcPlaces.OffsetOf(sValue)))
	{
		return false;
	}
	uiStep = (uint32)mpcBackend->mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits;
	if (psInstruction->uiOpcode == IRO_Sub)
	{
		uiStep = 0 - uiStep;
	}
	if (bWord)
	{
		uiStep = (uiStep & 0xFFFF) == 0xFFFF ? 0xFFFFFFFF : (uiStep & 0xFFFF);
	}
	if ((uiStep != 1) && (uiStep != 0xFFFFFFFF))
	{
		return false;
	}
	iOffset = mpcBackend->mcPlaces.OffsetOf(sValue);
	if (bWord)
	{
		Frame((uiStep == 1) ? "inc" : "dec", iOffset);
		return true;
	}
	iSkip = mpcBackend->NewLabel();
	if (uiStep == 1)
	{
		Frame("inc", iOffset);
		mpcBackend->Branch("bne", iSkip);
		Frame("inc", iOffset + 2);
		mpcBackend->Label(iSkip);
	}
	else
	{
		Frame("lda", iOffset);
		mpcBackend->Branch("bne", iSkip);
		Frame("dec", iOffset + 2);
		mpcBackend->Label(iSkip);
		Frame("dec", iOffset);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// and, or, xor and not a word at a time (WordInA); an and with a constant
// word 0 is 0 without reading the other.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::Logic(SIRInstruction* psInstruction)
{
	int		i;
	uint16	uiWord;

	for (i = 0; i < mpcBackend->Words(psInstruction->sType); i++)
	{
		if ((psInstruction->uiOpcode == IRO_And) && (ConstantWord(psInstruction->asArguments[1], i, &uiWord) || ConstantWord(psInstruction->asArguments[0], i, &uiWord)) && (uiWord == 0))
		{
			if (mpcBackend->mcPlaces.InX(psInstruction->sResult))
			{
				mpcBackend->Line("ldx 0");
			}
			else
			{
				Frame("stz", mpcBackend->mcPlaces.OffsetOf(psInstruction->sResult) + 2 * i);
			}
			continue;
		}
		WordInA(psInstruction, i);
		if (psInstruction->uiOpcode == IRO_Not)
		{
			Mask(psInstruction->sType);
		}
		else if (mpcBackend->IsLowByteOnly(psInstruction->asArguments[0]) || mpcBackend->IsLowByteOnly(psInstruction->asArguments[1]))
		{
			//An operand left unmasked (FindLowByteUses): the result's high byte is cleared here, but by an and with a
			//constant byte, which clears it itself.
			if (!((psInstruction->uiOpcode == IRO_And) && (ConstantWord(psInstruction->asArguments[1], i, &uiWord) || ConstantWord(psInstruction->asArguments[0], i, &uiWord)) && (uiWord <= 0xFF)))
			{
				Mask(psInstruction->sType);
			}
		}
		Store(psInstruction->sResult, i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An extension or trunc to one word kept in X: the value's low word in A,
// masked to a narrow result or sign extended from i8 or i1, then tax (a
// zero extension or trunc to 16 bits is ldx of the word).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::ExtendInX(SIRInstruction* psInstruction)
{
	SIRReference	sValue;
	SIRType			sSource;
	SIRType			sType;

	sValue = psInstruction->asArguments[0];
	sSource = mpcBackend->mpcFunction->TypeOf(sValue);
	sType = psInstruction->sType;
	if ((psInstruction->uiOpcode == IRO_SExt) && sSource.Is(IRTK_I1))
	{
		mpcBackend->Line("lda 0");
		mpcBackend->Line("sec");
		Operand("sbc", sValue, 0);
		Mask(sType);
	}
	else if ((psInstruction->uiOpcode == IRO_SExt) && sSource.Is(IRTK_I8))
	{
		Operand("lda", sValue, 0);
		SignExtendInA(7);
		Mask(sType);
	}
	else if (sType.Is(IRTK_I8) || sType.Is(IRTK_I1))
	{
		Operand("lda", sValue, 0);
		Mask(sType);
	}
	else
	{
		Operand("ldx", sValue, 0);
		return;
	}
	mpcBackend->Line("tax");
}


//////////////////////////////////////////////////////////////////////////
//
// zext, sext, trunc and the pointer conversions.  Narrow values are already zero extended in their word.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::Extend(SIRInstruction* psInstruction)
{
	SIRType			sSource;
	SIRType			sType;
	SIRReference	sValue;
	int32			iResult;
	int				iSourceWords;
	int				iWords;
	int				iCopied;

	sValue = psInstruction->asArguments[0];
	sSource = mpcBackend->mpcFunction->TypeOf(sValue);
	sType = psInstruction->sType;
	if (mpcBackend->mcPlaces.InX(psInstruction->sResult))
	{
		ExtendInX(psInstruction);
		return;
	}
	iResult = mpcBackend->mcPlaces.OffsetOf(psInstruction->sResult);
	iSourceWords = mpcBackend->Words(sSource);
	iWords = mpcBackend->Words(sType);
	iCopied = (iSourceWords < iWords) ? iSourceWords : iWords;

	//Only shifts' counts: they read the low word, and only its 6 bits, which extending 8 bits or more doesn't change.
	if (((psInstruction->uiOpcode == IRO_SExt) || (psInstruction->uiOpcode == IRO_ZExt)) && !sSource.Is(IRTK_I1) && OnlyCounts(psInstruction->sResult))
	{
		CopyWords(iResult, sValue, 1);
		return;
	}

	if (psInstruction->uiOpcode == IRO_SExt)
	{
		if (sSource.Is(IRTK_I1))
		{
			mpcBackend->Line("lda 0");
			mpcBackend->Line("sec");
			Operand("sbc", sValue, 0);
			Mask(sType);
			Frame("sta", iResult);
			iCopied = 1;
		}
		else if (sSource.Is(IRTK_I8))
		{
			Operand("lda", sValue, 0);
			SignExtendInA(7);
			Frame("sta", iResult);
			iCopied = 1;
		}
		else
		{
			CopyWords(iResult, sValue, iCopied);
		}
		FillSign(iResult, iCopied, iWords - iCopied);
		return;
	}

	CopyWords(iResult, sValue, iCopied);
	ZeroWords(iResult, iCopied, iWords - iCopied);
	if ((psInstruction->uiOpcode == IRO_Trunc) && (sType.Is(IRTK_I8) || sType.Is(IRTK_I1)))
	{
		Frame("lda", iResult);
		Mask(sType);
		Frame("sta", iResult);
	}
	else if (psInstruction->uiOpcode == IRO_IToPtr)
	{
		//A pointer's top byte is always 0.
		Frame("lda", iResult + 2);
		mpcBackend->Line("and 0xFF");
		Frame("sta", iResult + 2);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A slot's or argument's address: D + offset, in bank 0.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::Address(SIRInstruction* psInstruction, int32 iOffset)
{
	mpcBackend->Line("tdc");
	mpcBackend->Line("clc");
	mpcBackend->Line("adc ", iOffset);
	Store(psInstruction->sResult, 0);
	Frame("stz", mpcBackend->mcPlaces.OffsetOf(psInstruction->sResult) + 2);
}


//////////////////////////////////////////////////////////////////////////
//
// A word at a time (WordInA).  An i8 is read as a word and masked (the byte after it is only memory), or with m 8 bits
// when volatile (it may be a device's register).  A value in its pointer's place (p = p->psNext, PackTemporaries) has
// its words past the pointer read first, then its second word is kept on the stack while the first is read.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::Load(SIRInstruction* psInstruction)
{
	int		i;
	bool	bByte;

	bByte = psInstruction->sType.Is(IRTK_I8) && (psInstruction->uiFlags & IRF_Volatile);
	if (bByte)
	{
		mpcBackend->Line("sep m");
	}
	else if ((mpcBackend->Words(psInstruction->sType) >= 2) && !mpcBackend->mcPlaces.InX(psInstruction->sResult) && mpcBackend->mcFusion.FusedOverlaps(psInstruction->sResult, mpcBackend->mcFolds.FoldedBase(psInstruction->asArguments[0]), true))
	{
		for (i = mpcBackend->Words(psInstruction->sType) - 1; i >= 2; i--)
		{
			WordInA(psInstruction, i);
			Store(psInstruction->sResult, i);
		}
		WordInA(psInstruction, 1);
		mpcBackend->Line("pha");
		WordInA(psInstruction, 0);
		Store(psInstruction->sResult, 0);
		mpcBackend->Line("pla");
		Store(psInstruction->sResult, 1);
		return;
	}
	for (i = 0; i < mpcBackend->Words(psInstruction->sType); i++)
	{
		WordInA(psInstruction, i);
		if (bByte)
		{
			mpcBackend->Line("rep m");
		}
		Mask(psInstruction->sType);
		Store(psInstruction->sResult, i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A word of all ones of sType: an i8's or i1's only its bits.
//
//////////////////////////////////////////////////////////////////////////
static uint16 IR816AllOnes(SIRType sType)
{
	return sType.Is(IRTK_I8) ? 0xFF : (sType.Is(IRTK_I1) ? 1 : 0xFFFF);
}


//////////////////////////////////////////////////////////////////////////
//
// Word iWord of a load's, add's, sub's, ptradd's, and's, or's, xor's or
// not's value into A: through the pointer ([[D + p]:24 + Y]) or a symbol's
// long address, or the first operand's word and the second's (an add's or
// sub's carry first with word 0).  A commutative one reads the operand kept
// in X, or fused (CanFuse), first.  Fused operands are worked out as they
// are read.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::WordInA(SIRInstruction* psInstruction, int iWord)
{
	int32			iPlace;
	int32			iOffset;
	char*			szSymbol;
	uint16			uiWord;
	const char*		szMnemonic;
	SIRReference	sFirst;
	SIRReference	sSecond;
	bool			bAnd;
	CChars			sz;

	sFirst = psInstruction->asArguments[0];
	sSecond = psInstruction->asArguments[1];
	if ((psInstruction->uiOpcode != IRO_Sub) && (psInstruction->uiOpcode != IRO_Not) && (psInstruction->uiOpcode != IRO_Load) &&
		(mpcBackend->mcFusion.Fused(sSecond) || ((mpcBackend->Words(psInstruction->sType) == 1) && mpcBackend->mcPlaces.InX(sSecond) && !mpcBackend->mcPlaces.InX(sFirst))))
	{
		sFirst = psInstruction->asArguments[1];
		sSecond = psInstruction->asArguments[0];
	}
	switch (psInstruction->uiOpcode)
	{
	case IRO_Load:
		if (mpcBackend->mcFolds.DirectSlot(sFirst, &iOffset))
		{
			Frame("lda", iOffset + 2 * iWord);
			return;
		}
		PointerPlace(sFirst, &iPlace, &szSymbol, &iOffset);
		sz.Init(CIRW65C816Backend::GetScratch());
		if (szSymbol)
		{
			sz.Append("lda [");
			sz.Append(szSymbol);
			sz.Append(" + ");
			sz.Append(iOffset + 2 * iWord);
			sz.Append("]:24");
		}
		else
		{
			sz.Append("lda [[D + ");
			sz.Append(iPlace);
			sz.Append(mpcBackend->mcFolds.LoadY(sFirst, iOffset + 2 * iWord) ? "]:24 + Y]" : "]:24]");
		}
		mpcBackend->Line(sz.Text());
		sz.Kill();
		return;
	case IRO_Add:
	case IRO_PtrAdd:
	case IRO_Sub:
		if (iWord == 0)
		{
			mpcBackend->Line((psInstruction->uiOpcode == IRO_Sub) ? "sec" : "clc");
		}
		Operand("lda", sFirst, iWord);
		Operand((psInstruction->uiOpcode == IRO_Sub) ? "sbc" : "adc", sSecond, iWord);
		return;
	case IRO_Not:
		Operand("lda", sFirst, iWord);
		mpcBackend->Line("eor 0xFFFF");
		return;
	default:
		//A constant word that leaves the other's (and all ones, or and eor 0) is a copy; and 0 is 0.
		bAnd = psInstruction->uiOpcode == IRO_And;
		szMnemonic = bAnd ? "and" : ((psInstruction->uiOpcode == IRO_Or) ? "ora" : "eor");
		if (ConstantWord(sSecond, iWord, &uiWord))
		{
			if (bAnd && (uiWord == 0))
			{
				mpcBackend->Line("lda 0");
				return;
			}
			if ((bAnd && (uiWord == IR816AllOnes(psInstruction->sType))) || (!bAnd && (uiWord == 0)))
			{
				Operand("lda", sFirst, iWord);
				return;
			}
		}
		Operand("lda", sFirst, iWord);
		Operand(szMnemonic, sSecond, iWord);
		return;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::StoreMemory(SIRInstruction* psInstruction)
{
	int32	iPlace;
	int32	iOffset;
	char*	szSymbol;
	int		i;
	CChars	sz;

	if (mpcBackend->mcFolds.DirectSlot(psInstruction->asArguments[1], &iOffset))
	{
		DirectStore(psInstruction, iOffset);
		return;
	}
	PointerPlace(psInstruction->asArguments[1], &iPlace, &szSymbol, &iOffset);
	sz.Init(CIRW65C816Backend::GetScratch());
	if (psInstruction->sType.Is(IRTK_I8))
	{
		mpcBackend->Line("sep m");
	}
	for (i = 0; i < mpcBackend->Words(psInstruction->sType); i++)
	{
		sz.Clear();
		if (szSymbol)
		{
			sz.Append("sta [");
			sz.Append(szSymbol);
			sz.Append(" + ");
			sz.Append(iOffset + 2 * i);
			sz.Append("]:24");
		}
		else
		{
			sz.Append("sta [[D + ");
			sz.Append(iPlace);
		}

		//The value first: a fused load sets Y.
		Operand("lda", psInstruction->asArguments[0], i);
		if (!szSymbol)
		{
			sz.Append(mpcBackend->mcFolds.LoadY(psInstruction->asArguments[1], iOffset + 2 * i) ? "]:24 + Y]" : "]:24]");
		}
		mpcBackend->Line(sz.Text());
	}
	if (psInstruction->sType.Is(IRTK_I8))
	{
		mpcBackend->Line("rep m");
	}
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A store into a direct slot (CIRW65C816Folds::DirectSlot) at D +
// iOffset, a word at a time, a byte under sep m.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Selector::DirectStore(SIRInstruction* psInstruction, int32 iOffset)
{
	int		i;
	bool	bByte;

	bByte = psInstruction->sType.Is(IRTK_I8);
	for (i = 0; i < mpcBackend->Words(psInstruction->sType); i++)
	{
		Operand("lda", psInstruction->asArguments[0], i);
		if (bByte)
		{
			mpcBackend->Line("sep m");
		}
		Frame("sta", iOffset + 2 * i);
		if (bByte)
		{
			mpcBackend->Line("rep m");
		}
	}
}
