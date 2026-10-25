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
#include "IR65Widths.h"


#define IR65_WIDTH_ALL			0x0F		//A use bit per register.
#define IR65_WIDTH_CONFLICT		0xFF		//Widths that differ between the ways a line is reached.
#define IR65_WIDTH_MAX_OPERANDS	3


//An instruction's operand: where it is in the line.
struct SIR65WidthOperand
{
	const char*	szStart;
	size		uiLength;
};


//The instructions the back end writes (IR65Backend.cpp) and how they decode.
struct SIR65WidthMnemonic
{
	const char*	szName;
	uint8		uiKind;
	bool		bReadsFirst;	//An Alu reading its first operand; stp reading all four.
	bool		bWritesFirst;	//An Alu writing its first operand.
	bool		bShift;
};


static const char gacIR65WidthRegisters[IR65_WIDTH_REGISTERS] = { 'A', 'B', 'X', 'Y' };


static const SIR65WidthMnemonic gasIR65WidthMnemonics[] =
{
	{ "cpy",	IR65WK_Alu,			false,	true,	false },
	{ "add",	IR65WK_Alu,			true,	true,	false },
	{ "sub",	IR65WK_Alu,			true,	true,	false },
	{ "and",	IR65WK_Alu,			true,	true,	false },
	{ "or",		IR65WK_Alu,			true,	true,	false },
	{ "xor",	IR65WK_Alu,			true,	true,	false },
	{ "shl",	IR65WK_Alu,			true,	true,	true },
	{ "shr",	IR65WK_Alu,			true,	true,	true },
	{ "cmp",	IR65WK_Alu,			true,	false,	false },
	{ "neg",	IR65WK_Negate,		false,	true,	false },
	{ "psh",	IR65WK_Push,		false,	false,	false },
	{ "pul",	IR65WK_Pull,		false,	false,	false },
	{ "wth",	IR65WK_Width,		false,	false,	false },
	{ "wtth",	IR65WK_Width,		false,	false,	false },
	{ "wthx",	IR65WK_Extend,		false,	false,	false },
	{ "mul",	IR65WK_Multiply,	false,	false,	false },
	{ "div",	IR65WK_Divide,		false,	false,	false },
	{ "jsr",	IR65WK_Call,		false,	false,	false },
	{ "ret",	IR65WK_Return,		false,	false,	false },
	{ "stp",	IR65WK_Return,		true,	false,	false },
	{ "bra",	IR65WK_Jump,		false,	false,	false },
	{ "bras",	IR65WK_Branch,		false,	false,	false },
	{ "brac",	IR65WK_Branch,		false,	false,	false },
	{ "brasa",	IR65WK_Branch,		false,	false,	false },
	{ "braso",	IR65WK_Branch,		false,	false,	false },
	{ NULL,		IR65WK_Other,		false,	false,	false },
};


//////////////////////////////////////////////////////////////////////////
//
// The bits read or written at a width, as bands: bit 0 is bits 0 to 7,
// bit 1 bits 8 to 15, bit 2 bits 16 to 31 and bit 3 bits 32 to 63.  An
// unknown width is all of them.
//
//////////////////////////////////////////////////////////////////////////
static uint8 IR65WidthBands(uint8 uiBits)
{
	switch (uiBits)
	{
	case 8:		return 0x1;
	case 16:	return 0x3;
	case 32:	return 0x7;
	default:	return 0xF;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// 0 to 3 for A, B, X or Y, -1 for anything else.
//
//////////////////////////////////////////////////////////////////////////
static int IR65WidthRegister(SIR65WidthOperand* psOperand)
{
	int	i;

	if (psOperand->uiLength != 1)
	{
		return -1;
	}
	for (i = 0; i < IR65_WIDTH_REGISTERS; i++)
	{
		if (psOperand->szStart[0] == gacIR65WidthRegisters[i])
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the operand is any register an instruction can name (A, B, X,
// Y, S, V or W).
//
//////////////////////////////////////////////////////////////////////////
static bool IR65WidthIsRegister(SIR65WidthOperand* psOperand)
{
	return (psOperand->uiLength == 1) && (strchr("ABXYSVW", psOperand->szStart[0]) != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// A bit for each of A, B, X and Y named on its own in a memory operand
// ([4 + X], [A], [[8 + W] + Y]); 0 for an operand that isn't memory.
//
//////////////////////////////////////////////////////////////////////////
static uint8 IR65WidthAddresses(SIR65WidthOperand* psOperand)
{
	const char*	sz;
	const char*	szEnd;
	const char*	szWord;
	uint8		uiAddresses;
	int			i;

	if ((psOperand->uiLength == 0) || (psOperand->szStart[0] != '['))
	{
		return 0;
	}
	uiAddresses = 0;
	szEnd = psOperand->szStart + psOperand->uiLength;
	sz = psOperand->szStart;
	while (sz < szEnd)
	{
		if (((*sz >= 'A') && (*sz <= 'Z')) || ((*sz >= 'a') && (*sz <= 'z')) || ((*sz >= '0') && (*sz <= '9')) || (*sz == '_'))
		{
			szWord = sz;
			while ((sz < szEnd) && (((*sz >= 'A') && (*sz <= 'Z')) || ((*sz >= 'a') && (*sz <= 'z')) || ((*sz >= '0') && (*sz <= '9')) || (*sz == '_')))
			{
				sz++;
			}
			if (sz - szWord == 1)
			{
				for (i = 0; i < IR65_WIDTH_REGISTERS; i++)
				{
					if (*szWord == gacIR65WidthRegisters[i])
					{
						uiAddresses |= (uint8)(1 << i);
					}
				}
			}
		}
		else
		{
			sz++;
		}
	}
	return uiAddresses;
}


//////////////////////////////////////////////////////////////////////////
//
// A bit for each register of a "(A, B)" list, or the one register named;
// false if anything else is named.
//
//////////////////////////////////////////////////////////////////////////
static bool IR65WidthRegisterList(SIR65WidthOperand* psOperand, uint8* puiRegisters)
{
	SIR65WidthOperand	sName;
	const char*			sz;
	const char*			szEnd;
	int					iRegister;

	*puiRegisters = 0;
	sz = psOperand->szStart;
	szEnd = sz + psOperand->uiLength;
	if ((sz < szEnd) && (*sz == '('))
	{
		sz++;
		szEnd--;
	}
	while (sz < szEnd)
	{
		while ((sz < szEnd) && ((*sz == ' ') || (*sz == ',')))
		{
			sz++;
		}
		sName.szStart = sz;
		while ((sz < szEnd) && (*sz != ' ') && (*sz != ','))
		{
			sz++;
		}
		sName.uiLength = sz - sName.szStart;
		if (sName.uiLength > 0)
		{
			iRegister = IR65WidthRegister(&sName);
			if (iRegister < 0)
			{
				return false;
			}
			*puiRegisters |= (uint8)(1 << iRegister);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Splits the operands after the mnemonic at the commas outside brackets;
// the number found, or -1 for too many.
//
//////////////////////////////////////////////////////////////////////////
static int IR65WidthOperands(const char* sz, SIR65WidthOperand* asOperands)
{
	int		iOperands;
	int		iDepth;

	iOperands = 0;
	while (*sz == ' ')
	{
		sz++;
	}
	if ((*sz == '\0') || (*sz == ';'))
	{
		return 0;
	}
	asOperands[0].szStart = sz;
	iDepth = 0;
	for (;;)
	{
		if ((*sz == '\0') || (*sz == ';') || ((*sz == ',') && (iDepth == 0)))
		{
			asOperands[iOperands].uiLength = sz - asOperands[iOperands].szStart;
			while ((asOperands[iOperands].uiLength > 0) && (asOperands[iOperands].szStart[asOperands[iOperands].uiLength - 1] == ' '))
			{
				asOperands[iOperands].uiLength--;
			}
			iOperands++;
			if (*sz != ',')
			{
				return iOperands;
			}
			if (iOperands == IR65_WIDTH_MAX_OPERANDS)
			{
				return -1;
			}
			sz++;
			while (*sz == ' ')
			{
				sz++;
			}
			asOperands[iOperands].szStart = sz;
			continue;
		}
		if ((*sz == '[') || (*sz == '('))
		{
			iDepth++;
		}
		else if ((*sz == ']') || (*sz == ')'))
		{
			iDepth--;
		}
		sz++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::Init(void)
{
	maszLines.Init();
	masLines.Init();
	masWidths.Init();
	masLive.Init();
	masEntries.Init();
	mauiReached.Init();
	mauiJumpedBack.Init();
	mauiArrived.Init();
	mmLabels.Init();
	mpszOut = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::Kill(void)
{
	mmLabels.Kill();
	mauiArrived.Kill();
	mauiJumpedBack.Kill();
	mauiReached.Kill();
	masEntries.Kill();
	masLive.Kill();
	masWidths.Kill();
	masLines.Kill();
	maszLines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Rewrites the function's code from uiStart (its label first).  Code with
// anything it doesn't know is left as it is.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::Optimise(CChars* pszCode, size uiStart)
{
	CChars	szText;

	maszLines.ReInit();
	szText.Init(pszCode->Text() + uiStart);
	szText.SplitLines(&maszLines);
	szText.Kill();
	mpszOut = pszCode;

	if (!Decode() || !ComputeWidths())
	{
		return;
	}
	do
	{
		ComputeLive();
	}
	while (RemoveExtensions());

	pszCode->SetLength(uiStart);
	Write();
}


//////////////////////////////////////////////////////////////////////////
//
// Decodes every line; false if one isn't understood or a branch goes
// outside the function.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Widths::Decode(void)
{
	size			ui;
	size			uiNum;
	CChars*			pszLine;
	SIR65WidthLine*	psLine;
	CChars			szName;
	bool			bDecoded;

	uiNum = maszLines.NumElements();
	mmLabels.Kill();
	mmLabels.Init();
	for (ui = 0; ui < uiNum; ui++)
	{
		pszLine = maszLines.Get(ui);
		if (!pszLine->Empty() && !pszLine->StartsWith("\t") && pszLine->EndsWith(':'))
		{
			szName.Init();
			szName.AppendSubString(pszLine->Text(), pszLine->Length() - 1);
			mmLabels.Put(szName.Text(), (int32)ui);
			szName.Kill();
		}
	}

	masLines.ReInit();
	mauiJumpedBack.ReInit();
	for (ui = 0; ui < uiNum; ui++)
	{
		psLine = masLines.Add();
		mauiJumpedBack.Add()[0] = 0;
		bDecoded = DecodeLine(maszLines.Get(ui), psLine);
		if (!bDecoded)
		{
			return false;
		}
		if ((psLine->iTarget >= 0) && ((size)psLine->iTarget <= ui))
		{
			*mauiJumpedBack.Get(psLine->iTarget) = 1;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Decodes one line of the back end's code (IR65Backend.cpp writes only
// these instructions).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Widths::DecodeLine(CChars* pszLine, SIR65WidthLine* psLine)
{
	const char*					szText;
	char						szMnemonic[8];
	size						uiMnemonic;
	SIR65WidthOperand			asOperands[IR65_WIDTH_MAX_OPERANDS];
	SIR65WidthOperand*			psLast;
	const SIR65WidthMnemonic*	psMnemonic;
	int							iOperands;
	int							i;
	CChars						szTarget;

	memset(psLine, 0, sizeof(SIR65WidthLine));
	psLine->iFirst = -1;
	psLine->iSecond = -1;
	psLine->iTarget = -1;
	psLine->uiKind = IR65WK_Other;

	if (pszLine->Empty())
	{
		return true;
	}
	if (!pszLine->StartsWith("\t"))
	{
		if (pszLine->EndsWith(':'))
		{
			psLine->uiKind = IR65WK_Label;
		}
		return true;
	}

	szText = pszLine->Text();
	while ((*szText == '\t') || (*szText == ' '))
	{
		szText++;
	}
	if ((*szText == '\0') || (*szText == ';'))
	{
		return true;
	}
	for (uiMnemonic = 0; (szText[uiMnemonic] != '\0') && (szText[uiMnemonic] != ' '); uiMnemonic++)
	{
		if (uiMnemonic + 1 >= sizeof(szMnemonic))
		{
			return false;
		}
		szMnemonic[uiMnemonic] = szText[uiMnemonic];
	}
	szMnemonic[uiMnemonic] = '\0';
	for (psMnemonic = gasIR65WidthMnemonics; psMnemonic->szName != NULL; psMnemonic++)
	{
		if (StringCompare(psMnemonic->szName, szMnemonic) == 0)
		{
			break;
		}
	}
	if (psMnemonic->szName == NULL)
	{
		return false;
	}
	iOperands = IR65WidthOperands(szText + uiMnemonic, asOperands);
	if (iOperands < 0)
	{
		return false;
	}
	for (i = 0; i < iOperands; i++)
	{
		psLine->uiAddresses |= IR65WidthAddresses(&asOperands[i]);
	}
	psLast = (iOperands > 0) ? &asOperands[iOperands - 1] : NULL;

	psLine->uiKind = psMnemonic->uiKind;
	psLine->bReadsFirst = psMnemonic->bReadsFirst;
	psLine->bWritesFirst = psMnemonic->bWritesFirst;
	psLine->bShift = psMnemonic->bShift;
	switch (psMnemonic->uiKind)
	{
	case IR65WK_Alu:
		if (iOperands != 2)
		{
			return false;
		}
		psLine->iFirst = (int8)IR65WidthRegister(&asOperands[0]);
		psLine->iSecond = (int8)IR65WidthRegister(&asOperands[1]);
		psLine->bFirstRegister = IR65WidthIsRegister(&asOperands[0]);
		return true;

	case IR65WK_Negate:
		//neg R1 {, R2}: R2 (R1 when there is one register) is negated into R1.
		if ((iOperands < 1) || (iOperands > 2))
		{
			return false;
		}
		psLine->iFirst = (int8)IR65WidthRegister(&asOperands[0]);
		psLine->iSecond = (int8)IR65WidthRegister(psLast);
		psLine->bFirstRegister = true;
		return (psLine->iFirst >= 0) && (psLine->iSecond >= 0);

	case IR65WK_Push:
	case IR65WK_Pull:
		//Registers, memory or (psh) a value.  S, V and W are 32 bits; P and C would change or show the widths.
		if (iOperands != 1)
		{
			return false;
		}
		if ((asOperands[0].szStart[0] == '(') || (IR65WidthRegister(&asOperands[0]) >= 0))
		{
			return IR65WidthRegisterList(&asOperands[0], &psLine->uiRegisters);
		}
		return (asOperands[0].uiLength > 0) && (asOperands[0].szStart[0] != 'P') && (asOperands[0].szStart[0] != 'C');

	case IR65WK_Width:
	case IR65WK_Extend:
		if (iOperands != 2)
		{
			return false;
		}
		psLine->uiBits = (uint8)atoi(asOperands[1].szStart);
		if ((psLine->uiBits != 8) && (psLine->uiBits != 16) && (psLine->uiBits != 32) && (psLine->uiBits != 64))
		{
			return false;
		}
		if (psLine->uiKind == IR65WK_Extend)
		{
			psLine->iFirst = (int8)IR65WidthRegister(&asOperands[0]);
			return psLine->iFirst >= 0;
		}
		return IR65WidthRegisterList(&asOperands[0], &psLine->uiRegisters) && (psLine->uiRegisters != 0);

	case IR65WK_Multiply:
	case IR65WK_Divide:
	case IR65WK_Return:
		return iOperands == 0;

	case IR65WK_Call:
		if (iOperands != 1)
		{
			return false;
		}
		psLine->iFirst = (int8)IR65WidthRegister(&asOperands[0]);
		return true;

	case IR65WK_Branch:
	case IR65WK_Jump:
		if (psLast == NULL)
		{
			return false;
		}
		szTarget.Init();
		szTarget.AppendSubString(psLast->szStart, psLast->uiLength);
		psLine->iTarget = mmLabels.Get(szTarget.Text(), -1);
		szTarget.Kill();
		return psLine->iTarget >= 0;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// What a line does to A, B, X and Y given the widths it starts with
// (changed to the widths it leaves): the bits it reads and writes (as
// IR65WidthBands) and a bit for each register whose width it depends on.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::Effect(SIR65WidthLine* psLine, SIR65Widths* psWidths, SIR65Widths* psReads, SIR65Widths* psWrites, uint8* puiUses)
{
	uint8*	auiWidths;
	uint8	uiBits;
	int		i;

	auiWidths = psWidths->auiRegisters;
	memset(psReads, 0, sizeof(SIR65Widths));
	memset(psWrites, 0, sizeof(SIR65Widths));
	*puiUses = 0;

	for (i = 0; i < IR65_WIDTH_REGISTERS; i++)
	{
		if (psLine->uiAddresses & (1 << i))
		{
			*puiUses |= (uint8)(1 << i);
			psReads->auiRegisters[i] |= IR65WidthBands(auiWidths[i]);
		}
	}

	switch (psLine->uiKind)
	{
	case IR65WK_Alu:
	case IR65WK_Negate:
		//The operation is the first register's width; with memory first, the second register's (or 32 bits).
		if (psLine->iFirst >= 0)
		{
			uiBits = auiWidths[psLine->iFirst];
		}
		else if (!psLine->bFirstRegister && (psLine->iSecond >= 0))
		{
			uiBits = auiWidths[psLine->iSecond];
		}
		else
		{
			uiBits = 32;
		}
		if (psLine->iFirst >= 0)
		{
			*puiUses |= (uint8)(1 << psLine->iFirst);
			if (psLine->bReadsFirst)
			{
				psReads->auiRegisters[psLine->iFirst] |= IR65WidthBands(uiBits);
			}
			if (psLine->bWritesFirst)
			{
				psWrites->auiRegisters[psLine->iFirst] |= IR65WidthBands(uiBits);
			}
		}
		if (psLine->iSecond >= 0)
		{
			*puiUses |= (uint8)(1 << psLine->iSecond);
			if (psLine->bShift || (auiWidths[psLine->iSecond] < uiBits))
			{
				psReads->auiRegisters[psLine->iSecond] |= IR65WidthBands(auiWidths[psLine->iSecond]);
			}
			else
			{
				psReads->auiRegisters[psLine->iSecond] |= IR65WidthBands(uiBits);
			}
		}
		//A register read and written in one instruction (add A, A) is still read.
		break;

	case IR65WK_Push:
	case IR65WK_Pull:
		for (i = 0; i < IR65_WIDTH_REGISTERS; i++)
		{
			if (psLine->uiRegisters & (1 << i))
			{
				*puiUses |= (uint8)(1 << i);
				if (psLine->uiKind == IR65WK_Push)
				{
					psReads->auiRegisters[i] |= IR65WidthBands(auiWidths[i]);
				}
				else
				{
					psWrites->auiRegisters[i] |= IR65WidthBands(auiWidths[i]);
				}
			}
		}
		break;

	case IR65WK_Width:
		for (i = 0; i < IR65_WIDTH_REGISTERS; i++)
		{
			if (psLine->uiRegisters & (1 << i))
			{
				auiWidths[i] = psLine->uiBits;
			}
		}
		break;

	case IR65WK_Extend:
		i = psLine->iFirst;
		*puiUses |= (uint8)(1 << i);
		psReads->auiRegisters[i] |= IR65WidthBands(auiWidths[i]);
		if (psLine->uiBits > auiWidths[i])
		{
			psWrites->auiRegisters[i] |= IR65WidthBands(psLine->uiBits) & ~IR65WidthBands(auiWidths[i]);
		}
		auiWidths[i] = psLine->uiBits;
		break;

	case IR65WK_Multiply:
		*puiUses |= 0x1 | 0x2 | 0x8;
		psReads->auiRegisters[0] |= IR65WidthBands(auiWidths[0]);
		psReads->auiRegisters[1] |= IR65WidthBands(auiWidths[1]);
		psWrites->auiRegisters[3] |= IR65WidthBands(auiWidths[3]);
		break;

	case IR65WK_Divide:
		*puiUses |= 0x1 | 0x2 | 0x8;
		psReads->auiRegisters[0] |= IR65WidthBands(auiWidths[0]);
		psReads->auiRegisters[1] |= IR65WidthBands(auiWidths[1]);
		psReads->auiRegisters[3] |= IR65WidthBands(auiWidths[3]);
		psWrites->auiRegisters[1] |= IR65WidthBands(auiWidths[1]);
		psWrites->auiRegisters[3] |= IR65WidthBands(auiWidths[3]);
		break;

	case IR65WK_Call:
		//The callee starts with all four 32 bits wide, may read them (this in X) and changes them all.
		*puiUses = IR65_WIDTH_ALL;
		for (i = 0; i < IR65_WIDTH_REGISTERS; i++)
		{
			psReads->auiRegisters[i] |= IR65WidthBands(32);
			psWrites->auiRegisters[i] = IR65WidthBands(64);
			auiWidths[i] = 32;
		}
		if (psLine->iFirst >= 0)
		{
			psReads->auiRegisters[psLine->iFirst] |= IR65WidthBands(64);
		}
		break;

	case IR65WK_Return:
		//The result is A, all of it for a 64 bit one; stp may be looked at by anything.
		*puiUses = IR65_WIDTH_ALL;
		psReads->auiRegisters[0] = IR65WidthBands(64);
		if (psLine->bReadsFirst)
		{
			for (i = 1; i < IR65_WIDTH_REGISTERS; i++)
			{
				psReads->auiRegisters[i] = IR65WidthBands(64);
			}
		}
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Widths::FallsThrough(SIR65WidthLine* psLine)
{
	return (psLine->uiKind != IR65WK_Jump) && (psLine->uiKind != IR65WK_Return);
}


//////////////////////////////////////////////////////////////////////////
//
// The widths each line starts with: 32 bits at the function's label,
// then what the lines before it leave.  False if a line can be reached
// with two different widths of one register.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Widths::ComputeWidths(void)
{
	size			ui;
	size			uiNum;
	size			uiNext;
	SIR65Widths		sWidths;
	SIR65Widths		sReads;
	SIR65Widths		sWrites;
	SIR65WidthLine*	psLine;
	SIR65Widths*	psTo;
	uint8			uiUses;
	bool			bChanged;
	int				i;
	int				iTo;
	size			auiTo[2];

	uiNum = masLines.NumElements();
	masWidths.ReInit();
	mauiReached.ReInit();
	for (ui = 0; ui < uiNum; ui++)
	{
		memset(masWidths.Add(), 0, sizeof(SIR65Widths));
		mauiReached.Add()[0] = 0;
	}
	if (uiNum == 0)
	{
		return false;
	}
	memset(masWidths.Get(0), 32, sizeof(SIR65Widths));
	mauiReached.Set(0, (uint8*)"\1");

	do
	{
		bChanged = false;
		for (ui = 0; ui < uiNum; ui++)
		{
			if (!*mauiReached.Get(ui))
			{
				continue;
			}
			psLine = masLines.Get(ui);
			sWidths = *masWidths.Get(ui);
			Effect(psLine, &sWidths, &sReads, &sWrites, &uiUses);

			auiTo[0] = (FallsThrough(psLine) && (ui + 1 < uiNum)) ? ui + 1 : uiNum;
			auiTo[1] = (psLine->iTarget >= 0) ? (size)psLine->iTarget : uiNum;
			for (iTo = 0; iTo < 2; iTo++)
			{
				uiNext = auiTo[iTo];
				if (uiNext >= uiNum)
				{
					continue;
				}
				psTo = masWidths.Get(uiNext);
				if (!*mauiReached.Get(uiNext))
				{
					*psTo = sWidths;
					*mauiReached.Get(uiNext) = 1;
					bChanged = true;
					continue;
				}
				for (i = 0; i < IR65_WIDTH_REGISTERS; i++)
				{
					if ((psTo->auiRegisters[i] != sWidths.auiRegisters[i]) && (psTo->auiRegisters[i] != IR65_WIDTH_CONFLICT))
					{
						psTo->auiRegisters[i] = IR65_WIDTH_CONFLICT;
						bChanged = true;
					}
				}
			}
		}
	}
	while (bChanged);

	for (ui = 0; ui < uiNum; ui++)
	{
		if (*mauiReached.Get(ui))
		{
			for (i = 0; i < IR65_WIDTH_REGISTERS; i++)
			{
				if (masWidths.Get(ui)->auiRegisters[i] == IR65_WIDTH_CONFLICT)
				{
					return false;
				}
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The bits of each register read before they are written, from the start
// of each line (live in).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::ComputeLive(void)
{
	size			ui;
	size			uiNum;
	SIR65Widths		sWidths;
	SIR65Widths		sReads;
	SIR65Widths		sWrites;
	SIR65Widths		sOut;
	SIR65Widths*	psIn;
	uint8			uiUses;
	uint8			uiIn;
	bool			bChanged;
	int				i;

	uiNum = masLines.NumElements();
	masLive.ReInit();
	for (ui = 0; ui < uiNum; ui++)
	{
		memset(masLive.Add(), 0, sizeof(SIR65Widths));
	}

	do
	{
		bChanged = false;
		for (ui = uiNum; ui-- > 0;)
		{
			if (!*mauiReached.Get(ui))
			{
				continue;
			}
			LiveOut(ui, &sOut);
			sWidths = *masWidths.Get(ui);
			Effect(masLines.Get(ui), &sWidths, &sReads, &sWrites, &uiUses);
			psIn = masLive.Get(ui);
			for (i = 0; i < IR65_WIDTH_REGISTERS; i++)
			{
				uiIn = sReads.auiRegisters[i] | (sOut.auiRegisters[i] & ~sWrites.auiRegisters[i]);
				if (uiIn != psIn->auiRegisters[i])
				{
					psIn->auiRegisters[i] = uiIn;
					bChanged = true;
				}
			}
		}
	}
	while (bChanged);
}


//////////////////////////////////////////////////////////////////////////
//
// The bits read after a line before they are written: its successors'
// live in.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::LiveOut(size uiLine, SIR65Widths* psOut)
{
	SIR65WidthLine*	psLine;
	SIR65Widths*	psNext;
	int				i;

	psLine = masLines.Get(uiLine);
	memset(psOut, 0, sizeof(SIR65Widths));
	if (FallsThrough(psLine) && (uiLine + 1 < masLines.NumElements()))
	{
		psNext = masLive.Get(uiLine + 1);
		for (i = 0; i < IR65_WIDTH_REGISTERS; i++)
		{
			psOut->auiRegisters[i] |= psNext->auiRegisters[i];
		}
	}
	if (psLine->iTarget >= 0)
	{
		psNext = masLive.Get(psLine->iTarget);
		for (i = 0; i < IR65_WIDTH_REGISTERS; i++)
		{
			psOut->auiRegisters[i] |= psNext->auiRegisters[i];
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A wthx whose extended bits are never read becomes a wth (only the width
// changes).  True if one did.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Widths::RemoveExtensions(void)
{
	size			ui;
	SIR65WidthLine*	psLine;
	SIR65Widths		sOut;
	uint8			uiFrom;
	uint8			uiExtended;
	bool			bRemoved;

	bRemoved = false;
	for (ui = 0; ui < masLines.NumElements(); ui++)
	{
		psLine = masLines.Get(ui);
		if ((psLine->uiKind != IR65WK_Extend) || !*mauiReached.Get(ui))
		{
			continue;
		}
		LiveOut(ui, &sOut);
		uiFrom = masWidths.Get(ui)->auiRegisters[psLine->iFirst];
		uiExtended = IR65WidthBands(psLine->uiBits) & ~IR65WidthBands(uiFrom);
		if ((sOut.auiRegisters[psLine->iFirst] & uiExtended) == 0)
		{
			psLine->uiKind = IR65WK_Width;
			psLine->uiRegisters = (uint8)(1 << psLine->iFirst);
			psLine->iFirst = -1;
			bRemoved = true;
		}
	}
	return bRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
// The width the first line from uiFirst on that uses the register needs,
// looking only down the code that runs straight on; 0 if a branch or the
// end comes first.
//
//////////////////////////////////////////////////////////////////////////
uint8 CIR65Widths::NextNeed(size uiFirst, int iRegister)
{
	size			ui;
	SIR65WidthLine*	psLine;
	SIR65Widths		sWidths;
	SIR65Widths		sReads;
	SIR65Widths		sWrites;
	uint8			uiUses;

	for (ui = uiFirst; ui < masLines.NumElements(); ui++)
	{
		psLine = masLines.Get(ui);
		if (!*mauiReached.Get(ui) || (psLine->uiKind == IR65WK_Branch) || (psLine->uiKind == IR65WK_Jump))
		{
			return 0;
		}
		sWidths = *masWidths.Get(ui);
		Effect(psLine, &sWidths, &sReads, &sWrites, &uiUses);
		if (uiUses & (1 << iRegister))
		{
			return masWidths.Get(ui)->auiRegisters[iRegister];
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Before going to a label that isn't a loop's: the registers the label's
// code needs at a width they aren't are set to it here, where the label is
// left for (and, for a branch, only when the code it falls through to
// needs the same width), so the ways into the label agree.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::Anticipate(size uiLabel, size uiFallThrough)
{
	SIR65Widths	sTarget;
	int			i;

	for (i = 0; i < IR65_WIDTH_REGISTERS; i++)
	{
		sTarget.auiRegisters[i] = NextNeed(uiLabel + 1, i);
		if ((uiFallThrough < masLines.NumElements()) && (NextNeed(uiFallThrough, i) != sTarget.auiRegisters[i]))
		{
			sTarget.auiRegisters[i] = 0;
		}
	}
	SetWidths(&sTarget, masLines.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
// The widths a loop's label (one a later line jumps back to) starts with
// in the new code: what its code first needs, so the loop sets them once
// before it, else what the code reaching it has (bFromActual), else what
// it had.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::DecideEntry(size uiLabel, bool bFromActual)
{
	SIR65Widths*	psEntry;
	uint8			uiNeed;
	int				i;

	psEntry = masEntries.Get(uiLabel);
	for (i = 0; i < IR65_WIDTH_REGISTERS; i++)
	{
		if (psEntry->auiRegisters[i] != 0)
		{
			continue;
		}
		uiNeed = NextNeed(uiLabel + 1, i);
		if (uiNeed == 0)
		{
			uiNeed = bFromActual ? msActual.auiRegisters[i] : masWidths.Get(uiLabel)->auiRegisters[i];
		}
		psEntry->auiRegisters[i] = uiNeed;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The code reaching a label that isn't a loop's (all of it is before the
// label) has the widths msActual.  The label starts with the widths they
// all agree on, and an unknown width (0) for a register they don't.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::Arrive(size uiLabel)
{
	SIR65Widths*	psEntry;
	int				i;

	psEntry = masEntries.Get(uiLabel);
	for (i = 0; i < IR65_WIDTH_REGISTERS; i++)
	{
		if (!*mauiArrived.Get(uiLabel))
		{
			psEntry->auiRegisters[i] = msActual.auiRegisters[i];
		}
		else if (psEntry->auiRegisters[i] != msActual.auiRegisters[i])
		{
			psEntry->auiRegisters[i] = 0;
		}
	}
	*mauiArrived.Get(uiLabel) = 1;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the wth that set the registers psTarget gives a width (not 0) to
// it.  Two registers going to one width share a wtth, as does a register
// the code from uiNext on next needs at that width.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::SetWidths(SIR65Widths* psTarget, size uiNext)
{
	int		i;
	int		iPair;
	uint8	uiBits;

	for (i = 0; i < IR65_WIDTH_REGISTERS; i++)
	{
		uiBits = psTarget->auiRegisters[i];
		if ((uiBits == 0) || (msActual.auiRegisters[i] == uiBits))
		{
			continue;
		}
		for (iPair = i + 1; iPair < IR65_WIDTH_REGISTERS; iPair++)
		{
			if ((psTarget->auiRegisters[iPair] == uiBits) && (msActual.auiRegisters[iPair] != uiBits))
			{
				break;
			}
		}
		if (iPair == IR65_WIDTH_REGISTERS)
		{
			for (iPair = 0; iPair < IR65_WIDTH_REGISTERS; iPair++)
			{
				if ((iPair != i) && (psTarget->auiRegisters[iPair] == 0) && (msActual.auiRegisters[iPair] != uiBits) && (NextNeed(uiNext, iPair) == uiBits))
				{
					break;
				}
			}
		}
		if (iPair < IR65_WIDTH_REGISTERS)
		{
			mpszOut->Append("\t\twtth (")->Append(gacIR65WidthRegisters[i])->Append(", ")->Append(gacIR65WidthRegisters[iPair])->Append("), ")->Append((int32)uiBits)->AppendNewLine();
			msActual.auiRegisters[iPair] = uiBits;
		}
		else
		{
			mpszOut->Append("\t\twth ")->Append(gacIR65WidthRegisters[i])->Append(", ")->Append((int32)uiBits)->AppendNewLine();
		}
		msActual.auiRegisters[i] = uiBits;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the code again without its wth and wtth: each instruction gets
// the widths of the registers it uses set just before it, each label the
// widths it was decided to start with.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::Write(void)
{
	size			ui;
	size			uiNum;
	SIR65WidthLine*	psLine;
	SIR65Widths		sWidths;
	SIR65Widths		sReads;
	SIR65Widths		sWrites;
	SIR65Widths		sTarget;
	uint8			uiUses;
	bool			bKnown;
	bool			bFirstLabel;
	int				i;

	uiNum = masLines.NumElements();
	masEntries.ReInit();
	mauiArrived.ReInit();
	for (ui = 0; ui < uiNum; ui++)
	{
		memset(masEntries.Add(), 0, sizeof(SIR65Widths));
		mauiArrived.Add()[0] = 0;
	}
	memset(&msActual, 32, sizeof(SIR65Widths));
	bKnown = true;
	bFirstLabel = true;

	for (ui = 0; ui < uiNum; ui++)
	{
		psLine = masLines.Get(ui);
		if (!*mauiReached.Get(ui) || (psLine->uiKind == IR65WK_Other))
		{
			WriteLine(ui);
			continue;
		}

		switch (psLine->uiKind)
		{
		case IR65WK_Label:
			if (bFirstLabel)
			{
				//The function's own label: callers set all four to 32 bits.
				memset(masEntries.Get(ui), 32, sizeof(SIR65Widths));
				*mauiArrived.Get(ui) = 1;
				bFirstLabel = false;
			}
			else if (*mauiJumpedBack.Get(ui))
			{
				DecideEntry(ui, bKnown);
				if (bKnown)
				{
					SetWidths(masEntries.Get(ui), uiNum);
				}
			}
			else if (bKnown)
			{
				Anticipate(ui, uiNum);
				Arrive(ui);
			}
			msActual = *masEntries.Get(ui);
			bKnown = true;
			WriteLine(ui);
			break;

		case IR65WK_Width:
			break;

		case IR65WK_Branch:
		case IR65WK_Jump:
			if (*mauiJumpedBack.Get(psLine->iTarget))
			{
				DecideEntry(psLine->iTarget, true);
				SetWidths(masEntries.Get(psLine->iTarget), uiNum);
			}
			else
			{
				Anticipate(psLine->iTarget, (psLine->uiKind == IR65WK_Branch) ? ui + 1 : uiNum);
				Arrive(psLine->iTarget);
			}
			WriteLine(ui);
			bKnown = psLine->uiKind == IR65WK_Branch;
			break;

		default:
			sWidths = *masWidths.Get(ui);
			Effect(psLine, &sWidths, &sReads, &sWrites, &uiUses);
			memset(&sTarget, 0, sizeof(SIR65Widths));
			for (i = 0; i < IR65_WIDTH_REGISTERS; i++)
			{
				if (uiUses & (1 << i))
				{
					sTarget.auiRegisters[i] = masWidths.Get(ui)->auiRegisters[i];
				}
			}
			SetWidths(&sTarget, ui);
			WriteLine(ui);
			for (i = 0; i < IR65_WIDTH_REGISTERS; i++)
			{
				if (uiUses & (1 << i))
				{
					msActual.auiRegisters[i] = sWidths.auiRegisters[i];
				}
			}
			bKnown = psLine->uiKind != IR65WK_Return;
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A line as it was, with the new line after it that the code had.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Widths::WriteLine(size uiLine)
{
	mpszOut->Append(maszLines.Get(uiLine));
	if (uiLine + 1 < maszLines.NumElements())
	{
		mpszOut->AppendNewLine();
	}
}
