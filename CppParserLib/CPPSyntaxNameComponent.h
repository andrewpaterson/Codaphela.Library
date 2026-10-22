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
#ifndef __C_P_P_SYNTAX_NAME_COMPONENT_H__
#define __C_P_P_SYNTAX_NAME_COMPONENT_H__
#include "CPPSyntax.h"


class CCPPSyntaxTypeId;


class CCPPSyntaxNameComponent : public CCPPSyntax
{
public:
	ECPPNameComponent		meKind;
	bool					mbTemplateKeyword;
	int32					miName;
	bool					mbTemplateArguments;
	CArraySyntax			macpTemplateArguments;
	CCPPSyntaxTypeId*		mpcConversionType;
	CCPPSymbol*				mpcSymbol;

	void					Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent) override;
	void					Kill(void) override;
	char*					GetType(void) override;
	ECPPSyntax				GetKind(void) override;
	void					PrettyPrint(CChars* pszDest, int iDepth) override;
	size					NumChildren(void) override;
	CCPPSyntax*				GetChild(size iIndex) override;
};


#endif // __C_P_P_SYNTAX_NAME_COMPONENT_H__

