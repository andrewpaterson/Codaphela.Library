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
#ifndef __C_P_P_TOKEN_MARKS_H__
#define __C_P_P_TOKEN_MARKS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/ArrayInt32.h"


//Some of a unit's tokens, marked, so that whether any of a run of tokens is marked is two reads: for each token, how
//many of those before it are.  A syntax node's tokens are a run (miFirstToken to miLastToken) that holds its
//children's, so a visitor looking for what a marked token starts needn't enter a node that has none: it walks only
//the ways down to them, not the whole tree (CCPPUnit::ReportNamedInstances).
//
//Init, then Next for each token from the first, in order; then Any.
class CCPPTokenMarks
{
protected:
	CArrayInt32	maiBefore;		//For each token, the marked tokens before it; and one more, for the end.
	size		muiTokens;		//The tokens there are...
	size		muiNext;		//...and how many Next has been told of.

public:
	void	Init(size uiTokens);
	void	Kill(void);

	void	Next(bool bMarked);
	bool	Any(int32 iFirstToken, int32 iLastToken) const;
	size	NumMarked(void) const;
};


#endif // __C_P_P_TOKEN_MARKS_H__
