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
#include "CPPSymbol.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSymbol::Init(int64 lliName, ECPPSymbolKind eKind, CCPPSyntax* pcDeclaration, CCPPScope* pcScope)
{
	mlliName = lliName;
	meKind = eKind;
	mpcDeclaration = pcDeclaration;
	mpcScope = pcScope;
	mpcMembers = NULL;
	mpcNext = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSymbol::IsType(void)
{
	return (meKind == CPPSK_Class) || (meKind == CPPSK_Enum) || (meKind == CPPSK_Typedef) || (meKind == CPPSK_ClassTemplate) || (meKind == CPPSK_AliasTemplate) || (meKind == CPPSK_TypeParameter) || (meKind == CPPSK_BuiltinType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSymbol::IsTemplate(void)
{
	return (meKind == CPPSK_ClassTemplate) || (meKind == CPPSK_FunctionTemplate) || (meKind == CPPSK_AliasTemplate);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSymbol::HasTemplate(void)
{
	CCPPSymbol*	pcSymbol;

	//An overloaded name is a template name if any of its declarations is a template.  [temp.names] 3
	for (pcSymbol = this; pcSymbol; pcSymbol = pcSymbol->mpcNext)
	{
		if (pcSymbol->IsTemplate())
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSymbol::IsNamespace(void)
{
	return meKind == CPPSK_Namespace;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSymbol::IsClass(void)
{
	return (meKind == CPPSK_Class) || (meKind == CPPSK_ClassTemplate);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSymbol::IsFunction(void)
{
	return (meKind == CPPSK_Function) || (meKind == CPPSK_FunctionTemplate);
}


