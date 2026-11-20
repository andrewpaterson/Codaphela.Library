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
#ifndef __REGEX_H__
#define __REGEX_H__
#include "Chars.h"
#include "ArrayTemplate.h"


class CTextParser;


#define REGEX_MAX_GROUPS	10			//All of the match, group 0, and the groups \1 to \9 and $1 to $9 name.
#define REGEX_MAX_PROGRAM	16000		//The most instructions a pattern compiles to.
#define REGEX_MAX_REPEAT	1000		//The most a {n,m} counts to.
#define REGEX_UNBOUNDED		MAX_UINT16	//A {n,}'s, a *'s and a +'s most.
#define REGEX_NO_POSITION	MAX_SIZE	//Where a group that took no part in a match starts and ends.


//What a pattern compiles to: instructions a matcher steps through, each jump relative to its own instruction so that
//a run of them can be moved or copied as it is.
enum ERegexOp
{
	RO_Char,				//The character uiChar.
	RO_CharNoCase,			//uiChar, a lower case letter, in either case.
	RO_Any,					//Any character but a line's end.
	RO_Class,				//A character of class iX.
	RO_Split,				//Go on at iX; if that fails, at iY.
	RO_Jump,				//Go on at iX.
	RO_Save,				//Here is where capture iX is: 2n is where group n starts, 2n + 1 where it ends.
	RO_Mark,				//A loop's turn starts here: remembered as mark iX.
	RO_Progress,			//If the turn that started at mark iX matched nothing, leave the loop: go on at iY.
	RO_LineStart,			//^
	RO_LineEnd,				//$
	RO_WordBoundary,		//\b
	RO_NotWordBoundary,		//\B
	RO_BackReference,		//What group iX matched, again.
	RO_Match,				//The pattern has matched.
};


struct SRegexInstruction
{
	uint8	uiOp;	//ERegexOp.
	uint8	uiChar;
	int16	iX;
	int16	iY;
};


//A set of characters, a bit for each.
struct SRegexClass
{
	uint8	auiBits[32];
};


//What the matcher goes back to when a way through the pattern fails: another way to try (RF_Thread: instruction
//uiIndex at the text's uiValue), on the way undoing what the failed one remembered (capture or mark uiIndex was
//uiValue).
enum ERegexFrame
{
	RF_Thread,
	RF_Capture,
	RF_Mark,
};


struct SRegexFrame
{
	uint16	uiType;
	uint16	uiIndex;
	size	uiValue;
};


//Where a pattern matched: all of it, and each group (group 0 is all of it).  A group that took no part starts and
//ends at REGEX_NO_POSITION.
struct SRegexMatch
{
	size	uiStart;
	size	uiEnd;  //After its last character.
	size	auiGroupStarts[REGEX_MAX_GROUPS];
	size	auiGroupEnds[REGEX_MAX_GROUPS];
};


typedef CArrayTemplate<SRegexInstruction>	CArrayRegexInstructions;
typedef CArrayTemplate<SRegexClass>			CArrayRegexClasses;
typedef CArrayTemplate<SRegexFrame>			CArrayRegexFrames;
typedef CArrayTemplate<size>				CArrayRegexMarks;


//A regular expression, as IntelliJ IDEA's find takes them (Java's, which is ECMAScript's for what is here), compiled
//once (Compile) and then matched against any text (MatchAt, Find).  The pattern is parsed with a CTextParser.
//
//  x          the character x; \ before a character that means something here (\. \* \( \[ \\ ...) is that character
//  .          any character but a line's end
//  [abc] [a-z0-9_] [^abc]    one of a set of characters, a range of them, or one that isn't
//  \d \w \s    a digit, a word character (a letter, digit or underscore), white space; \D \W \S one that isn't;
//             all of them in a set too
//  \t \n \r \f \v \0 \xHH    a tab, a line feed, ..., the character with a hexadecimal code
//  ^ $        the start and the end of a line (or of the text)
//  \b \B      at a word's start or end, and not at one
//  (x)        a group, remembered: \1 to \9 match what the group matched again, and a replacement's $1 to $9 are it
//  (?:x)      a group that isn't remembered
//  x|y        x or y
//  x* x+ x?   none or more, one or more, none or one: as many as it can; with ? after (x*? x+? x??) as few
//  x{n} x{n,} x{n,m}    n of them, n or more, n to m; with ? after as few.  A { that doesn't start one of these is
//             a {, so C++ can be searched for without escaping every brace
//
//Matching can ignore the case of letters.  It works a byte at a time, so a character outside ASCII is the bytes it is
//written with.
//
//The matcher backtracks: where the pattern can go on two ways (x*, x|y) it takes the first and remembers the second
//on a stack of its own, not the machine's, so a long text doesn't overflow one (the W65C816's is small).  A loop
//whose turn matches nothing ends rather than turn for ever ((a*)*).  A pattern that backtracks without end on a text
//((a+)+b on a run of a) is given up on after a number of steps in proportion to the text: Find and MatchAt return
//false and GaveUp says so.
//
//A replacement (AppendReplacement) is text with $0 to $9 for the groups; \$ and \\ are those characters, \n a line
//feed and \t a tab.
class CRegex
{
protected:
	CArrayRegexInstructions	masProgram;
	CArrayRegexClasses		masClasses;
	CArrayRegexFrames		masStack;
	CArrayRegexMarks		mauiMarks;
	CChars					mszError;
	size					muiErrorPosition;
	size					mauiSaves[REGEX_MAX_GROUPS * 2];
	uint32					muiSteps;
	uint32					muiStepLimit;
	uint16					muiNumGroups;	//Not counting group 0.
	uint16					muiNumMarks;
	uint16					muiDepth;		//Of groups, while compiling.
	bool					mbCaseSensitive;
	bool					mbCompiled;
	bool					mbGaveUp;

public:
	void		Init(void);
	void		Kill(void);

	bool		Compile(const char* szPattern, bool bCaseSensitive = true);
	bool		IsCompiled(void) const;
	const char*	GetError(void);
	size		GetErrorPosition(void) const;
	uint16		NumGroups(void) const;
	size		NumInstructions(void) const;

	bool		MatchAt(const char* szText, size uiLength, size uiPosition, SRegexMatch* psMatch);
	bool		Find(const char* szText, size uiLength, size uiFrom, SRegexMatch* psMatch);
	bool		GaveUp(void) const;

	bool		CheckReplacement(const char* szReplacement);
	void		AppendReplacement(const char* szText, SRegexMatch* psMatch, const char* szReplacement, CChars* pszDest);

protected:
	bool		ParseAlternation(CTextParser* pcParser, bool* pbNullable);
	bool		ParseSequence(CTextParser* pcParser, bool* pbNullable);
	bool		ParseQuantified(CTextParser* pcParser, bool* pbNullable);
	bool		ParseAtom(CTextParser* pcParser, bool* pbNullable);
	bool		ParseGroup(CTextParser* pcParser, bool* pbNullable);
	bool		ParseClass(CTextParser* pcParser);
	bool		ParseEscape(CTextParser* pcParser, bool* pbNullable);
	bool		ParseClassCharacter(CTextParser* pcParser, SRegexClass* psClass, char* pc, bool* pbSet);
	bool		ParseHexadecimal(CTextParser* pcParser, char* pc);
	int			ParseQuantifier(CTextParser* pcParser, uint16* puiMinimum, uint16* puiMaximum, bool* pbLazy);
	bool		ParseCount(CTextParser* pcParser, uint16* puiCount);
	bool		Repeat(CTextParser* pcParser, size uiAtom, uint16 uiMinimum, uint16 uiMaximum, bool bLazy, bool bNullable);

	bool		Emit(CTextParser* pcParser, uint8 uiOp, int16 iX, int16 iY);
	bool		EmitCharacter(CTextParser* pcParser, char c);
	bool		EmitClass(CTextParser* pcParser, SRegexClass* psClass);
	bool		EmitBlock(CTextParser* pcParser, CArrayRegexInstructions* pasBlock);
	bool		Error(CTextParser* pcParser, const char* szError);

	bool		Run(const char* szText, size uiLength, size uiStart, size* puiEnd);
	void		PushFrame(uint16 uiType, uint16 uiIndex, size uiValue);
	void		FillMatch(size uiStart, size uiEnd, SRegexMatch* psMatch);
	void		SetStepLimit(size uiLength);
};


#endif // __REGEX_H__

