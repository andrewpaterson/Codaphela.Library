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
#include "IRW65C816Backend.h"
#include "IRW65C816Code.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::Init(void)
{
	masLines.Init(CIRW65C816Backend::GetScratch());
	maiOrder.Init(CIRW65C816Backend::GetScratch());
	memset(mabKept, 0, sizeof(mabKept));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::Kill(void)
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
void CIRW65C816Code::Clear(void)
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
SIRW65C816CodeLine* CIRW65C816Code::AddLine(const char* szText, size uiLength)
{
	SIRW65C816CodeLine*	psCode;

	psCode = masLines.Add();
	psCode->szText.Init(CIRW65C816Backend::GetScratch());
	psCode->szText.AppendSubString(szText, uiLength);
	psCode->uiNote = 0;
	Decode(psCode);
	return psCode;
}


//////////////////////////////////////////////////////////////////////////
//
// The line at uiLine in the code's order.
//
//////////////////////////////////////////////////////////////////////////
SIRW65C816CodeLine* CIRW65C816Code::Line(size uiLine)
{
	return masLines.Get(*maiOrder.Get(uiLine));
}


//////////////////////////////////////////////////////////////////////////
//
// The code from uiStart on, a line each (the new line that ends the last
// isn't a line), each decoded.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::Load(CChars* pszCode, size uiStart)
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
void CIRW65C816Code::Write(CChars* pszCode, size uiStart)
{
	size	ui;

	pszCode->SetLength(uiStart);
	for (ui = 0; ui < maiOrder.NumElements(); ui++)
	{
		PrintLine(ui, pszCode);
		pszCode->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A line as the printer writes it (without its new line), appended to psz.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::PrintLine(size uiLine, CChars* psz)
{
	SIRW65C816CodeLine*	psCode;

	psCode = Line(uiLine);
	if (psCode->bText)
	{
		psz->Append(psCode->szText);
	}
	else
	{
		IR816AppendLine(psz, psCode->sLine.uiMnemonic, psCode->szText.Text(), psCode->szText.Length(), psCode->sLine.bMarked);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIRW65C816Code::NumLines(void) const
{
	return maiOrder.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
// A label's, directive's or comment's text (an instruction's is its
// fields: Decoded).
//
//////////////////////////////////////////////////////////////////////////
const char* CIRW65C816Code::Text(size uiLine)
{
	return Line(uiLine)->szText.Text();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line is a label ("L12:" or a function's).
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Code::IsLabel(size uiLine)
{
	return Line(uiLine)->uiKind == IR816CL_LABEL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line may be an instruction: it starts with a tab and has a
// mnemonic's room.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Code::IsTabbed(size uiLine)
{
	return Line(uiLine)->uiKind == IR816CL_TABBED;
}


//////////////////////////////////////////////////////////////////////////
//
// A tabbed line decoded (*psLine, its operand in the line's text), else
// false.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Code::Decoded(size uiLine, SIRW65C816Line* psLine)
{
	SIRW65C816CodeLine*	psCode;

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
bool CIRW65C816Code::Has(size uiLine, uint32 uiFlags)
{
	SIRW65C816CodeLine*	psCode;

	psCode = Line(uiLine);
	return (psCode->uiKind == IR816CL_TABBED) && ((psCode->sLine.uiFlags & uiFlags) != 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line is an instruction (IR816IsInstruction: two tabs and no
// directive), decoded in *psLine.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Code::Instruction(size uiLine, SIRW65C816Line* psLine)
{
	SIRW65C816CodeLine*	psCode;

	psCode = Line(uiLine);
	if (!psCode->bText)
	{
		return Decoded(uiLine, psLine);
	}
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
// Whether a line is the instruction uiMnemonic alone (no operand or mark).
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Code::IsBare(size uiLine, uint16 uiMnemonic)
{
	SIRW65C816CodeLine*	psCode;

	psCode = Line(uiLine);
	return (psCode->uiKind == IR816CL_TABBED) && (psCode->sLine.uiMnemonic == uiMnemonic) && (psCode->sLine.uiOperand == 0) && !psCode->sLine.bMarked;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line is the instruction uiMnemonic with exactly the operand
// szOperand (and no mark).
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Code::IsWith(size uiLine, uint16 uiMnemonic, const char* szOperand)
{
	SIRW65C816CodeLine*	psCode;
	size				uiOperand;

	psCode = Line(uiLine);
	uiOperand = StrLen(szOperand);
	return (psCode->uiKind == IR816CL_TABBED) && (psCode->sLine.uiMnemonic == uiMnemonic) && !psCode->sLine.bMarked &&
		   (psCode->sLine.uiOperand == uiOperand) && (strncmp(psCode->szText.Text() + psCode->uiOperand, szOperand, uiOperand) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line is an instruction held as its fields (not a label,
// directive or comment, nor a line kept as text).
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Code::IsInstructionLine(size uiLine)
{
	return !Line(uiLine)->bText;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line is a directive (".width ...").
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Code::IsDirective(size uiLine)
{
	SIRW65C816CodeLine*	psCode;

	psCode = Line(uiLine);
	return psCode->bText && (psCode->uiKind == IR816CL_TABBED) && (psCode->szText.Text()[2] == '.');
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line is a comment (a tabbed line starting ';').
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Code::IsComment(size uiLine)
{
	SIRW65C816CodeLine*	psCode;

	psCode = Line(uiLine);
	return psCode->bText && (psCode->uiKind == IR816CL_TABBED) && (psCode->szText.Text()[2] == ';');
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line is a bra JumpTo marked as going round a loop
// (IR816_ROUND).
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Code::IsRound(size uiLine)
{
	return Line(uiLine)->szText.EndsWith(IR816_ROUND);
}


//////////////////////////////////////////////////////////////////////////
//
// A label line's name (without its colon) and its length, else NULL.
//
//////////////////////////////////////////////////////////////////////////
const char* CIRW65C816Code::LabelName(size uiLine, size* puiLength)
{
	SIRW65C816CodeLine*	psCode;

	psCode = Line(uiLine);
	if (psCode->uiKind != IR816CL_LABEL)
	{
		return NULL;
	}
	*puiLength = psCode->szText.Length() - 1;
	return psCode->szText.Text();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line is the label szLabel (uiLength long).
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Code::IsLabelNamed(size uiLine, const char* szLabel, size uiLength)
{
	const char*	szName;
	size		uiName;

	szName = LabelName(uiLine, &uiName);
	return szName && (uiName == uiLength) && (strncmp(szName, szLabel, uiLength) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The number of a local label's line ("L12:"), else -1.
//
//////////////////////////////////////////////////////////////////////////
int64 CIRW65C816Code::LocalLabelNumber(size uiLine)
{
	const char*	szName;
	size		uiName;

	szName = LabelName(uiLine, &uiName);
	return szName ? IR816LocalLabelNumber(szName, uiName) : -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the names a line mentions are: a tabbed line's operand, a label's
// name, anything else's text.
//
//////////////////////////////////////////////////////////////////////////
const char* CIRW65C816Code::Mentions(size uiLine, size* puiLength)
{
	SIRW65C816CodeLine*	psCode;

	psCode = Line(uiLine);
	if (psCode->uiKind == IR816CL_TABBED)
	{
		*puiLength = psCode->sLine.uiOperand;
		return psCode->szText.Text() + psCode->uiOperand;
	}
	if (psCode->uiKind == IR816CL_LABEL)
	{
		*puiLength = psCode->szText.Length() - 1;
		return psCode->szText.Text();
	}
	*puiLength = psCode->szText.Length();
	return psCode->szText.Text();
}


//////////////////////////////////////////////////////////////////////////
//
// How many times szText is in the lines from uiFrom on (in what they
// mention: Mentions).
//
//////////////////////////////////////////////////////////////////////////
int32 CIRW65C816Code::CountMentions(size uiFrom, const char* szText)
{
	const char*	szLine;
	size		uiLength;
	size		uiText;
	size		ui;
	size		uj;
	int32		iCount;

	uiText = StrLen(szText);
	iCount = 0;
	for (ui = uiFrom; ui < NumLines(); ui++)
	{
		szLine = Mentions(ui, &uiLength);
		for (uj = 0; uj + uiText <= uiLength; uj++)
		{
			if (strncmp(szLine + uj, szText, uiText) == 0)
			{
				iCount++;
				uj += uiText - 1;
			}
		}
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
// A line made the instruction uiMnemonic with szOperand (NULL for none).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::SetInstruction(size uiLine, uint16 uiMnemonic, const char* szOperand)
{
	MakeInstruction(Line(uiLine), uiMnemonic, szOperand, szOperand ? StrLen(szOperand) : 0, false);
}


//////////////////////////////////////////////////////////////////////////
//
// A line made the branch uiMnemonic to szLabel (uiLabel long).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::SetBranch(size uiLine, uint16 uiMnemonic, const char* szLabel, size uiLabel)
{
	MakeInstruction(Line(uiLine), uiMnemonic, szLabel, uiLabel, false);
}


//////////////////////////////////////////////////////////////////////////
//
// A bra's IR816_ROUND mark taken off.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::ClearRound(size uiLine)
{
	SIRW65C816CodeLine*	psCode;

	psCode = Line(uiLine);
	if (!psCode->szText.EndsWith(IR816_ROUND))
	{
		return;
	}
	psCode->szText.SetLength(psCode->szText.Length() - StrLen(IR816_ROUND));
	if (psCode->bText)
	{
		Decode(psCode);
	}
	else
	{
		IR816MakeLine(&psCode->sLine, psCode->sLine.uiMnemonic, psCode->szText.Text(), psCode->szText.Length(), psCode->sLine.bMarked);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The instruction uiMnemonic with szOperand (NULL for none) added after
// the last line.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::AppendInstruction(uint16 uiMnemonic, const char* szOperand, bool bMarked)
{
	MakeInstruction(NewLine(), uiMnemonic, szOperand, szOperand ? StrLen(szOperand) : 0, bMarked);
}


//////////////////////////////////////////////////////////////////////////
//
// The instruction uiMnemonic with the number lliOperand added after the
// last line.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::AppendInstruction(uint16 uiMnemonic, int64 lliOperand)
{
	CChars	sz;

	sz.Init(CIRW65C816Backend::GetScratch());
	sz.Append(lliOperand);
	AppendInstruction(uiMnemonic, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An instruction as psLine decodes one (its mnemonic and operand), with
// the IR816_KEEP_IN_A mark or without, added after the last line.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::AppendInstruction(SIRW65C816Line* psLine, bool bMarked)
{
	SIRW65C816CodeLine*	psCode;
	CChars				sz;

	psCode = NewLine();
	if (psLine->uiMnemonic != IR816M_Unknown)
	{
		MakeInstruction(psCode, psLine->uiMnemonic, psLine->szOperand, psLine->uiOperand, bMarked);
		return;
	}
	sz.Init(CIRW65C816Backend::GetScratch());
	sz.Append("\t\t");
	sz.Append(psLine->szMnemonic);
	if (psLine->uiOperand > 0)
	{
		sz.Append(' ');
		sz.AppendSubString(psLine->szOperand, psLine->uiOperand);
	}
	if (bMarked)
	{
		sz.Append(IR816_KEEP_IN_A);
	}
	psCode->szText.Append(sz);
	Decode(psCode);
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Line uiLine of pcFrom added after the last line.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::AppendLine(CIRW65C816Code* pcFrom, size uiLine)
{
	SIRW65C816CodeLine*	psFrom;
	SIRW65C816CodeLine*	psCode;

	psFrom = pcFrom->Line(uiLine);
	psCode = NewLine();
	psCode->uiKind = psFrom->uiKind;
	psCode->uiNote = psFrom->uiNote;
	psCode->bText = psFrom->bText;
	psCode->sLine = psFrom->sLine;
	psCode->uiOperand = psFrom->uiOperand;
	psCode->szText.Append(psFrom->szText);
}


//////////////////////////////////////////////////////////////////////////
//
// The instruction uiMnemonic with szOperand (NULL for none) inserted
// before line uiLine (after the last for NumLines).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::InsertInstruction(size uiLine, uint16 uiMnemonic, const char* szOperand)
{
	size	uiIndex;

	NewLine();
	uiIndex = *maiOrder.Get(maiOrder.NumElements() - 1);
	maiOrder.RemoveAt(maiOrder.NumElements() - 1, true);
	*maiOrder.InsertAt(uiLine) = uiIndex;
	MakeInstruction(masLines.Get(uiIndex), uiMnemonic, szOperand, szOperand ? StrLen(szOperand) : 0, false);
}


//////////////////////////////////////////////////////////////////////////
//
// The label szName's line ("szName:") added after the last line.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::AppendLabel(const char* szName)
{
	SIRW65C816CodeLine*	psCode;

	psCode = NewLine();
	psCode->uiKind = IR816CL_LABEL;
	psCode->szText.Append(szName);
	psCode->szText.Append(':');
}


//////////////////////////////////////////////////////////////////////////
//
// The directive szName (".width") with szOperand (NULL for none) added
// after the last line: its text, with what IR816Decode would make of it
// (no mnemonic the passes know, the operand after its name).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::AppendDirective(const char* szName, const char* szOperand)
{
	SIRW65C816CodeLine*	psCode;

	psCode = NewLine();
	psCode->uiKind = IR816CL_TABBED;
	psCode->szText.Append("\t\t");
	psCode->szText.Append(szName);
	if (szOperand)
	{
		psCode->szText.Append(' ');
		psCode->szText.Append(szOperand);
	}
	StrCpySafe(psCode->sLine.szMnemonic, szName, sizeof(psCode->sLine.szMnemonic));
	psCode->sLine.uiMnemonic = IR816M_Unknown;
	psCode->sLine.uiFlags = IR816UnknownFlags(szName);
	psCode->uiOperand = 2 + StrLen(szName) + (szOperand ? 1 : 0);
	psCode->sLine.uiOperand = psCode->szText.Length() - psCode->uiOperand;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether line uiLine is line uiOther of pcOther: the same instruction
// (mnemonic, operand and mark), or the same text.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Code::SameLine(size uiLine, CIRW65C816Code* pcOther, size uiOther)
{
	SIRW65C816CodeLine*	psLine;
	SIRW65C816CodeLine*	psOther;

	psLine = Line(uiLine);
	psOther = pcOther->Line(uiOther);
	if (psLine->bText != psOther->bText)
	{
		return false;
	}
	if (!psLine->bText && ((psLine->sLine.uiMnemonic != psOther->sLine.uiMnemonic) || (psLine->sLine.bMarked != psOther->sLine.bMarked)))
	{
		return false;
	}
	return psLine->szText.Equals(psOther->szText.Text());
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the lines from uiLine on are pcLines' (all of them, in order).
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Code::LinesAt(size uiLine, CIRW65C816Code* pcLines)
{
	size	ui;

	if (uiLine + pcLines->NumLines() > NumLines())
	{
		return false;
	}
	for (ui = 0; ui < pcLines->NumLines(); ui++)
	{
		if (!SameLine(uiLine + ui, pcLines, ui))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::SetNote(size uiLine, uint8 uiNote)
{
	Line(uiLine)->uiNote = uiNote;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 CIRW65C816Code::Note(size uiLine)
{
	return Line(uiLine)->uiNote;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line is the instruction uiMnemonic (no mark) whose operand is
// szOpen, a decimal number and szClose ("adc 36" with "" and "", "lda
// [D + 33]" with "[D + " and "]").
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Code::IsNumberLine(size uiLine, uint16 uiMnemonic, const char* szOpen, const char* szClose)
{
	SIRW65C816CodeLine*	psCode;
	const char*			szOperand;
	size				uiOpen;
	size				uiClose;
	size				ui;
	size				uiLength;

	psCode = Line(uiLine);
	if (psCode->bText || (psCode->sLine.uiMnemonic != uiMnemonic) || psCode->sLine.bMarked)
	{
		return false;
	}
	szOperand = psCode->szText.Text();
	uiLength = psCode->szText.Length();
	uiOpen = StrLen(szOpen);
	uiClose = StrLen(szClose);
	if ((uiLength < uiOpen + 1 + uiClose) || (strncmp(szOperand, szOpen, uiOpen) != 0) || (strncmp(szOperand + uiLength - uiClose, szClose, uiClose) != 0))
	{
		return false;
	}
	for (ui = uiOpen; ui < uiLength - uiClose; ui++)
	{
		if ((szOperand[ui] < '0') || (szOperand[ui] > '9'))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A new line after the last, empty.
//
//////////////////////////////////////////////////////////////////////////
SIRW65C816CodeLine* CIRW65C816Code::NewLine(void)
{
	SIRW65C816CodeLine*	psCode;
	size				uiIndex;

	uiIndex = masLines.NumElements();
	psCode = masLines.Add();
	psCode->szText.Init(CIRW65C816Backend::GetScratch());
	psCode->uiKind = IR816CL_OTHER;
	psCode->uiNote = 0;
	psCode->bText = true;
	memset(&psCode->sLine, 0, sizeof(SIRW65C816Line));
	psCode->uiOperand = 0;
	*maiOrder.Add() = uiIndex;
	return psCode;
}


//////////////////////////////////////////////////////////////////////////
//
// A line made the instruction uiMnemonic with szOperand (uiOperand long)
// and the IR816_KEEP_IN_A mark or not.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::MakeInstruction(SIRW65C816CodeLine* psCode, uint16 uiMnemonic, const char* szOperand, size uiOperand, bool bMarked)
{
	CChars	szOperandCopy;

	//The operand may be the line's own.
	szOperandCopy.Init(CIRW65C816Backend::GetScratch());
	szOperandCopy.AppendSubString(szOperand, uiOperand);
	psCode->szText.Clear();
	psCode->szText.Append(szOperandCopy);
	szOperandCopy.Kill();
	psCode->uiKind = IR816CL_TABBED;
	psCode->bText = false;
	psCode->uiOperand = 0;
	IR816MakeLine(&psCode->sLine, uiMnemonic, psCode->szText.Text(), psCode->szText.Length(), bMarked);
	psCode->sLine.szOperand = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A line's text replaced, and decoded again.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::Set(size uiLine, const char* szText)
{
	SIRW65C816CodeLine*	psCode;

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
void CIRW65C816Code::Remove(size uiLine)
{
	maiOrder.RemoveAt(uiLine, true);
}


//////////////////////////////////////////////////////////////////////////
//
// The lines pabRemove marks (per line, 1 to take out) taken out at once.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::RemoveMarked(CArrayTemplate<uint8>* pabRemove)
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
void CIRW65C816Code::RemoveMarked(CArrayTemplate<bool>* pabRemove)
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
void CIRW65C816Code::Insert(size uiLine, const char* szText)
{
	size	uiIndex;

	uiIndex = masLines.NumElements();
	AddLine(szText, StrLen(szText));
	*maiOrder.InsertAt(uiLine) = uiIndex;
}


//////////////////////////////////////////////////////////////////////////
//
// A line added after the last: uiLength characters of szText (a pass
// building the code again, CIRW65C816Code::Take).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::Append(const char* szText, size uiLength)
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
void CIRW65C816Code::Append(const char* szText)
{
	Append(szText, StrLen(szText));
}


//////////////////////////////////////////////////////////////////////////
//
// The lines from uiLines on taken out.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::Truncate(size uiLines)
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
void CIRW65C816Code::Take(CIRW65C816Code* pcFrom)
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
void CIRW65C816Code::Decode(SIRW65C816CodeLine* psCode)
{
	CChars*	psz;

	psz = &psCode->szText;
	memset(&psCode->sLine, 0, sizeof(SIRW65C816Line));
	psCode->uiOperand = 0;
	psCode->bText = true;
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
		if (IR816IsInstruction(psz->Text()) && (psCode->sLine.uiMnemonic != IR816M_Unknown) && IR816IsPrinted(psz, &psCode->sLine, psCode->uiOperand))
		{
			//As the printer writes it: the line is its fields.
			psz->RemoveFromStart((int)psCode->uiOperand);
			psz->SetLength(psCode->sLine.uiOperand);
			psCode->uiOperand = 0;
			psCode->bText = false;
		}
	}
	else
	{
		psCode->uiKind = IR816CL_OTHER;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// iBytes from [D + iOffset] are a variable a debugger reads (code for
// one, CCPPCompileOptions::mbDebuggable): no pass takes a store to them out,
// as if the function's end read them.  Past the direct page nothing is
// taken out anyway.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::SetKept(int32 iOffset, int32 iBytes)
{
	int32	i;

	for (i = iOffset; (i < iOffset + iBytes) && (i < IR816_DIRECT_PAGE); i++)
	{
		if (i >= 0)
		{
			mabKept[i] = 1;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether any of iBytes from [D + iOffset] is kept (SetKept).
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Code::IsKept(int32 iOffset, int32 iBytes)
{
	int32	i;

	for (i = iOffset; (i < iOffset + iBytes) && (i < IR816_DIRECT_PAGE); i++)
	{
		if ((i >= 0) && mabKept[i])
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The kept bytes marked read in abRead (a byte for each of the direct
// page's, IR816FindReads).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Code::MarkKept(uint8* abRead)
{
	int32	i;

	for (i = 0; i < IR816_DIRECT_PAGE; i++)
	{
		abRead[i] |= mabKept[i];
	}
}
