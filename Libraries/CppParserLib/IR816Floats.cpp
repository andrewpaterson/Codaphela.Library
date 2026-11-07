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
#include "IR816BackendInternal.h"


//The W65C816 back end's floating point instructions: arithmetic, compares and conversions, as calls of the soft
//float runtime (pushed operands, or the frame operand entries of softfloat816.asm).


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
				SignWordInA();
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
		SignWordInA();
	}
	else
	{
		Line("lda 0");
	}
	Line("pha");
	Frame("lda", IR816_SCRATCH_VALUE);
	Line("pha");
}
