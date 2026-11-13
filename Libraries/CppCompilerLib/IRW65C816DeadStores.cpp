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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "IRW65C816BackendInternal.h"
#include "IRW65C816Code.h"


//The W65C816 back end's stores nothing reads taken out (RemoveUnreadStores, RemoveDeadStores), and what the
//function's code reads of its frame (IR816FindReads).


//////////////////////////////////////////////////////////////////////////
//
// Whether an operand may read or write the direct page bytes near
// iOffset: it names [D + n] with n within 2 of it (a pointer is 3 bytes;
// words may start at either byte), or [D + X].  A whole operand [D + n] is
// a word, bytes n and n + 1: near only when it shares one with iOffset's.
//
//////////////////////////////////////////////////////////////////////////
bool IR816MentionsNear(const char* szOperand, size uiOperand, int32 iOffset)
{
	size	ui;
	int32	iValue;
	size	uiDigit;
	bool	bWord;

	for (ui = 0; ui + 5 <= uiOperand; ui++)
	{
		if (strncmp(szOperand + ui, "D + ", 4) != 0)
		{
			continue;
		}
		if (szOperand[ui + 4] == 'X')
		{
			return true;
		}
		iValue = 0;
		for (uiDigit = ui + 4; (uiDigit < uiOperand) && (szOperand[uiDigit] >= '0') && (szOperand[uiDigit] <= '9'); uiDigit++)
		{
			iValue = iValue * 10 + (szOperand[uiDigit] - '0');
		}
		bWord = (ui == 1) && (szOperand[0] == '[') && (uiDigit + 1 == uiOperand) && (szOperand[uiDigit] == ']');
		if (bWord ? ((iValue >= iOffset - 1) && (iValue <= iOffset + 1)) : ((iValue >= iOffset - 2) && (iValue <= iOffset + 2)))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a store to the direct page word szWord (as "[D + 12]") by the line
// before line uiFrom is never read: in straight line code (no label, jump, call
// or width change) the next instruction that names a byte near it, other
// than a store to another direct page word, stores a word to it.
//
//////////////////////////////////////////////////////////////////////////
bool IR816DeadStore(CIRW65C816Code* pcCode, size uiFrom, const char* szWord, size uiWord)
{
	SIRW65C816Line	sLine;
	int32		iOffset;
	bool		bStore;
	size		ui;

	iOffset = atoi(szWord + 5);
	for (ui = uiFrom; ui < pcCode->NumLines(); ui++)
	{
		if (!pcCode->Instruction(ui, &sLine))
		{
			return false;
		}
		if (sLine.uiFlags & (IR816MF_LEAVES | IR816MF_WIDTH | IR816MF_BLOCK_MOVE | IR816MF_SETS_D | IR816MF_INTERRUPT))
		{
			return false;
		}
		if (IR816MentionsNear(sLine.szOperand, sLine.uiOperand, iOffset))
		{
			bStore = (sLine.uiFlags & IR816MF_STORE) != 0;
			if (bStore && IR816OperandIs(&sLine, szWord, uiWord))
			{
				return true;
			}
			if (bStore && (strncmp(sLine.szOperand, "[D + ", 5) == 0) && (sLine.szOperand[sLine.uiOperand - 1] == ']') && !memchr(sLine.szOperand, ':', sLine.uiOperand))
			{
				//A store to another direct page word reads nothing.
				continue;
			}
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an lda before line uiFrom is never read: in straight line
// code, before anything reads A or the N and Z the lda set, an instruction
// sets A again (only IR816LeavesA instructions come between).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816DeadLoad(CIRW65C816Code* pcCode, size uiFrom)
{
	SIRW65C816Line	sLine;
	size		ui;

	for (ui = uiFrom; ui < pcCode->NumLines(); ui++)
	{
		if (!pcCode->Instruction(ui, &sLine))
		{
			return false;
		}
		if (sLine.uiFlags & IR816MF_LOADS_A)
		{
			return true;
		}
		if (!IR816LeavesA(&sLine))
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the tdc on line uiLine only moves S: a tcs follows it, after nothing
// but a clc and an adc of a constant (a frame's exit).
//
//////////////////////////////////////////////////////////////////////////
bool IR816MovesS(CIRW65C816Code* pcCode, size uiLine)
{
	SIRW65C816Line	sLine;
	int			i;
	size		ui;

	for (i = 0, ui = uiLine + 1; (i < 3) && (ui < pcCode->NumLines()) && pcCode->Instruction(ui, &sLine); i++, ui++)
	{
		if (sLine.uiMnemonic == IR816M_Tcs)
		{
			return true;
		}
		if ((sLine.uiMnemonic != IR816M_Clc) && !((sLine.uiMnemonic == IR816M_Adc) && (sLine.uiOperand > 0) && (sLine.szOperand[0] >= '0') && (sLine.szOperand[0] <= '9')))
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The direct page bytes the function's code from line uiFrom may read: for each
// instruction mentioning [D + n] other than a store to it, the word's two
// bytes ([D + n]), or a pointer's three ([[D + n]:24 ...]), and the
// scratch bytes when it calls a runtime routine (they work in the caller's
// frame, IR816ScratchRoutine).  True (all of them may be) when the code
// indexes the direct page ([D + X]) or takes D's value (tdc: the frame's
// address may be passed on), unless only to move S (IR816MovesS).
//
//////////////////////////////////////////////////////////////////////////
bool IR816FindReads(CIRW65C816Code* pcCode, size uiFrom, uint8* abRead)
{
	SIRW65C816Line	sLine;
	const char*	szOperand;
	int32		iOffset;
	int			iBytes;
	int			i;
	size		ui;

	memset(abRead, 0, IR816_DIRECT_PAGE);
	for (ui = uiFrom; ui < pcCode->NumLines(); ui++)
	{
		if (!pcCode->Instruction(ui, &sLine) || (pcCode->Text(ui)[2] == ';'))
		{
			continue;
		}
		if ((sLine.uiMnemonic == IR816M_Tdc) && !IR816MovesS(pcCode, ui))
		{
			return true;
		}
		if (sLine.uiMnemonic == IR816M_Unknown)
		{
			return true;
		}
		if ((sLine.uiMnemonic == IR816M_Jsl) && (sLine.uiOperand > 3) && (strncmp(&sLine.szOperand[sLine.uiOperand - 3], IR816_FRAME_OPERANDS, 3) == 0))
		{
			return true;
		}
		if ((sLine.uiMnemonic == IR816M_Jsl) && (strncmp(sLine.szOperand, IR816_RUNTIME_PREFIX, IR816_RUNTIME_PREFIX_LENGTH) == 0))
		{
			if (IR816IndexedRoutine(sLine.szOperand, sLine.uiOperand))
			{
				return true;
			}
			memset(&abRead[IR816_SCRATCH_FIRST], 1, IR816_SCRATCH_END - IR816_SCRATCH_FIRST);
			continue;
		}
		szOperand = sLine.szOperand;
		if ((sLine.uiOperand >= 7) && (strncmp(szOperand, "[D + X]", 7) == 0))
		{
			return true;
		}
		if (sLine.bDirect)
		{
			if (sLine.uiFlags & IR816MF_STORE)
			{
				continue;
			}
			iOffset = sLine.iOffset;
			iBytes = 2;
		}
		else if ((sLine.uiOperand >= 7) && (strncmp(szOperand, "[[D + ", 6) == 0) && (szOperand[6] >= '0') && (szOperand[6] <= '9'))
		{
			iOffset = atoi(&szOperand[6]);
			iBytes = 3;
		}
		else
		{
			continue;
		}
		for (i = 0; i < iBytes; i++)
		{
			if (iOffset + i >= IR816_DIRECT_PAGE)
			{
				return true;
			}
			abRead[iOffset + i] = 1;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Stores to a direct page word whose bytes nothing in the function's code
// from uiStart reads (IR816FindReads) taken out, wherever they are: the
// high words of values only their low words of are used (a shift's
// count), and values worked out for nothing.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::RemoveUnreadStores(CChars* pszDest, size uiStart)
{
	CIRW65C816Code	cCode;

	cCode.Init();
	cCode.Load(pszDest, uiStart);
	RemoveUnreadStores(&cCode);
	cCode.Write(pszDest, uiStart);
	cCode.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::RemoveUnreadStores(CIRW65C816Code* pcCode)
{
	SIRW65C816Line				sLine;
	uint8					abRead[IR816_DIRECT_PAGE];
	size					ui;
	CArrayTemplate<uint8>	abRemove;

	if (IR816FindReads(pcCode, 0, abRead))
	{
		return;
	}
	abRemove.Init(CIRW65C816Backend::GetScratch());
	abRemove.GrowToAtLeastNumElements(pcCode->NumLines(), true, 0);
	for (ui = 0; ui < pcCode->NumLines(); ui++)
	{
		if (pcCode->Instruction(ui, &sLine) && (pcCode->Text(ui)[2] != ';'))
		{
			if (sLine.bDirect && (sLine.uiFlags & IR816MF_STORE) && (sLine.iOffset < IR816_DIRECT_PAGE - 1) && !abRead[sLine.iOffset] && !abRead[sLine.iOffset + 1])
			{
				*abRemove.Get(ui) = 1;
			}
		}
	}
	pcCode->RemoveMarked(&abRemove);
	abRemove.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// After Peephole: stores to a direct page word that the next store to it
// overwrites unread (IR816DeadStore) go, and loads of a direct page word or
// a constant that A is loaded again over before it is read (IR816DeadLoad),
// until none are left.
// Places are shared, so a value worked out in steps is often stored after
// each.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::RemoveDeadStores(CChars* pszDest, size uiStart)
{
	CIRW65C816Code	cCode;

	cCode.Init();
	cCode.Load(pszDest, uiStart);
	RemoveDeadStores(&cCode);
	cCode.Write(pszDest, uiStart);
	cCode.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each pass decides from the code as it was when the pass began (a line a
// pass takes out still counts for the lines before it), then takes the
// lines out.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Backend::RemoveDeadStores(CIRW65C816Code* pcCode)
{
	CArrayTemplate<uint8>	abRemove;
	SIRW65C816Line				sLine;
	size					ui;
	bool					bRemoved;

	//Again while any went: the lda that fed a store that went is loaded over unread now.
	abRemove.Init(CIRW65C816Backend::GetScratch());
	do
	{
		bRemoved = false;
		abRemove.GrowToAtLeastNumElements(pcCode->NumLines(), false);
		for (ui = 0; ui < pcCode->NumLines(); ui++)
		{
			*abRemove.Get(ui) = 0;
			if ((pcCode->GetText(ui)->Length() >= 3) && pcCode->Instruction(ui, &sLine))
			{
				if (sLine.bDirect && (sLine.uiFlags & IR816MF_STORE) && IR816DeadStore(pcCode, ui + 1, sLine.szOperand, sLine.uiOperand))
				{
					*abRemove.Get(ui) = 1;
					bRemoved = true;
					continue;
				}
				if ((sLine.uiMnemonic == IR816M_Lda) && (sLine.bDirect || ((sLine.uiOperand > 0) && (sLine.szOperand[0] != '['))) && IR816DeadLoad(pcCode, ui + 1))
				{
					*abRemove.Get(ui) = 1;
					bRemoved = true;
					continue;
				}
			}
		}
		pcCode->RemoveMarked(&abRemove);
	}
	while (bRemoved);
	abRemove.Kill();
}
