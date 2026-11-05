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
#include "IR816Code.h"


//The W65C816 back end's branches tidied (PeepholeBranches), and the code nothing reaches taken out.


//////////////////////////////////////////////////////////////////////////
//
// Whether a line is a branch (bra or a conditional one) to a label: the
// line decoded and its label (without the colon).
//
//////////////////////////////////////////////////////////////////////////
static bool IR816BranchLine(CIR816Code* pcCode, size uiLine, SIR816Line* psLine, char* szLabel)
{
	size	ui;

	if (!pcCode->Decoded(uiLine, psLine))
	{
		return false;
	}
	if (!(psLine->uiFlags & IR816MF_BRANCH) || (psLine->uiOperand < 2) || (psLine->uiOperand > 30) || (psLine->szOperand[0] != 'L'))
	{
		return false;
	}
	for (ui = 0; (ui < psLine->uiOperand) && (psLine->szOperand[ui] != ' '); ui++)
	{
		szLabel[ui] = psLine->szOperand[ui];
	}
	szLabel[ui] = '\0';
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the lines from uiFrom up to the first instruction are labels
// with szLabel among them.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816LabelFollows(CIR816Code* pcCode, size uiFrom, const char* szLabel)
{
	size	ui;
	size	uiLength;
	CChars*	psz;

	uiLength = StrLen(szLabel);
	for (ui = uiFrom; (ui < pcCode->NumLines()) && pcCode->IsLabel(ui); ui++)
	{
		psz = pcCode->GetText(ui);
		if ((psz->Length() == uiLength + 1) && (strncmp(psz->Text(), szLabel, uiLength) == 0))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The number of a local label's name ("L12", or "L12:" with bColon), else -1.
//
//////////////////////////////////////////////////////////////////////////
static int64 IR816LocalLabelNumber(const char* szLabel, bool bColon)
{
	size	ui;

	if ((szLabel[0] != 'L') || (szLabel[1] < '0') || (szLabel[1] > '9') || ((szLabel[1] == '0') && (szLabel[2] >= '0') && (szLabel[2] <= '9')))
	{
		return -1;
	}
	for (ui = 1; (szLabel[ui] >= '0') && (szLabel[ui] <= '9'); ui++)
	{
	}
	if (bColon ? ((szLabel[ui] != ':') || (szLabel[ui + 1] != '\0')) : (szLabel[ui] != '\0'))
	{
		return -1;
	}
	return (ui <= 10) ? atoll(&szLabel[1]) : -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The line local label uiNumber is defined on (its index + 1 in pauiLines, 0 for none), made again from the code
// when it is out of date: lines move as others are removed, so the line found is checked to be the label's.
// Labels are defined once.  Returns the line, or -1.
//
//////////////////////////////////////////////////////////////////////////
static size IR816LocalLabelLine(CIR816Code* pcCode, CArrayTemplate<size>* pauiLines, int64 lliNumber, const char* szLabel)
{
	size	ui;
	size	uiLine;
	size	uiZero;
	int64	lliDefined;
	CChars*	psz;

	uiLine = ((size)lliNumber < pauiLines->NumElements()) ? *pauiLines->Get((size)lliNumber) : 0;
	if ((uiLine != 0) && (uiLine <= pcCode->NumLines()))
	{
		psz = pcCode->GetText(uiLine - 1);
		if ((psz->Length() == StrLen(szLabel) + 1) && (strncmp(psz->Text(), szLabel, psz->Length() - 1) == 0) && (psz->Text()[psz->Length() - 1] == ':'))
		{
			return uiLine - 1;
		}
	}

	uiZero = 0;
	pauiLines->FakeSetUsedElements(0);
	for (ui = 0; ui < pcCode->NumLines(); ui++)
	{
		psz = pcCode->GetText(ui);
		lliDefined = pcCode->IsLabel(ui) ? IR816LocalLabelNumber(psz->Text(), true) : -1;
		if (lliDefined < 0)
		{
			continue;
		}
		while (pauiLines->NumElements() <= (size)lliDefined)
		{
			pauiLines->Add(&uiZero);
		}
		if (*pauiLines->Get((size)lliDefined) == 0)
		{
			*pauiLines->Get((size)lliDefined) = ui + 1;
		}
	}
	uiLine = ((size)lliNumber < pauiLines->NumElements()) ? *pauiLines->Get((size)lliNumber) : 0;
	return (uiLine != 0) ? uiLine - 1 : (size)-1;
}


//////////////////////////////////////////////////////////////////////////
//
// The index of the first instruction after label szLabel, else -1.  pauiLabelLines is where local labels are
// defined (IR816LocalLabelLine), so they aren't searched for.
//
//////////////////////////////////////////////////////////////////////////
static size IR816LabelledInstruction(CIR816Code* pcCode, const char* szLabel, CArrayTemplate<size>* pauiLabelLines)
{
	size	ui;
	size	uiLength;
	int64	lliNumber;
	CChars*	psz;

	lliNumber = IR816LocalLabelNumber(szLabel, false);
	if (lliNumber >= 0)
	{
		ui = IR816LocalLabelLine(pcCode, pauiLabelLines, lliNumber, szLabel);
	}
	else
	{
		uiLength = StrLen(szLabel);
		for (ui = 0; ui < pcCode->NumLines(); ui++)
		{
			psz = pcCode->GetText(ui);
			if (pcCode->IsLabel(ui) && (psz->Length() == uiLength + 1) && (strncmp(psz->Text(), szLabel, uiLength) == 0))
			{
				break;
			}
		}
		if (ui == pcCode->NumLines())
		{
			ui = (size)-1;
		}
	}
	if (ui == (size)-1)
	{
		return (size)-1;
	}
	for (ui++; (ui < pcCode->NumLines()) && pcCode->IsLabel(ui); ui++)
	{
	}
	return (ui < pcCode->NumLines()) ? ui : (size)-1;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether c may be part of a label's name.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816IsNameCharacter(char c)
{
	return ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9')) || (c == '_');
}


//////////////////////////////////////////////////////////////////////////
//
// Adds iChange to the count of each local label (L and a number) an instruction line names, in paiMentions by
// number.  A name is bounded by characters that can't be in a name, as IR816LabelUsed matches it.
//
//////////////////////////////////////////////////////////////////////////
static void IR816CountMentions(const char* szLine, CArrayTemplate<int32>* paiMentions, int32 iChange)
{
	const char*	szAt;
	const char*	szEnd;
	int64		lliNumber;
	int32		iZero;

	iZero = 0;
	for (szAt = szLine; (szAt = strchr(szAt, 'L')) != NULL; szAt++)
	{
		if ((szAt == szLine) || IR816IsNameCharacter(szAt[-1]) || (szAt[1] < '0') || (szAt[1] > '9'))
		{
			continue;
		}
		for (szEnd = szAt + 1; (*szEnd >= '0') && (*szEnd <= '9'); szEnd++)
		{
		}
		if (IR816IsNameCharacter(*szEnd) || ((szAt[1] == '0') && (szEnd - szAt > 2)) || (szEnd - szAt > 11))
		{
			continue;
		}
		lliNumber = atoll(szAt + 1);
		while (paiMentions->NumElements() <= (size)lliNumber)
		{
			paiMentions->Add(&iZero);
		}
		*paiMentions->Get((size)lliNumber) += iChange;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a label ("L12:") is named by an instruction's operand.  A local label's mentions are counted
// (IR816CountMentions, made when first needed and kept up to date as lines are removed: *pbCounted); another's
// are searched for.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816LabelUsed(CIR816Code* pcCode, CChars* pszLabel, CArrayTemplate<int32>* paiMentions, bool* pbCounted)
{
	size		ui;
	size		uiLength;
	int64		lliNumber;
	const char*	szAt;
	const char*	szText;

	lliNumber = IR816LocalLabelNumber(pszLabel->Text(), true);
	if (lliNumber >= 0)
	{
		if (!*pbCounted)
		{
			paiMentions->FakeSetUsedElements(0);
			for (ui = 0; ui < pcCode->NumLines(); ui++)
			{
				szText = pcCode->Text(ui);
				if (szText[0] == '\t')
				{
					IR816CountMentions(szText, paiMentions, 1);
				}
			}
			*pbCounted = true;
		}
		return ((size)lliNumber < paiMentions->NumElements()) && (*paiMentions->Get((size)lliNumber) > 0);
	}

	uiLength = pszLabel->Length() - 1;
	for (ui = 0; ui < pcCode->NumLines(); ui++)
	{
		szText = pcCode->Text(ui);
		if (szText[0] != '\t')
		{
			continue;
		}
		for (szAt = szText; (szAt = strchr(szAt, 'L')) != NULL; szAt++)
		{
			if ((strncmp(szAt, pszLabel->Text(), uiLength) == 0) && !IR816IsNameCharacter(szAt[-1]) && !IR816IsNameCharacter(szAt[uiLength]))
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Removes the instructions after a bra, brl, jml, jmp, rtl, rts or stp
// (IR816MF_ENDS_FLOW) up to a label other than an unused local one
// (IR816LabelUsed), which go too.  True if any did.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816Unreachable(CIR816Code* pcCode)
{
	size					ui;
	CChars*					psz;
	bool					bChanged;
	CArrayTemplate<int32>	aiMentions;
	bool					bCounted;

	bChanged = false;
	aiMentions.Init(CIR816Backend::GetScratch());
	bCounted = false;
	for (ui = 0; ui + 1 < pcCode->NumLines(); ui++)
	{
		if (!pcCode->Has(ui, IR816MF_ENDS_FLOW))
		{
			continue;
		}
		while (ui + 1 < pcCode->NumLines())
		{
			psz = pcCode->GetText(ui + 1);
			if (psz->Empty() || ((psz->Text()[0] != '\t') && ((psz->Text()[0] != 'L') || !pcCode->IsLabel(ui + 1) || IR816LabelUsed(pcCode, psz, &aiMentions, &bCounted))))
			{
				break;
			}
			if ((psz->Text()[0] == '\t') && (psz->Text()[2] == '.'))
			{
				break;
			}
			if (bCounted && (psz->Text()[0] == '\t'))
			{
				IR816CountMentions(psz->Text(), &aiMentions, -1);
			}
			pcCode->Remove(ui + 1);
			bChanged = true;
		}
	}
	aiMentions.Kill();
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether what runs from line uiFrom on (past labels, following bras, past
// instructions that leave A and don't read it or N and Z: IR816LeavesA)
// starts with an instruction that sets all of A (IR816MF_LOADS_A) and N and
// Z, so a value in A and its flags aren't read.  pauiLabelLines is as for
// IR816LabelledInstruction.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816SetsAFirst(CIR816Code* pcCode, size uiFrom, CArrayTemplate<size>* pauiLabelLines)
{
	size		ui;
	size		uiTarget;
	int			iSteps;
	SIR816Line	sLine;
	char		szLabel[32];

	ui = uiFrom;
	for (iSteps = 0; (iSteps < 32) && (ui < pcCode->NumLines()); iSteps++)
	{
		if (pcCode->IsLabel(ui))
		{
			ui++;
			continue;
		}
		if (pcCode->Decoded(ui, &sLine) && IR816LeavesA(&sLine))
		{
			ui++;
			continue;
		}
		if (IR816BranchLine(pcCode, ui, &sLine, szLabel) && (sLine.uiMnemonic == IR816M_Bra))
		{
			uiTarget = IR816LabelledInstruction(pcCode, szLabel, pauiLabelLines);
			if (uiTarget == (size)-1)
			{
				return false;
			}
			ui = uiTarget;
			continue;
		}
		return pcCode->Has(ui, IR816MF_LOADS_A);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the straight line code from line uiFrom on writes X (tax, ldx,
// plx) before anything that might read it: only instructions that don't
// (on A, Y, the flags or the direct page, not "[D + X]") come between, and
// no label, branch, call or return.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816SetsXFirst(CIR816Code* pcCode, size uiFrom)
{
	size		ui;
	CChars*		psz;
	SIR816Line	sLine;

	for (ui = uiFrom; ui < pcCode->NumLines(); ui++)
	{
		psz = pcCode->GetText(ui);
		if (!pcCode->Decoded(ui, &sLine) || (strstr(psz->Text(), "X]") != NULL) || (strstr(psz->Text(), ", X") != NULL))
		{
			return false;
		}
		if ((sLine.uiFlags & IR816MF_WRITES_X) && (sLine.uiMnemonic != IR816M_Tyx))
		{
			return true;
		}
		if (!(sLine.uiFlags & (IR816MF_A_ALU | IR816MF_Y_ALU | IR816MF_CARRY | IR816MF_WIDTH | IR816MF_SHIFT_STEP)) && (sLine.uiMnemonic != IR816M_Stz))
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The local labels (L and a number) the lines name anywhere but in their
// own definitions (branches, jumps and jump tables' .words), marked by
// number in pabUsed.
//
//////////////////////////////////////////////////////////////////////////
static void IR816FindUsedLabels(CIR816Code* pcCode, CArrayTemplate<uint8>* pabUsed)
{
	const char*	szText;
	size		ui;
	size		uiNumber;
	uint8		uiZero;
	uint8		uiOne;

	uiZero = 0;
	uiOne = 1;
	pabUsed->FakeSetUsedElements(0);
	for (ui = 0; ui < pcCode->NumLines(); ui++)
	{
		szText = pcCode->Text(ui);
		if (szText[0] == 'L')
		{
			//A label's definition.
			continue;
		}
		for (; *szText; szText++)
		{
			if ((szText[0] == 'L') && (szText[1] >= '0') && (szText[1] <= '9') && ((szText == pcCode->Text(ui)) || !IR816IsNameCharacter(szText[-1])))
			{
				uiNumber = (size)atoi(&szText[1]);
				while (pabUsed->NumElements() <= uiNumber)
				{
					pabUsed->Add(&uiZero);
				}
				pabUsed->Set(uiNumber, &uiOne);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// False if psz is a local label's definition ("L12:") that nothing names
// (IR816FindUsedLabels); true for any other line.
//
//////////////////////////////////////////////////////////////////////////
static bool IR816LabelUsed(CChars* psz, CArrayTemplate<uint8>* pabUsed)
{
	const char*	szText;
	size		ui;
	size		uiNumber;

	szText = psz->Text();
	if ((szText[0] != 'L') || (szText[1] < '0') || (szText[1] > '9'))
	{
		return true;
	}
	for (ui = 1; (szText[ui] >= '0') && (szText[ui] <= '9'); ui++)
	{
	}
	if ((szText[ui] != ':') || (szText[ui + 1] != '\0'))
	{
		return true;
	}
	uiNumber = (size)atoi(&szText[1]);
	return (uiNumber < pabUsed->NumElements()) && (*pabUsed->Get(uiNumber) != 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Writes a branch line, "\t\tszMnemonic szLabel", over line uiLine.
//
//////////////////////////////////////////////////////////////////////////
static void IR816SetBranch(CIR816Code* pcCode, size uiLine, const char* szMnemonic, const char* szLabel)
{
	CChars	sz;

	sz.Init(CIR816Backend::GetScratch());
	sz.Append("\t\t");
	sz.Append(szMnemonic);
	sz.Append(" ");
	sz.Append(szLabel);
	pcCode->Set(uiLine, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Branches tidied in the code written from uiStart on, until none changes:
// a bra to the label that follows goes; a branch to a label whose first
// instruction is a bra goes where that does; Bcc X, bra Y, X: is B!cc Y,
// X: (and bcc X, beq X, bra Y, X: is beq X, bcs Y, X:); Bcc X, bra Y
// round a loop (marked IR816_ROUND by JumpTo) is B!cc Y, bra X; an lda of a direct
// page word or a constant goes when what runs next (falling through or
// after a bra) sets A and N and Z before reading A or the flags; a tax goes
// when X is written before it is read (IR816SetsXFirst); txa after tax (or tax after txa),
// only clc or sec between, goes; a txy goes after a txy and an instruction that changes neither X
// nor Y; and code after a bra, rtl or stp up to a label that is used goes
// (IR816Unreachable).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PeepholeBranches(CChars* pszDest, size uiStart)
{
	CIR816Code	cCode;

	cCode.Init();
	cCode.Load(pszDest, uiStart);
	PeepholeBranches(&cCode);
	cCode.Write(pszDest, uiStart);
	cCode.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PeepholeBranches(CIR816Code* pcCode)
{
	CArrayTemplate<uint8>	abUsed;
	CArrayTemplate<size>	auiLabelLines;
	CChars*					psz;
	CChars					szRound;
	SIR816Line				sLine;
	SIR816Line				sLine2;
	SIR816Line				sLine3;
	char					szLabel[32];
	char					szLabel2[32];
	char					szLabel3[32];
	size					ui;
	size					uiTarget;
	int						iPass;
	bool					bChanged;

	abUsed.Init(CIR816Backend::GetScratch());
	auiLabelLines.Init(CIR816Backend::GetScratch());
	for (iPass = 0; iPass < 8; iPass++)
	{
		bChanged = false;
		IR816FindUsedLabels(pcCode, &abUsed);
		for (ui = 0; ui < pcCode->NumLines(); ui++)
		{
			psz = pcCode->GetText(ui);
			if (!IR816LabelUsed(psz, &abUsed))
			{
				//A label nothing goes to (its branches were sent elsewhere): it would keep the rules below apart.
				pcCode->Remove(ui);
				ui--;
				bChanged = true;
				continue;
			}
			if (IR816BranchLine(pcCode, ui, &sLine, szLabel))
			{
				//To a bra: straight to where it goes.
				uiTarget = IR816LabelledInstruction(pcCode, szLabel, &auiLabelLines);
				if ((uiTarget != (size)-1) && IR816BranchLine(pcCode, uiTarget, &sLine2, szLabel2) && (sLine2.uiMnemonic == IR816M_Bra) &&
					(StringCompare(szLabel2, szLabel) != 0))
				{
					IR816SetBranch(pcCode, ui, sLine.szMnemonic, szLabel2);
					bChanged = true;
					continue;
				}

				//A bra to the next label.
				if ((sLine.uiMnemonic == IR816M_Bra) && IR816LabelFollows(pcCode, ui + 1, szLabel))
				{
					pcCode->Remove(ui);
					ui--;
					bChanged = true;
					continue;
				}

				//Bcc X, bra Y, X:
				if ((ui + 1 < pcCode->NumLines()) && IR816InvertedBranch(sLine.uiMnemonic) && IR816BranchLine(pcCode, ui + 1, &sLine2, szLabel2) &&
					(sLine2.uiMnemonic == IR816M_Bra) && IR816LabelFollows(pcCode, ui + 2, szLabel))
				{
					IR816SetBranch(pcCode, ui, IR816InvertedBranch(sLine.uiMnemonic), szLabel2);
					pcCode->Remove(ui + 1);
					bChanged = true;
					continue;
				}

				//Bcc X, bra Y round a loop (JumpTo marked it) is B!cc Y, bra X: going round, the usual way, is one branch taken.
				if ((ui + 1 < pcCode->NumLines()) && IR816InvertedBranch(sLine.uiMnemonic) && IR816BranchLine(pcCode, ui + 1, &sLine2, szLabel2) &&
					(sLine2.uiMnemonic == IR816M_Bra) && pcCode->GetText(ui + 1)->EndsWith(IR816_ROUND) && (StringCompare(szLabel, szLabel2) != 0))
				{
					IR816SetBranch(pcCode, ui, IR816InvertedBranch(sLine.uiMnemonic), szLabel2);
					IR816SetBranch(pcCode, ui + 1, "bra", szLabel);
					bChanged = true;
					continue;
				}

				//bcc X, beq X, bra Y, X: (either order of the first two) is beq X, bcs Y, X:.
				if ((ui + 2 < pcCode->NumLines()) && IR816BranchLine(pcCode, ui + 1, &sLine2, szLabel2) &&
					IR816BranchLine(pcCode, ui + 2, &sLine3, szLabel3) && (sLine3.uiMnemonic == IR816M_Bra) && (StringCompare(szLabel, szLabel2) == 0) &&
					(((sLine.uiMnemonic == IR816M_Bcc) && (sLine2.uiMnemonic == IR816M_Beq)) || ((sLine.uiMnemonic == IR816M_Beq) && (sLine2.uiMnemonic == IR816M_Bcc))) &&
					IR816LabelFollows(pcCode, ui + 3, szLabel))
				{
					IR816SetBranch(pcCode, ui, "beq", szLabel);
					IR816SetBranch(pcCode, ui + 1, "bcs", szLabel3);
					pcCode->Remove(ui + 2);
					bChanged = true;
					continue;
				}
				continue;
			}

			//An lda (of a direct page word or a constant) for nothing: what runs next sets A before reading A or N and Z.
			if ((psz->Length() > 3) && pcCode->Decoded(ui, &sLine))
			{
				if ((sLine.uiMnemonic == IR816M_Lda) && ((strncmp(sLine.szOperand, "[D + ", 5) == 0) || IR816IsDecimal(sLine.szOperand)) &&
					IR816SetsAFirst(pcCode, ui + 1, &auiLabelLines))
				{
					pcCode->Remove(ui);
					ui--;
					bChanged = true;
					continue;
				}
			}

			//A tax for nothing: X is written again before anything reads it.
			if (psz->Equals("\t\ttax") && IR816SetsXFirst(pcCode, ui + 1))
			{
				pcCode->Remove(ui);
				ui--;
				bChanged = true;
				continue;
			}

			//tax then txa, or txa then tax, with only clc or sec between: A and X hold the same already (and N and Z
			//are as the first set them).
			if (psz->Equals("\t\ttax") || psz->Equals("\t\ttxa"))
			{
				for (uiTarget = ui + 1; (uiTarget < pcCode->NumLines()) && (pcCode->GetText(uiTarget)->Equals("\t\tclc") || pcCode->GetText(uiTarget)->Equals("\t\tsec")); uiTarget++)
				{
				}
				if ((uiTarget < pcCode->NumLines()) && pcCode->GetText(uiTarget)->Equals((psz->Text()[3] == 'a') ? "\t\ttxa" : "\t\ttax"))
				{
					pcCode->Remove(uiTarget);
					bChanged = true;
					continue;
				}
			}

			//txy, an instruction that keeps X and Y (lda, sta, and, ora, eor, adc, sbc, cmp, sep, rep, clc or sec), txy.
			if ((ui + 2 < pcCode->NumLines()) && psz->Equals("\t\ttxy") && pcCode->Decoded(ui + 1, &sLine) &&
				(((sLine.uiFlags & IR816MF_A_ALU) && (sLine.uiMnemonic != IR816M_Bit)) || (sLine.uiFlags & (IR816MF_WIDTH | IR816MF_CARRY))) &&
				pcCode->GetText(ui + 2)->Equals("\t\ttxy"))
			{
				pcCode->Remove(ui + 2);
				bChanged = true;
				continue;
			}
		}
		bChanged |= IR816Unreachable(pcCode);
		if (!bChanged)
		{
			break;
		}
	}

	for (ui = 0; ui < pcCode->NumLines(); ui++)
	{
		psz = pcCode->GetText(ui);
		if (psz->EndsWith(IR816_ROUND))
		{
			szRound.Init(CIR816Backend::GetScratch());
			szRound.AppendSubString(psz->Text(), psz->Length() - StrLen(IR816_ROUND));
			pcCode->Set(ui, szRound.Text());
			szRound.Kill();
		}
	}
	auiLabelLines.Kill();
	abUsed.Kill();
}
