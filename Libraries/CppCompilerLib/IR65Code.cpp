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
#include "BaseLib/StringHelper.h"
#include "IR65Code.h"
#include "CPPNameCharacters.h"


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
	{ "tst",	IR65CK_Alu,			true,	false,	false,	false,	IR65_FLAG_N | IR65_FLAG_Z },
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
	return CIR65Code::RegisterIndex(psOperand->szStart, psOperand->uiLength);
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
		if (IsCPPNameCharacter(*sz))
		{
			szWord = sz;
			while ((sz < szEnd) && IsCPPNameCharacter(*sz))
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
		if (IsCPPNameCharacter(*sz))
		{
			szWord = sz;
			while ((sz < szEnd) && IsCPPNameCharacter(*sz))
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
	mauiWritten.Init();
	masLines.Init();
	masWidths.Init();
	mauiReached.Init();
	mbLoaded = false;
	mauiJumpedBack.Init();
	mauiRemoved.Init();
	maszNewLines.Init();
	mauiNewWritten.Init();
	mbLastKept = false;
	mmLabels.Init();
	mbReturnReadsCarry = false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a ret reads the carry (the flag a caller's thrown tests), so that
// what sets it before a ret isn't taken to be for nothing.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Code::SetReturnReadsCarry(bool bReads)
{
	mbReturnReadsCarry = bReads;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Code::Kill(void)
{
	mmLabels.Kill();
	mauiNewWritten.Kill();
	maszNewLines.Kill();
	mauiRemoved.Kill();
	mauiJumpedBack.Kill();
	mauiReached.Kill();
	masWidths.Kill();
	masLines.Kill();
	mauiWritten.Kill();
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
	SetText(pszCode, uiStart);
	return Analyse(bConflicts);
}


//////////////////////////////////////////////////////////////////////////
//
// The function's code from uiStart (its label first), split into lines,
// not yet decoded.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Code::SetText(CChars* pszCode, size uiStart)
{
	CChars		szText;
	CArrayChars	aszLines;
	size		ui;

	aszLines.Init();
	szText.Init(pszCode->Text() + uiStart);
	szText.SplitLines(&aszLines);
	szText.Kill();
	maszLines.ReInit();
	mauiWritten.ReInit();
	for (ui = 0; ui < aszLines.NumElements(); ui++)
	{
		AddTextLine(&maszLines, &mauiWritten, aszLines.Get(ui)->Text());
	}
	aszLines.Kill();
	masLines.ReInit();
	mauiRemoved.ReInit();
	for (ui = 0; ui < maszLines.NumElements(); ui++)
	{
		mauiRemoved.Add()[0] = 0;
	}
	mbLoaded = false;
}


//////////////////////////////////////////////////////////////////////////
//
// A line of text added to paszLines: "\t\tmnemonic operands" (or with no
// operands) as an instruction, its mnemonic and operands; any other line,
// or one that wouldn't print as it was written, as its text.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Code::AddTextLine(CArrayChars* paszLines, CArrayTemplate<uint8>* pauiWritten, const char* szLine)
{
	const char*	szName;
	size		uiName;
	uint8		uiMnemonic;

	if ((szLine[0] == '\t') && (szLine[1] == '\t'))
	{
		szName = szLine + 2;
		for (uiName = 0; (szName[uiName] != '\0') && (szName[uiName] != ' '); uiName++)
		{
		}
		for (uiMnemonic = 0; uiMnemonic < IR65M_Unknown; uiMnemonic++)
		{
			if ((StrLen(gasIR65CodeMnemonics[uiMnemonic].szName) == uiName) && (strncmp(gasIR65CodeMnemonics[uiMnemonic].szName, szName, uiName) == 0))
			{
				break;
			}
		}
		if ((uiMnemonic < IR65M_Unknown) && ((szName[uiName] == '\0') || (szName[uiName + 1] != '\0')))
		{
			paszLines->Add((szName[uiName] == '\0') ? "" : szName + uiName + 1);
			pauiWritten->Add()[0] = uiMnemonic;
			return;
		}
	}
	paszLines->Add(szLine);
	pauiWritten->Add()[0] = IR65M_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
// Decodes the lines as they are now (the lines passes took out go first);
// false for code the passes must leave as it is.  With bConflicts a
// register may have a width that depends on the way a line was reached
// (IR65_WIDTH_CONFLICT), as it can after the Widths pass.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::Analyse(bool bConflicts)
{
	Compact();
	mbLoaded = Decode() && (ComputeWidths() || (bConflicts && (masLines.NumElements() > 0)));
	return mbLoaded;
}


//////////////////////////////////////////////////////////////////////////
//
// The lines without those taken out, as they would be split from the
// code written (WriteLine): a last line taken out leaves the new line
// before it, so an empty line ends them.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Code::Compact(void)
{
	size	ui;
	size	uiNum;
	bool	bRemoved;

	uiNum = maszLines.NumElements();
	bRemoved = false;
	for (ui = 0; ui < uiNum; ui++)
	{
		if (*mauiRemoved.Get(ui))
		{
			bRemoved = true;
			break;
		}
	}
	if (!bRemoved)
	{
		return;
	}
	BeginLines();
	for (ui = 0; ui < uiNum; ui++)
	{
		KeepLine(ui);
	}
	EndLines();
}


//////////////////////////////////////////////////////////////////////////
//
// The code as it is now, every line but those taken out.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Code::Write(CChars* pszOut)
{
	size	ui;

	for (ui = 0; ui < maszLines.NumElements(); ui++)
	{
		WriteLine(pszOut, ui);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Starts writing the code again: the lines kept (KeepLine) and added
// (AddLine) from here are the code's when EndLines is called, and until
// then the code is as it was.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Code::BeginLines(void)
{
	maszNewLines.ReInit();
	mauiNewWritten.ReInit();
	mbLastKept = false;
}


//////////////////////////////////////////////////////////////////////////
//
// The code's line uiLine, unless a pass took it out, next in the code
// being written again.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Code::KeepLine(size uiLine)
{
	if (*mauiRemoved.Get(uiLine))
	{
		return;
	}
	maszNewLines.Add(maszLines.Get(uiLine)->Text());
	mauiNewWritten.Add()[0] = *mauiWritten.Get(uiLine);
	mbLastKept = (uiLine + 1 == maszLines.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
// A new instruction next in the code being written again: uiMnemonic
// with szOperands (NULL for none).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Code::AddLine(uint8 uiMnemonic, const char* szOperands)
{
	maszNewLines.Add(szOperands ? szOperands : "");
	mauiNewWritten.Add()[0] = uiMnemonic;
	mbLastKept = false;
}


//////////////////////////////////////////////////////////////////////////
//
// The code written again is the code, not decoded until the next Analyse.
// Each line written ended with a new line but the code's last, so without
// that an empty line ends them.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Code::EndLines(void)
{
	size	ui;

	if (!mbLastKept)
	{
		maszNewLines.Add("");
		mauiNewWritten.Add()[0] = IR65M_Unknown;
	}
	maszLines.ReInit();
	mauiWritten.ReInit();
	mauiRemoved.ReInit();
	for (ui = 0; ui < maszNewLines.NumElements(); ui++)
	{
		maszLines.Add(maszNewLines.Get(ui)->Text());
		mauiWritten.Add()[0] = *mauiNewWritten.Get(ui);
		mauiRemoved.Add()[0] = 0;
	}
	maszNewLines.ReInit();
	mauiNewWritten.ReInit();
	masLines.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// Takes a line out of code not decoded: it isn't written.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Code::Drop(size uiLine)
{
	*mauiRemoved.Get(uiLine) = 1;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the last Analyse could read the code.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::IsLoaded(void) const
{
	return mbLoaded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIR65Code::NumLines(void)
{
	return maszLines.NumElements();
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
CChars* CIR65Code::LineText(size uiLine)
{
	return maszLines.Get(uiLine);
}


//////////////////////////////////////////////////////////////////////////
//
// The mnemonic a line was written with (IR65M_Unknown for a line that is
// text), known before the code is decoded.
//
//////////////////////////////////////////////////////////////////////////
uint8 CIR65Code::WrittenMnemonic(size uiLine)
{
	return *mauiWritten.Get(uiLine);
}


//////////////////////////////////////////////////////////////////////////
//
// Makes a line of code not decoded the instruction uiMnemonic with the
// operands szText, or (IR65M_Unknown) the text szText.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Code::SetLine(size uiLine, uint8 uiMnemonic, const char* szText)
{
	maszLines.Get(uiLine)->Kill();
	maszLines.Get(uiLine)->Init(szText);
	*mauiWritten.Get(uiLine) = uiMnemonic;
}


//////////////////////////////////////////////////////////////////////////
//
// A line as it is printed: "\t\tmnemonic operands" for an instruction,
// any other line's text.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Code::PrintLine(CChars* pszOut, size uiLine)
{
	uint8	uiMnemonic;
	CChars*	pszText;

	uiMnemonic = *mauiWritten.Get(uiLine);
	pszText = maszLines.Get(uiLine);
	if (uiMnemonic == IR65M_Unknown)
	{
		pszOut->Append(pszText);
		return;
	}
	pszOut->Append("\t\t");
	pszOut->Append(gasIR65CodeMnemonics[uiMnemonic].szName);
	if (!pszText->Empty())
	{
		pszOut->Append(' ');
		pszOut->Append(pszText);
	}
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
// 0 to 3 for a register A, B, X or Y named by sz (uiLength long: just the
// name), -1 for anything else.
//
//////////////////////////////////////////////////////////////////////////
int CIR65Code::RegisterIndex(const char* sz, size uiLength)
{
	int	i;

	if (uiLength != 1)
	{
		return -1;
	}
	for (i = 0; i < IR65_CODE_REGISTERS; i++)
	{
		if (sz[0] == gacIR65CodeRegisters[i])
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The function's own memory, [n + W], [n + V] or [n + S] (a pseudo
// register or the frame, which only the function's own code touches, so
// never volatile): its base register ('W', 'V' or 'S') and offset.  False
// for any other operand, and for [n + V] where V is a value's home
// (bHomeV: SIR65Frame::bHomeV), which is anyone's memory.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::IsOwnMemory(const char* szOperand, size uiLength, bool bHomeV, uint8* puiBase, int32* piOffset)
{
	size	ui;
	char	cBase;

	if ((uiLength < 7) || (szOperand[0] != '[') || (szOperand[uiLength - 1] != ']'))
	{
		return false;
	}
	cBase = szOperand[uiLength - 2];
	if (((cBase != 'W') && (cBase != 'V') && (cBase != 'S')) || (memcmp(&szOperand[uiLength - 5], " + ", 3) != 0) || (bHomeV && (cBase == 'V')))
	{
		return false;
	}
	for (ui = 1; ui < uiLength - 5; ui++)
	{
		if ((szOperand[ui] < '0') || (szOperand[ui] > '9'))
		{
			return false;
		}
	}
	*puiBase = (uint8)cBase;
	*piOffset = atoi(&szOperand[1]);
	return true;
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
	bool			bDecoded;

	uiNum = maszLines.NumElements();
	mmLabels.ReInit();
	for (ui = 0; ui < uiNum; ui++)
	{
		pszLine = maszLines.Get(ui);
		if ((*mauiWritten.Get(ui) == IR65M_Unknown) && !pszLine->Empty() && !pszLine->StartsWith("\t") && pszLine->EndsWith(':'))
		{
			*mmLabels.FindOrAdd(pszLine->Text(), (size)(pszLine->Length() - 1), NULL) = (int32)ui;
		}
	}

	masLines.ReInit();
	mauiJumpedBack.ReInit();
	for (ui = 0; ui < uiNum; ui++)
	{
		psLine = masLines.Add();
		mauiJumpedBack.Add()[0] = 0;
		bDecoded = DecodeLine(ui, psLine);
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
bool CIR65Code::DecodeLine(size uiLine, SIR65CodeLine* psLine)
{
	if (*mauiWritten.Get(uiLine) != IR65M_Unknown)
	{
		return DecodeInstruction(*mauiWritten.Get(uiLine), maszLines.Get(uiLine)->Text(), psLine);
	}
	return DecodeText(maszLines.Get(uiLine), psLine);
}


//////////////////////////////////////////////////////////////////////////
//
// Decodes a line that is text: a label, an empty line or a comment, or an
// instruction written otherwise than "\t\tmnemonic operands".
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::DecodeText(CChars* pszLine, SIR65CodeLine* psLine)
{
	const char*					szText;
	char						szMnemonic[8];
	size						uiMnemonic;
	const SIR65CodeMnemonic*	psMnemonic;

	memset(psLine, 0, sizeof(SIR65CodeLine));
	psLine->iFirst = -1;
	psLine->iSecond = -1;
	psLine->iTarget = -1;
	psLine->uiKind = IR65CK_Other;
	psLine->uiMnemonic = IR65M_Unknown;

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
	return DecodeInstruction((uint8)(psMnemonic - gasIR65CodeMnemonics), szText + uiMnemonic, psLine);
}


//////////////////////////////////////////////////////////////////////////
//
// Decodes the instruction uiMnemonic with its operands (as written after
// the mnemonic).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::DecodeInstruction(uint8 uiMnemonic, const char* szOperands, SIR65CodeLine* psLine)
{
	SIR65CodeOperand			asOperands[IR65_CODE_MAX_OPERANDS];
	SIR65CodeOperand*			psLast;
	const SIR65CodeMnemonic*	psMnemonic;
	int							iOperands;
	int							i;
	int32*						piTarget;

	memset(psLine, 0, sizeof(SIR65CodeLine));
	psLine->iFirst = -1;
	psLine->iSecond = -1;
	psLine->iTarget = -1;
	psMnemonic = &gasIR65CodeMnemonics[uiMnemonic];
	iOperands = IR65CodeOperands(szOperands, asOperands);
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
	psLine->uiMnemonic = uiMnemonic;
	psLine->bReadsFirst = psMnemonic->bReadsFirst;
	psLine->bWritesFirst = psMnemonic->bWritesFirst;
	psLine->bShift = psMnemonic->bShift;
	psLine->bResultFlags = psMnemonic->bResultFlags;
	psLine->uiFlagsWritten = psMnemonic->uiFlags;
	if (mbReturnReadsCarry && (psMnemonic->uiKind == IR65CK_Return) && !psMnemonic->bReadsFirst)
	{
		//ret (not stp).
		psLine->uiFlagsRead = IR65_FLAG_C;
	}
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
		if (psLine->bFirstMemory && (uiMnemonic == IR65M_Cpyx))
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
		piTarget = mmLabels.Get(psLast->szStart, (size)psLast->uiLength);
		psLine->iTarget = piTarget ? *piTarget : -1;
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
		*puiUses |= IR65_REGISTER_BIT(IR65CR_A) | IR65_REGISTER_BIT(IR65CR_B) | IR65_REGISTER_BIT(IR65CR_Y);
		psReads->auiRegisters[IR65CR_A] |= Bands(auiWidths[IR65CR_A]);
		psReads->auiRegisters[IR65CR_B] |= Bands(auiWidths[IR65CR_B]);
		psWrites->auiRegisters[IR65CR_Y] |= Bands(auiWidths[IR65CR_Y]);
		break;

	case IR65CK_Divide:
		*puiUses |= IR65_REGISTER_BIT(IR65CR_A) | IR65_REGISTER_BIT(IR65CR_B) | IR65_REGISTER_BIT(IR65CR_Y);
		psReads->auiRegisters[IR65CR_A] |= Bands(auiWidths[IR65CR_A]);
		psReads->auiRegisters[IR65CR_B] |= Bands(auiWidths[IR65CR_B]);
		psReads->auiRegisters[IR65CR_Y] |= Bands(auiWidths[IR65CR_Y]);
		psWrites->auiRegisters[IR65CR_B] |= Bands(auiWidths[IR65CR_B]);
		psWrites->auiRegisters[IR65CR_Y] |= Bands(auiWidths[IR65CR_Y]);
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
		psReads->auiRegisters[IR65CR_A] = Bands(64);
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
// How the line goes on, for the shared branch passes: anything else not
// run (a comment, a directive) is an instruction they may take out.
//
//////////////////////////////////////////////////////////////////////////
EIRCodeFlow CIR65Code::Flow(size uiLine)
{
	switch (masLines.Get(uiLine)->uiKind)
	{
	case IR65CK_Label:
		return IRCF_Label;
	case IR65CK_Branch:
		return IRCF_Branch;
	case IR65CK_Jump:
		return IRCF_Jump;
	case IR65CK_Return:
		return IRCF_End;
	case IR65CK_Other:
		return (IsRemoved(uiLine) || ((*mauiWritten.Get(uiLine) == IR65M_Unknown) && maszLines.Get(uiLine)->Empty())) ? IRCF_None : IRCF_Instruction;
	default:
		return IRCF_Instruction;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIR65Code::Target(size uiLine)
{
	int32	iTarget;

	iTarget = masLines.Get(uiLine)->iTarget;
	return (iTarget >= 0) ? (size)iTarget : IR_NO_LINE;
}


//////////////////////////////////////////////////////////////////////////
//
// Only the function's label: Load fails for code with a branch out of it,
// and nothing else names its labels.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::IsEntry(size uiLine)
{
	return uiLine == 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The branch or jump at uiLine (bra, bras, brac, brasa or braso) with its
// condition, to the label at uiLabel.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::Retarget(size uiLine, size uiLabel)
{
	return Branch(uiLine, Mnemonic(uiLine), uiLabel);
}


//////////////////////////////////////////////////////////////////////////
//
// bras to the label at uiLabel for a brac, brac for a bras; false for
// brasa and braso, which have no opposite.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::Invert(size uiLine, size uiLabel)
{
	if (Mnemonic(uiLine) == IR65M_Bras)
	{
		return Branch(uiLine, IR65M_Brac, uiLabel);
	}
	if (Mnemonic(uiLine) == IR65M_Brac)
	{
		return Branch(uiLine, IR65M_Bras, uiLabel);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes the branch or jump at uiLine uiMnemonic (bra, bras or brac ...)
// with its condition, to the label at uiLabel.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::Branch(size uiLine, uint8 uiMnemonic, size uiLabel)
{
	CChars*		pszLabel;
	CChars		szOperands;
	const char*	szOperand;
	const char*	szTarget;
	size		uiOperands;
	bool		bReplaced;

	pszLabel = LineText(uiLabel);
	szOperands.Init();
	if (GetLine(uiLine)->uiKind == IR65CK_Branch)
	{
		//"bras z, L5": the condition is before the target.
		szOperand = OperandsText(uiLine, &uiOperands);
		szTarget = FindChar(szOperand, ',', true);
		szOperands.AppendSubString(szOperand, szTarget + 2);
	}
	szOperands.AppendSubString(pszLabel->Text(), pszLabel->Length() - 1);
	bReplaced = ReplaceInstruction(uiLine, uiMnemonic, szOperands.Text());
	szOperands.Kill();
	return bReplaced;
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
	masLines.Get(uiLine)->uiMnemonic = IR65M_Unknown;
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
// Makes a line the instruction uiMnemonic with szOperands (NULL for none),
// decoded; its widths stay those it started with, which the caller may
// change.  False (and the line as it was) if it isn't one the decoder
// knows.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::ReplaceInstruction(size uiLine, uint8 uiMnemonic, const char* szOperands)
{
	SIR65CodeLine	sLine;

	if (szOperands == NULL)
	{
		szOperands = "";
	}
	if (!DecodeInstruction(uiMnemonic, szOperands, &sLine))
	{
		return false;
	}
	if ((sLine.iTarget >= 0) && ((size)sLine.iTarget <= uiLine))
	{
		*mauiJumpedBack.Get(sLine.iTarget) = 1;
	}
	*masLines.Get(uiLine) = sLine;
	SetLine(uiLine, uiMnemonic, szOperands);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A line's mnemonic (IR65M_Unknown for a label or a line that isn't an
// instruction).
//
//////////////////////////////////////////////////////////////////////////
uint8 CIR65Code::Mnemonic(size uiLine)
{
	return masLines.Get(uiLine)->uiMnemonic;
}


//////////////////////////////////////////////////////////////////////////
//
// An instruction's operands, as written after its mnemonic ("[8 + W], A"),
// and their length; "" for none.
//
//////////////////////////////////////////////////////////////////////////
const char* CIR65Code::OperandsText(size uiLine, size* puiLength)
{
	const char*	sz;

	if (*mauiWritten.Get(uiLine) != IR65M_Unknown)
	{
		*puiLength = maszLines.Get(uiLine)->Length();
		return maszLines.Get(uiLine)->Text();
	}
	sz = maszLines.Get(uiLine)->Text();
	while ((*sz == '\t') || (*sz == ' '))
	{
		sz++;
	}
	while ((*sz != '\0') && (*sz != ' '))
	{
		sz++;
	}
	if (*sz == ' ')
	{
		sz++;
	}
	*puiLength = StrLen(sz);
	return sz;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an instruction's operands have szFind in them.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::OperandsHave(size uiLine, const char* szFind)
{
	size	uiLength;

	return strstr(OperandsText(uiLine, &uiLength), szFind) != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line is the instruction uiMnemonic with exactly the operands
// szOperands.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::IsWith(size uiLine, uint8 uiMnemonic, const char* szOperands)
{
	size	uiLength;

	return (Mnemonic(uiLine) == uiMnemonic) && (StringCompare(OperandsText(uiLine, &uiLength), szOperands) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether two lines are the same instruction (mnemonic and operands).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Code::SameInstruction(size uiLine1, size uiLine2)
{
	size	uiLength;

	return (Mnemonic(uiLine1) == Mnemonic(uiLine2)) && (StringCompare(OperandsText(uiLine1, &uiLength), OperandsText(uiLine2, &uiLength)) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CIR65Code::MnemonicName(uint8 uiMnemonic)
{
	return gasIR65CodeMnemonics[uiMnemonic].szName;
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
	PrintLine(pszOut, uiLine);
	if (uiLine + 1 < maszLines.NumElements())
	{
		pszOut->AppendNewLine();
	}
}
