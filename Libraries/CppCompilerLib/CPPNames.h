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
#ifndef __C_P_P_NAMES_H__
#define __C_P_P_NAMES_H__
#include "BaseLib/PrimitiveTypes.h"
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"
#include "CPPClass.h"


class CCPPUnit;
class CCPPBinder;
class CCPPDiagnostics;
class CCPPDeclarators;
class CCPPContext;
class CCPPTemplate;


//What a name means as a class or as a class's member (CCPPBinder::GetNames): the class a name, its qualifiers or one
//of its components names, the member a qualified name names with the class that declares it, and whether lookup in a
//class finds a name as more than one thing.  A class is worked out as the parser found it, or where the parser
//couldn't (in a template), by walking the name's qualifiers.  [basic.lookup.qual], [class.qual], [class.member.lookup]
class CCPPNames
{
protected:
	CCPPUnit*			mpcUnit;
	CCPPBinder*			mpcBinder;
	CCPPDiagnostics*	mpcDiagnostics;
	CCPPDeclarators*	mpcDeclarators;
	CCPPContext*		mpcContext;

public:
	void			Init(CCPPUnit* pcUnit, CCPPBinder* pcBinder);
	void			Kill(void);

	CCPPClass*		GetNameClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode);
	CCPPClass*		NameQualifierClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode);
	CCPPClass*		QualifiersClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode);
	CCPPClass*		QualifiedClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode);
	CCPPClass*		GetComponentClass(CCPPSyntaxNameComponent* pcComponent, CCPPSymbol* pcSymbol, CCPPSyntax* pcNode, CCPPClass* pcContext = NULL);
	CCPPScope*		ComponentLookup(CCPPSyntaxNameComponent* pcComponent);
	CCPPClass*		EnclosingInstance(CCPPSymbol* pcSymbol);
	CCPPClass*		MemberScopeClass(CCPPSymbol* pcSymbol);
	CCPPClass*		QualifierClass(CCPPClass* pcClass, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode);

	CCPPSymbol*		QualifiedMember(CCPPSyntaxName* pcName, ECPPSymbolKind eKind, CCPPSyntax* pcNode, CCPPClass** ppcClass);
	CCPPSymbol*		QualifiedEnumerator(CCPPSyntaxName* pcName, CCPPSyntax* pcNode, CCPPClass** ppcClass);
	bool			IsAmbiguousName(CCPPSyntaxName* pcName);
	bool			HasMember(CCPPClass* pcClass, int64 lliName);
	bool			HasOperator(CCPPClass* pcClass, int64 lliName, size uiArguments);
	bool			IsAmbiguousInBases(CCPPClass* pcClass, int64 lliName);
	bool			CheckBaseMemberName(CCPPSyntaxName* pcName, CCPPSyntax* pcNode);

protected:
	CCPPClass*		MemberClass(CCPPClass* pcClass, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode);
	CCPPClass*		FindBaseInstance(CCPPClass* pcClass, CCPPTemplate* pcTemplate);
	CCPPSymbol*		DecltypeEnumerator(CCPPSyntaxName* pcName);
	void			FindInBases(CCPPClass* pcClass, int64 lliName, CArrayCPPClasses* papcFound);
};


#endif // __C_P_P_NAMES_H__

