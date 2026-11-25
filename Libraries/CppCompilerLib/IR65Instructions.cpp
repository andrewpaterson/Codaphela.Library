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
#include "BaseLib/StringHelper.h"
#include "IR65BackendInternal.h"


//The 65C64 back end's code for each IR instruction (Instruction): integer arithmetic and logic, conversions, loads
//and stores, and which operands an instruction takes in A and B.  Operands, multiplies and divides, floating point,
//fused instructions, calls and compares are in IR65Operands, IR65Multiplies, IR65Floats, IR65Fusion, IR65Calls and
//IR65Compares.


//////////////////////////////////////////////////////////////////////////
//
// What the lowering of an opcode needs and changes (IR65F_ flags).
//
//////////////////////////////////////////////////////////////////////////
uint8 IR65OpcodeFlags(uint16 uiOpcode)
{
	if (IsIRIntegerComparison(uiOpcode))
	{
		return IR65F_TAKES_A | IR65F_TAKES_B | (IsIRCommutative(uiOpcode) ? IR65F_COMMUTES : 0);
	}
	switch (uiOpcode)
	{
	case IRO_Add:
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
	case IRO_PtrAdd:
		return IR65F_TAKES_A | IR65F_TAKES_B | IR65F_COMMUTES;
	case IRO_Mul:
		return IR65F_TAKES_A | IR65F_TAKES_B | IR65F_COMMUTES | IR65F_CHANGES_Y | IR65F_NO_DEFER;
	case IRO_Sub:
	case IRO_Shl:
	case IRO_LShr:
		return IR65F_TAKES_A | IR65F_TAKES_B;
	case IRO_AShr:
		return IR65F_TAKES_A | IR65F_TAKES_B | IR65F_NO_DEFER;
	case IRO_SDiv:
	case IRO_SRem:
		return IR65F_TAKES_A | IR65F_TAKES_B | IR65F_CHANGES_XY | IR65F_NO_DEFER;
	case IRO_UDiv:
	case IRO_URem:
		return IR65F_TAKES_A | IR65F_TAKES_B | IR65F_CHANGES_Y | IR65F_NO_DEFER;
	case IRO_Neg:
	case IRO_Not:
	case IRO_Copy:
	case IRO_SExt:
	case IRO_ZExt:
	case IRO_Trunc:
	case IRO_PtrToI:
	case IRO_IToPtr:
	case IRO_SpaceCast:
	case IRO_Store:
		return IR65F_TAKES_A;
	case IRO_Call:
		return IR65F_CALLS;
	case IRO_FAdd:
	case IRO_FSub:
	case IRO_FMul:
	case IRO_FDiv:
	case IRO_SIToF:
	case IRO_UIToF:
	case IRO_FToSI:
	case IRO_FToUI:
	case IRO_FExt:
	case IRO_FTrunc:
	case IRO_FEq:
	case IRO_FNe:
	case IRO_FLt:
	case IRO_FLe:
	case IRO_FGt:
	case IRO_FGe:
	case IRO_FOrdered:
	case IRO_FUnordered:
		return IR65F_CHANGES_XY;
	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The mnemonic an integer opcode is written with; IR65M_Unknown if none is.  not is an
// xor (with -1), and ptradd an add.
//
//////////////////////////////////////////////////////////////////////////
uint8 IR65IntegerMnemonic(uint16 uiOpcode)
{
	switch (uiOpcode)
	{
	case IRO_Add:
	case IRO_PtrAdd:
		return IR65M_Add;
	case IRO_Sub:
		return IR65M_Sub;
	case IRO_And:
		return IR65M_And;
	case IRO_Or:
		return IR65M_Or;
	case IRO_Xor:
	case IRO_Not:
		return IR65M_Xor;
	case IRO_Shl:
		return IR65M_Shl;
	case IRO_LShr:
		return IR65M_Shr;
	case IRO_Neg:
		return IR65M_Neg;
	default:
		return IR65M_Unknown;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Selector::Init(CIR65Backend* pcBackend)
{
	mpcBackend = pcBackend;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Selector::Kill(void)
{
	mpcBackend = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::Instruction(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;
	int32			iTrue;
	int32			iEnd;
	const char*		szWork;

	psInstruction = pcBlock->masInstructions.Get(uiIndex);
	switch (psInstruction->uiOpcode)
	{
	case IRO_Add:
	case IRO_Sub:
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
	case IRO_Shl:
	case IRO_LShr:
		return Integer(psInstruction);

	case IRO_Neg:
	case IRO_Not:
	case IRO_Copy:
	case IRO_PtrAdd:
		return Integer(psInstruction);

	case IRO_Mul:
		return MultiplyInstruction(psInstruction);

	case IRO_AShr:
		return ShiftSigned(psInstruction);

	case IRO_SDiv:
	case IRO_UDiv:
	case IRO_SRem:
	case IRO_URem:
		//A partner not written (its result isn't used) is still given its result (CanFuse pairs the others).
		return Divide(psInstruction, PairedDivide(pcBlock, uiIndex));

	case IRO_SExt:
	case IRO_ZExt:
	case IRO_Trunc:
	case IRO_PtrToI:
	case IRO_IToPtr:
	case IRO_SpaceCast:
		return Conversion(psInstruction);

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
	case IRO_FEq:
	case IRO_FNe:
	case IRO_FLt:
	case IRO_FLe:
	case IRO_FGt:
	case IRO_FGe:
	case IRO_FOrdered:
	case IRO_FUnordered:
		//The i1: 1 when the comparison is true.
		iTrue = mpcBackend->NewLabel();
		iEnd = mpcBackend->NewLabel();
		if (!Compare(psInstruction, true, iTrue))
		{
			return false;
		}
		szWork = WorkRegister(psInstruction);
		if (StringCompare(szWork, "A") == 0)
		{
			mpcBackend->msAvailable = IRNone();
		}
		mpcBackend->Line(IR65M_Cpy)->Append(szWork)->Append(", 0");
		mpcBackend->Branch(IR65M_Bra, NULL, iEnd);
		mpcBackend->Label(iTrue);
		mpcBackend->Line(IR65M_Cpy)->Append(szWork)->Append(", 1");
		mpcBackend->Label(iEnd);
		StoreResult(psInstruction->sResult, szWork);
		return true;

	case IRO_FAdd:
	case IRO_FSub:
	case IRO_FMul:
	case IRO_FDiv:
	case IRO_FNeg:
		return FloatOperation(psInstruction);

	case IRO_SIToF:
	case IRO_UIToF:
	case IRO_FToSI:
	case IRO_FToUI:
	case IRO_FExt:
	case IRO_FTrunc:
		return FloatConversion(psInstruction);

	case IRO_Slot:
	case IRO_ArgAddress:
		return true;

	case IRO_Load:
		return Load(pcBlock, uiIndex, NULL);
	case IRO_Store:
		return Store(pcBlock, uiIndex);
	case IRO_Arg:
		return true;
	case IRO_Call:
		return mpcBackend->mcCalls.Call(pcBlock, uiIndex);

	case IRO_Thrown:
		//The call before it branched on the carry (CIR65Calls::Call).
		return true;

	default:
		return mpcBackend->Error("Opcode not supported by the 65C64 back end.", GetIROpcode(psInstruction->uiOpcode) ? GetIROpcode(psInstruction->uiOpcode)->szName : NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::Commutes(uint16 uiOpcode)
{
	return (IR65OpcodeFlags(uiOpcode) & IR65F_COMMUTES) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A deferred load, an operation on it (psInstruction, fused into the
// store) and a store of the result back to the same place (psStore) is one
// instruction on memory: add [4 + X], 1 (the operand a register or -8 to 8,
// 1 to 8 for a shift).  The store writes it (WriteInstruction).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::ReadModifyWrite(SIRInstruction* psInstruction, SIRInstruction* psStore)
{
	SIRReference	sMemory;
	SIRReference	sOther;
	CChars			szAddress;
	CChars			szOperand;
	char			cRegister;
	bool			bShift;
	bool			bResult;

	switch (psInstruction->uiOpcode)
	{
	case IRO_Add:
	case IRO_Sub:
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
	case IRO_Shl:
	case IRO_LShr:
		break;
	default:
		return false;
	}
	bShift = (psInstruction->uiOpcode == IRO_Shl) || (psInstruction->uiOpcode == IRO_LShr);
	if (mpcBackend->IsNarrow(psInstruction->sType) || mpcBackend->IsWide(psInstruction->sType) || !psInstruction->sResult.IsTemporary() || !mpcBackend->mcPlaces.PlaceOf(psInstruction->sResult)->bInA)
	{
		return false;
	}
	if (mpcBackend->mcPlaces.IsDeferred(psInstruction->asArguments[0]))
	{
		sMemory = psInstruction->asArguments[0];
		sOther = psInstruction->asArguments[1];
	}
	else if (mpcBackend->mcPlaces.IsDeferred(psInstruction->asArguments[1]) && Commutes(psInstruction->uiOpcode))
	{
		sMemory = psInstruction->asArguments[1];
		sOther = psInstruction->asArguments[0];
	}
	else
	{
		return false;
	}

	if ((psStore->uiOpcode != IRO_Store) || !psStore->asArguments[0].Equals(psInstruction->sResult) || (psStore->uiFlags & IRF_Volatile) || !psStore->sType.Equals(psInstruction->sType))
	{
		return false;
	}

	szOperand.Init();
	if (mpcBackend->mcPlaces.IsInA(sOther))
	{
		szOperand.Append('A');
	}
	else if (mpcBackend->mcPlaces.InRegister(sOther) && !bShift)
	{
		//A shift of memory takes its amount from A or B.
		szOperand.Append(mpcBackend->mcPlaces.RegisterName(mpcBackend->mcPlaces.PlaceOf(sOther)));
	}
	else if (SmallImmediate(sOther, bShift ? 1 : -8, 8))
	{
		AppendImmediate(&szOperand, sOther, IR65E_Zero);
	}
	szAddress.Init();
	cRegister = '\0';
	bResult = !szOperand.Empty() && PureMemoryOperand(psStore->asArguments[1], &szAddress, &cRegister) && szAddress.Equals(mpcBackend->mszDeferred.Text());
	if (bResult && ((psInstruction->uiOpcode == IRO_Add) || (psInstruction->uiOpcode == IRO_Sub) || bShift) && mpcBackend->mcPlaces.IsFrameOperand(&szAddress))
	{
		//add, sub, shl and shr can't address memory with S.
		bResult = false;
	}
	if (bResult)
	{
		mpcBackend->Line(IR65IntegerMnemonic(psInstruction->uiOpcode))->Append(szAddress)->Append(", ")->Append(szOperand);
		mpcBackend->msInA = mpcBackend->msAvailable;
	}
	szAddress.Kill();
	szOperand.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// add, sub, and, or, xor, shl and lshr (and neg, not and copy), 32 bits or
// narrower.  The result is worked out where it is kept when its first
// operand is kept there too (add X, 1; add [8 + W], A), in its register
// (cpy X, [4 + W] / add X, B), or in A.  Narrow operands are held zero
// extended, which these don't mind; the result is truncated.  64 bit values
// work with A and B 64 bits wide.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::Integer(SIRInstruction* psInstruction)
{
	uint16			uiOpcode;
	uint8			uiOperation;
	const char*		szWork;
	bool			bArithmetic;
	bool			bShift;
	SIRReference	sLeft;
	SIRReference	sRight;
	SIRReference	sResult;
	SIR65Place*		psResult;
	CChars			szOperand;
	CChars*			psz;

	if (mpcBackend->IsWide(psInstruction->sType))
	{
		return IntegerWide(psInstruction);
	}
	uiOpcode = psInstruction->uiOpcode;
	sLeft = psInstruction->asArguments[0];
	sRight = psInstruction->asArguments[1];
	sResult = psInstruction->sResult;
	bShift = (uiOpcode == IRO_Shl) || (uiOpcode == IRO_LShr);
	bArithmetic = (uiOpcode == IRO_Add) || (uiOpcode == IRO_Sub) || (uiOpcode == IRO_PtrAdd) || bShift;
	uiOperation = IR65IntegerMnemonic(uiOpcode);

	//Where the first operand is kept already (or the second, if they swap).
	if (Commutes(uiOpcode) && mpcBackend->mcPlaces.SamePlace(sRight, sResult) && !mpcBackend->mcPlaces.SamePlace(sLeft, sResult))
	{
		sLeft = psInstruction->asArguments[1];
		sRight = psInstruction->asArguments[0];
	}
	if (sResult.IsTemporary() && !mpcBackend->IsNarrow(psInstruction->sType) && mpcBackend->mcPlaces.SamePlace(sLeft, sResult) && !mpcBackend->mcPlaces.SamePlace(sRight, sResult))
	{
		psResult = mpcBackend->mcPlaces.PlaceOf(sResult);
		szOperand.Init();
		if (uiOpcode == IRO_Copy)
		{
			szOperand.Kill();
			return true;
		}
		if (psResult->uiKind == IR65P_Register)
		{
			if (uiOpcode == IRO_Neg)
			{
				mpcBackend->Line(IR65M_Neg)->Append(mpcBackend->mcPlaces.RegisterName(psResult))->Append(", ")->Append(mpcBackend->mcPlaces.RegisterName(psResult));
				szOperand.Kill();
				return true;
			}
			if (uiOpcode == IRO_Not)
			{
				szOperand.Append("-1");
			}
			if ((uiOpcode == IRO_Not) || Operand(sRight, &szOperand, bArithmetic, IR65E_Zero))
			{
				if (bShift && (szOperand.Text()[0] == '['))
				{
					//Only A and B shift by an amount in memory.
					LoadValue(sRight, "B", IR65E_Zero);
					szOperand.Clear();
					szOperand.Append('B');
				}
				mpcBackend->Line(uiOperation)->Append(mpcBackend->mcPlaces.RegisterName(psResult))->Append(", ")->Append(szOperand);
				szOperand.Kill();
				return true;
			}
		}
		else if ((psResult->uiKind == IR65P_Pseudo) && (uiOpcode != IRO_Neg))
		{
			if (uiOpcode == IRO_Not)
			{
				szOperand.Append("-1");
			}
			else if (mpcBackend->mcPlaces.IsInA(sRight))
			{
				szOperand.Append('A');
			}
			else if (mpcBackend->mcPlaces.InRegister(sRight) && !bShift)
			{
				//A shift of memory takes its amount from A or B.
				szOperand.Append(mpcBackend->mcPlaces.RegisterName(mpcBackend->mcPlaces.PlaceOf(sRight)));
			}
			else if (SmallImmediate(sRight, bShift ? 1 : -8, 8))
			{
				AppendImmediate(&szOperand, sRight, IR65E_Zero);
			}
			if (!szOperand.Empty())
			{
				psz = mpcBackend->Line(uiOperation);
				AppendPlace(psz, psResult);
				psz->Append(", ")->Append(szOperand);
				szOperand.Kill();
				return true;
			}
		}
		szOperand.Kill();
	}

	szWork = WorkRegister(psInstruction);
	if ((StringCompare(szWork, "A") != 0) && (mpcBackend->mcPlaces.IsInA(sLeft) || mpcBackend->mcPlaces.SamePlace(sRight, sResult)))
	{
		szWork = "A";
	}

	szOperand.Init();
	if ((StringCompare(szWork, "A") == 0) && mpcBackend->mcPlaces.IsInA(sRight) && !mpcBackend->mcPlaces.IsInA(sLeft))
	{
		//The second operand is in A.
		if (Commutes(uiOpcode))
		{
			if (!Operand(sLeft, &szOperand, bArithmetic, IR65E_Zero))
			{
				LoadValue(sLeft, "B", IR65E_Zero);
				szOperand.Append('B');
			}
		}
		else if ((uiOpcode == IRO_Sub) && Operand(sLeft, &szOperand, true, IR65E_Zero))
		{
			//x - A is -A + x.
			mpcBackend->Line(IR65M_Neg, "A, A");
			uiOperation = IR65M_Add;
		}
		else
		{
			szOperand.Clear();
			mpcBackend->Line(IR65M_Cpy, "B, A");
			LoadValue(sLeft, "A", IR65E_Zero);
			szOperand.Append('B');
		}
	}
	else
	{
		LoadValue(sLeft, szWork, IR65E_Zero);
		if (!sRight.IsNone() && !Operand(sRight, &szOperand, bArithmetic, IR65E_Zero))
		{
			LoadValue(sRight, "B", IR65E_Zero);
			szOperand.Append('B');
		}
		else if (bShift && (szWork[0] != 'A') && (szOperand.Text()[0] == '['))
		{
			//Only A and B shift by an amount in memory.
			LoadValue(sRight, "B", IR65E_Zero);
			szOperand.Clear();
			szOperand.Append('B');
		}
	}

	if (uiOpcode == IRO_Neg)
	{
		mpcBackend->Line(IR65M_Neg)->Append(szWork)->Append(", ")->Append(szWork);
	}
	else if (uiOpcode == IRO_Not)
	{
		mpcBackend->Line(IR65M_Xor)->Append(szWork)->Append(", -1");
	}
	else if (uiOperation != IR65M_Unknown)
	{
		mpcBackend->Line(uiOperation)->Append(szWork)->Append(", ")->Append(szOperand);
	}
	szOperand.Kill();
	if (StringCompare(szWork, "A") == 0)
	{
		mpcBackend->msAvailable = IRNone();
	}
	StoreResult(sResult, szWork);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// 64 bit add, sub, and, or, xor, neg, not, copy, shl and lshr: A = A op B
// with both 64 bits wide.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::IntegerWide(SIRInstruction* psInstruction)
{
	uint8	uiMnemonic;
	CChars*	psz;

	LoadOperands(psInstruction, IR65E_Zero, true);
	mpcBackend->Line(IR65M_Wtth, "(A, B), 64");
	if (psInstruction->uiOpcode == IRO_Neg)
	{
		mpcBackend->Line(IR65M_Neg, "A, A");
	}
	else if (psInstruction->uiOpcode == IRO_Not)
	{
		mpcBackend->Line(IR65M_Xor, "A, -1");
	}
	else
	{
		uiMnemonic = IR65IntegerMnemonic(psInstruction->uiOpcode);
		if (uiMnemonic != IR65M_Unknown)
		{
			psz = mpcBackend->Line(uiMnemonic, "A, ");
			if (mpcBackend->mbRightImmediate)
			{
				AppendImmediate(psz, psInstruction->asArguments[1], IR65E_Zero);
			}
			else
			{
				psz->Append('B');
			}
		}
	}
	mpcBackend->Line(IR65M_Wtth, "(A, B), 32");
	mpcBackend->msAvailable = IRNone();
	StoreResult(psInstruction->sResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// ashr: shr is logical, so a negative value is inverted, shifted and
// inverted back.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::ShiftSigned(SIRInstruction* psInstruction)
{
	bool	bWide;
	int32	iPositive;
	int32	iEnd;
	CChars	szAmount;

	bWide = mpcBackend->IsWide(psInstruction->sType);
	LoadOperands(psInstruction, IR65E_Sign, true);
	szAmount.Init();
	if (mpcBackend->mbRightImmediate)
	{
		AppendImmediate(&szAmount, psInstruction->asArguments[1], IR65E_Zero);
	}
	else
	{
		szAmount.Append('B');
	}
	if (bWide)
	{
		mpcBackend->Line(IR65M_Wtth, "(A, B), 64");
	}
	iPositive = mpcBackend->NewLabel();
	iEnd = mpcBackend->NewLabel();
	mpcBackend->Line(IR65M_Cmp, "A, 0");
	mpcBackend->Branch(IR65M_Brac, "n, ", iPositive);
	mpcBackend->Line(IR65M_Xor, "A, -1");
	mpcBackend->Line(IR65M_Shr, "A, ")->Append(szAmount);
	mpcBackend->Line(IR65M_Xor, "A, -1");
	mpcBackend->Branch(IR65M_Bra, NULL, iEnd);
	mpcBackend->Label(iPositive);
	mpcBackend->Line(IR65M_Shr, "A, ")->Append(szAmount);
	mpcBackend->Label(iEnd);
	szAmount.Kill();
	if (bWide)
	{
		mpcBackend->Line(IR65M_Wtth, "(A, B), 32");
	}
	mpcBackend->msAvailable = IRNone();
	StoreResult(psInstruction->sResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// sext, zext, trunc and the pointer conversions.  Narrow values are kept zero
// extended, so a conversion is a load (extended), a widening to 64 bits if
// the result is wide, and a store (truncated).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::Conversion(SIRInstruction* psInstruction)
{
	SIRType		sFrom;
	bool		bSigned;
	const char*	szWork;

	sFrom = mpcBackend->mpcFunction->TypeOf(psInstruction->asArguments[0]);
	bSigned = psInstruction->uiOpcode == IRO_SExt;
	szWork = WorkRegister(psInstruction);
	if (mpcBackend->ValueBytes(sFrom) != 4)
	{
		szWork = WidthRegister(szWork);
	}
	LoadValue(psInstruction->asArguments[0], szWork, bSigned ? IR65E_Sign : IR65E_Zero);
	if (mpcBackend->IsWide(psInstruction->sType) && !mpcBackend->IsWide(sFrom))
	{
		mpcBackend->Line(IR65M_Wthx, bSigned ? "A, 64:s" : "A, 64");
		mpcBackend->Line(IR65M_Wth, "A, 32");
	}
	StoreResult(psInstruction->sResult, szWork);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Into the result's register (or A): narrow values read at their width
// and widened, 8 byte values with A 64 bits wide.  The load psExtend, the
// sext just after it (ExtendOf), writes is sign extended as it is widened,
// giving the sext's result.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::Load(CIRBlock* pcBlock, size uiIndex, SIRInstruction* psExtend)
{
	SIRInstruction*	psInstruction;
	CChars			szOperand;
	int				iBytes;
	const char*		szWork;
	bool			bCopyExtend;

	psInstruction = pcBlock->masInstructions.Get(uiIndex);
	iBytes = mpcBackend->ValueBytes(psInstruction->sType);
	szWork = WorkRegister(psExtend ? psExtend : psInstruction);
	if (iBytes != 4)
	{
		szWork = WidthRegister(szWork);
	}
	szOperand.Init();
	bCopyExtend = false;
	if ((iBytes < 4) && !mpcBackend->mcPlaces.NarrowOnly(psInstruction->sResult))
	{
		//cpyx extends memory of the value's width into the 32 bit register.  A load only compared at its own width
		//(NarrowOnly) isn't: CIR65Widths keeps the register narrow for it, often across a whole loop.
		bCopyExtend = CopyExtendOperand(psInstruction->asArguments[0], &szOperand, szWork, pcBlock, uiIndex, szWork);
	}
	else
	{
		//Read with cpy, whatever the width.
		mpcBackend->mbCopyAddress = true;
		MemoryOperand(psInstruction->asArguments[0], &szOperand, (iBytes == 4) ? NULL : szWork);
		mpcBackend->mbCopyAddress = false;
	}
	if (StringCompare(szWork, "A") == 0)
	{
		mpcBackend->msAvailable = IRNone();
	}
	if (iBytes == 8)
	{
		mpcBackend->Line(IR65M_Wth, "A, 64");
		mpcBackend->Line(IR65M_Cpy, "A, ")->Append(szOperand);
		mpcBackend->Line(IR65M_Wth, "A, 32");
	}
	else if (iBytes == 4)
	{
		mpcBackend->Line(IR65M_Cpy)->Append(szWork)->Append(", ")->Append(szOperand);
	}
	else if (bCopyExtend)
	{
		mpcBackend->Line(IR65M_Cpyx)->Append(szWork)->Append(", ")->Append(szOperand)->Append(psExtend ? ":s" : ":")->Append(8 * iBytes);
	}
	else
	{
		mpcBackend->Line(IR65M_Wth)->Append(szWork)->Append(", ")->Append(8 * iBytes);
		mpcBackend->Line(IR65M_Cpy)->Append(szWork)->Append(", ")->Append(szOperand);
		mpcBackend->Line(IR65M_Wthx)->Append(szWork)->Append(psExtend ? ", 32:s" : ", 32");
	}
	szOperand.Kill();
	if (psExtend)
	{
		StoreResult(psExtend->sResult, szWork);
		return true;
	}
	StoreResult(psInstruction->sResult, szWork, true);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// From the value's register (or A, or a small constant straight to a 4
// byte place): narrow values written at their width, 8 byte values with A
// 64 bits wide.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::Store(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;
	CChars			szOperand;
	CChars			szValue;
	int				iBytes;
	const char*		szValueRegister;
	SIRReference	sValue;
	bool			bLoad;
	bool			bCopyExtend;

	psInstruction = pcBlock->masInstructions.Get(uiIndex);
	sValue = psInstruction->asArguments[0];
	iBytes = mpcBackend->ValueBytes(psInstruction->sType);
	szOperand.Init();
	szValue.Init();
	szValueRegister = NULL;
	bLoad = false;
	if ((iBytes == 4) && SmallImmediate(sValue, -8, 8))
	{
		AppendImmediate(&szValue, sValue, IR65E_Zero);
	}
	else if ((iBytes != 8) && mpcBackend->mcPlaces.IsInA(sValue))
	{
		szValueRegister = "A";
	}
	else if ((iBytes != 8) && mpcBackend->mcPlaces.InRegister(sValue) && ((iBytes == 4) || mpcBackend->mcPlaces.HasWidth(mpcBackend->mcPlaces.PlaceOf(sValue))))
	{
		szValueRegister = mpcBackend->mcPlaces.RegisterName(mpcBackend->mcPlaces.PlaceOf(sValue));
	}
	else
	{
		szValueRegister = "A";
		bLoad = true;
	}

	//The address first, so it doesn't use A when the value is loaded into it afterwards.
	bCopyExtend = false;
	if ((iBytes < 4) && szValueRegister)
	{
		bCopyExtend = CopyExtendOperand(psInstruction->asArguments[1], &szOperand, szValueRegister, pcBlock, uiIndex, NULL);
	}
	else
	{
		//Written with cpy, whatever the width.
		mpcBackend->mbCopyAddress = true;
		MemoryOperand(psInstruction->asArguments[1], &szOperand, (bLoad || (iBytes != 4)) ? szValueRegister : NULL);
		mpcBackend->mbCopyAddress = false;
	}
	if (bLoad)
	{
		LoadValue(sValue, "A", IR65E_Zero);
	}

	if (szValueRegister == NULL)
	{
		mpcBackend->Line(IR65M_Cpy)->Append(szOperand)->Append(", ")->Append(szValue);
	}
	else if (iBytes == 4)
	{
		mpcBackend->Line(IR65M_Cpy)->Append(szOperand)->Append(", ")->Append(szValueRegister);
	}
	else if (iBytes == 8)
	{
		mpcBackend->Line(IR65M_Wth, "A, 64");
		mpcBackend->Line(IR65M_Cpy)->Append(szOperand)->Append(", A");
		mpcBackend->Line(IR65M_Wth, "A, 32");
	}
	else if (bCopyExtend)
	{
		//cpyx stores the register's low bits.
		mpcBackend->Line(IR65M_Cpyx)->Append(szOperand)->Append(':')->Append(8 * iBytes)->Append(", ")->Append(szValueRegister);
	}
	else
	{
		mpcBackend->Line(IR65M_Wth)->Append(szValueRegister)->Append(", ")->Append(8 * iBytes);
		mpcBackend->Line(IR65M_Cpy)->Append(szOperand)->Append(", ")->Append(szValueRegister);
		mpcBackend->Line(IR65M_Wth)->Append(szValueRegister)->Append(", 32");
	}
	szValue.Kill();
	szOperand.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if psNext's lowering takes sValue (its first operand, and not its
// second) from A when the instruction before left it there, so a result
// it alone uses needn't be kept.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::TakesInA(SIRInstruction* psNext, SIRReference sValue)
{
	return psNext->asArguments[0].Equals(sValue) && !psNext->asArguments[1].Equals(sValue) && (IR65OpcodeFlags(psNext->uiOpcode) & IR65F_TAKES_A);
}


//////////////////////////////////////////////////////////////////////////
//
// The same for what the block's jump does first: a fused comparison, a
// brif's condition or a return's value.  Not when the jump copies phi
// values first (a jmp, or a brif's then edge after the test is fine).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::JumpTakesInA(CIRBlock* pcBlock, SIRInstruction* psFused, SIRReference sValue)
{
	if (psFused)
	{
		return TakesInA(psFused, sValue);
	}
	if ((pcBlock->msJump.uiKind == IRJ_Branch) || (pcBlock->msJump.uiKind == IRJ_Return))
	{
		return pcBlock->msJump.sArgument.Equals(sValue);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue is a constant "op A, value" can take: an integer of 32
// bits or fewer.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::Immediate(SIRReference sValue, SIRType sType)
{
	return sValue.IsConstant() && !mpcBackend->IsWide(sType) && !sType.IsFloat();
}


//////////////////////////////////////////////////////////////////////////
//
// A two operand instruction's operands: the first in A, the second in B, or
// left as a constant when bImmediate and it is one (mbRightImmediate).  A
// second operand the instruction before left in A is copied to B first.
// For 64 bit values, signed narrow comparisons, mul and the divisions.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Selector::LoadOperands(SIRInstruction* psInstruction, EIR65Extend eExtend, bool bImmediate)
{
	SIRReference	sLeft;
	SIRReference	sRight;
	SIRType			sType;

	sLeft = psInstruction->asArguments[0];
	sRight = psInstruction->asArguments[1];
	mpcBackend->mbRightImmediate = false;
	if (sRight.IsNone())
	{
		LoadValue(sLeft, "A", eExtend);
		return;
	}
	sType = mpcBackend->mpcFunction->TypeOf(sRight);
	if (bImmediate && Immediate(sRight, sType))
	{
		LoadValue(sLeft, "A", eExtend);
		mpcBackend->mbRightImmediate = true;
		return;
	}
	if (mpcBackend->mcPlaces.IsInA(sRight) && !sLeft.Equals(sRight))
	{
		if (mpcBackend->IsWide(sType))
		{
			mpcBackend->Line(IR65M_Wtth, "(A, B), 64");
			mpcBackend->Line(IR65M_Cpy, "B, A");
			mpcBackend->Line(IR65M_Wtth, "(A, B), 32");
		}
		else
		{
			mpcBackend->Line(IR65M_Cpy, "B, A");
		}
		mpcBackend->msAvailable = IRNone();
		Extend("B", sType, eExtend);
		LoadValue(sLeft, "A", eExtend);
		return;
	}
	LoadValue(sLeft, "A", eExtend);
	LoadValue(sRight, "B", eExtend);
}


//////////////////////////////////////////////////////////////////////////
//
// True if psNext takes sValue as its second operand (and not its first)
// from A when the instruction before left it there.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::TakesInB(SIRInstruction* psNext, SIRReference sValue)
{
	return psNext->asArguments[1].Equals(sValue) && !psNext->asArguments[0].Equals(sValue) && (IR65OpcodeFlags(psNext->uiOpcode) & IR65F_TAKES_B);
}
