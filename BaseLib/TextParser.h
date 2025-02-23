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
#ifndef __TEXT_PARSER_H__
#define __TEXT_PARSER_H__
#include "Define.h"
#include "ErrorTypes.h"
#include "EnumeratorTemplate.h"
#include "Define.h"
#include "Number.h"
#include "TextPosition.h"


typedef CArrayTemplate<STextPosition>	CArrayTextPosition;


struct SParseState
{
	STextPosition		sCurrent;
	CArrayTextPosition	asPrev;

	void Init(void);
	void Kill(void);
};


enum ETextParseError
{
	TPE_EndOfFile,
	TPE_SyntaxError,
	TPE_NotSet,
};


#define NUMBER_PREFIX_DEFAULT		0x0000
#define NUMBER_PREFIX_BINARY		0x0001
#define NUMBER_PREFIX_OCTAL			0x0002
#define NUMBER_PREFIX_HEXADECIMAL	0x0004
#define INTEGER_PREFIX_ALL			(NUMBER_PREFIX_BINARY | NUMBER_PREFIX_OCTAL | NUMBER_PREFIX_HEXADECIMAL)
#define FLOAT_PREFIX_ALL			NUMBER_PREFIX_HEXADECIMAL

#define INTEGER_SUFFIX_NONE			0x0000
#define INTEGER_SUFFIX_L			0x0010
#define INTEGER_SUFFIX_LL			0x0020
#define INTEGER_SUFFIX_U			0x0040
#define INTEGER_SUFFIX_UL			0x0080
#define INTEGER_SUFFIX_ULL			0x0100
#define INTEGER_SUFFIX_CPP			(INTEGER_SUFFIX_L | INTEGER_SUFFIX_LL | INTEGER_SUFFIX_U | INTEGER_SUFFIX_UL | INTEGER_SUFFIX_ULL)
#define INTEGER_SUFFIX_JAVA			INTEGER_SUFFIX_L

#define FLOAT_SUFFIX_NONE			0x0000
#define FLOAT_SUFFIX_F				0x0200
#define FLOAT_SUFFIX_D				0x0400
#define FLOAT_SUFFIX_L				0x0800
#define FLOAT_SUFFIX_CPP			(FLOAT_SUFFIX_F | FLOAT_SUFFIX_D | FLOAT_SUFFIX_L)
#define FLOAT_SUFFIX_JAVA			(FLOAT_SUFFIX_F | FLOAT_SUFFIX_D)

#define FLOAT_EXPONENT_DEFAULT		0x0000
#define FLOAT_EXPONENT_DECIMAL		0x1000
#define FLOAT_EXPONENT_BINARY		0x2000
#define FLOAT_EXPONENT_ALL			(FLOAT_EXPONENT_DECIMAL | FLOAT_EXPONENT_BINARY)

#define NUMBER_SEPARATOR_UNDERSCORE	0x4000
#define NUMBER_SEPARATOR_APOSTROPHE	0x8000
#define NUMBER_SEPARATOR_NONE		0x0000

#define PARSER_NO_DIGIT				0xffff


class CTextParser
{
public:
	char*				mszStartOfText;
	size				miTextLen;
	char*				mszParserPos;
	char*				mszEndOfText;  //Is inclusive (ie: mszEndOfText == the last character in the parser).
	bool				mbCaseSensitive;
	bool				mbOutsideText;
	CArrayTextPosition	masPositions;
	size				miLine;
	size				miColumn;
	bool				mbAnnotated;
	ETextParseError		meError;

	bool 			Init(char* szText, size iTextLen);
	bool 			Init(char* szText);
	bool 			Init(CChars* szText);
	void			Init(void);
	void 			Kill(void);
	bool			IsEmpty(void);

	//These functions do not set errors...
	void 			StepRight(void);
	void 			StepLeft(void);
	void			TestEnd(void);
	void 			SkipWhiteSpace(bool bSkipComments = true);
	bool 			SkipCStyleComment(char** pszBegin = NULL, char** pszEnd = NULL);
	void			SkipLeftCStyleComment(void);
	bool  			SkipCPPStyleComment(char** pszBegin = NULL, char** pszEnd = NULL);
	void			SkipNewLine(void);
	void			Restart(void);
	void			SaveState(SParseState* psTextPosition);
	void			LoadState(SParseState* psTextPosition);
	void			PushPosition(void);
	void			PopPosition(void);
	void			PassPosition(void);
	bool			IsWhiteSpace(char cCurrent);
	void			PassPositions(size iNum);
	void			PopPositions(size iNum);
	void			LastPosition(void);
	STextPosition*	GetPosition(STextPosition* psPosition);
	void			SetPosition(STextPosition* psPosition);

	size			Line(void);
	size			Column(void);
	size			Length(void);
	size			RemainingLength(void);
	size			UsedLength(void);

	TRISTATE 		GetExactCharacter(char c, bool bSkipWhiteSpace = true);
	TRISTATE 		GetExactCaseInsensitiveCharacter(char c, bool bSkipWhiteSpace = true);
	TRISTATE		GetIdentifierCharacter(char* pc, bool bFirst);
	TRISTATE		GetEnumeratedCharacter(char* szCharacters, char* c, bool bSkipWhiteSpace = true);
	TRISTATE		GetCharacter(char* pc);
	TRISTATE		GetEscapeCode(char* c);

	TRISTATE		GetExactIdentifier(char* szIdentifier, bool bSkipWhiteSpace = true);
	TRISTATE		GetIdentifier(char* szIdentifier, size* piLength = NULL, bool bPassOnTest = false, bool bSkipWhiteSpace = true);
	TRISTATE		GetString(char* szString, size* piLength = NULL);  //Quotes not included.
	TRISTATE		GetQuotedCharacterSequence(char cOpenQuote, char cCloseQuote, char* szString, size* piLength = NULL, bool bPassOnTest = false, bool bSkipWhiteSpace = true, bool bAllowEscapeCharacters = true);  //'Quotes' not included.
	TRISTATE		GetExactCharacterSequence(char* szSequence, bool bSkipWhiteSpace = true);
	TRISTATE		GetCharacterSequence(char* szSequence, size* piLength = NULL, bool bSkipWhiteSpace = true);
	TRISTATE		GetExactCaseInsensitiveCharacterSequence(const char* szSequence, bool bSkipWhiteSpace = true);
	TRISTATE		GetComment(char* szComment, size* piLength, bool bSkipWhiteSpace = true);

	TRISTATE		GetDigit(uint16* pi, uint16 uiBase = 10);
	TRISTATE		GetSign(int16* pi);
	TRISTATE		GetDigits(uint64* pulli, int16* piSign, uint16* puiNumDigits, bool bSkipWhiteSpace = true, bool bTestSign = true, uint16 uiBase = 10, uint16 uiAllowedSeparator = NUMBER_SEPARATOR_NONE);
	TRISTATE		GetInteger(uint64* pulli, int16* piSign, uint16* puiNumDigits = NULL, bool bSkipWhiteSpace = true);
	TRISTATE		GetInteger(int64* pi, uint16* puiNumDigits = NULL, bool bSkipWhiteSpace = true);
	TRISTATE		GetHexadecimal(uint64* pulli, uint16* puiNumDigits = NULL, bool bSkipWhiteSpace = true);
	TRISTATE		GetHexadecimalPart(uint64* pulli, uint16* puiNumDigits, uint16 iMaxDigits = 16);
	TRISTATE		GetOctal(uint64* pulli, uint16* puiNumDigits = NULL, bool bSkipWhiteSpace = true);
	TRISTATE		GetFloat(float* pf, bool bSkipWhiteSpace = true);
	TRISTATE		GetFloat(double* pf, bool bSkipWhiteSpace = true);
	TRISTATE		GetNumber(CNumber* pcNumber, bool bSkipWhiteSpace = true);
	TRISTATE		GetIntegerLiteral(uint64* pulli, uint16 iAllowedPrefix, uint16* piBase, uint16 iAllowedSuffix, uint16* piSuffix, uint16 uiAllowedSeparator, uint16* puiNumDigits, bool bSkipWhiteSpace);
	TRISTATE		GetCharacterLiteral(uint16* pc, bool bAllowUTF16, uint16* piCharacterWidth, bool bSkipWhiteSpace);
	TRISTATE		GetStringLiteral(void* szDest, size uiDestByteLength, bool bAllowUTF16, size* piCharacterCount, uint16* piCharacterWidth, bool bSkipWhiteSpace);
	TRISTATE		GetFloatLiteral(float96* pldf, uint16 iAllowedPrefix, uint16* piBase, uint16 iAllowedSuffix, uint16* piSuffix, uint16 iAllowedExponent, uint16* piExponent, uint16 uiAllowedSeparator, uint16* piNumWholeDigits, uint16* piNumDecimalDigits, uint16* piNumExponentDigits, bool bSkipWhiteSpace);

	//Non linear functions.
	TRISTATE		FindExactIdentifier(char* szIdentifier);
	TRISTATE		FindExactCharacterSequence(char* szSequence);
	TRISTATE		FindStartOfLine(void);
	TRISTATE		FindEndOfLine(void);
	TRISTATE		FindWhiteSpace(void);

	//Helper Functions.
	TRISTATE		GetHFExactIdentifierAndInteger(char* szIdentifier, int64* piInt);
	TRISTATE		GetHFExactIdentifierAndString(char* szIdentifier, char* szString);
	TRISTATE		GetHFOpeningQuote(char* pcDestQuoteType);
	TRISTATE		GetHFClosingQuote(char cQuoteType);
	TRISTATE		ReadLine(CTextParser* pcLine);
	TRISTATE		GetDebugCharacterSequence(char* szSequence);
	TRISTATE		SkipUTF8BOM(void);

	void			SetErrorEndOfFile(void);
	void			SetErrorSyntaxError(void);
	void			AppendError(CChars* pszDest);
	void			GetText(CExternalString* pcDest);

	void			PrintPosition(CChars* pszDest);
	void			PrintPositionSingleLineParser(CChars* pszDest);
	void			PrintPositionMultilineParser(CChars* pszDest);

	bool			IsOutside(void);

	void			Dump(void);

	template<class M>	TRISTATE	GetEnumeratorIdentifier(__CEnumeratorTemplate<M>* pcEnumerator, size* piID, bool bSkipWhiteSpace = true);
	template<class M>	TRISTATE	GetEnumeratorSequence(__CEnumeratorTemplate<M>* pcEnumerator, size* piID, bool bSkipWhiteSpace = true);

	//Error Functions.
	void			GetLineLocationMarkerString(CChars* pszDest);

protected:
	TRISTATE	GetComment(char* szComment, size* piLength, char* szBegin, char* szEnd);
	TRISTATE	GetSingleInteger(char cCurrent, uint64* pulli, uint16* piBase, uint16* puiNumDigits);
	uint16		GetDigit(char cCurrent, uint16 uiBase);
	bool		IsDigit(char cCurrent, uint16 uiBase);
	TRISTATE	GetIntegerSuffix(uint16* piSuffix, uint16 iAllowedSuffix);
	TRISTATE	GetIntegerSeparator(uint16 uiAllowedSeparator);
	TRISTATE	GetCharacterLiteral(uint16* pc, bool bAllowUTF16, uint16* piCharacterWidth);
	uint16		ChangeWidth(uint16 iWidth, uint16 iOldWidth, void* szDest, size uiDestByteLength, size iLength);
	TRISTATE	GetFloatSuffix(uint16* piSuffix, uint16 iAllowedSuffix);
	float96		MakeLongDouble(uint16 uiBase, uint64 ulliWholeNumber, uint64 ulliDecimalNumber, uint16 iNumDecimalDigits, int64 lliExponentNumber);
};


TRISTATE ParseInteger(int64* pi, char* szText);
TRISTATE ParseFloat(double* pf, char* szText);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
TRISTATE CTextParser::GetEnumeratorIdentifier(__CEnumeratorTemplate<M>* pcEnumerator, size* piID, bool bSkipWhiteSpace)
{
	char* szName;
	SEnumeratorIterator		sIterator;
	size					iID;
	TRISTATE				tReturn;
	size					iLongestLength;
	size					iLength;
	bool					bFound;
	STextPosition			sStartPosition;
	STextPosition			sFoundPosition;

	GetPosition(&sStartPosition);
	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
	}

	iLongestLength = 0;
	bFound = false;
	pcEnumerator->StartIteration(&sIterator, &szName, &iID, NULL);
	while (sIterator.bValid)
	{
		tReturn = GetExactIdentifier(szName, false);
		if (tReturn == TRITRUE)
		{
			iLength = strlen(szName);
			if (iLength > iLongestLength)
			{
				iLongestLength = iLength;
				*piID = iID;
				bFound = true;
				GetPosition(&sFoundPosition);
			}
			SetPosition(&sStartPosition);
		}
		else if (tReturn == TRIERROR)
		{
			return TRIERROR;
		}
		pcEnumerator->Iterate(&sIterator, &szName, &iID, NULL);
	}

	if (bFound)
	{
		SetPosition(&sFoundPosition);
		return TRITRUE;
	}
	else
	{
		SetPosition(&sStartPosition);
		*piID = ARRAY_ELEMENT_NOT_FOUND;
		return TRIFALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
TRISTATE CTextParser::GetEnumeratorSequence(__CEnumeratorTemplate<M>* pcEnumerator, size* piID, bool bSkipWhiteSpace)
{
	char*					szName;
	SEnumeratorIterator		sIterator;
	size					iID;
	TRISTATE				tReturn;
	size					iLongestLength;
	size					iLength;
	bool					bFound;
	STextPosition			sStartPosition;
	STextPosition			sFoundPosition;

	GetPosition(&sStartPosition);
	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
	}

	iLongestLength = 0;
	bFound = false;
	pcEnumerator->StartIteration(&sIterator, &szName, &iID, NULL);
	while(sIterator.bValid)
	{
		tReturn = GetExactCharacterSequence(szName, false);
		if (tReturn == TRITRUE)
		{
			iLength = strlen(szName);
			if (iLength > iLongestLength)
			{
				iLongestLength = iLength;
				*piID = iID;
				bFound = true;
				GetPosition(&sFoundPosition);
			}
			SetPosition(&sStartPosition);
		}
		else if (tReturn == TRIERROR)
		{
			return TRIERROR;
		}
		pcEnumerator->Iterate(&sIterator, &szName, &iID, NULL);
	}

	if (bFound)
	{
		SetPosition(&sFoundPosition);
		return TRITRUE;
	}
	else
	{
		SetPosition(&sStartPosition);
		*piID = ARRAY_ELEMENT_NOT_FOUND;
		return TRIFALSE;
	}
}


#endif //__TEXT_PARSER_H__

