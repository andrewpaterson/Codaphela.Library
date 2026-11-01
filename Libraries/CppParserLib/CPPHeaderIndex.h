/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __C_P_P_HEADER_INDEX_H__
#define __C_P_P_HEADER_INDEX_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/MapLongTemplate.h"


//A header declaration seen in a unit: its tokens and the names it declares in its
//namespace.  bLazy if later units may leave it unparsed until one of its names is used.
struct SCPPHeaderDeclaration
{
	uint32		uiTokens;
	uint64		uiHash;
	bool		bLazy;
	CArrayChars	aszNames;
	CArrayInt	aiKinds;		//ECPPSymbolKind of each name.
	CChars		szClass;		//A member function defined outside its class (which is in the same namespace): the class.

	SCPPHeaderDeclaration*	psNext;		//Another declaration starting at the same place (a header included again with other macros).
};


//The header declarations the units compiled so far parsed, by where they start (the
//header and the line and column of their first token).  A later unit whose tokens there
//are the same (a header can be included with different macros, so there may be more than
//one declaration at a place) can skip the declaration and declare its names as lazy
//symbols (CCPPParser).  Kept by the builder for all its units.
class CCPPHeaderIndex
{
protected:
	CMapLongTemplate<SCPPHeaderDeclaration*>		mcDeclarations;
	CArrayTemplatePtr<SCPPHeaderDeclaration>		mapsDeclarations;

public:
	void					Init(void);
	void					Kill(void);

	SCPPHeaderDeclaration*	Get(uint64 uiKey);	//The first declaration starting there; the rest follow psNext.
	SCPPHeaderDeclaration*	Add(uint64 uiKey);
	size					NumDeclarations(void);

	static uint64			Key(const char* szFile, int iLine, int iColumn);
};


#endif // __C_P_P_HEADER_INDEX_H__
