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
#include "IR816Backend.h"
#include "IR816Code.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Code::Init(void)
{
	masLines.Init(CIR816Backend::GetScratch());
	maiOrder.Init(CIR816Backend::GetScratch());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Code::Kill(void)
{
	Clear();
	maiOrder.Kill();
	masLines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Every line gone (their texts freed).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Code::Clear(void)
{
	size	ui;

	for (ui = 0; ui < masLines.NumElements(); ui++)
	{
		masLines.Get(ui)->szText.Kill();
	}
	masLines.FakeSetUsedElements(0);
	maiOrder.FakeSetUsedElements(0);
}


//////////////////////////////////////////////////////////////////////////
//
// A new line, decoded, not yet in the code's order.
//
//////////////////////////////////////////////////////////////////////////
SIR816CodeLine* CIR816Code::AddLine(const char* szText, size uiLength)
{
	SIR816CodeLine*	psCode;

	psCode = masLines.Add();
	psCode->szText.Init(CIR816Backend::GetScratch());
	psCode->szText.AppendSubString(szText, uiLength);
	Decode(psCode);
	return psCode;
}


//////////////////////////////////////////////////////////////////////////
//
// The line at uiLine in the code's order.
//
//////////////////////////////////////////////////////////////////////////
SIR816CodeLine* CIR816Code::Line(size uiLine)
{
	return masLines.Get(*maiOrder.Get(uiLine));
}


//////////////////////////////////////////////////////////////////////////
//
// The code from uiStart on, a line each (the new line that ends the last
// isn't a line), each decoded.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Code::Load(CChars* pszCode, size uiStart)
{
	const char*	szLine;
	const char*	szEnd;

	Clear();
	for (szLine = pszCode->Text() + uiStart; *szLine; szLine = szEnd + 1)
	{
		szEnd = strchr(szLine, '\n');
		Append(szLine, szEnd ? (size)(szEnd - szLine) : StrLen(szLine));
		if (szEnd == NULL)
		{
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The lines written back over the code from uiStart on, each ending with a
// new line.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Code::Write(CChars* pszCode, size uiStart)
{
	size	ui;

	pszCode->SetLength(uiStart);
	for (ui = 0; ui < maiOrder.NumElements(); ui++)
	{
		pszCode->Append(Line(ui)->szText);
		pszCode->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIR816Code::NumLines(void)
{
	return maiOrder.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
// A line's text, to read (change it with Set).
//
//////////////////////////////////////////////////////////////////////////
CChars* CIR816Code::GetText(size uiLine)
{
	return &Line(uiLine)->szText;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CIR816Code::Text(size uiLine)
{
	return Line(uiLine)->szText.Text();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line is a label ("L12:" or a function's).
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Code::IsLabel(size uiLine)
{
	return Line(uiLine)->uiKind == IR816CL_LABEL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line may be an instruction: it starts with a tab and has a
// mnemonic's room.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Code::IsTabbed(size uiLine)
{
	return Line(uiLine)->uiKind == IR816CL_TABBED;
}


//////////////////////////////////////////////////////////////////////////
//
// A tabbed line decoded (*psLine, its operand in the line's text), else
// false.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Code::Decoded(size uiLine, SIR816Line* psLine)
{
	SIR816CodeLine*	psCode;

	psCode = Line(uiLine);
	if (psCode->uiKind != IR816CL_TABBED)
	{
		return false;
	}
	*psLine = psCode->sLine;
	psLine->szOperand = psCode->szText.Text() + psCode->uiOperand;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line is an instruction with any of the IR816MF_ bits in
// uiFlags.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Code::Has(size uiLine, uint32 uiFlags)
{
	SIR816CodeLine*	psCode;

	psCode = Line(uiLine);
	return (psCode->uiKind == IR816CL_TABBED) && ((psCode->sLine.uiFlags & uiFlags) != 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line is an instruction (IR816IsInstruction: two tabs and no
// directive), decoded in *psLine.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Code::Instruction(size uiLine, SIR816Line* psLine)
{
	SIR816CodeLine*	psCode;

	psCode = Line(uiLine);
	if (!IR816IsInstruction(psCode->szText.Text()))
	{
		return false;
	}
	if (!Decoded(uiLine, psLine))
	{
		IR816Decode(psCode->szText.Text(), psLine);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A line's text replaced, and decoded again.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Code::Set(size uiLine, const char* szText)
{
	SIR816CodeLine*	psCode;

	psCode = Line(uiLine);
	psCode->szText.Clear();
	psCode->szText.Append(szText);
	Decode(psCode);
}


//////////////////////////////////////////////////////////////////////////
//
// A line taken out of the code (its text stays until the code is cleared,
// as lines don't move).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Code::Remove(size uiLine)
{
	maiOrder.RemoveAt(uiLine, true);
}


//////////////////////////////////////////////////////////////////////////
//
// The lines pabRemove marks (per line, 1 to take out) taken out at once.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Code::RemoveMarked(CArrayTemplate<uint8>* pabRemove)
{
	size	ui;
	size	uiKept;

	uiKept = 0;
	for (ui = 0; ui < maiOrder.NumElements(); ui++)
	{
		if (!*pabRemove->Get(ui))
		{
			*maiOrder.Get(uiKept) = *maiOrder.Get(ui);
			uiKept++;
		}
	}
	maiOrder.FakeSetUsedElements(uiKept);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Code::RemoveMarked(CArrayTemplate<bool>* pabRemove)
{
	size	ui;
	size	uiKept;

	uiKept = 0;
	for (ui = 0; ui < maiOrder.NumElements(); ui++)
	{
		if (!*pabRemove->Get(ui))
		{
			*maiOrder.Get(uiKept) = *maiOrder.Get(ui);
			uiKept++;
		}
	}
	maiOrder.FakeSetUsedElements(uiKept);
}

//////////////////////////////////////////////////////////////////////////
//
// A line inserted before line uiLine (after the last for NumLines).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Code::Insert(size uiLine, const char* szText)
{
	size	uiIndex;

	uiIndex = masLines.NumElements();
	AddLine(szText, StrLen(szText));
	*maiOrder.InsertAt(uiLine) = uiIndex;
}


//////////////////////////////////////////////////////////////////////////
//
// A line added after the last: uiLength characters of szText (a pass
// building the code again, CIR816Code::Take).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Code::Append(const char* szText, size uiLength)
{
	size	uiIndex;

	uiIndex = masLines.NumElements();
	AddLine(szText, uiLength);
	*maiOrder.Add() = uiIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Code::Append(const char* szText)
{
	Append(szText, StrLen(szText));
}


//////////////////////////////////////////////////////////////////////////
//
// The lines from uiLines on taken out.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Code::Truncate(size uiLines)
{
	if (maiOrder.NumElements() > uiLines)
	{
		maiOrder.FakeSetUsedElements(uiLines);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The lines of pcFrom (a pass's new code) in place of these; pcFrom is left
// with these (freed when it is killed).  The arrays are swapped whole: they
// hold nothing that points at themselves.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Code::Take(CIR816Code* pcFrom)
{
	CArrayIR816CodeLines	asLines;
	CArrayIR816LineOrder	aiOrder;

	memcpy(&asLines, &masLines, sizeof(CArrayIR816CodeLines));
	memcpy(&aiOrder, &maiOrder, sizeof(CArrayIR816LineOrder));
	memcpy(&masLines, &pcFrom->masLines, sizeof(CArrayIR816CodeLines));
	memcpy(&maiOrder, &pcFrom->maiOrder, sizeof(CArrayIR816LineOrder));
	memcpy(&pcFrom->masLines, &asLines, sizeof(CArrayIR816CodeLines));
	memcpy(&pcFrom->maiOrder, &aiOrder, sizeof(CArrayIR816LineOrder));
}


//////////////////////////////////////////////////////////////////////////
//
// A line's kind and, when tabbed, its decoding.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Code::Decode(SIR816CodeLine* psCode)
{
	CChars*	psz;

	psz = &psCode->szText;
	memset(&psCode->sLine, 0, sizeof(SIR816Line));
	psCode->uiOperand = 0;
	if ((psz->Length() > 1) && (psz->Text()[0] != '\t') && (psz->Text()[psz->Length() - 1] == ':'))
	{
		psCode->uiKind = IR816CL_LABEL;
	}
	else if ((psz->Length() >= 3) && (psz->Text()[0] == '\t'))
	{
		psCode->uiKind = IR816CL_TABBED;
		IR816Decode(psz->Text(), &psCode->sLine);
		psCode->uiOperand = (size)(psCode->sLine.szOperand - psz->Text());
		psCode->sLine.szOperand = NULL;
	}
	else
	{
		psCode->uiKind = IR816CL_OTHER;
	}
}
