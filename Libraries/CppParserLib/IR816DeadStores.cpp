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
#include <stdlib.h>
#include <string.h>
#include "IR816BackendInternal.h"


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
// before szLine is never read: in straight line code (no label, jump, call
// or width change) the next instruction that names a byte near it, other
// than a store to another direct page word, stores a word to it.
//
//////////////////////////////////////////////////////////////////////////
bool IR816DeadStore(const char* szLine, const char* szWord, size uiWord)
{
	SIR816Line	sLine;
	int32		iOffset;
	bool		bStore;

	iOffset = atoi(szWord + 5);
	for (; szLine; szLine = IR816NextLine(szLine))
	{
		if (!IR816IsInstruction(szLine))
		{
			return false;
		}
		IR816Decode(szLine, &sLine);
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
// Whether an lda before the line at szLine is never read: in straight line
// code, before anything reads A or the N and Z the lda set, an instruction
// sets A again (only IR816LeavesA instructions come between).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816DeadLoad(const char* szLine)
{
	SIR816Line	sLine;

	for (; szLine; szLine = IR816NextLine(szLine))
	{
		if (!IR816IsInstruction(szLine))
		{
			return false;
		}
		IR816Decode(szLine, &sLine);
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
// Whether the tdc at szLine only moves S: a tcs follows it, after nothing
// but a clc and an adc of a constant (a frame's exit).
//
//////////////////////////////////////////////////////////////////////////
bool IR816MovesS(const char* szLine)
{
	SIR816Line	sLine;
	int			i;

	for (i = 0, szLine = IR816NextLine(szLine); (i < 3) && szLine && IR816IsInstruction(szLine); i++, szLine = IR816NextLine(szLine))
	{
		IR816Decode(szLine, &sLine);
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
// The direct page bytes the function's code at szLine may read: for each
// instruction mentioning [D + n] other than a store to it, the word's two
// bytes ([D + n]), or a pointer's three ([[D + n]:24 ...]), and the
// scratch bytes when it calls a runtime routine (they work in the caller's
// frame, IR816ScratchRoutine).  True (all of them may be) when the code
// indexes the direct page ([D + X]) or takes D's value (tdc: the frame's
// address may be passed on), unless only to move S (IR816MovesS).
//
//////////////////////////////////////////////////////////////////////////
bool IR816FindReads(const char* szLine, uint8* abRead)
{
	SIR816Line	sLine;
	const char*	szOperand;
	int32		iOffset;
	int			iBytes;
	int			i;

	memset(abRead, 0, IR816_DIRECT_PAGE);
	for (; szLine; szLine = IR816NextLine(szLine))
	{
		if (!IR816IsInstruction(szLine) || (szLine[2] == ';'))
		{
			continue;
		}
		IR816Decode(szLine, &sLine);
		if ((sLine.uiMnemonic == IR816M_Tdc) && !IR816MovesS(szLine))
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
void CIR816Backend::RemoveUnreadStores(CChars* pszDest, size uiStart)
{
	CChars		szOut;
	const char*	szLine;
	const char*	szEnd;
	SIR816Line	sLine;
	uint8		abRead[IR816_DIRECT_PAGE];

	if (IR816FindReads(pszDest->Text() + uiStart, abRead))
	{
		return;
	}
	szOut.Init(CIR816Backend::GetScratch());
	for (szLine = pszDest->Text() + uiStart; *szLine; szLine = szEnd + ((*szEnd == '\n') ? 1 : 0))
	{
		szEnd = IR816LineEnd(szLine);
		if (IR816IsInstruction(szLine) && (szLine[2] != ';'))
		{
			IR816Decode(szLine, &sLine);
			if (sLine.bDirect && (sLine.uiFlags & IR816MF_STORE) && (sLine.iOffset < IR816_DIRECT_PAGE - 1) && !abRead[sLine.iOffset] && !abRead[sLine.iOffset + 1])
			{
				continue;
			}
		}
		szOut.Append(szLine, (size)(szEnd - szLine));
		szOut.AppendNewLine();
	}
	pszDest->SetLength(uiStart);
	pszDest->Append(szOut.Text(), szOut.Length());
	szOut.Kill();
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
void CIR816Backend::RemoveDeadStores(CChars* pszDest, size uiStart)
{
	CChars		szOut;
	const char*	szLine;
	const char*	szEnd;
	SIR816Line	sLine;
	bool		bRemoved;

	//Again while any went: the lda that fed a store that went is loaded over unread now.
	do
	{
		bRemoved = false;
		szOut.Init(CIR816Backend::GetScratch());
		for (szLine = pszDest->Text() + uiStart; *szLine; szLine = szEnd + ((*szEnd == '\n') ? 1 : 0))
		{
			szEnd = IR816LineEnd(szLine);
			if ((szEnd - szLine >= 3) && IR816IsInstruction(szLine) && (*szEnd == '\n'))
			{
				IR816Decode(szLine, &sLine);
				if (sLine.bDirect && (sLine.uiFlags & IR816MF_STORE) && IR816DeadStore(szEnd + 1, sLine.szOperand, sLine.uiOperand))
				{
					bRemoved = true;
					continue;
				}
				if ((sLine.uiMnemonic == IR816M_Lda) && (sLine.bDirect || ((sLine.uiOperand > 0) && (sLine.szOperand[0] != '['))) && IR816DeadLoad(szEnd + 1))
				{
					bRemoved = true;
					continue;
				}
			}
			szOut.Append(szLine, (size)(szEnd - szLine));
			szOut.AppendNewLine();
		}
		pszDest->SetLength(uiStart);
		pszDest->Append(szOut.Text(), szOut.Length());
		szOut.Kill();
	}
	while (bRemoved);
}
