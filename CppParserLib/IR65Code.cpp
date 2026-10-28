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
#include "IR65Code.h"


#define IR65_CODE_MAX_OPERANDS	3


//An instruction's operand: where it is in the line.
struct SIR65CodeOperand
{
	const char*	szStart;
	size		uiLength;
};


//The instructions the back end writes (IR65Backend.cpp) and how they decode.
struct SIR65CodeMnemonic
{
	const char*	szName;
	uint8		uiKind;
	bool		bReadsFirst;	//An Alu reading its first operand; stp reading all four.
	bool		bWritesFirst;	//An Alu writing its first operand.
	bool		bShift;
	bool		bResultFlags;	//n and z are the value written.
	uint8		uiFlags;		//The flags it sets (65C64Simulator.txt; jsr: the callee may set any).
};


static const char gacIR65CodeRegisters[IR65_CODE_REGISTERS] = { 'A', 'B', 'X', 'Y' };


static const SIR65CodeMnemonic gasIR65CodeMnemonics[] =
{
	{ "cpy",	IR65CK_Alu,			false,	true,	false,	true,	IR65_FLAG_N | IR65_FLAG_Z },
	{ "cpyx",	IR65CK_Alu,			false,	true,	false,	true,	IR65_FLAG_N | IR65_FLAG_Z },
	{ "add",	IR65CK_Alu,			true,	true,	false,	true,	IR65_FLAGS_ALL },
	{ "sub",	IR65CK_Alu,			true,	true,	false,	true,	IR65_FLAGS_ALL },
	{ "and",	IR65CK_Alu,			true,	true,	false,	true,	IR65_FLAG_N | IR65_FLAG_Z },
	{ "or",		IR65CK_Alu,			true,	true,	false,	true,	IR65_FLAG_N | IR65_FLAG_Z },
	{ "xor",	IR65CK_Alu,			true,	true,	false,	true,	IR65_FLAG_N | IR65_FLAG_Z },
	{ "shl",	IR65CK_Alu,			true,	true,	true,	false,	IR65_FLAG_C | IR65_FLAG_V | IR65_FLAG_Z },
	{ "shr",	IR65CK_Alu,			true,	true,	true,	false,	IR65_FLAGS_ALL },
	{ "cmp",	IR65CK_Alu,			true,	false,	false,	false,	IR65_FLAGS_ALL },
	{ "neg",	IR65CK_Negate,		false,	true,	false,	true,	IR65_FLAG_N | IR65_FLAG_Z },
	{ "psh",	IR65CK_Push,		false,	false,	false,	false,	0 },
	{ "pul",	IR65CK_Pull,		false,	false,	false,	false,	0 },
	{ "wth",	IR65CK_Width,		false,	false,	false,	false,	0 },
	{ "wtth",	IR65CK_Width,		false,	false,	false,	false,	0 },
	{ "wthx",	IR65CK_Extend,		false,	false,	false,	false,	0 },
	{ "mul",	IR65CK_Multiply,	false,	false,	false,	false,	IR65_FLAG_V | IR65_FLAG_Z },
	{ "div",	IR65CK_Divide,		false,	false,	false,	false,	IR65_FLAG_C | IR65_FLAG_V | IR65_FLAG_Z },
	{ "jsr",	IR65CK_Call,		false,	false,	false,	false,	IR65_FLAGS_ALL },
	{ "ret",	IR65CK_Return,		false,	false,	false,	false,	0 },
	{ "stp",	IR65CK_Return,		true,	false,	false,	false,	0 },
	{ "bra",	IR65CK_Jump,		false,	false,	false,	false,	0 },
	{ "bras",	IR65CK_Branch,		false,	false,	false,	false,	0 },
	{ "brac",	IR65CK_Branch,		false,	false,	false,	false,	0 },
	{ "brasa",	IR65CK_Branch,		false,	false,	false,	false,	0 },
	{ "braso",	IR65CK_Branch,		false,	false,	false,	false,	0 },
	{ NULL,		IR65CK_Other,		false,	false,	false,	false,	0 },
};


//////////////////////////////////////////////////////////////////////////
//
// 0 to 3 for A, B, X or Y, -1 for anything else.
//
//////////////////////////////////////////////////////////////////////////
static int IR65CodeRegister(SIR65CodeOperand* psOperand)
{
	int	i;

	if (psOperand->uiLength != 1)
	{
		return -1;
	}
	for (i = 0; i < IR65_CODE_REGISTERS; i++)
	{
		if (psOperand->szStart[0] == gacIR65CodeRegisters[i])
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
static bool IR65CodeIsRegister(SIR65CodeOperand* psOperand)
{
	return (psOperand->uiLength == 1) && (strchr("ABXYSVW", psOperand->szStart[0]) != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// A bit for each of A, B, X and Y named on its own in a memory operand
// ([4 + X], [A], [[8 + W] + Y]); 0 for an operand that isn't memory.
//
//////////////////////////////////////////////////////////////////////////
static uint8 IR65CodeAddresses(SIR65CodeOperand* psOperand)
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
				for (i = 0; i < IR65_CODE_REGISTERS; i++)
				{
					if (*szWord == gacIR65CodeRegisters[i])
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
static bool IR65CodeRegisterList(SIR65CodeOperand* psOperand, uint8* puiRegisters)
{
	SIR65CodeOperand	sName;
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
			iRegister = IR65CodeRegister(&sName);
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
static int IR65CodeOperands(const char* sz, SIR65CodeOperand* asOperands)
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
			if (iOperands == IR65_CODE_MAX_OPERANDS)
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
// The IR65_FLAG_ bits a branch's condition names (c, z, v, n or a list in
// brackets); 0 for anything else.
//
//////////////////////////////////////////////////////////////////////////
static uint8 IR65CodeFlags(SIR65CodeOperand* psOperand)
{
	uint8	uiFlags;
	size	ui;

	uiFlags = 0;
	for (ui = 0; ui < psOperand->uiLength; ui++)
	{
		switch (psOperand->szStart[ui])
		{
		case 'c':	uiFlags |= IR65_FLAG_C;	break;
		case 'z':	uiFlags |= IR65_FLAG_Z;	break;
		case 'v':	uiFlags |= IR65_FLAG_V;	break;
		case 'n':	uiFlags |= IR65_FLAG_N;	break;
		case '(':
		case ')':
		case ',':
		case ' ':
			break;
		default:
			return 0;
		}
	}
	return uiFlags;
}


//////////////////////////////////////////////////////////////////////////
//
// True if a memory operand a generator wrote fits cpyx's minimal addressing
// modes: [address], [address + R], [[address]], [[address + R]], [R] or [[R]]
// with R V, X, Y or W (a pseudo register, written [n] at the end).  Not with A,
// B or S, nor [[address] + R] or two registers.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::IsCopyExtendAddress(CChars* pszOperand)
{
	const char*	sz;
	const char*	szEnd;
	const char*	szWord;
	int			iRegisters;

	sz = pszOperand->Text();
	szEnd = sz + pszOperand->Length();
	if ((pszOperand->Length() < 3) || (sz[0] != '[') || (szEnd[-1] != ']'))
	{
		return false;
	}
	sz++;
	szEnd--;
	if (*sz == '[')
	{
		//[[...]]: the inside must end the operand.
		if (szEnd[-1] != ']')
		{
			return false;
		}
		sz++;
		szEnd--;
	}

	iRegisters = 0;
	while (sz < szEnd)
	{
		if ((*sz == '[') || (*sz == ']'))
		{
			return false;
		}
		if (((*sz >= 'A') && (*sz <= 'Z')) || ((*sz >= 'a') && (*sz <= 'z')) || ((*sz >= '0') && (*sz <= '9')) || (*sz == '_'))
		{
			szWord = sz;
			while ((sz < szEnd) && (((*sz >= 'A') && (*sz <= 'Z')) || ((*sz >= 'a') && (*sz <= 'z')) || ((*sz >= '0') && (*sz <= '9')) || (*sz == '_')))
			{
				sz++;
			}
			if (sz - szWord == 1)
			{
				if ((*szWord == 'A') || (*szWord == 'B') || (*szWord == 'S'))
				{
					return false;
				}
				if ((*szWord == 'V') || (*szWord == 'W') || (*szWord == 'X') || (*szWord == 'Y'))
				{
					iRegisters++;
				}
			}
		}
		else
		{
			sz++;
		}
	}
	return iRegisters <= 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Code::Init(void)
{
	maszLines.Init();
	masLines.Init();
	masWidths.Init();
	mauiReached.Init();
	mauiJumpedBack.Init();
	mauiRemoved.Init();
	mmLabels.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Code::Kill(void)
{
	mmLabels.Kill();
	mauiRemoved.Kill();
	mauiJumpedBack.Kill();
	mauiReached.Kill();
	masWidths.Kill();
	masLines.Kill();
	maszLines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Splits the function's code from uiStart (its label first) into lines
// and decodes them; false for code the passes must leave as it is.  With
// bConflicts a register may have a width that depends on the way a line
// was reached (IR65_WIDTH_CONFLICT), as it can after the Widths pass.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::Load(CChars* pszCode, size uiStart, bool bConflicts)
{
	CChars	szText;

	maszLines.ReInit();
	szText.Init(pszCode->Text() + uiStart);
	szText.SplitLines(&maszLines);
	szText.Kill();
	if (!Decode())
	{
		return false;
	}
	return ComputeWidths() || (bConflicts && (masLines.NumElements() > 0));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIR65Code::NumLines(void)
{
	return masLines.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIR65CodeLine* CIR65Code::GetLine(size uiLine)
{
	return masLines.Get(uiLine);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CIR65Code::GetText(size uiLine)
{
	return maszLines.Get(uiLine);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIR65Widths* CIR65Code::GetWidths(size uiLine)
{
	return masWidths.Get(uiLine);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::IsReached(size uiLine)
{
	return *mauiReached.Get(uiLine) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::IsJumpedBack(size uiLine)
{
	return *mauiJumpedBack.Get(uiLine) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The bits read or written at a width, as bands: bit 0 is bits 0 to 7,
// bit 1 bits 8 to 15, bit 2 bits 16 to 31 and bit 3 bits 32 to 63.  An
// unknown width is all of them.
//
//////////////////////////////////////////////////////////////////////////
uint8 CIR65Code::Bands(uint8 uiBits)
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
// 'A', 'B', 'X' or 'Y'.
//
//////////////////////////////////////////////////////////////////////////
char CIR65Code::RegisterName(int iRegister)
{
	return gacIR65CodeRegisters[iRegister];
}


//////////////////////////////////////////////////////////////////////////
//
// Decodes every line; false if one isn't understood or a branch goes
// outside the function.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::Decode(void)
{
	size			ui;
	size			uiNum;
	CChars*			pszLine;
	SIR65CodeLine*	psLine;
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
	mauiRemoved.ReInit();
	for (ui = 0; ui < uiNum; ui++)
	{
		psLine = masLines.Add();
		mauiJumpedBack.Add()[0] = 0;
		mauiRemoved.Add()[0] = 0;
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
bool CIR65Code::DecodeLine(CChars* pszLine, SIR65CodeLine* psLine)
{
	const char*					szText;
	char						szMnemonic[8];
	size						uiMnemonic;
	SIR65CodeOperand			asOperands[IR65_CODE_MAX_OPERANDS];
	SIR65CodeOperand*			psLast;
	const SIR65CodeMnemonic*	psMnemonic;
	int							iOperands;
	int							i;
	CChars						szTarget;

	memset(psLine, 0, sizeof(SIR65CodeLine));
	psLine->iFirst = -1;
	psLine->iSecond = -1;
	psLine->iTarget = -1;
	psLine->uiKind = IR65CK_Other;

	if (pszLine->Empty())
	{
		return true;
	}
	if (!pszLine->StartsWith("\t"))
	{
		if (pszLine->EndsWith(':'))
		{
			psLine->uiKind = IR65CK_Label;
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
	for (psMnemonic = gasIR65CodeMnemonics; psMnemonic->szName != NULL; psMnemonic++)
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
	iOperands = IR65CodeOperands(szText + uiMnemonic, asOperands);
	if (iOperands < 0)
	{
		return false;
	}
	for (i = 0; i < iOperands; i++)
	{
		psLine->uiAddresses |= IR65CodeAddresses(&asOperands[i]);
	}
	psLast = (iOperands > 0) ? &asOperands[iOperands - 1] : NULL;

	psLine->uiKind = psMnemonic->uiKind;
	psLine->bReadsFirst = psMnemonic->bReadsFirst;
	psLine->bWritesFirst = psMnemonic->bWritesFirst;
	psLine->bShift = psMnemonic->bShift;
	psLine->bResultFlags = psMnemonic->bResultFlags;
	psLine->uiFlagsWritten = psMnemonic->uiFlags;
	switch (psMnemonic->uiKind)
	{
	case IR65CK_Alu:
		if (iOperands != 2)
		{
			return false;
		}
		psLine->iFirst = (int8)IR65CodeRegister(&asOperands[0]);
		if ((asOperands[1].uiLength == 3) && (asOperands[1].szStart[1] == ':') && (asOperands[1].szStart[2] == 's'))
		{
			//R:s, sign extended.
			asOperands[1].uiLength = 1;
		}
		psLine->iSecond = (int8)IR65CodeRegister(&asOperands[1]);
		psLine->bFirstRegister = IR65CodeIsRegister(&asOperands[0]);
		psLine->bFirstMemory = asOperands[0].szStart[0] == '[';
		if (psLine->bFirstMemory && (StringCompare(szMnemonic, "cpyx") == 0))
		{
			//cpyx [m]:bits, R sets n and z from the bits stored, not from R at its width.
			psLine->bResultFlags = false;
		}
		return true;

	case IR65CK_Negate:
		//neg R1 {, R2}: R2 (R1 when there is one register) is negated into R1.
		if ((iOperands < 1) || (iOperands > 2))
		{
			return false;
		}
		psLine->iFirst = (int8)IR65CodeRegister(&asOperands[0]);
		psLine->iSecond = (int8)IR65CodeRegister(psLast);
		psLine->bFirstRegister = true;
		return (psLine->iFirst >= 0) && (psLine->iSecond >= 0);

	case IR65CK_Push:
	case IR65CK_Pull:
		//Registers, memory or (psh) a value.  S, V and W are 32 bits; P and C would change or show the widths.
		if (iOperands != 1)
		{
			return false;
		}
		if ((asOperands[0].szStart[0] == '(') || (IR65CodeRegister(&asOperands[0]) >= 0))
		{
			return IR65CodeRegisterList(&asOperands[0], &psLine->uiRegisters);
		}
		psLine->bFirstMemory = (psLine->uiKind == IR65CK_Pull) && (asOperands[0].szStart[0] == '[');
		return (asOperands[0].uiLength > 0) && (asOperands[0].szStart[0] != 'P') && (asOperands[0].szStart[0] != 'C');

	case IR65CK_Width:
	case IR65CK_Extend:
		if (iOperands != 2)
		{
			return false;
		}
		psLine->uiBits = (uint8)atoi(asOperands[1].szStart);
		if ((psLine->uiBits != 8) && (psLine->uiBits != 16) && (psLine->uiBits != 32) && (psLine->uiBits != 64))
		{
			return false;
		}
		if (psLine->uiKind == IR65CK_Extend)
		{
			psLine->iFirst = (int8)IR65CodeRegister(&asOperands[0]);
			return psLine->iFirst >= 0;
		}
		return IR65CodeRegisterList(&asOperands[0], &psLine->uiRegisters) && (psLine->uiRegisters != 0);

	case IR65CK_Multiply:
	case IR65CK_Divide:
	case IR65CK_Return:
		return iOperands == 0;

	case IR65CK_Call:
		if (iOperands != 1)
		{
			return false;
		}
		psLine->iFirst = (int8)IR65CodeRegister(&asOperands[0]);
		return true;

	case IR65CK_Branch:
	case IR65CK_Jump:
		if (psLast == NULL)
		{
			return false;
		}
		if (psLine->uiKind == IR65CK_Branch)
		{
			//The flags named before the target: z, or (n, v).
			psLine->uiFlagsRead = IR65CodeFlags(&asOperands[0]);
			if ((iOperands != 2) || (psLine->uiFlagsRead == 0))
			{
				return false;
			}
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
// Bands) and a bit for each register whose width it depends on.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Code::Effect(size uiLine, SIR65Widths* psWidths, SIR65Widths* psReads, SIR65Widths* psWrites, uint8* puiUses)
{
	SIR65CodeLine*	psLine;
	uint8*			auiWidths;
	uint8			uiBits;
	int				i;

	psLine = masLines.Get(uiLine);
	auiWidths = psWidths->auiRegisters;
	memset(psReads, 0, sizeof(SIR65Widths));
	memset(psWrites, 0, sizeof(SIR65Widths));
	*puiUses = 0;

	for (i = 0; i < IR65_CODE_REGISTERS; i++)
	{
		if (psLine->uiAddresses & (1 << i))
		{
			*puiUses |= (uint8)(1 << i);
			psReads->auiRegisters[i] |= Bands(auiWidths[i]);
		}
	}

	switch (psLine->uiKind)
	{
	case IR65CK_Alu:
	case IR65CK_Negate:
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
				psReads->auiRegisters[psLine->iFirst] |= Bands(uiBits);
			}
			if (psLine->bWritesFirst)
			{
				psWrites->auiRegisters[psLine->iFirst] |= Bands(uiBits);
			}
		}
		if (psLine->iSecond >= 0)
		{
			*puiUses |= (uint8)(1 << psLine->iSecond);
			if (psLine->bShift || (auiWidths[psLine->iSecond] < uiBits))
			{
				psReads->auiRegisters[psLine->iSecond] |= Bands(auiWidths[psLine->iSecond]);
			}
			else
			{
				psReads->auiRegisters[psLine->iSecond] |= Bands(uiBits);
			}
		}
		//A register read and written in one instruction (add A, A) is still read.
		break;

	case IR65CK_Push:
	case IR65CK_Pull:
		for (i = 0; i < IR65_CODE_REGISTERS; i++)
		{
			if (psLine->uiRegisters & (1 << i))
			{
				*puiUses |= (uint8)(1 << i);
				if (psLine->uiKind == IR65CK_Push)
				{
					psReads->auiRegisters[i] |= Bands(auiWidths[i]);
				}
				else
				{
					psWrites->auiRegisters[i] |= Bands(auiWidths[i]);
				}
			}
		}
		break;

	case IR65CK_Width:
		for (i = 0; i < IR65_CODE_REGISTERS; i++)
		{
			if (psLine->uiRegisters & (1 << i))
			{
				auiWidths[i] = psLine->uiBits;
			}
		}
		break;

	case IR65CK_Extend:
		i = psLine->iFirst;
		*puiUses |= (uint8)(1 << i);
		psReads->auiRegisters[i] |= Bands(auiWidths[i]);
		if (psLine->uiBits > auiWidths[i])
		{
			psWrites->auiRegisters[i] |= Bands(psLine->uiBits) & ~Bands(auiWidths[i]);
		}
		auiWidths[i] = psLine->uiBits;
		break;

	case IR65CK_Multiply:
		*puiUses |= 0x1 | 0x2 | 0x8;
		psReads->auiRegisters[0] |= Bands(auiWidths[0]);
		psReads->auiRegisters[1] |= Bands(auiWidths[1]);
		psWrites->auiRegisters[3] |= Bands(auiWidths[3]);
		break;

	case IR65CK_Divide:
		*puiUses |= 0x1 | 0x2 | 0x8;
		psReads->auiRegisters[0] |= Bands(auiWidths[0]);
		psReads->auiRegisters[1] |= Bands(auiWidths[1]);
		psReads->auiRegisters[3] |= Bands(auiWidths[3]);
		psWrites->auiRegisters[1] |= Bands(auiWidths[1]);
		psWrites->auiRegisters[3] |= Bands(auiWidths[3]);
		break;

	case IR65CK_Call:
		//A function starts with all four 32 bits wide and leaves them so; a runtime routine may be called with
		//others (R_DivideSigned64 with A and B 64 bits) and leaves those as they were.  It may read them all (this
		//in X) and changes them all.
		*puiUses = IR65_CODE_ALL;
		for (i = 0; i < IR65_CODE_REGISTERS; i++)
		{
			psReads->auiRegisters[i] |= Bands(auiWidths[i]);
			psWrites->auiRegisters[i] = Bands(64);
			if (auiWidths[i] != 32)
			{
				auiWidths[i] = IR65_WIDTH_UNKNOWN;
			}
		}
		if (psLine->iFirst >= 0)
		{
			psReads->auiRegisters[psLine->iFirst] |= Bands(64);
		}
		break;

	case IR65CK_Return:
		//The result is A, all of it for a 64 bit one; stp may be looked at by anything.
		*puiUses = IR65_CODE_ALL;
		psReads->auiRegisters[0] = Bands(64);
		if (psLine->bReadsFirst)
		{
			for (i = 1; i < IR65_CODE_REGISTERS; i++)
			{
				psReads->auiRegisters[i] = Bands(64);
			}
		}
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::FallsThrough(size uiLine)
{
	SIR65CodeLine*	psLine;

	psLine = masLines.Get(uiLine);
	return (psLine->uiKind != IR65CK_Jump) && (psLine->uiKind != IR65CK_Return);
}


//////////////////////////////////////////////////////////////////////////
//
// The widths each line starts with: 32 bits at the function's label,
// then what the lines before it leave.  False if a line can be reached
// with two different widths of one register.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::ComputeWidths(void)
{
	size			ui;
	size			uiNum;
	size			uiNext;
	SIR65Widths		sWidths;
	SIR65Widths		sReads;
	SIR65Widths		sWrites;
	SIR65CodeLine*	psLine;
	SIR65Widths*	psTo;
	uint8			uiUses;
	uint8			uiBits;
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
			Effect(ui, &sWidths, &sReads, &sWrites, &uiUses);

			auiTo[0] = (FallsThrough(ui) && (ui + 1 < uiNum)) ? ui + 1 : uiNum;
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
				for (i = 0; i < IR65_CODE_REGISTERS; i++)
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

	//Code is only rewritten when the width of every register an instruction uses is known.
	for (ui = 0; ui < uiNum; ui++)
	{
		if (*mauiReached.Get(ui))
		{
			sWidths = *masWidths.Get(ui);
			Effect(ui, &sWidths, &sReads, &sWrites, &uiUses);
			for (i = 0; i < IR65_CODE_REGISTERS; i++)
			{
				uiBits = masWidths.Get(ui)->auiRegisters[i];
				if ((uiBits == IR65_WIDTH_CONFLICT) || ((uiBits == IR65_WIDTH_UNKNOWN) && (uiUses & (1 << i))))
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
// The flags read after each line before they are set again (IR65_FLAG_
// bits), worked out backwards.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Code::ComputeFlagDemand(CArrayTemplate<uint8>* pauiDemand)
{
	CArrayTemplate<uint8>	auiIn;
	size					ui;
	size					uiNum;
	SIR65CodeLine*			psLine;
	uint8					uiOut;
	uint8					uiIn;
	bool					bChanged;

	uiNum = masLines.NumElements();
	auiIn.Init();
	pauiDemand->ReInit();
	for (ui = 0; ui < uiNum; ui++)
	{
		auiIn.Add()[0] = 0;
		pauiDemand->Add()[0] = 0;
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
			psLine = masLines.Get(ui);
			uiOut = 0;
			if (FallsThrough(ui) && (ui + 1 < uiNum))
			{
				uiOut |= *auiIn.Get(ui + 1);
			}
			if (psLine->iTarget >= 0)
			{
				uiOut |= *auiIn.Get(psLine->iTarget);
			}
			uiIn = psLine->uiFlagsRead | (uiOut & ~psLine->uiFlagsWritten);
			*pauiDemand->Get(ui) = uiOut;
			if (uiIn != *auiIn.Get(ui))
			{
				*auiIn.Get(ui) = uiIn;
				bChanged = true;
			}
		}
	}
	while (bChanged);
	auiIn.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Takes a line out: it does nothing and isn't written.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Code::Remove(size uiLine)
{
	memset(masLines.Get(uiLine), 0, sizeof(SIR65CodeLine));
	masLines.Get(uiLine)->uiKind = IR65CK_Other;
	masLines.Get(uiLine)->iFirst = -1;
	masLines.Get(uiLine)->iSecond = -1;
	masLines.Get(uiLine)->iTarget = -1;
	*mauiRemoved.Get(uiLine) = 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::IsRemoved(size uiLine)
{
	return *mauiRemoved.Get(uiLine) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes a line another instruction, decoded; its widths stay those it
// started with, which the caller may change.  False (and the line as it
// was) if it isn't one the decoder knows.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::Replace(size uiLine, const char* szInstruction)
{
	CChars			szLine;
	SIR65CodeLine	sLine;

	szLine.Init("\t\t");
	szLine.Append(szInstruction);
	if (!DecodeLine(&szLine, &sLine))
	{
		szLine.Kill();
		return false;
	}
	if ((sLine.iTarget >= 0) && ((size)sLine.iTarget <= uiLine))
	{
		*mauiJumpedBack.Get(sLine.iTarget) = 1;
	}
	*masLines.Get(uiLine) = sLine;
	maszLines.Get(uiLine)->Kill();
	maszLines.Get(uiLine)->Init(szLine);
	szLine.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A line as it is (a removed line is not written), with the new line
// after it that the code had.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Code::WriteLine(CChars* pszOut, size uiLine)
{
	if (*mauiRemoved.Get(uiLine))
	{
		return;
	}
	pszOut->Append(maszLines.Get(uiLine));
	if (uiLine + 1 < maszLines.NumElements())
	{
		pszOut->AppendNewLine();
	}
}
