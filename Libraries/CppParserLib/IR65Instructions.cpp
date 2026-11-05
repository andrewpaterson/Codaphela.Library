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
#include "BaseLib/StringHelper.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/IntegerHelper.h"
#include "IR65BackendInternal.h"


//The 65C64 back end's code for each IR instruction, and the operands, loads and stores it is written with.


//////////////////////////////////////////////////////////////////////////
//
// What the lowering of an opcode needs and changes (IR65F_ flags).
//
//////////////////////////////////////////////////////////////////////////
uint8 IR65OpcodeFlags(uint16 uiOpcode)
{
	if (IsIRIntegerComparison(uiOpcode))
	{
		return IR65F_TAKES_A | IR65F_TAKES_B | (((uiOpcode == IRO_Eq) || (uiOpcode == IRO_Ne)) ? IR65F_COMMUTES : 0);
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
// A home as an operand: X, [n + W] or [n + V] ([n + S] in a function
// without a frame).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::AppendPlace(CChars* psz, SIR65Place* psPlace)
{
	switch (psPlace->uiKind)
	{
	case IR65P_Register:
		psz->Append(RegisterName(psPlace));
		break;
	case IR65P_Pseudo:
		psz->Append('[')->Append(psPlace->iOffset)->Append(" + W]");
		break;
	case IR65P_Frame:
	case IR65P_FrameAddress:
		psz->Append('[')->Append(1 + psPlace->iOffset)->Append(" + ")->Append(msFrame.cRegister)->Append(']');
		break;
	case IR65P_Argument:
	case IR65P_ArgumentAddress:
		psz->Append('[')->Append(msFrame.iArguments + psPlace->iOffset)->Append(" + ")->Append(msFrame.cRegister)->Append(']');
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A constant as an immediate, extended to 32 bits as eExtend says.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::AppendImmediate(CChars* psz, SIRReference sConstant, EIR65Extend eExtend)
{
	SIRConstant*	psConstant;
	int64			lliValue;

	psConstant = mpcFunction->GetConstant(sConstant);
	lliValue = (eExtend == IR65E_Sign) ? IRSignExtend(psConstant->sType, psConstant->ulliBits) : (int64)psConstant->ulliBits;
	psz->Append((int64)(int32)lliValue);
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue can be the second operand of an instruction as it is:
// a 32 bit (or narrower) constant, a symbol's address, A if the
// instruction before left it there, its register or pseudo register, or
// its frame place unless the instruction is bArithmetic and the function is
// frameless (add, sub, cmp, shl and shr can't address memory with S).  Appends
// the operand.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Operand(SIRReference sValue, CChars* pszOperand, bool bArithmetic, EIR65Extend eExtend)
{
	SIRType		sType;
	SIR65Place*	psPlace;

	if (IsDeferred(sValue))
	{
		//add, sub, cmp, shl and shr can't address memory with S.
		if (bArithmetic && IsFrameOperand(&mszDeferred))
		{
			return false;
		}
		pszOperand->Append(mszDeferred);
		return true;
	}
	sType = mpcFunction->TypeOf(sValue);
	if (IsWide(sType))
	{
		return false;
	}
	if (sValue.IsConstant())
	{
		AppendImmediate(pszOperand, sValue, eExtend);
		return true;
	}
	if (sValue.IsSymbol())
	{
		pszOperand->Append(mpcModule->GetSymbolName(sValue.Index()));
		return true;
	}
	if (!sValue.IsTemporary() || ((eExtend == IR65E_Sign) && IsNarrow(sType)))
	{
		return false;
	}
	if (IsInA(sValue))
	{
		pszOperand->Append('A');
		return true;
	}
	psPlace = PlaceOf(sValue);
	switch (psPlace->uiKind)
	{
	case IR65P_Register:
	case IR65P_Pseudo:
		AppendPlace(pszOperand, psPlace);
		return true;
	case IR65P_Frame:
		if (bArithmetic && (msFrame.cRegister == 'S'))
		{
			return false;
		}
		AppendPlace(pszOperand, psPlace);
		return true;
	case IR65P_Argument:
		if ((bArithmetic && (msFrame.cRegister == 'S')) || (psPlace->iBytes != 4))
		{
			return false;
		}
		AppendPlace(pszOperand, psPlace);
		return true;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue is a constant from iMin to iMax (as held: narrow values
// zero extended), which an instruction can use with memory.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::SmallImmediate(SIRReference sValue, int iMin, int iMax)
{
	SIRConstant*	psConstant;
	int64			lliValue;

	if (!sValue.IsConstant())
	{
		return false;
	}
	psConstant = mpcFunction->GetConstant(sValue);
	if (IsWide(psConstant->sType))
	{
		return false;
	}
	lliValue = IsNarrow(psConstant->sType) ? (int64)psConstant->ulliBits : (int64)(int32)psConstant->ulliBits;
	return (lliValue >= iMin) && (lliValue <= iMax);
}


//////////////////////////////////////////////////////////////////////////
//
// True if no temporary kept in X or Y (iRegister) is live during the block's
// instruction uiIndex, so it can hold something else for that instruction.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::IsRegisterFree(int iRegister, CIRBlock* pcBlock, size uiIndex)
{
	SIR65Home*	psHome;
	SIRRange*	psRange;
	int32		iPosition;
	size		ui;

	iPosition = mcLiveness.Position(pcBlock, uiIndex);
	psHome = masHomes.Get(iRegister);
	for (ui = 0; ui < psHome->asRanges.NumElements(); ui++)
	{
		psRange = psHome->asRanges.Get(ui);
		if ((psRange->iFrom <= iPosition) && (iPosition < psRange->iTo))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// X or Y to hold an address for the block's instruction uiIndex: szPrefer
// when it is X or Y (the instruction's result goes there, after it), else
// one that is free (IsRegisterFree), else NULL.
//
//////////////////////////////////////////////////////////////////////////
const char* CIR65Backend::FreeAddressRegister(CIRBlock* pcBlock, size uiIndex, const char* szPrefer)
{
	if (szPrefer && ((StringCompare(szPrefer, "X") == 0) || (StringCompare(szPrefer, "Y") == 0)))
	{
		return szPrefer;
	}
	if (IsRegisterFree(IR65R_X, pcBlock, uiIndex))
	{
		return "X";
	}
	if (IsRegisterFree(IR65R_Y, pcBlock, uiIndex))
	{
		return "Y";
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// MemoryOperand for a narrow load or store that would be a cpyx: if the
// address it gives is through A or B (which cpyx can't use), it is written
// again with the address in a free X or Y.  True if the operand fits cpyx;
// otherwise it is the one MemoryOperand gives with szWidened.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::CopyExtendOperand(SIRReference sAddress, CChars* pszOperand, const char* szWidened, CIRBlock* pcBlock, size uiIndex, const char* szPrefer)
{
	size		uiMark;
	const char*	szFree;

	uiMark = mpszOut->Length();
	MemoryOperand(sAddress, pszOperand, szWidened);
	if (CIR65Code::IsCopyExtendAddress(pszOperand))
	{
		return true;
	}
	szFree = FreeAddressRegister(pcBlock, uiIndex, szPrefer);
	if (szFree == NULL)
	{
		return false;
	}

	//Loading an address into B, X or Y changes nothing the back end keeps track of, so what was written can go.
	mpszOut->SetLength(uiMark);
	pszOperand->Kill();
	pszOperand->Init();
	mszAddress = szFree;
	MemoryOperand(sAddress, pszOperand, NULL);
	mszAddress = "B";
	if (CIR65Code::IsCopyExtendAddress(pszOperand))
	{
		return true;
	}

	mpszOut->SetLength(uiMark);
	pszOperand->Kill();
	pszOperand->Init();
	MemoryOperand(sAddress, pszOperand, szWidened);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The register to work out an instruction's result in: its own if it is
// kept in X or Y, otherwise A.
//
//////////////////////////////////////////////////////////////////////////
const char* CIR65Backend::WorkRegister(SIRInstruction* psInstruction)
{
	SIR65Place*	psPlace;

	if (!psInstruction->sResult.IsTemporary() || IsWide(psInstruction->sType))
	{
		return "A";
	}
	psPlace = PlaceOf(psInstruction->sResult);
	return (psPlace->uiKind == IR65P_Register) ? RegisterName(psPlace) : "A";
}


//////////////////////////////////////////////////////////////////////////
//
// Sign extends a narrow integer held zero extended (flip and subtract the
// sign bit), when eExtend says so.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Extend(const char* szRegister, SIRType sType, EIR65Extend eExtend)
{
	int		iBits;
	int64	lliSign;

	iBits = sType.IntegerBits();
	if ((eExtend == IR65E_Sign) && (iBits >= 1) && (iBits < 32))
	{
		lliSign = (int64)1 << (iBits - 1);
		Line("xor ")->Append(szRegister)->Append(", ")->Append(lliSign)->AppendNewLine();
		Line("sub ")->Append(szRegister)->Append(", ")->Append(lliSign)->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps the low bits of a narrow integer, zero extended.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Truncate(const char* szRegister, SIRType sType)
{
	if (IsNarrow(sType))
	{
		Line("and ")->Append(szRegister)->Append(", ")->Append((((int64)1) << sType.IntegerBits()) - 1)->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::LoadConstant(const char* szRegister, SIRType sType, uint64 ulliBits, EIR65Extend eExtend)
{
	int64	lliValue;

	if (IsWide(sType))
	{
		Line("wth ")->Append(szRegister)->Append(", 64")->AppendNewLine();
		Line("cpy ")->Append(szRegister)->Append(", ")->Append((int64)ulliBits)->AppendNewLine();
		Line("wth ")->Append(szRegister)->Append(", 32")->AppendNewLine();
		return;
	}
	lliValue = (eExtend == IR65E_Sign) ? IRSignExtend(sType, ulliBits) : (int64)ulliBits;
	Line("cpy ")->Append(szRegister)->Append(", ")->Append((int64)(int32)lliValue)->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// Loads a home into a register, narrow integers extended to 32 bits as
// eExtend says, 64 bit values with the register 64 bits wide (it is left 32
// bits wide, holding all 64).  A parameter pushed narrow is read at its own
// width and widened.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::LoadPlace(SIR65Place* psPlace, SIRType sType, const char* szRegister, EIR65Extend eExtend)
{
	CChars*	psz;

	if (psPlace->uiKind == IR65P_Register)
	{
		if (StringCompare(RegisterName(psPlace), szRegister) != 0)
		{
			Line("cpy ")->Append(szRegister)->Append(", ")->Append(RegisterName(psPlace))->AppendNewLine();
		}
		Extend(szRegister, sType, eExtend);
		return;
	}
	if (IsWide(sType))
	{
		Line("wth ")->Append(szRegister)->Append(", 64")->AppendNewLine();
		psz = Line("cpy ")->Append(szRegister)->Append(", ");
		AppendPlace(psz, psPlace);
		psz->AppendNewLine();
		Line("wth ")->Append(szRegister)->Append(", 32")->AppendNewLine();
		return;
	}
	if ((psPlace->uiKind == IR65P_Argument) && (psPlace->iBytes < 4) && (msFrame.cRegister == 'V'))
	{
		//cpyx extends the argument into the 32 bit register (it can't address with S).
		psz = Line("cpyx ")->Append(szRegister)->Append(", ");
		AppendPlace(psz, psPlace);
		psz->Append(((eExtend == IR65E_Sign) && IsNarrow(sType)) ? ":s" : ":")->Append(8 * psPlace->iBytes)->AppendNewLine();
		return;
	}
	if ((psPlace->uiKind == IR65P_Argument) && (psPlace->iBytes < 4))
	{
		Line("wth ")->Append(szRegister)->Append(", ")->Append(8 * psPlace->iBytes)->AppendNewLine();
		psz = Line("cpy ")->Append(szRegister)->Append(", ");
		AppendPlace(psz, psPlace);
		psz->AppendNewLine();
		Line("wthx ")->Append(szRegister)->Append(((eExtend == IR65E_Sign) && IsNarrow(sType)) ? ", 32:s" : ", 32")->AppendNewLine();
		return;
	}
	psz = Line("cpy ")->Append(szRegister)->Append(", ");
	AppendPlace(psz, psPlace);
	psz->AppendNewLine();
	Extend(szRegister, sType, eExtend);
}


//////////////////////////////////////////////////////////////////////////
//
// Stores a register to a home.  Narrow values are already zero extended.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::StorePlace(SIR65Place* psPlace, SIRType sType, const char* szRegister)
{
	CChars*	psz;

	if (psPlace->uiKind == IR65P_Register)
	{
		if (StringCompare(RegisterName(psPlace), szRegister) != 0)
		{
			Line("cpy ")->Append(RegisterName(psPlace))->Append(", ")->Append(szRegister)->AppendNewLine();
		}
		return;
	}
	if (IsWide(sType))
	{
		Line("wth ")->Append(szRegister)->Append(", 64")->AppendNewLine();
	}
	psz = Line("cpy ");
	AppendPlace(psz, psPlace);
	psz->Append(", ")->Append(szRegister)->AppendNewLine();
	if (IsWide(sType))
	{
		Line("wth ")->Append(szRegister)->Append(", 32")->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Loads a value into a register.  Only a result's own register is loaded
// in place (its operand dies there, so extending it is fine).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::LoadValue(SIRReference sValue, const char* szRegister, EIR65Extend eExtend)
{
	SIR65Place*	psPlace;
	bool		bA;

	bA = StringCompare(szRegister, "A") == 0;
	if (IsDeferred(sValue))
	{
		if (bA)
		{
			msAvailable = IRNone();
		}
		Line("cpy ")->Append(szRegister)->Append(", ")->Append(mszDeferred)->AppendNewLine();
		return;
	}
	if (IsInA(sValue))
	{
		if (!bA && IsWide(mpcFunction->TypeOf(sValue)))
		{
			Line("wtth (A, ")->Append(szRegister)->Append("), 64")->AppendNewLine();
			Line("cpy ")->Append(szRegister)->Append(", A")->AppendNewLine();
			Line("wtth (A, ")->Append(szRegister)->Append("), 32")->AppendNewLine();
			return;
		}
		if (!bA)
		{
			Line("cpy ")->Append(szRegister)->Append(", A")->AppendNewLine();
		}
		else if ((eExtend == IR65E_Sign) && IsNarrow(mpcFunction->TypeOf(sValue)))
		{
			msAvailable = IRNone();
		}
		Extend(szRegister, mpcFunction->TypeOf(sValue), eExtend);
		return;
	}
	if (bA)
	{
		msAvailable = IRNone();
	}

	switch (sValue.Kind())
	{
	case IRRK_Constant:
		LoadConstant(szRegister, mpcFunction->GetConstant(sValue)->sType, mpcFunction->GetConstant(sValue)->ulliBits, eExtend);
		return;
	case IRRK_Symbol:
		Line("cpy ")->Append(szRegister)->Append(", ")->Append(mpcModule->GetSymbolName(sValue.Index()))->AppendNewLine();
		return;
	case IRRK_Temporary:
		break;
	default:
		Error("Operand can't be loaded.");
		return;
	}

	psPlace = PlaceOf(sValue);
	switch (psPlace->uiKind)
	{
	case IR65P_FrameAddress:
	case IR65P_ArgumentAddress:
		Line("cpy ")->Append(szRegister)->Append(", ")->Append(msFrame.cRegister)->AppendNewLine();
		Line("add ")->Append(szRegister)->Append(", ")->Append(((psPlace->uiKind == IR65P_ArgumentAddress) ? msFrame.iArguments : 1) + psPlace->iOffset)->AppendNewLine();
		return;
	case IR65P_Register:
	case IR65P_Pseudo:
	case IR65P_Frame:
	case IR65P_Argument:
		LoadPlace(psPlace, mpcFunction->GetTemporaryType(sValue), szRegister, eExtend);
		return;
	default:
		Error("Temporary isn't where it was left.");
		return;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps a result worked out in szRegister (truncated if narrow, unless
// bExtended says it is already) in its home, or in A for the next
// instruction.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::StoreResult(SIRReference sResult, const char* szRegister, bool bExtended)
{
	SIR65Place*	psPlace;
	SIRType		sType;
	bool		bA;

	if (!sResult.IsTemporary())
	{
		return;
	}
	psPlace = PlaceOf(sResult);
	sType = mpcFunction->GetTemporaryType(sResult);
	bA = StringCompare(szRegister, "A") == 0;
	if (psPlace->bInA)
	{
		if (!bA)
		{
			Line("cpy A, ")->Append(szRegister)->AppendNewLine();
		}
		if (!bExtended)
		{
			Truncate("A", sType);
		}
		msInA = sResult;
		return;
	}
	if (psPlace->uiKind == IR65P_None)
	{
		//Not used.
		return;
	}
	if ((psPlace->uiKind != IR65P_Register) && (psPlace->uiKind != IR65P_Pseudo) && (psPlace->uiKind != IR65P_Frame))
	{
		Error("Result has no home.");
		return;
	}
	if (!bExtended)
	{
		Truncate(szRegister, sType);
	}
	StorePlace(psPlace, sType, szRegister);
	if (bA)
	{
		msInA = sResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The memory operand for an address: [symbol], [n + V] for a slot or an
// argument, [X] for an address kept in a register, [[n + W]] or [[n + V]]
// for one kept in memory, otherwise [B].  szWidened is the register the
// access narrows or widens (an address in it is copied to B first).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::MemoryOperand(SIRReference sAddress, CChars* pszOperand, const char* szWidened)
{
	SIR65Place*	psPlace;

	if (sAddress.IsSymbol())
	{
		pszOperand->Append('[')->Append(mpcModule->GetSymbolName(sAddress.Index()))->Append(']');
		return;
	}
	if (sAddress.IsConstant())
	{
		pszOperand->Append('[')->Append((int64)(uint32)mpcFunction->GetConstant(sAddress)->ulliBits)->Append(']');
		return;
	}
	if (sAddress.IsTemporary() && (PlaceOf(sAddress)->uiKind == IR65P_Folded))
	{
		FoldedOperand(PlaceOf(sAddress), pszOperand, szWidened);
		return;
	}
	if (sAddress.IsTemporary())
	{
		if (IsInA(sAddress) && !InRegister(sAddress))
		{
			if (((szWidened == NULL) || (StringCompare(szWidened, "A") != 0)) && (StringCompare(mszAddress, "B") == 0))
			{
				pszOperand->Append("[A]");
				return;
			}
		}
		else
		{
			psPlace = PlaceOf(sAddress);
			switch (psPlace->uiKind)
			{
			case IR65P_FrameAddress:
			case IR65P_ArgumentAddress:
				AppendPlace(pszOperand, psPlace);
				return;
			case IR65P_Register:
				if ((szWidened == NULL) || (StringCompare(szWidened, RegisterName(psPlace)) != 0))
				{
					pszOperand->Append('[')->Append(RegisterName(psPlace))->Append(']');
					return;
				}
				break;
			default:
				if (IsMemoryHome(psPlace))
				{
					pszOperand->Append('[');
					AppendPlace(pszOperand, psPlace);
					pszOperand->Append(']');
					return;
				}
				break;
			}
		}
	}
	LoadValue(sAddress, mszAddress, IR65E_Zero);
	pszOperand->Append('[')->Append(mszAddress)->Append(']');
}


//////////////////////////////////////////////////////////////////////////
//
// A folded ptradd's address: [symbol + offset], [n + V] for a slot or an
// argument, [offset + R] with the base in a register (not szWidened) or B.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::FoldedOperand(SIR65Place* psFolded, CChars* pszOperand, const char* szWidened)
{
	SIRReference	sBase;
	SIR65Place*		psBase;
	SIR65Place		sAddress;
	const char*		szRegister;

	sBase = psFolded->sBase;
	if (sBase.IsSymbol())
	{
		pszOperand->Append('[')->Append(mpcModule->GetSymbolName(sBase.Index()));
		if (psFolded->iOffset != 0)
		{
			pszOperand->Append((psFolded->iOffset < 0) ? " - " : " + ")->Append((int64)((psFolded->iOffset < 0) ? -(int64)psFolded->iOffset : psFolded->iOffset));
		}
		pszOperand->Append(']');
		return;
	}
	if (sBase.IsConstant())
	{
		pszOperand->Append('[')->Append((int64)(uint32)(mpcFunction->GetConstant(sBase)->ulliBits + psFolded->iOffset))->Append(']');
		return;
	}

	szRegister = NULL;
	if (IsInA(sBase))
	{
		szRegister = "A";
	}
	else if (sBase.IsTemporary())
	{
		psBase = PlaceOf(sBase);
		if ((psBase->uiKind == IR65P_FrameAddress) || (psBase->uiKind == IR65P_ArgumentAddress))
		{
			sAddress = *psBase;
			sAddress.iOffset += psFolded->iOffset;
			AppendPlace(pszOperand, &sAddress);
			return;
		}
		if (psBase->uiKind == IR65P_Register)
		{
			szRegister = RegisterName(psBase);
		}
	}
	if ((szRegister == NULL) || (szWidened && (StringCompare(szRegister, szWidened) == 0)) || ((StringCompare(szRegister, "A") == 0) && (StringCompare(mszAddress, "B") != 0)))
	{
		LoadValue(sBase, mszAddress, IR65E_Zero);
		szRegister = mszAddress;
	}
	pszOperand->Append('[')->Append(psFolded->iOffset)->Append(" + ")->Append(szRegister)->Append(']');
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes a value at the size of sType: straight from its register, memory
// or as an immediate when it can be, otherwise from A.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Push(SIRReference sValue, SIRType sType)
{
	int			iBytes;
	CChars		szOperand;
	SIR65Place*	psPlace;
	const char*	szRegister;

	iBytes = ValueBytes(sType);
	if (iBytes == 8)
	{
		LoadValue(sValue, "A", IR65E_Zero);
		Line("wth A, 64")->AppendNewLine();
		Line("psh A")->AppendNewLine();
		Line("wth A, 32")->AppendNewLine();
		return;
	}
	if (sValue.IsConstant())
	{
		szOperand.Init();
		AppendImmediate(&szOperand, sValue, IR65E_Zero);
		Line("psh ")->Append(szOperand)->Append(':')->Append(8 * iBytes)->AppendNewLine();
		szOperand.Kill();
		return;
	}

	szRegister = NULL;
	if (IsInA(sValue))
	{
		szRegister = "A";
	}
	else if (InRegister(sValue))
	{
		szRegister = RegisterName(PlaceOf(sValue));
	}
	else if ((iBytes == 4) && sValue.IsTemporary())
	{
		psPlace = PlaceOf(sValue);
		if (IsMemoryHome(psPlace))
		{
			szOperand.Init();
			AppendPlace(&szOperand, psPlace);
			Line("psh ")->Append(szOperand)->AppendNewLine();
			szOperand.Kill();
			return;
		}
	}
	if (szRegister == NULL)
	{
		LoadValue(sValue, "A", IR65E_Zero);
		szRegister = "A";
	}
	if (iBytes == 4)
	{
		Line("psh ")->Append(szRegister)->AppendNewLine();
		return;
	}
	Line("wth ")->Append(szRegister)->Append(", ")->Append(8 * iBytes)->AppendNewLine();
	Line("psh ")->Append(szRegister)->AppendNewLine();
	Line("wth ")->Append(szRegister)->Append(", 32")->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// mul and div count their steps in W, from 0.  W is always 0 (the calling
// convention, CPP65Generator.txt), so it is only set back afterwards.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Multiply(const char* szInstruction)
{
	Line(szInstruction)->AppendNewLine();
	Line("cpy W, 0")->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Instruction(CIRBlock* pcBlock, size uiIndex)
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
		iTrue = NewLabel();
		iEnd = NewLabel();
		if (!Compare(psInstruction, true, iTrue))
		{
			return false;
		}
		szWork = WorkRegister(psInstruction);
		if (StringCompare(szWork, "A") == 0)
		{
			msAvailable = IRNone();
		}
		Line("cpy ")->Append(szWork)->Append(", 0")->AppendNewLine();
		Branch("bra ", iEnd);
		Label(iTrue);
		Line("cpy ")->Append(szWork)->Append(", 1")->AppendNewLine();
		Label(iEnd);
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
		return Call(pcBlock, uiIndex);

	default:
		return Error("Opcode not supported by the 65C64 back end.", GetIROpcode(psInstruction->uiOpcode) ? GetIROpcode(psInstruction->uiOpcode)->szName : NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Commutes(uint16 uiOpcode)
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
bool CIR65Backend::ReadModifyWrite(SIRInstruction* psInstruction, SIRInstruction* psStore)
{
	SIRReference	sMemory;
	SIRReference	sOther;
	CChars			szAddress;
	CChars			szOperand;
	char			cRegister;
	bool			bShift;
	bool			bResult;
	CChars*			psz;

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
	if (IsNarrow(psInstruction->sType) || IsWide(psInstruction->sType) || !psInstruction->sResult.IsTemporary() || !PlaceOf(psInstruction->sResult)->bInA)
	{
		return false;
	}
	if (IsDeferred(psInstruction->asArguments[0]))
	{
		sMemory = psInstruction->asArguments[0];
		sOther = psInstruction->asArguments[1];
	}
	else if (IsDeferred(psInstruction->asArguments[1]) && Commutes(psInstruction->uiOpcode))
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
	if (IsInA(sOther))
	{
		szOperand.Append('A');
	}
	else if (InRegister(sOther) && !bShift)
	{
		//A shift of memory takes its amount from A or B.
		szOperand.Append(RegisterName(PlaceOf(sOther)));
	}
	else if (SmallImmediate(sOther, bShift ? 1 : -8, 8))
	{
		AppendImmediate(&szOperand, sOther, IR65E_Zero);
	}
	szAddress.Init();
	cRegister = '\0';
	bResult = !szOperand.Empty() && PureMemoryOperand(psStore->asArguments[1], &szAddress, &cRegister) && szAddress.Equals(mszDeferred.Text());
	if (bResult && ((psInstruction->uiOpcode == IRO_Add) || (psInstruction->uiOpcode == IRO_Sub) || bShift) && IsFrameOperand(&szAddress))
	{
		//add, sub, shl and shr can't address memory with S.
		bResult = false;
	}
	if (bResult)
	{
		switch (psInstruction->uiOpcode)
		{
		case IRO_Add:	psz = Line("add ");	break;
		case IRO_Sub:	psz = Line("sub ");	break;
		case IRO_And:	psz = Line("and ");	break;
		case IRO_Or:	psz = Line("or ");	break;
		case IRO_Xor:	psz = Line("xor ");	break;
		case IRO_Shl:	psz = Line("shl ");	break;
		default:		psz = Line("shr ");	break;
		}
		psz->Append(szAddress)->Append(", ")->Append(szOperand)->AppendNewLine();
		msInA = msAvailable;
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
bool CIR65Backend::Integer(SIRInstruction* psInstruction)
{
	uint16			uiOpcode;
	const char*		szOperation;
	const char*		szWork;
	bool			bArithmetic;
	bool			bShift;
	SIRReference	sLeft;
	SIRReference	sRight;
	SIRReference	sResult;
	SIR65Place*		psResult;
	CChars			szOperand;
	CChars*			psz;

	if (IsWide(psInstruction->sType))
	{
		return IntegerWide(psInstruction);
	}
	uiOpcode = psInstruction->uiOpcode;
	sLeft = psInstruction->asArguments[0];
	sRight = psInstruction->asArguments[1];
	sResult = psInstruction->sResult;
	bShift = (uiOpcode == IRO_Shl) || (uiOpcode == IRO_LShr);
	bArithmetic = (uiOpcode == IRO_Add) || (uiOpcode == IRO_Sub) || (uiOpcode == IRO_PtrAdd) || bShift;
	switch (uiOpcode)
	{
	case IRO_Add:
	case IRO_PtrAdd:
		szOperation = "add ";
		break;
	case IRO_Sub:
		szOperation = "sub ";
		break;
	case IRO_And:
		szOperation = "and ";
		break;
	case IRO_Or:
		szOperation = "or ";
		break;
	case IRO_Xor:
	case IRO_Not:
		szOperation = "xor ";
		break;
	case IRO_Shl:
		szOperation = "shl ";
		break;
	case IRO_LShr:
		szOperation = "shr ";
		break;
	case IRO_Neg:
		szOperation = "neg ";
		break;
	default:
		szOperation = NULL;
		break;
	}

	//Where the first operand is kept already (or the second, if they swap).
	if (Commutes(uiOpcode) && SamePlace(sRight, sResult) && !SamePlace(sLeft, sResult))
	{
		sLeft = psInstruction->asArguments[1];
		sRight = psInstruction->asArguments[0];
	}
	if (sResult.IsTemporary() && !IsNarrow(psInstruction->sType) && SamePlace(sLeft, sResult) && !SamePlace(sRight, sResult))
	{
		psResult = PlaceOf(sResult);
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
				Line("neg ")->Append(RegisterName(psResult))->Append(", ")->Append(RegisterName(psResult))->AppendNewLine();
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
				Line(szOperation)->Append(RegisterName(psResult))->Append(", ")->Append(szOperand)->AppendNewLine();
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
			else if (IsInA(sRight))
			{
				szOperand.Append('A');
			}
			else if (InRegister(sRight) && !bShift)
			{
				//A shift of memory takes its amount from A or B.
				szOperand.Append(RegisterName(PlaceOf(sRight)));
			}
			else if (SmallImmediate(sRight, bShift ? 1 : -8, 8))
			{
				AppendImmediate(&szOperand, sRight, IR65E_Zero);
			}
			if (!szOperand.Empty())
			{
				psz = Line(szOperation);
				AppendPlace(psz, psResult);
				psz->Append(", ")->Append(szOperand)->AppendNewLine();
				szOperand.Kill();
				return true;
			}
		}
		szOperand.Kill();
	}

	szWork = WorkRegister(psInstruction);
	if ((StringCompare(szWork, "A") != 0) && (IsInA(sLeft) || SamePlace(sRight, sResult)))
	{
		szWork = "A";
	}

	szOperand.Init();
	if ((StringCompare(szWork, "A") == 0) && IsInA(sRight) && !IsInA(sLeft))
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
			Line("neg A, A")->AppendNewLine();
			szOperation = "add ";
		}
		else
		{
			szOperand.Clear();
			Line("cpy B, A")->AppendNewLine();
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
		Line("neg ")->Append(szWork)->Append(", ")->Append(szWork)->AppendNewLine();
	}
	else if (uiOpcode == IRO_Not)
	{
		Line("xor ")->Append(szWork)->Append(", -1")->AppendNewLine();
	}
	else if (szOperation)
	{
		Line(szOperation)->Append(szWork)->Append(", ")->Append(szOperand)->AppendNewLine();
	}
	szOperand.Kill();
	if (StringCompare(szWork, "A") == 0)
	{
		msAvailable = IRNone();
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
bool CIR65Backend::IntegerWide(SIRInstruction* psInstruction)
{
	const char*	szInstruction;

	LoadOperands(psInstruction, IR65E_Zero, true);
	Line("wtth (A, B), 64")->AppendNewLine();
	switch (psInstruction->uiOpcode)
	{
	case IRO_Add:
	case IRO_PtrAdd:
		szInstruction = "add A, ";
		break;
	case IRO_Sub:
		szInstruction = "sub A, ";
		break;
	case IRO_And:
		szInstruction = "and A, ";
		break;
	case IRO_Or:
		szInstruction = "or A, ";
		break;
	case IRO_Xor:
		szInstruction = "xor A, ";
		break;
	case IRO_Shl:
		szInstruction = "shl A, ";
		break;
	case IRO_LShr:
		szInstruction = "shr A, ";
		break;
	case IRO_Neg:
		szInstruction = "neg A, A";
		break;
	case IRO_Not:
		szInstruction = "xor A, -1";
		break;
	default:
		szInstruction = NULL;
		break;
	}
	if (szInstruction)
	{
		Line(szInstruction);
		if (szInstruction[StrLen(szInstruction) - 1] == ' ')
		{
			if (mbRightImmediate)
			{
				AppendImmediate(mpszOut, psInstruction->asArguments[1], IR65E_Zero);
			}
			else
			{
				mpszOut->Append('B');
			}
		}
		mpszOut->AppendNewLine();
	}
	Line("wtth (A, B), 32")->AppendNewLine();
	msAvailable = IRNone();
	StoreResult(psInstruction->sResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A x B -> Y.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::MultiplyInstruction(SIRInstruction* psInstruction)
{
	bool	bWide;
	bool	bByte;

	bWide = IsWide(psInstruction->sType);
	if (!bWide && MultiplyByShifts(psInstruction))
	{
		return true;
	}
	if (!bWide && IsInA(psInstruction->asArguments[1]) && !IsInA(psInstruction->asArguments[0]))
	{
		LoadValue(psInstruction->asArguments[0], "B", IR65E_Zero);
	}
	else
	{
		LoadOperands(psInstruction, IR65E_Zero, false);
	}
	if (bWide)
	{
		Line("wtth (A, B), 64")->AppendNewLine();
		Line("wth Y, 64")->AppendNewLine();
	}
	bByte = !bWide && psInstruction->asArguments[1].IsConstant() && (IRTruncate(psInstruction->sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits) < 256);
	if (bByte)
	{
		//mul takes a step for each byte of B: a multiplier below 256 takes one.
		Line("wth B, 8")->AppendNewLine();
	}
	Multiply("mul");
	if (bByte)
	{
		Line("wth B, 32")->AppendNewLine();
	}
	msAvailable = IRNone();
	if (bWide)
	{
		Line("cpy A, Y")->AppendNewLine();
		Line("wth Y, 32")->AppendNewLine();
		Line("wtth (A, B), 32")->AppendNewLine();
		StoreResult(psInstruction->sResult);
	}
	else
	{
		StoreResult(psInstruction->sResult, "Y");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// x * (2^a + 2^b) is ((x << (a - b)) + x) << b and x * (2^a - 2^b) is
// ((x << (a - b)) - x) << b, which beat mul (a step a byte of B, with W
// saved round it).  Returns false for other multipliers.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::MultiplyByShifts(SIRInstruction* psInstruction)
{
	uint32		uiMultiplier;
	uint32		uiLow;
	uint32		uiRest;
	int			iLow;
	int			iHigh;
	const char*	szCombine;

	if (!psInstruction->asArguments[1].IsConstant())
	{
		return false;
	}
	uiMultiplier = (uint32)IRTruncate(psInstruction->sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits);
	if (uiMultiplier == 0)
	{
		return false;
	}
	uiLow = uiMultiplier & (0 - uiMultiplier);
	uiRest = uiMultiplier - uiLow;
	szCombine = "add A, B";
	if ((uiRest == 0) || ((uiRest & (uiRest - 1)) != 0))
	{
		//Not two bits: maybe a run of them, 2^a - 2^b.
		uiRest = uiMultiplier + uiLow;
		if ((uiRest == 0) || ((uiRest & (uiRest - 1)) != 0))
		{
			return false;
		}
		szCombine = "sub A, B";
	}
	iLow = (int)FindFirstSetBit(uiLow);
	iHigh = (int)FindFirstSetBit(uiRest);
	if ((iHigh - iLow > 8) || (iLow > 8))
	{
		return false;
	}

	LoadValue(psInstruction->asArguments[0], "A", IR65E_Zero);
	Line("cpy B, A")->AppendNewLine();
	Line("shl A, ")->Append(iHigh - iLow)->AppendNewLine();
	Line(szCombine)->AppendNewLine();
	if (iLow > 0)
	{
		Line("shl A, ")->Append(iLow)->AppendNewLine();
	}
	msAvailable = IRNone();
	StoreResult(psInstruction->sResult, "A");
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// ashr: shr is logical, so a negative value is inverted, shifted and
// inverted back.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::ShiftSigned(SIRInstruction* psInstruction)
{
	bool	bWide;
	int32	iPositive;
	int32	iEnd;
	CChars	szAmount;

	bWide = IsWide(psInstruction->sType);
	LoadOperands(psInstruction, IR65E_Sign, true);
	szAmount.Init();
	if (mbRightImmediate)
	{
		AppendImmediate(&szAmount, psInstruction->asArguments[1], IR65E_Zero);
	}
	else
	{
		szAmount.Append('B');
	}
	if (bWide)
	{
		Line("wtth (A, B), 64")->AppendNewLine();
	}
	iPositive = NewLabel();
	iEnd = NewLabel();
	Line("cmp A, 0")->AppendNewLine();
	Branch("brac n, ", iPositive);
	Line("xor A, -1")->AppendNewLine();
	Line("shr A, ")->Append(szAmount)->AppendNewLine();
	Line("xor A, -1")->AppendNewLine();
	Branch("bra ", iEnd);
	Label(iPositive);
	Line("shr A, ")->Append(szAmount)->AppendNewLine();
	Label(iEnd);
	szAmount.Kill();
	if (bWide)
	{
		Line("wtth (A, B), 32")->AppendNewLine();
	}
	msAvailable = IRNone();
	StoreResult(psInstruction->sResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Unsigned: div (Y / A -> Y, the remainder in B, which starts at 0); a
// psNext that is its DividePartner gets its result from the same div.
// Signed: R_DivideSigned (A / B -> A, the remainder in B).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Divide(SIRInstruction* psInstruction, SIRInstruction* psNext)
{
	bool			bWide;
	bool			bSigned;
	bool			bRemainder;
	SIRReference	sLeft;
	SIRReference	sRight;

	bWide = IsWide(psInstruction->sType);
	bSigned = (psInstruction->uiOpcode == IRO_SDiv) || (psInstruction->uiOpcode == IRO_SRem);
	bRemainder = (psInstruction->uiOpcode == IRO_SRem) || (psInstruction->uiOpcode == IRO_URem);
	sLeft = psInstruction->asArguments[0];
	sRight = psInstruction->asArguments[1];

	if (bSigned || bWide)
	{
		LoadOperands(psInstruction, bSigned ? IR65E_Sign : IR65E_Zero, false);
		if (bWide)
		{
			Line("wtth (A, B), 64")->AppendNewLine();
		}
		if (bSigned)
		{
			Line(bWide ? "jsr R_DivideSigned64" : "jsr R_DivideSigned")->AppendNewLine();
			if (bWide)
			{
				mbSignedDivide64 = true;
			}
			else
			{
				mbSignedDivide = true;
			}
		}
		else
		{
			Line("wth Y, 64")->AppendNewLine();
			Line("cpy Y, A")->AppendNewLine();
			Line("cpy A, B")->AppendNewLine();
			Line("cpy B, 0")->AppendNewLine();
			Multiply("div");
			Line("cpy A, Y")->AppendNewLine();
			Line("wth Y, 32")->AppendNewLine();
		}
		if (bRemainder)
		{
			Line("cpy A, B")->AppendNewLine();
		}
		if (bWide)
		{
			Line("wtth (A, B), 32")->AppendNewLine();
		}
		msAvailable = IRNone();
		StoreResult(psInstruction->sResult);
		return true;
	}

	//The dividend to Y and the divisor to A, in an order that loses neither.
	if (IsInA(sRight) && !IsInA(sLeft))
	{
		LoadValue(sLeft, "Y", IR65E_Zero);
	}
	else if (InRegister(sRight) && (PlaceOf(sRight)->iOffset == IR65R_Y) && !sLeft.Equals(sRight))
	{
		if (IsInA(sLeft))
		{
			Line("cpy B, A")->AppendNewLine();
			Line("cpy A, Y")->AppendNewLine();
			Line("cpy Y, B")->AppendNewLine();
		}
		else
		{
			LoadValue(sRight, "A", IR65E_Zero);
			LoadValue(sLeft, "Y", IR65E_Zero);
		}
	}
	else
	{
		LoadValue(sLeft, "Y", IR65E_Zero);
		LoadValue(sRight, "A", IR65E_Zero);
	}
	Line("cpy B, 0")->AppendNewLine();
	Multiply("div");
	msAvailable = IRNone();
	if (psNext && DividePartner(psInstruction, psNext) && !PlaceOf(psInstruction->sResult)->bInA)
	{
		//The result from Y goes first, as the other may be kept in Y; one kept in A goes last.
		if (!bRemainder)
		{
			StoreResult(psInstruction->sResult, "Y");
			StoreResult(psNext->sResult, "B");
			return true;
		}
		if (!PlaceOf(psNext->sResult)->bInA)
		{
			StoreResult(psNext->sResult, "Y");
			StoreResult(psInstruction->sResult, "B");
			return true;
		}
	}
	StoreResult(psInstruction->sResult, bRemainder ? "B" : "Y");
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// sext, zext, trunc and the pointer conversions.  Narrow values are kept zero
// extended, so a conversion is a load (extended), a widening to 64 bits if
// the result is wide, and a store (truncated).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Conversion(SIRInstruction* psInstruction)
{
	SIRType		sFrom;
	bool		bSigned;
	const char*	szWork;

	sFrom = mpcFunction->TypeOf(psInstruction->asArguments[0]);
	bSigned = psInstruction->uiOpcode == IRO_SExt;
	szWork = WorkRegister(psInstruction);
	LoadValue(psInstruction->asArguments[0], szWork, bSigned ? IR65E_Sign : IR65E_Zero);
	if (IsWide(psInstruction->sType) && !IsWide(sFrom))
	{
		Line(bSigned ? "wthx A, 64:s" : "wthx A, 64")->AppendNewLine();
		Line("wth A, 32")->AppendNewLine();
	}
	StoreResult(psInstruction->sResult, szWork);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Floating point is the soft float runtime's: the operands' bits are pushed
// (the right first, as arguments are) and the result is in A.  fneg flips the
// sign bit in line.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::FloatOperation(SIRInstruction* psInstruction)
{
	bool		bDouble;
	const char*	szOperation;
	CChars		szName;

	bDouble = psInstruction->sType.Is(IRTK_F64);
	if (psInstruction->uiOpcode == IRO_FNeg)
	{
		LoadValue(psInstruction->asArguments[0], "A", IR65E_Zero);
		if (bDouble)
		{
			Line("wth A, 64")->AppendNewLine();
			Line("xor A, -9223372036854775808")->AppendNewLine();
			Line("wth A, 32")->AppendNewLine();
		}
		else
		{
			Line("xor A, -2147483648")->AppendNewLine();
		}
		msAvailable = IRNone();
		StoreResult(psInstruction->sResult);
		return true;
	}

	switch (psInstruction->uiOpcode)
	{
	case IRO_FAdd:	szOperation = "Add";		break;
	case IRO_FSub:	szOperation = "Subtract";	break;
	case IRO_FMul:	szOperation = "Multiply";	break;
	default:		szOperation = "Divide";		break;
	}
	Push(psInstruction->asArguments[1], psInstruction->sType);
	Push(psInstruction->asArguments[0], psInstruction->sType);
	szName.Init(bDouble ? "SoftDouble" : "SoftFloat");
	szName.Append(szOperation);
	CallRuntime(szName.Text(), 2 * ValueBytes(psInstruction->sType));
	szName.Kill();
	msAvailable = IRNone();
	StoreResult(psInstruction->sResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Conversions to, from and between floats, by the soft float runtime.
// Integers narrower than 32 bits convert as ints.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::FloatConversion(SIRInstruction* psInstruction)
{
	SIRType	sFrom;
	SIRType	sTo;
	bool	bSigned;
	CChars	szName;

	sFrom = mpcFunction->TypeOf(psInstruction->asArguments[0]);
	sTo = psInstruction->sType;
	switch (psInstruction->uiOpcode)
	{
	case IRO_FExt:
	case IRO_FTrunc:
		Push(psInstruction->asArguments[0], sFrom);
		CallRuntime(sTo.Is(IRTK_F64) ? "SoftFloatToDouble" : "SoftDoubleToFloat", ValueBytes(sFrom));
		break;

	case IRO_SIToF:
	case IRO_UIToF:
		bSigned = psInstruction->uiOpcode == IRO_SIToF;
		szName.Init(sTo.Is(IRTK_F64) ? "SoftDoubleFrom" : "SoftFloatFrom");
		if (IsWide(sFrom))
		{
			szName.Append(bSigned ? "LongLong" : "UnsignedLongLong");
		}
		else
		{
			szName.Append((bSigned || (sFrom.IntegerBits() < 32)) ? "Int" : "Unsigned");
		}
		if (IsWide(sFrom))
		{
			Push(psInstruction->asArguments[0], sFrom);
		}
		else
		{
			//Narrow integers are pushed as ints, extended.
			LoadValue(psInstruction->asArguments[0], "A", bSigned ? IR65E_Sign : IR65E_Zero);
			Line("psh A")->AppendNewLine();
		}
		CallRuntime(szName.Text(), IsWide(sFrom) ? 8 : 4);
		szName.Kill();
		break;

	default:
		bSigned = psInstruction->uiOpcode == IRO_FToSI;
		szName.Init(sFrom.Is(IRTK_F64) ? "SoftDoubleTo" : "SoftFloatTo");
		if (IsWide(sTo))
		{
			szName.Append(bSigned ? "LongLong" : "UnsignedLongLong");
		}
		else
		{
			szName.Append((bSigned || (sTo.IntegerBits() < 32)) ? "Int" : "Unsigned");
		}
		Push(psInstruction->asArguments[0], sFrom);
		CallRuntime(szName.Text(), ValueBytes(sFrom));
		szName.Kill();
		break;
	}
	msAvailable = IRNone();
	StoreResult(psInstruction->sResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether instruction uiIndex of pcBlock isn't written in its turn, as the
// W65C816 back end's CanFuse: the next instruction written (its consumer,
// WriteInstruction) writes it.  A load it reads as a memory operand
// (CanDefer), a narrow load its sext sign extends (ExtendOf), a div its
// partner gives the other result of (PairedDivide), or a result only the
// consumer uses, which it takes from A (bInA, MarkInA).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::CanFuse(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;
	SIRInstruction*	psPartner;
	SIR65Fused*		psLast;

	psLast = (masFused.NumElements() > 0) ? masFused.Get(masFused.NumElements() - 1) : NULL;
	if (psLast && ((psLast->uiKind == IR65FU_Extended) || (psLast->uiKind == IR65FU_Paired)))
	{
		//The sext or partner that writes the instruction before it is written in its turn.
		return false;
	}
	psInstruction = pcBlock->masInstructions.Get(uiIndex);
	if (psInstruction->uiOpcode == IRO_Load)
	{
		//PureMemoryOperand asks what A will hold when the consumer starts.
		msAvailable = InAAfterFused();
		if (CanDefer(pcBlock, uiIndex))
		{
			Fuse(pcBlock, uiIndex, IR65FU_Deferred);
			return true;
		}
		if (ExtendOf(pcBlock, uiIndex))
		{
			Fuse(pcBlock, uiIndex, IR65FU_Extended);
			return true;
		}
	}
	psPartner = PairedDivide(pcBlock, uiIndex);
	if (psPartner && !Skipped(psPartner))
	{
		Fuse(pcBlock, uiIndex, IR65FU_Paired);
		return true;
	}
	if (psInstruction->sResult.IsTemporary() && PlaceOf(psInstruction->sResult)->bInA)
	{
		Fuse(pcBlock, uiIndex, IR65FU_InA);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Fuse(CIRBlock* pcBlock, size uiIndex, uint8 uiKind)
{
	SIR65Fused*	psFused;

	psFused = masFused.Add();
	psFused->psInstruction = pcBlock->masInstructions.Get(uiIndex);
	psFused->uiIndex = uiIndex;
	psFused->uiKind = uiKind;
}


//////////////////////////////////////////////////////////////////////////
//
// What A holds when the consumer of the instructions CanFuse has left
// starts: the last one taken from A, else what the last instruction
// written left there (a deferred load doesn't change A).
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIR65Backend::InAAfterFused(void)
{
	size	ui;

	for (ui = masFused.NumElements(); ui > 0; ui--)
	{
		if (masFused.Get(ui - 1)->uiKind == IR65FU_InA)
		{
			return masFused.Get(ui - 1)->psInstruction->sResult;
		}
	}
	return msInA;
}


//////////////////////////////////////////////////////////////////////////
//
// The sext just after the narrow load uiLoad when it is the load's only
// use: the load is sign extended as it is widened, giving the sext's result
// (Load).  NULL for none.
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIR65Backend::ExtendOf(CIRBlock* pcBlock, size uiLoad)
{
	SIRInstruction*	psLoad;
	SIRInstruction*	psExtend;

	psLoad = pcBlock->masInstructions.Get(uiLoad);
	psExtend = pcBlock->masInstructions.SafeGet(uiLoad + 1);
	if ((psLoad->uiOpcode != IRO_Load) || (ValueBytes(psLoad->sType) >= 4) || (psExtend == NULL) || (psExtend->uiOpcode != IRO_SExt) || !psExtend->asArguments[0].Equals(psLoad->sResult) ||
		!psLoad->sResult.IsTemporary() || (*mauiUses.Get(psLoad->sResult.Index()) != 1) || IsWide(psExtend->sType) || Skipped(psExtend))
	{
		return NULL;
	}
	return psExtend;
}


//////////////////////////////////////////////////////////////////////////
//
// The partner (DividePartner) just after the udiv or urem uiIndex that one
// div gives the results of both, the one kept in Y not kept in A; NULL for
// none.
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIR65Backend::PairedDivide(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;
	SIRInstruction*	psPartner;

	psInstruction = pcBlock->masInstructions.Get(uiIndex);
	psPartner = pcBlock->masInstructions.SafeGet(uiIndex + 1);
	if ((psPartner == NULL) || !DividePartner(psInstruction, psPartner) || PlaceOf(psInstruction->sResult)->bInA)
	{
		return NULL;
	}
	if ((psInstruction->uiOpcode == IRO_URem) && PlaceOf(psPartner->sResult)->bInA)
	{
		return NULL;
	}
	return psPartner;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a 4 byte load left in A for the next instruction can be deferred
// (IR65FU_Deferred): not written, its address needing no instructions and
// using no register the next instruction changes before reading it, so the
// next instruction reads memory itself (add A, [8 + X]; cpy Y, [[4 + W]]).
// Not before mul, div or ashr, nor just after another deferred load.  Sets
// mszDeferred.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::CanDefer(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psLoad;
	SIRInstruction*	psNext;
	size			uiNext;
	const char*		szWork;
	char			cRegister;
	SIRReference	sResult;
	CChars			szOperand;

	psLoad = pcBlock->masInstructions.Get(uiIndex);
	sResult = psLoad->sResult;
	if (((masFused.NumElements() > 0) && (masFused.Get(masFused.NumElements() - 1)->uiKind == IR65FU_Deferred)) || !sResult.IsTemporary() || !PlaceOf(sResult)->bInA || (psLoad->uiFlags & IRF_Volatile) || (ValueBytes(psLoad->sType) != 4))
	{
		return false;
	}
	uiNext = NextWritten(pcBlock, uiIndex, pcBlock->masInstructions.NumElements());
	psNext = pcBlock->masInstructions.SafeGet(uiNext);
	if (psNext)
	{
		if (IR65OpcodeFlags(psNext->uiOpcode) & IR65F_NO_DEFER)
		{
			return false;
		}
		szWork = WorkRegister(psNext);
	}
	else
	{
		szWork = "A";
	}

	szOperand.Init();
	cRegister = '\0';
	if (!PureMemoryOperand(psLoad->asArguments[0], &szOperand, &cRegister) || (cRegister == szWork[0]))
	{
		szOperand.Kill();
		return false;
	}
	mszDeferred.Kill();
	mszDeferred.Init(&szOperand);
	szOperand.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An address's memory operand when it needs no instructions and doesn't use
// A or B; cRegister is set to X or Y if it uses one.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::PureMemoryOperand(SIRReference sAddress, CChars* pszOperand, char* pcRegister)
{
	SIR65Place*		psPlace;
	SIR65Place*		psBase;
	SIRReference	sBase;

	if (sAddress.IsSymbol() || sAddress.IsConstant())
	{
		MemoryOperand(sAddress, pszOperand, NULL);
		return true;
	}
	if (!sAddress.IsTemporary() || IsInA(sAddress))
	{
		return false;
	}
	psPlace = PlaceOf(sAddress);
	switch (psPlace->uiKind)
	{
	case IR65P_Folded:
		sBase = psPlace->sBase;
		if (sBase.IsTemporary())
		{
			if (IsInA(sBase))
			{
				return false;
			}
			psBase = PlaceOf(sBase);
			if (psBase->uiKind == IR65P_Register)
			{
				*pcRegister = RegisterName(psBase)[0];
			}
			else if ((psBase->uiKind != IR65P_FrameAddress) && (psBase->uiKind != IR65P_ArgumentAddress))
			{
				return false;
			}
		}
		FoldedOperand(psPlace, pszOperand, NULL);
		return true;
	case IR65P_Register:
		*pcRegister = RegisterName(psPlace)[0];
		MemoryOperand(sAddress, pszOperand, NULL);
		return true;
	case IR65P_FrameAddress:
	case IR65P_ArgumentAddress:
		MemoryOperand(sAddress, pszOperand, NULL);
		return true;
	default:
		if (!IsMemoryHome(psPlace))
		{
			return false;
		}
		MemoryOperand(sAddress, pszOperand, NULL);
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Into the result's register (or A): narrow values read at their width
// and widened, 8 byte values with A 64 bits wide.  The load psExtend, the
// sext just after it (ExtendOf), writes is sign extended as it is widened,
// giving the sext's result.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Load(CIRBlock* pcBlock, size uiIndex, SIRInstruction* psExtend)
{
	SIRInstruction*	psInstruction;
	CChars			szOperand;
	int				iBytes;
	const char*		szWork;
	bool			bCopyExtend;

	psInstruction = pcBlock->masInstructions.Get(uiIndex);
	iBytes = ValueBytes(psInstruction->sType);
	szWork = WorkRegister(psExtend ? psExtend : psInstruction);
	szOperand.Init();
	bCopyExtend = false;
	if ((iBytes < 4) && !NarrowOnly(psInstruction->sResult))
	{
		//cpyx extends memory of the value's width into the 32 bit register.  A load only compared at its own width
		//(NarrowOnly) isn't: CIR65Widths keeps the register narrow for it, often across a whole loop.
		bCopyExtend = CopyExtendOperand(psInstruction->asArguments[0], &szOperand, szWork, pcBlock, uiIndex, szWork);
	}
	else
	{
		MemoryOperand(psInstruction->asArguments[0], &szOperand, (iBytes == 4) ? NULL : szWork);
	}
	if (StringCompare(szWork, "A") == 0)
	{
		msAvailable = IRNone();
	}
	if (iBytes == 8)
	{
		Line("wth A, 64")->AppendNewLine();
		Line("cpy A, ")->Append(szOperand)->AppendNewLine();
		Line("wth A, 32")->AppendNewLine();
	}
	else if (iBytes == 4)
	{
		Line("cpy ")->Append(szWork)->Append(", ")->Append(szOperand)->AppendNewLine();
	}
	else if (bCopyExtend)
	{
		Line("cpyx ")->Append(szWork)->Append(", ")->Append(szOperand)->Append(psExtend ? ":s" : ":")->Append(8 * iBytes)->AppendNewLine();
	}
	else
	{
		Line("wth ")->Append(szWork)->Append(", ")->Append(8 * iBytes)->AppendNewLine();
		Line("cpy ")->Append(szWork)->Append(", ")->Append(szOperand)->AppendNewLine();
		Line("wthx ")->Append(szWork)->Append(psExtend ? ", 32:s" : ", 32")->AppendNewLine();
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
bool CIR65Backend::Store(CIRBlock* pcBlock, size uiIndex)
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
	iBytes = ValueBytes(psInstruction->sType);
	szOperand.Init();
	szValue.Init();
	szValueRegister = NULL;
	bLoad = false;
	if ((iBytes == 4) && SmallImmediate(sValue, -8, 8))
	{
		AppendImmediate(&szValue, sValue, IR65E_Zero);
	}
	else if ((iBytes != 8) && IsInA(sValue))
	{
		szValueRegister = "A";
	}
	else if ((iBytes != 8) && InRegister(sValue))
	{
		szValueRegister = RegisterName(PlaceOf(sValue));
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
		MemoryOperand(psInstruction->asArguments[1], &szOperand, (bLoad || (iBytes != 4)) ? szValueRegister : NULL);
	}
	if (bLoad)
	{
		LoadValue(sValue, "A", IR65E_Zero);
	}

	if (szValueRegister == NULL)
	{
		Line("cpy ")->Append(szOperand)->Append(", ")->Append(szValue)->AppendNewLine();
	}
	else if (iBytes == 4)
	{
		Line("cpy ")->Append(szOperand)->Append(", ")->Append(szValueRegister)->AppendNewLine();
	}
	else if (iBytes == 8)
	{
		Line("wth A, 64")->AppendNewLine();
		Line("cpy ")->Append(szOperand)->Append(", A")->AppendNewLine();
		Line("wth A, 32")->AppendNewLine();
	}
	else if (bCopyExtend)
	{
		//cpyx stores the register's low bits.
		Line("cpyx ")->Append(szOperand)->Append(':')->Append(8 * iBytes)->Append(", ")->Append(szValueRegister)->AppendNewLine();
	}
	else
	{
		Line("wth ")->Append(szValueRegister)->Append(", ")->Append(8 * iBytes)->AppendNewLine();
		Line("cpy ")->Append(szOperand)->Append(", ")->Append(szValueRegister)->AppendNewLine();
		Line("wth ")->Append(szValueRegister)->Append(", 32")->AppendNewLine();
	}
	szValue.Kill();
	szOperand.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The arg instructions before the call (CallArguments) are pushed last
// first (PushArguments), the callee found (CalleeOperand), a thiscall's
// this put in X (LoadRegisterArguments), then jsr and the caller removes
// the arguments and keeps the result (CallerCleanUp).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Call(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psCall;
	size			uiFirst;
	int32			iBytes;
	bool			bThisCall;
	CChars			szCallee;

	psCall = pcBlock->masInstructions.Get(uiIndex);
	uiFirst = CallArguments(pcBlock, uiIndex);
	bThisCall = (psCall->uiFlags & IRF_ThisCall) && (uiFirst < uiIndex);

	iBytes = PushArguments(pcBlock, uiFirst + (bThisCall ? 1 : 0), uiIndex);
	szCallee.Init();
	CalleeOperand(psCall->asArguments[0], bThisCall, &szCallee);
	if (bThisCall)
	{
		LoadRegisterArguments(pcBlock, uiFirst);
	}
	Line("jsr ")->Append(szCallee)->AppendNewLine();
	szCallee.Kill();
	CallerCleanUp(iBytes, psCall);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The first of the arg instructions just before the call uiCall (uiCall if
// it has none).
//
//////////////////////////////////////////////////////////////////////////
size CIR65Backend::CallArguments(CIRBlock* pcBlock, size uiCall)
{
	size	uiFirst;

	uiFirst = uiCall;
	while ((uiFirst > 0) && (pcBlock->masInstructions.Get(uiFirst - 1)->uiOpcode == IRO_Arg))
	{
		uiFirst--;
	}
	return uiFirst;
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes the arg instructions uiFirst to uiEnd - 1, last first, each at its
// own size.  Returns the bytes pushed.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR65Backend::PushArguments(CIRBlock* pcBlock, size uiFirst, size uiEnd)
{
	size			ui;
	int32			iBytes;
	SIRInstruction*	psArg;

	iBytes = 0;
	for (ui = uiEnd; ui > uiFirst; ui--)
	{
		psArg = pcBlock->masInstructions.Get(ui - 1);
		Push(psArg->asArguments[0], psArg->sType);
		iBytes += ValueBytes(psArg->sType);
	}
	return iBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// The callee is jumped to where it is: a register (X only when this doesn't
// go there), or through a pointer in a pseudo register or the frame (jsr
// [n + W]); else it is loaded into A.  Appends it to pszCallee.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::CalleeOperand(SIRReference sCallee, bool bThisCall, CChars* pszCallee)
{
	if (sCallee.IsSymbol())
	{
		pszCallee->Append(mpcModule->GetSymbolName(sCallee.Index()));
	}
	else if (InRegister(sCallee) && ((PlaceOf(sCallee)->iOffset == IR65R_Y) || !bThisCall))
	{
		AppendPlace(pszCallee, PlaceOf(sCallee));
	}
	else if (sCallee.IsTemporary() && !IsInA(sCallee) && IsMemoryHome(PlaceOf(sCallee)) && ((PlaceOf(sCallee)->uiKind == IR65P_Pseudo) || !msFrame.bFrameless))
	{
		AppendPlace(pszCallee, PlaceOf(sCallee));
	}
	else
	{
		LoadValue(sCallee, "A", IR65E_Zero);
		pszCallee->Append('A');
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The arguments a call takes in registers: a thiscall's this (the arg
// instruction uiFirst) in X.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::LoadRegisterArguments(CIRBlock* pcBlock, size uiFirst)
{
	LoadValue(pcBlock->masInstructions.Get(uiFirst)->asArguments[0], "X", IR65E_Zero);
}


//////////////////////////////////////////////////////////////////////////
//
// After a call: the iBytes of arguments are removed and the result (in A)
// goes to its home, or stays in A.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::CallerCleanUp(int32 iBytes, SIRInstruction* psCall)
{
	RemoveArguments(iBytes);
	msAvailable = IRNone();
	StoreResult(psCall->sResult);
}


//////////////////////////////////////////////////////////////////////////
//
// Removes iBytes of arguments a call was given.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::RemoveArguments(int32 iBytes)
{
	if (iBytes > 0)
	{
		Line("add S, ")->Append(iBytes)->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Branches to iLabel when the comparison is bJumpIf.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Compare(SIRInstruction* psInstruction, bool bJumpIf, int32 iLabel)
{
	if (psInstruction->sType.IsFloat())
	{
		return FloatCompare(psInstruction, bJumpIf, iLabel);
	}
	return IntegerCompare(psInstruction, bJumpIf, iLabel);
}


//////////////////////////////////////////////////////////////////////////
//
// cmp first, second for a comparison of 32 bits or less that needs no
// sign extending: the first operand where it is kept (X, [4 + W]) or in A
// or B, the second as an operand that can go with it.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::CompareOperands(SIRReference sFirst, SIRReference sSecond)
{
	CChars	szLeft;
	CChars	szRight;
	bool	bSmall;

	szLeft.Init();
	szRight.Init();
	bSmall = IsInA(sSecond) || InRegister(sSecond) || SmallImmediate(sSecond, -8, 8);

	if (IsInA(sFirst))
	{
		szLeft.Append('A');
	}
	else if (InRegister(sFirst))
	{
		szLeft.Append(RegisterName(PlaceOf(sFirst)));
	}
	else if (bSmall && sFirst.IsTemporary() && (PlaceOf(sFirst)->uiKind == IR65P_Pseudo))
	{
		//Memory goes with a register or -8 to 8.
		AppendPlace(&szLeft, PlaceOf(sFirst));
	}
	else if (IsInA(sSecond))
	{
		LoadValue(sFirst, "B", IR65E_Zero);
		szLeft.Append('B');
	}
	else
	{
		LoadValue(sFirst, "A", IR65E_Zero);
		szLeft.Append('A');
	}

	if (!Operand(sSecond, &szRight, true, IR65E_Zero))
	{
		LoadValue(sSecond, (StringCompare(szLeft.Text(), "B") == 0) ? "A" : "B", IR65E_Zero);
		szRight.Append((StringCompare(szLeft.Text(), "B") == 0) ? 'A' : 'B');
	}
	Line("cmp ")->Append(szLeft)->Append(", ")->Append(szRight)->AppendNewLine();
	szRight.Kill();
	szLeft.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// After cmp X, Y: z is X = Y, c clear is X < Y unsigned and n != v is X < Y
// signed.  > and <= swap the operands (with a constant they were made >=
// and < by AdjustComparisons).  There is no branch on n = v, so it branches
// round a bra.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::IntegerCompare(SIRInstruction* psInstruction, bool bJumpIf, int32 iLabel)
{
	uint16			uiOpcode;
	bool			bSigned;
	bool			bSwap;
	bool			bLess;
	bool			bWide;
	int32			iSkip;
	SIRReference	sLeft;
	SIRReference	sRight;
	int				iBits;

	uiOpcode = psInstruction->uiOpcode;
	if (mbFlagsKnown)
	{
		//The block before compared the same operands.
		Branch(((uiOpcode == IRO_Eq) == bJumpIf) ? "bras z, " : "brac z, ", iLabel);
		return true;
	}
	bSigned = IsIRSignedComparison(uiOpcode);
	bSwap = IsIRSwappedComparison(uiOpcode);
	bWide = IsWide(psInstruction->sType);
	sLeft = psInstruction->asArguments[0];
	sRight = psInstruction->asArguments[1];

	if (bWide || (IsNarrow(psInstruction->sType) && (bSigned || (NarrowOnly(sLeft) && NarrowOnly(sRight)))))
	{
		//Signed i8 and i16 are compared at their own width (the registers narrowed), which needs no extending; so
		//are unsigned ones of values only ever compared, whose extending then goes (CIR65Widths).
		iBits = bWide ? 64 : psInstruction->sType.IntegerBits();
		if (!bWide && (iBits != 1) && InRegister(sLeft) && InRegister(sRight) && (PlaceOf(sLeft)->iOffset != PlaceOf(sRight)->iOffset))
		{
			//Both kept in X and Y: those are narrowed instead.
			Line("wtth (X, Y), ")->Append(iBits)->AppendNewLine();
			Line("cmp ")->Append(RegisterName(PlaceOf(bSwap ? sRight : sLeft)))->Append(", ")->Append(RegisterName(PlaceOf(bSwap ? sLeft : sRight)))->AppendNewLine();
			Line("wtth (X, Y), 32")->AppendNewLine();
		}
		else if (!bWide && (iBits != 1) && !bSwap && InRegister(sLeft) && Immediate(sRight, psInstruction->sType))
		{
			//Kept in X or Y and compared with a constant: the register is narrowed.
			Line("wth ")->Append(RegisterName(PlaceOf(sLeft)))->Append(", ")->Append(iBits)->AppendNewLine();
			Line("cmp ")->Append(RegisterName(PlaceOf(sLeft)))->Append(", ");
			AppendImmediate(mpszOut, sRight, bSigned ? IR65E_Sign : IR65E_Zero);
			mpszOut->AppendNewLine();
			Line("wth ")->Append(RegisterName(PlaceOf(sLeft)))->Append(", 32")->AppendNewLine();
		}
		else
		{
			if (iBits == 1)
			{
				LoadOperands(psInstruction, IR65E_Sign, !bSwap);
				iBits = 32;
			}
			else
			{
				LoadOperands(psInstruction, IR65E_Zero, !bSwap);
			}
			if (iBits != 32)
			{
				Line("wtth (A, B), ")->Append(iBits)->AppendNewLine();
			}
			if (mbRightImmediate)
			{
				Line("cmp A, ");
				AppendImmediate(mpszOut, sRight, bSigned ? IR65E_Sign : IR65E_Zero);
				mpszOut->AppendNewLine();
			}
			else
			{
				Line(bSwap ? "cmp B, A" : "cmp A, B")->AppendNewLine();
			}
			if (iBits != 32)
			{
				Line("wtth (A, B), 32")->AppendNewLine();
			}
		}
	}
	else if (bSwap)
	{
		CompareOperands(sRight, sLeft);
	}
	else
	{
		CompareOperands(sLeft, sRight);
	}

	if ((uiOpcode == IRO_Eq) || (uiOpcode == IRO_Ne))
	{
		Branch(((uiOpcode == IRO_Eq) == bJumpIf) ? "bras z, " : "brac z, ", iLabel);
		return true;
	}

	//slt, ult and the swapped sgt, ugt are less; sge, uge and the swapped sle, ule are greater or equal.
	bLess = ((uiOpcode == IRO_SLt) || (uiOpcode == IRO_ULt) || (uiOpcode == IRO_SGt) || (uiOpcode == IRO_UGt)) == bJumpIf;
	if (!bSigned)
	{
		Branch(bLess ? "brac c, " : "bras c, ", iLabel);
	}
	else if (!bSwap && IsZero(sRight))
	{
		//cmp x, 0 can't overflow, so n alone is x < 0 (x > -1 is x >= 0 by AdjustComparisons; x > 0 is x >= 1).
		Branch(bLess ? "bras n, " : "brac n, ", iLabel);
	}
	else if (bLess)
	{
		Branch("braso (n, v), ", iLabel);
	}
	else
	{
		iSkip = NewLabel();
		Branch("braso (n, v), ", iSkip);
		Branch("bra ", iLabel);
		Label(iSkip);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The runtime's compare gives -1, 0 or 1, or 2 when either is a NaN (so
// every comparison but fne and funordered is false then).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::FloatCompare(SIRInstruction* psInstruction, bool bJumpIf, int32 iLabel)
{
	const char*	szTrue;
	const char*	szFalse;

	Push(psInstruction->asArguments[1], psInstruction->sType);
	Push(psInstruction->asArguments[0], psInstruction->sType);
	CallRuntime(psInstruction->sType.Is(IRTK_F64) ? "SoftDoubleCompare" : "SoftFloatCompare", 2 * ValueBytes(psInstruction->sType));
	msAvailable = IRNone();

	szTrue = "bras z, ";
	szFalse = "brac z, ";
	switch (psInstruction->uiOpcode)
	{
	case IRO_FEq:
		Line("cmp A, 0")->AppendNewLine();
		break;
	case IRO_FNe:
		Line("cmp A, 0")->AppendNewLine();
		szTrue = "brac z, ";
		szFalse = "bras z, ";
		break;
	case IRO_FLt:
		Line("cmp A, -1")->AppendNewLine();
		break;
	case IRO_FGt:
		Line("cmp A, 1")->AppendNewLine();
		break;
	case IRO_FLe:
		//-1 or 0: A + 1 is below 2, unsigned.
		Line("add A, 1")->AppendNewLine();
		Line("cmp A, 2")->AppendNewLine();
		szTrue = "brac c, ";
		szFalse = "bras c, ";
		break;
	case IRO_FGe:
		//0 or 1.
		Line("cmp A, 2")->AppendNewLine();
		szTrue = "brac c, ";
		szFalse = "bras c, ";
		break;
	case IRO_FOrdered:
		Line("cmp A, 2")->AppendNewLine();
		szTrue = "brac z, ";
		szFalse = "bras z, ";
		break;
	default:
		Line("cmp A, 2")->AppendNewLine();
		break;
	}
	Branch(bJumpIf ? szTrue : szFalse, iLabel);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if psNext's lowering takes sValue (its first operand, and not its
// second) from A when the instruction before left it there, so a result
// it alone uses needn't be kept.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::TakesInA(SIRInstruction* psNext, SIRReference sValue)
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
bool CIR65Backend::JumpTakesInA(CIRBlock* pcBlock, SIRInstruction* psFused, SIRReference sValue)
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
bool CIR65Backend::Immediate(SIRReference sValue, SIRType sType)
{
	return sValue.IsConstant() && !IsWide(sType) && !sType.IsFloat();
}


//////////////////////////////////////////////////////////////////////////
//
// A two operand instruction's operands: the first in A, the second in B, or
// left as a constant when bImmediate and it is one (mbRightImmediate).  A
// second operand the instruction before left in A is copied to B first.
// For 64 bit values, signed narrow comparisons, mul and the divisions.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::LoadOperands(SIRInstruction* psInstruction, EIR65Extend eExtend, bool bImmediate)
{
	SIRReference	sLeft;
	SIRReference	sRight;
	SIRType			sType;

	sLeft = psInstruction->asArguments[0];
	sRight = psInstruction->asArguments[1];
	mbRightImmediate = false;
	if (sRight.IsNone())
	{
		LoadValue(sLeft, "A", eExtend);
		return;
	}
	sType = mpcFunction->TypeOf(sRight);
	if (bImmediate && Immediate(sRight, sType))
	{
		LoadValue(sLeft, "A", eExtend);
		mbRightImmediate = true;
		return;
	}
	if (IsInA(sRight) && !sLeft.Equals(sRight))
	{
		if (IsWide(sType))
		{
			Line("wtth (A, B), 64")->AppendNewLine();
			Line("cpy B, A")->AppendNewLine();
			Line("wtth (A, B), 32")->AppendNewLine();
		}
		else
		{
			Line("cpy B, A")->AppendNewLine();
		}
		msAvailable = IRNone();
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
bool CIR65Backend::TakesInB(SIRInstruction* psNext, SIRReference sValue)
{
	return psNext->asArguments[1].Equals(sValue) && !psNext->asArguments[0].Equals(sValue) && (IR65OpcodeFlags(psNext->uiOpcode) & IR65F_TAKES_B);
}
