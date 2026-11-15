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
	TEXT					mcText;

public:
	void					_Init(void);
	STRING*					Init(void);
	STRING*					Init(CMallocator* pcMalloc);
	STRING*					Init(const char* szString);
	STRING*					Init(STRING szString);
	STRING*					Init(STRING* psz);
	STRING*					Init(const char* szString, typename TEXT::Count iStartInclusive);
	STRING*					Init(STRING szString, typename TEXT::Count iStartInclusive, typename TEXT::Count iEndExclusive);
	STRING*					Init(const char* szString, typename TEXT::Count iStartInclusive, typename TEXT::Count iEndExclusive);
	STRING*					Init(char cPadCharacter, typename TEXT::Count iNumber);
	STRING*					Init(CExternalString* pcString);
	STRING*					InitList(const char* szFirst, ...);
	STRING*					InitList(STRING* szFirst, ...);
	STRING*					InitLength(typename TEXT::Count iLength);
	STRING*					InitLength(CMallocator* pcMalloc, typename TEXT::Count iLength);
	bool					InitData2(const char* szData, typename TEXT::Count iDataLength);
	void					Kill(void);
	void					DumpKill(void);
	void					Fake(const char* szString);
	void					Fake(const char* szString, typename TEXT::Count iStartInclusive, typename TEXT::Count iEndExclusive);
	STRING*					ReInit(void);

	typename TEXT::Count	Length(void);
	void					Set(const char* szString);
	void					Set(STRING szString);
	void					Set(STRING* psz);
	STRING*					Append(const char* szString);
	STRING*					Append(const char* szString, typename TEXT::Count iStrlen);
	STRING*					Append(STRING szString);
	STRING*					Append(STRING* psz);
	STRING*					Append(CCharsImmutable* psz);
	STRING*					Append(char cPadCharacter);
	STRING*					Append(char cPadCharacter, typename TEXT::Count iNumber);
	STRING*					Append(int32 i);
	STRING*					Append(int32 i, uint16 iBase);
	STRING*					Append(uint32 ui);
	STRING*					Append(uint32 ui, uint16 iBase);
#ifdef __W65C816__
	//An int is 16 bits on the W65C816: ints match these exactly (the 32 bit ones would all need a conversion, so none
	//would be chosen).
	STRING*					Append(int16 i);
	STRING*					Append(int16 i, uint16 iBase);
	STRING*					Append(uint16 ui);
	STRING*					Append(uint16 ui, uint16 iBase);
#endif // __W65C816__
	STRING*					Append(float f, typename TEXT::Count iMaxDecimals = -1, bool bAppendF = false);
	STRING*					Append(double d, typename TEXT::Count iMaxDecimals = -1, bool bAppendD = false);
	STRING*					Append(int64 lli);
	STRING*					Append(uint64 ulli);
	STRING*					Append(CExternalString* pcString);
	STRING*					AppendList(const char* szFirst, ...);
	STRING*					AppendQuoted(char c);
	STRING*					AppendQuoted(const char* szString);
	STRING*					AppendNewLine(void);
	STRING*					AppendNewLine(STRING szString);
	STRING*					AppendCarriageReturn(void);
	STRING*					AppendHexHiLo(void* pv, typename TEXT::Count iNumBytes);
	STRING*					AppendHexLoHi(void* pv, typename TEXT::Count iNumBytes);
	STRING*					AppendSubString(const char* szString, typename TEXT::Count iLength);
	STRING*					AppendSubString(STRING szString, typename TEXT::Count iStartInclusive, typename TEXT::Count iEndExclusive);
	STRING*					AppendSubString(STRING* pszString, typename TEXT::Count iStartInclusive, typename TEXT::Count iEndExclusive);
	STRING*					AppendSubString(const char* szString, typename TEXT::Count iStartInclusive, typename TEXT::Count iEndExclusive);
	STRING*					AppendSubString(const char* szStartInclusive, const char* szEndExclusive);
	STRING*					AppendBool(bool bValue);
	STRING*					AppendBool(bool bValue, const char* szTrue, const char* szFalse);
	STRING*					AppendData(const char* szData, typename TEXT::Count iMaxLength);
	STRING*					AppendData(const char* szData, typename TEXT::Count iDataLength, typename TEXT::Count iMaxLength);
	bool					AppendData2(const char* szData, typename TEXT::Count iDataLength);
	STRING*					AppendPointer(void* pv);
	bool					AppendFlag(uint32 msFlags, uint32 uiFlag, const char* szFlagName, bool bAppendComma = false);
	void					Insert(typename TEXT::Count iPos, char c);
	void					Insert(typename TEXT::Count iPos, char c, typename TEXT::Count uiCount);
	void					Insert(typename TEXT::Count iPos, const char* szString);
	void					Insert(typename TEXT::Count iPos, STRING* pszString);
	void					Minimize(void);
	char*					Text(void);
	char*					Text(typename TEXT::Count iIndex);
	bool					Empty(void);
	void					LeftAlign(STRING szString, char cPadCharacter, typename TEXT::Count iWidth);
	void					LeftAlign(const char* szString, char cPadCharacter, typename TEXT::Count iWidth);
	void					RightAlign(STRING szString, char cPadCharacter, typename TEXT::Count iWidth);
	void					RightAlign(const char* szString, char cPadCharacter, typename TEXT::Count iWidth);
	void					RightAlign(typename TEXT::Count uiOffset, char cPadCharacter, typename TEXT::Count uiWidth);
	void					RightAlign(char cPadCharacter, typename TEXT::Count iWidth);
	void					RemoveLastCharacter(void);
	void					RemoveFromStart(typename TEXT::Count iNumChars);
	void					RemoveFromEnd(typename TEXT::Count iNumChars);
	void					Remove(typename TEXT::Count iStartInclusive, typename TEXT::Count iEndExclusive);
	void					RemoveEnd(typename TEXT::Count iIndex);
	void					RemoveCharacter(typename TEXT::Count iPos);
	void					Split(CArrayChars* aszDest, char cSplitter);
	void					SplitLines(CArrayChars* aszDest);
	bool					Equals(const char* szString);
	bool					Equals(STRING szString);
	bool					Equals(const char* szString, typename TEXT::Count iLen);
	bool					Equals(STRING* pszString);
	bool					EqualsIgnoreCase(const char* szString);
	bool					EqualsIgnoreCase(STRING szString);
	bool					Contains(const char* szString);
	bool					ContainsIgnoreCase(const char* szString);
	bool					Contains(char c);
	bool					EndsWith(const char* szString);
	bool					EndsWithIgnoreCase(const char* szString);
	bool					EndsWith(char c);
	bool					StartsWith(const char* szString);
	bool					StartsWithIgnoreCase(const char* szString);
	bool					StartsWith(char c);
	typename TEXT::Count	Occurrences(const char* szString);
	bool					SubStringEquals(typename TEXT::Count iStart, const char* szString);
	bool					SubStringEqualsIgnoreCase(typename TEXT::Count iStart, const char* szString);
	typename TEXT::Count	FindFromEnd(const char* szString);
	typename TEXT::Count	FindFromEnd(typename TEXT::Count iPos, const char* szString);
	typename TEXT::Count	FindFromEnd(char c);
	typename TEXT::Count	FindFromEnd(typename TEXT::Count iIndex, char c);
	typename TEXT::Count	Find(const char* szString);
	typename TEXT::Count	Find(typename TEXT::Count iPos, const char* szString);
	typename TEXT::Count	Find(typename TEXT::Count iPos, char c);
	typename TEXT::Count	Find(char c);
	typename TEXT::Count	FindDigit(typename TEXT::Count iStartIndex);
	char					GetChar(typename TEXT::Count iIndex);
	void					SetChar(typename TEXT::Count iPos, char c);
	void					Difference(CArrayInt* paiNewToOldIndices, CArrayInt* paiOldToNewIndices, STRING szOldString);
	bool					IsWhitespace(typename TEXT::Count iPos, bool bIncludeNewLines);
	bool					IsWhitespace(void);
	void					SetLength(typename TEXT::Count iLength);
	typename TEXT::Count	StripWhitespace(bool bIncludeNewLines = false);
	int						Compare(STRING* szOther);
	int						Compare(const char* szOther);
	int						CompareIgnoreCase(STRING* szOther);
	int						CompareIgnoreCase(const char* szOther);
	typename TEXT::Count	Replace(char cFind, char cReplace);
	typename TEXT::Count	Replace(const char* szFind, const char* szReplace);
	void					Overwrite(typename TEXT::Count iPos, const char* szReplace);
	void					Reverse(void);
	void					Clear(void);
	typename TEXT::Count	CountNewLines(void);
	typename TEXT::Count	Count(char c);
	void					LowerCase(void);
	void					UpperCase(void);
	void					ReplaceNonTabsWithSpace(void);

	bool					WriteString(CFileWriter* pcWriter);
	bool					ReadString(CFileReader* pcReader);
	bool					ReadChars(CFileReader* pcReader);  //Call this if the string was saved from a char*

	char*					CopyIntoBuffer(char* szDest, typename TEXT::Count iDestLength);

	//Suspicious... should be in CharsHelper or something.
	void					PassifyNewlines(void);
	void					MakeCPlusPlus(void);
	char*					FindLineContaining(const char* szPosition, typename TEXT::Count* piLineNumber);

	void					Dump(void);
	static void Dump(const char* szString);

protected:
	char*					PrivateGrow(typename TEXT::Count iNumberOfCharacters);
	typename TEXT::Count	ReplaceWithLonger(const char* szFind, const char* szReplace, typename TEXT::Count iFindLen, typename TEXT::Count iDifference);
	typename TEXT::Count	ReplaceWithShorter(const char* szFind, const char* szReplace, typename TEXT::Count iReplaceLen, typename TEXT::Count iFindLen, typename TEXT::Count iDifference);
	typename TEXT::Count	ReplaceWithEqualLength(const char* szFind, const char* szReplace, typename TEXT::Count iFindLen);
	typename TEXT::Count	FindEndOfLeadingWhitespace(bool bIncludeNewLines);
	typename TEXT::Count	FindStartOfTrailingWhitespace(bool bIncludeNewLines);
	void					SetNonNull(const char* szString, typename TEXT::Count iLen);
	void					SetEmpty(void);
	void					CleanIfEmpty(void);
	void					InitEmpty(void);
	bool					KeepsArray(void);
	bool					IsFakeEmpty(void);
	typename TEXT::Count	CStringLength(const char* szString);
	void					Unfake(void);
	void					UnfakeIfFakeEmpty(void);
};


//The names the strings are known by.  A large string also appends a natural one.
class CChars : public CCharsBase<CChars, CArrayChar>
{
CONSTRUCTABLE(CChars)
};


class CLargeChars : public CCharsBase<CLargeChars, CArrayLargeChar>
{
CONSTRUCTABLE(CLargeChars)
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

