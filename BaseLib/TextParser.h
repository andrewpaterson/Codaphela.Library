/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

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


#define NUMBER_PREFIX_DEFAULT		0x00
#define NUMBER_PREFIX_BINARY		0x01
#define NUMBER_PREFIX_OCTAL			0x02
#define NUMBER_PREFIX_HEXADECIMAL	0x04

#define INTEGER_PREFIX_ALL			(NUMBER_PREFIX_BINARY | NUMBER_PREFIX_OCTAL | NUMBER_PREFIX_HEXADECIMAL)
#define FLOAT_PREFIX_ALL			NUMBER_PREFIX_HEXADECIMAL

#define INTEGER_SUFFIX_NONE			0x0000
#define INTEGER_SUFFIX_L			0x0100
#define INTEGER_SUFFIX_LL			0x0200
#define INTEGER_SUFFIX_U			0x0400
#define INTEGER_SUFFIX_UL			0x0800
#define INTEGER_SUFFIX_ULL			0x1000
#define INTEGER_SUFFIX_CPP			(INTEGER_SUFFIX_L | INTEGER_SUFFIX_LL | INTEGER_SUFFIX_U | INTEGER_SUFFIX_UL | INTEGER_SUFFIX_ULL)
#define INTEGER_SUFFIX_JAVA			INTEGER_SUFFIX_L

#define FLOAT_SUFFIX_NONE			0x0000
#define FLOAT_SUFFIX_F				0x0100
#define FLOAT_SUFFIX_D				0x0200
#define FLOAT_SUFFIX_L				0x0400

#define FLOAT_SUFFIX_CPP			(FLOAT_SUFFIX_F | FLOAT_SUFFIX_D | FLOAT_SUFFIX_L)
#define FLOAT_SUFFIX_JAVA			(FLOAT_SUFFIX_F | FLOAT_SUFFIX_D)

#define FLOAT_EXPONENT_DEFAULT		0x000000
#define FLOAT_EXPONENT_DECIMAL		0x100000
#define FLOAT_EXPONENT_BINARY		0x200000
#define FLOAT_EXPONENT_ALL			(FLOAT_EXPONENT_DECIMAL | FLOAT_EXPONENT_BINARY)

#define NUMBER_SEPARATOR_UNDERSCORE	0x10000000
#define NUMBER_SEPARATOR_APOSTROPHE	0x20000000
#define NUMBER_SEPARATOR_NONE		0x00000000


class CTextParser
{
public:
	char*				mszStartOfText;
	int					miTextLen;
	char*				mszParserPos;
	char*				mszEndOfText;  //Is inclusive (ie: mszEndOfText == the last character in the parser).
	BOOL				mbCaseSensitive;
	BOOL				mbOutsideText;
	CArrayTextPosition	masPositions;
	int					miLine;
	int					miColumn;
	BOOL				mbAnnotated;
	ETextParseError		meError;

	BOOL 			Init(char* szText, int iTextLen);
	BOOL 			Init(char* szText);
	BOOL 			Init(CChars* szText);
	void			Init(void);
	void 			Kill(void);
	BOOL			IsEmpty(void);

	//These functions do not set errors...
	void 			StepRight(void);
	void 			StepLeft(void);
	void			TestEnd(void);
	void 			SkipWhiteSpace(BOOL bSkipComments = TRUE);
	BOOL 			SkipCStyleComment(char** pszBegin = NULL, char** pszEnd = NULL);
	void			SkipLeftCStyleComment(void);
	BOOL  			SkipCPPStyleComment(char** pszBegin = NULL, char** pszEnd = NULL);
	void			SkipNewLine(void);
	void			Restart(void);
	void			SaveState(SParseState* psTextPosition);
	void			LoadState(SParseState* psTextPosition);
	void			PushPosition(void);
	void			PopPosition(void);
	void			PassPosition(void);
	BOOL			IsWhiteSpace(char cCurrent);
	void			PassPositions(int iNum);
	void			PopPositions(int iNum);
	void			LastPosition(void);
	STextPosition*	GetPosition(STextPosition* psPosition);
	void			SetPosition(STextPosition* psPosition);

	int				Line(void);
	int				Column(void);
	int				Length(void);
	int				RemainingLength(void);
	int				UsedLength(void);

	TRISTATE 		GetExactCharacter(char c, BOOL bSkipWhiteSpace = TRUE);
	TRISTATE 		GetExactCaseInsensitiveCharacter(char c, BOOL bSkipWhiteSpace = TRUE);
	TRISTATE		GetIdentifierCharacter(char* pc, BOOL bFirst);
	TRISTATE		GetEnumeratedCharacter(char* szCharacters, char* c, BOOL bSkipWhiteSpace = TRUE);
	TRISTATE		GetCharacter(char* pc);
	TRISTATE		GetEscapeCode(char* c);

	TRISTATE		GetExactIdentifier(char* szIdentifier, BOOL bSkipWhiteSpace = TRUE);
	TRISTATE		GetIdentifier(char* szIdentifier, int* piLength = NULL, BOOL bPassOnTest = FALSE, BOOL bSkipWhiteSpace = TRUE);
	TRISTATE		GetString(char* szString, int* piLength = NULL);  //Quotes not included.
	TRISTATE		GetQuotedCharacterSequence(char cOpenQuote, char cCloseQuote, char* szString, int* piLength = NULL, BOOL bPassOnTest = FALSE, BOOL bSkipWhiteSpace = TRUE, BOOL bAllowEscapeCharacters = TRUE);  //'Quotes' not included.
	TRISTATE		GetExactCharacterSequence(char* szSequence, BOOL bSkipWhiteSpace = TRUE);
	TRISTATE		GetCharacterSequence(char* szSequence, int* piLength = NULL, BOOL bSkipWhiteSpace = TRUE);
	TRISTATE		GetExactCaseInsensitiveCharacterSequence(const char* szSequence, BOOL bSkipWhiteSpace = TRUE);
	TRISTATE		GetComment(char* szComment, int* piLength, BOOL bSkipWhiteSpace = TRUE);

	TRISTATE		GetDigit(int* pi, int iBase = 10);
	TRISTATE		GetSign(int* pi);
	TRISTATE		GetDigits(unsigned long long int* pulli, int* piSign, int* iNumDigits, BOOL bSkipWhiteSpace = TRUE, BOOL bTestSign = TRUE, int iBase = 10, int iAllowedSeparator = NUMBER_SEPARATOR_NONE);
	TRISTATE		GetInteger(unsigned long long int* pulli, int* piSign, int* iNumDigits = NULL, BOOL bSkipWhiteSpace = TRUE);
	TRISTATE		GetInteger(int* pi, int* iNumDigits = NULL, BOOL bSkipWhiteSpace = TRUE);
	TRISTATE		GetHexadecimal(unsigned long long int* pulli, int* iNumDigits = NULL, BOOL bSkipWhiteSpace = TRUE);
	TRISTATE		GetHexadecimalPart(unsigned long long int* pulli, int* piNumDigits, int iMaxDigits = 16);
	TRISTATE		GetOctal(unsigned long long int* pulli, int* iNumDigits = NULL, BOOL bSkipWhiteSpace = TRUE);
	TRISTATE		GetFloat(float* pf, BOOL bSkipWhiteSpace = TRUE);
	TRISTATE		GetFloat(double* pf, BOOL bSkipWhiteSpace = TRUE);
	TRISTATE		GetNumber(CNumber* pcNumber, BOOL bSkipWhiteSpace = TRUE);
	TRISTATE		GetIntegerLiteral(unsigned long long int* pulli, int iAllowedPrefix, int* piBase, int iAllowedSuffix, int* piSuffix, int iAllowedSeparator, int* piNumDigits, BOOL bSkipWhiteSpace);
	TRISTATE		GetCharacterLiteral(unsigned short* pc, BOOL bAllowUTF16, int* piCharacterWidth, BOOL bSkipWhiteSpace);
	TRISTATE		GetStringLiteral(void* szDest, size_t uiDestByteLength, BOOL bAllowUTF16, int* piCharacterCount, int* piCharacterWidth, BOOL bSkipWhiteSpace);
	TRISTATE		GetFloatLiteral(long double* pldf , int iAllowedPrefix, int* piBase, int iAllowedSuffix, int* piSuffix, int iAllowedExponent, int* piExponent, int iAllowedSeparator, int* piNumWholeDigits, int* piNumDecinalDigits, int* piNumExponentDigits, BOOL bSkipWhiteSpace);

	//Non linear functions.
	TRISTATE		FindExactIdentifier(char* szIdentifier);
	TRISTATE		FindExactCharacterSequence(char* szSequence);
	TRISTATE		FindStartOfLine(void);
	TRISTATE		FindEndOfLine(void);
	TRISTATE		FindWhiteSpace(void);

	//Helper Functions.
	TRISTATE		GetHFExactIdentifierAndInteger(char* szIdentifier, int* piInt);
	TRISTATE		GetHFExactIdentifierAndString(char* szIdentifier, char* szString);
	TRISTATE		GetHFOpeningQuote(char* pcDestQuoteType);
	TRISTATE		GetHFClosingQuote(char cQuoteType);
	TRISTATE		ReadLine(CTextParser* pcLine);
	TRISTATE		GetDebugCharacterSequence(char* szSequence);

	void			SetErrorEndOfFile(void);
	void			SetErrorSyntaxError(void);
	void			AppendError(CChars* pszDest);

	void			PrintPosition(CChars* pszDest);
	void			PrintPositionSingleLineParser(CChars* pszDest);
	void			PrintPositionMultilineParser(CChars* pszDest);

	BOOL			IsOutside(void);

	void			Dump(void);

	template<class M>	TRISTATE	GetEnumeratorIdentifier(__CEnumeratorTemplate<M>* pcEnumerator, int* piID, BOOL bSkipWhiteSpace = TRUE);
	template<class M>	TRISTATE	GetEnumeratorSequence(__CEnumeratorTemplate<M>* pcEnumerator, int* piID, BOOL bSkipWhiteSpace = TRUE);

protected:
	TRISTATE	GetComment(char* szComment, int* piLength, char* szBegin, char* szEnd);
	TRISTATE	GetSingleInteger(char cCurrent, unsigned long long int* pulli, int* piBase, int* piNumDigits);
	int			GetDigit(char cCurrent, int iBase);
	BOOL		IsDigit(char cCurrent, int iBase);
	TRISTATE	GetIntegerSuffix(int* piSuffix, int iAllowedSuffix);
	TRISTATE	GetIntegerSeparator(int iAllowedSeparator);
	TRISTATE	GetCharacterLiteral(unsigned short* pc, BOOL bAllowUTF16, int* piCharacterWidth);
	int			ChangeWidth(int iWidth, int iOldWidth, void* szDest, size_t uiDestByteLength, int iLength);
	TRISTATE	GetFloatSuffix(int* piSuffix, int iAllowedSuffix);
	long double	MakeLongDouble(int iBase, unsigned long long int ulliWholeNumber, unsigned long long int ulliDecimalNumber, int iNumDecimalDigits, long long int lliExponentNumber);
};


TRISTATE ParseInteger(int* pi, char* szText);
TRISTATE ParseFloat(double* pf, char* szText);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
TRISTATE CTextParser::GetEnumeratorIdentifier(__CEnumeratorTemplate<M>* pcEnumerator, int* piID, BOOL bSkipWhiteSpace)
{
	char* szName;
	SEnumeratorIterator		sIterator;
	int						iID;
	TRISTATE				tReturn;
	int						iLongestLength;
	int						iLength;
	BOOL					bFound;
	STextPosition			sStartPosition;
	STextPosition			sFoundPosition;

	GetPosition(&sStartPosition);
	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
	}

	iLongestLength = -1;
	bFound = FALSE;
	pcEnumerator->StartIteration(&sIterator, &szName, &iID, NULL);
	while (sIterator.bValid)
	{
		tReturn = GetExactIdentifier(szName, FALSE);
		if (tReturn == TRITRUE)
		{
			iLength = strlen(szName);
			if (iLength > iLongestLength)
			{
				iLongestLength = iLength;
				*piID = iID;
				bFound = TRUE;
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
		*piID = -1;
		return TRIFALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
TRISTATE CTextParser::GetEnumeratorSequence(__CEnumeratorTemplate<M>* pcEnumerator, int* piID, BOOL bSkipWhiteSpace)
{
	char*					szName;
	SEnumeratorIterator		sIterator;
	int						iID;
	TRISTATE				tReturn;
	int						iLongestLength;
	int						iLength;
	BOOL					bFound;
	STextPosition			sStartPosition;
	STextPosition			sFoundPosition;

	GetPosition(&sStartPosition);
	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
	}

	iLongestLength = -1;
	bFound = FALSE;
	pcEnumerator->StartIteration(&sIterator, &szName, &iID, NULL);
	while(sIterator.bValid)
	{
		tReturn = GetExactCharacterSequence(szName, FALSE);
		if (tReturn == TRITRUE)
		{
			iLength = strlen(szName);
			if (iLength > iLongestLength)
			{
				iLongestLength = iLength;
				*piID = iID;
				bFound = TRUE;
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
		*piID = -1;
		return TRIFALSE;
	}
}


#endif //__TEXT_PARSER_H__

