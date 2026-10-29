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
//No include guard: CharsClass.inl is included once per string size (ArraySizeNatural.inl and
//ArraySizeLarge.inl define the names and types it is written with).

class CHARS
{
CONSTRUCTABLE_MACRO(CHARS)
friend class CFileReader;
friend class CFileWriter;
protected:
	ARRAY_CHAR	mcText;

public:
	void		_Init(void);
	CHARS*		Init(void);
	CHARS*		Init(const char* szString);
	CHARS*		Init(CHARS szString);
	CHARS*		Init(CHARS* psz);
	CHARS*		Init(const char* szString, ASIZE iStartInclusive);
	CHARS*		Init(CHARS szString, ASIZE iStartInclusive, ASIZE iEndExclusive);
	CHARS*		Init(const char* szString, ASIZE iStartInclusive, ASIZE iEndExclusive);
	CHARS*		Init(char cPadCharacter, ASIZE iNumber);
	CHARS*		Init(CExternalString* pcString);
	CHARS*		InitList(const char* szFirst, ...);
	CHARS*		InitList(CHARS* szFirst, ...);
	CHARS*		InitLength(ASIZE iLength);
	CHARS*		InitLength(CMallocator* pcMalloc, ASIZE iLength);
	bool		InitData2(const char* szData, ASIZE iDataLength);
	void		Kill(void);
	void		DumpKill(void);
	void		Fake(const char* szString);
	void		Fake(const char* szString, ASIZE iStartInclusive, ASIZE iEndExclusive);
	CHARS*		ReInit(void);

	ASIZE		Length(void);
	void		Set(const char* szString);
	void		Set(CHARS szString);
	void		Set(CHARS* psz);
	CHARS*		Append(const char* szString);
	CHARS*		Append(const char* szString, ASIZE iStrlen);
	CHARS*		Append(CHARS szString);
	CHARS*		Append(CHARS* psz);
#ifdef CHARS_NATURAL
	CHARS*		Append(CHARS_NATURAL szString);
	CHARS*		Append(CHARS_NATURAL* psz);
#endif // CHARS_NATURAL
	CHARS*		Append(CCharsImmutable* psz);
	CHARS*		Append(char cPadCharacter);
	CHARS*		Append(char cPadCharacter, ASIZE iNumber);
	CHARS*		Append(int32 i);
	CHARS*		Append(int32 i, uint16 iBase);
	CHARS*		Append(uint32 ui);
	CHARS*		Append(uint32 ui, uint16 iBase);
#ifdef __W65C816__
	//An int is 16 bits on the W65C816: ints match these exactly (the 32 bit ones would all need a conversion, so none
	//would be chosen).
	CHARS*		Append(int16 i);
	CHARS*		Append(int16 i, uint16 iBase);
	CHARS*		Append(uint16 ui);
	CHARS*		Append(uint16 ui, uint16 iBase);
#endif // __W65C816__
	CHARS*		Append(float f, ASIZE iMaxDecimals = -1, bool bAppendF = false);
	CHARS*		Append(double d, ASIZE iMaxDecimals = -1, bool bAppendD = false);
	CHARS*		Append(int64 lli);
	CHARS*		Append(uint64 ulli);
	CHARS*		Append(CExternalString* pcString);
	CHARS*		AppendList(const char* szFirst, ...);
	CHARS*		AppendQuoted(char c);
	CHARS*		AppendQuoted(const char* szString);
	CHARS*		AppendNewLine(void);
	CHARS*		AppendNewLine(CHARS szString);
	CHARS*		AppendCarriageReturn(void);
	CHARS*		AppendHexHiLo(void* pv, ASIZE iNumBytes);
	CHARS*		AppendHexLoHi(void* pv, ASIZE iNumBytes);
	CHARS*		AppendSubString(const char* szString, ASIZE iLength);
	CHARS*		AppendSubString(CHARS szString, ASIZE iStartInclusive, ASIZE iEndExclusive);
	CHARS*		AppendSubString(CHARS* pszString, ASIZE iStartInclusive, ASIZE iEndExclusive);
	CHARS*		AppendSubString(const char* szString, ASIZE iStartInclusive, ASIZE iEndExclusive);
	CHARS*		AppendSubString(const char* szStartInclusive, const char* szEndExclusive);
	CHARS*		AppendBool(bool bValue);
	CHARS*		AppendBool(bool bValue, const char* szTrue, const char* szFalse);
	CHARS*		AppendData(const char* szData, ASIZE iMaxLength);
	CHARS*		AppendData(const char* szData, ASIZE iDataLength, ASIZE iMaxLength);
	bool		AppendData2(const char* szData, ASIZE iDataLength);
	CHARS*		AppendPointer(void* pv);
	bool		AppendFlag(uint32 msFlags, uint32 uiFlag, const char* szFlagName, bool bAppendComma = false);
	void		Insert(ASIZE iPos, char c);
	void		Insert(ASIZE iPos, char c, ASIZE uiCount);
	void		Insert(ASIZE iPos, const char* szString);
	void		Insert(ASIZE iPos, CHARS* pszString);
	void		Minimize(void);
	char*		Text(void);
	char*		Text(ASIZE iIndex);
	bool		Empty(void);
	void		LeftAlign(CHARS szString, char cPadCharacter, ASIZE iWidth);
	void		LeftAlign(const char* szString, char cPadCharacter, ASIZE iWidth);
	void		RightAlign(CHARS szString, char cPadCharacter, ASIZE iWidth);
	void		RightAlign(const char* szString, char cPadCharacter, ASIZE iWidth);
	void		RightAlign(ASIZE uiOffset, char cPadCharacter, ASIZE uiWidth);
	void		RightAlign(char cPadCharacter, ASIZE iWidth);
	void		RemoveLastCharacter(void);
	void		RemoveFromStart(ASIZE iNumChars);
	void		RemoveFromEnd(ASIZE iNumChars);
	void		Remove(ASIZE iStartInclusive, ASIZE iEndExclusive);
	void		RemoveEnd(ASIZE iIndex);
	void		RemoveCharacter(ASIZE iPos);
	void		Split(CArrayChars* aszDest, char cSplitter);
	void		SplitLines(CArrayChars* aszDest);
	bool		Equals(const char* szString);
	bool		Equals(CHARS szString);
	bool		Equals(const char* szString, ASIZE iLen);
	bool		Equals(CHARS* pszString);
	bool		EqualsIgnoreCase(const char* szString);
	bool		EqualsIgnoreCase(CHARS szString);
	bool		Contains(const char* szString);
	bool		ContainsIgnoreCase(const char* szString);
	bool		Contains(char c);
	bool		EndsWith(const char* szString);
	bool		EndsWithIgnoreCase(const char* szString);
	bool		EndsWith(char c);
	bool		StartsWith(const char* szString);
	bool		StartsWithIgnoreCase(const char* szString);
	bool		StartsWith(char c);
	ASIZE		Occurrences(const char* szString);
	bool		SubStringEquals(ASIZE iStart, const char* szString);
	bool		SubStringEqualsIgnoreCase(ASIZE iStart, const char* szString);
	ASIZE		FindFromEnd(const char* szString);
	ASIZE		FindFromEnd(ASIZE iPos, const char* szString);
	ASIZE		FindFromEnd(char c);
	ASIZE		FindFromEnd(ASIZE iIndex, char c);
	ASIZE		Find(const char* szString);
	ASIZE		Find(ASIZE iPos, const char* szString);
	ASIZE		Find(ASIZE iPos, char c);
	ASIZE		Find(char c);
	ASIZE		FindDigit(ASIZE iStartIndex);
	char		GetChar(ASIZE iIndex);
	void		SetChar(ASIZE iPos, char c);
	void		Difference(CArrayInt* paiNewToOldIndices, CArrayInt* paiOldToNewIndices, CHARS szOldString);
	bool		IsWhitespace(ASIZE iPos, bool bIncludeNewLines);
	bool		IsWhitespace(void);
	void		SetLength(ASIZE iLength);
	ASIZE		StripWhitespace(bool bIncludeNewLines = false);
	int			Compare(CHARS* szOther);
	int			Compare(const char* szOther);
	int			CompareIgnoreCase(CHARS* szOther);
	int			CompareIgnoreCase(const char* szOther);
	ASIZE		Replace(char cFind, char cReplace);
	ASIZE		Replace(const char* szFind, const char* szReplace);
	void		Overwrite(ASIZE iPos, const char* szReplace);
	void		Reverse(void);
	void		Clear(void);
	ASIZE		CountNewLines(void);
	ASIZE		Count(char c);
	void		LowerCase(void);
	void		UpperCase(void);
	void		ReplaceNonTabsWithSpace(void);

	bool		WriteString(CFileWriter* pcWriter);
	bool		ReadString(CFileReader* pcReader);
	bool		ReadChars(CFileReader* pcReader);  //Call this if the string was saved from a char*

	char*		CopyIntoBuffer(char* szDest, ASIZE iDestLength);

	//Suspicious... should be in CharsHelper or something.
	void		PassifyNewlines(void);
	void		MakeCPlusPlus(void);
	char*		FindLineContaining(const char* szPosition, ASIZE* piLineNumber);

	void		Dump(void);
	static void Dump(const char* szString);

protected:
	char*	PrivateGrow(ASIZE iNumberOfCharacters);
	ASIZE	ReplaceWithLonger(const char* szFind, const char* szReplace, ASIZE iFindLen, ASIZE iDifference);
	ASIZE	ReplaceWithShorter(const char* szFind, const char* szReplace, ASIZE iReplaceLen, ASIZE iFindLen, ASIZE iDifference);
	ASIZE	ReplaceWithEqualLength(const char* szFind, const char* szReplace, ASIZE iFindLen);
	ASIZE	FindEndOfLeadingWhitespace(bool bIncludeNewLines);
	ASIZE	FindStartOfTrailingWhitespace(bool bIncludeNewLines);
	void	SetNonNull(const char* szString, ASIZE iLen);
	void	SetEmpty(void);
	void	CleanIfEmpty(void);
	void	InitEmpty(void);
	bool	IsFakeEmpty(void);
	void	Unfake(void);
	void	UnfakeIfFakeEmpty(void);
};
