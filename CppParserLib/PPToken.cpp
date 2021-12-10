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
#include "PPToken.h"


int64		giTokenNum = 0;
int64		giTokenBreak = -1;

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetBreakOnTokenInit(int64 iTokenNum)
{
	giTokenBreak = iTokenNum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPToken::Init(int iLine, int iColumn)
{
	Set(iLine, iColumn);
	miUsage = 0;

#ifdef DEBUG
	miTokenNum = giTokenNum;

	if (miTokenNum == giTokenBreak)
	{
		gcUserError.Set("Break on token.");
	}

	giTokenNum++;
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
BOOL CPPToken::IsDirective(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPToken::IsUnknown(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPToken::IsText(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPToken::IsWhiteSpace(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPToken::IsReplacement(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPToken::IsHash(void)
{
	return FALSE;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPToken::IsTokenReplacementsHolder(void)
{
	return FALSE;
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPToken::IsLine(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPToken::IsFile(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPToken::IsBlock(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPToken::IsTokenListHolder(void)
{
	return FALSE;
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
BOOL CPPToken::NeedsNewLine(void)
{
	return FALSE;
}

