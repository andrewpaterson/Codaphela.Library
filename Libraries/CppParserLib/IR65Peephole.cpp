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
#include "IR65Peephole.h"


#define IR65_PEEPHOLE_ROUNDS	8		//The most times the jumps are looked at again after some changed.
#define IR65_PEEPHOLE_HOPS		8		//The most jumps a jump is followed through (a loop of bras never ends).
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
	case 'A':	return 0x01;
	case 'B':	return 0x02;
	case 'X':	return 0x04;
	case 'Y':	return 0x08;
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
// Finds the operands of an instruction line ("\t\tcpy [8 + W], A"): the
// first starts at *pszFirst, the second (if there is one) at *pszSecond,
// and their lengths (a cpyx memory operand's without its width).
//
//////////////////////////////////////////////////////////////////////////
static void IR65PeepholeOperands(CChars* pszLine, const char** pszFirst, size* puiFirst, const char** pszSecond, size* puiSecond)
{
	const char*	sz;
	const char*	szComma;

	sz = FindChar(pszLine->Text() + 2, ' ', false);
	*pszFirst = (sz != NULL) ? sz + 1 : pszLine->Text() + pszLine->Length();
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
	mcCode.Init();
	mauiReached.Init();
	maszValues.Init();
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
	maszValues.Kill();
	mauiReached.Kill();
	mcCode.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Rewrites the function's code from uiStart (its label first) with its
// jumps tidied.  Code with anything the decoder doesn't know is left as
// it is.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Peephole::Optimise(CChars* pszCode, size uiStart)
{
	size	ui;
	int		iRound;
	bool	bChanged;
	bool	bAny;

	if (!mcCode.Load(pszCode, uiStart, true))
	{
		return;
	}

	bAny = RemoveCopies();
	for (iRound = 0; iRound < IR65_PEEPHOLE_ROUNDS; iRound++)
	{
		bChanged = ThreadJumps();
		bChanged |= ReturnDirectly();
		bChanged |= InvertBranches();
		bChanged |= RemoveBranchesToNext();
		bChanged |= RemoveUnreached();
		if (!bChanged)
		{
			break;
		}
		bAny = true;
	}
	if (!bAny)
	{
		return;
	}

	pszCode->SetLength(uiStart);
	for (ui = 0; ui < mcCode.NumLines(); ui++)
	{
		mcCode.WriteLine(pszCode, ui);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Takes out each cpy into A, B, X or Y (at 32 bits) of the value it
// already holds, when the n and z it sets aren't read.
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
	mcCode.ComputeFlagDemand(&mauiDemand);

	bChanged = false;
	uiNum = mcCode.NumLines();
	for (ui = 0; ui < uiNum; ui++)
	{
		psLine = mcCode.GetLine(ui);
		psState = masStates.Get(ui);
		iCopied = *maiCopied.Get(ui);
		if ((iCopied < 0) || (psLine->iFirst < 0) || (psState->aiValues[0] == IR65_VALUE_UNSET) || (*mauiDemand.Get(ui) & (IR65_FLAG_N | IR65_FLAG_Z)))
		{
			continue;
		}
		if ((psState->aiValues[psLine->iFirst] >= 0) && (psState->aiValues[psLine->iFirst] == Canonical(psState, iCopied)))
		{
			mcCode.Remove(ui);
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
	CChars*		psz;
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
	else if (!CIR65Code::IsOwnMemory(szOperand, uiLength, &sValue.uiBase, &sValue.iOffset))
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

	for (ui = 0; ui < maszValues.NumElements(); ui++)
	{
		psz = maszValues.Get(ui);
		if ((psz->Length() == uiLength) && (memcmp(psz->Text(), szOperand, uiLength) == 0))
		{
			return (int16)ui;
		}
	}
	if (maszValues.NumElements() >= IR65_PEEPHOLE_VALUES)
	{
		return IR65_VALUE_UNKNOWN;
	}
	maszValues.Add()->Init()->AppendSubString(szOperand, uiLength);
	masValues.Add(&sValue);
	return (int16)(maszValues.NumElements() - 1);
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
	CChars*			pszLine;
	const char*		szFirst;
	const char*		szSecond;
	size			uiFirst;
	size			uiSecond;
	int16			iCopied;
	uint8			uiBase;
	int32			iOffset;

	maszValues.ReInit();
	masValues.ReInit();
	maiCopied.ReInit();
	uiNum = mcCode.NumLines();
	for (ui = 0; ui < uiNum; ui++)
	{
		psLine = mcCode.GetLine(ui);
		pszLine = mcCode.GetText(ui);
		psWidths = mcCode.GetWidths(ui);
		iCopied = IR65_VALUE_UNKNOWN;
		if ((psLine->uiKind == IR65CK_Alu) && pszLine->StartsWith("\t\tcpy "))
		{
			IR65PeepholeOperands(pszLine, &szFirst, &uiFirst, &szSecond, &uiSecond);
			if ((psLine->iFirst >= 0) && (psWidths->auiRegisters[psLine->iFirst] == 32) && ((psLine->iSecond < 0) || (psWidths->auiRegisters[psLine->iSecond] == 32)))
			{
				iCopied = AddValue(szSecond, uiSecond);
			}
			else if (psLine->bFirstMemory && (psLine->iSecond >= 0) && (psWidths->auiRegisters[psLine->iSecond] == 32) && CIR65Code::IsOwnMemory(szFirst, uiFirst, &uiBase, &iOffset))
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

	uiNum = mcCode.NumLines();
	masStates.ReInit();
	for (ui = 0; ui < uiNum; ui++)
	{
		psTo = masStates.Add();
		for (i = 0; i < IR65_CODE_REGISTERS; i++)
		{
			psTo->aiValues[i] = (ui == 0) ? IR65_VALUE_UNKNOWN : IR65_VALUE_UNSET;
		}
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
			psLine = mcCode.GetLine(ui);
			sState = *masStates.Get(ui);
			Transfer(ui, &sState);

			auiTo[0] = (mcCode.FallsThrough(ui) && (ui + 1 < uiNum)) ? ui + 1 : uiNum;
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
	CChars*			pszLine;
	const char*		szFirst;
	const char*		szSecond;
	size			uiFirst;
	size			uiSecond;
	uint8			uiUses;
	int16			iCopied;
	int				i;

	psLine = mcCode.GetLine(uiLine);
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

	sWidths = *mcCode.GetWidths(uiLine);
	mcCode.Effect(uiLine, &sWidths, &sReads, &sWrites, &uiUses);
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
		IR65PeepholeOperands(mcCode.GetText(uiLine), &szFirst, &uiFirst, &szSecond, &uiSecond);
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
		//They count W up (it is put back to 0 after them).
		Forget(psState, IR65_NAME_W);
		break;
	case IR65CK_Alu:
		if (!psLine->bWritesFirst)
		{
			break;
		}
		pszLine = mcCode.GetText(uiLine);
		IR65PeepholeOperands(pszLine, &szFirst, &uiFirst, &szSecond, &uiSecond);
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

	bOwn = CIR65Code::IsOwnMemory(szOperand, uiLength, &uiBase, &iOffset);
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
// The first line from uiLine on that runs (not a label, blank or taken
// out); NumLines if there is none.
//
//////////////////////////////////////////////////////////////////////////
size CIR65Peephole::NextInstruction(size uiLine)
{
	size			uiNum;
	SIR65CodeLine*	psLine;

	uiNum = mcCode.NumLines();
	for (; uiLine < uiNum; uiLine++)
	{
		psLine = mcCode.GetLine(uiLine);
		if ((psLine->uiKind != IR65CK_Label) && (psLine->uiKind != IR65CK_Other))
		{
			return uiLine;
		}
	}
	return uiNum;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes the branch or jump at uiLine szMnemonic (bra, bras or brac) with
// its condition, to the label at uiTarget.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Peephole::Retarget(size uiLine, const char* szMnemonic, size uiTarget)
{
	CChars*		pszLine;
	CChars*		pszLabel;
	CChars		szInstruction;
	const char*	szCondition;
	const char*	szTarget;
	bool		bReplaced;

	pszLine = mcCode.GetText(uiLine);
	pszLabel = mcCode.GetText(uiTarget);
	szInstruction.Init(szMnemonic);
	szInstruction.Append(' ');
	if (mcCode.GetLine(uiLine)->uiKind == IR65CK_Branch)
	{
		//"\t\tbras z, L5": the condition is between the mnemonic and the target.
		szCondition = FindChar(pszLine->Text() + 2, ' ', false) + 1;
		szTarget = FindChar(pszLine->Text(), ',', true);
		szInstruction.AppendSubString(szCondition, szTarget + 2);
	}
	szInstruction.AppendSubString(pszLabel->Text(), pszLabel->Length() - 1);
	bReplaced = mcCode.Replace(uiLine, szInstruction.Text());
	szInstruction.Kill();
	return bReplaced;
}


//////////////////////////////////////////////////////////////////////////
//
// A branch or jump to a label whose code is a bra goes where that bra
// goes (the Widths pass sets the widths a label needs before a jump to
// it, so the bra is reached with those its label needs too).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Peephole::ThreadJumps(void)
{
	size			ui;
	size			uiNum;
	size			uiTarget;
	size			uiNext;
	SIR65CodeLine*	psLine;
	SIR65CodeLine*	psNext;
	CChars			szMnemonic;
	int				iHops;
	bool			bChanged;

	bChanged = false;
	uiNum = mcCode.NumLines();
	for (ui = 0; ui < uiNum; ui++)
	{
		psLine = mcCode.GetLine(ui);
		if ((psLine->uiKind != IR65CK_Branch) && (psLine->uiKind != IR65CK_Jump))
		{
			continue;
		}
		uiTarget = (size)psLine->iTarget;
		for (iHops = 0; iHops < IR65_PEEPHOLE_HOPS; iHops++)
		{
			uiNext = NextInstruction(uiTarget);
			if (uiNext >= uiNum)
			{
				break;
			}
			psNext = mcCode.GetLine(uiNext);
			if ((psNext->uiKind != IR65CK_Jump) || ((size)psNext->iTarget == uiTarget))
			{
				break;
			}
			uiTarget = (size)psNext->iTarget;
		}
		if (uiTarget != (size)psLine->iTarget)
		{
			szMnemonic.Init();
			szMnemonic.AppendSubString(mcCode.GetText(ui)->Text() + 2, FindChar(mcCode.GetText(ui)->Text() + 2, ' ', false));
			bChanged |= Retarget(ui, szMnemonic.Text(), uiTarget);
			szMnemonic.Kill();
		}
	}
	return bChanged;
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
	uiNum = mcCode.NumLines();
	for (ui = 0; ui < uiNum; ui++)
	{
		psLine = mcCode.GetLine(ui);
		if (psLine->uiKind != IR65CK_Jump)
		{
			continue;
		}
		uiNext = NextInstruction((size)psLine->iTarget);
		if ((uiNext < uiNum) && (mcCode.GetLine(uiNext)->uiKind == IR65CK_Return) && !mcCode.GetLine(uiNext)->bReadsFirst)
		{
			bChanged |= mcCode.Replace(ui, "ret");
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
//			bras z, L1			->			brac z, L2
//			bra L2
//	L1:								L1:
//
// (when nothing else goes to the bra).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Peephole::InvertBranches(void)
{
	size			ui;
	size			uiNum;
	size			uiJump;
	size			uiTarget;
	SIR65CodeLine*	psLine;
	SIR65CodeLine*	psJump;
	CChars*			pszLine;
	bool			bChanged;

	bChanged = false;
	uiNum = mcCode.NumLines();
	for (ui = 0; ui < uiNum; ui++)
	{
		psLine = mcCode.GetLine(ui);
		pszLine = mcCode.GetText(ui);
		if ((psLine->uiKind != IR65CK_Branch) || !(pszLine->StartsWith("\t\tbras ") || pszLine->StartsWith("\t\tbrac ")))
		{
			continue;
		}
		for (uiJump = ui + 1; (uiJump < uiNum) && (mcCode.GetLine(uiJump)->uiKind == IR65CK_Other); uiJump++)
		{
		}
		if ((uiJump >= uiNum) || (mcCode.GetLine(uiJump)->uiKind != IR65CK_Jump))
		{
			continue;
		}
		psJump = mcCode.GetLine(uiJump);
		uiTarget = (size)psLine->iTarget;
		if ((uiTarget <= uiJump) || (NextInstruction(uiJump + 1) < uiTarget) || ((size)psJump->iTarget == uiTarget))
		{
			continue;
		}
		if (Retarget(ui, pszLine->StartsWith("\t\tbras ") ? "brac" : "bras", (size)psJump->iTarget))
		{
			mcCode.Remove(uiJump);
			bChanged = true;
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// A branch or jump to a label with nothing that runs between it and the
// label goes.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Peephole::RemoveBranchesToNext(void)
{
	size			ui;
	size			uiNum;
	size			uiTarget;
	SIR65CodeLine*	psLine;
	bool			bChanged;

	bChanged = false;
	uiNum = mcCode.NumLines();
	for (ui = 0; ui < uiNum; ui++)
	{
		psLine = mcCode.GetLine(ui);
		if ((psLine->uiKind != IR65CK_Branch) && (psLine->uiKind != IR65CK_Jump))
		{
			continue;
		}
		uiTarget = (size)psLine->iTarget;
		if ((uiTarget > ui) && (NextInstruction(ui + 1) >= uiTarget))
		{
			mcCode.Remove(ui);
			bChanged = true;
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// Takes out the lines (labels too) that nothing reaches from the
// function's label.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Peephole::RemoveUnreached(void)
{
	size					ui;
	size					uiNum;
	SIR65CodeLine*			psLine;
	CArrayTemplate<size>	auiStack;
	bool					bChanged;

	uiNum = mcCode.NumLines();
	mauiReached.ReInit();
	for (ui = 0; ui < uiNum; ui++)
	{
		mauiReached.Add()[0] = 0;
	}
	auiStack.Init();
	auiStack.Add()[0] = 0;
	while (auiStack.Pop(&ui))
	{
		for (; (ui < uiNum) && !*mauiReached.Get(ui); ui++)
		{
			*mauiReached.Get(ui) = 1;
			psLine = mcCode.GetLine(ui);
			if (psLine->iTarget >= 0)
			{
				auiStack.Add()[0] = (size)psLine->iTarget;
			}
			if (!mcCode.FallsThrough(ui))
			{
				break;
			}
		}
	}
	auiStack.Kill();

	bChanged = false;
	for (ui = 0; ui < uiNum; ui++)
	{
		if (!*mauiReached.Get(ui) && !mcCode.IsRemoved(ui) && !mcCode.GetText(ui)->Empty())
		{
			mcCode.Remove(ui);
			bChanged = true;
		}
	}
	return bChanged;
}

