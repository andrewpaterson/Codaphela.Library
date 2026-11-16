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
#ifndef __C_P_P_SYNTAX_COPIER_H__
#define __C_P_P_SYNTAX_COPIER_H__
#include "CPPSyntaxMemory.h"
#include "CPPSymbol.h"


//Copies a syntax tree: each node made again in the memory, with its fields as they are (as VisitFields gives them:
//values, tokens, scopes and links the same) and its children copied, but for the symbols Substitute says to replace,
//as each copy of a pack expansion's pattern names one of its packs' elements where the pattern names the pack.
class CCPPSyntaxCopier
{
protected:
	CCPPSyntaxMemory*	mpcMemory;
	CArrayCPPSymbols	mapcFrom;
	CArrayCPPSymbols	mapcTo;

public:
	void		Init(CCPPSyntaxMemory* pcMemory);
	void		Kill(void);

	void		Substitute(CCPPSymbol* pcFrom, CCPPSymbol* pcTo);
	CCPPSyntax*	Copy(CCPPSyntax* pcNode, CCPPSyntax* pcParent);
	CCPPSymbol*	Substituted(CCPPSymbol* pcSymbol);
};


#endif // __C_P_P_SYNTAX_COPIER_H__
