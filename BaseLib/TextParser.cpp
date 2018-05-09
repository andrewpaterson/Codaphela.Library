/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include <math.h>
#include "Newline.h"
#include "Log.h"
#include "PointerFunctions.h"
#include "StringHelper.h"
#include "TextParser.h"
#include "EscapeCodes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextParser::Init(char* szText)
{
	return Init(szText, (int)strlen(szText));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextParser::Init(CChars* szText)
{
	return Init(szText->Text(), szText->Length());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::Init(void)
{
	Init(NULL, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextParser::Init(char* szText, int iTextLen)
{
	//This assumes that szText has already been passified.
	if (szText == NULL)
	{
		miLine = 0;
		miColumn = 0;
		mszStartOfText = NULL;
		mszParserPos = NULL;
		mszEndOfText = NULL;
		miTextLen = 0;
		mbCaseSensitive = TRUE;
		mbAnnotated = FALSE;
		mbOutsideText = TRUE;
		masPositions.Init();
		return TRUE;
	}
	else if (CountCarriageReturns(szText, iTextLen) == 0)
	{
		miLine = 0;
		miColumn = 0;
		mszStartOfText = szText;
		mszParserPos = szText;
		mszEndOfText = &mszStartOfText[iTextLen - 1];
		miTextLen = iTextLen;
		mbCaseSensitive = TRUE;
		mbAnnotated = FALSE;
		TestEnd();
		masPositions.Init();
		PushPosition();
		return TRUE;
	}
	else
	{
		miLine = 0;
		miColumn = 0;
		mszStartOfText = NULL;
		mszParserPos = NULL;
		mszEndOfText = NULL;
		miTextLen = 0;
		masPositions.Init();

		gcLogger.Error("Parsed text contains carriage returns.");
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::Kill(void)
{
	mszParserPos = NULL;
	mszStartOfText = NULL;
	miTextLen = 0;
	mszEndOfText = NULL;
	masPositions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextParser::IsEmpty(void)
{
	return miTextLen <= 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STextPosition::Init(void)
{
	szParserPos = NULL;
	iLine = 0;
	iColumn = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SParseState::Init(void)
{
	sCurrent.Init();
	asPrev.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SParseState::Kill(void)
{
	sCurrent.Init();
	asPrev.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::StepRight(void)
{
	//Can only move right if we are not sitting to the right of the last character.
	if (mszParserPos <= mszEndOfText)
	{
		if (mszParserPos[0] == '\n')
		{
			miLine++;
			miColumn = 0;
		}
		else
		{
			miColumn++;
		}
		mszParserPos = &mszParserPos[1];
	}
	TestEnd();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::StepLeft(void)
{
	char* szSearchPos;

	//Can only move left if we are not sitting to the left of the first character.
	if (mszParserPos >= mszStartOfText)
	{
		mszParserPos = &mszParserPos[-1];
		if (mszParserPos[0] == '\n')
		{
			miLine--;

			szSearchPos = &mszParserPos[-1];
			miColumn = 0;
			if (szSearchPos > mszStartOfText)
			{
				for (;;)
				{
					if ((szSearchPos[0] == '\n') || (szSearchPos == mszStartOfText))
					{
						break;
					}
					miColumn++;
					szSearchPos = &szSearchPos[-1];
				}
			}
		}
		else
		{
			miColumn--;
		}
	}
	TestEnd();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::TestEnd(void)
{
	if ((mszParserPos >= mszStartOfText) && (mszParserPos <= mszEndOfText))
	{
		mbOutsideText = FALSE;
		return;
	}
	mbOutsideText = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextParser::IsWhiteSpace(char cCurrent)
{
	return ((cCurrent == ' ') || (cCurrent == '\n') || (cCurrent == '\t'));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::SkipWhiteSpace(void)
{
	char	cCurrent;

	for (;;)
	{
		if (mbOutsideText)
		{
			return;
		}

		cCurrent = mszParserPos[0];

		//Nice clean white space...
		if (IsWhiteSpace(cCurrent))
		{
			StepRight();
		}

		//Possibly nasty comments...
		else if (cCurrent == '/')
		{
			StepRight();

			if (!mbOutsideText)
			{
				cCurrent = mszParserPos[0];
				if (cCurrent == '*')
				{
					//Put the parser back where it was.
					StepLeft();
					SkipCStyleComment();
				}
				else if (cCurrent == '/')
				{
					//Put the parser back where it was.
					StepLeft();
					if (!SkipCPPStyleComment())
					{
						break;
					}
				}
				else
				{
					//Was something other than white-space starting with /
					StepLeft();
					break;
				}
			}
		}
		else
		{
			//Was not white-space at all.
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::SkipNewLine(void)
{
	char	cCurrent;

	if (mbOutsideText)
	{
		return;
	}

	cCurrent = mszParserPos[0];

	//Nice clean white space...
	if (cCurrent == '\n')
	{
		StepRight();
	}
	else if (mszParserPos == mszEndOfText)
	{
		StepRight();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::SkipCStyleComment(void)
{
	char	cCurrent;
	int		iDepth;

	iDepth = 0;

	PushPosition();
	for (;;)
	{
		if (mbOutsideText)
		{
			PassPosition();
			return;
		}

		cCurrent = mszParserPos[0];
		if (cCurrent == '/')
		{
			StepRight();
			if (!mbOutsideText)
			{
				cCurrent = mszParserPos[0];
				if (cCurrent == '*')
				{
					iDepth++;
				}
				else
				{
					//Wasn't a comment start... step back.
					StepLeft();
				}
			}
			else
			{
				PassPosition();
				return;
			}
		}
		else if (cCurrent == '*')
		{
			StepRight();
			if (!mbOutsideText)
			{
				cCurrent = mszParserPos[0];
				if (cCurrent == '/')
				{
					iDepth--;
				}
				else
				{
					//Wasn't the end of a comment... step back...
					StepLeft();
				}
			}
			else
			{
				PassPosition();
				return;
			}
		}

		if (iDepth == 0)
		{
			//No more nested comments...  bail..
			return;
		}
		StepRight();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::SkipLeftCStyleComment(void)
{
	char	cCurrent;
	int		iDepth;

	iDepth = 0;

	PushPosition();
	for (;;)
	{
		if (mbOutsideText)
		{
			PassPosition();
			return;
		}

		cCurrent = mszParserPos[0];
		if (cCurrent == '/')
		{
			StepLeft();
			if (!mbOutsideText)
			{
				cCurrent = mszParserPos[0];
				if (cCurrent == '*')
				{
					iDepth++;
				}
				else
				{
					//Wasn't a comment start... step back.
					StepRight();
				}
			}
			else
			{
				PassPosition();
				return;
			}
		}
		else if (cCurrent == '*')
		{
			StepLeft();
			if (!mbOutsideText)
			{
				cCurrent = mszParserPos[0];
				if (cCurrent == '/')
				{
					iDepth--;
				}
				else
				{
					//Wasn't the end of a comment... step back...
					StepRight();
				}
			}
			else
			{
				PassPosition();
				return;
			}
		}

		if (iDepth == 0)
		{
			//No more nested comments...  bail..
			return;
		}
		StepLeft();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextParser::SkipCPPStyleComment(void)
{
	char	cCurrent;
	int		iCount;

	if (mbOutsideText)
	{
		return TRUE;
	}

	PushPosition();
	cCurrent = mszParserPos[0];

	if (cCurrent == '/')
	{
		StepRight();
		if (!mbOutsideText)
		{
			cCurrent = mszParserPos[0];
			if (cCurrent == '/')
			{
				for (iCount = 0;; iCount++)
				{
					StepRight();
					if (!mbOutsideText)
					{
						cCurrent = mszParserPos[0];

						if (cCurrent == '\n')
						{
							//This is the end of the line and the end of the comment.
							StepRight();
							PassPosition();
							return TRUE;
						}

						if (mbAnnotated)
						{
							if (cCurrent == '@')
							{
								//Wasn't a comment, was an annotation.
								PopPosition();
								return FALSE;
							}
						}
					}
					else
					{
						PassPosition();
						return TRUE;
					}
				}
			}
			else
			{
				PopPosition();
				return TRUE;
			}
		}
		else
		{
			//Wasn't a comment.
			StepLeft();
			return TRUE;
		}
	}
	PopPosition();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetExactCharacter(char c, BOOL bSkipWhiteSpace)
{
	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
	}
	if (!mbOutsideText)
	{
		if (mszParserPos[0] == c)
		{
			StepRight();
			return TRITRUE;
		}
		return TRIFALSE;
	}
	else
	{
		SetErrorEndOfFile();
		return TRIERROR;
	}
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetExactCaseInsensitiveCharacter(char c, BOOL bSkipWhiteSpace)
{
	char	c1, c2;

	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
	}
	if (!mbOutsideText)
	{
		c1 = ToUpper(mszParserPos[0]);
		c2 = ToUpper(c);
		if (c1 == c2)
		{
			StepRight();
			return TRITRUE;
		}
		return TRIFALSE;
	}
	else
	{
		SetErrorEndOfFile();
		return TRIERROR;
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetEnumeratedCharacter(char* szCharacters, char* c, BOOL bSkipWhiteSpace)
{
	int			iNumCharacters;
	char		cCurrent;
	TRISTATE	tResult;

	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
	}

	iNumCharacters = (int)(strlen(szCharacters));
	for (int i = 0; i < iNumCharacters; i++)
	{
		cCurrent = szCharacters[i];
		tResult = GetExactCharacter(cCurrent, FALSE);
		ReturnOnError(tResult);
		if (tResult == TRITRUE)
		{
			if (c != NULL)
			{
				*c = cCurrent;
				return TRITRUE;
			}
		}
	}
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetCharacter(char* pc)
{
	if (!mbOutsideText)
	{
		*pc = mszParserPos[0];
		StepRight();
		return TRITRUE;
	}
	else
	{
		SetErrorEndOfFile();
		return TRIERROR;
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetIdentifierCharacter(char* pc, BOOL bFirst)
{
	char	cCurrent;

	if (!mbOutsideText)
	{
		cCurrent = mszParserPos[0];
		*pc = cCurrent;
		//The first character of an identifier must be one of these...
		if (((cCurrent >= 'a') && (cCurrent <= 'z')) || ((cCurrent >= 'A') && (cCurrent <= 'Z')) || (cCurrent == '_'))
		{
			StepRight();
			return TRITRUE;
		}

		//Additional characters can also be...
		if (!bFirst)
		{
			if ((cCurrent >= '0') && (cCurrent <= '9'))
			{
				StepRight();
				return TRITRUE;
			}
		}
		return TRIFALSE;
	}
	else
	{
		SetErrorEndOfFile();
		return TRIERROR;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetExactCharacterSequence(char* szSequence)
{
	char		cCurrent;
	int			iPos;

	iPos = 0;
	PushPosition();
	SkipWhiteSpace();

	//Make sure we're not off the end of the file.
	if (mbOutsideText)
	{
		PopPosition();
		SetErrorEndOfFile();
		return TRIERROR;
	}

	for (;;)
	{
		if (szSequence[iPos] == 0)
		{
			//Got all the way to the NULL character.
			PassPosition();
			return TRITRUE;
		}
		if (!mbOutsideText)
		{
			cCurrent = mszParserPos[0];
			if (cCurrent == szSequence[iPos])
			{
				StepRight();
				iPos++;
			}
			else
			{
				//Put the parser back where it was.
				PopPosition();
				return TRIFALSE;
			}
		}
		else
		{
			//Put the parser back where it was.
			PopPosition();
			return TRIFALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetExactCaseInsensitiveCharacterSequence(const char* szSequence)
{
	int			iPos;
	char		c1, c2;

	PushPosition();
	SkipWhiteSpace();
	iPos = 0;

	//Make sure we're not off the end of the file.
	if (mbOutsideText)
	{
		PopPosition();
		SetErrorEndOfFile();
		return TRIERROR;
	}

	for (;;)
	{
		if (szSequence[iPos] == 0)
		{
			//Got all the way to the NULL character.
			PassPosition();
			return TRITRUE;
		}
		if (!mbOutsideText)
		{
			c1 = ToUpper(mszParserPos[0]);
			c2 = ToUpper(szSequence[iPos]);
			if (c1 == c2)
			{
				StepRight();
				iPos++;
			}
			else
			{
				//Put the parser back where it was.
				PopPosition();
				return TRIFALSE;
			}
		}
		else
		{
			//Put the parser back where it was.
			PopPosition();
			return TRIFALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetDebugCharacterSequence(char* szSequence)
{
	TRISTATE	tResult;

	PushPosition();
	tResult = GetExactCharacterSequence(szSequence);
	PopPosition();
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetCharacterSequence(char* szSequence, int* piLength)
{
	char	c;
	BOOL	bFirst;
	int		iPos;

	bFirst = TRUE;
	iPos = 0;
	PushPosition();
	SkipWhiteSpace();

	//Make sure we're not off the end of the file.
	if (mbOutsideText)
	{
		PopPosition();
		SetErrorEndOfFile();
		return TRIERROR;
	}

	for (;;)
	{
		if (!mbOutsideText)
		{
			c = mszParserPos[0];
			StepRight();

			if (IsWhiteSpace(c))
			{
				if (bFirst)
				{
					if (szSequence)
						szSequence[iPos] = 0;

					PopPosition();
					return TRIFALSE;
				}
				else
				{
					if (szSequence)
						szSequence[iPos] = 0;

					if (szSequence)
						PassPosition();
					else
						PopPosition();

					SafeAssign(piLength, iPos);
					return TRITRUE;
				}
			}
			else
			{
				if (szSequence)
					szSequence[iPos] = c;
			}
		}
		else
		{
			if (bFirst)
			{
				PopPosition();
				SetErrorEndOfFile();
				return TRIERROR;
			}
			else
			{
				if (szSequence)
					szSequence[iPos] = 0;

				if (szSequence)
					PassPosition();
				else
					PopPosition();

				SafeAssign(piLength, iPos);
				return TRITRUE;
			}
		}
		bFirst = FALSE;
		iPos++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetExactIdentifier(char* szIdentifier)
{
	char		cCurrent;
	int			iPos;
	TRISTATE	tResult;

	iPos = 0;
	PushPosition();
	SkipWhiteSpace();

	//Make sure we're not off the end of the file.
	if (mbOutsideText)
	{
		PopPosition();
		SetErrorEndOfFile();
		return TRIERROR;
	}

	for (;;)
	{
		if (szIdentifier[iPos] == 0)
		{
			//Got all the way to the NULL character.
			//If there are additional identifier characters then we do not have the right identifier.
			if (!mbOutsideText)
			{
				tResult = GetIdentifierCharacter(&cCurrent, iPos == 0);
				if (tResult == TRITRUE)
				{
					//Put the parser back where it was.
					PopPosition();
					return TRIFALSE;
				}
			}
			PassPosition();
			return TRITRUE;
		}
		if (!mbOutsideText)
		{
			cCurrent = mszParserPos[0];
			if (cCurrent == szIdentifier[iPos])
			{
				StepRight();
				iPos++;
			}
			else
			{
				//Put the parser back where it was.
				PopPosition();
				return TRIFALSE;
			}
		}
		else
		{
			//Put the parser back where it was.
			PopPosition();
			return TRIFALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetIdentifier(char* szIdentifier, int* piLength, BOOL bPassOnTest)
{
	char	c;
	BOOL	bFirst;
	int		iPos;

	bFirst = TRUE;
	iPos = 0;
	PushPosition();
	SkipWhiteSpace();

	//Make sure we're not off the end of the file.
	if (mbOutsideText)
	{
		PopPosition();
		SetErrorEndOfFile();
		return TRIERROR;
	}

	for (;;)
	{
		if (!mbOutsideText)
		{
			if (GetIdentifierCharacter(&c, bFirst) != TRITRUE)
			{
				if (bFirst)
				{
					if (szIdentifier)
						szIdentifier[iPos] = 0;

					PopPosition();
					return TRIFALSE;
				}
				else
				{
					if (szIdentifier)
						szIdentifier[iPos] = 0;

					if (szIdentifier || bPassOnTest)
						PassPosition();
					else
						PopPosition();

					SafeAssign(piLength, iPos);
					return TRITRUE;
				}
			}
			else
			{
				if (szIdentifier)
					szIdentifier[iPos] = c;
			}
		}
		else
		{
			if (bFirst)
			{
				PopPosition();
				SetErrorEndOfFile();
				return TRIERROR;
			}
			else
			{
				if (szIdentifier)
					szIdentifier[iPos] = 0;

				if (szIdentifier)
					PassPosition();
				else
					PopPosition();

				SafeAssign(piLength, iPos);
				return TRITRUE;
			}
		}
		bFirst = FALSE;
		iPos++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetString(char* szString, int* piLength)
{
	return GetQuotedCharacterSequence('\"', '\"', szString, piLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetQuotedCharacterSequence(char cOpenQuote, char cCloseQuote, char* szString, int* piLength, BOOL bPassOnTest, BOOL bSkipWhiteSpace)
{
	int			iPos;
	char		cCurrent;
	TRISTATE	tReturn;
	char		cEscape;

	PushPosition();

	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
	}

	if (!mbOutsideText)
	{
		if (GetExactCharacter(cOpenQuote, FALSE))
		{
			iPos = 0;
			for (;;)
			{
				if (!mbOutsideText)
				{
					cCurrent = mszParserPos[0];
					if (cCurrent == cCloseQuote)
					{
						if (szString)
							szString[iPos] = 0;

						StepRight();

						if (szString || bPassOnTest)
							PassPosition();
						else
							PopPosition();

						SafeAssign(piLength, iPos);
						return TRITRUE;
					}
					//We have an escape character...
					else if (cCurrent == '\\')
					{
						StepRight();
						tReturn = GetEscapeCode(&cEscape);
						if (szString)
							szString[iPos] = cEscape;

						iPos++;
						if (tReturn == TRIFALSE)
						{
							PopPosition();
							SetErrorSyntaxError();
							return TRIERROR;
						}
						else if (tReturn == TRIERROR)
						{
							PopPosition();
							//Don't set the error here, it's already been set by GetEscapeCode
							return TRIERROR;
						}
					}
					else if (cCurrent == '\n')
					{
						//Just ignore new lines.
						StepRight();
					}
					else
					{
						if (szString)
							szString[iPos] = cCurrent;

						iPos++;
						StepRight();
					}
				}
				else
				{
					//This has never been tested.
					PopPosition();
					SetErrorSyntaxError();
					return TRIERROR;
				}
			}
		}
		else
		{
			//No quote so not a string.
			PopPosition();
			return TRIFALSE;
		}
	}
	else
	{
		PopPosition();
		SetErrorEndOfFile();
		return TRIERROR;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetEscapeCode(char* c)
{
	char cReturn;

	if (!mbOutsideText)
	{

		cReturn = ::GetEscapeCode(mszParserPos[0]);
		if (cReturn == '@')
		{
			SetErrorSyntaxError();
			return TRIERROR;
		}
		else
		{
			*c = cReturn;
		}

		StepRight();
		return TRITRUE;
	}
	else
	{
		SetErrorEndOfFile();
		return TRIERROR;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetDigit(int* pi, int iBase)
{
	char	cCurrent;

	if (!mbOutsideText)
	{
		cCurrent = mszParserPos[0];
		if (iBase <= 10)
		{
			if ((cCurrent >= '0') && (cCurrent <= ('0' + iBase - 1)))
			{
				*pi = (int)(cCurrent - '0');
				StepRight();
				return TRITRUE;
			}
			else
			{
				return TRIFALSE;
			}
		}
		else
		{
			cCurrent = ToUpper(cCurrent);

			if ((cCurrent >= '0') && (cCurrent <= '9'))
			{
				*pi = (int)(cCurrent - '0');
				StepRight();
				return TRITRUE;
			}
			else if ((cCurrent >= 'A') && (cCurrent <= ('A' + iBase - 11)))
			{
				*pi = (int)(cCurrent - 'A' + 10);
				StepRight();
				return TRITRUE;
			}
			else
			{
				return TRIFALSE;
			}
		}
	}
	else
	{
		SetErrorEndOfFile();
		return TRIERROR;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetSign(int* pi)
{
	char	cCurrent;

	if (!mbOutsideText)
	{
		*pi = 1;
		cCurrent = mszParserPos[0];
		if (cCurrent == '-')
		{
			*pi = -1;
			StepRight();
			return TRITRUE;
		}
		else if (cCurrent == '+')
		{
			StepRight();
			return TRITRUE;
		}
		else return TRIFALSE;
	}
	else
	{
		SetErrorEndOfFile();
		return TRIERROR;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetInteger(unsigned long long int* pulli, int* piSign, int* piNumDigits, BOOL bSkipWhiteSpace)
{
	TRISTATE	tResult;

	PushPosition();

	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
	}

	//Make sure we're not off the end of the file.
	if (mbOutsideText)
	{
		PopPosition();
		SetErrorEndOfFile();
		return TRIERROR;
	}

	tResult  = GetDigits(pulli, piSign, piNumDigits, bSkipWhiteSpace);
	if (tResult == TRITRUE)
	{
		//Make sure there are no decimals.
		if (mszParserPos[0] == '.')
		{
			PopPosition();
			return TRIFALSE;
		}

		PassPosition();
		return TRITRUE;
	}
	PopPosition();
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetInteger(int* pi, int* piNumDigits, BOOL bSkipWhiteSpace)
{
	unsigned long long int	ulliTemp;
	TRISTATE				tReturn;
	int						iSign;

	tReturn = GetInteger(&ulliTemp, &iSign, piNumDigits, bSkipWhiteSpace);
	*pi = ((int)ulliTemp) * iSign;
	return tReturn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetDigits(unsigned long long int* pulli, int* piSign, int* piNumDigits, BOOL bSkipWhiteSpace, BOOL bTestSign)
{
	unsigned long long int	iNum;
	int						iSign;
	int						iTemp;
	TRISTATE				tReturn;
	BOOL					bFirstDigit;
	int						i;

	//This still needs to be failed on the case where the number is larger than MAX_ULONG.
	PushPosition();
	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
	}

	*pulli = 0;
	i = 0;
	if (!mbOutsideText)
	{
		iNum = 0;

		if (bTestSign)
			GetSign(&iSign);
		else
			iSign = 1;

		bFirstDigit = TRUE;
		for (;;)
		{
			if (!mbOutsideText)
			{
				tReturn = GetDigit(&iTemp);
				if (tReturn == TRITRUE)
				{
					i++;
					iNum *= 10;
					iNum += iTemp;
				}
				else if ((tReturn == TRIFALSE) || (tReturn == TRIERROR))
				{
					break;
				}
				bFirstDigit = FALSE;
			}
			else
			{
				break;
			}
		}

		if (bFirstDigit)
		{
			//might already have got a sign...  so reset the parser.
			PopPosition();
			return TRIFALSE;
		}
		else
		{
			*piSign = iSign;
			*pulli = iNum;
			if (piNumDigits)
			{
				*piNumDigits = i;
			}
			PassPosition();
			return TRITRUE;
		}
	}
	else
	{
		PopPosition();
		SetErrorEndOfFile();
		return TRIERROR;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetHexadecimal(unsigned long long int* pulli, int* piNumDigits)
{
	TRISTATE				tReturn;

	PushPosition();
	SkipWhiteSpace();

	tReturn = GetExactCharacter('0', FALSE);
	if (tReturn != TRITRUE)
	{
		PopPosition();
		return tReturn;
	}

	tReturn = GetExactCaseInsensitiveCharacter('X', FALSE);
	if (tReturn != TRITRUE)
	{
		PopPosition();
		return tReturn;
	}

	tReturn = GetHexadecimalPart(pulli, piNumDigits);
	if (tReturn != TRITRUE)
	{
		PopPosition();
	}
	else
	{
		PassPosition();
	}
	return tReturn;
}


///////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetHexadecimalPart(unsigned long long int* pulli, int* piNumDigits, int iMaxDigits)
{
	unsigned long long int	iNum;
	int						iTemp;
	TRISTATE				tReturn;
	int						i;

	*pulli = 0;
	if (!mbOutsideText)
	{
		iNum = 0;
		i = 0;
		for (;;)
		{
			if (!mbOutsideText)
			{
				tReturn = GetDigit(&iTemp, 16);
				if (tReturn == TRITRUE)
				{
					i++;
					iNum *= 16;
					iNum += iTemp;
					if (i == iMaxDigits)
					{
						*pulli = iNum;
						if (piNumDigits)
						{
							*piNumDigits = i;
						}
						return TRITRUE;
					}
				}
				else if ((tReturn == TRIFALSE) || (tReturn == TRIERROR))
				{
					*pulli = iNum;
					if (piNumDigits)
					{
						*piNumDigits = i;
					}
					return TRITRUE;
				}
			}
			else
			{
				if (i > 0)
				{
					*pulli = iNum;
					if (piNumDigits)
					{
						*piNumDigits = i;
					}
					return TRITRUE;
				}

				SetErrorSyntaxError();
				return TRIERROR;
			}
		}
	}
	else
	{
		SetErrorEndOfFile();
		return TRIERROR;
	}
}


/////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetOctal(unsigned long long int* pulli, int* piNumDigits)
{
	unsigned long long int	iNum;
	int						iTemp;
	TRISTATE				tReturn;
	int						i;

	PushPosition();
	SkipWhiteSpace();

	tReturn = GetExactCharacter('0', FALSE);
	if (tReturn != TRITRUE)
	{
		PopPosition();
		return tReturn;
	}

	*pulli = 0;
	i = 0;
	if (!mbOutsideText)
	{
		iNum = 0;

		for (;;)
		{
			if (!mbOutsideText)
			{
				tReturn = GetDigit(&iTemp, 8);
				if (tReturn == TRITRUE)
				{
					i++;
					iNum *= 8;
					iNum += iTemp;
				}
				else if ((tReturn == TRIFALSE) || (tReturn == TRIERROR))
				{
					*pulli = iNum;
					if (piNumDigits)
					{
						*piNumDigits = i;
					}
					PassPosition();
					return TRITRUE;
				}
			}
			else
			{
				if (i > 0)
				{
					*pulli = iNum;
					if (piNumDigits)
					{
						*piNumDigits = i;
					}
					PassPosition();
					return TRITRUE;
				}

				PopPosition();
				SetErrorSyntaxError();
				return TRIERROR;
			}
		}
	}
	else
	{
		PopPosition();
		SetErrorEndOfFile();
		return TRIERROR;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetFloat(float* pf)
{
	double		d;
	TRISTATE	t;

	t = GetFloat(&d);
	if (t == TRITRUE)
	{
		*pf = (float)d;
	}
	return t;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetFloat(double* pf)
{
	unsigned long long int	ulliLeft;
	unsigned long long int	ulliRight;
	TRISTATE				tReturn;
	int						iNumDecimals;
	double					fLeft;
	double					fRight;
	double					fTemp;
	BOOL					bLeft;
	int						iSign;

	PushPosition();
	SkipWhiteSpace();

	*pf = 0.0f;
	if (!mbOutsideText)
	{
		//Try and get the mantissa.
		tReturn = GetDigits(&ulliLeft, &iSign, NULL);
		bLeft = TRUE;

		//Just return on errors an non-numbers.
		if (tReturn == TRIFALSE)
		{
			//There may still be a decimal point...
			ulliLeft = 0;
			bLeft = FALSE;
		}
		else if (tReturn == TRIERROR)
		{
			PopPosition();
			SetErrorSyntaxError();
			return TRIERROR;
		}

		fLeft = ((double)ulliLeft) * iSign;
		tReturn = GetExactCharacter('.', FALSE);
		if (tReturn == TRITRUE)
		{
			tReturn = GetDigits(&ulliRight, &iSign, &iNumDecimals);
			if (tReturn == TRITRUE)
			{
				if (iSign <= 0)
				{
					//Cant have: 34.-342 but -34.342 would be fine.
					PopPosition();
					SetErrorSyntaxError();
					return TRIERROR;
				}

				fRight = (double)ulliRight;
				fTemp = pow(10.0L, -iNumDecimals);
				fRight *= fTemp;

				if (fLeft >= 0)
				{
					*pf = fLeft + fRight;
				}
				else
				{
					*pf = fLeft - fRight;
				}
				PassPosition();
				return TRITRUE;
			}
			else
			{
				//A decimal point must be followed by a number.
				PopPosition();
				SetErrorSyntaxError();
				return TRIERROR;
			}
		}
		else
		{
			//No decimal point...
			if (!bLeft)
			{
				//No digits and no point...
				PopPosition();
				return TRIFALSE;
			}
			else
			{
				*pf = fLeft;
				PassPosition();
				return TRITRUE;
			}
		}
	}
	else
	{
		PopPosition();
		SetErrorEndOfFile();
		return TRIERROR;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetNumber(CNumber* pcNumber)
{
	char*		szStart;
	double	fIgnored;
	TRISTATE	tResult;
	int			iLength;

	SkipWhiteSpace();
	szStart = mszParserPos;
	tResult = GetFloat(&fIgnored);
	if (tResult == TRITRUE)
	{
		iLength = (int)(mszParserPos-szStart);
		pcNumber->Init(szStart, iLength);
		return TRITRUE;
	}
	else
	{
		return tResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::SaveState(SParseState* psParserState)
{
	psParserState->Init();
	psParserState->sCurrent.szParserPos = mszParserPos;
	psParserState->sCurrent.iLine = miLine;
	psParserState->sCurrent.iColumn = miColumn;

	psParserState->asPrev.Copy(&masPositions);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::LoadState(SParseState* psParserState)
{
	mszParserPos = psParserState->sCurrent.szParserPos;
	miLine = psParserState->sCurrent.iLine;
	miColumn = psParserState->sCurrent.iColumn;
	masPositions.Copy(&psParserState->asPrev);
	TestEnd();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::FindExactIdentifier(char* szIdentifier)
{
	char*			szPosition;
	TRISTATE		result;

	PushPosition();
	SkipWhiteSpace();

	for (;;)
	{
		szPosition = mszParserPos;
		result = GetExactIdentifier(szIdentifier);
		if (result == TRIERROR)
		{
			//We've reached the end of the file without finding the identifier.
			PopPosition();
			return TRIFALSE;
		}
		else if (result == TRIFALSE)
		{
			//Try the next actual character along.
			StepRight();
			SkipWhiteSpace();
		}
		else if (result == TRITRUE)
		{
			mszParserPos = szPosition;
			PassPosition();
			return TRITRUE;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::FindExactCharacterSequence(char* szSequence)
{
	char*			szPosition;
	TRISTATE		result;

	PushPosition();
	for (;;)
	{
		szPosition = mszParserPos;
		result = GetExactCharacterSequence(szSequence);
		if (result == TRIERROR)
		{
			//We've reached the end of the file without finding the identifier.
			PopPosition();
			return TRIFALSE;
		}
		else if (result == TRIFALSE)
		{
			//Try the next actual character along.
			StepRight();
			SkipWhiteSpace();
		}
		else if (result == TRITRUE)
		{
			mszParserPos = szPosition;
			PassPosition();
			return TRITRUE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::Restart(void)
{
	mszParserPos = mszStartOfText;
	miLine = 0;
	miColumn = 0;
	masPositions.Kill();
	masPositions.Init();
	PushPosition();
	TestEnd();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::FindStartOfLine(void)
{
	char cCurrent;
	BOOL bInQuotes;

	PushPosition();

	//If we're off the end of the file we can't return the beginning of the line.
	if (mbOutsideText)
	{
		PopPosition();
		SetErrorEndOfFile();
		return TRIERROR;
	}

	bInQuotes = FALSE;
	for (;;)
	{
		cCurrent = mszParserPos[0];
		if (cCurrent == '"')
		{
			bInQuotes = !bInQuotes;
		}
		if (!bInQuotes)
		{
			SkipLeftCStyleComment();
		}
		StepLeft();

		//If we have no more text then the start of the line is the start of the text.
		if (mbOutsideText)
		{
			mszParserPos = mszStartOfText;
			miColumn = 0;
			PassPosition();
			return TRITRUE;
		}

		if (!bInQuotes)
		{
			//Get the current character.
			cCurrent = mszParserPos[0];

			//If we get find an end of line character we've gone to far, go right again.
			if (cCurrent == '\n')
			{
				StepRight();
				PassPosition();
				return TRITRUE;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::FindEndOfLine(void)
{
	char	cCurrent;
	BOOL	bInQuotes;
	char	cPrev;
	BOOL	bInCppComment;

	PushPosition();

	//If we're off the end of the file we can't go to the end of the line.
	if (mbOutsideText)
	{
		PopPosition();
		SetErrorEndOfFile();
		return TRIERROR;
	}

	bInCppComment = FALSE;
	bInQuotes = FALSE;
	cCurrent = 0;
	for (;;)
	{
		cPrev = cCurrent;
		cCurrent = mszParserPos[0];

		if ((cPrev == '/') && (cCurrent == '/'))
		{
			bInCppComment = TRUE;
		}

		if (!bInCppComment)
		{
			if (cCurrent == '"')
			{
				bInQuotes = !bInQuotes;
			}

			if (!bInQuotes)
			{
				SkipCStyleComment();
			}
		}


		//If we have no more text then ... blah, blah, blah
		if (mbOutsideText)
		{
			mszParserPos = mszEndOfText;
			miColumn--;  //This might work.
			PassPosition();
			return TRITRUE;
		}

		if ((!bInQuotes) || (bInCppComment))  //Note:  (NOT in quotes) OR (in Cpp comment).
		{
			//If we get find an end of line character we've gone to far, go right again.
			cCurrent = mszParserPos[0];
			if (cCurrent == '\n')
			{
				PassPosition();
				return TRITRUE;
			}
		}

		StepRight();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::FindWhiteSpace(void)
{
	char cCurrent;
	BOOL bStartOfComment;

	bStartOfComment = FALSE;
	for (;;)
	{
		if (mbOutsideText)
		{
			return TRIFALSE;
		}

		cCurrent = mszParserPos[0];
		if (IsWhiteSpace(cCurrent))
		{
			return TRITRUE;
		}
		if (bStartOfComment)
		{
			if ((cCurrent == '/') || (cCurrent == '*'))
			{
				StepLeft();
				return TRITRUE;
			}
		}
		if (cCurrent = '/')
		{
			bStartOfComment = TRUE;
		}
		else
		{
			bStartOfComment = FALSE;
		}
		StepRight();
	}
}


// ----------------------- Helper Functions ------------------------------
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetHFExactIdentifierAndInteger(char* szIdentifier, int* piInt)
{
	TRISTATE	tReturn;

	PushPosition();

	tReturn = GetExactIdentifier(szIdentifier);
	if ((tReturn == TRIERROR) || (tReturn == TRIFALSE))
	{
		PopPosition();
		return tReturn;
	}
	tReturn = GetInteger(piInt);
	if ((tReturn == TRIERROR) || (tReturn == TRIFALSE))
	{
		PopPosition();
		return tReturn;
	}

	PassPosition();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetHFExactIdentifierAndString(char* szIdentifier, char* szString)
{
	TRISTATE	tReturn;

	PushPosition();

	tReturn = GetExactIdentifier(szIdentifier);
	if ((tReturn == TRIERROR) || (tReturn == TRIFALSE))
	{
		PopPosition();
		return tReturn;
	}
	tReturn = GetString(szString);
	if ((tReturn == TRIERROR) || (tReturn == TRIFALSE))
	{
		PopPosition();
		return tReturn;
	}

	PassPosition();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::ReadLine(CTextParser* pcLine)
{
	char*	szStart;
	char*	szEnd;

	pcLine->Init();
	ReturnErrorOnErrorAndFalse(FindStartOfLine());
	szStart = mszParserPos;
	ReturnErrorOnErrorAndFalse(FindEndOfLine());
	szEnd = mszParserPos;

	SkipNewLine();
	if (mbOutsideText)
	{
		szEnd++;
	}

	pcLine->Init(szStart, (int)(szEnd - szStart));
	pcLine->mbAnnotated = mbAnnotated;
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::PushPosition(void)
{
	STextPosition*	psTextPosition;

	psTextPosition = masPositions.Push();
	psTextPosition->iLine = miLine;
	psTextPosition->iColumn = miColumn;
	psTextPosition->szParserPos = mszParserPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::PopPosition(void)
{
	STextPosition	sTextPosition;

	masPositions.Pop(&sTextPosition);
	miLine = sTextPosition.iLine;
	miColumn = sTextPosition.iColumn;
	mszParserPos = sTextPosition.szParserPos;
	TestEnd();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::PassPosition(void)
{
	masPositions.Pop();
	TestEnd();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::PassPositions(int iNum)
{
	if (iNum == 0)
	{
		return;
	}
	masPositions.RemoveRange(masPositions.NumElements()-iNum, masPositions.NumElements());
	TestEnd();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::PopPositions(int iNum)
{
	STextPosition*	psTextPosition;

	psTextPosition = masPositions.Get(masPositions.NumElements()-iNum-0);
	miLine = psTextPosition->iLine;
	miColumn = psTextPosition->iColumn;
	mszParserPos = psTextPosition->szParserPos;
	masPositions.RemoveRange(masPositions.NumElements()-iNum, masPositions.NumElements());
	TestEnd();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int	CTextParser::Line(void)
{
	return miLine;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int	CTextParser::Column(void)
{
	return miColumn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTextParser::Length(void)
{
	return (int)(mszEndOfText - mszStartOfText) + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTextParser::RemainingLength(void)
{
	return (int)(mszEndOfText - mszParserPos) + 1;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTextParser::UsedLength(void)
{
	return (int)(mszParserPos - mszStartOfText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::SetErrorEndOfFile(void)
{
	meError = TPE_EndOfFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::SetErrorSyntaxError(void)
{
	meError = TPE_SyntaxError;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseInteger(int* pi, char* szText)
{
	CTextParser	cTextParser;
	TRISTATE	tResult;

	cTextParser.Init(szText);
	tResult = cTextParser.GetInteger(pi);
	cTextParser.Kill();

	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseFloat(double* pf, char* szText)
{
	CTextParser		cTextParser;
	TRISTATE		tResult;
	double		f;
	int				iExponent;
	int				iTemp;
	double		fTen;

	cTextParser.Init(szText);
	tResult = cTextParser.GetFloat(&f);
	if (tResult == TRITRUE)
	{
		tResult = cTextParser.GetExactCaseInsensitiveCharacter('e', FALSE);
		if (tResult == TRITRUE)
		{
			tResult = cTextParser.GetInteger(&iExponent, &iTemp, FALSE);
			if (tResult == TRITRUE)
			{
				fTen = 10.0;
				*pf = pow(fTen, iExponent) * f;
				return TRITRUE;
			}
			else
			{
				return TRIFALSE;
			}
		}
		else
		{
			*pf = f;
			return TRITRUE;
		}
	}
	else
	{
		return TRIFALSE;
	}
	cTextParser.Kill();

	return tResult;
}
