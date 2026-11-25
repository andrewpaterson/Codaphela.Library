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


//The 65C64 back end's operands: places and immediates as operand text, values loaded into and stored from
//registers (extended or truncated), memory operands through pointers and folded addresses, and values pushed.


//////////////////////////////////////////////////////////////////////////
//
// A home as an operand: X, [n + W] or [n + V] ([n + S] in a function
// without a frame).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Selector::AppendPlace(CChars* psz, SIR65Place* psPlace)
{
	switch (psPlace->uiKind)
	{
	case IR65P_Register:
		psz->Append(mpcBackend->mcPlaces.RegisterName(psPlace));
		break;
	case IR65P_Pseudo:
		psz->Append('[')->Append(psPlace->iOffset)->Append(" + W]");
		break;
	case IR65P_Frame:
	case IR65P_FrameAddress:
		psz->Append('[')->Append(1 + psPlace->iOffset)->Append(" + ")->Append(mpcBackend->msFrame.cRegister)->Append(']');
		break;
	case IR65P_Argument:
	case IR65P_ArgumentAddress:
		psz->Append('[')->Append(mpcBackend->msFrame.iArguments + psPlace->iOffset)->Append(" + ")->Append(mpcBackend->msFrame.cRegister)->Append(']');
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A constant as an immediate, extended to 32 bits as eExtend says.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Selector::AppendImmediate(CChars* psz, SIRReference sConstant, EIR65Extend eExtend)
{
	SIRConstant*	psConstant;
	int64			lliValue;

	psConstant = mpcBackend->mpcFunction->GetConstant(sConstant);
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
bool CIR65Selector::Operand(SIRReference sValue, CChars* pszOperand, bool bArithmetic, EIR65Extend eExtend)
{
	SIRType		sType;
	SIR65Place*	psPlace;

	if (mpcBackend->mcPlaces.IsDeferred(sValue))
	{
		//add, sub, cmp, shl and shr can't address memory with S.
		if (bArithmetic && mpcBackend->mcPlaces.IsFrameOperand(&mpcBackend->mszDeferred))
		{
			return false;
		}
		pszOperand->Append(mpcBackend->mszDeferred);
		return true;
	}
	sType = mpcBackend->mpcFunction->TypeOf(sValue);
	if (mpcBackend->IsWide(sType))
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
		pszOperand->Append(mpcBackend->mpcModule->GetSymbolName(sValue.Index()));
		return true;
	}
	if (!sValue.IsTemporary() || ((eExtend == IR65E_Sign) && mpcBackend->IsNarrow(sType)))
	{
		return false;
	}
	if (mpcBackend->mcPlaces.IsInA(sValue))
	{
		pszOperand->Append('A');
		return true;
	}
	psPlace = mpcBackend->mcPlaces.PlaceOf(sValue);
	switch (psPlace->uiKind)
	{
	case IR65P_Register:
	case IR65P_Pseudo:
		AppendPlace(pszOperand, psPlace);
		return true;
	case IR65P_Frame:
		if (bArithmetic && (mpcBackend->msFrame.cRegister == 'S'))
		{
			return false;
		}
		AppendPlace(pszOperand, psPlace);
		return true;
	case IR65P_Argument:
		if ((bArithmetic && (mpcBackend->msFrame.cRegister == 'S')) || (psPlace->iBytes != 4))
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
bool CIR65Selector::SmallImmediate(SIRReference sValue, int iMin, int iMax)
{
	SIRConstant*	psConstant;
	int64			lliValue;

	if (!sValue.IsConstant())
	{
		return false;
	}
	psConstant = mpcBackend->mpcFunction->GetConstant(sValue);
	if (mpcBackend->IsWide(psConstant->sType))
	{
		return false;
	}
	lliValue = mpcBackend->IsNarrow(psConstant->sType) ? (int64)psConstant->ulliBits : (int64)(int32)psConstant->ulliBits;
	return (lliValue >= iMin) && (lliValue <= iMax);
}


//////////////////////////////////////////////////////////////////////////
//
// True if no temporary kept in X or Y (iRegister) is live during the block's
// instruction uiIndex, so it can hold something else for that instruction.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::IsRegisterFree(int iRegister, CIRBlock* pcBlock, size uiIndex)
{
	SIR65Home*	psHome;
	SIRRange*	psRange;
	int32		iPosition;
	size		ui;

	iPosition = mpcBackend->mcLiveness.Position(pcBlock, uiIndex);
	psHome = mpcBackend->masHomes.Get(iRegister);
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
const char* CIR65Selector::FreeAddressRegister(CIRBlock* pcBlock, size uiIndex, const char* szPrefer)
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
bool CIR65Selector::CopyExtendOperand(SIRReference sAddress, CChars* pszOperand, const char* szWidened, CIRBlock* pcBlock, size uiIndex, const char* szPrefer)
{
	size		uiMark;
	const char*	szFree;

	uiMark = mpcBackend->CodeLines();
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
	mpcBackend->CutCode(uiMark);
	pszOperand->Kill();
	pszOperand->Init();
	mpcBackend->mszAddress = szFree;
	MemoryOperand(sAddress, pszOperand, NULL);
	mpcBackend->mszAddress = "B";
	if (CIR65Code::IsCopyExtendAddress(pszOperand))
	{
		return true;
	}

	mpcBackend->CutCode(uiMark);
	pszOperand->Kill();
	pszOperand->Init();
	MemoryOperand(sAddress, pszOperand, szWidened);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// szRegister where a value is read or written at another width than 32
// bits (wth, wthx and cpyx): A for V and W, which have only the one.
//
//////////////////////////////////////////////////////////////////////////
const char* CIR65Selector::WidthRegister(const char* szRegister)
{
	return ((szRegister[0] == 'V') || (szRegister[0] == 'W')) ? "A" : szRegister;
}


//////////////////////////////////////////////////////////////////////////
//
// The register to work out an instruction's result in: its own if it is
// kept in X, Y, V or W, otherwise A (B is what the code works in besides A).
//
//////////////////////////////////////////////////////////////////////////
const char* CIR65Selector::WorkRegister(SIRInstruction* psInstruction)
{
	SIR65Place*	psPlace;

	if (!psInstruction->sResult.IsTemporary() || mpcBackend->IsWide(psInstruction->sType))
	{
		return "A";
	}
	psPlace = mpcBackend->mcPlaces.PlaceOf(psInstruction->sResult);
	return ((psPlace->uiKind == IR65P_Register) && (psPlace->iOffset != IR65R_B)) ? mpcBackend->mcPlaces.RegisterName(psPlace) : "A";
}


//////////////////////////////////////////////////////////////////////////
//
// Sign extends a narrow integer held zero extended (flip and subtract the
// sign bit), when eExtend says so.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Selector::Extend(const char* szRegister, SIRType sType, EIR65Extend eExtend)
{
	int		iBits;
	int64	lliSign;

	iBits = sType.IntegerBits();
	if ((eExtend == IR65E_Sign) && (iBits >= 1) && (iBits < 32))
	{
		lliSign = (int64)1 << (iBits - 1);
		mpcBackend->Line(IR65M_Xor)->Append(szRegister)->Append(", ")->Append(lliSign);
		mpcBackend->Line(IR65M_Sub)->Append(szRegister)->Append(", ")->Append(lliSign);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps the low bits of a narrow integer, zero extended.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Selector::Truncate(const char* szRegister, SIRType sType)
{
	if (mpcBackend->IsNarrow(sType))
	{
		mpcBackend->Line(IR65M_And)->Append(szRegister)->Append(", ")->Append((((int64)1) << sType.IntegerBits()) - 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Selector::LoadConstant(const char* szRegister, SIRType sType, uint64 ulliBits, EIR65Extend eExtend)
{
	int64	lliValue;

	if (mpcBackend->IsWide(sType))
	{
		mpcBackend->Line(IR65M_Wth)->Append(szRegister)->Append(", 64");
		mpcBackend->Line(IR65M_Cpy)->Append(szRegister)->Append(", ")->Append((int64)ulliBits);
		mpcBackend->Line(IR65M_Wth)->Append(szRegister)->Append(", 32");
		return;
	}
	lliValue = (eExtend == IR65E_Sign) ? IRSignExtend(sType, ulliBits) : (int64)ulliBits;
	mpcBackend->Line(IR65M_Cpy)->Append(szRegister)->Append(", ")->Append((int64)(int32)lliValue);
}


//////////////////////////////////////////////////////////////////////////
//
// Loads a home into a register, narrow integers extended to 32 bits as
// eExtend says, 64 bit values with the register 64 bits wide (it is left 32
// bits wide, holding all 64).  A parameter pushed narrow is read at its own
// width and widened.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Selector::LoadPlace(SIR65Place* psPlace, SIRType sType, const char* szRegister, EIR65Extend eExtend)
{
	CChars*	psz;

	if (psPlace->uiKind == IR65P_Register)
	{
		if (StringCompare(mpcBackend->mcPlaces.RegisterName(psPlace), szRegister) != 0)
		{
			mpcBackend->Line(IR65M_Cpy)->Append(szRegister)->Append(", ")->Append(mpcBackend->mcPlaces.RegisterName(psPlace));
		}
		Extend(szRegister, sType, eExtend);
		return;
	}
	if (mpcBackend->IsWide(sType))
	{
		mpcBackend->Line(IR65M_Wth)->Append(szRegister)->Append(", 64");
		psz = mpcBackend->Line(IR65M_Cpy)->Append(szRegister)->Append(", ");
		AppendPlace(psz, psPlace);
		mpcBackend->Line(IR65M_Wth)->Append(szRegister)->Append(", 32");
		return;
	}
	if ((psPlace->uiKind == IR65P_Argument) && (psPlace->iBytes < 4) && (mpcBackend->msFrame.cRegister == 'V'))
	{
		//cpyx extends the argument into the 32 bit register (it can't address with S).
		psz = mpcBackend->Line(IR65M_Cpyx)->Append(szRegister)->Append(", ");
		AppendPlace(psz, psPlace);
		psz->Append(((eExtend == IR65E_Sign) && mpcBackend->IsNarrow(sType)) ? ":s" : ":")->Append(8 * psPlace->iBytes);
		return;
	}
	if ((psPlace->uiKind == IR65P_Argument) && (psPlace->iBytes < 4))
	{
		mpcBackend->Line(IR65M_Wth)->Append(szRegister)->Append(", ")->Append(8 * psPlace->iBytes);
		psz = mpcBackend->Line(IR65M_Cpy)->Append(szRegister)->Append(", ");
		AppendPlace(psz, psPlace);
		mpcBackend->Line(IR65M_Wthx)->Append(szRegister)->Append(((eExtend == IR65E_Sign) && mpcBackend->IsNarrow(sType)) ? ", 32:s" : ", 32");
		return;
	}
	psz = mpcBackend->Line(IR65M_Cpy)->Append(szRegister)->Append(", ");
	AppendPlace(psz, psPlace);
	Extend(szRegister, sType, eExtend);
}


//////////////////////////////////////////////////////////////////////////
//
// Stores a register to a home.  Narrow values are already zero extended.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Selector::StorePlace(SIR65Place* psPlace, SIRType sType, const char* szRegister)
{
	CChars*	psz;

	if (psPlace->uiKind == IR65P_Register)
	{
		if (StringCompare(mpcBackend->mcPlaces.RegisterName(psPlace), szRegister) != 0)
		{
			mpcBackend->Line(IR65M_Cpy)->Append(mpcBackend->mcPlaces.RegisterName(psPlace))->Append(", ")->Append(szRegister);
		}
		return;
	}
	if (mpcBackend->IsWide(sType))
	{
		mpcBackend->Line(IR65M_Wth)->Append(szRegister)->Append(", 64");
	}
	psz = mpcBackend->Line(IR65M_Cpy);
	AppendPlace(psz, psPlace);
	psz->Append(", ")->Append(szRegister);
	if (mpcBackend->IsWide(sType))
	{
		mpcBackend->Line(IR65M_Wth)->Append(szRegister)->Append(", 32");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Loads a value into a register.  Only a result's own register is loaded
// in place (its operand dies there, so extending it is fine).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Selector::LoadValue(SIRReference sValue, const char* szRegister, EIR65Extend eExtend)
{
	SIR65Place*	psPlace;
	bool		bA;

	bA = StringCompare(szRegister, "A") == 0;
	if (mpcBackend->mcPlaces.IsDeferred(sValue))
	{
		if (bA)
		{
			mpcBackend->msAvailable = IRNone();
		}
		mpcBackend->Line(IR65M_Cpy)->Append(szRegister)->Append(", ")->Append(mpcBackend->mszDeferred);
		return;
	}
	if (mpcBackend->mcPlaces.IsInA(sValue))
	{
		if (!bA && mpcBackend->IsWide(mpcBackend->mpcFunction->TypeOf(sValue)))
		{
			mpcBackend->Line(IR65M_Wtth, "(A, ")->Append(szRegister)->Append("), 64");
			mpcBackend->Line(IR65M_Cpy)->Append(szRegister)->Append(", A");
			mpcBackend->Line(IR65M_Wtth, "(A, ")->Append(szRegister)->Append("), 32");
			return;
		}
		if (!bA)
		{
			mpcBackend->Line(IR65M_Cpy)->Append(szRegister)->Append(", A");
		}
		else if ((eExtend == IR65E_Sign) && mpcBackend->IsNarrow(mpcBackend->mpcFunction->TypeOf(sValue)))
		{
			mpcBackend->msAvailable = IRNone();
		}
		Extend(szRegister, mpcBackend->mpcFunction->TypeOf(sValue), eExtend);
		return;
	}
	if (bA)
	{
		mpcBackend->msAvailable = IRNone();
	}

	switch (sValue.Kind())
	{
	case IRRK_Constant:
		LoadConstant(szRegister, mpcBackend->mpcFunction->GetConstant(sValue)->sType, mpcBackend->mpcFunction->GetConstant(sValue)->ulliBits, eExtend);
		return;
	case IRRK_Symbol:
		mpcBackend->Line(IR65M_Cpy)->Append(szRegister)->Append(", ")->Append(mpcBackend->mpcModule->GetSymbolName(sValue.Index()));
		return;
	case IRRK_Temporary:
		break;
	default:
		mpcBackend->Error("Operand can't be loaded.");
		return;
	}

	psPlace = mpcBackend->mcPlaces.PlaceOf(sValue);
	switch (psPlace->uiKind)
	{
	case IR65P_FrameAddress:
	case IR65P_ArgumentAddress:
		mpcBackend->Line(IR65M_Cpy)->Append(szRegister)->Append(", ")->Append(mpcBackend->msFrame.cRegister);
		mpcBackend->Line(IR65M_Add)->Append(szRegister)->Append(", ")->Append(((psPlace->uiKind == IR65P_ArgumentAddress) ? mpcBackend->msFrame.iArguments : 1) + psPlace->iOffset);
		return;
	case IR65P_Register:
	case IR65P_Pseudo:
	case IR65P_Frame:
	case IR65P_Argument:
		LoadPlace(psPlace, mpcBackend->mpcFunction->GetTemporaryType(sValue), szRegister, eExtend);
		return;
	default:
		mpcBackend->Error("Temporary isn't where it was left.");
		return;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps a result worked out in szRegister (truncated if narrow, unless
// bExtended says it is already or mcMost that the instruction's value
// fits its type) in its home, or in A for the next instruction.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Selector::StoreResult(SIRReference sResult, const char* szRegister, bool bExtended)
{
	SIR65Place*	psPlace;
	SIRType		sType;
	bool		bA;

	if (!sResult.IsTemporary())
	{
		return;
	}
	psPlace = mpcBackend->mcPlaces.PlaceOf(sResult);
	sType = mpcBackend->mpcFunction->GetTemporaryType(sResult);
	bA = StringCompare(szRegister, "A") == 0;
	if (psPlace->bInA)
	{
		if (!bA)
		{
			mpcBackend->Line(IR65M_Cpy, "A, ")->Append(szRegister);
		}
		if (!bExtended && !mpcBackend->mcMost.FitsType(sResult))
		{
			Truncate("A", sType);
		}
		mpcBackend->msInA = sResult;
		return;
	}
	if (psPlace->uiKind == IR65P_None)
	{
		//Not used.
		return;
	}
	if ((psPlace->uiKind != IR65P_Register) && (psPlace->uiKind != IR65P_Pseudo) && (psPlace->uiKind != IR65P_Frame))
	{
		mpcBackend->Error("Result has no home.");
		return;
	}
	if (!bExtended && !mpcBackend->mcMost.FitsType(sResult))
	{
		Truncate(szRegister, sType);
	}
	StorePlace(psPlace, sType, szRegister);
	if (bA)
	{
		mpcBackend->msInA = sResult;
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
void CIR65Selector::MemoryOperand(SIRReference sAddress, CChars* pszOperand, const char* szWidened)
{
	SIR65Place*	psPlace;

	if (sAddress.IsSymbol())
	{
		pszOperand->Append('[')->Append(mpcBackend->mpcModule->GetSymbolName(sAddress.Index()))->Append(']');
		return;
	}
	if (sAddress.IsConstant())
	{
		pszOperand->Append('[')->Append((int64)(uint32)mpcBackend->mpcFunction->GetConstant(sAddress)->ulliBits)->Append(']');
		return;
	}
	if (sAddress.IsTemporary() && (mpcBackend->mcPlaces.PlaceOf(sAddress)->uiKind == IR65P_Folded))
	{
		FoldedOperand(mpcBackend->mcPlaces.PlaceOf(sAddress), pszOperand, szWidened);
		return;
	}
	if (sAddress.IsTemporary())
	{
		if (mpcBackend->mcPlaces.IsInA(sAddress) && !mpcBackend->mcPlaces.InRegister(sAddress))
		{
			if (((szWidened == NULL) || (StringCompare(szWidened, "A") != 0)) && (StringCompare(mpcBackend->mszAddress, "B") == 0))
			{
				pszOperand->Append("[A]");
				return;
			}
		}
		else
		{
			psPlace = mpcBackend->mcPlaces.PlaceOf(sAddress);
			switch (psPlace->uiKind)
			{
			case IR65P_FrameAddress:
			case IR65P_ArgumentAddress:
				AppendPlace(pszOperand, psPlace);
				return;
			case IR65P_Register:
				//[W] too for a cpy (mpcBackend->mbCopyAddress): cpy addresses memory with any register, and [W] with no offset
				//isn't how a pseudo register is written.
				if ((mpcBackend->mcPlaces.IsAddressRegister(psPlace) || mpcBackend->mbCopyAddress) && ((szWidened == NULL) || (StringCompare(szWidened, mpcBackend->mcPlaces.RegisterName(psPlace)) != 0)))
				{
					pszOperand->Append('[')->Append(mpcBackend->mcPlaces.RegisterName(psPlace))->Append(']');
					return;
				}
				break;
			default:
				if (mpcBackend->mcPlaces.IsMemoryHome(psPlace))
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
	LoadValue(sAddress, mpcBackend->mszAddress, IR65E_Zero);
	pszOperand->Append('[')->Append(mpcBackend->mszAddress)->Append(']');
}


//////////////////////////////////////////////////////////////////////////
//
// A folded ptradd's address: [symbol + offset], [n + V] for a slot or an
// argument, [offset + R] with the base in a register (not szWidened) or B.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Selector::FoldedOperand(SIR65Place* psFolded, CChars* pszOperand, const char* szWidened)
{
	SIRReference	sBase;
	SIR65Place*		psBase;
	SIR65Place		sAddress;
	const char*		szRegister;

	sBase = psFolded->sBase;
	if (sBase.IsSymbol())
	{
		pszOperand->Append('[')->Append(mpcBackend->mpcModule->GetSymbolName(sBase.Index()));
		if (psFolded->iOffset != 0)
		{
			pszOperand->Append((psFolded->iOffset < 0) ? " - " : " + ")->Append((int64)((psFolded->iOffset < 0) ? -(int64)psFolded->iOffset : psFolded->iOffset));
		}
		pszOperand->Append(']');
		return;
	}
	if (sBase.IsConstant())
	{
		pszOperand->Append('[')->Append((int64)(uint32)(mpcBackend->mpcFunction->GetConstant(sBase)->ulliBits + psFolded->iOffset))->Append(']');
		return;
	}

	szRegister = NULL;
	if (mpcBackend->mcPlaces.IsInA(sBase))
	{
		szRegister = "A";
	}
	else if (sBase.IsTemporary())
	{
		psBase = mpcBackend->mcPlaces.PlaceOf(sBase);
		if ((psBase->uiKind == IR65P_FrameAddress) || (psBase->uiKind == IR65P_ArgumentAddress))
		{
			sAddress = *psBase;
			sAddress.iOffset += psFolded->iOffset;
			AppendPlace(pszOperand, &sAddress);
			return;
		}
		if (mpcBackend->mcPlaces.IsAddressRegister(psBase))
		{
			szRegister = mpcBackend->mcPlaces.RegisterName(psBase);
		}
	}
	if ((szRegister == NULL) || (szWidened && (StringCompare(szRegister, szWidened) == 0)) || ((StringCompare(szRegister, "A") == 0) && (StringCompare(mpcBackend->mszAddress, "B") != 0)))
	{
		LoadValue(sBase, mpcBackend->mszAddress, IR65E_Zero);
		szRegister = mpcBackend->mszAddress;
	}
	pszOperand->Append('[')->Append(psFolded->iOffset)->Append(" + ")->Append(szRegister)->Append(']');
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes a value at the size of sType: straight from its register, memory
// or as an immediate when it can be, otherwise from A.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Selector::Push(SIRReference sValue, SIRType sType)
{
	int			iBytes;
	CChars		szOperand;
	SIR65Place*	psPlace;
	const char*	szRegister;

	iBytes = mpcBackend->ValueBytes(sType);
	if (iBytes == 8)
	{
		LoadValue(sValue, "A", IR65E_Zero);
		mpcBackend->Line(IR65M_Wth, "A, 64");
		mpcBackend->Line(IR65M_Psh, "A");
		mpcBackend->Line(IR65M_Wth, "A, 32");
		return;
	}
	if (sValue.IsConstant())
	{
		szOperand.Init();
		AppendImmediate(&szOperand, sValue, IR65E_Zero);
		mpcBackend->Line(IR65M_Psh)->Append(szOperand)->Append(':')->Append(8 * iBytes);
		szOperand.Kill();
		return;
	}

	szRegister = NULL;
	if (mpcBackend->mcPlaces.IsInA(sValue))
	{
		szRegister = "A";
	}
	else if (mpcBackend->mcPlaces.InRegister(sValue) && ((iBytes == 4) || mpcBackend->mcPlaces.HasWidth(mpcBackend->mcPlaces.PlaceOf(sValue))))
	{
		szRegister = mpcBackend->mcPlaces.RegisterName(mpcBackend->mcPlaces.PlaceOf(sValue));
	}
	else if ((iBytes == 4) && sValue.IsTemporary())
	{
		psPlace = mpcBackend->mcPlaces.PlaceOf(sValue);
		if (mpcBackend->mcPlaces.IsMemoryHome(psPlace))
		{
			szOperand.Init();
			AppendPlace(&szOperand, psPlace);
			mpcBackend->Line(IR65M_Psh)->Append(szOperand);
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
		mpcBackend->Line(IR65M_Psh)->Append(szRegister);
		return;
	}
	mpcBackend->Line(IR65M_Wth)->Append(szRegister)->Append(", ")->Append(8 * iBytes);
	mpcBackend->Line(IR65M_Psh)->Append(szRegister);
	mpcBackend->Line(IR65M_Wth)->Append(szRegister)->Append(", 32");
}


//////////////////////////////////////////////////////////////////////////
//
// An address's memory operand when it needs no instructions and doesn't use
// A or B; cRegister is set to X or Y if it uses one.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::PureMemoryOperand(SIRReference sAddress, CChars* pszOperand, char* pcRegister)
{
	SIR65Place*		psPlace;
	SIR65Place*		psBase;
	SIRReference	sBase;

	if (sAddress.IsSymbol() || sAddress.IsConstant())
	{
		MemoryOperand(sAddress, pszOperand, NULL);
		return true;
	}
	if (!sAddress.IsTemporary() || mpcBackend->mcPlaces.IsInA(sAddress))
	{
		return false;
	}
	psPlace = mpcBackend->mcPlaces.PlaceOf(sAddress);
	switch (psPlace->uiKind)
	{
	case IR65P_Folded:
		sBase = psPlace->sBase;
		if (sBase.IsTemporary())
		{
			if (mpcBackend->mcPlaces.IsInA(sBase))
			{
				return false;
			}
			psBase = mpcBackend->mcPlaces.PlaceOf(sBase);
			if (mpcBackend->mcPlaces.IsAddressRegister(psBase))
			{
				*pcRegister = mpcBackend->mcPlaces.RegisterName(psBase)[0];
			}
			else if ((psBase->uiKind != IR65P_FrameAddress) && (psBase->uiKind != IR65P_ArgumentAddress))
			{
				return false;
			}
		}
		FoldedOperand(psPlace, pszOperand, NULL);
		return true;
	case IR65P_Register:
		if (!mpcBackend->mcPlaces.IsAddressRegister(psPlace))
		{
			return false;
		}
		*pcRegister = mpcBackend->mcPlaces.RegisterName(psPlace)[0];
		MemoryOperand(sAddress, pszOperand, NULL);
		return true;
	case IR65P_FrameAddress:
	case IR65P_ArgumentAddress:
		MemoryOperand(sAddress, pszOperand, NULL);
		return true;
	default:
		if (!mpcBackend->mcPlaces.IsMemoryHome(psPlace))
		{
			return false;
		}
		MemoryOperand(sAddress, pszOperand, NULL);
		return true;
	}
}
