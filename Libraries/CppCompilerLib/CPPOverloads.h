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
#include "BaseLib/ArrayInt.h"
#include "BaseLib/MapLongTemplate.h"
#include "BaseLib/HashMapPrimitiveTemplate.h"
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"
#include "CPP65Type.h"
#include "CPPClass.h"
#include "CPPFunction.h"
#include "CPPTemplate.h"


class CCPPUnit;
class CCPPDiagnostics;


//One of the functions a call could mean, or one of the templates.
struct SCPPCandidate
{
	CCPPFunction*	pcFunction;
	CCPPTemplate*	pcTemplate;
};


typedef CArrayTemplate<SCPPCandidate>	CArrayCPPCandidates;


//The functions (or the function templates) a call could mean: those of its name that take its arguments, each
//with the rank of every argument's conversion to its parameter (see CCPPOverloads::ConversionRank).  A candidate's
//ranks are written to NextRanks, and Add keeps them with it.
class CCPPCandidates
{
protected:
	CArrayCPPCandidates	masCandidates;
	CArrayInt			maiRanks;
	size				muiArguments;

public:
	void			Init(size uiArguments);
	void			Kill(void);

	int*			NextRanks(void);
	void			Add(CCPPFunction* pcFunction, CCPPTemplate* pcTemplate);
	size			NumElements(void) const;
	CCPPFunction*	GetFunction(size uiIndex) { return masCandidates.Get(uiIndex)->pcFunction; }
	CCPPTemplate*	GetTemplate(size uiIndex) { return masCandidates.Get(uiIndex)->pcTemplate; }
	int				Compare(size uiFirst, size uiSecond);
	int				Rank(size uiCandidate, size uiArgument) { return maiRanks.GetValue(uiCandidate * muiArguments + uiArgument); }
};


typedef CHashMapLongTemplate<char>	CMapCPPResolved;


//Chooses the function a call means.  [over.match]  The candidates are the functions of a name that a class, the
//namespaces in reach or the arguments' own namespaces have, and the function templates among them whose arguments
//can be deduced; each is ranked by the conversions its parameters need of the arguments, and the best is the
//answer, or none is if two are as good as each other.  It asks the unit what a name is and where the compiler
//is; it keeps only what it has chosen for before, to count the calls it is asked about again.
#define CPP_BETTER_FUNCTION_ARGUMENTS	32


class CCPPOverloads
{
protected:
	CCPPUnit*			mpcUnit;
	CCPPDiagnostics*	mpcDiagnostics;	//Where its errors go: the unit's.
	CMapCPPResolved		mcResolved;		//The nodes and names FindFunction has chosen for (CPPCC_FindFunctionAgain).
	bool				mbConverting;	//The next FindFunction of constructors takes only converting ones (FindConstructor).
	CArrayCPP65Types*	mpapsOnly;		//The next FindFunction takes only functions of these types: a block's declarations of the name (FindNamedFunction).

public:
	bool				mbAmbiguousConversion;	//FindConversion found two as good.
	bool				mbAmbiguousTemplate;	//ChooseTemplate found two function templates as good and as specialised.

protected:

public:
	void					Init(CCPPUnit* pcUnit);
	void					Kill(void);

	CCPPTemplate*			FindMemberTemplate(CCPPClass* pcClass, int64 lliName, size uiArguments, ECPPCalledObject eObject = CPPCO_None);
	CCPPTemplate*			ChooseMemberTemplate(CCPPClass* pcClass, int64 lliName, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, CArraySyntax* papcExplicit, ECPPCalledObject eObject);
	void					GetMemberTemplates(CCPPClass* pcClass, int64 lliName, size uiArguments, CArrayCPPTemplates* papcCandidates);
	CCPPTemplate*			GetCallTemplate(CCPPSymbol* pcSymbol, int64 lliName, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, CArraySyntax* papcExplicit);
	void					AddCallTemplates(CCPPSymbol* pcSymbol, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, CArrayCPPTemplates* papcCandidates);
	CCPPTemplate*			ChooseTemplate(CArrayCPPTemplates* papcCandidates, ECPPCalledObject eObject, CArraySyntax* papcExplicit, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments);
	bool					TemplateTakes(CCPPTemplate* pcTemplate, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode, int* aiRanks);
	void					MostSpecialised(CArrayCPPTemplates* papcTemplates, size uiArguments, CCPPSyntax* pcNode);
	bool					AtLeastAsSpecialised(CCPPTemplate* pcFirst, CCPPTemplate* pcSecond, size uiArguments, CCPPSyntax* pcNode);
	bool					HasPackAt(CCPPTemplate* pcTemplate, size uiIndex, size uiArguments);
	bool					TemplateParameterTypes(CCPPTemplate* pcTemplate, CCPPTemplateArguments* pcBindings, size uiArguments, CArrayCPP65Types* papsTypes);
	bool					IsOverloadOf(CCPPClass* pcClass, CCPPFunction* pcFunction, int64 lliName);
	CCPPFunction*			FindFunction(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, CArraySyntax* papcArguments, bool bBases, CCPPSyntax* pcNode, ECPPCalledObject eObject = CPPCO_None, CCPPOverloadScopes* papcNamespaces = NULL, CCPPFunction* pcInstance = NULL);
	CCPPFunction*			FindFunctionOrInstance(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, CArraySyntax* papcArguments, bool bBases, CCPPSyntax* pcNode, ECPPCalledObject eObject);
	CCPPFunction*			FindConstructor(CCPPClass* pcClass, CArraySyntax* papcArguments, CCPPSyntax* pcNode, bool bConverting = false);
	CCPPFunction*			FindNamedFunction(CCPPSyntaxName* pcName, int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	CCPPFunction*			FindAddressedFunction(CCPPSyntaxName* pcName, int64 lliName, SCPP65Type* psTarget, CCPPSyntax* pcNode, bool* pbOverloaded);
	CCPPFunction*			AddressedInstance(CCPPSyntaxName* pcName, int64 lliName, SCPP65Type* psTarget, CCPPOverloadScopes* papcNamespaces, CCPPSyntax* pcNode, bool* pbAmbiguous);
	CCPPFunction*			FindAssociatedFunction(CCPPSyntaxName* pcName, int64 lliName, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments);
	void					GetOverloadNamespaces(CCPPSyntaxName* pcName, int64 lliName, CCPPOverloadScopes* papcNamespaces);
	void					GetCallNamespaces(CCPPSyntaxName* pcName, int64 lliName, CArraySyntax* papcArguments, CCPPOverloadScopes* papcNamespaces);
	void					AddArgumentNamespaces(CArraySyntax* papcArguments, CCPPOverloadScopes* papcNamespaces);
	bool					UsesArgumentLookup(CCPPSyntaxName* pcName);
	void					AddOverloadSetNamespaces(CCPPSyntaxName* pcName, CCPPOverloadScopes* papcNamespaces);
	void					AddAssociatedNamespaces(SCPP65Type* psType, CCPPOverloadScopes* papcNamespaces, int iDepth);
	void					AddNamespaceOf(CCPPScope* pcScope, CCPPOverloadScopes* papcNamespaces);
	void					NormaliseNamespaces(CCPPOverloadScopes* papcNamespaces);
	int						DeclarationToken(CCPPFunction* pcFunction);
	bool					IsCandidate(CCPPCandidates* pcCandidates, CCPPFunction* pcFunction);
	bool					RankArguments(CCPPFunction* pcFunction, SCPP65Type** apsArguments, CArraySyntax* papcArguments, int* aiRanks);
	bool					IsBetterFunction(CCPPFunction* pcFirst, CCPPFunction* pcSecond, CArraySyntax* papcArguments);
	int						CompareOperators(CCPPFunction* pcMember, CCPPClass* pcClass, CCPPFunction* pcFree, CCPPSyntax* pcLeft, CCPPSyntax* pcRight, ECPPCalledObject eObject);
	CCPPFunction*			FindSurrogate(CCPPClass* pcClass, CCPPFunction* pcMember, CArraySyntax* papcArguments, bool* pbAmbiguous);
	bool					RankFunctionType(SCPP65Type* psFunction, CArraySyntax* papcArguments, int* aiRanks);
	static SCPP65Type*		SurrogateType(CCPPFunction* pcConversion);
	int						ListRank(SCPP65Type* psParameter, CCPPSyntax* pcList);
	int						ConversionRank(SCPP65Type* psParameter, SCPP65Type* psArgument, CCPPSyntax* pcArgument, bool bUserDefined = true);
	bool					HasConvertingConstructor(CCPPClass* pcClass, SCPP65Type* psArgument, CCPPSyntax* pcArgument);
	bool					Converts(CCPPFunction* pcConstructor, CCPPClass* pcClass, SCPP65Type* psArgument, CCPPSyntax* pcArgument);
	int						ConvertRank(CCPPFunction* pcConstructor, CCPPClass* pcClass, SCPP65Type* psArgument, CCPPSyntax* pcArgument);
	int						ConvertingConstructorRank(CCPPClass* pcClass, SCPP65Type* psArgument, CCPPSyntax* pcArgument);
	CCPPFunction*			MemberInstance(CCPPClass* pcClass, int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode, ECPPCalledObject eObject);
	CCPPFunction*			ConstructorInstance(CCPPClass* pcClass, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	bool					InstanceCopies(CCPPClass* pcClass, CCPPSyntax* pcSource);
	CCPPFunction*			ChooseFunction(CCPPCandidates* pcCandidates, CArraySyntax* papcArguments, ECPPCalledObject eObject, bool* pbAmbiguous);
	int						QualificationRank(SCPP65Type* psTo, SCPP65Type* psFrom, bool bTopLevel);
	bool					IsTemporary(CCPPSyntax* pcArgument, CCPPClass* pcClass);
	int						CompareCandidates(CCPPCandidates* pcCandidates, size uiFirst, size uiSecond, CArraySyntax* papcArguments, ECPPCalledObject eObject);
	int						CompareObjects(CCPPFunction* pcFirst, CCPPFunction* pcSecond, ECPPCalledObject eObject);
	int						CompareArgument(CCPPFunction* pcFirst, CCPPFunction* pcSecond, size uiArgument, CCPPSyntax* pcArgument);
	CCPPFunction*			FindConversion(CCPPClass* pcClass, SCPP65Type* psTo, bool bExplicit, CCPPSyntax* pcNode);
	bool					RefersTo(SCPP65Type* psReturn, SCPP65Type* psReferred);
	CCPPFunction*			FindContextualConversion(CCPPClass* pcClass, ECPPContextualTypes eWanted, CCPPSyntax* pcNode, bool* pbAmbiguous);
	CCPPFunction*			ConversionInstance(CCPPTemplate* pcTemplate, SCPP65Type* psTo, CCPPSyntax* pcNode);
	void					NoFunctionMatches(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, SCPP65Type** apsArguments, size uiArguments, CCPPSyntax* pcNode);
	bool					HasOverloadOfType(CCPPSyntaxIdExpression* pcFunction, SCPP65Type* psPointer);
	CCPPFunction*			FindMemberFunction(CCPPClass* pcClass, int64 lliName, SCPP65Type* psTarget, bool bStatic, CCPPClass** ppcClass, bool* pbOverloaded);
	CCPPFunction*			FindOperator(CCPPClass* pcClass, int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode, ECPPCalledObject eObject);
	CCPPScope*				CurrentLookupScope(void);
	bool					HasFreeOperator(int64 lliName);
	CCPPFunction*			FindFreeOperator(int64 lliName, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
};


#endif // __C_P_P_OVERLOADS_H__
