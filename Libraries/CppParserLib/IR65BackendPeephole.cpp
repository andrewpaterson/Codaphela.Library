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


//The 65C64 back end's first and last passes over a function's code text (Peephole, IR65PseudoWithoutW); the
//passes between are CIR65Flags, CIR65Widths and CIR65Peephole.


//////////////////////////////////////////////////////////////////////////
//
// add, sub, cmp, shl, shr and cpyx can't address memory with W (the user,
// 2026-10-28), so their pseudo registers are written [n] instead of [n + W] (W
// is 0 by the calling convention).  The passes before see [n + W] everywhere, so
// it is done last.
//
//////////////////////////////////////////////////////////////////////////
void IR65PseudoWithoutW(CChars* pszCode, size uiStart)
{
	static const char*	aszArithmetic[] = { "\t\tadd ", "\t\tsub ", "\t\tcmp ", "\t\tshl ", "\t\tshr ", "\t\tcpyx ", NULL };
	CChars				szOut;
	CChars				szLine;
	const char*			szText;
	const char*			szEnd;
	size				uiLength;
	int					i;

	szOut.Init();
	szText = pszCode->Text() + uiStart;
	while (*szText != '\0')
	{
		szEnd = FindChar(szText, '\n');
		uiLength = szEnd ? (size)(szEnd - szText + 1) : StrLen(szText);
		szLine.Init();
		szLine.Append(szText, uiLength);
		for (i = 0; aszArithmetic[i]; i++)
		{
			if (szLine.StartsWith(aszArithmetic[i]))
			{
				szLine.Replace(" + W]", "]");
				break;
			}
		}
		szOut.Append(szLine);
		szLine.Kill();
		szText += uiLength;
	}
	pszCode->SetLength(uiStart);
	pszCode->Append(szOut);
	szOut.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Splits "\t\tcpy first, second" into its operands; false if pszLine isn't
// a cpy of two.
//
//////////////////////////////////////////////////////////////////////////
static bool IR65CopyOperands(CChars* pszLine, CChars* pszFirst, CChars* pszSecond)
{
	const char*	szText;
	const char*	szComma;

	if (!pszLine->StartsWith("\t\tcpy "))
	{
		return false;
	}
	szText = pszLine->Text() + 6;
	szComma = strstr(szText, ", ");
	if ((szComma == NULL) || strstr(szComma + 2, ", "))
	{
		return false;
	}
	pszFirst->AppendSubString(szText, szComma);
	pszSecond->Append(szComma + 2);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A, B, X and Y 32 bits wide, as between instructions.
//
//////////////////////////////////////////////////////////////////////////
static void IR65ResetWidths(int* aiWidths)
{
	int	iRegister;

	for (iRegister = 0; iRegister < IR65_CODE_REGISTERS; iRegister++)
	{
		aiWidths[iRegister] = 32;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps track of the widths wth, wtth and wthx give A, B, X and Y (0 when
// unknown).
//
//////////////////////////////////////////////////////////////////////////
static void IR65UpdateWidths(CChars* pszLine, int* aiWidths)
{
	const char*	szText;
	const char*	szComma;
	int			iRegister;
	int			iWidth;

	if (pszLine->StartsWith("\t\twtth ("))
	{
		szText = pszLine->Text() + 8;
		szComma = strstr(szText, "), ");
		iWidth = szComma ? atoi(szComma + 3) : 0;
		iRegister = CIR65Code::RegisterIndex(szText, 1);
		if (iRegister >= 0)
		{
			aiWidths[iRegister] = iWidth;
		}
		iRegister = CIR65Code::RegisterIndex(szText + 3, 1);
		if (iRegister >= 0)
		{
			aiWidths[iRegister] = iWidth;
		}
		return;
	}
	if (pszLine->StartsWith("\t\twth ") || pszLine->StartsWith("\t\twthx "))
	{
		szText = pszLine->Text() + (pszLine->StartsWith("\t\twth ") ? 6 : 7);
		szComma = strstr(szText, ", ");
		iRegister = CIR65Code::RegisterIndex(szText, szComma ? (size)(szComma - szText) : 0);
		if (iRegister >= 0)
		{
			aiWidths[iRegister] = szComma ? atoi(szComma + 2) : 0;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if the branch at uiLine goes to a label straight after it.
//
//////////////////////////////////////////////////////////////////////////
static bool IR65BranchToNext(CArrayChars* paszLines, size uiLine)
{
	CChars*		pszLine;
	CChars*		pszLabel;
	const char*	szTarget;
	size		ui;

	pszLine = paszLines->Get(uiLine);
	if (!pszLine->StartsWith("\t\tbra ") && !pszLine->StartsWith("\t\tbras ") && !pszLine->StartsWith("\t\tbrac ") && !pszLine->StartsWith("\t\tbrasa ") && !pszLine->StartsWith("\t\tbraso "))
	{
		return false;
	}
	szTarget = FindChar(pszLine->Text(), ' ', true) + 1;
	for (ui = uiLine + 1; ui < paszLines->NumElements(); ui++)
	{
		pszLabel = paszLines->Get(ui);
		if (pszLabel->Empty() || pszLabel->StartsWith('\t'))
		{
			return false;
		}
		if (pszLabel->EndsWith(':') && (pszLabel->Length() == strlen(szTarget) + 1) && pszLabel->StartsWith(szTarget))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Cleans up the function's code written from uiStart: a branch to the
// label just after it goes, and a value stored to a pseudo register or the
// frame then read straight back (at 32 bits) is copied from the register it
// was stored from instead (or not at all into the same register: the store
// set the same flags).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Peephole(size uiStart)
{
	CChars			szText;
	CArrayChars		aszLines;
	CChars*			pszLine;
	CChars*			pszNext;
	CChars			szStored;
	CChars			szFrom;
	CChars			szTo;
	CChars			szLoaded;
	size			ui;
	int				aiWidths[IR65_CODE_REGISTERS];
	int				iFrom;
	int				iTo;
	uint8			uiBase;
	int32			iOffset;

	szText.Init(mpszOut->Text() + uiStart);
	aszLines.Init();
	szText.SplitLines(&aszLines);
	szText.Kill();
	mpszOut->SetLength(uiStart);

	IR65ResetWidths(aiWidths);
	for (ui = 0; ui < aszLines.NumElements(); ui++)
	{
		pszLine = aszLines.Get(ui);
		if (!pszLine->StartsWith("\t\t"))
		{
			//Each block starts with all four 32 bits wide.
			IR65ResetWidths(aiWidths);
		}
		else if ((pszLine->Length() == 2) || IR65BranchToNext(&aszLines, ui))
		{
			continue;
		}
		else if (ui + 1 < aszLines.NumElements())
		{
			pszNext = aszLines.Get(ui + 1);
			szStored.Init();
			szFrom.Init();
			szTo.Init();
			szLoaded.Init();
			if (IR65CopyOperands(pszLine, &szStored, &szFrom) && IR65CopyOperands(pszNext, &szTo, &szLoaded) && CIR65Code::IsOwnMemory(szStored.Text(), szStored.Length(), &uiBase, &iOffset) && szLoaded.Equals(szStored))
			{
				iFrom = CIR65Code::RegisterIndex(szFrom.Text(), szFrom.Length());
				iTo = CIR65Code::RegisterIndex(szTo.Text(), szTo.Length());
				if ((iFrom >= 0) && (iTo >= 0) && (aiWidths[iFrom] == 32) && (aiWidths[iTo] == 32))
				{
					pszNext->Kill();
					pszNext->Init("\t\t");
					if (iFrom != iTo)
					{
						pszNext->Append("cpy ")->Append(szTo)->Append(", ")->Append(szFrom);
					}
				}
			}
			szLoaded.Kill();
			szTo.Kill();
			szFrom.Kill();
			szStored.Kill();
		}
		IR65UpdateWidths(pszLine, aiWidths);
		mpszOut->Append(*pszLine);
		if (ui + 1 < aszLines.NumElements())
		{
			//The last piece is what follows the last new line.
			mpszOut->AppendNewLine();
		}
	}
	aszLines.Kill();
}
