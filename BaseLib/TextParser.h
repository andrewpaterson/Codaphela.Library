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
#ifndef __TEXT_PARSER_H__
#define __TEXT_PARSER_H__
#include "Define.h"
#include "ErrorTypes.h"
#include "Define.h"
#include "Number.h"
#include "TextParserConfig.h"
#include "TextParserParameters.h"


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
	STextParserConfig	msConfig;

	bool 			Init(char* szText, size iTextLen);
	bool 			Init(char* szText);
	bool 			Init(CChars* szText);
	bool			Init(void);
	bool			Init(char* szText, STextParserConfig* psConfig);
	bool			Init(char* szText, size iTextLen, STextParserConfig* psConfig);
	void 			Kill(void);
	bool			IsEmpty(void);

	void			SetStringParser(TextParserParseString fParseString);
	void			SetIdentifierParser(TextParserParseIdentifier fParseIdentifier);
	void			SetExactIdentifierParser(TextParserParseExactIdentifier fParseExactIdentifier);

	//These functions do not set errors...
	void 			StepRight(void);
	void 			StepLeft(void);
	void			TestEnd(void);
	void 			SkipWhitespace(void);
	void			SkipNewLine(void);
	void			Restart(void);
	void			SaveState(SParseState* psTextPosition);
	void			LoadState(SParseState* psTextPosition);
	void			PushPosition(void);
	void			PopPosition(void);
	void			PassPosition(void);
	bool			IsWhitespace(char cCurrent);
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

	TRISTATE 		GetExactCharacter(char c, bool bSkipWhitespace = true);
	TRISTATE 		GetExactCaseInsensitiveCharacter(char c, bool bSkipWhitespace = true);
	TRISTATE		GetIdentifierCharacter(char* pc, bool bFirst);
	TRISTATE		GetEnumeratedCharacter(char* szCharacters, char* c, bool bSkipWhitespace = true);
	TRISTATE		GetCharacter(char* pc);
	TRISTATE		GetEscapeCode(char* c);

	TRISTATE		GetExactIdentifier(char* szIdentifier, bool bSkipWhitespace = true);
	TRISTATE		GetIdentifier(char* szIdentifier, size* piLength = NULL, bool bSkipWhitespace = true);
	TRISTATE		GetString(char* szString, size* piLength = NULL, bool bSkipWhitespace = true);
	TRISTATE		GetExactCharacterSequence(char* szSequence, bool bSkipWhitespace = true);
	TRISTATE		GetExactCaseInsensitiveCharacterSequence(const char* szSequence, bool bSkipWhitespace = true);

	TRISTATE		GetDigit(uint16* pi, uint16 uiBase = 10);
	TRISTATE		GetSign(int16* pi);
	TRISTATE		GetDigits(uint64* pulli, int16* piSign, uint16* puiNumDigits, bool bSkipWhitespace = true, bool bTestSign = true, uint16 uiBase = 10, uint16 uiAllowedSeparator = NUMBER_SEPARATOR_NONE);
	TRISTATE		GetInteger(uint64* pulli, int16* piSign, uint16* puiNumDigits = NULL, bool bSkipWhitespace = true);
	TRISTATE		GetInteger(int64* pi, uint16* puiNumDigits = NULL, bool bSkipWhitespace = true);
	TRISTATE		GetHexadecimal(uint64* pulli, uint16* puiNumDigits = NULL, bool bSkipWhitespace = true);
	TRISTATE		GetHexadecimalPart(uint64* pulli, uint16* puiNumDigits, uint16 iMaxDigits = 16);
	TRISTATE		GetOctal(uint64* pulli, uint16* puiNumDigits = NULL, bool bSkipWhitespace = true);
	TRISTATE		GetFloat(float* pf, bool bSkipWhitespace = true);
	TRISTATE		GetFloat(double* pf, bool bSkipWhitespace = true);
	TRISTATE		GetNumber(CNumber* pcNumber, bool bSkipWhitespace = true);
	TRISTATE		GetNumber(CNumber* pcNumber, int16 cMaxWholeNumbers, int16 cMaxDecimals, bool bSkipWhitespace = true);
	TRISTATE		GetIntegerLiteral(uint64* pulli, uint16 iAllowedPrefix, uint16* piBase, uint16 iAllowedSuffix, uint16* piSuffix, uint16 uiAllowedSeparator, uint16* puiNumDigits, bool bSkipWhitespace);
	TRISTATE		GetCharacterLiteral(uint16* pc, bool bAllowUTF16, uint16* piCharacterWidth, bool bSkipWhitespace);
	TRISTATE		GetStringLiteral(void* szDest, size uiDestByteLength, bool bAllowUTF16, size* piCharacterCount, uint16* piCharacterWidth, bool bSkipWhitespace);
	TRISTATE		GetFloatLiteral(float96* pldf, uint16 iAllowedPrefix, uint16* piBase, uint16 iAllowedSuffix, uint16* piSuffix, uint16 iAllowedExponent, uint16* piExponent, uint16 uiAllowedSeparator, uint16* piNumWholeDigits, uint16* piNumDecimalDigits, uint16* piNumExponentDigits, bool bSkipWhitespace);

	//Non linear functions.
	TRISTATE		FindExactIdentifier(char* szIdentifier);
	TRISTATE		FindExactCharacterSequence(char* szSequence);
	TRISTATE		FindStartOfLine(void);
	TRISTATE		FindEndOfLine(void);
	TRISTATE		FindWhitespace(void);

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
	char			Current(void);
	char			Current(int iIndex);
	char*			Start(void);
	char*			End(void);
	char*			Position(void);
	char*			Position(int iIndex);

	void			Dump(void);

	template<class M>	TRISTATE	GetEnumeratorIdentifier(__CEnumeratorTemplate<M>* pcEnumerator, size* piID, bool bSkipWhitespace = true);
	template<class M>	TRISTATE	GetEnumeratorSequence(__CEnumeratorTemplate<M>* pcEnumerator, size* piID, bool bSkipWhitespace = true);

	//Error Functions.
	void			GetLineLocationMarkerString(CChars* pszDest);

protected:
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
TRISTATE CTextParser::GetEnumeratorIdentifier(__CEnumeratorTemplate<M>* pcEnumerator, size* piID, bool bSkipWhitespace)
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
	if (bSkipWhitespace)
	{
		SkipWhitespace();
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
TRISTATE CTextParser::GetEnumeratorSequence(__CEnumeratorTemplate<M>* pcEnumerator, size* piID, bool bSkipWhitespace)
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
	if (bSkipWhitespace)
	{
		SkipWhitespace();
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

