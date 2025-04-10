/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "BaseLib/Validation.h"
#include "PPToken.h"


int64		giTokenNum = 0;
int64		giTokenBreak = -1;

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PPSetBreakOnTokenInit(int64 iTokenNum)
{
	giTokenBreak = iTokenNum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPToken::Init(int iLine, int iColumn, char* szFileName)
{
	Set(iLine, iColumn);
	miUsage = 0;

#ifdef DEBUG
	miTokenNum = giTokenNum;

	if (miTokenNum == giTokenBreak)
	{
		BREAK();
	}

	giTokenNum++;

	mszFileName = szFileName;
#endif // DEBUG
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPToken::Unuse(void)
{
	miUsage--;
	if (miUsage == 0)
	{
		Kill();
	}
	else if (miUsage < 0)
	{
		gcUserError.Set("Usage went negative.");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPToken::Use(void)
{
	if (miUsage < 0)
	{
		gcUserError.Set("Usage started negative.");
	}
	miUsage++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPToken::Set(int iLine, int iColumn)
{
	miLine = iLine;
	miColumn = iColumn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPPToken::ShortFileName(void)
{
#ifdef DEBUG
	return mszFileName;
#else
	return gszEmptyString;
#endif // DEBUG
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPToken::IsDirective(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPToken::IsUnknown(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPToken::IsText(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPToken::IsWhitespace(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPToken::IsReplacement(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPToken::IsHash(void)
{
	return false;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPToken::IsTokenReplacementsHolder(void)
{
	return false;
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPToken::IsLine(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPToken::IsFile(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPToken::IsBlock(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPToken::IsTokenListHolder(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CPPToken::Line(void)
{
	return miLine;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CPPToken::Column(void)
{
	return miColumn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPToken::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPToken::NeedsNewLine(void)
{
	return false;
}

