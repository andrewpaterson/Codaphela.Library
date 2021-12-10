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
#include "BaseLib/ExternalString.h"
#include "BaseLib/Chars.h"
#include "PreprocessorParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorParser::Init(char* szStart, char* szEnd, char* szFileName)
{
	int		iLen;

	mszParserPos = szStart;
	mszStartOfText = mszParserPos;
	if (!szEnd)
	{
		iLen = (int)strlen(szStart);
		mszEndOfText = szStart+iLen-1;
	}
	else
	{
		mszEndOfText = szEnd;
	}
	TestEnd();
	miLine = 0;
	miColumn = 0;
	mszFileName = szFileName;
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
		mszParserPos++;
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
		mszParserPos--;
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

		cCurrent = mszParserPos[0];

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
				cCurrent = mszParserPos[0];
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
					mszParserPos--;
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

		cCurrent = mszParserPos[0];
		if (cCurrent == '/')
		{
			StepRight();
			TestEnd();
			if (!mbEndOfFile)
			{
				cCurrent = mszParserPos[0];
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
				cCurrent = mszParserPos[0];
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

	cCurrent = mszParserPos[0];
	iCharNum = 0;
	while (!mbEndOfFile)
	{
		cCurrent = mszParserPos[0];

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
	if (mszParserPos <= mszEndOfText)
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
	c = *mszParserPos;
	return (c == ' ') || (c >= 128) || (c >= 1 && c <= 9) || (c == 11) || (c == 12);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorParser::IsFirstIdentifier(void)
{
	char c;

	c = *mszParserPos;
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
	if (((*mszParserPos) >= '0') && ((*mszParserPos) <= '9'))
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
		if ((*mszParserPos) == '\n')
		{
			mszParserPos++;
			if ((*mszParserPos) == '\r')
			{
				mszParserPos++;
			}
			miLine++;
			miColumn = 0;
		}
		else if ((*mszParserPos) == '\r')
		{
			mszParserPos++;
			if ((*mszParserPos) == '\n')
			{
				mszParserPos++;
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
	sz = mszParserPos;
	bAny = FALSE;
	while ((*sz != '\r') && (*sz != '\n') && (sz != mszStartOfText))
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
	if ((*mszParserPos) == '\n')
	{
		mszParserPos--;
		if ((*mszParserPos) == '\r')
		{
			mszParserPos--;
		}
		miLine--;
		miColumn = CalculateColumn();
	}
	else if ((*mszParserPos) == '\r')
	{
		mszParserPos--;
		if ((*mszParserPos) == '\n')
		{
			mszParserPos--;
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
	if (((*mszParserPos) == '\n') || ((*mszParserPos) == '\r'))
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPreprocessorParser::GetPos(void)
{
	return mszParserPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPreprocessorParser::GetEnd(void)
{
	return mszEndOfText;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorParser::Dump(void)
{
	CChars				sz;
	CExternalString		pac;
	int					iBefore;

	pac.Init(mszStartOfText, mszEndOfText);
	sz.Init(&pac);

	iBefore = mszParserPos - mszStartOfText;
	sz.AppendNewLine();
	sz.Append(' ', iBefore);
	sz.Append('^');
	sz.AppendNewLine();

	sz.DumpKill();
}

