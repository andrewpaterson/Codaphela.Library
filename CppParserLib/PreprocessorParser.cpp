/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "BaseLib/ErrorHandler.h"
#include "PreprocessorParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorParser::Init(char* szStart, char* szEnd)
{
	int		iLen;

	mszPos = szStart;
	mszStart = mszPos;
	if (!szEnd)
	{
		iLen = (int)strlen(szStart);
		mszEnd = szStart+iLen-1;
	}
	else
	{
		mszEnd = szEnd;
	}
	TestEnd();
	miLine = 0;
	miColumn = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorParser::StepRight(void)
{
	if (IsNewLine())
	{
		SkipNewLine();
	}
	else
	{
		miColumn++;
		mszPos++;
	}
	TestEnd();
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorParser::StepLeft(void)
{
	if (IsNewLine())
	{
		BackupNewLine();
	}
	else
	{
		miColumn--;
		mszPos--;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorParser::StepRight(int iDist)
{
	int  i;

	for (i = 0; i < iDist; i++)
	{
		StepRight();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorParser::SkipWhiteSpace(void)
{
	char	cCurrent;		

	for (;;)
	{
		if (mbEndOfFile)
		{
			return;
		}

		cCurrent = mszPos[0];

		//Nice clean white space...
		if (IsWhiteSpace())
		{
			StepRight();
		}

		//Possibly nasty comments...
		else if (cCurrent == '/')
		{
			StepRight();

			if (!mbEndOfFile)
			{
				cCurrent = mszPos[0];
				if (cCurrent == '*')
				{
					StepRight();
					SkipCStyleComment();
				}
				else if (cCurrent == '/')
				{
					StepRight();
					SkipCPPStyleComment();
				}
				else
				{
					mszPos--;
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
void CPreprocessorParser::SkipCStyleComment(void)
{
	char	cCurrent;		
	int		iDepth;

	//It's assumed that the initial /* has already been consumed.
	iDepth = 1;

	for (;;)
	{
		if (mbEndOfFile)
		{
			return;
		}

		cCurrent = mszPos[0];
		if (cCurrent == '/')
		{
			StepRight();
			TestEnd();
			if (!mbEndOfFile)
			{
				cCurrent = mszPos[0];
				if (cCurrent == '*')
				{
					iDepth++;
				}
			}
			else
			{
				return;
			}
		}
		else if (cCurrent == '*')
		{
			StepRight();
			TestEnd();
			if (!mbEndOfFile)
			{
				cCurrent = mszPos[0];
				if (cCurrent == '/')
				{
					StepRight();
					iDepth--;
				}
			}
			else
			{
				return;
			}
		}
		else
		{
			StepRight();
		}

		if (iDepth == 0)
		{
			return;
		}
	}
	return;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorParser::SkipCPPStyleComment(void)
{
	char	cCurrent;
	int		iCharNum;

	if (mbEndOfFile)
	{
		return;
	}

	cCurrent = mszPos[0];
	iCharNum = 0;
	while (!mbEndOfFile)
	{
		cCurrent = mszPos[0];

		if ((cCurrent == '\n') || (cCurrent == '\r'))
		{
			//Don't consume the newline.
			return;
		}

		StepRight();
		iCharNum++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorParser::TestEnd(void)
{
	if (mszPos <= mszEnd)
	{
		mbEndOfFile = FALSE;
	}
	else
	{
		mbEndOfFile = TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorParser::IsWhiteSpace(void)
{
	char c;
	c = *mszPos;
	return (c == ' ') || (c >= 128) || (c >= 1 && c <= 9) || (c == 11) || (c == 12);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorParser::IsFirstIdentifier(void)
{
	char c;

	c = *mszPos;
	if ((c >= 'A') && (c <= 'Z'))
	{
		return TRUE;
	}

	if ((c >= 'a') && (c <= 'z'))
	{
		return TRUE;
	}

	if (c == '_')
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorParser::IsDigit(void)
{
	if (((*mszPos) >= '0') && ((*mszPos) <= '9'))
	{
		return TRUE;
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorParser::SkipNewLine(void)
{
	if (!mbEndOfFile)
	{
		if ((*mszPos) == '\n')
		{
			mszPos++;
			if ((*mszPos) == '\r')
			{
				mszPos++;
			}
			miLine++;
			miColumn = 0;
		}
		else if ((*mszPos) == '\r')
		{
			mszPos++;
			if ((*mszPos) == '\n')
			{
				mszPos++;
			}
			miLine++;
			miColumn = 0;
		}
	}
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CPreprocessorParser::CalculateColumn(void)
{
	char*	sz;
	int		iCount;
	BOOL	bAny;

	iCount = 0;
	sz = mszPos;
	bAny = FALSE;
	while ((*sz != '\r') && (*sz != '\n') && (sz != mszStart))
	{
		sz--;
		iCount++;
		bAny = TRUE;
	}

	if (bAny)
	{
		iCount--;
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorParser::BackupNewLine(void)
{
	if ((*mszPos) == '\n')
	{
		mszPos--;
		if ((*mszPos) == '\r')
		{
			mszPos--;
		}
		miLine--;
		miColumn = CalculateColumn();
	}
	else if ((*mszPos) == '\r')
	{
		mszPos--;
		if ((*mszPos) == '\n')
		{
			mszPos--;
		}
		miLine--;
		miColumn = CalculateColumn();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorParser::IsNewLine(void)
{
	if (((*mszPos) == '\n') || ((*mszPos) == '\r'))
	{
		return TRUE;
	}
	return FALSE;
}
