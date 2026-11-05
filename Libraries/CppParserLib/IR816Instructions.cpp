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
#include <stdio.h>
#include <string.h>
#include "IR816BackendInternal.h"


//The W65C816 back end's instructions: each IR instruction's code (Instruction), from their operands and places:
//arithmetic and logic, shifts, soft float, compares and branches on them, extensions, loads, stores and calls.


#define IR816_SHIFT_LOOP_BELOW	5		//Variable shifts by fewer bits loop a bit at a time instead.


//////////////////////////////////////////////////////////////////////////
//
// szMnemonic on the frame's word at iOffset: [D + offset], or through X past the direct page's 256 bytes.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Frame(const char* szMnemonic, int32 iOffset)
{
	mpszOut->Append("\t\t");
	if (iOffset <= 255)
	{
		mpszOut->Append(szMnemonic);
		mpszOut->Append(" [D + ");
		mpszOut->Append(iOffset);
		mpszOut->Append("]");
	}
	else
	{
		mpszOut->Append("ldx ");
		mpszOut->Append(iOffset);
		mpszOut->AppendNewLine();
		mpszOut->Append("\t\t");
		mpszOut->Append(szMnemonic);
		mpszOut->Append(" [D + X]");
	}
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// szMnemonic with word iWord of a temporary, constant or symbol.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Operand(const char* szMnemonic, SIRReference sValue, int iWord)
{
	uint16	uiWord;
	char*	szName;

	if ((miFused > 0) && (strcmp(szMnemonic, "lda") == 0) && Fused(sValue))
	{
		WordInA(Fused(sValue), iWord);
		return;
	}
	if (sValue.IsTemporary())
	{
		if ((iWord == 0) && InX(sValue))
		{
			//Kept in X (AllocateX): read through A or a push, or else the scratch word.
			if (strcmp(szMnemonic, "lda") == 0)
			{
				Line("txa");
			}
			else if (strcmp(szMnemonic, "ldy") == 0)
			{
				Line("txy");
			}
			else if (strcmp(szMnemonic, "pei") == 0)
			{
				Line("phx");
			}
			else if (strcmp(szMnemonic, "ldx") != 0)
			{
				Frame("stx", IR816_SCRATCH_COUNT);
				Frame(szMnemonic, IR816_SCRATCH_COUNT);
			}
			return;
		}
		if (iWord < Words(mpcFunction->GetTemporaryType(sValue)))
		{
			Frame(szMnemonic, OffsetOf(sValue) + 2 * iWord);
			return;
		}
		uiWord = 0;
	}
	else if (sValue.IsConstant())
	{
		uiWord = (iWord < 4) ? (uint16)(mpcFunction->GetConstant(sValue)->ulliBits >> (16 * iWord)) : 0;
	}
	else if (sValue.IsSymbol())
	{
		szName = mpcModule->GetSymbolName(sValue.Index());
		mpszOut->Append("\t\t");
		mpszOut->Append(szMnemonic);
		if (iWord == 0)
		{
			mpszOut->Append(" ");
			mpszOut->Append(szName);
			mpszOut->Append(" & 0xFFFF");
		}
		else if (iWord == 1)
		{
			mpszOut->Append(" ");
			mpszOut->Append(szName);
			mpszOut->Append(" >> 16");
		}
		else
		{
			mpszOut->Append(" 0");
		}
		mpszOut->AppendNewLine();
		return;
	}
	else
	{
		uiWord = 0;
	}
	mpszOut->Append("\t\t");
	mpszOut->Append(szMnemonic);
	mpszOut->Append(" ");
	mpszOut->Append((int32)uiWord);
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Store(SIRReference sResult, int iWord)
{
	if (InX(sResult))
	{
		Line("tax");
		return;
	}
	if ((miBlock >= 0) && (Words(mpcFunction->GetTemporaryType(sResult)) == 1) && (OffsetOf(sResult) <= 255) &&
		(sResult.Index() < maiUseBlock.NumElements()) && (*maiUseBlock.Get(sResult.Index()) == miBlock))
	{
		//Its one use is later in the block: the store is marked for Peephole, which leaves the value in A when that
		//use is the next lda of it with A not changed before it.
		mpszOut->Append("\t\tsta [D + ");
		mpszOut->Append(OffsetOf(sResult));
		mpszOut->Append("]" IR816_KEEP_IN_A);
		mpszOut->AppendNewLine();
		return;
	}
	Frame("sta", OffsetOf(sResult) + 2 * iWord);
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps an i8 or i1 in A zero extended.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Mask(SIRType sType)
{
	if (sType.Is(IRTK_I8))
	{
		Line("and 0xFF");
	}
	else if (sType.Is(IRTK_I1))
	{
		Line("and 1");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::CopyWords(int32 iDestination, SIRReference sValue, int iWords)
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
// constant's word, or 0 past the end of a temporary or a constant.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::ConstantWord(SIRReference sValue, int iWord, uint16* puiWord)
{
	if (sValue.IsConstant())
	{
		*puiWord = (iWord < 4) ? (uint16)(mpcFunction->GetConstant(sValue)->ulliBits >> (16 * iWord)) : 0;
		return true;
	}
	if (sValue.IsTemporary() && (iWord >= Words(mpcFunction->GetTemporaryType(sValue))))
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
void CIR816Backend::ZeroWords(int32 iDestination, int iFirst, int iWords)
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
void CIR816Backend::FillSign(int32 iDestination, int iFirst, int iWords)
{
	int		i;

	if (iWords <= 0)
	{
		return;
	}
	Frame("lda", iDestination + 2 * (iFirst - 1));
	Line("asl A");
	Line("lda 0");
	Line("adc 0xFFFF");
	Line("eor 0xFFFF");
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
void CIR816Backend::PushValue(SIRReference sValue, SIRType sType)
{
	int		i;

	for (i = Words(sType) - 1; i >= 0; i--)
	{
		PushWord(sValue, i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes word iWord of a temporary, constant or symbol.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PushWord(SIRReference sValue, int iWord)
{
	if (sValue.IsTemporary() && (iWord < Words(mpcFunction->GetTemporaryType(sValue))) && ((OffsetOf(sValue) + 2 * iWord <= 255) || InX(sValue)))
	{
		//pei pushes a direct page word without A (Peephole makes it pha when A holds it).
		Operand("pei", sValue, iWord);
	}
	else if (sValue.IsTemporary())
	{
		Operand("lda", sValue, iWord);
		Line("pha");
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
void CIR816Backend::PushFrameAddress(int32 iOffset)
{
	Line("pea 0");
	Line("tdc");
	Line("clc");
	Line("adc ", iOffset);
	Line("pha");
}


//////////////////////////////////////////////////////////////////////////
//
// Where a load or store's pointer is: a symbol (long addresses) or a place in the direct page ([d],y needs it within
// the first 256 bytes, else it is copied to the scratch pointer), and the constant a folded pointer adds to it.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PointerPlace(SIRReference sPointer, int32* piPlace, char** pszSymbol, int32* piOffset)
{
	*piOffset = 0;
	if (!FoldedBase(sPointer).Equals(sPointer))
	{
		*piOffset = *maiFoldOffset.Get(sPointer.Index());
		sPointer = FoldedBase(sPointer);
	}
	*pszSymbol = NULL;
	*piPlace = IR816_SCRATCH_POINTER;
	if (sPointer.IsSymbol())
	{
		*pszSymbol = mpcModule->GetSymbolName(sPointer.Index());
		return;
	}
	if (sPointer.IsTemporary() && (OffsetOf(sPointer) <= 253))
	{
		*piPlace = OffsetOf(sPointer);
		return;
	}
	CopyWords(IR816_SCRATCH_POINTER, sPointer, 2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::Instruction(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;
	int				i;

	psInstruction = pcBlock->masInstructions.Get(uiIndex);
	switch (psInstruction->uiOpcode)
	{
	case IRO_Add:
	case IRO_PtrAdd:
		if (FoldedBase(psInstruction->sResult).Equals(psInstruction->sResult))
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
		Line("sec");
		for (i = 0; i < Words(psInstruction->sType); i++)
		{
			Line("lda 0");
			Operand("sbc", psInstruction->asArguments[0], i);
			if (Words(psInstruction->sType) == 1)
			{
				Mask(psInstruction->sType);
			}
			Store(psInstruction->sResult, i);
		}
		return true;
	case IRO_Copy:
		if (InX(psInstruction->sResult))
		{
			Operand("ldx", psInstruction->asArguments[0], 0);
			return true;
		}
		CopyWords(OffsetOf(psInstruction->sResult), psInstruction->asArguments[0], Words(psInstruction->sType));
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
		if (FoldedBase(psInstruction->sResult).Equals(psInstruction->sResult))
		{
			Extend(psInstruction);
		}
		return true;
	case IRO_Slot:
		Address(psInstruction, *maiSlots.Get(psInstruction->sResult.Index()));
		return true;
	case IRO_ArgAddress:
		Address(psInstruction, *maiParameters.Get((size)mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits));
		return true;
	case IRO_Load:
		Load(psInstruction);
		return true;
	case IRO_Store:
		StoreMemory(psInstruction);
		return true;
	case IRO_Arg:
		mauiPendingArguments.Add(&uiIndex);
		return true;
	case IRO_Call:
		Call(pcBlock, psInstruction);
		return true;
	case IRO_FAdd:
	case IRO_FSub:
	case IRO_FMul:
	case IRO_FDiv:
		FloatArithmetic(psInstruction);
		return true;
	case IRO_FNeg:
		CopyWords(OffsetOf(psInstruction->sResult), psInstruction->asArguments[0], Words(psInstruction->sType));
		Frame("lda", OffsetOf(psInstruction->sResult) + 2 * (Words(psInstruction->sType) - 1));
		Line("eor 0x8000");
		Frame("sta", OffsetOf(psInstruction->sResult) + 2 * (Words(psInstruction->sType) - 1));
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
		return Error("The W65C816 back end doesn't do this instruction yet", GetIROpcode(psInstruction->uiOpcode)->szName);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The soft float runtime (Libc65's softfloat.cpp, compiled for the W65C816): _SoftFloat<operation> for f32 and
// _SoftDouble<operation> for f64.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::FloatArithmetic(SIRInstruction* psInstruction)
{
	CChars			szName;
	int32			iFirst;
	int32			iSecond;
	int32			iBytes;
	SIRReference	asValues[2];
	SIRType			asTypes[2];
	bool			bHidden;

	szName.Init(psInstruction->sType.Is(IRTK_F64) ? "_SoftDouble" : "_SoftFloat");
	switch (psInstruction->uiOpcode)
	{
	case IRO_FAdd:	szName.Append("Add");		break;
	case IRO_FSub:	szName.Append("Subtract");	break;
	case IRO_FMul:	szName.Append("Multiply");	break;
	default:		szName.Append("Divide");	break;
	}
	if (gbIR816SoftFloatInFrame)
	{
		iFirst = FrameOperand(psInstruction->asArguments[0], psInstruction->sType, IR816_SCRATCH_RESULT);
		iSecond = FrameOperand(psInstruction->asArguments[1], psInstruction->sType, IR816_SCRATCH_ARGUMENT_A);
		FrameOperandCall(szName.Text(), psInstruction, iFirst, iSecond);
	}
	else
	{
		asValues[0] = psInstruction->asArguments[0];
		asValues[1] = psInstruction->asArguments[1];
		asTypes[0] = psInstruction->sType;
		asTypes[1] = psInstruction->sType;
		bHidden = IsHiddenResult(psInstruction->sType);
		iBytes = SoftFloatCall(szName.Text(), asValues, asTypes, 2, -1, false, bHidden, psInstruction->sResult.IsNone() ? IR816_SCRATCH_RESULT : OffsetOf(psInstruction->sResult));
		CallerCleanUp(iBytes, psInstruction);
	}
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The place in the frame of an operand of a frame operand call: its own
// when it is a temporary with one, else it is copied to iScratch.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR816Backend::FrameOperand(SIRReference sValue, SIRType sType, int32 iScratch)
{
	if (sValue.IsTemporary() && !InX(sValue) && !Fused(sValue) && (OffsetOf(sValue) >= IR816_FIRST_TEMPORARY))
	{
		return OffsetOf(sValue);
	}
	CopyWords(iScratch, sValue, Words(sType));
	return iScratch;
}


//////////////////////////////////////////////////////////////////////////
//
// A call of the soft float assembly's frame operand entry (szName.dp,
// Libc65's softfloat816.asm): the operands stay in the frame (FrameOperand)
// and their places (from D) are in X and Y (iSecond is -1 for none); a 64
// bit result's place is in A and the routine writes it there, any other
// result comes back in A (and X).  Nothing is pushed or removed.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::FrameOperandCall(const char* szName, SIRInstruction* psInstruction, int32 iFirst, int32 iSecond)
{
	CChars	sz;
	bool	bHidden;

	bHidden = IsHiddenResult(psInstruction->sType);
	Line("ldx ", iFirst);
	if (iSecond >= 0)
	{
		Line("ldy ", iSecond);
	}
	if (bHidden)
	{
		Line("lda ", psInstruction->sResult.IsNone() ? IR816_SCRATCH_RESULT : OffsetOf(psInstruction->sResult));
	}
	sz.Init(szName);
	sz.Append(IR816_FRAME_OPERANDS);
	UseRuntime(sz.Text());
	sz.Insert(0, "jsl ");
	Line(sz.Text());
	sz.Kill();
	if (!bHidden)
	{
		CallerCleanUp(0, psInstruction);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes the hidden result pointer when the result is 64 bits, calls szName and removes the iBytes of arguments
// already pushed (and the pointer), leaving the result in its place.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::CallFunction(const char* szName, int32 iBytes, SIRInstruction* psInstruction)
{
	CChars	sz;

	UseRuntime(szName);
	if (!psInstruction->sResult.IsNone() && IsHiddenResult(psInstruction->sType))
	{
		PushFrameAddress(OffsetOf(psInstruction->sResult));
		iBytes += 4;
	}
	sz.Init("jsl ");
	sz.Append(szName);
	Line(sz.Text());
	sz.Kill();
	CallerCleanUp(iBytes, psInstruction);
}


//////////////////////////////////////////////////////////////////////////
//
// Calls softfloat.cpp's (compiled) szName with asValues (of asTypes; the
// one at iExtended, if not -1, extended to 32 bits as PushInt32 does) where
// its register entry takes them: placed as Call places a register
// function's arguments (IR816AssignRegisters, the hidden result pointer
// first when bHidden; it points at the frame's iHiddenOffset), the pushed
// ones pushed, the others loaded (LoadRegisters), then a jsl to its
// suffixed label, so not through its stub.  When registers aren't used
// (SetRegisterArguments) or the extended value would go in them, all are
// pushed and the plain label is called.  Returns the bytes pushed, for the
// caller to remove.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR816Backend::SoftFloatCall(const char* szName, SIRReference* asValues, SIRType* asTypes, int iValues, int iExtended, bool bSigned, bool bHidden, int32 iHiddenOffset)
{
	CArrayIR816Arguments	asArguments;
	SIR816Argument*			psArgument;
	SIRReference			asRegisters[IR816_ARGUMENT_REGISTERS];
	int						aiWords[IR816_ARGUMENT_REGISTERS];
	int32					aiHidden[IR816_ARGUMENT_REGISTERS];
	int						iFirst;
	int						iRegisters;
	int						i;
	size					ui;
	int32					iBytes;
	CChars					sz;

	asArguments.Init(CIR816Backend::GetScratch());
	if (bHidden)
	{
		psArgument = asArguments.Add();
		psArgument->iWords = 2;
		psArgument->iRegister = -1;
	}
	for (i = 0; i < iValues; i++)
	{
		psArgument = asArguments.Add();
		psArgument->iWords = (i == iExtended) ? 2 : Words(asTypes[i]);
		psArgument->iRegister = -1;
	}
	iFirst = bHidden ? 1 : 0;
	iRegisters = gbIR816RegisterArguments ? IR816AssignRegisters(&asArguments) : 0;
	if ((iExtended >= 0) && (asArguments.Get(iFirst + iExtended)->iRegister >= 0))
	{
		iRegisters = 0;
		for (ui = 0; ui < asArguments.NumElements(); ui++)
		{
			asArguments.Get(ui)->iRegister = -1;
		}
	}

	//The pushed ones, last first, then the hidden result pointer.
	iBytes = 0;
	for (i = iValues - 1; i >= 0; i--)
	{
		psArgument = asArguments.Get(iFirst + i);
		if (psArgument->iRegister >= 0)
		{
			continue;
		}
		if (i == iExtended)
		{
			PushInt32(asValues[i], asTypes[i], bSigned);
		}
		else
		{
			PushValue(asValues[i], asTypes[i]);
		}
		iBytes += 2 * psArgument->iWords;
	}
	if (bHidden && (asArguments.Get(0)->iRegister < 0))
	{
		PushFrameAddress(iHiddenOffset);
		iBytes += 4;
	}

	//The registers' words.
	for (i = 0; i < iRegisters; i++)
	{
		asRegisters[i] = IRNone();
		aiWords[i] = 0;
		aiHidden[i] = -1;
		for (ui = 0; ui < asArguments.NumElements(); ui++)
		{
			psArgument = asArguments.Get(ui);
			if ((psArgument->iRegister >= 0) && (i >= psArgument->iRegister) && (i < psArgument->iRegister + psArgument->iWords))
			{
				aiWords[i] = i - psArgument->iRegister;
				if ((int)ui < iFirst)
				{
					aiHidden[i] = iHiddenOffset;
				}
				else
				{
					asRegisters[i] = asValues[(int)ui - iFirst];
				}
			}
		}
	}
	if (iRegisters > 0)
	{
		LoadRegisters(asRegisters, aiWords, aiHidden, iRegisters);
	}

	UseRuntime(szName);
	sz.Init("jsl ");
	sz.Append(szName);
	if (iRegisters > 0)
	{
		IR816AppendRegisterSuffix(&sz, &asArguments);
	}
	Line(sz.Text());
	sz.Kill();
	asArguments.Kill();
	return iBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// _SoftFloatCompare gives -1, 0 or 1, or 2 when either is a NaN, in A: each comparison is true for some of those.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::FloatCompare(SIRInstruction* psInstruction)
{
	int					aiValues[2];
	int					iValues;
	int32				iBytes;
	SIRReference		asValues[2];
	SIRType				asTypes[2];
	bool				bNot;
	int32				iTrue;
	int32				iFalse;
	int32				iEnd;
	int					i;
	CChars				sz;
	SIRType				sType;

	sType = psInstruction->sType;
	if (gbIR816SoftFloatInFrame)
	{
		Line("ldx ", FrameOperand(psInstruction->asArguments[0], sType, IR816_SCRATCH_RESULT));
		Line("ldy ", FrameOperand(psInstruction->asArguments[1], sType, IR816_SCRATCH_ARGUMENT_A));
		UseRuntime(sType.Is(IRTK_F64) ? "_SoftDoubleCompare" IR816_FRAME_OPERANDS : "_SoftFloatCompare" IR816_FRAME_OPERANDS);
		Line(sType.Is(IRTK_F64) ? "jsl _SoftDoubleCompare" IR816_FRAME_OPERANDS : "jsl _SoftFloatCompare" IR816_FRAME_OPERANDS);
		Frame("sta", IR816_SCRATCH_RESULT);
	}
	else
	{
		asValues[0] = psInstruction->asArguments[0];
		asValues[1] = psInstruction->asArguments[1];
		asTypes[0] = sType;
		asTypes[1] = sType;
		iBytes = SoftFloatCall(sType.Is(IRTK_F64) ? "_SoftDoubleCompare" : "_SoftFloatCompare", asValues, asTypes, 2, -1, false, false, -1);
		Frame("sta", IR816_SCRATCH_RESULT);
		if (iBytes > 0)
		{
			Line("tsc");
			Line("clc");
			Line("adc ", iBytes);
			Line("tcs");
		}
	}

	//The compare's results that make it true (or false, for fne and fordered).
	bNot = false;
	iValues = 1;
	switch (psInstruction->uiOpcode)
	{
	case IRO_FEq:		aiValues[0] = 0;	break;
	case IRO_FNe:		aiValues[0] = 0;	bNot = true;	break;
	case IRO_FLt:		aiValues[0] = -1;	break;
	case IRO_FGt:		aiValues[0] = 1;	break;
	case IRO_FLe:		aiValues[0] = -1;	aiValues[1] = 0;	iValues = 2;	break;
	case IRO_FGe:		aiValues[0] = 0;	aiValues[1] = 1;	iValues = 2;	break;
	case IRO_FOrdered:	aiValues[0] = 2;	bNot = true;	break;
	default:			aiValues[0] = 2;	break;
	}

	iTrue = NewLabel();
	iFalse = NewLabel();
	iEnd = NewLabel();
	sz.Init(CIR816Backend::GetScratch());
	Frame("lda", IR816_SCRATCH_RESULT);
	for (i = 0; i < iValues; i++)
	{
		Line("cmp ", aiValues[i] & 0xFFFF);
		sz.Clear();
		sz.Append("beq L");
		sz.Append(bNot ? iFalse : iTrue);
		Line(sz.Text());
	}
	sz.Clear();
	sz.Append("bra L");
	sz.Append(bNot ? iTrue : iFalse);
	Line(sz.Text());
	Label(iTrue);
	Line("lda 1");
	sz.Clear();
	sz.Append("bra L");
	sz.Append(iEnd);
	Line(sz.Text());
	Label(iFalse);
	Line("lda 0");
	Label(iEnd);
	Store(psInstruction->sResult, 0);
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Conversions with floats: an integer narrower than 32 bits converts as an int32_t (sign or zero extended), a 64
// bit one as a long long.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::FloatConversion(SIRInstruction* psInstruction)
{
	SIRType			sFrom;
	SIRType			sTo;
	SIRReference	sValue;
	bool			bSigned;
	bool			bHidden;
	CChars			szName;
	int32			iBytes;
	int32			iFirst;
	int				iExtended;	//The value extended to 32 bits (SoftFloatCall), or -1.

	sValue = psInstruction->asArguments[0];
	iExtended = -1;
	bSigned = false;
	sFrom = mpcFunction->TypeOf(sValue);
	sTo = psInstruction->sType;
	szName.Init(CIR816Backend::GetScratch());
	switch (psInstruction->uiOpcode)
	{
	case IRO_FExt:
	case IRO_FTrunc:
		szName.Append(sTo.Is(IRTK_F64) ? "_SoftFloatToDouble" : "_SoftDoubleToFloat");
		break;
	case IRO_SIToF:
	case IRO_UIToF:
		bSigned = psInstruction->uiOpcode == IRO_SIToF;
		szName.Append(sTo.Is(IRTK_F64) ? "_SoftDoubleFrom" : "_SoftFloatFrom");
		if (sFrom.Is(IRTK_I64))
		{
			szName.Append(bSigned ? "LongLong" : "UnsignedLongLong");
		}
		else
		{
			szName.Append((bSigned || (sFrom.IntegerBits() < 32)) ? "Int" : "Unsigned");
			iExtended = 0;
		}
		break;
	default:
		bSigned = psInstruction->uiOpcode == IRO_FToSI;
		szName.Append(sFrom.Is(IRTK_F64) ? "_SoftDoubleTo" : "_SoftFloatTo");
		if (sTo.Is(IRTK_I64))
		{
			szName.Append(bSigned ? "LongLong" : "UnsignedLongLong");
		}
		else
		{
			szName.Append((bSigned || (sTo.IntegerBits() < 32)) ? "Int" : "Unsigned");
		}
		break;
	}
	if (gbIR816SoftFloatInFrame)
	{
		if (((psInstruction->uiOpcode == IRO_SIToF) || (psInstruction->uiOpcode == IRO_UIToF)) && (Words(sFrom) < 2))
		{
			//Extended to 32 bits (as PushInt32 does) in the scratch bytes.
			Operand("lda", sValue, 0);
			if (sFrom.Is(IRTK_I8) || sFrom.Is(IRTK_I1))
			{
				Mask(sFrom);
				if (bSigned && sFrom.Is(IRTK_I8))
				{
					SignExtendInA(7);
				}
			}
			Frame("sta", IR816_SCRATCH_RESULT);
			if (bSigned)
			{
				Line("cmp 0x8000");
				Line("lda 0");
				Line("bcc C + 1");
				Line("dec A");
			}
			else
			{
				Line("lda 0");
			}
			Frame("sta", IR816_SCRATCH_RESULT + 2);
			iFirst = IR816_SCRATCH_RESULT;
		}
		else
		{
			iFirst = FrameOperand(sValue, sFrom, IR816_SCRATCH_RESULT);
		}
		FrameOperandCall(szName.Text(), psInstruction, iFirst, -1);
	}
	else
	{
		bHidden = IsHiddenResult(sTo);
		iBytes = SoftFloatCall(szName.Text(), &sValue, &sFrom, 1, iExtended, bSigned, bHidden, psInstruction->sResult.IsNone() ? IR816_SCRATCH_RESULT : OffsetOf(psInstruction->sResult));
		CallerCleanUp(iBytes, psInstruction);
	}
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes an integer of up to 32 bits as 32 bits, sign or zero extended.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PushInt32(SIRReference sValue, SIRType sType, bool bSigned)
{
	if (Words(sType) == 2)
	{
		PushValue(sValue, sType);
		return;
	}
	Operand("lda", sValue, 0);
	if (bSigned && sType.Is(IRTK_I8))
	{
		Line("eor 0x80");
		Line("sec");
		Line("sbc 0x80");
	}
	Frame("sta", IR816_SCRATCH_VALUE);
	if (bSigned && !sType.Is(IRTK_I1))
	{
		Line("asl A");
		Line("lda 0");
		Line("adc 0xFFFF");
		Line("eor 0xFFFF");
	}
	else
	{
		Line("lda 0");
	}
	Line("pha");
	Frame("lda", IR816_SCRATCH_VALUE);
	Line("pha");
}


//////////////////////////////////////////////////////////////////////////
//
// add, sub and ptradd: a word at a time with the carry between them
// (WordInA), or an i16 kept in X stepped by 1 or 2.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Arithmetic(SIRInstruction* psInstruction)
{
	int		i;
	int		iWords;
	uint16	uiStep;

	iWords = Words(psInstruction->sType);
	if ((iWords == 1) && psInstruction->sType.Is(IRTK_I16) && InX(psInstruction->sResult) && InX(psInstruction->asArguments[0]) &&
		ConstantWord(psInstruction->asArguments[1], 0, &uiStep) && (uiStep >= 1) && (uiStep <= 2))
	{
		for (i = 0; i < uiStep; i++)
		{
			Line((psInstruction->uiOpcode == IRO_Sub) ? "dex" : "inx");
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
bool CIR816Backend::StepInPlace(SIRInstruction* psInstruction)
{
	SIRReference	sValue;
	uint32			uiStep;
	int32			iOffset;
	int32			iSkip;
	bool			bWord;

	sValue = psInstruction->asArguments[0];
	bWord = psInstruction->sType.Is(IRTK_I16);
	if (((Words(psInstruction->sType) != 2) && !bWord) || !psInstruction->sResult.IsTemporary() || !sValue.IsTemporary() || Fused(sValue) ||
		!psInstruction->asArguments[1].IsConstant() || InX(sValue) || InX(psInstruction->sResult) ||
		(OffsetOf(psInstruction->sResult) != OffsetOf(sValue)) || (OffsetOf(sValue) > 253))
	{
		return false;
	}
	uiStep = (uint32)mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits;
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
	iOffset = OffsetOf(sValue);
	if (bWord)
	{
		Frame((uiStep == 1) ? "inc" : "dec", iOffset);
		return true;
	}
	iSkip = NewLabel();
	if (uiStep == 1)
	{
		Frame("inc", iOffset);
		Branch("bne", iSkip);
		Frame("inc", iOffset + 2);
		Label(iSkip);
	}
	else
	{
		Frame("lda", iOffset);
		Branch("bne", iSkip);
		Frame("dec", iOffset + 2);
		Label(iSkip);
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
void CIR816Backend::Logic(SIRInstruction* psInstruction)
{
	int		i;
	uint16	uiWord;

	for (i = 0; i < Words(psInstruction->sType); i++)
	{
		if ((psInstruction->uiOpcode == IRO_And) && (ConstantWord(psInstruction->asArguments[1], i, &uiWord) || ConstantWord(psInstruction->asArguments[0], i, &uiWord)) && (uiWord == 0))
		{
			if (InX(psInstruction->sResult))
			{
				Line("ldx 0");
			}
			else
			{
				Frame("stz", OffsetOf(psInstruction->sResult) + 2 * i);
			}
			continue;
		}
		WordInA(psInstruction, i);
		if (psInstruction->uiOpcode == IRO_Not)
		{
			Mask(psInstruction->sType);
		}
		Store(psInstruction->sResult, i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The value is copied to the result and shifted there: by a constant (ShiftConstant), or a bit at a time by a count in
// X with a word in A (ShiftInA), or for 64 bits or a frame past the direct page in memory (ShiftInMemory).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Shift(SIRInstruction* psInstruction)
{
	int32			iResult;
	int				iWords;
	int32			iLoop;
	int32			iEnd;
	SIRReference	sAmount;
	SIRType			sType;

	sType = psInstruction->sType;
	iResult = OffsetOf(psInstruction->sResult);
	iWords = Words(sType);
	sAmount = psInstruction->asArguments[1];
	if (InX(psInstruction->sResult))
	{
		//A 16 bit shift by a constant (AllocateX), in A.
		Operand("lda", psInstruction->asArguments[0], 0);
		ShiftWordInA(psInstruction->uiOpcode, (int)(mpcFunction->GetConstant(sAmount)->ulliBits & 63));
		Line("tax");
		return;
	}
	if (BitShift(psInstruction) || ShiftTwoWords(psInstruction) || ShiftCall(psInstruction))
	{
		return;
	}
	CopyWords(iResult, psInstruction->asArguments[0], iWords);
	if (sAmount.IsConstant() && ((mpcFunction->GetConstant(sAmount)->ulliBits & 63) == 0))
	{
		return;
	}

	if ((psInstruction->uiOpcode == IRO_AShr) && (sType.Is(IRTK_I8) || sType.Is(IRTK_I1)))
	{
		Frame("lda", iResult);
		Line(sType.Is(IRTK_I8) ? "eor 0x80" : "eor 1");
		Line("sec");
		Line(sType.Is(IRTK_I8) ? "sbc 0x80" : "sbc 1");
		Frame("sta", iResult);
	}

	if (sAmount.IsConstant())
	{
		//Without a count in X (a value kept in X may be live: AllocateX only keeps them from variable shifts).
		ShiftConstant(psInstruction->uiOpcode, iResult, iWords, (int)(mpcFunction->GetConstant(sAmount)->ulliBits & 63));
		if (sType.Is(IRTK_I8) || sType.Is(IRTK_I1))
		{
			Frame("lda", iResult);
			Mask(sType);
			Frame("sta", iResult);
		}
		return;
	}

	iLoop = NewLabel();
	iEnd = NewLabel();
	if ((iWords <= 2) && (iResult + 2 * iWords <= 255))
	{
		ShiftInA(psInstruction->uiOpcode, iResult, iWords, sAmount, iLoop, iEnd);
	}
	else
	{
		ShiftInMemory(psInstruction->uiOpcode, iResult, iWords, sAmount, iLoop, iEnd);
	}
	if (sType.Is(IRTK_I8) || sType.Is(IRTK_I1))
	{
		Frame("lda", iResult);
		Mask(sType);
		Frame("sta", iResult);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A 32 or 64 bit shift by a count known when it runs calls a runtime
// routine (IR816AppendShift), the count in A: a 32 bit value is copied to
// the scratch argument a and comes back in A and X; a 64 bit one is copied
// to the result and shifted there (its place in X).  False, with nothing
// written, for any other shift.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::ShiftCall(SIRInstruction* psInstruction)
{
	CChars	szName;
	int		iWords;

	iWords = Words(psInstruction->sType);
	if (psInstruction->asArguments[1].IsConstant() || ((iWords != 2) && (iWords != 4)) || psInstruction->sResult.IsNone())
	{
		return false;
	}
	szName.Init(IR816_RUNTIME_PREFIX);
	switch (psInstruction->uiOpcode)
	{
	case IRO_Shl:	szName.Append("Shl");	break;
	case IRO_LShr:	szName.Append("LShr");	break;
	default:		szName.Append("AShr");	break;
	}
	szName.Append(16 * iWords);
	UseRuntime(szName.Text());
	if (iWords == 2)
	{
		CopyWords(IR816_SCRATCH_ARGUMENT_A, psInstruction->asArguments[0], 2);
		Operand("lda", psInstruction->asArguments[1], 0);
	}
	else
	{
		CopyWords(OffsetOf(psInstruction->sResult), psInstruction->asArguments[0], 4);
		Operand("lda", psInstruction->asArguments[1], 0);
		Line("ldx ", OffsetOf(psInstruction->sResult));
	}
	szName.Insert(0, "jsl ");
	Line(szName.Text());
	szName.Kill();
	if (iWords == 2)
	{
		CallerCleanUp(0, psInstruction);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A 32 bit shl or lshr by 1 to 7 from the value to the result, not copied
// to the result first: a bit through A from word to word, or more with the
// high word copied and the low word read into A, shifted there and stored.
// (The result is at the value's place or apart from it: the value's word
// still to be read isn't written first.)  False, with nothing written, for
// any other shift, or a result past the direct page.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::ShiftTwoWords(SIRInstruction* psInstruction)
{
	SIRReference	sValue;
	int32			iResult;
	int				iAmount;
	int				i;

	if ((Words(psInstruction->sType) != 2) || !psInstruction->asArguments[1].IsConstant() || psInstruction->sResult.IsNone() ||
		((psInstruction->uiOpcode != IRO_Shl) && (psInstruction->uiOpcode != IRO_LShr)))
	{
		return false;
	}
	iAmount = (int)(mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits & 63);
	iResult = OffsetOf(psInstruction->sResult);
	if ((iAmount < 1) || (iAmount > 7) || (iResult + 4 > 255))
	{
		return false;
	}
	sValue = psInstruction->asArguments[0];
	if (iAmount == 1)
	{
		if (psInstruction->uiOpcode == IRO_Shl)
		{
			Operand("lda", sValue, 0);
			Line("asl A");
			Frame("sta", iResult);
			Operand("lda", sValue, 1);
			Line("rol A");
			Frame("sta", iResult + 2);
		}
		else
		{
			Operand("lda", sValue, 1);
			Line("lsr A");
			Frame("sta", iResult + 2);
			Operand("lda", sValue, 0);
			Line("ror A");
			Frame("sta", iResult);
		}
		return true;
	}
	Operand("lda", sValue, 1);
	Frame("sta", iResult + 2);
	Operand("lda", sValue, 0);
	for (i = 0; i < iAmount; i++)
	{
		if (psInstruction->uiOpcode == IRO_Shl)
		{
			Line("asl A");
			Frame("rol", iResult + 2);
		}
		else
		{
			Frame("lsr", iResult + 2);
			Line("ror A");
		}
	}
	Frame("sta", iResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// 1 << n of 16, 32 or 64 bits, n known when it runs, looked up in
// R816_Bits16, R816_Bits32 or R816_Bits64 (AppendBitsTable) at n * 2, 4 or
// 8 in X instead of a shift: bitmaps' masks, and soft float's rounding
// masks ((1 << n) - 1, the subtract after).  False, with nothing written,
// for any other shift, or a result past the direct page (Frame would use
// X).
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::BitShift(SIRInstruction* psInstruction)
{
	CChars	szTable;
	int32	iResult;
	int		iWords;
	int		i;

	iWords = Words(psInstruction->sType);
	if ((psInstruction->uiOpcode != IRO_Shl) || psInstruction->asArguments[1].IsConstant() || !psInstruction->asArguments[0].IsConstant() ||
		(mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits != 1) || !(psInstruction->sType.Is(IRTK_I16) || psInstruction->sType.Is(IRTK_I32) || psInstruction->sType.Is(IRTK_I64)) ||
		InX(psInstruction->sResult) || psInstruction->sResult.IsNone())
	{
		return false;
	}
	iResult = OffsetOf(psInstruction->sResult);
	if (iResult + 2 * iWords > 255)
	{
		return false;
	}
	szTable.Init(IR816_BITS_TABLE);
	szTable.Append(16 * iWords);
	UseRuntime(szTable.Text());
	Operand("lda", psInstruction->asArguments[1], 0);
	Line("and 63");
	for (i = 1; i < 2 * iWords; i *= 2)
	{
		Line("asl A");
	}
	Line("tax");
	for (i = 0; i < iWords; i++)
	{
		mpszOut->Append("\t\tlda [")->Append(szTable);
		if (i > 0)
		{
			mpszOut->Append(" + ")->Append(2 * i);
		}
		mpszOut->Append(" + X]:24\n");
		Frame("sta", iResult + 2 * i);
	}
	szTable.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A shift by a count known when it runs, with a word in A: the value's
// word, or for 32 bits the word the bits leave from, the other rolled in
// memory.  Counts below IR816_SHIFT_LOOP_BELOW loop a bit at a time (the
// cheapest for so few).  Larger counts past the width fill the value
// (ShiftConstant); a whole word (32 bits) and then a byte are moved as
// ShiftConstant moves them; the 0 to 7 bits left jump (jmp [table + X])
// into an unrolled run of seven one bit steps.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::ShiftInA(uint16 uiOpcode, int32 iResult, int iWords, SIRReference sAmount, int32 iLoop, int32 iEnd)
{
	int32	iSmall;
	int32	iSmallLoop;
	int32	iUnder;
	int32	iBits;
	int32	iDone;
	int32	aiSteps[IR816_SHIFT_STEPS];
	int32	iInA;
	int32	iOther;
	int		i;
	CChars	sz;

	//Left shifts carry from the low word up, right shifts from the high word down.
	iInA = (uiOpcode == IRO_Shl) ? iResult : iResult + 2 * (iWords - 1);
	iOther = (uiOpcode == IRO_Shl) ? iResult + 2 : iResult;

	//The count is tested in A; it waits in X while a word or byte move uses A.
	iSmall = NewLabel();
	Operand("lda", sAmount, 0);
	Line("and 63");
	Branch("beq", iEnd);
	sz.Init("cmp ");
	sz.Append(IR816_SHIFT_LOOP_BELOW);
	Line(sz.Text());
	sz.Kill();
	Branch("bcc", iSmall);

	//The whole value shifted out.
	iUnder = NewLabel();
	Line((iWords == 2) ? "cmp 32" : "cmp 16");
	Branch("bcc", iUnder);
	ShiftConstant(uiOpcode, iResult, iWords, 16 * iWords);
	Branch("bra", iEnd);
	Label(iUnder);

	if (iWords == 2)
	{
		iBits = NewLabel();
		Line("cmp 16");
		Branch("bcc", iBits);
		Line("tax");
		ShiftConstant(uiOpcode, iResult, 2, 16);
		Line("txa");
		Line("and 15");
		Branch("beq", iEnd);
		Label(iBits);
	}

	iBits = NewLabel();
	Line("cmp 8");
	Branch("bcc", iBits);
	Line("tax");
	ShiftConstant(uiOpcode, iResult, iWords, 8);
	Line("txa");
	Line("and 7");
	Branch("beq", iEnd);
	Label(iBits);

	iDone = NewLabel();
	for (i = 0; i < IR816_SHIFT_STEPS; i++)
	{
		aiSteps[i] = NewLabel();
	}
	Line("asl A");
	Line("tax");
	Frame("lda", iInA);
	sz.Init("jmp [L");
	sz.Append(iLoop);
	sz.Append(" + X]");
	Line(sz.Text());
	sz.Kill();

	//Entry n runs the last n steps.
	Label(iLoop);
	for (i = 0; i <= IR816_SHIFT_STEPS; i++)
	{
		sz.Init(".word L");
		sz.Append((i == 0) ? iDone : aiSteps[IR816_SHIFT_STEPS - i]);
		sz.Append(" & 0xFFFF");
		Line(sz.Text());
		sz.Kill();
	}
	for (i = 0; i < IR816_SHIFT_STEPS; i++)
	{
		Label(aiSteps[i]);
		ShiftStepInA(uiOpcode, iWords, iOther);
	}
	Label(iDone);
	Frame("sta", iInA);
	Branch("bra", iEnd);

	//A few bits: one a time, counted down in X.
	Label(iSmall);
	iSmallLoop = NewLabel();
	Line("tax");
	Frame("lda", iInA);
	Label(iSmallLoop);
	ShiftStepInA(uiOpcode, iWords, iOther);
	Line("dex");
	Branch("bne", iSmallLoop);
	Frame("sta", iInA);
	Label(iEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// One bit of a shift with a word in A (the other word, for 32 bits, rolled
// in memory at iOther).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::ShiftStepInA(uint16 uiOpcode, int iWords, int32 iOther)
{
	if (uiOpcode == IRO_Shl)
	{
		Line("asl A");
	}
	else if (uiOpcode == IRO_LShr)
	{
		Line("lsr A");
	}
	else
	{
		Line("cmp 0x8000");
		Line("ror A");
	}
	if (iWords == 2)
	{
		Frame((uiOpcode == IRO_Shl) ? "rol" : "ror", iOther);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The 16 bit word in A shifted iAmount bits: 8 of them at once by
// swapping its bytes (xba, then the byte shifted in made 0), the rest a
// bit at a time.  An ashr by 15 or more is the sign (asl puts it in the
// carry; 0 + $FFFF and the carry is 0 or $FFFF, inverted); one by 3 or more
// is the lsr then the sign bit, now at bit 15 - iAmount, extended (x ^ m) -
// m (IR816SignExtend); by 1 or 2 a ror with the sign in the carry.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::ShiftWordInA(uint16 uiOpcode, int iAmount)
{
	int		iBits;

	if ((uiOpcode == IRO_AShr) && (iAmount >= 15))
	{
		Line("asl A");
		Line("lda 0");
		Line("adc 0xFFFF");
		Line("eor 0xFFFF");
		return;
	}
	if (iAmount >= 16)
	{
		Line("lda 0");
		return;
	}
	if ((uiOpcode == IRO_AShr) && (iAmount < 3))
	{
		for (iBits = 0; iBits < iAmount; iBits++)
		{
			Line("cmp 0x8000");
			Line("ror A");
		}
		return;
	}

	iBits = iAmount;
	if (iBits >= 8)
	{
		Line("xba");
		Line((uiOpcode == IRO_Shl) ? "and 0xFF00" : "and 0xFF");
		iBits -= 8;
	}
	for (; iBits > 0; iBits--)
	{
		Line((uiOpcode == IRO_Shl) ? "asl A" : "lsr A");
	}
	if ((uiOpcode == IRO_AShr) && (iAmount > 0))
	{
		SignExtendInA(15 - iAmount);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The word in A (0 above bit iSignBit) with that bit copied up through bit
// 15: (x ^ m) - m with m the sign bit.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::SignExtendInA(int iSignBit)
{
	char	szLine[32];

	snprintf(szLine, sizeof(szLine), "eor 0x%X", 1u << iSignBit);
	Line(szLine);
	Line("sec");
	snprintf(szLine, sizeof(szLine), "sbc 0x%X", 1u << iSignBit);
	Line(szLine);
}


//////////////////////////////////////////////////////////////////////////
//
// A shift in memory (64 bits, or a frame past the direct page) by a count
// known when it runs, kept in the scratch value word (ShiftConstant's ashr
// uses the scratch count for its sign word, and X addresses frames past the
// direct page).  Counts past the width fill the value; then 32 and 16 bits
// are moved as whole words and 8 as a byte (ShiftConstant), so only the 0
// to 7 bits left are shifted a bit at a time.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::ShiftInMemory(uint16 uiOpcode, int32 iResult, int iWords, SIRReference sAmount, int32 iLoop, int32 iEnd)
{
	static const int	aiSteps[] = { 32, 16, 8 };
	CChars				sz;
	int					i;
	int32				iSkip;

	Operand("lda", sAmount, 0);
	Line("and 63");
	Branch("beq", iEnd);

	//The whole value shifted out (64 bits never is: the count is below 64).
	if (16 * iWords <= 63)
	{
		iSkip = NewLabel();
		sz.Init("cmp ");
		sz.Append(16 * iWords);
		Line(sz.Text());
		sz.Kill();
		Branch("bcc", iSkip);
		ShiftConstant(uiOpcode, iResult, iWords, 16 * iWords);
		Branch("bra", iEnd);
		Label(iSkip);
	}
	Frame("sta", IR816_SCRATCH_VALUE);

	//Each step only when the count, now less than twice it, reaches it.
	for (i = 0; i < 3; i++)
	{
		if (aiSteps[i] >= 16 * iWords)
		{
			continue;
		}
		iSkip = NewLabel();
		sz.Init("cmp ");
		sz.Append(aiSteps[i]);
		Line(sz.Text());
		Branch("bcc", iSkip);
		sz.Kill();
		ShiftConstant(uiOpcode, iResult, iWords, aiSteps[i]);
		Frame("lda", IR816_SCRATCH_VALUE);
		sz.Init("and ");
		sz.Append(aiSteps[i] - 1);
		Line(sz.Text());
		sz.Kill();
		Branch("beq", iEnd);
		Frame("sta", IR816_SCRATCH_VALUE);
		Label(iSkip);
	}

	Label(iLoop);
	if (uiOpcode == IRO_Shl)
	{
		Frame("asl", iResult);
		for (i = 1; i < iWords; i++)
		{
			Frame("rol", iResult + 2 * i);
		}
	}
	else
	{
		if (uiOpcode == IRO_LShr)
		{
			Frame("lsr", iResult + 2 * (iWords - 1));
		}
		else
		{
			Frame("lda", iResult + 2 * (iWords - 1));
			Line("cmp 0x8000");
			Line("ror A");
			Frame("sta", iResult + 2 * (iWords - 1));
		}
		for (i = iWords - 2; i >= 0; i--)
		{
			Frame("ror", iResult + 2 * i);
		}
	}
	Frame("dec", IR816_SCRATCH_VALUE);
	Branch("bne", iLoop);
	Label(iEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// The value at iResult (iWords words) shifted iAmount bits where it is: whole
// words are moved first (the words shifted in are 0, or the sign for ashr),
// then a byte when 8 or more bits are left, then the bits left a bit at a
// time without a count: one word in A, two with the low one in A, more in
// memory; ashr's bits as lshr's, then the top word's sign extended.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::ShiftConstant(uint16 uiOpcode, int32 iResult, int iWords, int iAmount)
{
	int		iWordShift;
	int		iBits;
	int		i;
	int		j;
	int32	iLow;
	int		iRemaining;

	if (iWords == 1)
	{
		Frame("lda", iResult);
		ShiftWordInA(uiOpcode, iAmount);
		Frame("sta", iResult);
		return;
	}

	iWordShift = iAmount / 16;
	iBits = iAmount % 16;
	if (iWordShift >= iWords)
	{
		iWordShift = iWords;
		iBits = 0;
	}

	if (iWordShift > 0)
	{
		if (uiOpcode == IRO_Shl)
		{
			for (i = iWords - 1; i >= iWordShift; i--)
			{
				Frame("lda", iResult + 2 * (i - iWordShift));
				Frame("sta", iResult + 2 * i);
			}
			for (i = iWordShift - 1; i >= 0; i--)
			{
				Frame("stz", iResult + 2 * i);
			}
		}
		else
		{
			if (uiOpcode == IRO_AShr)
			{
				//The sign word: $FFFF when the top bit is set.
				Frame("lda", iResult + 2 * (iWords - 1));
				Line("cmp 0x8000");
				Line("lda 0");
				Line("bcc C + 1");
				Line("dec A");
				Frame("sta", IR816_SCRATCH_COUNT);
			}
			for (i = 0; i < iWords - iWordShift; i++)
			{
				Frame("lda", iResult + 2 * (i + iWordShift));
				Frame("sta", iResult + 2 * i);
			}
			for (i = iWords - iWordShift; i < iWords; i++)
			{
				if (uiOpcode == IRO_AShr)
				{
					Frame("lda", IR816_SCRATCH_COUNT);
					Frame("sta", iResult + 2 * i);
				}
				else
				{
					Frame("stz", iResult + 2 * i);
				}
			}
		}
	}

	if (iBits >= 8)
	{
		//A byte is moved a word at a time: a word read a byte along is the bytes shifted 8 bits.  The byte past the
		//value's end (or before its start) is 0, or the sign for ashr (the words moved in are), except at the value's
		//own ends.
		if (uiOpcode == IRO_Shl)
		{
			for (i = iWords - 1; i >= iWordShift; i--)
			{
				Frame("lda", iResult + 2 * i - 1);
				if (i == 0)
				{
					Line("and 0xFF00");
				}
				Frame("sta", iResult + 2 * i);
			}
		}
		else
		{
			for (i = 0; i < iWords - iWordShift; i++)
			{
				Frame("lda", iResult + 2 * i + 1);
				if (i == iWords - 1)
				{
					Line("and 0xFF");
					if (uiOpcode == IRO_AShr)
					{
						Line("cmp 0x80");
						Line("bcc C + 3");
						Line("ora 0xFF00");
					}
				}
				Frame("sta", iResult + 2 * i);
			}
		}
		iBits -= 8;
	}

	if (iBits == 0)
	{
		return;
	}
	iLow = (uiOpcode == IRO_Shl) ? iResult + 2 * iWordShift : iResult;
	iRemaining = iWords - iWordShift;
	if (iRemaining == 1)
	{
		//The one word left, shifted in A (a bit of shl or lshr in memory).
		if ((iBits == 1) && (uiOpcode != IRO_AShr))
		{
			Frame((uiOpcode == IRO_Shl) ? "asl" : "lsr", iLow);
			return;
		}
		Frame("lda", iLow);
		ShiftWordInA(uiOpcode, iBits);
		Frame("sta", iLow);
		return;
	}
	if (iRemaining == 2)
	{
		//Two words left: the low one in A, the bit between them carried through memory's high word.
		Frame("lda", iLow);
		for (j = 0; j < iBits; j++)
		{
			if (uiOpcode == IRO_Shl)
			{
				Line("asl A");
				Frame("rol", iLow + 2);
			}
			else
			{
				Frame("lsr", iLow + 2);
				Line("ror A");
			}
		}
		Frame("sta", iLow);
	}
	else
	{
		//More words, a bit at a time in memory.
		for (j = 0; j < iBits; j++)
		{
			if (uiOpcode == IRO_Shl)
			{
				Frame("asl", iLow);
				for (i = 1; i < iRemaining; i++)
				{
					Frame("rol", iLow + 2 * i);
				}
			}
			else
			{
				Frame("lsr", iLow + 2 * (iRemaining - 1));
				for (i = iRemaining - 2; i >= 0; i--)
				{
					Frame("ror", iLow + 2 * i);
				}
			}
		}
	}

	//ashr was shifted as lshr: the top word left gets its sign bit, now at 15 - iBits, extended.
	if (uiOpcode == IRO_AShr)
	{
		Frame("lda", iLow + 2 * (iRemaining - 1));
		SignExtendInA(15 - iBits);
		Frame("sta", iLow + 2 * (iRemaining - 1));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether RuntimeCall calls a scratch routine (IR816ScratchRoutine) for
// psInstruction, of two or more words: its operands are copied a word at a
// time to the scratch bytes, the first then the second.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::IsScratchCall(SIRInstruction* psInstruction)
{
	CChars	szName;
	bool	bScratch;

	if (psInstruction->sType.IsFloat() || (Words(psInstruction->sType) < 2))
	{
		return false;
	}
	RuntimeName(&szName, psInstruction);
	bScratch = IR816ScratchRoutine(szName.Text());
	szName.Kill();
	return bScratch;
}


//////////////////////////////////////////////////////////////////////////
//
// mul, udiv, sdiv, urem and srem call R816_<operation><bits>, an i8 as 16 bits (sign extended for sdiv and srem).
// The scratch routines get their operands at IR816_SCRATCH_ARGUMENT_A and _B, the others pushed.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::RuntimeCall(SIRInstruction* psInstruction)
{
	CChars		szName;
	SIRType		sType;
	int32		iBytes;
	bool		bSigned;
	int			i;

	sType = psInstruction->sType;
	if (sType.IsFloat() || sType.Is(IRTK_I1))
	{
		return Error("Multiply and divide are for i8 to i64");
	}
	bSigned = (psInstruction->uiOpcode == IRO_SDiv) || (psInstruction->uiOpcode == IRO_SRem);
	if (TableDivide(psInstruction, bSigned) || ByteMultiply(psInstruction) || WideMultiply(psInstruction))
	{
		return true;
	}

	RuntimeName(&szName, psInstruction);

	iBytes = 0;
	if (IR816ScratchRoutine(szName.Text()))
	{
		CopyWords(IR816_SCRATCH_ARGUMENT_A, psInstruction->asArguments[0], Words(sType));
		CopyWords(IR816_SCRATCH_ARGUMENT_B, psInstruction->asArguments[1], Words(sType));
	}
	else
	{
		iBytes = 2 * ValueBytes(sType);
		for (i = 1; i >= 0; i--)
		{
			if (bSigned && sType.Is(IRTK_I8))
			{
				Operand("lda", psInstruction->asArguments[i], 0);
				Line("eor 0x80");
				Line("sec");
				Line("sbc 0x80");
				Line("pha");
			}
			else
			{
				PushValue(psInstruction->asArguments[i], sType);
			}
		}
		if (IsHiddenResult(sType))
		{
			PushFrameAddress(OffsetOf(psInstruction->sResult));
			iBytes += 4;
		}
	}
	UseRuntime(szName.Text());
	szName.Insert(0, "jsl ");
	Line(szName.Text());
	szName.Kill();
	CallerCleanUp(iBytes, psInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The instruction that works out the temporary sValue, or NULL (a
// parameter, a phi, or not a temporary).
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIR816Backend::FindDefinition(SIRReference sValue)
{
	size			uiBlock;
	size			ui;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;

	if (!sValue.IsTemporary())
	{
		return NULL;
	}
	for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
		{
			psInstruction = pcBlock->masInstructions.Get(ui);
			if (psInstruction->sResult.Equals(sValue))
			{
				return psInstruction;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the 64 bit sValue's high 32 bits are known to be 0: a constant
// under 2^32, a zext of 32 bits or fewer, an and with one, or an lshr by
// 32 or more.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::IsHighHalfZero(SIRReference sValue, int iDepth)
{
	SIRInstruction*	psDefinition;
	uint64			ulliAmount;

	if (sValue.IsConstant())
	{
		return (mpcFunction->GetConstant(sValue)->ulliBits >> 32) == 0;
	}
	psDefinition = FindDefinition(sValue);
	if ((psDefinition == NULL) || (iDepth > 2))
	{
		return false;
	}
	switch (psDefinition->uiOpcode)
	{
	case IRO_ZExt:
		return !psDefinition->asArguments[0].IsTemporary() || (mpcFunction->GetTemporaryType(psDefinition->asArguments[0]).Bits() <= 32);
	case IRO_And:
		return IsHighHalfZero(psDefinition->asArguments[0], iDepth + 1) || IsHighHalfZero(psDefinition->asArguments[1], iDepth + 1);
	case IRO_LShr:
		if (!psDefinition->asArguments[1].IsConstant())
		{
			return false;
		}
		ulliAmount = mpcFunction->GetConstant(psDefinition->asArguments[1])->ulliBits & 63;
		return ulliAmount >= 32;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A 64 bit multiply of two values whose high halves are 0 (IsHighHalfZero:
// softfloat's 32 bit halves of a fraction) is R816_UMulWide32 of their low
// halves, much faster than R816_Mul64.  False, with nothing written, for
// any other multiply.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::WideMultiply(SIRInstruction* psInstruction)
{
	int		i;

	if ((psInstruction->uiOpcode != IRO_Mul) || (Words(psInstruction->sType) != 4) ||
		!IsHighHalfZero(psInstruction->asArguments[0], 0) || !IsHighHalfZero(psInstruction->asArguments[1], 0))
	{
		return false;
	}
	CopyWords(IR816_SCRATCH_ARGUMENT_A, psInstruction->asArguments[0], 2);
	CopyWords(IR816_SCRATCH_ARGUMENT_B, psInstruction->asArguments[1], 2);
	UseRuntime(IR816_RUNTIME_PREFIX "UMulWide32");
	Line("jsl " IR816_RUNTIME_PREFIX "UMulWide32");
	if (!psInstruction->sResult.IsNone())
	{
		for (i = 0; i < 4; i++)
		{
			Frame("lda", IR816_SCRATCH_RESULT + 2 * i);
			Store(psInstruction->sResult, i);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A hot i8 divide or remainder by a constant (IRF_Hot, CIROptimiser::
// ConstantDivisions) looked up in R816_STable<c> or R816_UTable<c>: a word
// per byte value, q in its low byte and the remainder in its high byte.
// False, with nothing written, for any other.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::TableDivide(SIRInstruction* psInstruction, bool bSigned)
{
	CChars	szName;
	uint64	ulliDivisor;

	if (!(psInstruction->uiFlags & IRF_Hot) || !psInstruction->sType.Is(IRTK_I8) || !psInstruction->asArguments[1].IsConstant())
	{
		return false;
	}
	ulliDivisor = mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits & 0xFF;
	if ((ulliDivisor < 3) || (bSigned && (ulliDivisor >= 0x80)))
	{
		return false;
	}
	szName.Init(bSigned ? IR816_SIGNED_TABLE : IR816_UNSIGNED_TABLE);
	szName.Append((int)ulliDivisor);
	UseRuntime(szName.Text());
	Operand("lda", psInstruction->asArguments[0], 0);
	Line("asl A");
	Line("tax");
	mpszOut->Append("\t\tlda [")->Append(szName)->Append(" + X]:24\n");
	szName.Kill();
	if ((psInstruction->uiOpcode == IRO_SRem) || (psInstruction->uiOpcode == IRO_URem))
	{
		Line("xba");
	}
	Line("and 0xFF");
	if (!psInstruction->sResult.IsNone())
	{
		Store(psInstruction->sResult, 0);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An i8 multiply: the low byte of the product in MathMult (the word at
// (a * 256 + b) * 2 is a * b, as R816_Mul16 reads it), looked up in place.
// False, with nothing written, for any other multiply.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::ByteMultiply(SIRInstruction* psInstruction)
{
	int32	iHigh;
	int32	iGot;

	if ((psInstruction->uiOpcode != IRO_Mul) || !psInstruction->sType.Is(IRTK_I8))
	{
		return false;
	}
	iHigh = NewLabel();
	iGot = NewLabel();
	Operand("lda", psInstruction->asArguments[1], 0);
	Line("xba");
	Operand("ora", psInstruction->asArguments[0], 0);
	Line("asl A");
	Line("tax");
	Branch("bcs", iHigh);
	Line("lda [MathMult + X]:24");
	Branch("bra", iGot);
	Label(iHigh);
	Line("lda [MathMult + 0x10000 + X]:24");
	Label(iGot);
	Line("and 0xFF");
	if (!psInstruction->sResult.IsNone())
	{
		Store(psInstruction->sResult, 0);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// After a call: the arguments are removed (pulled into Y, or S moved with A kept in Y; X is kept), then the result (A,
// and X for 32 bits) goes to its place.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::CallerCleanUp(int32 iBytes, SIRInstruction* psInstruction)
{
	bool	bResult;
	int		iWords;
	int		i;

	bResult = !psInstruction->sResult.IsNone() && !IsHiddenResult(psInstruction->sType);
	iWords = bResult ? Words(psInstruction->sType) : 0;
	if ((iBytes == 2) || (iBytes == 4))
	{
		//Pulled into Y, which keeps A and X.
		for (i = 0; i < iBytes; i += 2)
		{
			Line("ply");
		}
	}
	else if (iBytes != 0)
	{
		if (bResult)
		{
			Line("tay");
		}
		Line("tsc");
		Line("clc");
		Line("adc ", iBytes);
		Line("tcs");
		if (bResult)
		{
			Line("tya");
		}
	}
	if (bResult)
	{
		if (iWords == 1)
		{
			Mask(psInstruction->sType);
			Store(psInstruction->sResult, 0);
		}
		else if (OffsetOf(psInstruction->sResult) + 2 <= 255)
		{
			Store(psInstruction->sResult, 0);
			Frame("stx", OffsetOf(psInstruction->sResult) + 2);
		}
		else
		{
			//Past the direct page Frame goes through X: the high word waits in the scratch word.
			Frame("stx", IR816_SCRATCH_RESULT + 2);
			Store(psInstruction->sResult, 0);
			Frame("lda", IR816_SCRATCH_RESULT + 2);
			Store(psInstruction->sResult, 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Leaves C and Z as an unsigned compare of the left with the right leaves them, high words first.  A signed compare
// flips the sign bits so it can compare unsigned (a constant's as it is written).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::CompareFlags(SIRReference sLeft, SIRReference sRight, SIRType sType, bool bSigned)
{
	int		iWords;
	int		i;
	int32	iDone;
	const char*	szFlip;
	uint64	uiFlip;
	CChars	sz;

	iWords = Words(sType);
	iDone = NewLabel();
	szFlip = sType.Is(IRTK_I1) ? "eor 1" : (sType.Is(IRTK_I8) ? "eor 0x80" : "eor 0x8000");
	uiFlip = sType.Is(IRTK_I1) ? 1 : (sType.Is(IRTK_I8) ? 0x80 : 0x8000);
	for (i = iWords - 1; i >= 0; i--)
	{
		if (bSigned && (i == iWords - 1) && sRight.IsConstant())
		{
			//The constant's sign bit flipped as it is written.
			Operand("lda", sLeft, i);
			Line(szFlip);
			Line("cmp ", (int64)((((i < 4) ? (mpcFunction->GetConstant(sRight)->ulliBits >> (16 * i)) : 0) ^ uiFlip) & 0xFFFF));
		}
		else if (bSigned && (i == iWords - 1))
		{
			Operand("lda", sRight, i);
			Line(szFlip);
			Frame("sta", IR816_SCRATCH_VALUE);
			Operand("lda", sLeft, i);
			Line(szFlip);
			Frame("cmp", IR816_SCRATCH_VALUE);
		}
		else if ((iWords == 1) && InX(sLeft))
		{
			Operand("cpx", sRight, i);
		}
		else
		{
			Operand("lda", sLeft, i);
			Operand("cmp", sRight, i);
		}
		if (i > 0)
		{
			sz.Init("bne L");
			sz.Append(iDone);
			Line(sz.Text());
			sz.Kill();
		}
	}
	Label(iDone);
}


//////////////////////////////////////////////////////////////////////////
//
// Leaves N set when the first of two 16 bit signed values is less than the
// second (the second less than the first when bSwapped): the subtraction's
// sign, flipped when it overflowed.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::SignedLessFlags(SIRReference* psArguments, bool bSwapped)
{
	int32	iNoOverflow;

	iNoOverflow = NewLabel();
	Line("sec");
	Operand("lda", psArguments[bSwapped ? 1 : 0], 0);
	Operand("sbc", psArguments[bSwapped ? 0 : 1], 0);
	Branch("bvc", iNoOverflow);
	Line("eor 0x8000");
	Label(iNoOverflow);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the instruction at uiIndex is an integer comparison the block's
// branch works out itself: the block's last instruction, whose result only
// the branch uses.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::IsFusedCompare(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;

	psInstruction = BranchComparison(pcBlock);
	if ((psInstruction == NULL) || (uiIndex + 1 != pcBlock->masInstructions.NumElements()))
	{
		return false;
	}
	switch (psInstruction->uiOpcode)
	{
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
		break;
	default:
		return false;
	}
	return (psInstruction->sResult.Index() < maiUses.NumElements()) && (*maiUses.Get(psInstruction->sResult.Index()) == 1);
}


//////////////////////////////////////////////////////////////////////////
//
// A branch to iLabel when a comparison is bWhen: the flags, then the
// branches on them (CompareFlags leaves C and Z as an unsigned compare
// leaves them).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::FusedBranch(SIRInstruction* psInstruction, bool bWhen, int32 iLabel)
{
	int32	iSkip;
	uint16	uiOpcode;
	uint16	uiZero;

	uiOpcode = bWhen ? psInstruction->uiOpcode : IRInvertedComparison(psInstruction->uiOpcode);
	if (IsIRSignedComparison(uiOpcode) && psInstruction->sType.Is(IRTK_I16) && ConstantWord(psInstruction->asArguments[1], 0, &uiZero) && (uiZero == 0))
	{
		//Against 0 the sign and zero flags of the value itself answer (lda or txa sets them).
		Operand("lda", psInstruction->asArguments[0], 0);
		switch (uiOpcode)
		{
		case IRO_SLt:
			Branch("bmi", iLabel);
			break;
		case IRO_SGe:
			Branch("bpl", iLabel);
			break;
		case IRO_SGt:
			iSkip = NewLabel();
			Branch("beq", iSkip);
			Branch("bpl", iLabel);
			Label(iSkip);
			break;
		default:
			Branch("bmi", iLabel);
			Branch("beq", iLabel);
			break;
		}
		return;
	}
	if (IsIRSignedComparison(uiOpcode) && psInstruction->sType.Is(IRTK_I16) && !psInstruction->asArguments[1].IsConstant())
	{
		//Two values: one subtraction with its overflow folded into the sign answers lt and ge; gt and le subtract the
		//other way around (a > b is b < a).
		SignedLessFlags(psInstruction->asArguments, (uiOpcode == IRO_SGt) || (uiOpcode == IRO_SLe));
		Branch(((uiOpcode == IRO_SLt) || (uiOpcode == IRO_SGt)) ? "bmi" : "bpl", iLabel);
		return;
	}
	CompareFlags(psInstruction->asArguments[0], psInstruction->asArguments[1], psInstruction->sType, IsIRSignedComparison(psInstruction->uiOpcode));
	switch (uiOpcode)
	{
	case IRO_Eq:
		Branch("beq", iLabel);
		break;
	case IRO_Ne:
		Branch("bne", iLabel);
		break;
	case IRO_SLt:
	case IRO_ULt:
		Branch("bcc", iLabel);
		break;
	case IRO_SGe:
	case IRO_UGe:
		Branch("bcs", iLabel);
		break;
	case IRO_SGt:
	case IRO_UGt:
		//gt: C set and Z clear.
		iSkip = NewLabel();
		Branch("beq", iSkip);
		Branch("bcs", iLabel);
		Label(iSkip);
		break;
	default:
		//le: C clear or Z set.
		Branch("bcc", iLabel);
		Branch("beq", iLabel);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The comparison's i1 (0 or 1).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Compare(SIRInstruction* psInstruction)
{
	int32	iTrue;
	int32	iFalse;
	int32	iEnd;
	uint16	uiOpcode;
	CChars	sz;

	uiOpcode = psInstruction->uiOpcode;
	CompareFlags(psInstruction->asArguments[0], psInstruction->asArguments[1], psInstruction->sType, IsIRSignedComparison(uiOpcode));
	iTrue = NewLabel();
	iFalse = NewLabel();
	iEnd = NewLabel();
	sz.Init(CIR816Backend::GetScratch());
	switch (uiOpcode)
	{
	case IRO_Eq:
		sz.Append("bne L");
		break;
	case IRO_Ne:
		sz.Append("beq L");
		break;
	case IRO_SLt:
	case IRO_ULt:
		sz.Append("bcs L");
		break;
	case IRO_SGe:
	case IRO_UGe:
		sz.Append("bcc L");
		break;
	case IRO_SGt:
	case IRO_UGt:
		sz.Append("bcc L");
		sz.Append(iFalse);
		Line(sz.Text());
		sz.Clear();
		sz.Append("beq L");
		break;
	default:
		//le: true when C is clear or Z set.
		sz.Append("bcc L");
		sz.Append(iTrue);
		Line(sz.Text());
		sz.Clear();
		sz.Append("bne L");
		break;
	}
	sz.Append(iFalse);
	Line(sz.Text());
	Label(iTrue);
	Line("lda 1");
	sz.Clear();
	sz.Append("bra L");
	sz.Append(iEnd);
	Line(sz.Text());
	Label(iFalse);
	Line("lda 0");
	Label(iEnd);
	Store(psInstruction->sResult, 0);
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An extension or trunc to one word kept in X: the value's low word in A,
// masked to a narrow result or sign extended from i8 or i1, then tax (a
// zero extension or trunc to 16 bits is ldx of the word).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::ExtendInX(SIRInstruction* psInstruction)
{
	SIRReference	sValue;
	SIRType			sSource;
	SIRType			sType;

	sValue = psInstruction->asArguments[0];
	sSource = mpcFunction->TypeOf(sValue);
	sType = psInstruction->sType;
	if ((psInstruction->uiOpcode == IRO_SExt) && sSource.Is(IRTK_I1))
	{
		Line("lda 0");
		Line("sec");
		Operand("sbc", sValue, 0);
		Mask(sType);
	}
	else if ((psInstruction->uiOpcode == IRO_SExt) && sSource.Is(IRTK_I8))
	{
		Operand("lda", sValue, 0);
		Line("eor 0x80");
		Line("sec");
		Line("sbc 0x80");
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
	Line("tax");
}


//////////////////////////////////////////////////////////////////////////
//
// zext, sext, trunc and the pointer conversions.  Narrow values are already zero extended in their word.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Extend(SIRInstruction* psInstruction)
{
	SIRType			sSource;
	SIRType			sType;
	SIRReference	sValue;
	int32			iResult;
	int				iSourceWords;
	int				iWords;
	int				iCopied;

	sValue = psInstruction->asArguments[0];
	sSource = mpcFunction->TypeOf(sValue);
	sType = psInstruction->sType;
	if (InX(psInstruction->sResult))
	{
		ExtendInX(psInstruction);
		return;
	}
	iResult = OffsetOf(psInstruction->sResult);
	iSourceWords = Words(sSource);
	iWords = Words(sType);
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
			Line("lda 0");
			Line("sec");
			Operand("sbc", sValue, 0);
			Mask(sType);
			Frame("sta", iResult);
			iCopied = 1;
		}
		else if (sSource.Is(IRTK_I8))
		{
			Operand("lda", sValue, 0);
			Line("eor 0x80");
			Line("sec");
			Line("sbc 0x80");
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
		Line("and 0xFF");
		Frame("sta", iResult + 2);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a temporary is used, and only as the count of shifts by a count
// known when they run (all of which read just its low word: BitShift,
// ShiftInA, ShiftInMemory, ShiftCall).
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::OnlyCounts(SIRReference sValue)
{
	size	ui;

	if (!sValue.IsTemporary())
	{
		return false;
	}
	ui = sValue.Index();
	return (ui < maiCountUses.NumElements()) && (ui < maiUses.NumElements()) && (*maiCountUses.Get(ui) > 0) && (*maiCountUses.Get(ui) == *maiUses.Get(ui));
}


//////////////////////////////////////////////////////////////////////////
//
// A slot's or argument's address: D + offset, in bank 0.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Address(SIRInstruction* psInstruction, int32 iOffset)
{
	Line("tdc");
	Line("clc");
	Line("adc ", iOffset);
	Store(psInstruction->sResult, 0);
	Frame("stz", OffsetOf(psInstruction->sResult) + 2);
}


//////////////////////////////////////////////////////////////////////////
//
// A word at a time (WordInA).  An i8 is read as a word and masked (the byte after it is only memory), or with m 8 bits
// when volatile (it may be a device's register).  A value in its pointer's place (p = p->psNext, PackTemporaries) has
// its words past the pointer read first, then its second word is kept on the stack while the first is read.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Load(SIRInstruction* psInstruction)
{
	int		i;
	bool	bByte;

	bByte = psInstruction->sType.Is(IRTK_I8) && (psInstruction->uiFlags & IRF_Volatile);
	if (bByte)
	{
		Line("sep m");
	}
	else if ((Words(psInstruction->sType) >= 2) && !InX(psInstruction->sResult) && FusedOverlaps(psInstruction->sResult, FoldedBase(psInstruction->asArguments[0]), true))
	{
		for (i = Words(psInstruction->sType) - 1; i >= 2; i--)
		{
			WordInA(psInstruction, i);
			Store(psInstruction->sResult, i);
		}
		WordInA(psInstruction, 1);
		Line("pha");
		WordInA(psInstruction, 0);
		Store(psInstruction->sResult, 0);
		Line("pla");
		Store(psInstruction->sResult, 1);
		return;
	}
	for (i = 0; i < Words(psInstruction->sType); i++)
	{
		WordInA(psInstruction, i);
		if (bByte)
		{
			Line("rep m");
		}
		Mask(psInstruction->sType);
		Store(psInstruction->sResult, i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether sResult's place overlaps sInput's (a temporary's) so that writing
// its first word changes a later word of sInput's: any overlap when
// bAnyOverlap (a pointer read for each word), else one not at the same place.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::FusedOverlaps(SIRReference sResult, SIRReference sInput, bool bAnyOverlap)
{
	int32	iResult;
	int32	iInput;

	if (!sInput.IsTemporary() || sResult.IsNone())
	{
		return false;
	}
	iResult = OffsetOf(sResult);
	iInput = OffsetOf(sInput);
	if ((iResult >= iInput + ValueBytes(mpcFunction->GetTemporaryType(sInput))) || (iInput >= iResult + ValueBytes(mpcFunction->GetTemporaryType(sResult))))
	{
		return false;
	}
	return bAnyOverlap || (iResult != iInput);
}


//////////////////////////////////////////////////////////////////////////
//
// The instruction waiting to be read into A (CanFuse) that works out
// sValue, else NULL.
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIR816Backend::Fused(SIRReference sValue)
{
	int		i;

	for (i = 0; i < miFused; i++)
	{
		if (mapsFused[i]->sResult.Equals(sValue))
		{
			return mapsFused[i];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether what psInstruction (to be fused) reads is still there for each
// word after sResult's earlier words are written: a load's pointer anywhere
// else (it is read for each word) and in the direct page, other values not
// partly overlapped.  The fused values it reads aren't written.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::FusedInputsKept(SIRInstruction* psInstruction, SIRReference sResult)
{
	SIRReference	sPointer;
	SIRReference	sIndex;
	int				i;

	if (psInstruction->uiOpcode == IRO_Load)
	{
		sPointer = FoldedBase(psInstruction->asArguments[0]);
		sIndex = FoldedIndex(psInstruction->asArguments[0]);
		if (sIndex.IsTemporary() && !InX(sIndex) && FusedOverlaps(sResult, sIndex, true))
		{
			return false;
		}
		if (sPointer.IsTemporary())
		{
			return (OffsetOf(sPointer) <= 253) && !FusedOverlaps(sResult, sPointer, true);
		}
		return sPointer.IsSymbol();
	}
	for (i = 0; i < 2; i++)
	{
		if (!Fused(psInstruction->asArguments[i]) && FusedOverlaps(sResult, psInstruction->asArguments[i], false))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psInstruction is an and with a constant.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816AndsConstant(SIRInstruction* psInstruction)
{
	return (psInstruction->uiOpcode == IRO_And) && (psInstruction->asArguments[0].IsConstant() || psInstruction->asArguments[1].IsConstant());
}


//////////////////////////////////////////////////////////////////////////
//
// Whether instruction uiIndex of pcBlock, a load, and, or, xor or not of
// two or more words, or an add or sub, needn't be written: its result's one
// use is the next instruction, an and, or, xor, not, add or sub, or the
// value of a store, which then reads each word into A as it gets to it
// (WordInA): no store and load of each.  The instructions before it may
// be fused too (a chain read into A a word at a time), with one add or sub
// at most in the chain and the instruction that reads it (the carry), and a
// load in it only when the store at the end is to the same place (else the
// store could change a word still to be loaded).
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::CanFuse(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;
	SIRInstruction*	psNext;
	SIRInstruction*	psChained;
	SIRReference	sResult;
	SIRReference	sNextResult;
	int				iCarries;		//The next instruction's.
	int				iChainCarries;	//The chain's, this one's too.
	int				i;
	bool			bStore;
	bool			bWordCopy;
	size			uiNext;

	if (miFused >= IR816_MAX_FUSED)
	{
		return false;
	}
	psInstruction = pcBlock->masInstructions.Get(uiIndex);

	//The next instruction that writes code (IsFolded ones don't).
	for (uiNext = uiIndex + 1; (uiNext < pcBlock->masInstructions.NumElements()) && IsFolded(pcBlock->masInstructions.Get(uiNext)); uiNext++)
	{
	}
	if (uiNext >= pcBlock->masInstructions.NumElements())
	{
		return CanFuseReturn(pcBlock, psInstruction);
	}
	psNext = pcBlock->masInstructions.Get(uiNext);
	sResult = psInstruction->sResult;

	//A 1 word load only stored by the next instruction is a copy through A (a byte's under the store's sep m), from
	//any address: there is no second word to read after the first is written.
	bWordCopy = (miFused == 0) && (psInstruction->uiOpcode == IRO_Load) && (Words(psInstruction->sType) == 1) && (psNext->uiOpcode == IRO_Store) &&
		psNext->sType.Equals(psInstruction->sType) && psNext->asArguments[0].Equals(sResult) && !psNext->asArguments[1].Equals(sResult);
	if (!sResult.IsTemporary() || ((Words(psInstruction->sType) < 2) && !bWordCopy) || psInstruction->sType.IsFloat() ||
		(*maiUseBlock.Get(sResult.Index()) != miBlock))
	{
		return false;
	}

	iCarries = 0;
	iChainCarries = 0;
	switch (psInstruction->uiOpcode)
	{
	case IRO_Load:
		if (psInstruction->uiFlags & IRF_Volatile)
		{
			return false;
		}
		break;
	case IRO_Add:
	case IRO_PtrAdd:
		if (!FoldedBase(sResult).Equals(sResult))
		{
			return false;
		}
		break;
	case IRO_Sub:
		break;
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
	case IRO_Not:
		break;
	default:
		return false;
	}

	bStore = false;
	sNextResult = IRNone();
	switch (psNext->uiOpcode)
	{
	case IRO_Store:
		if (!psNext->asArguments[0].Equals(sResult) || psNext->asArguments[1].Equals(sResult))
		{
			return false;
		}
		bStore = true;
		break;
	case IRO_Add:
	case IRO_PtrAdd:
	case IRO_Sub:
		if ((psNext->uiOpcode != IRO_Sub) && !FoldedBase(psNext->sResult).Equals(psNext->sResult))
		{
			return false;
		}
		iCarries++;
		//Fall through.
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
	case IRO_Not:
		if ((Words(psNext->sType) != Words(psInstruction->sType)) || psNext->asArguments[0].Equals(psNext->asArguments[1]))
		{
			return false;
		}
		if (!psNext->asArguments[0].Equals(sResult) && !(psNext->asArguments[1].Equals(sResult) && (psNext->uiOpcode != IRO_Sub) && (psNext->uiOpcode != IRO_Not)))
		{
			return false;
		}
		sNextResult = psNext->sResult;
		break;
	case IRO_Mul:
	case IRO_UDiv:
	case IRO_SDiv:
	case IRO_URem:
	case IRO_SRem:
		//Read into the scratch routine's operand a word at a time (RuntimeCall), all before its result is written.
		if (!IsScratchCall(psNext) || (Words(psNext->sType) != Words(psInstruction->sType)) || psNext->asArguments[0].Equals(psNext->asArguments[1]) ||
			(!psNext->asArguments[0].Equals(sResult) && !psNext->asArguments[1].Equals(sResult)))
		{
			return false;
		}
		break;
	default:
		return false;
	}
	if (sNextResult.IsTemporary() && (InX(sNextResult) || (OffsetOf(sNextResult) > 255)))
	{
		return false;
	}

	//The whole chain: the carry, loads before a store, and what each reads.
	for (i = 0; i <= miFused; i++)
	{
		psChained = (i < miFused) ? mapsFused[i] : psInstruction;
		if ((psChained->uiOpcode == IRO_Add) || (psChained->uiOpcode == IRO_PtrAdd) || (psChained->uiOpcode == IRO_Sub))
		{
			iChainCarries++;
		}
		if ((psChained->uiOpcode == IRO_Load) && bStore && !bWordCopy && !psChained->asArguments[0].Equals(psNext->asArguments[1]))
		{
			return false;
		}
		if (!FusedInputsKept(psChained, sNextResult))
		{
			return false;
		}
	}
	if (iChainCarries && (IR816AndsConstant(psInstruction) || IR816AndsConstant(psNext)))
	{
		//An and of a constant word 0 doesn't read the word: an add's carry wouldn't get to the next.
		return false;
	}
	return iChainCarries + iCarries <= 1;
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
// Whether psInstruction, pcBlock's last, is fused into the block's ret of
// its two words (Return reads the high word into A then X first, so the
// chain can't have an add or sub's carry).
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::CanFuseReturn(CIRBlock* pcBlock, SIRInstruction* psInstruction)
{
	SIRInstruction*	psChained;
	int				i;

	if ((pcBlock->msJump.uiKind != IRJ_Return) || !pcBlock->msJump.sArgument.Equals(psInstruction->sResult) || (msFrame.iHiddenResult >= 0) ||
		!psInstruction->sResult.IsTemporary() || (Words(psInstruction->sType) != 2) || psInstruction->sType.IsFloat() ||
		(*maiUseBlock.Get(psInstruction->sResult.Index()) != miBlock))
	{
		return false;
	}
	for (i = 0; i <= miFused; i++)
	{
		psChained = (i < miFused) ? mapsFused[i] : psInstruction;
		switch (psChained->uiOpcode)
		{
		case IRO_Load:
			if (psChained->uiFlags & IRF_Volatile)
			{
				return false;
			}
		case IRO_And:
		case IRO_Or:
		case IRO_Xor:
		case IRO_Not:
			break;
		default:
			return false;
		}
		if (!FusedInputsKept(psChained, IRNone()))
		{
			return false;
		}
	}
	return true;
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
void CIR816Backend::WordInA(SIRInstruction* psInstruction, int iWord)
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
		(Fused(sSecond) || ((Words(psInstruction->sType) == 1) && InX(sSecond) && !InX(sFirst))))
	{
		sFirst = psInstruction->asArguments[1];
		sSecond = psInstruction->asArguments[0];
	}
	switch (psInstruction->uiOpcode)
	{
	case IRO_Load:
		PointerPlace(sFirst, &iPlace, &szSymbol, &iOffset);
		sz.Init(CIR816Backend::GetScratch());
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
			sz.Append(LoadY(sFirst, iOffset + 2 * iWord) ? "]:24 + Y]" : "]:24]");
		}
		Line(sz.Text());
		sz.Kill();
		return;
	case IRO_Add:
	case IRO_PtrAdd:
	case IRO_Sub:
		if (iWord == 0)
		{
			Line((psInstruction->uiOpcode == IRO_Sub) ? "sec" : "clc");
		}
		Operand("lda", sFirst, iWord);
		Operand((psInstruction->uiOpcode == IRO_Sub) ? "sbc" : "adc", sSecond, iWord);
		return;
	case IRO_Not:
		Operand("lda", sFirst, iWord);
		Line("eor 0xFFFF");
		return;
	default:
		//A constant word that leaves the other's (and all ones, or and eor 0) is a copy; and 0 is 0.
		bAnd = psInstruction->uiOpcode == IRO_And;
		szMnemonic = bAnd ? "and" : ((psInstruction->uiOpcode == IRO_Or) ? "ora" : "eor");
		if (ConstantWord(sSecond, iWord, &uiWord))
		{
			if (bAnd && (uiWord == 0))
			{
				Line("lda 0");
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
void CIR816Backend::StoreMemory(SIRInstruction* psInstruction)
{
	int32	iPlace;
	int32	iOffset;
	char*	szSymbol;
	int		i;
	CChars	sz;

	PointerPlace(psInstruction->asArguments[1], &iPlace, &szSymbol, &iOffset);
	sz.Init(CIR816Backend::GetScratch());
	if (psInstruction->sType.Is(IRTK_I8))
	{
		Line("sep m");
	}
	for (i = 0; i < Words(psInstruction->sType); i++)
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
			sz.Append(LoadY(psInstruction->asArguments[1], iOffset + 2 * i) ? "]:24 + Y]" : "]:24]");
		}
		Line(sz.Text());
	}
	if (psInstruction->sType.Is(IRTK_I8))
	{
		Line("rep m");
	}
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes the arguments last first (and the hidden result pointer), calls and removes them.  A call through a pointer
// pushes the return address and the target - 1, and rtl's to it (rtl adds 1 to the low 16 bits only).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Call(CIRBlock* pcBlock, SIRInstruction* psInstruction)
{
	int32					iBytes;
	size					ui;
	SIRInstruction*			psArgument;
	SIRReference			sCallee;
	int32					iReturn;
	int32					iCallee;
	int						iRegisters;
	CChars					sz;
	CArrayIR816Arguments	asArguments;
	SIR816Argument*			psPlaced;

	//The arguments as passed: the hidden result pointer first, each pushed unless a register function takes it in
	//registers.
	sCallee = psInstruction->asArguments[0];
	asArguments.Init(CIR816Backend::GetScratch());
	if (!psInstruction->sResult.IsNone() && IsHiddenResult(psInstruction->sType))
	{
		psPlaced = asArguments.Add();
		psPlaced->iWords = 2;
		psPlaced->iRegister = -1;
	}
	for (ui = 0; ui < mauiPendingArguments.NumElements(); ui++)
	{
		psArgument = pcBlock->masInstructions.Get(*mauiPendingArguments.Get(ui));
		psPlaced = asArguments.Add();
		psPlaced->iWords = Words(psArgument->sType);
		psPlaced->iRegister = -1;
	}
	iRegisters = IsRegisterCall(sCallee) ? IR816AssignRegisters(&asArguments) : 0;

	iBytes = PushArguments(pcBlock, psInstruction, &asArguments);
	if (iRegisters > 0)
	{
		LoadRegisterArguments(pcBlock, psInstruction, &asArguments);
	}
	mauiPendingArguments.FakeSetUsedElements(0);

	sz.Init(CIR816Backend::GetScratch());
	if (sCallee.IsSymbol())
	{
		sz.Append("jsl ");
		sz.Append(mpcModule->GetSymbolName(sCallee.Index()));
		if (iRegisters > 0)
		{
			IR816AppendRegisterSuffix(&sz, &asArguments);
		}
		Line(sz.Text());
	}
	else
	{
		iReturn = NewLabel();
		iCallee = OffsetOf(sCallee);
		Line("phk");
		sz.Append("per L");
		sz.Append(iReturn);
		sz.Append(" - 1");
		Line(sz.Text());
		Line("sep m");
		Frame("lda", iCallee + 2);
		Line("pha");
		Line("rep m");
		Frame("lda", iCallee);
		Line("dec A");
		Line("pha");
		Line("rtl");
		Label(iReturn);
	}
	sz.Kill();
	asArguments.Kill();
	CallerCleanUp(iBytes, psInstruction);
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes the call's arguments that aren't in registers (pasArguments, as
// Call places them), last first, then the hidden result pointer if it
// isn't.  Returns the bytes pushed.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR816Backend::PushArguments(CIRBlock* pcBlock, SIRInstruction* psInstruction, CArrayIR816Arguments* pasArguments)
{
	int32			iBytes;
	size			ui;
	size			uiFirst;
	SIRInstruction*	psArgument;

	iBytes = 0;
	uiFirst = pasArguments->NumElements() - mauiPendingArguments.NumElements();
	for (ui = mauiPendingArguments.NumElements(); ui > 0; ui--)
	{
		if (pasArguments->Get(uiFirst + ui - 1)->iRegister >= 0)
		{
			continue;
		}
		psArgument = pcBlock->masInstructions.Get(*mauiPendingArguments.Get(ui - 1));
		PushValue(psArgument->asArguments[0], psArgument->sType);
		iBytes += ValueBytes(psArgument->sType);
	}
	if ((uiFirst == 1) && (pasArguments->Get(0)->iRegister < 0))
	{
		PushFrameAddress(OffsetOf(psInstruction->sResult));
		iBytes += 4;
	}
	return iBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the call's register argument word for iRegister comes from: its
// value and word, or the hidden result pointer (*piHiddenResult its frame
// offset, else -1).
//
//////////////////////////////////////////////////////////////////////////
static void IR816RegisterSource(CIRBlock* pcBlock, SIRInstruction* psInstruction, CArrayTemplate<size>* pauiPending, CArrayIR816Arguments* pasArguments, int iRegister, int32 iHiddenOffset, SIRReference* psValue, int* piWord, int32* piHiddenResult)
{
	size			ui;
	size			uiFirst;
	SIR816Argument*	psArgument;

	uiFirst = pasArguments->NumElements() - pauiPending->NumElements();
	*psValue = IRNone();
	*piWord = 0;
	*piHiddenResult = -1;
	for (ui = 0; ui < pasArguments->NumElements(); ui++)
	{
		psArgument = pasArguments->Get(ui);
		if ((psArgument->iRegister >= 0) && (iRegister >= psArgument->iRegister) && (iRegister < psArgument->iRegister + psArgument->iWords))
		{
			*piWord = iRegister - psArgument->iRegister;
			if (ui < uiFirst)
			{
				*piHiddenResult = iHiddenOffset;
			}
			else
			{
				*psValue = pcBlock->masInstructions.Get(*pauiPending->Get(ui - uiFirst))->asArguments[0];
			}
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Puts the call's register arguments in A, X and Y (after PushArguments has
// pushed the others): LoadRegisters.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::LoadRegisterArguments(CIRBlock* pcBlock, SIRInstruction* psInstruction, CArrayIR816Arguments* pasArguments)
{
	SIRReference	asValues[IR816_ARGUMENT_REGISTERS];
	int				aiWords[IR816_ARGUMENT_REGISTERS];
	int32			aiHidden[IR816_ARGUMENT_REGISTERS];
	int32			iHiddenOffset;
	int				iRegisters;
	int				i;

	iHiddenOffset = psInstruction->sResult.IsNone() ? -1 : OffsetOf(psInstruction->sResult);
	iRegisters = 0;
	for (i = 0; i < IR816_ARGUMENT_REGISTERS; i++)
	{
		IR816RegisterSource(pcBlock, psInstruction, &mauiPendingArguments, pasArguments, i, iHiddenOffset, &asValues[i], &aiWords[i], &aiHidden[i]);
		if (asValues[i].IsNone() && (aiHidden[i] < 0))
		{
			break;
		}
		iRegisters++;
	}
	LoadRegisters(asValues, aiWords, aiHidden, iRegisters);
}


//////////////////////////////////////////////////////////////////////////
//
// Loads the first iRegisters of A, X and Y with their words: word
// aiWords[i] of asValues[i], or of the hidden result pointer when
// aiHidden[i] isn't -1 (its frame offset).  Straight from where they are
// when each can be (LoadsRegisterDirectly): Y and X first and A last,
// unless A's value is kept in X (then A first).  Otherwise they are pushed
// and pulled.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::LoadRegisters(SIRReference* asValues, int* aiWords, int32* aiHidden, int iRegisters)
{
	int		i;
	bool	bDirect;
	bool	bAFirst;

	bDirect = true;
	for (i = 0; i < iRegisters; i++)
	{
		bDirect &= (aiHidden[i] >= 0) || LoadsRegisterDirectly(asValues[i], aiWords[i]);
	}

	if (bDirect)
	{
		bAFirst = (aiHidden[0] < 0) && (aiWords[0] == 0) && InX(asValues[0]);
		if (bAFirst)
		{
			LoadRegisterWord(0, asValues[0], aiWords[0], aiHidden[0]);
		}
		for (i = iRegisters - 1; i > 0; i--)
		{
			LoadRegisterWord(i, asValues[i], aiWords[i], aiHidden[i]);
		}
		if (!bAFirst)
		{
			LoadRegisterWord(0, asValues[0], aiWords[0], aiHidden[0]);
		}
		return;
	}

	//Pushed Y's first, so A's is pulled first.
	for (i = iRegisters - 1; i >= 0; i--)
	{
		if ((aiHidden[i] >= 0) && (aiWords[i] == 0))
		{
			Line("tdc");
			Line("clc");
			Line("adc ", aiHidden[i]);
			Line("pha");
		}
		else if (aiHidden[i] >= 0)
		{
			Line("pea 0");
		}
		else
		{
			PushWord(asValues[i], aiWords[i]);
		}
	}
	Line("pla");
	if (iRegisters > 1)
	{
		Line("plx");
	}
	if (iRegisters > 2)
	{
		Line("ply");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a register argument's word can be loaded straight into A, X or Y
// (LoadRegisterWord) without A or X: a constant, a symbol's address, a
// value kept in X, or a word in the direct page.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::LoadsRegisterDirectly(SIRReference sValue, int iWord)
{
	if (!sValue.IsTemporary())
	{
		return true;
	}
	if ((iWord == 0) && InX(sValue))
	{
		return true;
	}
	return (iWord >= Words(mpcFunction->GetTemporaryType(sValue))) || (OffsetOf(sValue) + 2 * iWord <= 255);
}


//////////////////////////////////////////////////////////////////////////
//
// Loads a register argument's word into A (iRegister 0), X (1) or Y (2):
// the word of the hidden result pointer when iHiddenResult isn't -1 (its
// frame offset; the high word is 0), else of sValue.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::LoadRegisterWord(int iRegister, SIRReference sValue, int iWord, int32 iHiddenResult)
{
	static const char*	aszLoads[IR816_ARGUMENT_REGISTERS] = { "lda", "ldx", "ldy" };
	static const char*	aszZeros[IR816_ARGUMENT_REGISTERS] = { "lda 0", "ldx 0", "ldy 0" };

	if ((iHiddenResult >= 0) && (iWord == 0))
	{
		//The hidden result pointer is the first argument: its low word is A's.
		Line("tdc");
		Line("clc");
		Line("adc ", iHiddenResult);
	}
	else if (iHiddenResult >= 0)
	{
		Line(aszZeros[iRegister]);
	}
	else
	{
		Operand(aszLoads[iRegister], sValue, iWord);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a call of sCallee passes its arguments in registers: a symbol the
// front end marked (CIRModule::SetRegisterCall).
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::IsRegisterCall(SIRReference sCallee)
{
	return gbIR816RegisterArguments && sCallee.IsSymbol() && mpcModule->IsRegisterCall(sCallee.Index());
}

