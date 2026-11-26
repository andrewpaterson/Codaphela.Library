/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "StringHelper.h"
#include "TextParser.h"
#include "Regex.h"


//The steps a match from one place may take before it is given up on: these, and some for each character of the
//text after the place.  A pattern that only walks the text takes a few for each character; one that backtracks
//without end takes more than any text has.
#define REGEX_STEPS				200000
#define REGEX_STEPS_PER_CHAR	16


//////////////////////////////////////////////////////////////////////////
//
// The most elements of uiElementSize bytes one of a pattern's arrays is
// let hold: uiWanted, or fewer where a size can't count as many bytes.
// An array's bytes are counted in a size, and it grows past what it
// holds: with a 16 bit size (the W65C816's) that is under 64K bytes, so
// it is kept to a quarter of what a size can count.
//
//////////////////////////////////////////////////////////////////////////
static size RegexMostElements(size uiElementSize, uint32 uiWanted)
{
	size	uiMost;

	uiMost = (size)((MAX_SIZE / 4) / uiElementSize);
	if (uiMost > uiWanted)
	{
		uiMost = (size)uiWanted;
	}
	return uiMost;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsRegexWord(char c)
{
	return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')) || (c == '_');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsRegexLetter(char c)
{
	return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsRegexSpace(char c)
{
	return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') || (c == '\f') || (c == '\v');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static char RegexLower(char c)
{
	return ((c >= 'A') && (c <= 'Z')) ? (char)(c + ('a' - 'A')) : c;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void SetRegexClassBit(SRegexClass* psClass, uint8 ui)
{
	psClass->auiBits[ui >> 3] |= (uint8)(1 << (ui & 7));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsRegexClassBit(SRegexClass* psClass, uint8 ui)
{
	return (psClass->auiBits[ui >> 3] & (uint8)(1 << (ui & 7))) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The characters \d, \w and \s name ('d', 'w' or 's') are added to the
// class, or with bNot those they don't.
//
//////////////////////////////////////////////////////////////////////////
static void AddRegexClassSet(SRegexClass* psClass, char cSet, bool bNot)
{
	uint16	ui;
	bool	bIn;

	for (ui = 0; ui < 256; ui++)
	{
		if (cSet == 'd')
		{
			bIn = (ui >= '0') && (ui <= '9');
		}
		else if (cSet == 'w')
		{
			bIn = IsRegexWord((char)ui);
		}
		else
		{
			bIn = IsRegexSpace((char)ui);
		}
		if (bIn != bNot)
		{
			SetRegexClassBit(psClass, (uint8)ui);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The pattern's character iAhead on from the parser's, 0 past its end.
//
//////////////////////////////////////////////////////////////////////////
static char PeekRegex(CTextParser* pcParser, int iAhead)
{
	if (pcParser->mbOutsideText || (&pcParser->mszParserPos[iAhead] > pcParser->mszEndOfText))
	{
		return '\0';
	}
	return pcParser->mszParserPos[iAhead];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRegex::Init(void)
{
	masProgram.Init();
	masClasses.Init();
	masStack.Init();
	mauiMarks.Init();
	mszError.Init();
	muiErrorPosition = 0;
	muiSteps = 0;
	muiStepLimit = 0;
	muiNumGroups = 0;
	muiNumMarks = 0;
	muiDepth = 0;
	mbCaseSensitive = true;
	mbCompiled = false;
	mbGaveUp = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRegex::Kill(void)
{
	mszError.Kill();
	mauiMarks.Kill();
	masStack.Kill();
	masClasses.Kill();
	masProgram.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The pattern is compiled, in place of any there was.  False if it isn't
// a pattern: GetError says why and GetErrorPosition where in it.
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::Compile(const char* szPattern, bool bCaseSensitive)
{
	CTextParser	cParser;
	bool		bNullable;
	bool		bResult;

	masProgram.ReInit();
	masClasses.ReInit();
	mszError.Clear();
	muiErrorPosition = 0;
	muiNumGroups = 0;
	muiNumMarks = 0;
	muiDepth = 0;
	mbCaseSensitive = bCaseSensitive;
	mbCompiled = false;
	mbGaveUp = false;

	if (strchr(szPattern, '\r') != NULL)
	{
		//The text parser doesn't take them.
		mszError.Append("A pattern can't have a carriage return in it: \\r matches one.");
		muiErrorPosition = (size)(strchr(szPattern, '\r') - szPattern);
		return false;
	}

	cParser.Init(szPattern);
	bResult = ParseAlternation(&cParser, &bNullable);
	if (bResult && !cParser.mbOutsideText)
	{
		//Only a ) stops the pattern short of its end.
		bResult = Error(&cParser, "There is a ) with no ( before it.");
	}
	bResult = bResult && Emit(&cParser, RO_Match, 0, 0);
	cParser.Kill();

	if (!bResult)
	{
		masProgram.ReInit();
		masClasses.ReInit();
		return false;
	}
	mauiMarks.ReInit();
	mauiMarks.GrowToAtLeastNumElements(muiNumMarks, true, 0);
	mbCompiled = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Why the pattern isn't one, and where the parser had got to.  Always
// false.
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::Error(CTextParser* pcParser, const char* szError)
{
	if (mszError.Empty())
	{
		mszError.Append(szError);
		muiErrorPosition = pcParser->mbOutsideText ? (size)pcParser->miTextLen : (size)(pcParser->mszParserPos - pcParser->mszStartOfText);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// An instruction at the program's end.  False if the program has grown
// too big.
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::Emit(CTextParser* pcParser, uint8 uiOp, int16 iX, int16 iY)
{
	SRegexInstruction*	psInstruction;

	if (masProgram.NumElements() >= RegexMostElements(sizeof(SRegexInstruction), REGEX_MAX_PROGRAM))
	{
		return Error(pcParser, "The pattern is too big: its repeats multiply out to too much.");
	}
	psInstruction = masProgram.Add();
	psInstruction->uiOp = uiOp;
	psInstruction->uiChar = 0;
	psInstruction->iX = iX;
	psInstruction->iY = iY;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The character c: a letter in either case when case is ignored.
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::EmitCharacter(CTextParser* pcParser, char c)
{
	if (!mbCaseSensitive && IsRegexLetter(c))
	{
		if (!Emit(pcParser, RO_CharNoCase, 0, 0))
		{
			return false;
		}
		masProgram.Tail()->uiChar = (uint8)RegexLower(c);
		return true;
	}
	if (!Emit(pcParser, RO_Char, 0, 0))
	{
		return false;
	}
	masProgram.Tail()->uiChar = (uint8)c;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A character of the class: with case ignored a letter in it is in it in
// both cases.
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::EmitClass(CTextParser* pcParser, SRegexClass* psClass)
{
	SRegexClass*	psAdded;
	char			c;

	if (!mbCaseSensitive)
	{
		for (c = 'a'; c <= 'z'; c++)
		{
			if (IsRegexClassBit(psClass, (uint8)c) || IsRegexClassBit(psClass, (uint8)(c - ('a' - 'A'))))
			{
				SetRegexClassBit(psClass, (uint8)c);
				SetRegexClassBit(psClass, (uint8)(c - ('a' - 'A')));
			}
		}
	}
	if (masClasses.NumElements() >= MAX_INT16)
	{
		return Error(pcParser, "The pattern has too many character classes.");
	}
	psAdded = masClasses.Add();
	memcpy(psAdded, psClass, sizeof(SRegexClass));
	return Emit(pcParser, RO_Class, (int16)(masClasses.NumElements() - 1), 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The block's instructions again at the program's end: their jumps are
// relative, so they are right wherever they are.
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::EmitBlock(CTextParser* pcParser, CArrayRegexInstructions* pasBlock)
{
	size	i;

	if (masProgram.NumElements() + pasBlock->NumElements() > RegexMostElements(sizeof(SRegexInstruction), REGEX_MAX_PROGRAM))
	{
		return Error(pcParser, "The pattern is too big: its repeats multiply out to too much.");
	}
	for (i = 0; i < pasBlock->NumElements(); i++)
	{
		masProgram.Add(pasBlock->Get(i));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// x|y|z: each alternative but the last starts with a split, to it or to
// the next, and ends with a jump past them all.  The split is put in
// before the alternative once the | after it is seen.  pbNullable is
// whether what was parsed can match nothing.
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::ParseAlternation(CTextParser* pcParser, bool* pbNullable)
{
	CArrayTemplate<uint16>	auiJumps;
	SRegexInstruction*		psInstruction;
	SRegexInstruction		sSplit;
	size					uiAlternative;
	size					i;
	uint16					uiJump;
	bool					bNullable;
	bool					bResult;

	auiJumps.Init();
	*pbNullable = false;
	bResult = true;
	for (;;)
	{
		uiAlternative = masProgram.NumElements();
		bResult = ParseSequence(pcParser, &bNullable);
		if (!bResult)
		{
			break;
		}
		*pbNullable = *pbNullable || bNullable;
		if (PeekRegex(pcParser, 0) != '|')
		{
			break;
		}
		pcParser->StepRight();

		//The split goes to the alternative, after it, or past the jump that will end it.
		sSplit.uiOp = RO_Split;
		sSplit.uiChar = 0;
		sSplit.iX = 1;
		sSplit.iY = (int16)(masProgram.NumElements() - uiAlternative + 2);
		masProgram.InsertAt(&sSplit, uiAlternative);
		bResult = Emit(pcParser, RO_Jump, 0, 0);
		if (!bResult)
		{
			break;
		}
		uiJump = (uint16)(masProgram.NumElements() - 1);
		auiJumps.Add(&uiJump);
	}

	for (i = 0; bResult && (i < auiJumps.NumElements()); i++)
	{
		uiJump = *auiJumps.Get(i);
		psInstruction = masProgram.Get(uiJump);
		psInstruction->iX = (int16)(masProgram.NumElements() - uiJump);
	}
	auiJumps.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// One thing after another, up to a |, a ) or the pattern's end.  It can
// match nothing if everything in it can.
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::ParseSequence(CTextParser* pcParser, bool* pbNullable)
{
	char	c;
	bool	bNullable;

	*pbNullable = true;
	for (;;)
	{
		c = PeekRegex(pcParser, 0);
		if (pcParser->mbOutsideText || (c == '|'))
		{
			return true;
		}
		if (c == ')')
		{
			if (muiDepth == 0)
			{
				return Error(pcParser, "There is a ) with no ( before it.");
			}
			return true;
		}
		if (!ParseQuantified(pcParser, &bNullable))
		{
			return false;
		}
		*pbNullable = *pbNullable && bNullable;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// One thing and how many of it: x, x*, x+?, x{2,3}.
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::ParseQuantified(CTextParser* pcParser, bool* pbNullable)
{
	size	uiAtom;
	uint16	uiMinimum;
	uint16	uiMaximum;
	char	c;
	int		iQuantifier;
	bool	bLazy;
	bool	bNullable;

	uiAtom = masProgram.NumElements();
	if (!ParseAtom(pcParser, &bNullable))
	{
		return false;
	}
	*pbNullable = bNullable;

	iQuantifier = ParseQuantifier(pcParser, &uiMinimum, &uiMaximum, &bLazy);
	if (iQuantifier < 0)
	{
		return false;
	}
	if (iQuantifier == 0)
	{
		return true;
	}
	c = PeekRegex(pcParser, 0);
	if ((c == '*') || (c == '+') || (c == '?'))
	{
		return Error(pcParser, "A repeat can't follow a repeat: put the first in a group.");
	}
	*pbNullable = bNullable || (uiMinimum == 0);
	return Repeat(pcParser, uiAtom, uiMinimum, uiMaximum, bLazy, bNullable);
}


//////////////////////////////////////////////////////////////////////////
//
// A count's digits.  False if there are none, or the count is too big,
// which is an error.
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::ParseCount(CTextParser* pcParser, uint16* puiCount)
{
	uint16	uiDigit;
	uint16	uiCount;
	bool	bDigits;

	uiCount = 0;
	bDigits = false;
	while (!pcParser->mbOutsideText && (pcParser->GetDigit(&uiDigit) == TRITRUE))
	{
		uiCount = (uint16)(uiCount * 10 + uiDigit);
		bDigits = true;
		if (uiCount > REGEX_MAX_REPEAT)
		{
			return Error(pcParser, "A repeat's count is too big.");
		}
	}
	*puiCount = uiCount;
	return bDigits;
}


//////////////////////////////////////////////////////////////////////////
//
// The repeat after a thing, if there is one: *, +, ?, {n}, {n,} or {n,m},
// and a ? after it for as few as will do.  1 if there was one, 0 if not,
// -1 for one that is wrong.  A { that doesn't start a count is left: it is
// a character.
//
//////////////////////////////////////////////////////////////////////////
int CRegex::ParseQuantifier(CTextParser* pcParser, uint16* puiMinimum, uint16* puiMaximum, bool* pbLazy)
{
	char	c;
	bool	bCount;

	*pbLazy = false;
	c = PeekRegex(pcParser, 0);
	if (c == '*')
	{
		*puiMinimum = 0;
		*puiMaximum = REGEX_UNBOUNDED;
		pcParser->StepRight();
	}
	else if (c == '+')
	{
		*puiMinimum = 1;
		*puiMaximum = REGEX_UNBOUNDED;
		pcParser->StepRight();
	}
	else if (c == '?')
	{
		*puiMinimum = 0;
		*puiMaximum = 1;
		pcParser->StepRight();
	}
	else if (c == '{')
	{
		pcParser->PushPosition();
		pcParser->StepRight();
		bCount = ParseCount(pcParser, puiMinimum);
		*puiMaximum = *puiMinimum;
		if (bCount && (PeekRegex(pcParser, 0) == ','))
		{
			pcParser->StepRight();
			if (PeekRegex(pcParser, 0) == '}')
			{
				*puiMaximum = REGEX_UNBOUNDED;
			}
			else
			{
				bCount = ParseCount(pcParser, puiMaximum);
			}
		}
		if (!mszError.Empty())
		{
			pcParser->PassPosition();
			return -1;
		}
		if (!bCount || (PeekRegex(pcParser, 0) != '}'))
		{
			pcParser->PopPosition();
			return 0;
		}
		pcParser->StepRight();
		pcParser->PassPosition();
		if (*puiMaximum < *puiMinimum)
		{
			Error(pcParser, "A {n,m} repeat's numbers are the wrong way round.");
			return -1;
		}
	}
	else
	{
		return 0;
	}

	if (PeekRegex(pcParser, 0) == '?')
	{
		*pbLazy = true;
		pcParser->StepRight();
	}
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
// What was compiled from uiAtom on is repeated: taken off the program and
// put back as many times as it must match, then as a loop (none or more,
// one or more) or as many more times as it may, each of those skipped by a
// split.  A loop whose body can match nothing marks where each turn starts
// and leaves when a turn has matched nothing, or it would turn for ever.
// A split's first way is tried first: for as many as it can that is into
// the body, for as few (bLazy) past it.
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::Repeat(CTextParser* pcParser, size uiAtom, uint16 uiMinimum, uint16 uiMaximum, bool bLazy, bool bNullable)
{
	CArrayRegexInstructions	asBlock;
	CArrayTemplate<uint16>	auiSplits;
	SRegexInstruction*		psInstruction;
	size					i;
	size					uiLoop;
	uint16					uiPlain;
	uint16					uiSplit;
	uint16					uiMark;
	int16					iOut;
	bool					bResult;

	asBlock.Init();
	for (i = uiAtom; i < masProgram.NumElements(); i++)
	{
		asBlock.Add(masProgram.Get(i));
	}
	masProgram.RemoveRange(uiAtom, masProgram.NumElements());

	//The ones that must be there; a one-or-more loop is the last of them.
	uiPlain = uiMinimum;
	if ((uiMaximum == REGEX_UNBOUNDED) && (uiMinimum > 0))
	{
		uiPlain = (uint16)(uiMinimum - 1);
	}
	bResult = true;
	for (i = 0; bResult && (i < uiPlain); i++)
	{
		bResult = EmitBlock(pcParser, &asBlock);
	}

	uiMark = 0;
	if (bResult && (uiMaximum == REGEX_UNBOUNDED) && bNullable)
	{
		uiMark = muiNumMarks;
		muiNumMarks++;
	}

	if (bResult && (uiMaximum == REGEX_UNBOUNDED) && (uiMinimum == 0))
	{
		//None or more:  L: split in, out;  [mark]  body  [progress, or out]  jump L;  out:
		uiLoop = masProgram.NumElements();
		bResult = Emit(pcParser, RO_Split, 0, 0);
		bResult = bResult && (!bNullable || Emit(pcParser, RO_Mark, (int16)uiMark, 0));
		bResult = bResult && EmitBlock(pcParser, &asBlock);
		bResult = bResult && (!bNullable || Emit(pcParser, RO_Progress, (int16)uiMark, 2));
		bResult = bResult && Emit(pcParser, RO_Jump, (int16)((int32)uiLoop - (int32)masProgram.NumElements()), 0);
		if (bResult)
		{
			iOut = (int16)(masProgram.NumElements() - uiLoop);
			psInstruction = masProgram.Get(uiLoop);
			psInstruction->iX = bLazy ? iOut : 1;
			psInstruction->iY = bLazy ? 1 : iOut;
		}
	}
	else if (bResult && (uiMaximum == REGEX_UNBOUNDED))
	{
		//One or more:  L: [mark]  body  [progress, or out]  split L, out;  out:
		uiLoop = masProgram.NumElements();
		bResult = !bNullable || Emit(pcParser, RO_Mark, (int16)uiMark, 0);
		bResult = bResult && EmitBlock(pcParser, &asBlock);
		bResult = bResult && (!bNullable || Emit(pcParser, RO_Progress, (int16)uiMark, 2));
		if (bResult)
		{
			iOut = (int16)((int32)uiLoop - (int32)masProgram.NumElements());
			bResult = Emit(pcParser, RO_Split, bLazy ? 1 : iOut, bLazy ? iOut : 1);
		}
	}
	else if (bResult)
	{
		//Up to so many more:  split in, out;  body;  split in, out;  body;  out:
		auiSplits.Init();
		for (i = uiMinimum; bResult && (i < uiMaximum); i++)
		{
			uiSplit = (uint16)masProgram.NumElements();
			auiSplits.Add(&uiSplit);
			bResult = Emit(pcParser, RO_Split, 0, 0) && EmitBlock(pcParser, &asBlock);
		}
		for (i = 0; bResult && (i < auiSplits.NumElements()); i++)
		{
			uiSplit = *auiSplits.Get(i);
			iOut = (int16)(masProgram.NumElements() - uiSplit);
			psInstruction = masProgram.Get(uiSplit);
			psInstruction->iX = bLazy ? iOut : 1;
			psInstruction->iY = bLazy ? 1 : iOut;
		}
		auiSplits.Kill();
	}
	asBlock.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// One thing: a character, a class, a group, an escape, ^ or $.
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::ParseAtom(CTextParser* pcParser, bool* pbNullable)
{
	char	c;

	*pbNullable = false;
	if (pcParser->GetCharacter(&c) != TRITRUE)
	{
		return Error(pcParser, "The pattern ends where something should be.");
	}
	switch (c)
	{
	case '(':
		return ParseGroup(pcParser, pbNullable);
	case '[':
		return ParseClass(pcParser);
	case '\\':
		return ParseEscape(pcParser, pbNullable);
	case '.':
		return Emit(pcParser, RO_Any, 0, 0);
	case '^':
		*pbNullable = true;
		return Emit(pcParser, RO_LineStart, 0, 0);
	case '$':
		*pbNullable = true;
		return Emit(pcParser, RO_LineEnd, 0, 0);
	case '*':
	case '+':
	case '?':
		pcParser->StepLeft();
		return Error(pcParser, "There is nothing before this to repeat.");
	default:
		return EmitCharacter(pcParser, c);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// After a (: a group that is remembered, or (?: one that isn't, up to its
// ).
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::ParseGroup(CTextParser* pcParser, bool* pbNullable)
{
	uint16	uiGroup;

	uiGroup = 0;
	if (PeekRegex(pcParser, 0) == '?')
	{
		if (PeekRegex(pcParser, 1) != ':')
		{
			return Error(pcParser, "Of the groups that start (? only (?: is taken.");
		}
		pcParser->StepRight();
		pcParser->StepRight();
	}
	else
	{
		if (muiNumGroups >= REGEX_MAX_GROUPS - 1)
		{
			return Error(pcParser, "A pattern can remember nine groups: make the others (?: groups.");
		}
		muiNumGroups++;
		uiGroup = muiNumGroups;
		if (!Emit(pcParser, RO_Save, (int16)(uiGroup * 2), 0))
		{
			return false;
		}
	}

	muiDepth++;
	if (!ParseAlternation(pcParser, pbNullable))
	{
		return false;
	}
	muiDepth--;
	if (PeekRegex(pcParser, 0) != ')')
	{
		return Error(pcParser, "A ( has no ) after it.");
	}
	pcParser->StepRight();
	if (uiGroup != 0)
	{
		return Emit(pcParser, RO_Save, (int16)(uiGroup * 2 + 1), 0);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Two hexadecimal digits, after \x.
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::ParseHexadecimal(CTextParser* pcParser, char* pc)
{
	uint16	uiHigh;
	uint16	uiLow;

	if (pcParser->mbOutsideText || (pcParser->GetDigit(&uiHigh, 16) != TRITRUE) || pcParser->mbOutsideText || (pcParser->GetDigit(&uiLow, 16) != TRITRUE))
	{
		return Error(pcParser, "\\x is followed by two hexadecimal digits.");
	}
	*pc = (char)(uiHigh * 16 + uiLow);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// After a \ outside a class: a set (\d), a place (\b), what a group
// matched (\1), a character by its letter (\t) or code (\x41), or the
// character itself (\.).
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::ParseEscape(CTextParser* pcParser, bool* pbNullable)
{
	SRegexClass	sClass;
	char		c;

	if (pcParser->GetCharacter(&c) != TRITRUE)
	{
		return Error(pcParser, "The pattern ends with a \\: \\\\ is the character.");
	}
	switch (c)
	{
	case 'd':
	case 'w':
	case 's':
		memset(&sClass, 0, sizeof(SRegexClass));
		AddRegexClassSet(&sClass, c, false);
		return EmitClass(pcParser, &sClass);
	case 'D':
	case 'W':
	case 'S':
		memset(&sClass, 0, sizeof(SRegexClass));
		AddRegexClassSet(&sClass, RegexLower(c), true);
		return EmitClass(pcParser, &sClass);
	case 'b':
		*pbNullable = true;
		return Emit(pcParser, RO_WordBoundary, 0, 0);
	case 'B':
		*pbNullable = true;
		return Emit(pcParser, RO_NotWordBoundary, 0, 0);
	case 't':
		return EmitCharacter(pcParser, '\t');
	case 'n':
		return EmitCharacter(pcParser, '\n');
	case 'r':
		return EmitCharacter(pcParser, '\r');
	case 'f':
		return EmitCharacter(pcParser, '\f');
	case 'v':
		return EmitCharacter(pcParser, '\v');
	case '0':
		return EmitCharacter(pcParser, '\0');
	case 'x':
		return ParseHexadecimal(pcParser, &c) && EmitCharacter(pcParser, c);
	default:
		break;
	}

	if ((c >= '1') && (c <= '9'))
	{
		if ((uint16)(c - '0') > muiNumGroups)
		{
			pcParser->StepLeft();
			return Error(pcParser, "There is no group of that number before this.");
		}
		*pbNullable = true;
		return Emit(pcParser, RO_BackReference, (int16)(c - '0'), 0);
	}
	if (IsRegexWord(c) && (c != '_'))
	{
		pcParser->StepLeft();
		return Error(pcParser, "A \\ before this letter means nothing.");
	}
	return EmitCharacter(pcParser, c);
}


//////////////////////////////////////////////////////////////////////////
//
// One character of a class, or a set in one (\d): *pbSet says which, and
// a set is added to psClass.  The character has been read; a \ is
// followed by what it escapes.
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::ParseClassCharacter(CTextParser* pcParser, SRegexClass* psClass, char* pc, bool* pbSet)
{
	char	c;

	*pbSet = false;
	if (*pc != '\\')
	{
		return true;
	}
	if (pcParser->GetCharacter(&c) != TRITRUE)
	{
		return Error(pcParser, "A [ has no ] after it.");
	}
	switch (c)
	{
	case 'd':
	case 'w':
	case 's':
		AddRegexClassSet(psClass, c, false);
		*pbSet = true;
		return true;
	case 'D':
	case 'W':
	case 'S':
		AddRegexClassSet(psClass, RegexLower(c), true);
		*pbSet = true;
		return true;
	case 't':
		*pc = '\t';
		return true;
	case 'n':
		*pc = '\n';
		return true;
	case 'r':
		*pc = '\r';
		return true;
	case 'f':
		*pc = '\f';
		return true;
	case 'v':
		*pc = '\v';
		return true;
	case 'b':
		*pc = '\b';
		return true;
	case '0':
		*pc = '\0';
		return true;
	case 'x':
		return ParseHexadecimal(pcParser, pc);
	default:
		break;
	}
	if (IsRegexWord(c) && (c != '_'))
	{
		pcParser->StepLeft();
		return Error(pcParser, "A \\ before this letter means nothing.");
	}
	*pc = c;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// After a [: the characters, ranges and sets up to its ], all of them or,
// after a ^, none of them.
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::ParseClass(CTextParser* pcParser)
{
	SRegexClass	sClass;
	uint16		ui;
	char		c;
	char		cLast;
	bool		bNot;
	bool		bSet;

	memset(&sClass, 0, sizeof(SRegexClass));
	bNot = false;
	if (PeekRegex(pcParser, 0) == '^')
	{
		bNot = true;
		pcParser->StepRight();
	}
	for (;;)
	{
		if (pcParser->GetCharacter(&c) != TRITRUE)
		{
			return Error(pcParser, "A [ has no ] after it.");
		}
		if (c == ']')
		{
			break;
		}
		if (!ParseClassCharacter(pcParser, &sClass, &c, &bSet))
		{
			return false;
		}
		if (bSet)
		{
			continue;
		}

		//A - between two characters is the range of them; first or last in the class it is a -.
		cLast = c;
		if ((PeekRegex(pcParser, 0) == '-') && (PeekRegex(pcParser, 1) != ']') && (PeekRegex(pcParser, 1) != '\0'))
		{
			pcParser->StepRight();
			pcParser->GetCharacter(&cLast);
			if (!ParseClassCharacter(pcParser, &sClass, &cLast, &bSet))
			{
				return false;
			}
			if (bSet)
			{
				return Error(pcParser, "A range can't end with a set of characters.");
			}
			if ((uint8)cLast < (uint8)c)
			{
				return Error(pcParser, "A range's characters are the wrong way round.");
			}
		}
		for (ui = (uint8)c; ui <= (uint8)cLast; ui++)
		{
			SetRegexClassBit(&sClass, (uint8)ui);
		}
	}

	//Case is folded before the class is turned over, so [^a] leaves out A too.
	if (!mbCaseSensitive)
	{
		for (c = 'a'; c <= 'z'; c++)
		{
			if (IsRegexClassBit(&sClass, (uint8)c) || IsRegexClassBit(&sClass, (uint8)(c - ('a' - 'A'))))
			{
				SetRegexClassBit(&sClass, (uint8)c);
				SetRegexClassBit(&sClass, (uint8)(c - ('a' - 'A')));
			}
		}
	}
	if (bNot)
	{
		for (ui = 0; ui < 32; ui++)
		{
			sClass.auiBits[ui] = (uint8)~sClass.auiBits[ui];
		}
	}
	if (masClasses.NumElements() >= MAX_INT16)
	{
		return Error(pcParser, "The pattern has too many character classes.");
	}
	memcpy(masClasses.Add(), &sClass, sizeof(SRegexClass));
	return Emit(pcParser, RO_Class, (int16)(masClasses.NumElements() - 1), 0);
}


//////////////////////////////////////////////////////////////////////////
//
// A way not yet tried, or a value to put back, on the stack.  False if
// the stack is as deep as it is let get (REGEX_MAX_STACK, or fewer where
// a size can't count as many): the match is given up.
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::PushFrame(uint16 uiType, uint16 uiIndex, size uiValue)
{
	SRegexFrame*	psFrame;

	if (masStack.NumElements() >= RegexMostElements(sizeof(SRegexFrame), REGEX_MAX_STACK))
	{
		return false;
	}
	psFrame = masStack.Add();
	psFrame->uiType = uiType;
	psFrame->uiIndex = uiIndex;
	psFrame->uiValue = uiValue;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The pattern matched from uiStart of the text: true, with where the
// match ends, if it can.  Where it can go on two ways the second is put on
// the stack, and so is what a capture or a mark was before it is changed;
// when a way fails the stack is taken back down to the last second way,
// undoing those on the way, and that is tried.  After too many steps, or
// with more ways kept than the stack is let hold, it gives up: false, and
// GaveUp says so.
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::Run(const char* szText, size uiLength, size uiStart, size* puiEnd)
{
	SRegexInstruction*	psInstruction;
	SRegexFrame			sFrame;
	size				uiPosition;
	size				uiGroupStart;
	size				uiGroupLength;
	size				i;
	uint16				uiPC;
	char				c;
	bool				bOK;
	bool				bBefore;
	bool				bAfter;
	bool				bDeep;

	for (i = 0; i < REGEX_MAX_GROUPS * 2; i++)
	{
		mauiSaves[i] = REGEX_NO_POSITION;
	}
	masStack.ReInit();
	muiSteps = 0;
	bDeep = false;
	PushFrame(RF_Thread, 0, uiStart);

	while (masStack.Pop(&sFrame))
	{
		if (sFrame.uiType == RF_Capture)
		{
			mauiSaves[sFrame.uiIndex] = sFrame.uiValue;
			continue;
		}
		if (sFrame.uiType == RF_Mark)
		{
			*mauiMarks.Get(sFrame.uiIndex) = sFrame.uiValue;
			continue;
		}

		uiPC = sFrame.uiIndex;
		uiPosition = sFrame.uiValue;
		for (;;)
		{
			muiSteps++;
			if ((muiSteps > muiStepLimit) || bDeep)
			{
				mbGaveUp = true;
				masStack.ReInit();
				return false;
			}

			psInstruction = masProgram.Get(uiPC);
			bOK = true;
			switch (psInstruction->uiOp)
			{
			case RO_Char:
				bOK = (uiPosition < uiLength) && ((uint8)szText[uiPosition] == psInstruction->uiChar);
				uiPosition++;
				uiPC++;
				break;
			case RO_CharNoCase:
				bOK = (uiPosition < uiLength) && ((uint8)RegexLower(szText[uiPosition]) == psInstruction->uiChar);
				uiPosition++;
				uiPC++;
				break;
			case RO_Any:
				bOK = (uiPosition < uiLength) && (szText[uiPosition] != '\n') && (szText[uiPosition] != '\r');
				uiPosition++;
				uiPC++;
				break;
			case RO_Class:
				bOK = (uiPosition < uiLength) && IsRegexClassBit(masClasses.Get((size)psInstruction->iX), (uint8)szText[uiPosition]);
				uiPosition++;
				uiPC++;
				break;
			case RO_Split:
				bDeep = !PushFrame(RF_Thread, (uint16)(uiPC + psInstruction->iY), uiPosition);
				uiPC = (uint16)(uiPC + psInstruction->iX);
				break;
			case RO_Jump:
				uiPC = (uint16)(uiPC + psInstruction->iX);
				break;
			case RO_Save:
				bDeep = !PushFrame(RF_Capture, (uint16)psInstruction->iX, mauiSaves[psInstruction->iX]);
				mauiSaves[psInstruction->iX] = uiPosition;
				uiPC++;
				break;
			case RO_Mark:
				bDeep = !PushFrame(RF_Mark, (uint16)psInstruction->iX, *mauiMarks.Get((size)psInstruction->iX));
				*mauiMarks.Get((size)psInstruction->iX) = uiPosition;
				uiPC++;
				break;
			case RO_Progress:
				if (*mauiMarks.Get((size)psInstruction->iX) == uiPosition)
				{
					uiPC = (uint16)(uiPC + psInstruction->iY);
				}
				else
				{
					uiPC++;
				}
				break;
			case RO_LineStart:
				bOK = (uiPosition == 0) || (szText[uiPosition - 1] == '\n');
				uiPC++;
				break;
			case RO_LineEnd:
				bOK = (uiPosition == uiLength) || (szText[uiPosition] == '\n') || ((szText[uiPosition] == '\r') && (uiPosition + 1 < uiLength) && (szText[uiPosition + 1] == '\n'));
				uiPC++;
				break;
			case RO_WordBoundary:
			case RO_NotWordBoundary:
				bBefore = (uiPosition > 0) && IsRegexWord(szText[uiPosition - 1]);
				bAfter = (uiPosition < uiLength) && IsRegexWord(szText[uiPosition]);
				bOK = (bBefore != bAfter) == (psInstruction->uiOp == RO_WordBoundary);
				uiPC++;
				break;
			case RO_BackReference:
				//A group that took no part matches nothing here, as Java's doesn't.
				uiGroupStart = mauiSaves[psInstruction->iX * 2];
				bOK = (uiGroupStart != REGEX_NO_POSITION) && (mauiSaves[psInstruction->iX * 2 + 1] != REGEX_NO_POSITION);
				if (bOK)
				{
					uiGroupLength = mauiSaves[psInstruction->iX * 2 + 1] - uiGroupStart;
					bOK = uiGroupLength <= uiLength - uiPosition;
					for (i = 0; bOK && (i < uiGroupLength); i++)
					{
						c = szText[uiGroupStart + i];
						bOK = mbCaseSensitive ? (szText[uiPosition + i] == c) : (RegexLower(szText[uiPosition + i]) == RegexLower(c));
					}
					uiPosition += uiGroupLength;
				}
				uiPC++;
				break;
			default:
				//RO_Match.
				*puiEnd = uiPosition;
				masStack.ReInit();
				return true;
			}
			if (!bOK)
			{
				break;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRegex::SetStepLimit(size uiLength)
{
	muiSteps = 0;
	muiStepLimit = REGEX_STEPS;
#if defined(SIZE_16)
	//A 16 bit length's steps always fit (200000 + 65535 * 16).
	muiStepLimit += (uint32)uiLength * REGEX_STEPS_PER_CHAR;
#else
	if (uiLength < (MAX_UINT32 - REGEX_STEPS) / REGEX_STEPS_PER_CHAR)
	{
		muiStepLimit += (uint32)uiLength * REGEX_STEPS_PER_CHAR;
	}
	else
	{
		muiStepLimit = MAX_UINT32;
	}
#endif
	mbGaveUp = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRegex::FillMatch(size uiStart, size uiEnd, SRegexMatch* psMatch)
{
	uint16	i;

	psMatch->uiStart = uiStart;
	psMatch->uiEnd = uiEnd;
	psMatch->auiGroupStarts[0] = uiStart;
	psMatch->auiGroupEnds[0] = uiEnd;
	for (i = 1; i < REGEX_MAX_GROUPS; i++)
	{
		if ((mauiSaves[i * 2] != REGEX_NO_POSITION) && (mauiSaves[i * 2 + 1] != REGEX_NO_POSITION))
		{
			psMatch->auiGroupStarts[i] = mauiSaves[i * 2];
			psMatch->auiGroupEnds[i] = mauiSaves[i * 2 + 1];
		}
		else
		{
			psMatch->auiGroupStarts[i] = REGEX_NO_POSITION;
			psMatch->auiGroupEnds[i] = REGEX_NO_POSITION;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the pattern matches the text from exactly uiPosition, and where
// (psMatch may be NULL).  The text is uiLength characters; it needn't end
// with a 0.
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::MatchAt(const char* szText, size uiLength, size uiPosition, SRegexMatch* psMatch)
{
	size	uiEnd;

	if (!mbCompiled || (uiPosition > uiLength))
	{
		return false;
	}
	SetStepLimit(uiLength - uiPosition);
	if (!Run(szText, uiLength, uiPosition, &uiEnd))
	{
		return false;
	}
	if (psMatch)
	{
		FillMatch(uiPosition, uiEnd, psMatch);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The first match that starts at or after uiFrom.  A match can be of
// nothing (x* where there is no x): to find the next after one of those,
// look from one on from its end.
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::Find(const char* szText, size uiLength, size uiFrom, SRegexMatch* psMatch)
{
	const char*	szFound;
	size		uiPosition;
	size		uiEnd;
	bool		bFirst;
	char		cFirst;

	if (!mbCompiled || (uiFrom > uiLength))
	{
		return false;
	}
	SetStepLimit(uiLength - uiFrom);

	//A pattern that starts with a character can only match where that character is.
	bFirst = masProgram.Get(0)->uiOp == RO_Char;
	cFirst = (char)masProgram.Get(0)->uiChar;
	for (uiPosition = uiFrom; uiPosition <= uiLength; uiPosition++)
	{
		if (bFirst)
		{
			szFound = (uiPosition < uiLength) ? (const char*)memchr(&szText[uiPosition], cFirst, uiLength - uiPosition) : NULL;
			if (szFound == NULL)
			{
				return false;
			}
			uiPosition = (size)(szFound - szText);
		}
		if (Run(szText, uiLength, uiPosition, &uiEnd))
		{
			if (psMatch)
			{
				FillMatch(uiPosition, uiEnd, psMatch);
			}
			return true;
		}
		if (mbGaveUp)
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a replacement only names groups the pattern has ($0 to its
// last).  False, with GetError saying so, if it doesn't.
//
//////////////////////////////////////////////////////////////////////////
bool CRegex::CheckReplacement(const char* szReplacement)
{
	const char*	sz;

	for (sz = szReplacement; *sz != '\0'; sz++)
	{
		if ((*sz == '\\') && (sz[1] != '\0'))
		{
			sz++;
		}
		else if ((*sz == '$') && (sz[1] >= '0') && (sz[1] <= '9'))
		{
			if ((uint16)(sz[1] - '0') > muiNumGroups)
			{
				mszError.Clear();
				mszError.Append("The replacement names group $");
				mszError.Append(sz[1]);
				mszError.Append(", which the pattern doesn't have.");
				muiErrorPosition = (size)(sz - szReplacement);
				return false;
			}
			sz++;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// What a match is replaced with: the replacement, with $0 to $9 as what
// those groups matched in the text (nothing for one that took no part or
// isn't there), \n a line feed, \t a tab, and any other character after a
// \ itself (\$ and \\).
//
//////////////////////////////////////////////////////////////////////////
void CRegex::AppendReplacement(const char* szText, SRegexMatch* psMatch, const char* szReplacement, CChars* pszDest)
{
	const char*	sz;
	uint16		uiGroup;

	for (sz = szReplacement; *sz != '\0'; sz++)
	{
		if ((*sz == '\\') && (sz[1] != '\0'))
		{
			sz++;
			if (*sz == 'n')
			{
				pszDest->Append('\n');
			}
			else if (*sz == 't')
			{
				pszDest->Append('\t');
			}
			else
			{
				pszDest->Append(*sz);
			}
		}
		else if ((*sz == '$') && (sz[1] >= '0') && (sz[1] <= '9'))
		{
			sz++;
			uiGroup = (uint16)(*sz - '0');
			if (psMatch->auiGroupStarts[uiGroup] != REGEX_NO_POSITION)
			{
				pszDest->Append(&szText[psMatch->auiGroupStarts[uiGroup]], psMatch->auiGroupEnds[uiGroup] - psMatch->auiGroupStarts[uiGroup]);
			}
		}
		else
		{
			pszDest->Append(*sz);
		}
	}
}


bool CRegex::IsCompiled(void) const { return mbCompiled; }
const char* CRegex::GetError(void) { return mszError.Text(); }
size CRegex::GetErrorPosition(void) const { return muiErrorPosition; }
uint16 CRegex::NumGroups(void) const { return muiNumGroups; }
size CRegex::NumInstructions(void) const { return masProgram.NumElements(); }
bool CRegex::GaveUp(void) const { return mbGaveUp; }

