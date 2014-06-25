/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "ArrayChar.h"
#include "DataTypes.h"
#include "ArrayTemplate.h"
#include "ArraySimple.h"
#include "ArrayInt.h"


class CArrayString;


//Fix CChars to not allocate memory for the empty string.
//CChars must be based on something other than CArrayChar.
//Currently mallocs every 8 characters.


class CChars
{
friend class CFileReader;
friend class CFileWriter;
protected:
	CArrayChar	mcText;

public:
	void	Zero(void);
	void	Init(void);
	void	Init(char* sz);
	void	Init(CChars sz);
	void	Init(char* sz, int iStartInclusive);
	void	Init(CChars sz, int iStartInclusive, int iEndExclusive);
	void	Init(char* sz, int iStartInclusive, int iEndExclusive);
	void	Init(int iChunkSize);
	void	Init(char cPadCharacter, int iNumber);
	void	InitList(char* szFirst, ...);
	void	InitList(CChars* szFirst, ...);
	void	InitLength(int iLength);
	void	Kill(void);
	void	Fake(char* sz);
	void	Fake(char* sz, int iStartInclusive, int iEndExclusive);
	int		Length(void);
	void	Set(const char* sz);
	void	Set(CChars sz);
	void	Set(CChars* psz);
	void	Append(char* sz);
	void	Append(char* sz, int iStrlen);
	void	Append(CChars sz);  //Oi fix these to use void	Append(char* sz, int iStrlen);
	void	Append(CChars* psz);  //Oi fix these to use void	Append(char* sz, int iStrlen);
	void	Append(char c);
	void	Append(char c, int iNumber);
	void	Append(int i);
	void	Append(unsigned int ui);
	void	Append(float f);
	void	Append(float f, int iNumDecimals);
	void	Append(double d);
	void	Append(double d, int iNumDecimals);
	void	Append(long long int lli);
	void	Append(unsigned long long int ulli);
	void	AppendList(char* szFirst, ...);
	void	AppendQuoted(char c);
	void	AppendQuoted(char* sz);
	void	AppendNewLine(void);
	void	AppendHexHiLo(void* pv, int iNumBytes);
	void	AppendHexLoHi(void* pv, int iNumBytes);
	void	AppendSubString(char* sz, int iLength);
	void	AppendSubString(CChars sz, int iStartInclusive, int iEndExclusive);
	void	AppendSubString(char* sz, int iStartInclusive, int iEndExclusive);
	void	AppendSubString(char* szStartInclusive, char* szEndExclusive);
	void	AppendBool(BOOL bValue);
	void	AppendBool(BOOL bValue, char* szTrue, char* szFalse);
	void	AppendData(char* szData, int iMaxLength);
	void	AppendData2(char* szData, int iDataLength, int iMaxLength);
	void	AppendPointer(void* pv);
	void	Insert(int iPos, char c);
	void	Insert(int iPos, char* szString);
	void	Insert(int iPos, CChars* pszString);
	void	Minimize(void);
	char*	Text(void);
	char*	Text(int iIndex);
	BOOL	Empty(void);
	void	LeftAlign(CChars sz, char cPadCharacter, int iWidth);
	void	LeftAlign(char* sz, char cPadCharacter, int iWidth);
	void	RightAlign(CChars sz, char cPadCharacter, int iWidth);
	void	RightAlign(char* sz, char cPadCharacter, int iWidth);
	void	RightAlign(char cPadCharacter, int iWidth);
	void	RemoveLastCharacter(void);
	void	RemoveFromStart(int iNumChars);
	void	RemoveFromEnd(int iNumChars);
	void	Remove(int iStart, int iEnd);
	void	RemoveEnd(int iIndex);
	void	RemoveCharacter(int iPos);
	void	Split(CArrayString* aszDest, char cSplitter);
	void	SplitLines(CArrayString* aszDest);
	BOOL	Equals(char* szString);
	BOOL	Equals(CChars szString);
	BOOL	Equals(char* szString, int iLen);
	BOOL	EqualsIgnoreCase(char* szString);
	BOOL	EqualsIgnoreCase(CChars szString);
	BOOL	Contains(char* szString);
	BOOL	ContainsIgnoreCase(char* szString);
	BOOL	EndsWith(char* szString);
	BOOL	EndsWithIgnoreCase(char* szString);
	BOOL	StartsWith(char* szString);
	BOOL	StartsWithIgnoreCase(char* szString);
	int		Occurrences(char* szString);
	BOOL	SubStringEquals(int iStart, char* szString);
	BOOL	SubStringEqualsIgnoreCase(int iStart, char* szString);
	int		FindFromEnd(char* szString);
	int		FindFromEnd(int iPos, char* szString);
	int		FindFromEnd(char c);
	int		FindFromEnd(int iIndex, char c);
	int		Find(int iPos, char* szString);
	int		Find(int iPos, char c);
	int		FindDigit(int iStartIndex);
	char	GetChar(int iIndex);
	void	SetChar(int iPos, char c);
	void	Difference(CArrayInt* paiNewToOldIndices, CArrayInt* paiOldToNewIndices, CChars szOldString);
	BOOL	IsWhiteSpace(int iPos, BOOL bIncludeNewLines);
	BOOL	IsWhiteSpace(void);
	void	SetLength(int iLength);
	int		StripWhiteSpace(BOOL bIncludeNewLines = FALSE);
	int		Compare(CChars* szOther);
	int		Compare(char* szOther);
	int		CompareIgnoreCase(CChars* szOther);
	int		CompareIgnoreCase(char* szOther);
	int		Replace(char cFind, char cReplace);
	int		Replace(char* szFind, char* szReplace);
	void	Overwrite(int iPos, char* szReplace);
	void	Reverse(void);
	void	Clear(void);
	int		CountNewLines(void);
	int		Count(char c);
	void	LowerCase(void);
	void	UpperCase(void);

	BOOL	WriteString(CFileWriter* pcWriter);
	BOOL	ReadString(CFileReader* pcReader);
	BOOL	ReadString(CFileReader* pcReader, BOOL bDoesntMatter);  //Call this if the string was saved from a char*

	//Suspicious... should probably be in a helper class or something.
	void	PassifyNewlines(void);

	void	Dump(void);
	static void Dump(char* sz);

protected:
	char*	PrivateGrow(int iNumberOfCharacters);
	int		PrivateReplaceWithLonger(char* szFind, char* szReplace, int iFindLen, int iDifference);
	int		PrivateReplaceWithShorter(char* szFind, char* szReplace, int iReplaceLen, int iFindLen, int iDifference);
	int		PrivateReplaceWithEqualLength(char* szFind, char* szReplace, int iFindLen, int iDifference);
	void	PrivateFixLength(void);
	int		PrivateFindEndOfLeadingWhiteSpace(BOOL bIncludeNewLines);
	int		PrivateFindStartOfTrailingWhiteSpace(BOOL bIncludeNewLines);
};


extern CChars	gszEmptyString;


void InitEmptyString(void);
void KillEmptyString(void);
int CompareChars(const void* arg1, const void* arg2);
int CompareCharsIgnoreCase(const void* arg1, const void* arg2);


#endif //__CHARS_H__

