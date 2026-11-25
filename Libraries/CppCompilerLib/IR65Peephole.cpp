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
#include "IR65Peephole.h"


#define IR65_PEEPHOLE_ROUNDS	8		//The most times the jumps are looked at again after some changed.
#define IR65_PEEPHOLE_VALUES	16384	//The most operands followed in one function.
#define IR65_NAME_S				0x10	//SIR65Value uiNames: S, V and W after A, B, X and Y.
#define IR65_NAME_V				0x20
#define IR65_NAME_W				0x40


//////////////////////////////////////////////////////////////////////////
//
// The SIR65Value uiNames bit of a register named by c (A, B, X, Y, S, V
// or W); 0 for anything else.
//
//////////////////////////////////////////////////////////////////////////
static uint8 IR65PeepholeName(char c)
{
	switch (c)
	{
	case 'A':	return IR65_REGISTER_BIT(IR65CR_A);
	case 'B':	return IR65_REGISTER_BIT(IR65CR_B);
	case 'X':	return IR65_REGISTER_BIT(IR65CR_X);
	case 'Y':	return IR65_REGISTER_BIT(IR65CR_Y);
	case 'S':	return IR65_NAME_S;
	case 'V':	return IR65_NAME_V;
	case 'W':	return IR65_NAME_W;
	default:	return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The length of a memory operand without cpyx's width (":8" in [8 + W]:8).
//
//////////////////////////////////////////////////////////////////////////
static size IR65PeepholeWithoutWidth(const char* szOperand, size uiLength)
{
	size	ui;

	if ((uiLength == 0) || (szOperand[0] != '['))
	{
		return uiLength;
	}
	for (ui = uiLength; ui > 0; ui--)
	{
		if (szOperand[ui - 1] == ']')
		{
			return ui;
		}
	}
	return uiLength;
}


//////////////////////////////////////////////////////////////////////////
//
// Finds an instruction's two operands ("[8 + W], A", CIR65Code::
// OperandsText): the first starts at *pszFirst, the second (if there is
// one) at *pszSecond, and their lengths.
//
//////////////////////////////////////////////////////////////////////////
static void IR65PeepholeOperands(CIR65Code* pcCode, size uiLine, const char** pszFirst, size* puiFirst, const char** pszSecond, size* puiSecond)
{
	const char*	szComma;
	size		uiLength;

	*pszFirst = pcCode->OperandsText(uiLine, &uiLength);
	szComma = strstr(*pszFirst, ", ");
	if (szComma != NULL)
	{
		*puiFirst = szComma - *pszFirst;
		*pszSecond = szComma + 2;
		*puiSecond = strlen(*pszSecond);
	}
	else
	{
		*puiFirst = strlen(*pszFirst);
		*pszSecond = NULL;
		*puiSecond = 0;
	}
	*puiFirst = IR65PeepholeWithoutWidth(*pszFirst, *puiFirst);
	if (*pszSecond != NULL)
	{
		*puiSecond = IR65PeepholeWithoutWidth(*pszSecond, *puiSecond);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Peephole::Init(void)
{
	mpcCode = NULL;
	maszValues.Init();
	mmValues.Init();
	masValues.Init();
	masStates.Init();
	maiCopied.Init();
	mauiDemand.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Peephole::Kill(void)
{
	mauiDemand.Kill();
	maiCopied.Kill();
	masStates.Kill();
	masValues.Kill();
	mmValues.Kill();
	maszValues.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Tidies pcCode's copies and jumps; true if it changed anything.  Code
// with anything the decoder doesn't know is left as it is.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Peephole::Run(CIR65Code* pcCode, bool bHomeV)
{
	int		iRound;
	bool	bChanged;
	bool	bAny;

	mbHomeV = bHomeV;
	mpcCode = pcCode;
	if (!mpcCode->Analyse(true))
	{
		return false;
	}

	bAny = RemoveCopies();
	for (iRound = 0; iRound < IR65_PEEPHOLE_ROUNDS; iRound++)
	{
		bChanged = IRThreadJumps(mpcCode);
		bChanged |= ReturnDirectly();
		bChanged |= IRInvertBranches(mpcCode);
		bChanged |= IRRemoveBranchesToNext(mpcCode);
		bChanged |= IRRemoveUnreached(mpcCode);
		if (!bChanged)
		{
			break;
		}
		bAny = true;
	}
	return bAny;
}


//////////////////////////////////////////////////////////////////////////
//
// The pass on pszCode from uiStart (its label first), as a whole function:
// its code is written again if it could be read.  For the tests.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Peephole::Optimise(CChars* pszCode, size uiStart, bool bHomeV)
{
	CIR65Code	cCode;

	cCode.Init();
	cCode.SetText(pszCode, uiStart);
	Run(&cCode, bHomeV);
	if (cCode.IsLoaded())
	{
		pszCode->SetLength(uiStart);
		cCode.Write(pszCode);
	}
	cCode.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Takes out each cpy into A, B, X or Y (at 32 bits) of the value it
// already holds, or of a register that was copied from it and that it
// hasn't changed since, when the n and z it sets aren't read.  And each
// cpy W, 0 where W is 0 already (one is written before every mul and div:
// CIR65Backend::Multiply).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Peephole::RemoveCopies(void)
{
	size			ui;
	size			uiNum;
	SIR65CodeLine*	psLine;
	SIR65Values*	psState;
	int16			iCopied;
	bool			bChanged;

	ComputeCopied();
	ComputeStates();
	mpcCode->ComputeFlagDemand(&mauiDemand);

	bChanged = false;
	uiNum = mpcCode->NumLines();
	for (ui = 0; ui < uiNum; ui++)
	{
		psLine = mpcCode->GetLine(ui);
		psState = masStates.Get(ui);
		iCopied = *maiCopied.Get(ui);
		if ((psState->aiValues[0] != IR65_VALUE_UNSET) && psState->bZeroW && !(*mauiDemand.Get(ui) & (IR65_FLAG_N | IR65_FLAG_Z)) && mpcCode->IsWith(ui, IR65M_Cpy, "W, 0"))
		{
			mpcCode->Remove(ui);
			bChanged = true;
			continue;
		}
		if ((iCopied < 0) || (psLine->iFirst < 0) || (psState->aiValues[0] == IR65_VALUE_UNSET) || (*mauiDemand.Get(ui) & (IR65_FLAG_N | IR65_FLAG_Z)))
		{
			continue;
		}
		iCopied = Canonical(psState, iCopied);
		if (((psState->aiValues[psLine->iFirst] >= 0) && (psState->aiValues[psLine->iFirst] == iCopied)) ||
			((maszValues.Get(iCopied)->Length() == 1) && (masValues.Get(iCopied)->uiNames == (uint8)(1 << psLine->iFirst))))
		{
			//The register holds the value already; or the value is one copied from the register itself, which
			//hasn't changed since (cpy X, A then cpy A, X: a parameter that came in A, kept in X).
			mpcCode->Remove(ui);
			bChanged = true;
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// The index of an operand a register can hold: A, B, X or Y, a number or
// a symbol, or the function's own memory.  IR65_VALUE_UNKNOWN for any
// other (memory through a pointer or at a symbol may be volatile).
//
//////////////////////////////////////////////////////////////////////////
int16 CIR65Peephole::AddValue(const char* szOperand, size uiLength)
{
	size		ui;
	SIR65Value	sValue;
	CChars		szValue;
	int32		iValue;
	int32*		piValue;
	char		c;

	memset(&sValue, 0, sizeof(SIR65Value));
	if (uiLength == 0)
	{
		return IR65_VALUE_UNKNOWN;
	}
	if ((uiLength == 1) && (IR65PeepholeName(szOperand[0]) != 0))
	{
		//A, B, X or Y (not S, V or W).
		sValue.uiNames = IR65PeepholeName(szOperand[0]);
		if ((sValue.uiNames & IR65_CODE_ALL) == 0)
		{
			return IR65_VALUE_UNKNOWN;
		}
	}
	else if (!CIR65Code::IsOwnMemory(szOperand, uiLength, mbHomeV, &sValue.uiBase, &sValue.iOffset))
	{
		//A number or a symbol: letters, digits, _ and a leading -.
		for (ui = 0; ui < uiLength; ui++)
		{
			c = szOperand[ui];
			if (!(((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9')) || (c == '_') || ((c == '-') && (ui == 0))))
			{
				return IR65_VALUE_UNKNOWN;
			}
		}
	}
	else
	{
		sValue.uiNames = IR65PeepholeName((char)sValue.uiBase);
	}

	piValue = mmValues.Get(szOperand, (size)uiLength);
	iValue = piValue ? *piValue : -1;
	if ((iValue == -1) && (maszValues.NumElements() < IR65_PEEPHOLE_VALUES))
	{
		iValue = (int32)maszValues.NumElements();
		szValue.Init()->AppendSubString(szOperand, uiLength);
		maszValues.Add(szValue.Text());
		szValue.Kill();
		masValues.Add(&sValue);
		*mmValues.FindOrAdd(szOperand, (size)uiLength, NULL) = iValue;
	}
	return (iValue == -1) ? IR65_VALUE_UNKNOWN : (int16)iValue;
}


//////////////////////////////////////////////////////////////////////////
//
// The value each line copies: a cpy into A, B, X or Y at 32 bits copies
// its second operand, a store of one of them at 32 bits to the function's
// own memory copies the register into it.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Peephole::ComputeCopied(void)
{
	size			ui;
	size			uiNum;
	SIR65CodeLine*	psLine;
	SIR65Widths*	psWidths;
	const char*		szFirst;
	const char*		szSecond;
	size			uiFirst;
	size			uiSecond;
	int16			iCopied;
	uint8			uiBase;
	int32			iOffset;

	maszValues.ReInit();
	mmValues.ReInit();
	masValues.ReInit();
	maiCopied.ReInit();
	uiNum = mpcCode->NumLines();
	for (ui = 0; ui < uiNum; ui++)
	{
		psLine = mpcCode->GetLine(ui);
		psWidths = mpcCode->GetWidths(ui);
		iCopied = IR65_VALUE_UNKNOWN;
		if ((psLine->uiKind == IR65CK_Alu) && (mpcCode->Mnemonic(ui) == IR65M_Cpy))
		{
			IR65PeepholeOperands(mpcCode, ui, &szFirst, &uiFirst, &szSecond, &uiSecond);
			if ((psLine->iFirst >= 0) && (psWidths->auiRegisters[psLine->iFirst] == 32) && ((psLine->iSecond < 0) || (psWidths->auiRegisters[psLine->iSecond] == 32)))
			{
				iCopied = AddValue(szSecond, uiSecond);
			}
			else if (psLine->bFirstMemory && (psLine->iSecond >= 0) && (psWidths->auiRegisters[psLine->iSecond] == 32) && CIR65Code::IsOwnMemory(szFirst, uiFirst, mbHomeV, &uiBase, &iOffset))
			{
				iCopied = AddValue(szFirst, uiFirst);
			}
		}
		maiCopied.Add()[0] = iCopied;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What the registers hold as each line starts: nothing known at the
// function's label, then what the lines before it leave (nothing where
// two ways in disagree).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Peephole::ComputeStates(void)
{
	size			ui;
	size			uiNum;
	size			uiNext;
	size			auiTo[2];
	int				iTo;
	int				i;
	SIR65Values		sState;
	SIR65Values*	psTo;
	SIR65CodeLine*	psLine;
	bool			bChanged;

	uiNum = mpcCode->NumLines();
	masStates.ReInit();
	for (ui = 0; ui < uiNum; ui++)
	{
		psTo = masStates.Add();
		for (i = 0; i < IR65_CODE_REGISTERS; i++)
		{
			psTo->aiValues[i] = (ui == 0) ? IR65_VALUE_UNKNOWN : IR65_VALUE_UNSET;
		}
		psTo->bZeroW = false;
	}

	do
	{
		bChanged = false;
		for (ui = 0; ui < uiNum; ui++)
		{
			if (masStates.Get(ui)->aiValues[0] == IR65_VALUE_UNSET)
			{
				continue;
			}
			psLine = mpcCode->GetLine(ui);
			sState = *masStates.Get(ui);
			Transfer(ui, &sState);
			TransferW(ui, &sState);

			auiTo[0] = (mpcCode->FallsThrough(ui) && (ui + 1 < uiNum)) ? ui + 1 : uiNum;
			auiTo[1] = (psLine->iTarget >= 0) ? (size)psLine->iTarget : uiNum;
			for (iTo = 0; iTo < 2; iTo++)
			{
				uiNext = auiTo[iTo];
				if (uiNext >= uiNum)
				{
					continue;
				}
				psTo = masStates.Get(uiNext);
				if (psTo->aiValues[0] == IR65_VALUE_UNSET)
				{
					*psTo = sState;
					bChanged = true;
					continue;
				}
				for (i = 0; i < IR65_CODE_REGISTERS; i++)
				{
					if ((psTo->aiValues[i] != sState.aiValues[i]) && (psTo->aiValues[i] != IR65_VALUE_UNKNOWN))
					{
						psTo->aiValues[i] = IR65_VALUE_UNKNOWN;
						bChanged = true;
					}
				}
				if (psTo->bZeroW && !sState.bZeroW)
				{
					psTo->bZeroW = false;
					bChanged = true;
				}
			}
		}
	}
	while (bChanged);
}


//////////////////////////////////////////////////////////////////////////
//
// What the registers hold after the line, from what they held before it.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Peephole::Transfer(size uiLine, SIR65Values* psState)
{
	SIR65CodeLine*	psLine;
	SIR65Widths		sWidths;
	SIR65Widths		sReads;
	SIR65Widths		sWrites;
	const char*		szFirst;
	const char*		szSecond;
	size			uiFirst;
	size			uiSecond;
	uint8			uiUses;
	int16			iCopied;
	int				i;

	psLine = mpcCode->GetLine(uiLine);
	if ((psLine->uiKind == IR65CK_Label) || (psLine->uiKind == IR65CK_Other))
	{
		return;
	}
	if (psLine->uiKind == IR65CK_Call)
	{
		//It changes every register, and memory: a callee uses [0 + W] to [127 + W] too.
		for (i = 0; i < IR65_CODE_REGISTERS; i++)
		{
			psState->aiValues[i] = IR65_VALUE_UNKNOWN;
		}
		return;
	}

	//What the line copies, as it was before the line changes anything.
	iCopied = *maiCopied.Get(uiLine);
	if ((iCopied >= 0) && (psLine->iFirst >= 0))
	{
		iCopied = Canonical(psState, iCopied);
	}

	sWidths = *mpcCode->GetWidths(uiLine);
	mpcCode->Effect(uiLine, &sWidths, &sReads, &sWrites, &uiUses);
	for (i = 0; i < IR65_CODE_REGISTERS; i++)
	{
		if (sWrites.auiRegisters[i] != 0)
		{
			Forget(psState, (uint8)(1 << i));
		}
	}
	switch (psLine->uiKind)
	{
	case IR65CK_Push:
		Forget(psState, IR65_NAME_S);
		break;
	case IR65CK_Pull:
		Forget(psState, IR65_NAME_S);
		IR65PeepholeOperands(mpcCode, uiLine, &szFirst, &uiFirst, &szSecond, &uiSecond);
		if (psLine->bFirstMemory)
		{
			ForgetMemory(psState, szFirst, uiFirst);
		}
		else if (uiFirst == 1)
		{
			//pul V (or W).
			Forget(psState, IR65PeepholeName(szFirst[0]));
		}
		break;
	case IR65CK_Multiply:
	case IR65CK_Divide:
		//They count W up.
		Forget(psState, IR65_NAME_W);
		break;
	case IR65CK_Alu:
	case IR65CK_Negate:
		if (!psLine->bWritesFirst)
		{
			break;
		}
		IR65PeepholeOperands(mpcCode, uiLine, &szFirst, &uiFirst, &szSecond, &uiSecond);
		if (psLine->bFirstMemory)
		{
			ForgetMemory(psState, szFirst, uiFirst);
		}
		else if ((psLine->iFirst < 0) && (uiFirst == 1))
		{
			//S, V or W.
			Forget(psState, IR65PeepholeName(szFirst[0]));
		}
		break;
	}

	if ((iCopied >= 0) && (psLine->iFirst >= 0))
	{
		//cpy R, value.
		if ((masValues.Get(iCopied)->uiNames & (1 << psLine->iFirst)) == 0)
		{
			psState->aiValues[psLine->iFirst] = iCopied;
		}
	}
	else if ((iCopied >= 0) && (psState->aiValues[psLine->iSecond] == IR65_VALUE_UNKNOWN))
	{
		//cpy [n + W], R.
		psState->aiValues[psLine->iSecond] = iCopied;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether W is known to be 0 after the line: cpy W, 0 makes it so; a call
// may change it, mul and div count in it, and anything else that writes it
// (its first operand, or a pul of it) leaves it unknown.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Peephole::TransferW(size uiLine, SIR65Values* psState)
{
	SIR65CodeLine*	psLine;
	const char*		szFirst;
	const char*		szSecond;
	size			uiFirst;
	size			uiSecond;

	psLine = mpcCode->GetLine(uiLine);
	switch (psLine->uiKind)
	{
	case IR65CK_Other:
	case IR65CK_Label:
	case IR65CK_Push:
		break;
	case IR65CK_Call:
	case IR65CK_Multiply:
	case IR65CK_Divide:
		psState->bZeroW = false;
		break;
	default:
		if (mpcCode->IsWith(uiLine, IR65M_Cpy, "W, 0"))
		{
			psState->bZeroW = true;
		}
		else if ((psLine->uiKind == IR65CK_Pull) && mpcCode->OperandsHave(uiLine, "W"))
		{
			psState->bZeroW = false;
		}
		else if ((psLine->uiKind != IR65CK_Alu) || psLine->bWritesFirst)
		{
			IR65PeepholeOperands(mpcCode, uiLine, &szFirst, &uiFirst, &szSecond, &uiSecond);
			if ((uiFirst == 1) && (szFirst[0] == 'W'))
			{
				psState->bZeroW = false;
			}
		}
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Forgets the values of registers in uiNames (A, B, X and Y bits) and of
// any register holding a value that names one of them.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Peephole::Forget(SIR65Values* psState, uint8 uiNames)
{
	int		i;
	int16	iValue;

	for (i = 0; i < IR65_CODE_REGISTERS; i++)
	{
		iValue = psState->aiValues[i];
		if (((1 << i) & uiNames) || ((iValue >= 0) && (masValues.Get(iValue)->uiNames & uiNames)))
		{
			psState->aiValues[i] = IR65_VALUE_UNKNOWN;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Forgets the memory values a write to the operand may change: own memory
// near it ([n + W] is only ever written as itself; the frame may also be
// written through a pointer), or all the frame for any other memory.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Peephole::ForgetMemory(SIR65Values* psState, const char* szOperand, size uiLength)
{
	int			i;
	int16		iValue;
	SIR65Value*	psValue;
	uint8		uiBase;
	int32		iOffset;
	bool		bOwn;

	bOwn = CIR65Code::IsOwnMemory(szOperand, uiLength, mbHomeV, &uiBase, &iOffset);
	for (i = 0; i < IR65_CODE_REGISTERS; i++)
	{
		iValue = psState->aiValues[i];
		if (iValue < 0)
		{
			continue;
		}
		psValue = masValues.Get(iValue);
		if (psValue->uiBase == 0)
		{
			continue;
		}
		if (!bOwn)
		{
			if (psValue->uiBase != 'W')
			{
				psState->aiValues[i] = IR65_VALUE_UNKNOWN;
			}
		}
		else if ((psValue->uiBase == 'W') != (uiBase == 'W'))
		{
			//The pseudo registers and the stack don't overlap.
		}
		else if ((psValue->uiBase != uiBase) || ((psValue->iOffset < iOffset + 8) && (iOffset < psValue->iOffset + 8)))
		{
			//Within the widest write (8 bytes) or the frame from S and V.
			psState->aiValues[i] = IR65_VALUE_UNKNOWN;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A register's value as what that register holds, when known.
//
//////////////////////////////////////////////////////////////////////////
int16 CIR65Peephole::Canonical(SIR65Values* psState, int16 iValue)
{
	CChars*	psz;
	int		i;

	psz = maszValues.Get(iValue);
	if (psz->Length() == 1)
	{
		for (i = 0; i < IR65_CODE_REGISTERS; i++)
		{
			if ((psz->Text()[0] == CIR65Code::RegisterName(i)) && (psState->aiValues[i] >= 0))
			{
				return psState->aiValues[i];
			}
		}
	}
	return iValue;
}


//////////////////////////////////////////////////////////////////////////
//
// A bra to a label whose code is a ret is a ret.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Peephole::ReturnDirectly(void)
{
	size			ui;
	size			uiNum;
	size			uiNext;
	SIR65CodeLine*	psLine;
	bool			bChanged;

	bChanged = false;
	uiNum = mpcCode->NumLines();
	for (ui = 0; ui < uiNum; ui++)
	{
		psLine = mpcCode->GetLine(ui);
		if (psLine->uiKind != IR65CK_Jump)
		{
			continue;
		}
		uiNext = IRNextInstruction(mpcCode, (size)psLine->iTarget);
		if ((uiNext < uiNum) && (mpcCode->GetLine(uiNext)->uiKind == IR65CK_Return) && !mpcCode->GetLine(uiNext)->bReadsFirst)
		{
			bChanged |= mpcCode->ReplaceInstruction(ui, IR65M_Ret, NULL);
		}
	}
	return bChanged;
}


