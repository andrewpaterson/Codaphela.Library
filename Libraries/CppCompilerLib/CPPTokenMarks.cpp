/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "CPPTokenMarks.h"


//////////////////////////////////////////////////////////////////////////
//
// For a unit of uiTokens tokens, none of them marked yet.
//
//////////////////////////////////////////////////////////////////////////
void CCPPTokenMarks::Init(size uiTokens)
{
	maiBefore.Init();
	maiBefore.Resize(uiTokens + 1);
	*maiBefore.Get(0) = 0;
	muiTokens = uiTokens;
	muiNext = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPTokenMarks::Kill(void)
{
	maiBefore.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The next token, from the first, is marked or isn't.
//
//////////////////////////////////////////////////////////////////////////
void CCPPTokenMarks::Next(bool bMarked)
{
	int32*	paiBefore;

	if (muiNext < muiTokens)
	{
		paiBefore = maiBefore.GetData();
		paiBefore[muiNext + 1] = paiBefore[muiNext] + (bMarked ? 1 : 0);
		muiNext++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a token from iFirstToken to iLastToken (which is one of them) is
// marked.  True for a run that isn't one of the unit's tokens (a node made
// with no tokens of its own has -1), or before every token has been told
// of: what is not known to have none is looked at.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokenMarks::Any(int32 iFirstToken, int32 iLastToken) const
{
	int32*	paiBefore;

	if ((muiNext < muiTokens) || (iFirstToken < 0) || (iLastToken < iFirstToken) || ((size)iLastToken >= muiTokens))
	{
		return true;
	}
	paiBefore = maiBefore.GetData();
	return paiBefore[iLastToken + 1] != paiBefore[iFirstToken];
}


//////////////////////////////////////////////////////////////////////////
//
// How many of the tokens told of are marked.
//
//////////////////////////////////////////////////////////////////////////
size CCPPTokenMarks::NumMarked(void) const
{
	return (size)maiBefore.GetData()[muiNext];
}
