/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
	CChars* Init(const char* sz);
	CChars* Init(CChars sz);
	CChars* Init(CChars* psz);
	CChars* Init(const char* sz, int iStartInclusive);
	CChars*	Init(CChars sz, int iStartInclusive, int iEndExclusive);
	CChars* Init(const char* sz, int iStartInclusive, int iEndExclusive);
	CChars* Init(char cPadCharacter, int iNumber);
	CChars* Init(CExternalString* pcString);
	CChars* InitList(const char* szFirst, ...);
	CChars* InitList(CChars* szFirst, ...);
	CChars* InitLength(int iLength);
	BOOL	InitData2(const char* szData, int iDataLength);
	void	Kill(void);
	void	DumpKill(void);
	void	Fake(const char* sz);
	void	Fake(const char* sz, int iStartInclusive, int iEndExclusive);
	CChars* Reinit(void);

	int		Length(void);
	void	Set(const char* sz);
	void	Set(CChars sz);
	void	Set(CChars* psz);
	CChars* Append(const char* sz);
	CChars* Append(const char* sz, int iStrlen);
	void	Append(CChars sz);
	void	Append(CChars* psz);
	void	Append(char c);
	void	Append(char c, int iNumber);
	CChars* Append(int i);
	CChars* Append(int i, int iBase);
	void	Append(unsigned int ui);
	void	Append(float f);
	void	Append(float f, int iNumDecimals);
	void	Append(double d);
	void	Append(double d, int iNumDecimals);
	void	Append(long long int lli);
	void	Append(unsigned long long int ulli);
	CChars*	Append(CExternalString* pcString);
	void	AppendList(const char* szFirst, ...);
	void	AppendQuoted(char c);
	void	AppendQuoted(const char* sz);
	CChars* AppendNewLine(void);
	CChars* AppendNewLine(CChars sz);
	void	AppendHexHiLo(void* pv, int iNumBytes);
	void	AppendHexLoHi(void* pv, int iNumBytes);
	void	AppendSubString(const char* sz, int iLength);
	void	AppendSubString(CChars sz, int iStartInclusive, int iEndExclusive);
	void	AppendSubString(const char* sz, int iStartInclusive, int iEndExclusive);
	void	AppendSubString(const char* szStartInclusive, const char* szEndExclusive);
	void	AppendBool(BOOL bValue);
	void	AppendBool(BOOL bValue, const char* szTrue, const char* szFalse);
	CChars* AppendData(const char* szData, size_t iMaxLength);
	CChars* AppendData(const char* szData, size_t iDataLength, size_t iMaxLength);
	BOOL	AppendData2(const char* szData, size_t iDataLength);
	void	AppendPointer(void* pv);
	BOOL	AppendFlag(unsigned int msFlags, unsigned int uiFlag, const char* szFlagName, BOOL bAppendComma = FALSE);
	void	Insert(int iPos, char c);
	void	Insert(int iPos, const char* szString);
	void	Insert(int iPos, CChars* pszString);
	void	Minimize(void);
	char*	Text(void);
	char*	Text(int iIndex);
	BOOL	Empty(void);
	void	LeftAlign(CChars sz, char cPadCharacter, unsigned int iWidth);
	void	LeftAlign(const char* sz, char cPadCharacter, unsigned int iWidth);
	void	RightAlign(CChars sz, char cPadCharacter, unsigned int iWidth);
	void	RightAlign(const char* sz, char cPadCharacter, unsigned int iWidth);
	void	RightAlign(char cPadCharacter, unsigned int iWidth);
	void	RemoveLastCharacter(void);
	void	RemoveFromStart(int iNumChars);
	void	RemoveFromEnd(int iNumChars);
	void	Remove(int iStartInclusive, int iEndExclusive);
	void	RemoveEnd(int iIndex);
	void	RemoveCharacter(int iPos);
	void	Split(CArrayChars* aszDest, char cSplitter);
	void	SplitLines(CArrayChars* aszDest);
	BOOL	Equals(const char* szString);
	BOOL	Equals(CChars szString);
	BOOL	Equals(const char* szString, int iLen);
	BOOL	Equals(CChars* pszString);
	BOOL	EqualsIgnoreCase(const char* szString);
	BOOL	EqualsIgnoreCase(CChars szString);
	BOOL	Contains(const char* szString);
	BOOL	ContainsIgnoreCase(const char* szString);
	BOOL	EndsWith(const char* szString);
	BOOL	EndsWithIgnoreCase(const char* szString);
	BOOL	StartsWith(const char* szString);
	BOOL	StartsWithIgnoreCase(const char* szString);
	int		Occurrences(const char* szString);
	BOOL	SubStringEquals(int iStart, const char* szString);
	BOOL	SubStringEqualsIgnoreCase(int iStart, const char* szString);
	int		FindFromEnd(const char* szString);
	int		FindFromEnd(int iPos, const char* szString);
	int		FindFromEnd(char c);
	int		FindFromEnd(int iIndex, char c);
	int		Find(const char* szString);
	int		Find(int iPos, const char* szString);
	int		Find(int iPos, char c);
	int		Find(char c);
	int		FindDigit(int iStartIndex);
	char	GetChar(int iIndex);
	void	SetChar(int iPos, char c);
	void	Difference(CArrayInt* paiNewToOldIndices, CArrayInt* paiOldToNewIndices, CChars szOldString);
	BOOL	IsWhiteSpace(int iPos, BOOL bIncludeNewLines);
	BOOL	IsWhiteSpace(void);
	void	SetLength(int iLength);
	int		StripWhiteSpace(BOOL bIncludeNewLines = FALSE);
	int		Compare(CChars* szOther);
	int		Compare(const char* szOther);
	int		CompareIgnoreCase(CChars* szOther);
	int		CompareIgnoreCase(const char* szOther);
	int		Replace(char cFind, char cReplace);
	int		Replace(const char* szFind, const char* szReplace);
	void	Overwrite(int iPos, const char* szReplace);
	void	Reverse(void);
	void	Clear(void);
	int		CountNewLines(void);
	int		Count(char c);
	void	LowerCase(void);
	void	UpperCase(void);

	BOOL	WriteString(CFileWriter* pcWriter);
	BOOL	ReadString(CFileReader* pcReader);
	BOOL	ReadChars(CFileReader* pcReader);  //Call this if the string was saved from a char*

	char*	CopyIntoBuffer(char* szDest, int iDestLength);

	//Suspicious... should probably be in a helper class or something.
	void	PassifyNewlines(void);
	void	MakeCPlusPlus(void);

	void	Dump(void);
	static void Dump(const char* sz);

protected:
	char*	PrivateGrow(int iNumberOfCharacters);
	int		ReplaceWithLonger(const char* szFind, const char* szReplace, int iFindLen, int iDifference);
	int		ReplaceWithShorter(const char* szFind, const char* szReplace, int iReplaceLen, int iFindLen, int iDifference);
	int		ReplaceWithEqualLength(const char* szFind, const char* szReplace, int iFindLen, int iDifference);
	int		FindEndOfLeadingWhiteSpace(BOOL bIncludeNewLines);
	int		FindStartOfTrailingWhiteSpace(BOOL bIncludeNewLines);
	void	SetNonNull(const char* sz, int iLen);
	void	SetEmpty(void);
	void	CleanIfEmpty(void);
	void	InitEmpty(void);
	BOOL	IsFakeEmpty(void);
	void	Unfake(void);
	void	UnfakeIfFakeEmpty(void);
};


int CompareChars(const void* arg1, const void* arg2);
int CompareCharsIgnoreCase(const void* arg1, const void* arg2);


#endif // !__CHARS_H__

