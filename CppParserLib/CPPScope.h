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
#ifndef __C_P_P_SCOPE_H__
#define __C_P_P_SCOPE_H__
#include "BaseLib/MapLongTemplate.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "CPPSymbol.h"


enum ECPPScopeKind
{
	CPPSC_Namespace,
	CPPSC_Class,
	CPPSC_Enum,
	CPPSC_Function,		//The parameters, and the outermost block of a function definition.
	CPPSC_Block,
	CPPSC_Template,		//Template parameters.
};


class CCPPScope;
typedef CArrayTemplatePtr<CCPPScope>		CArrayScopes;
typedef CMapLongTemplate<CCPPSymbol*>		CMapSymbols;


class CCPPScope
{
public:
	ECPPScopeKind	meKind;
	CCPPScope*		mpcParent;
	CCPPSymbol*		mpcOwner;			//The namespace, class or enum the scope belongs to.
	CMapSymbols		mcSymbols;
	CArrayScopes	mapcBases;			//The scopes of a class's base classes.
	CArrayScopes	mapcUsings;			//Namespaces nominated by using directives (and inline and unnamed namespaces).
	CCPPScope*		mpcQualifier;		//A member function defined outside its class: the class is searched before the parent.
	bool			mbDependent;		//A class whose bases or members depend on template parameters, so lookup can fail.

	void			Init(ECPPScopeKind eKind, CCPPScope* pcParent, CCPPSymbol* pcOwner);
	void			Kill(void);

	void			Add(CCPPSymbol* pcSymbol);
	void			AddAlias(int64 lliName, CCPPSymbol* pcSymbol);
	CCPPSymbol*		FindHere(int64 lliName);
	void			LoadLazy(int64 lliName);
	CCPPSymbol*		FindMember(int64 lliName);
	CCPPSymbol*		Find(int64 lliName);
	CCPPScope*		GetDeclarationScope(void);
	CCPPScope*		GetEnclosingNamespace(void);
	bool			IsDependent(void);

protected:
	CCPPSymbol*		FindMember(int64 lliName, int iDepth);
};


#endif // __C_P_P_SCOPE_H__

