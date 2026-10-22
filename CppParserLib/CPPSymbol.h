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
#ifndef __C_P_P_SYMBOL_H__
#define __C_P_P_SYMBOL_H__
#include "BaseLib/PrimitiveTypes.h"


enum ECPPSymbolKind
{
	CPPSK_Namespace,
	CPPSK_Class,
	CPPSK_Enum,
	CPPSK_Typedef,
	CPPSK_ClassTemplate,
	CPPSK_AliasTemplate,
	CPPSK_TypeParameter,
	CPPSK_Function,
	CPPSK_FunctionTemplate,
	CPPSK_Variable,
	CPPSK_Field,
	CPPSK_Parameter,
	CPPSK_ValueParameter,		//A non-type template parameter.
	CPPSK_Enumerator,
	CPPSK_Label,
	CPPSK_BuiltinType,			//Compiler types with identifier names, like __builtin_va_list.
};


class CCPPScope;
class CCPPSyntax;


//A declared name.  Names are resolved while parsing so the parser can tell types from values.  [basic.lookup]
class CCPPSymbol
{
public:
	int64			mlliName;
	ECPPSymbolKind	meKind;
	CCPPSyntax*		mpcDeclaration;		//The first declaration.
	CCPPScope*		mpcScope;			//The scope the name is declared in.
	CCPPScope*		mpcMembers;			//The members of a namespace, class or enum, or of the class a typedef names.
	CCPPSymbol*		mpcNext;			//Another declaration of the name in the same scope, such as an overload.

	void	Init(int64 lliName, ECPPSymbolKind eKind, CCPPSyntax* pcDeclaration, CCPPScope* pcScope);
	bool	IsType(void);
	bool	IsTemplate(void);
	bool	HasTemplate(void);
	bool	IsNamespace(void);
	bool	IsClass(void);
	bool	IsFunction(void);
	bool	HasMembers(void);
	char*	GetKindName(void);
};


#endif // __C_P_P_SYMBOL_H__

