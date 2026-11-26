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
#ifndef __CHARS_H__
#define __CHARS_H__
#include "StringHelper.h"
#include "ExternalString.h"
#include "ArrayChar.h"
#include "CharHelper.h"
#include "ArrayTemplate.h"
#include "ArrayInt.h"
#include "CharsImmutable.h"


class CArrayChars;
//CChars (a string counted in size, its characters a CArrayChar) and CLargeChars (counted in lsize, a CArrayLargeChar)
//are this, written once: STRING is the class deriving from it, whose functions take and return it, and TEXT the array
//it keeps its characters in.
template<class STRING, class TEXT>
class CCharsBase
{
friend class CFileReader;
friend class CFileWriter;
protected:
	TEXT	mcText;

public:
	typedef typename TEXT::Count	Count;

	void		_Init(void);
	STRING*		Init(void);
	STRING*		Init(CMallocator* pcMalloc);
	STRING*		Init(const char* szString);
	STRING*		Init(STRING szString);
	STRING*		Init(STRING* psz);
	STRING*		Init(const char* szString, Count iStartInclusive);
	STRING*		Init(STRING szString, Count iStartInclusive, Count iEndExclusive);
	STRING*		Init(const char* szString, Count iStartInclusive, Count iEndExclusive);
	STRING*		Init(char cPadCharacter, Count iNumber);
	STRING*		Init(CExternalString* pcString);
	STRING*		InitList(const char* szFirst, ...);
	STRING*		InitList(STRING* szFirst, ...);
	STRING*		InitLength(Count iLength);
	STRING*		InitLength(CMallocator* pcMalloc, Count iLength);
	bool		InitData2(const char* szData, Count iDataLength);
	void		Kill(void);
	void		DumpKill(void);
	void		Fake(const char* szString);
	void		Fake(const char* szString, Count iStartInclusive, Count iEndExclusive);
	STRING*		ReInit(void);

	Count		Length(void) const;
	void		Set(const char* szString);
	void		Set(STRING szString);
	void		Set(STRING* psz);
	STRING*		Append(const char* szString);
	STRING*		Append(const char* szString, Count iStrlen);
	STRING*		Append(STRING szString);
	STRING*		Append(STRING* psz);
	STRING*		Append(CCharsImmutable* psz);
	STRING*		Append(char cPadCharacter);
	STRING*		Append(char cPadCharacter, Count iNumber);
	STRING*		Append(int32 i);
	STRING*		Append(int32 i, uint16 iBase);
	STRING*		Append(uint32 ui);
	STRING*		Append(uint32 ui, uint16 iBase);
#ifdef __W65C816__
	//An int is 16 bits on the W65C816: ints match these exactly (the 32 bit ones would all need a conversion, so none
	//would be chosen).
	STRING*		Append(int16 i);
	STRING*		Append(int16 i, uint16 iBase);
	STRING*		Append(uint16 ui);
	STRING*		Append(uint16 ui, uint16 iBase);
#endif // __W65C816__
	STRING*		Append(float f, Count iMaxDecimals = -1, bool bAppendF = false);
	STRING*		Append(double d, Count iMaxDecimals = -1, bool bAppendD = false);
	STRING*		Append(int64 lli);
	STRING*		Append(uint64 ulli);
	STRING*		Append(CExternalString* pcString);
	STRING*		AppendList(const char* szFirst, ...);
	STRING*		AppendQuoted(char c);
	STRING*		AppendQuoted(const char* szString);
	STRING*		AppendNewLine(void);
	STRING*		AppendNewLine(STRING szString);
	STRING*		AppendCarriageReturn(void);
	STRING*		AppendHexHiLo(void* pv, Count iNumBytes);
	STRING*		AppendHexLoHi(void* pv, Count iNumBytes);
	STRING*		AppendSubString(const char* szString, Count iLength);
	STRING*		AppendSubString(STRING szString, Count iStartInclusive, Count iEndExclusive);
	STRING*		AppendSubString(STRING* pszString, Count iStartInclusive, Count iEndExclusive);
	STRING*		AppendSubString(const char* szString, Count iStartInclusive, Count iEndExclusive);
	STRING*		AppendSubString(const char* szStartInclusive, const char* szEndExclusive);
	STRING*		AppendBool(bool bValue);
	STRING*		AppendBool(bool bValue, const char* szTrue, const char* szFalse);
	STRING*		AppendData(const char* szData, Count iMaxLength);
	STRING*		AppendData(const char* szData, Count iDataLength, Count iMaxLength);
	bool		AppendData2(const char* szData, Count iDataLength);
	STRING*		AppendPointer(void* pv);
	bool		AppendFlag(uint32 msFlags, uint32 uiFlag, const char* szFlagName, bool bAppendComma = false);
	void		Insert(Count iPos, char c);
	void		Insert(Count iPos, char c, Count uiCount);
	void		Insert(Count iPos, const char* szString);
	void		Insert(Count iPos, STRING* pszString);
	void		Minimize(void);
	char*		Text(void);
	const char*	Text(void) const;
	char*		Text(Count iIndex);
	const char*	Text(Count iIndex) const;
	bool		Empty(void) const;
	void		LeftAlign(STRING szString, char cPadCharacter, Count iWidth);
	void		LeftAlign(const char* szString, char cPadCharacter, Count iWidth);
	void		RightAlign(STRING szString, char cPadCharacter, Count iWidth);
	void		RightAlign(const char* szString, char cPadCharacter, Count iWidth);
	void		RightAlign(Count uiOffset, char cPadCharacter, Count uiWidth);
	void		RightAlign(char cPadCharacter, Count iWidth);
	void		RemoveLastCharacter(void);
	void		RemoveFromStart(Count iNumChars);
	void		RemoveFromEnd(Count iNumChars);
	void		Remove(Count iStartInclusive, Count iEndExclusive);
	void		RemoveEnd(Count iIndex);
	void		RemoveCharacter(Count iPos);
	void		Split(CArrayChars* aszDest, char cSplitter) const;
	void		SplitLines(CArrayChars* aszDest) const;
	bool		Equals(const char* szString) const;
	bool		Equals(STRING szString) const;
	bool		Equals(const char* szString, Count iLen) const;
	bool		Equals(STRING* pszString) const;
	bool		EqualsIgnoreCase(const char* szString) const;
	bool		EqualsIgnoreCase(STRING szString) const;
	bool		Contains(const char* szString) const;
	bool		ContainsIgnoreCase(const char* szString) const;
	bool		Contains(char c) const;
	bool		EndsWith(const char* szString) const;
	bool		EndsWithIgnoreCase(const char* szString) const;
	bool		EndsWith(char c) const;
	bool		StartsWith(const char* szString) const;
	bool		StartsWithIgnoreCase(const char* szString) const;
	bool		StartsWith(char c) const;
	Count		Occurrences(const char* szString) const;
	Count		Occurrences(char c) const;
	bool		SubStringEquals(Count iStart, const char* szString) const;
	bool		SubStringEqualsIgnoreCase(Count iStart, const char* szString) const;
	Count		FindFromEnd(const char* szString) const;
	Count		FindFromEnd(Count iPos, const char* szString) const;
	Count		FindFromEnd(char c) const;
	Count		FindFromEnd(Count iIndex, char c) const;
	Count		Find(const char* szString) const;
	Count		Find(Count iPos, const char* szString) const;
	Count		Find(Count iPos, char c) const;
	Count		Find(char c) const;
	Count		FindDigit(Count iStartIndex) const;
	char		GetChar(Count iIndex) const;
	void		SetChar(Count iPos, char c);
	void		Difference(CArrayInt* paiNewToOldIndices, CArrayInt* paiOldToNewIndices, STRING szOldString) const;
	bool		IsWhitespace(Count iPos, bool bIncludeNewLines) const;
	bool		IsWhitespace(void) const;
	void		SetLength(Count iLength);
	Count		StripWhitespace(bool bIncludeNewLines = false);
	int			Compare(STRING* szOther) const;
	int			Compare(const char* szOther) const;
	int			CompareIgnoreCase(STRING* szOther) const;
	int			CompareIgnoreCase(const char* szOther) const;
	Count		Replace(char cFind, char cReplace);
	Count		Replace(const char* szFind, const char* szReplace);
	void		Overwrite(Count iPos, const char* szReplace);
	void		Reverse(void);
	void		Clear(void);
	Count		CountNewLines(void) const;
	void		LowerCase(void);
	void		UpperCase(void);
	void		ReplaceNonTabsWithSpace(void);

	bool		WriteString(CFileWriter* pcWriter);
	bool		ReadString(CFileReader* pcReader);
	bool		ReadChars(CFileReader* pcReader);  //Call this if the string was saved from a char*

	char*		CopyIntoBuffer(char* szDest, Count iDestLength) const;

	//Suspicious... should be in CharsHelper or something.
	void		PassifyNewlines(void);
	void		MakeCPlusPlus(void);
	char*		FindLineContaining(const char* szPosition, Count* piLineNumber);

	void		Dump(void) const;
	static void	Dump(const char* szString);

protected:
	char*		PrivateGrow(Count iNumberOfCharacters);
	Count		ReplaceWithLonger(const char* szFind, const char* szReplace, Count iFindLen, Count iDifference);
	Count		ReplaceWithShorter(const char* szFind, const char* szReplace, Count iReplaceLen, Count iFindLen, Count iDifference);
	Count		ReplaceWithEqualLength(const char* szFind, const char* szReplace, Count iFindLen);
	Count		FindEndOfLeadingWhitespace(bool bIncludeNewLines) const;
	Count		FindStartOfTrailingWhitespace(bool bIncludeNewLines) const;
	void		SetNonNull(const char* szString, Count iLen);
	void		SetEmpty(void);
	void		CleanIfEmpty(void);
	void		InitEmpty(void);
	bool		KeepsArray(void) const;
	bool		IsFakeEmpty(void) const;
	Count		CStringLength(const char* szString) const;
	void		Unfake(void);
	void		UnfakeIfFakeEmpty(void);
};


//The names the strings are known by.  A large string also appends a natural one.
class CChars : public CCharsBase<CChars, CArrayChar>
{
NAMED_CLASS(CChars)
};


class CLargeChars : public CCharsBase<CLargeChars, CArrayLargeChar>
{
NAMED_CLASS(CLargeChars)
public:
	using CCharsBase<CLargeChars, CArrayLargeChar>::Append;

	CLargeChars*	Append(CChars szString);
	CLargeChars*	Append(CChars* psz);
};


//Chars.cpp instantiates them; no other unit compiles their functions.
extern template class CCharsBase<CChars, CArrayChar>;
extern template class CCharsBase<CLargeChars, CArrayLargeChar>;


int CompareChars(const void* arg1, const void* arg2);
int CompareCharsIgnoreCase(const void* arg1, const void* arg2);


#endif // __CHARS_H__

