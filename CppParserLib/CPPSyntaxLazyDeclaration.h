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
#ifndef __C_P_P_SYNTAX_LAZY_DECLARATION_H__
#define __C_P_P_SYNTAX_LAZY_DECLARATION_H__
#include "CPPSyntaxDeclaration.h"


class CCPPScope;


//A header declaration that has not been parsed (CCPPHeaderIndex): its names are lazy
//symbols, and the first lookup of one of them parses it (CCPPParser::LoadLazy), replacing
//this node in mpapcOwner with the declaration.  A member function defined outside its
//class declares no name: it waits in the class's mpcMembers and is parsed after the
//class.  An unloaded one generates nothing.
class CCPPSyntaxLazyDeclaration : public CCPPSyntaxDeclaration
{
public:
	CArraySyntax*		mpapcOwner;		//The declarations it is in, at muiIndex.
	size				muiIndex;
	CCPPScope*			mpcScope;		//The namespace scope it was skipped in.
	int					miPack;			//The #pragma pack where it was skipped.
	bool				mbLoaded;

	CCPPSyntaxLazyDeclaration*	mpcMembers;		//Member functions of this class defined outside it (later), parsed after it.
	CCPPSyntaxLazyDeclaration*	mpcNextMember;

	void				Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent) override;
	ECPPSyntax			GetKind(void) override;
	void				PrettyPrint(CChars* pszDest, int iDepth) override;
};


#endif // __C_P_P_SYNTAX_LAZY_DECLARATION_H__
