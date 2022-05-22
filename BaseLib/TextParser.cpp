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
#include <math.h>
#include "Newline.h"
#include "Logger.h"
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
		meError = TPE_NotSet;
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
		meError = TPE_NotSet;
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
		meError = TPE_NotSet;
		miLine = 0;
		miColumn = 0;
		mszStartOfText = NULL;
		mszParserPos = NULL;
		mszEndOfText = NULL;
		miTextLen = 0;
		masPositions.Init();

		gcLogger.Error2(__METHOD__, " Parsed text contains carriage returns.", NULL);
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
		if (*mszParserPos == '\n')
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
		if (*mszParserPos == '\n')
		{
			miLine--;

			szSearchPos = &mszParserPos[-1];
			miColumn = 0;
			if (szSearchPos > mszStartOfText)
			{
				for (;;)
				{
					if ((*szSearchPos == '\n') || (szSearchPos == mszStartOfText))
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
void CTextParser::SkipWhiteSpace(BOOL bSkipComments)
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
			if (bSkipComments)
			{
				StepRight();

				if (!mbOutsideText)
				{
					cCurrent = mszParserPos[0];
					if (cCurrent == '*')
					{
						//Put the parser back where it was.
						StepLeft();
						if (!SkipCStyleComment())
						{
							break;
						}
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
				//Not skipping comments.
				break;
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
BOOL CTextParser::SkipCStyleComment(char** pszBegin, char** pszEnd)
{
	char	cCurrent;
	int		iDepth;

	iDepth = 0;

	SafeAssign(pszBegin, NULL);
	SafeAssign(pszEnd, NULL);

	PushPosition();
	for (;;)
	{
		if (mbOutsideText)
		{
			PassPosition();
			return TRUE;
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
					if (iDepth == 1)
					{
						SafeAssign(pszBegin, &mszParserPos[1]);
					}
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
				return TRUE;
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
					SafeAssign(pszEnd, &mszParserPos[-2]);

					StepRight();
					return TRUE;
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
				return TRUE;
			}
		}

		if (iDepth == 0)
		{
			//No more nested comments...  bail..
			return TRUE;
		}
		StepRight();
	}

	return FALSE;
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
BOOL CTextParser::SkipCPPStyleComment(char** pszBegin, char** pszEnd)
{
	char	cCurrent;
	int		iCount;

	if (mbOutsideText)
	{
		return TRUE;
	}

	PushPosition();
	cCurrent = mszParserPos[0];

	SafeAssign(pszBegin, NULL);
	SafeAssign(pszEnd, NULL);

	if (cCurrent == '/')
	{
		StepRight();
		if (!mbOutsideText)
		{
			cCurrent = mszParserPos[0];
			if (cCurrent == '/')
			{
				SafeAssign(pszBegin, &mszParserPos[1]);

				for (iCount = 0;; iCount++)
				{
					StepRight();
					if (!mbOutsideText)
					{
						cCurrent = mszParserPos[0];

						if (cCurrent == '\n')
						{
							SafeAssign(pszEnd, &mszParserPos[-1]);

							//This is the end of the line and the end of the comment.
							StepRight();
							PassPosition();
							return TRUE;
						}

						if (mbAnnotated)
						{
							if (cCurrent == '@')
							{
								SafeAssign(pszBegin, NULL);

								//Wasn't a comment, was an annotation.
								PopPosition();
								return FALSE;
							}
						}
					}
					else
					{
						SafeAssign(pszEnd, mszEndOfText);

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
		if (*mszParserPos == c)
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
TRISTATE CTextParser::GetExactCharacterSequence(char* szSequence, BOOL bSkipWhiteSpace)
{
	char		cCurrent;
	int			iPos;

	iPos = 0;
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
TRISTATE CTextParser::GetExactCaseInsensitiveCharacterSequence(const char* szSequence, BOOL bSkipWhiteSpace)
{
	int			iPos;
	char		c1, c2;

	PushPosition();

	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
	}

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
TRISTATE CTextParser::GetCharacterSequence(char* szSequence, int* piLength, BOOL bSkipWhiteSpace)
{
	char	c;
	BOOL	bFirst;
	int		iPos;

	bFirst = TRUE;
	iPos = 0;
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
TRISTATE CTextParser::GetExactIdentifier(char* szIdentifier, BOOL bSkipWhiteSpace)
{
	char		cCurrent;
	int			iPos;
	TRISTATE	tResult;

	iPos = 0;
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
TRISTATE CTextParser::GetIdentifier(char* szIdentifier, int* piLength, BOOL bPassOnTest, BOOL bSkipWhiteSpace)
{
	char	c;
	BOOL	bFirst;
	int		iPos;

	bFirst = TRUE;
	iPos = 0;
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
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetComment(char* szComment, int* piLength, char* szBegin, char* szEnd)
{
	ptrdiff_t	iLength;

	if ((szBegin == NULL || szEnd == NULL))
	{
		PopPosition();
		return TRIFALSE;
	}

	iLength = szEnd - szBegin + 1;
	if (szComment)
	{
		memcpy(szComment, szBegin, iLength);
		szComment[iLength] = '\0';
	}
	SafeAssign(piLength, iLength);

	PassPosition();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetComment(char* szComment, int* piLength, BOOL bSkipWhiteSpace)
{
	char		cCurrent;
	char*		szBegin;
	char*		szEnd;

	PushPosition();

	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace(FALSE);
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
				StepLeft();
				if (!SkipCStyleComment(&szBegin, &szEnd))
				{
					PassPosition();
					return TRIERROR;
				}

				return GetComment(szComment, piLength, szBegin, szEnd);
			}
			else if (cCurrent == '/')
			{
				StepLeft();
				if (!SkipCPPStyleComment(&szBegin, &szEnd))
				{
					PassPosition();
					return TRIERROR;
				}

				return GetComment(szComment, piLength, szBegin, szEnd);

			}
			else
			{
				PopPosition();
				return TRIFALSE;
			}
		}
		else
		{
			PassPosition();
			return TRIERROR;
		}
	}
	else
	{
		PopPosition();
		return TRIFALSE;
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
TRISTATE CTextParser::GetQuotedCharacterSequence(char cOpenQuote, char cCloseQuote, char* szString, int* piLength, BOOL bPassOnTest, BOOL bSkipWhiteSpace, BOOL bAllowEscapeCharacters)
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
					else if (cCurrent == '\\' && bAllowEscapeCharacters)
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
	int		iDigit;

	if (!mbOutsideText)
	{
		cCurrent = mszParserPos[0];
		iDigit = GetDigit(cCurrent, iBase);
		if (iDigit != -1)
		{
			*pi = iDigit;
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
		SetErrorEndOfFile();
		return TRIERROR;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTextParser::GetDigit(char cCurrent, int iBase)
{
	if (iBase <= 10)
	{
		if ((cCurrent >= '0') && (cCurrent <= ('0' + iBase - 1)))
		{
			return (int)(cCurrent - '0');
		}
		else
		{
			return -1;
		}
	}
	else
	{
		cCurrent = ToUpper(cCurrent);

		if ((cCurrent >= '0') && (cCurrent <= '9'))
		{
			return (int)(cCurrent - '0');
		}
		else if ((cCurrent >= 'A') && (cCurrent <= ('A' + iBase - 11)))
		{
			return (int)(cCurrent - 'A' + 10);
		}
		else
		{
			return -1;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextParser::IsDigit(char cCurrent, int iBase)
{
	if (iBase <= 10)
	{
		if ((cCurrent >= '0') && (cCurrent <= ('0' + iBase - 1)))
		{
			return TRUE;
		}
		else
			{
			return FALSE;
		}
	}
	else
	{
		cCurrent = ToUpper(cCurrent);

		if ((cCurrent >= '0') && (cCurrent <= '9'))
		{
			return TRUE;
		}
		else if ((cCurrent >= 'A') && (cCurrent <= ('A' + iBase - 11)))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetIntegerSuffix(int* piSuffix, int iAllowedSuffix)
{
	char	cCurrent;
	BOOL	bU;
	BOOL	bL;

	if (!mbOutsideText)
	{
		bU = FALSE;
		bL = FALSE;

		PushPosition();

		cCurrent = mszParserPos[0];
		if ((iAllowedSuffix & INTEGER_SUFFIX_ULL) || (iAllowedSuffix & INTEGER_SUFFIX_UL) || (iAllowedSuffix & INTEGER_SUFFIX_U))
		{
			if ((cCurrent == 'u') || (cCurrent == 'U'))
			{
				bU = TRUE;
				StepRight();

				if (mbOutsideText)
				{
					*piSuffix = INTEGER_SUFFIX_U;
					PassPosition();
					return TRITRUE;
				}
			}
			else if ((cCurrent == 'l') || (cCurrent == 'L'))
			{
			}
			else
			{
				PopPosition();
				return TRIFALSE;
			}
		}

		cCurrent = mszParserPos[0];
		if ((iAllowedSuffix & INTEGER_SUFFIX_ULL) || (iAllowedSuffix & INTEGER_SUFFIX_UL) || (iAllowedSuffix & INTEGER_SUFFIX_LL) || (iAllowedSuffix & INTEGER_SUFFIX_L))
		{
			if ((cCurrent == 'l') || (cCurrent == 'L'))
			{
				bL = TRUE;
				StepRight();

				if (mbOutsideText)
				{
					if (bU)
					{
						*piSuffix = INTEGER_SUFFIX_UL;
					}
					else
					{
						*piSuffix = INTEGER_SUFFIX_L;
					}
					PassPosition();
					return TRITRUE;
				}
			}
			else
			{
				if (bU && bL)
				{
					*piSuffix = INTEGER_SUFFIX_UL;
				}
				else if (bL && !bU)
				{
					*piSuffix = INTEGER_SUFFIX_L;
				}
				else if (!bL && !bU)
				{
					*piSuffix = INTEGER_SUFFIX_NONE;
				}
				PassPosition();
				return TRITRUE;
			}
		}

		cCurrent = mszParserPos[0];
		if ((iAllowedSuffix & INTEGER_SUFFIX_ULL) || (iAllowedSuffix & INTEGER_SUFFIX_LL))
		{
			if ((cCurrent == 'l') || (cCurrent == 'L'))
			{
				StepRight();

				if (mbOutsideText)
				{
					if (bU && bL)
					{
						*piSuffix = INTEGER_SUFFIX_ULL;
					}
					else if (!bU && bL)
					{
						*piSuffix = INTEGER_SUFFIX_LL;
					}
					else
					{
						PopPosition();
						return TRIFALSE;
					}
					PassPosition();
					return TRITRUE;
				}
				else
				{
					if (bU && bL)
					{
						*piSuffix = INTEGER_SUFFIX_ULL;
					}
					else if (!bU && bL)
					{
						*piSuffix = INTEGER_SUFFIX_LL;
					}
					else
					{
						PopPosition();
						return TRIFALSE;
					}
					PassPosition();
					return TRITRUE;
				}
			}
			else
			{
				if (bU && bL)
				{
					*piSuffix = INTEGER_SUFFIX_UL;
				}
				else if (!bU && bL)
				{
					*piSuffix = INTEGER_SUFFIX_L;
				}
				else
				{
					PopPosition();
					return TRIFALSE;
				}
				PassPosition();
				return TRITRUE;
			}
		}
		else
		{
			if (bU && bL)
			{
				*piSuffix = INTEGER_SUFFIX_UL;
			}
			else if (!bU && bL)
			{
				*piSuffix = INTEGER_SUFFIX_L;
			}
			else
			{
				PopPosition();
				return TRIFALSE;
			}
			PassPosition();
			return TRITRUE;
		}

		PopPosition();
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
TRISTATE CTextParser::GetFloatSuffix(int* piSuffix, int iAllowedSuffix)
{
	char	cCurrent;

	if (!mbOutsideText)
	{
		PushPosition();

		cCurrent = mszParserPos[0];
		if ((iAllowedSuffix & FLOAT_SUFFIX_F) && ((cCurrent == 'f') || (cCurrent == 'F')))
		{
			StepRight();

			*piSuffix = FLOAT_SUFFIX_F;
			PassPosition();
			return TRITRUE;
		}

		if ((iAllowedSuffix & FLOAT_SUFFIX_D) && ((cCurrent == 'd') || (cCurrent == 'D')))
		{
			StepRight();

			*piSuffix = FLOAT_SUFFIX_D;
			PassPosition();
			return TRITRUE;
		}

		if ((iAllowedSuffix & FLOAT_SUFFIX_L) && ((cCurrent == 'l') || (cCurrent == 'L')))
		{
			StepRight();

			*piSuffix = FLOAT_SUFFIX_D;
			PassPosition();
			return TRITRUE;
		}

		PopPosition();
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

	tResult = GetDigits(pulli, piSign, piNumDigits, bSkipWhiteSpace);
	if (tResult == TRITRUE)
	{
		//Make sure there are no decimals.
		if (*mszParserPos == '.')
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
TRISTATE CTextParser::GetIntegerLiteral(unsigned long long int* pulli, int iAllowedPrefix, int* piBase, int iAllowedSuffix, int* piSuffix, int iAllowedSeparator, int* piNumDigits, BOOL bSkipWhiteSpace)
{
	char		cCurrent;
	char		cNext;
	BOOL		bFirstZero;
	int			iBase;
	BOOL		bDone;
	TRISTATE	tResult;
	int			iSuffix;
	BOOL		bSeparator;

	PushPosition();

	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
	}

	iBase = 10;
	bDone = FALSE;

	if (!mbOutsideText)
	{
		cCurrent = mszParserPos[0];

		StepRight();

		if (mbOutsideText)
		{
			SafeAssign(piSuffix, INTEGER_SUFFIX_NONE);
			return GetSingleInteger(cCurrent, pulli, piBase, piNumDigits);
		}
		else
		{
			bFirstZero = cCurrent == '0';

			cNext = mszParserPos[0];

			if (bFirstZero)
			{
				if ((cNext == 'x' || cNext == 'X') && (iAllowedPrefix & NUMBER_PREFIX_HEXADECIMAL))
				{
					iBase = 16;
				}
				else if ((cNext == 'b' || cNext == 'B') && (iAllowedPrefix & NUMBER_PREFIX_BINARY))
				{
					iBase = 2;
				}
			}

			bSeparator = (((cNext == '_') && (iAllowedSeparator & NUMBER_SEPARATOR_UNDERSCORE)) ||
			 			((cNext == '\'') && (iAllowedSeparator & NUMBER_SEPARATOR_APOSTROPHE)));

			if (!IsDigit(cNext, iBase) && iBase == 10 && !bSeparator)
			{
				iSuffix = INTEGER_SUFFIX_NONE;
				tResult = GetIntegerSuffix(&iSuffix, iAllowedSuffix);
				if (tResult == TRITRUE || tResult == TRIFALSE)
				{
					SafeAssign(piSuffix, iSuffix);
					return GetSingleInteger(cCurrent, pulli, piBase, piNumDigits);
				}
				else
				{
					PassPosition();
					return TRIFALSE;
				}
			}

			if (bFirstZero)
			{
				if (iBase == 10)
				{
					if (cNext >= '0' && cNext <= '7')
					{
						iBase = 8;
					}
				}
				else
				{
					StepRight();
				}
			}
			else
			{
				StepLeft();
			}

			tResult = GetDigits(pulli, NULL, piNumDigits, FALSE, FALSE, iBase, iAllowedSeparator);
			if (tResult == TRITRUE)
			{
				iSuffix = INTEGER_SUFFIX_NONE;
				if (!mbOutsideText)
				{
					tResult = GetIntegerSuffix(&iSuffix, iAllowedSuffix);
					if (tResult == TRIERROR)
					{
						PassPosition();
						return TRIERROR;
					}
				}

				PassPosition();
				SafeAssign(piSuffix, iSuffix);
				SafeAssign(piBase, iBase);
				return TRITRUE;
			}
			else
			{
				PopPosition();
				return tResult;
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
TRISTATE CTextParser::GetFloatLiteral(long double* pldf, int iAllowedPrefix, int* piBase, int iAllowedSuffix, int* piSuffix, int iAllowedExponent, int* piExponent, int iAllowedSeparator, int* piNumWholeDigits, int* piNumDecinalDigits, int* piNumExponentDigits, BOOL bSkipWhiteSpace)
{
	char					cCurrent;
	char					cNext;
	BOOL					bFirstZero;
	int						iBase;
	BOOL					bDone;
	TRISTATE				tResult;
	int						iSuffix;
	int						iExponent;
	BOOL					bSeparator;
	unsigned long long int	ulliWholeNumber;
	unsigned long long int	ulliDecimalNumber;
	long long int			lliExponentNumber;
	long double				ldf;
	int						iNumWholeDigits;
	int						iNumDecimalDigits;
	int						iNumExponentDigits;
	int						iSign;

	PushPosition();

	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
	}

	iBase = 10;
	bDone = FALSE;

	if (!mbOutsideText)
	{
		cCurrent = mszParserPos[0];

		StepRight();

		if (mbOutsideText)
		{
			PopPosition();
			return TRIFALSE;
		}
		else
		{
			bFirstZero = cCurrent == '0';

			cNext = mszParserPos[0];

			if (bFirstZero)
			{
				if ((cNext == 'x' || cNext == 'X') && (iAllowedPrefix & NUMBER_PREFIX_HEXADECIMAL))
				{
					iBase = 16;
					StepRight();
					cNext = mszParserPos[0];
				}
			}

			bSeparator = (((cNext == '_') && (iAllowedSeparator & NUMBER_SEPARATOR_UNDERSCORE)) ||
			 			((cNext == '\'') && (iAllowedSeparator & NUMBER_SEPARATOR_APOSTROPHE)));

			if (iBase != 16)
			{
				StepLeft();
			}

			tResult = GetDigits(&ulliWholeNumber, NULL, &iNumWholeDigits, FALSE, FALSE, iBase, iAllowedSeparator);
			if (tResult == TRITRUE)
			{
				cCurrent = mszParserPos[0];
				if (cCurrent == '.')
				{
					StepRight();

					if (mbOutsideText)
					{
						PassPosition();
						iExponent = 0;
						iNumDecimalDigits = 0;
						ulliDecimalNumber = 0;
						lliExponentNumber = 0;
						iNumExponentDigits = 0;
						iSuffix = FLOAT_SUFFIX_NONE;
						ldf = MakeLongDouble(iBase, ulliWholeNumber, ulliDecimalNumber, iNumDecimalDigits, lliExponentNumber);
						SafeAssign(pldf, ldf);
						SafeAssign(piExponent, iExponent);
						SafeAssign(piNumWholeDigits, iNumWholeDigits);
						SafeAssign(piNumDecinalDigits, iNumDecimalDigits);
						SafeAssign(piNumExponentDigits, iNumExponentDigits);
						SafeAssign(piSuffix, iSuffix);
						SafeAssign(piBase, iBase);
						return TRITRUE;
					}

					iNumDecimalDigits = 0;
					ulliDecimalNumber = 0;
					tResult = GetDigits(&ulliDecimalNumber, NULL, &iNumDecimalDigits, FALSE, FALSE, iBase, iAllowedSeparator);
					if (tResult == TRIERROR)
					{
						PassPosition();
						return TRIERROR;
					}

					iExponent = FLOAT_EXPONENT_DEFAULT;
					if (iBase == 10)
					{
						cCurrent = mszParserPos[0];
						if (cCurrent == 'E' || cCurrent == 'e')
						{
							iExponent = FLOAT_EXPONENT_DECIMAL;
							StepRight();

							if (!mbOutsideText)
							{
								tResult = GetDigits((unsigned long long int*)&lliExponentNumber, &iSign, &iNumExponentDigits, FALSE, TRUE, iBase, iAllowedSeparator);
								if (tResult == TRIERROR)
								{
									PassPosition();
									return TRIERROR;
								}
							}
							else
							{
								PassPosition();
								return TRIERROR;
							}
						}
						else
						{
							lliExponentNumber = 0;
							iNumExponentDigits = 0;
						}
					}
					else if (iBase == 16)
					{
						cCurrent = mszParserPos[0];
						if (cCurrent == 'P' || cCurrent == 'p')
						{
							iExponent = FLOAT_EXPONENT_BINARY;
							StepRight();

							if (!mbOutsideText)
							{
								tResult = GetDigits((unsigned long long int*) &lliExponentNumber, &iSign, &iNumExponentDigits, FALSE, TRUE, iBase, iAllowedSeparator);
								if (tResult == TRIERROR)
								{
									PassPosition();
									return TRIERROR;
								}
								lliExponentNumber *= iSign;
							}
							else
							{
								PassPosition();
								return TRIERROR;
							}
						}
						else
						{
							lliExponentNumber = 0;
							iNumExponentDigits = 0;
						}
					}

					iSuffix = FLOAT_SUFFIX_NONE;
					if (!mbOutsideText)
					{
						tResult = GetFloatSuffix(&iSuffix, iAllowedSuffix);
						if (tResult == TRIERROR)
						{
							PassPosition();
							return TRIERROR;
						}
					}

					PassPosition();
					ldf = MakeLongDouble(iBase, ulliWholeNumber, ulliDecimalNumber, iNumDecimalDigits, lliExponentNumber);
					SafeAssign(pldf, ldf);
					SafeAssign(piExponent, iExponent);
					SafeAssign(piNumWholeDigits, iNumWholeDigits);
					SafeAssign(piNumDecinalDigits, iNumDecimalDigits);
					SafeAssign(piNumExponentDigits, iNumExponentDigits);
					SafeAssign(piSuffix, iSuffix);
					SafeAssign(piBase, iBase);
					return TRITRUE;
				}
				else
				{
					PopPosition();
					return TRIFALSE;
				}
			}
			else
			{
				PopPosition();
				return tResult;
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
long double	CTextParser::MakeLongDouble(int iBase, unsigned long long int ulliWholeNumber, unsigned long long int ulliDecimalNumber, int iNumDecimalDigits, long long int lliExponentNumber)
{
	long double		ldf;
	long double		ldfPow;
	long double		ldfExp;

	if (iBase == 10)
	{
		ldf = (long double)ulliWholeNumber;
		ldfPow = 1.l / powl(10, iNumDecimalDigits);
		ldf += ulliDecimalNumber * ldfPow;
		if (lliExponentNumber > 0)
		{
			ldfExp = powl(10, (long double)lliExponentNumber);
			ldf *= ldfExp;
		}
		else if (lliExponentNumber < 0)
		{
			ldfExp = 1.l / powl(10, -((long double)lliExponentNumber));
			ldf *= ldfExp;
		}
		return ldf;
	}
	else if (iBase == 16)
	{
		return 0;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Cannot make double with base [", IntToString(iBase), "].");
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetSingleInteger(char cCurrent, unsigned long long int* pulli, int* piBase, int* piNumDigits)
{
	if (cCurrent >= '0' && cCurrent <= '9')
	{
		PassPosition();
		SafeAssign(pulli, cCurrent - '0');
		SafeAssign(piBase, 10);
		SafeAssign(piNumDigits, 1);
		return TRITRUE;
	}
	else
	{
		PopPosition();
		return TRIFALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetDigits(unsigned long long int* pulli, int* piSign, int* piNumDigits, BOOL bSkipWhiteSpace, BOOL bTestSign, int iBase, int iAllowedSeparator)
{
	unsigned long long int	ulliValue;
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
		ulliValue = 0;

		if (bTestSign)
		{
			GetSign(&iSign);
		}
		else
		{
			iSign = 1;
		}

		bFirstDigit = TRUE;
		BOOL bSeparator = FALSE;
		for (;;)
		{
			if (!mbOutsideText)
			{
				tReturn = GetDigit(&iTemp, iBase);
				if (tReturn == TRITRUE)
				{
					i++;
					ulliValue *= iBase;
					ulliValue += iTemp;
					bSeparator = FALSE;
				}
				else if (tReturn == TRIFALSE)
				{
					if (!bSeparator)
					{
						tReturn = GetIntegerSeparator(iAllowedSeparator);
						if ((tReturn == TRIFALSE) || (tReturn == TRIERROR))
						{
							break;
						}
						else
						{
							bSeparator = TRUE;
						}
					}
					else
					{
						break;
					}
				}
				else if (tReturn == TRIERROR)
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

		if (bSeparator)
		{
			PopPosition();
			return TRIFALSE;
		}

		if (bFirstDigit)
		{
			PopPosition();
			return TRIFALSE;
		}
		else
		{
			PassPosition();
			SafeAssign(piSign, iSign);
			SafeAssign(pulli, ulliValue);
			SafeAssign(piNumDigits, i);
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
TRISTATE CTextParser::GetIntegerSeparator(int iAllowedSeparator)
{
	char	cCurrent;

	if (!mbOutsideText)
	{
		if (iAllowedSeparator & NUMBER_SEPARATOR_APOSTROPHE)
		{
			cCurrent = mszParserPos[0];
			if (cCurrent == '\'')
			{
				StepRight();
				return TRITRUE;
			}
			else
			{
				return TRIFALSE;
			}
		}
		if (iAllowedSeparator & NUMBER_SEPARATOR_UNDERSCORE)
		{
			cCurrent = mszParserPos[0];
			if (cCurrent == '_')
			{
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
			return TRIFALSE;
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
TRISTATE CTextParser::GetHexadecimal(unsigned long long int* pulli, int* piNumDigits, BOOL bSkipWhiteSpace)
{
	TRISTATE	tReturn;

	PushPosition();

	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
	}

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
TRISTATE CTextParser::GetOctal(unsigned long long int* pulli, int* piNumDigits, BOOL bSkipWhiteSpace)
{
	unsigned long long int	iNum;
	int						iTemp;
	TRISTATE				tReturn;
	int						i;

	PushPosition();

	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
	}

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
TRISTATE CTextParser::GetFloat(float* pf, BOOL bSkipWhiteSpace)
{
	double		d;
	TRISTATE	t;

	t = GetFloat(&d, bSkipWhiteSpace);
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
TRISTATE CTextParser::GetFloat(double* pf, BOOL bSkipWhiteSpace)
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

	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
	}

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
TRISTATE CTextParser::GetNumber(CNumber* pcNumber, BOOL bSkipWhiteSpace)
{
	char*		szStart;
	double		fIgnored;
	TRISTATE	tResult;
	int			iLength;

	szStart = mszParserPos;
	tResult = GetFloat(&fIgnored, bSkipWhiteSpace);
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
	psParserState->sCurrent.Init(mszParserPos, miLine, miColumn);

	psParserState->asPrev.Copy(&masPositions);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::LoadState(SParseState* psParserState)
{
	mszParserPos = psParserState->sCurrent.szPos;
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
	char	cCurrent;
	BOOL	bInQuotes;

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
TRISTATE CTextParser::GetHFOpeningQuote(char* pcDestQuoteType)
{
	TRISTATE		tSingle;
	TRISTATE		tDouble;

	tDouble = TRIFALSE;
	tSingle = GetExactCharacter('\'');
	ReturnOnError(tSingle);
	if (tSingle == TRIFALSE)
	{
		tDouble = GetExactCharacter('"');
		ReturnErrorOnErrorAndFalse(tDouble);
		*pcDestQuoteType = '"';
		return TRITRUE;
	}
	else
	{
		*pcDestQuoteType = '\'';
		return TRITRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetHFClosingQuote(char cQuoteType)
{
	TRISTATE tResult;

	tResult = GetExactCharacter(cQuoteType);
	ReturnErrorOnErrorAndFalse(tResult);
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
	psTextPosition->Init(mszParserPos, miLine, miColumn);
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
	mszParserPos = sTextPosition.szPos;
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
	mszParserPos = psTextPosition->szPos;
	TestEnd();

	masPositions.RemoveRange(masPositions.NumElements() - iNum, masPositions.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::LastPosition(void)
{
	STextPosition* psTextPosition;

	psTextPosition = masPositions.Tail();
	miLine = psTextPosition->iLine;
	miColumn = psTextPosition->iColumn;
	mszParserPos = psTextPosition->szPos;
	TestEnd();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
STextPosition* CTextParser::GetPosition(STextPosition* psPosition)
{
	psPosition->Init(mszParserPos, miLine, miColumn);
	return psPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::SetPosition(STextPosition* psPosition)
{
	miLine = psPosition->iLine;
	miColumn = psPosition->iColumn;
	mszParserPos = psPosition->szPos;
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
void CTextParser::AppendError(CChars* pszDest)
{
	pszDest->Append("parser error [");
	if (meError == TPE_NotSet)
	{
		pszDest->Append("Not Set");
	}
	else if (meError == TPE_EndOfFile)
	{
		pszDest->Append("End of File");
	}
	else if (meError == TPE_SyntaxError)
	{
		pszDest->Append("Syntax Error");
	}
	pszDest->Append("]");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::GetText(CExternalString* pcDest)
{
	pcDest->Init(mszStartOfText, mszEndOfText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::PrintPosition(CChars* pszDest)
{
	CChars	szParserText;
	int		iPreviousLineEnd;
	int		iLineEnd;

	szParserText.Fake(mszStartOfText, 0, Length() - 1);
	iPreviousLineEnd = szParserText.FindFromEnd(UsedLength() - 1, "\n");

	iLineEnd = szParserText.Find(UsedLength(), "\n");

	if (iPreviousLineEnd == -1 && iLineEnd == -1)
	{
		PrintPositionSingleLineParser(pszDest);
	}
	else
	{
		PrintPositionMultilineParser(pszDest);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::PrintPositionSingleLineParser(CChars* pszDest)
{
	CExternalString		pac;
	int					iBefore;

	pac.Init(mszStartOfText, mszEndOfText);

	pszDest->Append(&pac);
	iBefore = mszParserPos - mszStartOfText;
	pszDest->AppendNewLine();
	pszDest->Append(' ', iBefore);
	pszDest->Append('^');
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::PrintPositionMultilineParser(CChars* pszDest)
{
	CChars				szParserLine;
	CExternalString		pac;
	CChars				szParserText;
	int					iPreviousLineEnd;
	int					iLineEnd;
	int					iParserPos;
	CChars				szLine;

	szParserText.Fake(mszStartOfText, 0, Length() - 1);
	iPreviousLineEnd = szParserText.FindFromEnd(UsedLength() - 1, "\n");

	iLineEnd = szParserText.Find(UsedLength(), "\n");

	if (iPreviousLineEnd == -1)
	{
		iPreviousLineEnd = 0;
	}
	if (iLineEnd == -1)
	{
		iLineEnd = Length();
	}

	pac.Init(&mszStartOfText[iPreviousLineEnd], &mszStartOfText[iLineEnd]);

	iParserPos = mszParserPos - mszStartOfText;

	szParserLine.Init(&pac);
	szParserLine.Replace("\n", " ");
	szParserLine.Replace("\r", "");

	szLine.Init();
	szLine.Append('[');
	szLine.Append(miLine + 1);
	szLine.Append("]: ");

	pszDest->Append(szLine);
	pszDest->Append(szParserLine);
	pszDest->AppendNewLine();
	pszDest->Append(' ', szLine.Length());
	pszDest->Append(' ', iParserPos - iPreviousLineEnd);
	pszDest->Append('^');
	pszDest->AppendNewLine();

	szLine.Kill();
	szParserLine.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextParser::IsOutside(void)
{
	return mbOutsideText;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParser::Dump(void)
{
	CChars				sz;

	sz.Init();
	PrintPosition(&sz);
	sz.DumpKill();
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetCharacterLiteral(unsigned short* pc, BOOL bAllowUTF16, int* piCharacterWidth, BOOL bSkipWhiteSpace)
{
	TRISTATE		tResult;
	int				iWidth;
	unsigned short	ui;

	PushPosition();

	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
	}

	tResult = GetExactCharacter('\'', FALSE);
	if (tResult == TRITRUE)
	{
		tResult = GetCharacterLiteral(&ui, bAllowUTF16, &iWidth);
		if (tResult == TRITRUE)
		{
			tResult = GetExactCharacter('\'', FALSE);
			if (tResult == TRITRUE)
			{
				PassPosition();
				SafeAssign(piCharacterWidth, iWidth);
				SafeAssign(pc, ui);
				return TRITRUE;
			}
			else if (tResult == TRIFALSE)
			{
				PopPosition();
				return TRIFALSE;
			}
			else
			{
				PassPosition();
				return TRIERROR;
			}
		}
		else if (tResult == TRIFALSE)
		{
			PopPosition();
			return TRIFALSE;
		}
		else
		{
			PassPosition();
			return TRIERROR;
		}
	}
	else if (tResult == TRIFALSE)
	{
		PopPosition();
		return TRIFALSE;
	}
	else
	{
		PassPosition();
		return TRIERROR;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetCharacterLiteral(unsigned short* pc, BOOL bAllowUTF16, int* piCharacterWidth)
{
	TRISTATE				tResult;
	char					c;
	char					cEscape;
	int						iWidth;
	unsigned short			ui;
	unsigned long long int	ulli;
	int						iNumDigits;

	tResult = GetCharacter(&c);
	if (tResult == TRITRUE)
	{
		if (c == '\\')
		{
			tResult = GetCharacter(&c);
			if (tResult == TRITRUE)
			{
				cEscape = ::GetEscapeCode(c);
				if (cEscape != '@' && cEscape != 0)
				{
					ui = cEscape;
					iWidth = 1;
				}
				else if (IsDigit(c, 8))
				{
					StepLeft();
					tResult = GetDigits(&ulli, NULL, &iNumDigits, FALSE, FALSE, 8, NUMBER_SEPARATOR_NONE);
					if (tResult == TRITRUE)
					{
						if (ulli < 0x100)
						{
							ui = (unsigned short)ulli;
							iWidth = 1;
						}
						else
						{
							return TRIERROR;
						}
					}
					else
					{
						return TRIERROR;
					}
				}
				else if (c == 'x')
				{
					tResult = GetDigits(&ulli, NULL, &iNumDigits, FALSE, FALSE, 16, NUMBER_SEPARATOR_NONE);
					if (tResult == TRITRUE)
					{
						if (iNumDigits >= 1 && iNumDigits <= 4)
						{
							ui = (unsigned short)ulli;
							if (ulli < 0x100)
							{
								iWidth = 1;
							}
							else if (ulli < 0x10000)
							{
								iWidth = 2;
							}
							else
							{
								return TRIERROR;
							}
						}
						else
						{
							return TRIERROR;
						}
					}
					else if (tResult == TRIFALSE)
					{
						return TRIFALSE;
					}
					else
					{
						return TRIERROR;
					}
				}
				else if (c == 'u')
				{
					tResult = GetDigits(&ulli, NULL, &iNumDigits, FALSE, FALSE, 16, NUMBER_SEPARATOR_NONE);
					if (tResult == TRITRUE)
					{
						ui = (unsigned short)ulli;
						if (iNumDigits == 4)
						{
							iWidth = 2;
						}
						else
						{
							return TRIERROR;
						}
					}
					else if (tResult == TRIFALSE)
					{
						return TRIFALSE;
					}
					else
					{
						return TRIERROR;
					}
				}
				else if ((cEscape == '@') && (c == '\n'))
				{
					return TRIGNORED;
				}
				else
				{
					return TRIERROR;
				}

				*piCharacterWidth = iWidth;
				*pc = ui;
				return tResult;
			}
			else
			{
				return tResult;
			}
		}
		else
		{
			ui = c;
			iWidth = 1;
		}

		*piCharacterWidth = iWidth;
		*pc = ui;
		return tResult;
	}
	else
	{
	return TRIERROR;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTextParser::GetStringLiteral(void* szDest, size_t uiDestByteLength, BOOL bAllowUTF16, int* piCharacterCount, int* piCharacterWidth, BOOL bSkipWhiteSpace)
{
	TRISTATE				tResult;
	int						iWidth;
	unsigned short			ui;
	int						iPosition;
	int						iOldWidth;

	PushPosition();

	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
	}

	tResult = GetExactCharacter('"', FALSE);
	iOldWidth = 0;
	if (tResult == TRITRUE)
	{
		iPosition = 0;
		for (;;)
		{
			tResult = GetExactCharacter('"', FALSE);
			if (tResult == TRITRUE)
			{
				if ((iOldWidth == 1) || (iOldWidth == 0))
				{
					((char*)(szDest))[iPosition] = '\0';
					iOldWidth = 1;
				}
				else if (iOldWidth == 2)
				{
					((unsigned short*)(szDest))[iPosition] = 0;
				}
				SafeAssign(piCharacterCount, iPosition);
				SafeAssign(piCharacterWidth, iOldWidth);

				PassPosition();
				return TRITRUE;
			}
			else if (tResult == TRIERROR)
			{
				PassPosition();
				return TRIERROR;
			}

			tResult = GetCharacterLiteral(&ui, bAllowUTF16, &iWidth);
			if (tResult == TRITRUE)
			{
				iOldWidth = ChangeWidth(iWidth, iOldWidth, szDest, uiDestByteLength, iPosition);

				if (iOldWidth == 1)
				{
					((char*)(szDest))[iPosition] = (char)ui;
				}
				else if (iOldWidth == 2)
				{
					((unsigned short*)(szDest))[iPosition] = ui;
				}

				iPosition++;
			}
			else if (tResult == TRIFALSE)
			{
				PopPosition();
				return TRIFALSE;
			}
			else
			{
				PassPosition();
				return TRIERROR;
			}
		}
	}
	else if (tResult == TRIFALSE)
	{
		PopPosition();
		return TRIFALSE;
	}
	else
	{
		PassPosition();
		return TRIERROR;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTextParser::ChangeWidth(int iWidth, int iOldWidth, void* szDest, size_t uiDestByteLength, int iLength)
{
	int		i;
	int		iByte;
	char	c;

	if (iWidth == iOldWidth)
	{
		return iWidth;
	}

	if (iLength == 0)
	{
		return iWidth;
	}

	if ((iOldWidth == 1) && (iWidth == 2))
	{
		if ((size_t)(iLength * 2) > uiDestByteLength)
		{
			return 0;
		}

		for (i = iLength - 1; i >= 0; i--)
		{
			iByte = i * 2;
			c = ((char*)szDest)[i];
			((char*)szDest)[iByte + 1] = '\0';
			((char*)szDest)[iByte] = c;
		}
		return iWidth;
	}
	else if ((iOldWidth == 2) && (iWidth == 1))
	{
		return iOldWidth;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Cannot convert string with character width [", IntToString(iOldWidth), "] to new character width [", IntToString(iWidth), "].", NULL);
		return -1;
	}
}

