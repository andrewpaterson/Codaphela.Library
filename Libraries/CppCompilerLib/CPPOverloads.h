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
#ifndef __C_P_P_OVERLOADS_H__
#define __C_P_P_OVERLOADS_H__
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"
#include "CPP65Type.h"
#include "CPPClass.h"
#include "CPPFunction.h"
#include "CPPTemplate.h"


class CCPP65Generator;


//Chooses the function a call means.  [over.match]  The candidates are the functions of a name that a class, the
//namespaces in reach or the arguments' own namespaces have, and the function templates among them whose arguments
//can be deduced; each is ranked by the conversions its parameters need of the arguments, and the best is the
//answer, or none is if two are as good as each other.  It asks the unit what a name is and where the compiler
//is; it keeps nothing itself.
class CCPPOverloads
{
protected:
	CCPP65Generator*		mpcUnit;

public:
	void					Init(CCPP65Generator* pcUnit);
	void					Kill(void);

	CCPPTemplate*			FindMemberTemplate(CCPPClass* pcClass, int64 lliName, size uiArguments, int iConstObject = -1);
	CCPPTemplate*			ChooseMemberTemplate(CCPPClass* pcClass, int64 lliName, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcExplicit, int iConstObject);
	void					GetMemberTemplates(CCPPClass* pcClass, int64 lliName, size uiArguments, CArrayCPPTemplates* papcCandidates);
	CCPPTemplate*			GetCallTemplate(CCPPSymbol* pcSymbol, int64 lliName, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcExplicit);
	void					AddCallTemplates(CCPPSymbol* pcSymbol, CCPPSyntaxCallExpression* pcCall, CArrayCPPTemplates* papcCandidates);
	CCPPTemplate*			ChooseTemplate(CArrayCPPTemplates* papcCandidates, int iConstObject, CArraySyntax* papcExplicit, CCPPSyntaxCallExpression* pcCall);
	bool					TemplateTakes(CCPPTemplate* pcTemplate, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode, int* aiRanks);
	bool					TakesArguments(CCPPSyntaxFunctionSuffix* pcSuffix, size uiArguments);
	bool					IsOverloadOf(CCPPClass* pcClass, CCPPFunction* pcFunction, int64 lliName);
	CCPPFunction*			FindFunction(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, CArraySyntax* papcArguments, bool bBases, CCPPSyntax* pcNode, int iConstObject = -1, CArrayScopes* papcNamespaces = NULL);
	CCPPFunction*			FindNamedFunction(CCPPSyntaxName* pcName, int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	CCPPFunction*			FindAddressedFunction(CCPPSyntaxName* pcName, int64 lliName, SCPP65Type* psTarget, CCPPSyntax* pcNode, bool* pbOverloaded);
	CCPPFunction*			FindAssociatedFunction(CCPPSyntaxName* pcName, int64 lliName, CCPPSyntaxCallExpression* pcCall);
	void					GetOverloadNamespaces(CCPPSyntaxName* pcName, int64 lliName, CArrayScopes* papcNamespaces);
	void					GetCallNamespaces(CCPPSyntaxName* pcName, int64 lliName, CArraySyntax* papcArguments, CArrayScopes* papcNamespaces);
	void					AddArgumentNamespaces(CArraySyntax* papcArguments, CArrayScopes* papcNamespaces);
	void					AddAssociatedNamespaces(SCPP65Type* psType, CArrayScopes* papcNamespaces, int iDepth);
	void					AddNamespaceOf(CCPPScope* pcScope, CArrayScopes* papcNamespaces);
	void					NormaliseNamespaces(CArrayScopes* papcNamespaces);
	bool					RankArguments(CCPPFunction* pcFunction, SCPP65Type** apsArguments, CArraySyntax* papcArguments, int* aiRanks);
	int						ConversionRank(SCPP65Type* psParameter, SCPP65Type* psArgument, CCPPSyntax* pcArgument, bool bUserDefined = true);
	bool					HasConvertingConstructor(CCPPClass* pcClass, SCPP65Type* psArgument, CCPPSyntax* pcArgument);
	int						CompareRanks(int* aiFirst, int* aiSecond, size uiArguments);
	int						QualificationRank(SCPP65Type* psTo, SCPP65Type* psFrom, bool bTopLevel);
	int						CompareQualifiers(CCPPFunction* pcFirst, CCPPFunction* pcSecond, CArraySyntax* papcArguments, int iConstObject);
	CCPPFunction*			FindConversion(CCPPClass* pcClass, SCPP65Type* psTo, bool bExplicit, CCPPSyntax* pcNode);
	void					NoFunctionMatches(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, SCPP65Type** apsArguments, size uiArguments, CCPPSyntax* pcNode);
	bool					HasOverloadOfType(CCPPSyntaxIdExpression* pcFunction, SCPP65Type* psPointer);
	CCPPFunction*			FindStaticFunction(CCPPClass* pcClass, int64 lliName, SCPP65Type* psTarget, bool* pbOverloaded);
	CCPPFunction*			FindOperator(CCPPClass* pcClass, int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode, int iConstObject);
	CCPPScope*				CurrentLookupScope(void);
	CCPPFunction*			FindFreeOperator(int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
};


#endif // __C_P_P_OVERLOADS_H__
