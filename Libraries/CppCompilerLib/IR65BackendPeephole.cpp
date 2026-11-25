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
#include <stdlib.h>
#include <string.h>
#include "BaseLib/StringHelper.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/IntegerHelper.h"
#include "IR65BackendInternal.h"


//The 65C64 back end's first and last passes over a function's code (PeepholeCode, IR65PseudoWithoutW): gasIR65Passes
//(IR65Backend.cpp) has the passes between, CIR65Flags, CIR65Widths and CIR65Peephole.


//////////////////////////////////////////////////////////////////////////
//
// The pseudo registers are $0000 to $00FF whatever W holds (it is a register
// like any other: the user, 2026-11-14), so they are written [n], not
// [n + W].  The lowering and the passes before write and see [n + W]
// everywhere (it is how they tell a pseudo register from other memory), so
// it is done last.
//
//////////////////////////////////////////////////////////////////////////
bool IR65PseudoWithoutW(CIR65Code* pcCode)
{
	CChars*	pszLine;
	size	ui;
	bool	bChanged;

	bChanged = false;
	for (ui = 0; ui < pcCode->NumLines(); ui++)
	{
		pszLine = pcCode->GetText(ui);
		if (pszLine->Contains(" + W]"))
		{
			pszLine->Replace(" + W]", "]");
			bChanged = true;
		}
	}
	return bChanged;
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
// Whether pszLine (a label) is a block's (one of paiBlockLabels, "Ln:") or
// the function's own: the registers are 32 bits wide there.  A label in an
// instruction's code (ShiftSigned's) may be reached with other widths.
//
//////////////////////////////////////////////////////////////////////////
static bool IR65IsBlockLabel(CChars* pszLine, CArrayTemplate<int32>* paiBlockLabels)
{
	int32	iLabel;
	size	ui;

	if (!pszLine->StartsWith("L"))
	{
		return true;
	}
	iLabel = (int32)atoi(pszLine->Text() + 1);
	for (ui = 0; ui < paiBlockLabels->NumElements(); ui++)
	{
		if (*paiBlockLabels->Get(ui) == iLabel)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
// Cleans up pcCode, whose block labels are paiBlockLabels: a value stored to a
// pseudo register or the frame then read straight back (at 32 bits) is
// copied from the register it was stored from instead (or not at all into
// the same register: the store set the same flags).  A branch to the label
// after it is left for CIR65Peephole (IRRemoveBranchesToNext).
//
////////////////////////////////////////////////////////////////////////
// bHomeV: V is a home there (SIR65Frame::bHomeV).  True if it changed
// anything.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::PeepholeCode(CIR65Code* pcCode, CArrayTemplate<int32>* paiBlockLabels, bool bHomeV)
{
	CChars*			pszLine;
	CChars*			pszNext;
	CChars			szStored;
	CChars			szFrom;
	CChars			szTo;
	CChars			szLoaded;
	size			ui;
	size			uiNum;
	int				aiWidths[IR65_CODE_REGISTERS];
	int				iFrom;
	int				iTo;
	uint8			uiBase;
	int32			iOffset;
	bool			bChanged;

	bChanged = false;
	uiNum = pcCode->NumLines();
	IR65ResetWidths(aiWidths);
	for (ui = 0; ui < uiNum; ui++)
	{
		pszLine = pcCode->GetText(ui);
		if (!pszLine->StartsWith("\t\t"))
		{
			//Each block starts with all four 32 bits wide; at another label they aren't known.
			if (IR65IsBlockLabel(pszLine, paiBlockLabels))
			{
				IR65ResetWidths(aiWidths);
			}
			else
			{
				memset(aiWidths, 0, sizeof(aiWidths));
			}
		}
		else if (pszLine->Length() == 2)
		{
			pcCode->Drop(ui);
			bChanged = true;
			continue;
		}
		else if (ui + 1 < uiNum)
		{
			pszNext = pcCode->GetText(ui + 1);
			szStored.Init();
			szFrom.Init();
			szTo.Init();
			szLoaded.Init();
			if (IR65CopyOperands(pszLine, &szStored, &szFrom) && IR65CopyOperands(pszNext, &szTo, &szLoaded) && CIR65Code::IsOwnMemory(szStored.Text(), szStored.Length(), bHomeV, &uiBase, &iOffset) && szLoaded.Equals(szStored))
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
					bChanged = true;
				}
			}
			szLoaded.Kill();
			szTo.Kill();
			szFrom.Kill();
			szStored.Kill();
		}
		IR65UpdateWidths(pszLine, aiWidths);
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// PeepholeCode on pszCode from uiStart, written again.  For the tests.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::PeepholeCode(CChars* pszCode, size uiStart, CArrayTemplate<int32>* paiBlockLabels, bool bHomeV)
{
	CIR65Code	cCode;

	cCode.Init();
	cCode.SetText(pszCode, uiStart);
	PeepholeCode(&cCode, paiBlockLabels, bHomeV);
	pszCode->SetLength(uiStart);
	cCode.Write(pszCode);
	cCode.Kill();
}
