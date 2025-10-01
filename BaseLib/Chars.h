/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "DataTypes.h"
#include "CharHelper.h"
#include "ArrayTemplate.h"
#include "ArrayTemplateMinimal.h"
#include "ArrayInt.h"


class CArrayChars;


class CChars
{
CONSTRUCTABLE(CChars);
friend class CFileReader;
friend class CFileWriter;
protected:
	CArrayChar	mcText;

public:
	void	_Init(void);
	CChars*	Init(void);
	CChars* Init(const char* szString);
	CChars* Init(CChars szString);
	CChars* Init(CChars* psz);
	CChars* Init(const char* szString, size iStartInclusive);
	CChars*	Init(CChars szString, size iStartInclusive, size iEndExclusive);
	CChars* Init(const char* szString, size iStartInclusive, size iEndExclusive);
	CChars* Init(char cPadCharacter, size iNumber);
	CChars* Init(CExternalString* pcString);
	CChars* InitList(const char* szFirst, ...);
	CChars* InitList(CChars* szFirst, ...);
	CChars* InitLength(size iLength);
	CChars* InitLength(CMallocator* pcMalloc, size iLength);
	bool	InitData2(const char* szData, size iDataLength);
	void	Kill(void);
	void	DumpKill(void);
	void	Fake(const char* szString);
	void	Fake(const char* szString, size iStartInclusive, size iEndExclusive);
	CChars* ReInit(void);

	size	Length(void);
	void	Set(const char* szString);
	void	Set(CChars szString);
	void	Set(CChars* psz);
	CChars* Append(const char* szString);
	CChars* Append(const char* szString, size iStrlen);
	CChars* Append(CChars szString);
	CChars* Append(CChars* psz);
	CChars* Append(char cPadCharacter);
	CChars* Append(char cPadCharacter, size iNumber);
	CChars* Append(int32 i);
	CChars* Append(int32 i, uint16 iBase);
	CChars* Append(uint32 ui);
	CChars* Append(float f, size iMaxDecimals = -1, bool bAppendF = false);
	CChars* Append(double d, size iMaxDecimals = -1, bool bAppendD = false);
	CChars* Append(int64 lli);
	CChars* Append(uint64 ulli);
	CChars*	Append(CExternalString* pcString);
	CChars*	AppendList(const char* szFirst, ...);
	CChars*	AppendQuoted(char c);
	CChars*	AppendQuoted(const char* szString);
	CChars* AppendNewLine(void);
	CChars* AppendNewLine(CChars szString);
	CChars*	AppendHexHiLo(void* pv, size iNumBytes);
	CChars*	AppendHexLoHi(void* pv, size iNumBytes);
	CChars*	AppendSubString(const char* szString, size iLength);
	CChars*	AppendSubString(CChars szString, size iStartInclusive, size iEndExclusive);
	CChars*	AppendSubString(const char* szString, size iStartInclusive, size iEndExclusive);
	CChars*	AppendSubString(const char* szStartInclusive, const char* szEndExclusive);
	CChars*	AppendBool(bool bValue);
	CChars*	AppendBool(bool bValue, const char* szTrue, const char* szFalse);
	CChars* AppendData(const char* szData, size iMaxLength);
	CChars* AppendData(const char* szData, size iDataLength, size iMaxLength);
	bool	AppendData2(const char* szData, size iDataLength);
	CChars*	AppendPointer(void* pv);
	bool	AppendFlag(uint32 msFlags, uint32 uiFlag, const char* szFlagName, bool bAppendComma = false);
	void	Insert(size iPos, char c);
	void	Insert(size iPos, char c, size uiCount);
	void	Insert(size iPos, const char* szString);
	void	Insert(size iPos, CChars* pszString);
	void	Minimize(void);
	char*	Text(void);
	char*	Text(size iIndex);
	bool	Empty(void);
	void	LeftAlign(CChars szString, char cPadCharacter, size iWidth);
	void	LeftAlign(const char* szString, char cPadCharacter, size iWidth);
	void	RightAlign(CChars szString, char cPadCharacter, size iWidth);
	void	RightAlign(const char* szString, char cPadCharacter, size iWidth);
	void	RightAlign(size uiOffset, char cPadCharacter, size uiWidth);
	void	RightAlign(char cPadCharacter, size iWidth);
	void	RemoveLastCharacter(void);
	void	RemoveFromStart(size iNumChars);
	void	RemoveFromEnd(size iNumChars);
	void	Remove(size iStartInclusive, size iEndExclusive);
	void	RemoveEnd(size iIndex);
	void	RemoveCharacter(size iPos);
	void	Split(CArrayChars* aszDest, char cSplitter);
	void	SplitLines(CArrayChars* aszDest);
	bool	Equals(const char* szString);
	bool	Equals(CChars szString);
	bool	Equals(const char* szString, size iLen);
	bool	Equals(CChars* pszString);
	bool	EqualsIgnoreCase(const char* szString);
	bool	EqualsIgnoreCase(CChars szString);
	bool	Contains(const char* szString);
	bool	ContainsIgnoreCase(const char* szString);
	bool	EndsWith(const char* szString);
	bool	EndsWithIgnoreCase(const char* szString);
	bool	EndsWith(char c);
	bool	StartsWith(const char* szString);
	bool	StartsWithIgnoreCase(const char* szString);
	size	Occurrences(const char* szString);
	bool	SubStringEquals(size iStart, const char* szString);
	bool	SubStringEqualsIgnoreCase(size iStart, const char* szString);
	size	FindFromEnd(const char* szString);
	size	FindFromEnd(size iPos, const char* szString);
	size	FindFromEnd(char c);
	size	FindFromEnd(size iIndex, char c);
	size	Find(const char* szString);
	size	Find(size iPos, const char* szString);
	size	Find(size iPos, char c);
	size	Find(char c);
	size	FindDigit(size iStartIndex);
	char	GetChar(size iIndex);
	void	SetChar(size iPos, char c);
	void	Difference(CArrayInt* paiNewToOldIndices, CArrayInt* paiOldToNewIndices, CChars szOldString);
	bool	IsWhitespace(size iPos, bool bIncludeNewLines);
	bool	IsWhitespace(void);
	void	SetLength(size iLength);
	size	StripWhitespace(bool bIncludeNewLines = false);
	int		Compare(CChars* szOther);
	int		Compare(const char* szOther);
	int		CompareIgnoreCase(CChars* szOther);
	int		CompareIgnoreCase(const char* szOther);
	size	Replace(char cFind, char cReplace);
	size	Replace(const char* szFind, const char* szReplace);
	void	Overwrite(size iPos, const char* szReplace);
	void	Reverse(void);
	void	Clear(void);
	size	CountNewLines(void);
	size	Count(char c);
	void	LowerCase(void);
	void	UpperCase(void);
	void	ReplaceNonTabsWithSpace(void);

	bool	WriteString(CFileWriter* pcWriter);
	bool	ReadString(CFileReader* pcReader);
	bool	ReadChars(CFileReader* pcReader);  //Call this if the string was saved from a char*

	char*	CopyIntoBuffer(char* szDest, size iDestLength);

	//Suspicious... should probably be in a helper class or something.
	void	PassifyNewlines(void);
	void	MakeCPlusPlus(void);
	char*	FindLineContaining(char* szPosition, size* piLineNumber);

	void	Dump(void);
	static void Dump(const char* szString);

protected:
	char*	PrivateGrow(size iNumberOfCharacters);
	size	ReplaceWithLonger(const char* szFind, const char* szReplace, size iFindLen, size iDifference);
	size	ReplaceWithShorter(const char* szFind, const char* szReplace, size iReplaceLen, size iFindLen, size iDifference);
	size	ReplaceWithEqualLength(const char* szFind, const char* szReplace, size iFindLen);
	size	FindEndOfLeadingWhitespace(bool bIncludeNewLines);
	size	FindStartOfTrailingWhitespace(bool bIncludeNewLines);
	void	SetNonNull(const char* szString, size iLen);
	void	SetEmpty(void);
	void	CleanIfEmpty(void);
	void	InitEmpty(void);
	bool	IsFakeEmpty(void);
	void	Unfake(void);
	void	UnfakeIfFakeEmpty(void);
};


int CompareChars(const void* arg1, const void* arg2);
int CompareCharsIgnoreCase(const void* arg1, const void* arg2);


#endif // __CHARS_H__

