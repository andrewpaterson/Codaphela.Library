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
#ifndef __C_P_P_SCOPE_H__
#define __C_P_P_SCOPE_H__
#include "BaseLib/ArrayInt.h"
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
//What a lookup considers.  Ordinary lookup finds a variable, function or enumerator that hides a class or enum of the
//name in its scope [basic.scope.hiding] 2; a type's (an elaborated type specifier's, and a name before ::) only
//namespaces, types and templates [basic.lookup.elab] 2, [basic.lookup.qual] 1; a namespace's (a using-directive's and
//a namespace alias's) only namespaces [basic.lookup.udir] 1.
enum ECPPLookup
{
	CPPL_Ordinary,
	CPPL_Type,
	CPPL_Namespace,
};


typedef CArrayTemplatePtr<CCPPScope>		CArrayScopes;
typedef CMapLongTemplate<CCPPSymbol*>		CMapSymbols;


//using N::F;: lookup of the name here finds N's functions of the name declared before the using-declaration (its first
//token, iBefore), as well as the scope's own.  [namespace.udecl] 11
struct SCPPScopeUsedFunction
{
	int64		lliName;
	CCPPScope*	pcNamespace;
	int			iBefore;
};


typedef CArrayTemplate<SCPPScopeUsedFunction>	CArrayScopeUsedFunctions;


//The namespaces whose functions of a name a call can choose between (CCPPScope::GatherOverloadScopes).  One whose
//functions only a using-declaration brings has those declared before the declaration's first token (its limit, -1 for all
//of a namespace's).  [namespace.udecl] 11
class CCPPOverloadScopes
{
protected:
	CArrayScopes	mapcScopes;
	CArrayInt		maiBefore;

public:
	void		Init(void);
	void		Kill(void);

	void		Add(CCPPScope* pcScope);
	void		AddBefore(CCPPScope* pcScope, int iBefore);
	size		NumElements(void) const;
	CCPPScope*	GetPtr(size i);
	void		SetPtr(size i, CCPPScope* pcScope);
	bool		Has(CCPPScope* pcScope);
	bool		Takes(CCPPScope* pcNamespace, int iToken);
};


class CCPPScope
{
public:
	ECPPScopeKind	meKind;
	CCPPScope*		mpcParent;
	CCPPSymbol*		mpcOwner;			//The namespace, class or enum the scope belongs to.
	CMapSymbols		mcSymbols;
	CArrayScopes	mapcBases;			//The scopes of a class's base classes.
	CArrayScopes	mapcUsings;			//Namespaces nominated by using directives (and inline and unnamed namespaces).
	CArrayScopeUsedFunctions	masUsedFunctions;	//Functions using declarations name that the scope has functions of the name too.
	CCPPScope*		mpcQualifier;		//A member function defined outside its class: the class is searched before the parent.
	bool			mbDependent;		//A class whose bases or members depend on template parameters, so lookup can fail.
	bool			mbControl;			//An if's or a for's own scope: what its condition or first part declares.

	void			Init(ECPPScopeKind eKind, CCPPScope* pcParent, CCPPSymbol* pcOwner, CMallocator* pcMalloc = NULL);
	void			Kill(void);

	void			Add(CCPPSymbol* pcSymbol);
	void			AddAlias(int64 lliName, CCPPSymbol* pcSymbol, int iBefore);
	SCPPScopeUsedFunction*	FindUsedFunctions(int64 lliName, CCPPScope* pcNamespace);
	CCPPSymbol*		FindHere(int64 lliName);
	CCPPSymbol*		FindParsed(int64 lliName);
	void			LoadLazy(int64 lliName);
	CCPPSymbol*		FindMember(int64 lliName, ECPPLookup eLookup = CPPL_Ordinary);
	CCPPSymbol*		Find(int64 lliName, ECPPLookup eLookup = CPPL_Ordinary);
	CCPPSymbol*		FindDeclared(int64 lliName, int iDepth, ECPPLookup eLookup = CPPL_Ordinary);
	CCPPSymbol*		FindNominated(int64 lliName, CCPPScope* pcLevel, CArrayCPPSymbols* papcFound, ECPPLookup eLookup = CPPL_Ordinary);
	CCPPSymbol*		Pick(CCPPSymbol* pcFirst, ECPPLookup eLookup);
	bool			IsAmbiguous(int64 lliName);
	bool			IsAmbiguousMember(int64 lliName);
	static bool		AreDifferent(CArrayCPPSymbols* papcFound);
	CCPPScope*		NominationLevel(CCPPScope* pcNominated);
	CCPPScope*		GetMemberFunctionClass(void);
	CCPPScope*		GetDeclarationScope(void);
	CCPPScope*		GetEnclosingNamespace(void);
	CCPPScope*		GetNamespace(void);
	bool			IsUnnamedNamespace(void) const;
	bool			IsInlineNamespace(void);
	CCPPScope*		FindUnnamedNamespace(void);
	bool			IsInUnnamedNamespace(void);
	bool			IsDependent(void);
	bool			Encloses(CCPPScope* pcScope);
	void			GatherOverloadScopes(int64 lliName, bool bQualified, CCPPOverloadScopes* pcNamespaces);
	void			GatherEnclosingNamespaces(CCPPOverloadScopes* pcNamespaces);

protected:
	CCPPSymbol*		FindMember(int64 lliName, int iDepth, ECPPLookup eLookup);
	bool			AddOverloadScope(int64 lliName, CCPPOverloadScopes* pcNamespaces);
	void			AddNominated(CArrayScopes* papcNominated);
	bool			GatherUnqualified(int64 lliName, CArrayScopes* papcNominated, CCPPOverloadScopes* pcNamespaces);
	bool			GatherQualified(int64 lliName, CCPPOverloadScopes* pcNamespaces, int iDepth);
};


#endif // __C_P_P_SCOPE_H__

