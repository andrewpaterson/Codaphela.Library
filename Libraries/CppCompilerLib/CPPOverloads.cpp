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
#include "CPPUnit.h"
#include "CPPOverloads.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPCandidates::Init(size uiArguments)
{
	masCandidates.Init();
	maiRanks.Init();
	muiArguments = uiArguments;
	maiRanks.Resize(uiArguments);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPCandidates::Kill(void)
{
	maiRanks.Kill();
	masCandidates.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Where the ranks of the candidate that may be added next are written.  NULL for a call with no arguments.
//
//////////////////////////////////////////////////////////////////////////
int* CCPPCandidates::NextRanks(void)
{
	return (muiArguments > 0) ? maiRanks.Get(masCandidates.NumElements() * muiArguments) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds a candidate, whose ranks are those written to NextRanks.
//
//////////////////////////////////////////////////////////////////////////
void CCPPCandidates::Add(CCPPFunction* pcFunction, CCPPTemplate* pcTemplate)
{
	SCPPCandidate*	psCandidate;

	psCandidate = masCandidates.Add();
	psCandidate->pcFunction = pcFunction;
	psCandidate->pcTemplate = pcTemplate;
	maiRanks.Resize((masCandidates.NumElements() + 1) * muiArguments);
}


//////////////////////////////////////////////////////////////////////////
//
// 1 if the first candidate's arguments convert better (none worse, one better), -1
// if the second's do, 0 if neither.
//
//////////////////////////////////////////////////////////////////////////
int CCPPCandidates::Compare(size uiFirst, size uiSecond)
{
	int*	aiFirst;
	int*	aiSecond;
	size	i;
	bool	bFirst;
	bool	bSecond;

	aiFirst = NextRanks() - (masCandidates.NumElements() - uiFirst) * muiArguments;
	aiSecond = NextRanks() - (masCandidates.NumElements() - uiSecond) * muiArguments;
	bFirst = false;
	bSecond = false;
	for (i = 0; i < muiArguments; i++)
	{
		bFirst |= aiFirst[i] > aiSecond[i];
		bSecond |= aiSecond[i] > aiFirst[i];
	}
	if (bFirst && !bSecond)
	{
		return 1;
	}
	if (bSecond && !bFirst)
	{
		return -1;
	}
	return 0;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloads::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
	mpcDiagnostics = pcUnit->GetDiagnostics();
	mcResolved.Init();
	mbConverting = false;
	mpapsOnly = NULL;
	mbAmbiguousConversion = false;
	mbAmbiguousTemplate = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloads::Kill(void)
{
	mcResolved.Kill();
	mpcUnit = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcFunction, a function named lliName, is one of pcClass's overloads: its
// own, or a base's brought in by a using declaration and not hidden by one of its
// own with the same parameters.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::IsOverloadOf(CCPPClass* pcClass, CCPPFunction* pcFunction, int64 lliName)
{
	CCPPFunction*	pcOwn;
	SCPPUsing*	psUsing;
	size			i;
	bool			bUsed;

	if (pcFunction->mpcClass == pcClass)
	{
		return true;
	}
	if (!pcClass || !pcFunction->mpcClass)
	{
		return false;
	}
	bUsed = false;
	for (i = 0; i < pcClass->macUsings.NumElements(); i++)
	{
		psUsing = pcClass->macUsings.Get(i);
		if ((psUsing->pcBase == pcFunction->mpcClass) && (psUsing->lliName == lliName))
		{
			bUsed = true;
			break;
		}
	}
	if (!bUsed)
	{
		return false;
	}
	for (pcOwn = mpcUnit->FirstFunctionOfName(lliName); pcOwn; pcOwn = pcOwn->mpcNextOfName)
	{
		if ((pcOwn->mpcClass != pcClass) || (pcOwn->meKind != pcFunction->meKind) || pcOwn->mpcTemplate || (pcOwn->mapsParameters.NumElements() != pcFunction->mapsParameters.NumElements()))
		{
			continue;
		}
		if ((pcOwn->mbConstThis != pcFunction->mbConstThis) || (pcOwn->mbVolatileThis != pcFunction->mbVolatileThis) || (pcOwn->meRefQualifier != pcFunction->meRefQualifier))
		{
			//f() doesn't hide the base's f() const: the same parameters, const, volatile and & or && hide.  [namespace.udecl] 15
			continue;
		}
		if (pcOwn->TakesParameters(&pcFunction->mapsParameters))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a type is one of those a place wants an object converted to.
//
//////////////////////////////////////////////////////////////////////////
static bool IsContextualType(SCPP65Type* psType, ECPPContextualTypes eWanted)
{
	if (psType->IsPointer())
	{
		return (eWanted & (psType->psBase->IsFunction() ? CPPXT_FunctionPointer : CPPXT_ObjectPointer)) != 0;
	}
	if (psType->IsFloat())
	{
		return (eWanted & CPPXT_Float) != 0;
	}
	return psType->IsInteger() && !psType->IsScopedEnum() && ((eWanted & CPPXT_Integer) != 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The conversion function an object of pcClass is contextually implicitly
// converted by, where one of the types of eWanted is wanted (an integer for
// a switch's condition or a new array's bound, a pointer to an object for
// delete's operand, a number for a built-in operator's): the class's
// conversion function that isn't explicit and gives such a type.  NULL if it
// has none, or (*pbAmbiguous) they give more than one type.  [conv] 5
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::FindContextualConversion(CCPPClass* pcClass, ECPPContextualTypes eWanted, CCPPSyntax* pcNode, bool* pbAmbiguous)
{
	CCPPFunction*	pcFunction;
	CCPPFunction*	pcFound;
	SCPP65Type*		psType;

	*pbAmbiguous = false;
	pcFound = NULL;
	if (!mpcUnit->GetClasses()->CompleteClass(pcClass, pcNode))
	{
		return NULL;
	}
	for (pcFunction = mpcUnit->FirstFunctionOfName(CPP_CONVERSION_NAME); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if (pcFunction->mpcTemplate || pcFunction->mbExplicit || !mpcUnit->GetClasses()->IsDerived(pcClass, pcFunction->mpcClass, NULL))
		{
			continue;
		}
		psType = pcFunction->mpsReturn->WithoutReference()->Unqualified();
		if (!IsContextualType(psType, eWanted))
		{
			continue;
		}
		if (pcFound && !psType->Same(pcFound->mpsReturn->WithoutReference()->Unqualified()))
		{
			*pbAmbiguous = true;
			return NULL;
		}
		pcFound = pcFunction;
	}
	return pcFound;
}


//////////////////////////////////////////////////////////////////////////
//
// The conversion function (operator T, its class's own or a base's, or a
// conversion template's instance) that converts an object of pcClass to psTo
// best: by its result's conversion to psTo (see ConversionRank; a condition is
// a conversion to bool).  explicit ones only when
// bExplicit (a cast or a condition).  NULL, quietly, if none converts.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::FindConversion(CCPPClass* pcClass, SCPP65Type* psTo, bool bExplicit, CCPPSyntax* pcNode)
{
	CArrayCPPTemplates	apcTemplates;
	CCPPFunction*		pcFunction;
	CCPPFunction*		pcBest;
	int					iRank;
	int					iBest;
	size				i;

	mpcUnit->GetStatistics()->Count(CPPCC_FindConversion);
	if (!mpcUnit->GetClasses()->CompleteClass(pcClass, pcNode))
	{
		return NULL;
	}
	if (pcClass->mpsLambda && pcClass->mpsLambda->bGeneric && psTo->IsPointer() && psTo->psBase->IsFunction())
	{
		//A generic lambda's conversion to this pointer to a function is made when it is first wanted.
		mpcUnit->GetClosures()->GenericLambdaConversion(pcClass->mpsLambda, psTo->psBase);
	}
	pcBest = NULL;
	iBest = CPPCR_None;
	mbAmbiguousConversion = false;
	for (pcFunction = mpcUnit->FirstFunctionOfName(CPP_CONVERSION_NAME); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if (pcFunction->mpcTemplate || (pcFunction->mbExplicit && !bExplicit) || !mpcUnit->GetClasses()->IsDerived(pcClass, pcFunction->mpcClass, NULL))
		{
			continue;
		}
		iRank = ConversionRank(psTo, pcFunction->mpsReturn, NULL, false);
		if (iRank > iBest)
		{
			pcBest = pcFunction;
			iBest = iRank;
			mbAmbiguousConversion = false;
		}
		else if ((iRank == iBest) && (iRank != CPPCR_None) && !pcFunction->mpsReturn->Same(pcBest->mpsReturn))
		{
			//Two functions to other types whose results convert as well: ambiguous (one overriding the other has
			//the same type).  [over.match.conv] 1, [over.match.best] 2
			mbAmbiguousConversion = true;
		}
	}

	//A conversion template's instance for the type, which a function as good as it is preferred to.  [over.match.best] 1.6
	apcTemplates.Init();
	GetMemberTemplates(pcClass, CPP_CONVERSION_NAME, 0, &apcTemplates);
	for (i = 0; (iBest < CPPCR_Exact) && (i < apcTemplates.NumElements()); i++)
	{
		pcFunction = ConversionInstance(apcTemplates.GetPtr(i), psTo, pcNode);
		if (!pcFunction || (pcFunction->mbExplicit && !bExplicit))
		{
			continue;
		}
		iRank = ConversionRank(psTo, pcFunction->mpsReturn, NULL, false);
		if (iRank > iBest)
		{
			pcBest = pcFunction;
			iBest = iRank;
		}
	}
	apcTemplates.Kill();
	return pcBest;
}


//////////////////////////////////////////////////////////////////////////
//
// The instance of a conversion function template that converts to psTo:
// its template parameters deduced from the type (CCPPDeduction::
// DeduceConversion).  NULL, quietly, if there is none.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::ConversionInstance(CCPPTemplate* pcTemplate, SCPP65Type* psTo, CCPPSyntax* pcNode)
{
	CCPPSyntaxDeclarator*	pcDeclarator;
	CCPPSyntaxTypeId*		pcType;
	CCPPTemplateArguments*	pcBindings;
	CCPPDeduction			cDeduction;
	CCPPFunction*			pcFunction;
	CChars					szArguments;

	mpcDiagnostics->BeginTrial();
	pcFunction = NULL;
	pcDeclarator = mpcUnit->GetInstantiator()->DefineMemberTemplate(pcTemplate, pcNode) ? pcTemplate->FunctionDeclarator() : NULL;
	pcType = (pcDeclarator && pcDeclarator->GetName()) ? pcDeclarator->GetName()->GetLast()->mpcType : NULL;
	if (pcType)
	{
		cDeduction.Init(mpcUnit, pcTemplate);
		cDeduction.DeduceConversion(pcType, psTo);
		szArguments.Init();
		pcBindings = mpcUnit->GetInstantiator()->BindArguments(pcTemplate->mpcDeclaration, NULL, cDeduction.GetTypes(), &szArguments, pcNode, cDeduction.GetValues());
		pcFunction = pcBindings ? mpcUnit->GetInstantiator()->InstanceOfFunction(pcTemplate, pcBindings, &szArguments, pcNode) : NULL;
		szArguments.Kill();
		cDeduction.Kill();
	}
	return mpcDiagnostics->EndTrial() ? pcFunction : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function of pcFunction's (unqualified) name has the function type
// psPointer (a pointer to a function, or a reference to one) points to: an
// overloaded name passed for a parameter of that type is that overload.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::HasOverloadOfType(CCPPSyntaxIdExpression* pcFunction, SCPP65Type* psPointer)
{
	CCPPFunction*		pcFound;
	bool				bOverloaded;

	psPointer = psPointer->WithoutReference();
	if (!psPointer->IsPointer() || psPointer->IsMemberPointer() || !psPointer->psBase || !psPointer->psBase->IsFunction() || pcFunction->mpcName->IsQualifiedByClass())
	{
		return false;
	}
	pcFound = FindAddressedFunction(pcFunction->mpcName, pcFunction->mpcName->mpcSymbol->mlliName, psPointer->psBase, pcFunction, &bOverloaded);
	return pcFound && psPointer->psBase->Equals(mpcUnit->GetFunctionDeclarer()->FunctionType(pcFound));
}


//////////////////////////////////////////////////////////////////////////
//
// The static (bStatic) or non-static member function of the name in the
// class, or else in its bases (the first base that has one), and the class
// it is in (*ppcClass, if wanted): the one of psTarget's type, or the only
// one.  *pbOverloaded is set when the name has several and psTarget
// doesn't pick one.  [over.over] 1
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::FindMemberFunction(CCPPClass* pcClass, int64 lliName, SCPP65Type* psTarget, bool bStatic, CCPPClass** ppcClass, bool* pbOverloaded)
{
	CCPPFunction*		pcFunction;
	CCPPFunction*		pcFound;
	size				i;

	mpcUnit->GetStatistics()->Count(CPPCC_FindStaticFunction);
	pcFound = NULL;
	*pbOverloaded = false;
	for (pcFunction = mpcUnit->FirstFunctionOfName(lliName); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if ((pcFunction->mpcClass == pcClass) && (pcFunction->mbStatic == bStatic) && (pcFunction->meKind == CPPFK_Function) && !pcFunction->mpcTemplate)
		{
			if (psTarget && psTarget->Equals(mpcUnit->GetFunctionDeclarer()->FunctionType(pcFunction)))
			{
				pcFound = pcFunction;
				*pbOverloaded = false;
				break;
			}
			*pbOverloaded |= (pcFound != NULL);
			pcFound = pcFunction;
		}
	}
	if (pcFound)
	{
		if (ppcClass)
		{
			*ppcClass = pcClass;
		}
		return *pbOverloaded ? NULL : pcFound;
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		pcFound = FindMemberFunction(pcClass->macBases.Get(i)->pcClass, lliName, psTarget, bStatic, ppcClass, pbOverloaded);
		if (pcFound || *pbOverloaded)
		{
			return pcFound;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The operator function of pcClass (or a base) for the arguments (FindFunctionOrInstance).
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::FindOperator(CCPPClass* pcClass, int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode, ECPPCalledObject eObject)
{
	mpcUnit->GetStatistics()->Count(CPPCC_FindOperator);
	return FindFunctionOrInstance(pcClass, lliName, CPPFK_Function, papcArguments, true, pcNode, eObject);
}


//////////////////////////////////////////////////////////////////////////
//
// The constructor of pcClass for the arguments: the best of those it
// declares and the instance the arguments deduce of a constructor template
// (ConstructorInstance), template<class T> S(T&& t), which an object of
// the class is given to as well when it takes it better than the copy
// constructor does.  Only those that aren't explicit when bConverting (copy
// initialisation).  [temp.mem] 5, [over.match.best] 1, [over.match.ctor] 1
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::FindConstructor(CCPPClass* pcClass, CArraySyntax* papcArguments, CCPPSyntax* pcNode, bool bConverting)
{
	CCPPFunction*	pcInstance;

	pcInstance = ConstructorInstance(pcClass, papcArguments, pcNode);
	mbConverting = bConverting;
	return FindFunction(pcClass, pcClass->mlliName, CPPFK_Constructor, papcArguments, false, pcNode, CPPCO_None, NULL, pcInstance);
}


//////////////////////////////////////////////////////////////////////////
//
// The member function of pcClass (or of a base, if bBases) for the arguments:
// the best of those that are declared and the instance the arguments deduce of
// a member template of the name, if it has one (FindFunction's pcInstance).
// [over.match.funcs] 7
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::FindFunctionOrInstance(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, CArraySyntax* papcArguments, bool bBases, CCPPSyntax* pcNode, ECPPCalledObject eObject)
{
	return FindFunction(pcClass, lliName, eKind, papcArguments, bBases, pcNode, eObject, NULL, MemberInstance(pcClass, lliName, papcArguments, pcNode, eObject));
}


//////////////////////////////////////////////////////////////////////////
//
// The instance the arguments deduce of pcClass's (or a base's) member
// template of the name, or NULL: it has none, or the arguments deduce none
// of it, which is no error.  [temp.deduct] 8
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::MemberInstance(CCPPClass* pcClass, int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode, ECPPCalledObject eObject)
{
	CCPPTemplate*	pcTemplate;
	CCPPFunction*	pcInstance;

	pcTemplate = FindMemberTemplate(pcClass, lliName, papcArguments ? papcArguments->NumElements() : 0, eObject);
	if (!pcTemplate)
	{
		return NULL;
	}
	mpcDiagnostics->BeginSilent();
	pcInstance = mpcUnit->GetInstantiator()->InstantiateFunction(pcTemplate, NULL, papcArguments, pcNode);
	mpcDiagnostics->EndSilent();
	return pcInstance;
}


//////////////////////////////////////////////////////////////////////////
//
// The instance the arguments deduce of pcClass's constructor template, or
// NULL.  Never one that takes an object of its class by value: S(T) isn't
// S(S), which nothing could give its argument to.  [class.copy] 6
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::ConstructorInstance(CCPPClass* pcClass, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CCPPFunction*	pcInstance;
	SCPP65Type*		psFirst;

	pcInstance = MemberInstance(pcClass, pcClass->mlliName, papcArguments, pcNode, CPPCO_None);
	psFirst = (pcInstance && (pcInstance->mapsParameters.NumElements() > 0)) ? pcInstance->mapsParameters.GetPtr(0) : NULL;
	if (psFirst && psFirst->IsClass() && (psFirst->pcClass == pcClass))
	{
		return NULL;
	}
	return pcInstance;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a constructor template's instance copies pcSource, an object of
// pcClass or of a class derived from it, rather than the copy constructor
// the compiler declares (S(const S&), or S(S&&) for what is going): it
// takes the object better.  template<class T> S(T&&) takes an S that isn't
// const as S&, and an object of a derived class as itself; of a const S
// and of one that is going it is S(const S&) and S(S&&) again, and the one
// that isn't a template's is used.  [over.match.best] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::InstanceCopies(CCPPClass* pcClass, CCPPSyntax* pcSource)
{
	CArraySyntax	apcArguments;
	CCPPFunction*	pcInstance;
	SCPP65Type*		psFirst;
	SCPP65Type*		psObject;

	apcArguments.Init();
	apcArguments.Add(pcSource);
	pcInstance = ConstructorInstance(pcClass, &apcArguments, pcSource);
	apcArguments.Kill();
	psFirst = (pcInstance && (pcInstance->mapsParameters.NumElements() > 0) && (pcInstance->RequiredParameters() <= 1)) ? pcInstance->mapsParameters.GetPtr(0) : NULL;
	psObject = psFirst ? psFirst->WithoutReference() : NULL;
	if (!psObject || !psObject->IsClass())
	{
		return false;
	}
	return (psObject->pcClass != pcClass) || (psFirst->IsLvalueReference() && !psObject->bConst);
}


//////////////////////////////////////////////////////////////////////////
//
// The scope code being written is in, as far as the generator knows without
// its block: the one the function's name is declared from, or the namespace
// being declared in.
//
//////////////////////////////////////////////////////////////////////////
CCPPScope* CCPPOverloads::CurrentLookupScope(void)
{
	CCPPSyntaxName*	pcName;
	CCPPFunction*	pcFunction;

	pcFunction = mpcUnit->GetClosures()->OuterFunction();
	if (!pcFunction)
	{
		return mpcUnit->GetContext()->GetNamespace();
	}
	pcName = pcFunction->mpcDeclarator ? pcFunction->mpcDeclarator->GetName() : NULL;
	if (pcName && pcName->mpcLookup)
	{
		return pcName->mpcLookup;
	}
	if (pcFunction->mpcClass && pcFunction->mpcClass->mpcSymbol)
	{
		return pcFunction->mpcClass->mpcSymbol->mpcScope;
	}
	return pcFunction->mpcNamespace;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an operator function of the name that is no class's is declared,
// or a function template of the name that isn't a member.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::HasFreeOperator(int64 lliName)
{
	CCPPFunction*		pcFunction;
	CCPPTemplate*		pcTemplate;
	CCPPSyntaxName*		pcName;
	size				i;

	for (pcFunction = mpcUnit->FirstFunctionOfName(lliName); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if (!pcFunction->mpcClass)
		{
			return true;
		}
	}
	for (i = 0; i < mpcUnit->GetTemplates()->NumElements(); i++)
	{
		pcTemplate = mpcUnit->GetTemplates()->GetPtr(i);
		if (pcTemplate->mbMember || (pcTemplate->mpcPrimary != pcTemplate) || !pcTemplate->IsFunction())
		{
			continue;
		}
		pcName = pcTemplate->FunctionDeclarator() ? pcTemplate->FunctionDeclarator()->GetName() : NULL;
		if (pcName && (mpcUnit->GetFunctionName(pcName->GetLast()) == lliName))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The operator function (not a member) for the arguments: a declared one, or an
// instance of a function template of that name (only an instance, for
// papcExplicit's template arguments).  Those of the namespaces around
// the function being written (without the hiding names have) and those the
// arguments are associated with.  NULL, quietly, if none matches.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::FindFreeOperator(int64 lliName, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CCPPFunction*					pcFunction;
	CCPPTemplate*					pcTemplate;
	CCPPSyntaxName*					pcName;
	CCPPScope*						pcLookup;
	CCPPOverloadScopes				apcNamespaces;
	size							i;
	bool							bDeclared;

	mpcUnit->GetStatistics()->Count(CPPCC_FindFreeOperator);
	bDeclared = false;
	for (pcFunction = mpcUnit->FirstFunctionOfName(lliName); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if (!pcFunction->mpcClass && !pcFunction->mpcTemplate)
		{
			bDeclared = true;
		}
	}

	pcFunction = NULL;
	mpcDiagnostics->BeginSilent();
	apcNamespaces.Init();
	pcLookup = CurrentLookupScope();
	if (pcLookup)
	{
		pcLookup->GatherEnclosingNamespaces(&apcNamespaces);
	}
	AddArgumentNamespaces(papcArguments, &apcNamespaces);
	NormaliseNamespaces(&apcNamespaces);
	if (!apcNamespaces.Has(NULL))
	{
		apcNamespaces.Add(NULL);
	}
	if (bDeclared && !papcExplicit)
	{
		pcFunction = FindFunction(NULL, lliName, CPPFK_Function, papcArguments, false, pcNode, CPPCO_None, &apcNamespaces);
	}
	for (i = 0; !pcFunction && (i < mpcUnit->GetTemplates()->NumElements()); i++)
	{
		pcTemplate = mpcUnit->GetTemplates()->GetPtr(i);
		if (pcTemplate->mbMember || (pcTemplate->mpcPrimary != pcTemplate) || !pcTemplate->IsFunction())
		{
			continue;
		}
		pcName = pcTemplate->FunctionDeclarator() ? pcTemplate->FunctionDeclarator()->GetName() : NULL;
		if (pcName && (pcName->macpComponents.NumElements() == 1) && (mpcUnit->GetFunctionName(pcName->GetLast()) == lliName) && (apcNamespaces.Has(pcTemplate->mpcNamespace)))
		{
			//An instance made only from explicit arguments (template<char... C> operator "" _k) may not take the arguments.
			pcFunction = mpcUnit->GetInstantiator()->InstantiateFunction(pcTemplate, papcExplicit, papcArguments, pcNode);
			if (pcFunction && !pcFunction->TakesArgumentCount(papcArguments ? papcArguments->NumElements() : 0))
			{
				pcFunction = NULL;
			}
		}
	}
	apcNamespaces.Kill();
	mpcDiagnostics->EndSilent();
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// The error for a call no function of the name matches: the arguments' types
// and, when there is only one function of the name, its parameters'.  An
// unsigned short* passed for an unsigned long* (a 16 bit size where a 32 bit
// lsize is wanted) says so.
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloads::NoFunctionMatches(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, SCPP65Type** apsArguments, size uiArguments, CCPPSyntax* pcNode)
{
	CCPPFunction*		pcFunction;
	CCPPFunction*		pcOnly;
	CChars				szMessage;
	size				i;
	size				uiCandidates;

	pcOnly = NULL;
	uiCandidates = 0;
	for (pcFunction = mpcUnit->FirstFunctionOfName(lliName); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if ((pcFunction->meKind == eKind) && !pcFunction->mpcTemplate && IsOverloadOf(pcClass, pcFunction, lliName))
		{
			pcOnly = pcFunction;
			uiCandidates++;
		}
	}

	szMessage.Init("No function matches the arguments (");
	for (i = 0; i < uiArguments; i++)
	{
		if (i != 0)
		{
			szMessage.Append(", ");
		}
		if (apsArguments[i])
		{
			apsArguments[i]->Append(&szMessage);
		}
		else
		{
			szMessage.Append("overloaded function");
		}
	}
	szMessage.Append(")");
	if (uiCandidates == 1)
	{
		szMessage.Append(": it takes (");
		for (i = 0; i < pcOnly->mapsParameters.NumElements(); i++)
		{
			if (i != 0)
			{
				szMessage.Append(", ");
			}
			pcOnly->mapsParameters.GetPtr(i)->Append(&szMessage);
		}
		szMessage.Append(pcOnly->mbVariadic ? ", ...)" : ")");
	}
	mpcDiagnostics->ErrorOnce(pcNode, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The best of the candidates: the one whose every argument converts at least as well as
// for any other, and one better (CompareCandidates).  It is found by comparing each with the best
// so far, then checked to be better than every other: *pbAmbiguous if it is not.  NULL
// if there are no candidates.  [over.match.best]
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::ChooseFunction(CCPPCandidates* pcCandidates, CArraySyntax* papcArguments, ECPPCalledObject eObject, bool* pbAmbiguous)
{
	size	uiBest;
	size	i;

	*pbAmbiguous = false;
	if (pcCandidates->NumElements() == 0)
	{
		return NULL;
	}
	uiBest = 0;
	for (i = 1; i < pcCandidates->NumElements(); i++)
	{
		if (CompareCandidates(pcCandidates, i, uiBest, papcArguments, eObject) > 0)
		{
			uiBest = i;
		}
	}
	for (i = 0; i < pcCandidates->NumElements(); i++)
	{
		if (i == uiBest)
		{
			continue;
		}
		if (CompareCandidates(pcCandidates, uiBest, i, papcArguments, eObject) <= 0)
		{
			*pbAmbiguous = true;
		}
	}
	return pcCandidates->GetFunction(uiBest);
}


//////////////////////////////////////////////////////////////////////////
//
// Overload resolution: of the functions of the name (in the class, then its bases if
// bBases) that take the number of arguments (default arguments make the rest), the
// one whose every argument converts at least as well as for any other, and one
// better (see ConversionRank).  A template's instances aren't among them, but
// for the one the call's arguments deduced (pcInstance, of pcClass or the base it
// is a member of), which is chosen when it is better than each declared one.
// Returns NULL with an error if no one is best, or none matches functions of the
// name; NULL without an error if there are none.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::FindFunction(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, CArraySyntax* papcArguments, bool bBases, CCPPSyntax* pcNode, ECPPCalledObject eObject, CCPPOverloadScopes* papcNamespaces, CCPPFunction* pcInstance)
{
	CCPPCandidates		cCandidates;
	CCPPFunction*		pcFunction;
	CCPPFunction*		pcBest;
	SCPP65Type*			apsArguments[32];
	size				uiArguments;
	size				uiLooked;
	size				i;
	int64				lliResolved;
	char				cResolved;
	bool				bAmbiguous;
	bool				bFound;
	bool				bQualifiedObject;
	bool				bConverting;
	CArrayCPP65Types*	papsOnly;
	size				j;

	//Asked of this search only (FindConstructor, FindNamedFunction), not of those made to rank its candidates.
	bConverting = mbConverting && (eKind == CPPFK_Constructor);
	mbConverting = false;
	papsOnly = mpapsOnly;
	mpapsOnly = NULL;
	mpcUnit->GetStatistics()->Count(CPPCC_FindFunction);
	lliResolved = (int64)(size)pcNode + lliName * 31;
	if (mcResolved.Get(lliResolved))
	{
		mpcUnit->GetStatistics()->Count(CPPCC_FindFunctionAgain);
	}
	else
	{
		cResolved = 1;
		mcResolved.Put(lliResolved, &cResolved);
	}
	if (pcClass && !mpcUnit->GetClasses()->CompleteClass(pcClass, pcNode))
	{
		return NULL;
	}
	uiArguments = papcArguments ? papcArguments->NumElements() : 0;
	if (uiArguments > 32)
	{
		mpcDiagnostics->Error(pcNode, "Too many arguments");
		return NULL;
	}
	for (i = 0; i < uiArguments; i++)
	{
		//An overloaded function's name and a list in braces have no type of their own (NULL): RankArguments
		//matches the name to a parameter that points to one of its overloads, and the list to what it can make.
		apsArguments[i] = mpcUnit->GetTypeChecker()->TypeOf(papcArguments->GetPtr(i));
		if (!apsArguments[i] && !mpcUnit->GetBinder()->FunctionNameOf(papcArguments->GetPtr(i)) && (papcArguments->GetPtr(i)->GetKind() != CPPS_InitializerList))
		{
			//Its errors.
			mpcUnit->GetTypeChecker()->Value(papcArguments->GetPtr(i));
			return NULL;
		}
	}

	//The candidates: the functions of the name that the call can see, that take its arguments.
	cCandidates.Init(uiArguments);
	uiLooked = 0;
	bQualifiedObject = false;
	for (pcFunction = mpcUnit->FirstFunctionOfName(lliName); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if ((pcFunction->meKind != eKind) || pcFunction->mpcTemplate || !IsOverloadOf(pcClass, pcFunction, lliName) || (papcNamespaces && !papcNamespaces->Takes(pcFunction->mpcNamespace, DeclarationToken(pcFunction))))
		{
			continue;
		}
		if ((pcFunction->mbCLinkage && IsCandidate(&cCandidates, pcFunction)) || (bConverting && pcFunction->mbExplicit))
		{
			continue;
		}
		if (papsOnly)
		{
			for (j = 0; (j < papsOnly->NumElements()) && !papsOnly->GetPtr(j)->Equals(mpcUnit->GetFunctionDeclarer()->FunctionType(pcFunction)); j++)
			{
			}
			if (j == papsOnly->NumElements())
			{
				continue;
			}
		}
		uiLooked++;
		if (pcFunction->TakesObject(eObject))
		{
			if (RankArguments(pcFunction, apsArguments, papcArguments, cCandidates.NextRanks()))
			{
				cCandidates.Add(pcFunction, NULL);
			}
		}
		else if (pcFunction->TakesObject(CPPCalledUnqualified(eObject)) && RankArguments(pcFunction, apsArguments, papcArguments, cCandidates.NextRanks()))
		{
			//It would take the arguments, but not an object that is const or volatile.
			bQualifiedObject = true;
		}
	}
	if (pcInstance && (pcInstance->mpcClass == pcClass) && !(bConverting && pcInstance->mbExplicit))
	{
		uiLooked++;
		if (pcInstance->TakesObject(eObject) && RankArguments(pcInstance, apsArguments, papcArguments, cCandidates.NextRanks()))
		{
			cCandidates.Add(pcInstance, NULL);
		}
	}
	bFound = uiLooked > 0;
	mpcUnit->GetStatistics()->Count(CPPCC_Candidates, uiLooked);
	mpcUnit->GetStatistics()->Count(CPPCC_CandidatesViable, cCandidates.NumElements());
	pcBest = ChooseFunction(&cCandidates, papcArguments, eObject, &bAmbiguous);
	cCandidates.Kill();

	if (pcBest && !bAmbiguous)
	{
		if (pcBest->mpsReturn == &mpcUnit->GetTypes()->msUndeduced)
		{
			mpcDiagnostics->Error(pcNode, "The function is declared auto and what it returns isn't known until it is defined");
			return NULL;
		}
		if (pcBest->mbDeleted)
		{
			//It was the best match, and so the call is wrong: that is what deleting it is for.  [dcl.fct.def.delete]
			mpcDiagnostics->ErrorOnce(pcNode, "The function chosen is deleted");
			return NULL;
		}
		return pcBest;
	}
	if (bAmbiguous)
	{
		mpcDiagnostics->ErrorOnce(pcNode, "The call is ambiguous");
		return NULL;
	}
	if (bFound && bQualifiedObject)
	{
		mpcDiagnostics->Error(pcNode, CPPCalledConst(eObject) ? "The object is const and the function that takes these arguments isn't a const member function" : "The object is volatile and the function that takes these arguments isn't a volatile member function");
		return NULL;
	}
	if (bFound)
	{
		NoFunctionMatches(pcClass, lliName, eKind, apsArguments, uiArguments, pcNode);
		return NULL;
	}
	if (bBases && pcClass)
	{
		for (i = 0; i < pcClass->macBases.NumElements(); i++)
		{
			pcFunction = FindFunction(pcClass->macBases.Get(i)->pcClass, lliName, eKind, papcArguments, true, pcNode, eObject, NULL, pcInstance);
			if (pcFunction)
			{
				return pcFunction;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// True if a function of C linkage is a candidate already, declared in
// another namespace: extern "C" functions of a name with the same
// parameters are one function, so finding it twice isn't ambiguous.
// [dcl.link] 6
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::IsCandidate(CCPPCandidates* pcCandidates, CCPPFunction* pcFunction)
{
	CCPPFunction*	pcOther;
	size			i;

	for (i = 0; i < pcCandidates->NumElements(); i++)
	{
		pcOther = pcCandidates->GetFunction(i);
		if (!pcOther || !pcOther->mbCLinkage || (pcOther->mapsParameters.NumElements() != pcFunction->mapsParameters.NumElements()))
		{
			continue;
		}
		if (pcOther->TakesParameters(&pcFunction->mapsParameters))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The token a function is first declared at, as a using-declaration's
// overloads are those declared before it (CCPPOverloadScopes::Takes), or -1
// if it isn't in this unit's tree (an import's).
//
//////////////////////////////////////////////////////////////////////////
int CCPPOverloads::DeclarationToken(CCPPFunction* pcFunction)
{
	return (pcFunction->mpcDeclarator && (pcFunction->mpcDeclarator->mpcTree == mpcUnit->GetTree())) ? pcFunction->mpcDeclarator->miFirstToken : -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The namespaces as functions have them: NULL for the global one.
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloads::NormaliseNamespaces(CCPPOverloadScopes* papcNamespaces)
{
	size	i;

	for (i = 0; i < papcNamespaces->NumElements(); i++)
	{
		papcNamespaces->SetPtr(i, papcNamespaces->GetPtr(i)->GetNamespace());
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The namespaces (as the scopes they are) a name's function can be in
// (CCPPScope::GatherOverloadScopes).
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloads::GetOverloadNamespaces(CCPPSyntaxName* pcName, int64 lliName, CCPPOverloadScopes* papcNamespaces)
{
	if (pcName->mpcLookup)
	{
		pcName->mpcLookup->GatherOverloadScopes(lliName, pcName->mbGlobal || (pcName->GetQualifier() != NULL), papcNamespaces);
	}
	if ((papcNamespaces->NumElements() == 0) && pcName->mpcSymbol)
	{
		papcNamespaces->Add(pcName->mpcSymbol->mpcScope->GetEnclosingNamespace());
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the namespace pcScope is in, with the namespaces an inline one is in
// and the inline namespaces directly in it.  [basic.lookup.argdep] 2
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloads::AddNamespaceOf(CCPPScope* pcScope, CCPPOverloadScopes* papcNamespaces)
{
	CCPPScope*	pcNamespace;
	CCPPScope*	pcInline;
	size		i;

	pcNamespace = pcScope->GetEnclosingNamespace();
	papcNamespaces->Add(pcNamespace);
	for (i = 0; i < pcNamespace->mapcUsings.NumElements(); i++)
	{
		pcInline = pcNamespace->mapcUsings.GetPtr(i);
		if (pcInline->IsInlineNamespace() && (pcInline->mpcParent == pcNamespace))
		{
			papcNamespaces->Add(pcInline);
		}
	}
	for (; pcNamespace->IsInlineNamespace(); pcNamespace = pcNamespace->mpcParent)
	{
		papcNamespaces->Add(pcNamespace->mpcParent);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the namespaces associated with a type: a class's, its bases' and its
// template arguments', an enum's, and those of what a pointer, reference, array
// or function type is made from.  [basic.lookup.argdep] 2
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloads::AddAssociatedNamespaces(SCPP65Type* psType, CCPPOverloadScopes* papcNamespaces, int iDepth)
{
	CCPPClass*	pcClass;
	SCPPBinding*	psBinding;
	size			i;
	size			j;

	if (!psType || (iDepth > CPP65_MAX_ASSOCIATED_DEPTH))
	{
		return;
	}
	switch (psType->eKind)
	{
	case CPP65TK_Class:
		pcClass = psType->pcClass;
		if (pcClass->mpcSymbol)
		{
			AddNamespaceOf(pcClass->mpcSymbol->mpcScope, papcNamespaces);
		}
		for (i = 0; i < pcClass->macBases.NumElements(); i++)
		{
			AddAssociatedNamespaces(pcClass->macBases.Get(i)->pcClass->mpsType, papcNamespaces, iDepth + 1);
		}
		for (i = 0; pcClass->mpcBindings && (i < pcClass->mpcBindings->macBindings.NumElements()); i++)
		{
			psBinding = pcClass->mpcBindings->macBindings.Get(i);
			AddAssociatedNamespaces(psBinding->psType, papcNamespaces, iDepth + 1);
			for (j = 0; psBinding->papsPack && (j < psBinding->papsPack->NumElements()); j++)
			{
				AddAssociatedNamespaces(psBinding->papsPack->GetPtr(j), papcNamespaces, iDepth + 1);
			}
		}
		break;
	case CPP65TK_Function:
		AddAssociatedNamespaces(psType->psBase, papcNamespaces, iDepth + 1);
		for (i = 0; psType->papsParameters && (i < psType->papsParameters->NumElements()); i++)
		{
			AddAssociatedNamespaces(psType->papsParameters->GetPtr(i), papcNamespaces, iDepth + 1);
		}
		break;
	case CPP65TK_MemberPointer:
		AddAssociatedNamespaces(psType->pcClass->mpsType, papcNamespaces, iDepth + 1);
		AddAssociatedNamespaces(psType->psBase, papcNamespaces, iDepth + 1);
		break;
	case CPP65TK_Pointer:
	case CPP65TK_Reference:
	case CPP65TK_Array:
		AddAssociatedNamespaces(psType->psBase, papcNamespaces, iDepth + 1);
		break;
	default:
		if (psType->pcEnum)
		{
			AddNamespaceOf(psType->pcEnum->mpcScope, papcNamespaces);
		}
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the namespaces associated with the types of a call's arguments, and
// for the name of a function those of its overloads' types.
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloads::AddArgumentNamespaces(CArraySyntax* papcArguments, CCPPOverloadScopes* papcNamespaces)
{
	CCPPSyntaxIdExpression*	pcFunctionName;
	size					i;

	for (i = 0; papcArguments && (i < papcArguments->NumElements()); i++)
	{
		pcFunctionName = mpcUnit->GetBinder()->FunctionNameOf(papcArguments->GetPtr(i));
		if (pcFunctionName)
		{
			AddOverloadSetNamespaces(pcFunctionName->mpcName, papcNamespaces);
		}
		else
		{
			AddAssociatedNamespaces(mpcUnit->GetTypeChecker()->TypeOf(papcArguments->GetPtr(i)), papcNamespaces, 0);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An argument that names a function (z(N::cb)) is associated with what its
// overloads' parameter and return types are.  [basic.lookup.argdep] 2
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloads::AddOverloadSetNamespaces(CCPPSyntaxName* pcName, CCPPOverloadScopes* papcNamespaces)
{
	CCPPOverloadScopes	apcOverloads;
	CCPPFunction*		pcFunction;
	int64				lliName;
	size				i;

	lliName = pcName->mpcSymbol->mlliName;
	apcOverloads.Init();
	GetOverloadNamespaces(pcName, lliName, &apcOverloads);
	NormaliseNamespaces(&apcOverloads);
	for (pcFunction = mpcUnit->FirstFunctionOfName(lliName); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if (pcFunction->mpcClass || pcFunction->mpcTemplate || !apcOverloads.Takes(pcFunction->mpcNamespace, DeclarationToken(pcFunction)))
		{
			continue;
		}
		AddAssociatedNamespaces(pcFunction->mpsReturn, papcNamespaces, 1);
		for (i = 0; i < pcFunction->mapsParameters.NumElements(); i++)
		{
			AddAssociatedNamespaces(pcFunction->mapsParameters.GetPtr(i), papcNamespaces, 1);
		}
	}
	apcOverloads.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The namespaces a call's function can be in: what the name finds, and for
// a name that isn't qualified the namespaces its arguments are associated with.
// In a template's instance, what the name finds is what it found where the
// template is written: the functions declared before the call; those the
// arguments' namespaces have are any declared.  A function declared in a
// block (T Back(T);) is the namespace's function of the name, defined
// wherever.  [basic.lookup.argdep] 3, [temp.nondep] 1, [temp.dep.candidate] 1
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloads::GetCallNamespaces(CCPPSyntaxName* pcName, int64 lliName, CArraySyntax* papcArguments, CCPPOverloadScopes* papcNamespaces)
{
	GetOverloadNamespaces(pcName, lliName, papcNamespaces);
	if (mpcUnit->GetContext()->GetBindings() && (pcName->mpcTree == mpcUnit->GetTree()) && (pcName->miFirstToken >= 0) &&
		!(pcName->mpcSymbol && pcName->mpcSymbol->mpcScope && ((pcName->mpcSymbol->mpcScope->meKind == CPPSC_Block) || (pcName->mpcSymbol->mpcScope->meKind == CPPSC_Function))))
	{
		papcNamespaces->LimitTo(pcName->miFirstToken);
	}
	if (UsesArgumentLookup(pcName))
	{
		AddArgumentNamespaces(papcArguments, papcNamespaces);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a call of the name looks in its arguments' namespaces: a name
// neither qualified nor in parentheses, as (p)(s) isn't.
// [basic.lookup.argdep] 1, 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::UsesArgumentLookup(CCPPSyntaxName* pcName)
{
	CCPPSyntax*	pcId;

	pcId = pcName->mpcParent;
	return !pcName->mbGlobal && !pcName->GetQualifier() && !(pcId && pcId->mpcParent && (pcId->mpcParent->GetKind() == CPPS_ParenthesisedExpression));
}


//////////////////////////////////////////////////////////////////////////
//
// The function a call of a name that isn't a member's means.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::FindNamedFunction(CCPPSyntaxName* pcName, int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CCPPOverloadScopes	apcNamespaces;
	CCPPFunction*		pcFunction;
	CArrayCPP65Types	apsBlock;
	CCPPSymbol*			pcDeclaration;
	SCPP65Type*			psType;

	mpcUnit->GetStatistics()->Count(CPPCC_FindNamedFunction);
	apsBlock.Init();
	if (pcName->mpcSymbol && pcName->mpcSymbol->mpcScope && ((pcName->mpcSymbol->mpcScope->meKind == CPPSC_Block) || (pcName->mpcSymbol->mpcScope->meKind == CPPSC_Function)))
	{
		//A function declared in a block hides the others of its name: only its declarations there are candidates (a
		//call doesn't look in the arguments' namespaces either).  [over.dcl] 2, [basic.lookup.argdep] 3
		for (pcDeclaration = pcName->mpcSymbol; pcDeclaration; pcDeclaration = pcDeclaration->mpcNext)
		{
			psType = (pcDeclaration->meKind == CPPSK_Function) ? mpcUnit->GetDeclarators()->GetSymbolType(pcDeclaration, pcNode) : NULL;
			if (psType && psType->IsFunction())
			{
				apsBlock.Add(psType);
			}
		}
	}
	apcNamespaces.Init();
	if (apsBlock.NumElements() > 0)
	{
		GetOverloadNamespaces(pcName, lliName, &apcNamespaces);
		mpapsOnly = &apsBlock;
	}
	else
	{
		GetCallNamespaces(pcName, lliName, papcArguments, &apcNamespaces);
	}
	NormaliseNamespaces(&apcNamespaces);
	pcFunction = FindFunction(NULL, lliName, CPPFK_Function, papcArguments, false, pcNode, CPPCO_None, &apcNamespaces);
	mpapsOnly = NULL;
	apcNamespaces.Kill();
	apsBlock.Kill();
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// The function or function template's instance a call of a name nothing
// declares where it is means: one only its arguments' namespaces have.  NULL
// if there is none.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::FindAssociatedFunction(CCPPSyntaxName* pcName, int64 lliName, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments)
{
	CCPPFunction*	pcFunction;
	CCPPTemplate*	pcTemplate;
	size			uiErrors;

	mpcUnit->GetStatistics()->Count(CPPCC_FindAssociatedFunction);
	if ((lliName == -1) || pcName->mbGlobal || pcName->GetQualifier() || (papcArguments->NumElements() == 0))
	{
		return NULL;
	}
	uiErrors = mpcDiagnostics->GetErrors()->NumErrors();
	pcFunction = FindNamedFunction(pcName, lliName, papcArguments, pcCall);
	if (pcFunction || (mpcDiagnostics->GetErrors()->NumErrors() != uiErrors))
	{
		return pcFunction;
	}
	pcTemplate = GetCallTemplate(NULL, lliName, pcCall, papcArguments, NULL);
	return pcTemplate ? mpcUnit->GetInstantiator()->InstantiateFunction(pcTemplate, NULL, papcArguments, pcCall) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The function a name's address is of: a static member of the class that
// qualifies it or of the class of the function being written, or a function
// it can name.  The one of psTarget's type, else the only one; *pbOverloaded is
// set when the name has several and psTarget doesn't pick one.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::FindAddressedFunction(CCPPSyntaxName* pcName, int64 lliName, SCPP65Type* psTarget, CCPPSyntax* pcNode, bool* pbOverloaded)
{
	CCPPFunction*	pcFunction;
	CCPPFunction*	pcFound;
	CCPPClass*	pcClass;
	CCPPOverloadScopes	apcNamespaces;
	bool			bQualified;
	bool			bAmbiguous;

	mpcUnit->GetStatistics()->Count(CPPCC_FindAddressedFunction);
	pcFound = NULL;
	*pbOverloaded = false;
	pcClass = NULL;
	bQualified = pcName && pcName->IsQualifiedByClass();
	if (bQualified)
	{
		mpcDiagnostics->BeginQuiet();
		pcClass = mpcUnit->GetNames()->NameQualifierClass(pcName, pcNode);
		mpcDiagnostics->EndQuiet();
	}
	else if (mpcUnit->GetClosures()->OuterFunction() && mpcUnit->GetClosures()->OuterFunction()->mpcClass)
	{
		pcClass = mpcUnit->GetClosures()->OuterFunction()->mpcClass;
	}
	if (pcClass && mpcUnit->GetClasses()->CompleteClass(pcClass, pcNode))
	{
		pcFound = FindMemberFunction(pcClass, lliName, psTarget, true, NULL, pbOverloaded);
	}
	if (pcFound || bQualified)
	{
		return pcFound;
	}

	apcNamespaces.Init();
	if (pcName)
	{
		GetOverloadNamespaces(pcName, lliName, &apcNamespaces);
		NormaliseNamespaces(&apcNamespaces);
	}
	for (pcFunction = mpcUnit->FirstFunctionOfName(lliName); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if (!pcFunction->mpcClass && !pcFunction->mpcTemplate && (!pcName || apcNamespaces.Takes(pcFunction->mpcNamespace, DeclarationToken(pcFunction))))
		{
			if (psTarget && psTarget->Equals(mpcUnit->GetFunctionDeclarer()->FunctionType(pcFunction)))
			{
				pcFound = pcFunction;
				*pbOverloaded = false;
				break;
			}
			*pbOverloaded |= (pcFound != NULL);
			pcFound = pcFunction;
		}
	}
	if (psTarget && psTarget->IsFunction() && !(pcFound && !*pbOverloaded && psTarget->Equals(mpcUnit->GetFunctionDeclarer()->FunctionType(pcFound))))
	{
		//No function of the type that isn't a template's: an instance of a function template of the name may be.
		pcFunction = AddressedInstance(pcName, lliName, psTarget, pcName ? &apcNamespaces : NULL, pcNode, &bAmbiguous);
		if (pcFunction || bAmbiguous)
		{
			pcFound = pcFunction;
			*pbOverloaded = bAmbiguous;
		}
	}
	apcNamespaces.Kill();
	return pcFound;
}


//////////////////////////////////////////////////////////////////////////
//
// The instance of a function template of the name (in papcNamespaces, if
// given) whose arguments psTarget's function type deduces, the most
// specialised template's of those that make one; *pbAmbiguous if two are
// as specialised.  NULL if none does.  [over.over] 2, 4
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::AddressedInstance(CCPPSyntaxName* pcName, int64 lliName, SCPP65Type* psTarget, CCPPOverloadScopes* papcNamespaces, CCPPSyntax* pcNode, bool* pbAmbiguous)
{
	CArrayCPPTemplates		apcTemplates;
	CCPPTemplate*			pcTemplate;
	CCPPSyntaxName*			pcTemplateName;
	CCPPFunction*			pcInstance;
	size					i;

	*pbAmbiguous = false;
	apcTemplates.Init();
	for (i = 0; i < mpcUnit->GetTemplates()->NumElements(); i++)
	{
		pcTemplate = mpcUnit->GetTemplates()->GetPtr(i);
		if (pcTemplate->mbMember || pcTemplate->mpcClass || (pcTemplate->mpcPrimary != pcTemplate) || !pcTemplate->IsFunction())
		{
			continue;
		}
		pcTemplateName = pcTemplate->FunctionDeclarator() ? pcTemplate->FunctionDeclarator()->GetName() : NULL;
		if (!pcTemplateName || (mpcUnit->GetFunctionName(pcTemplateName->GetLast()) != lliName) || (papcNamespaces && !papcNamespaces->Takes(pcTemplate->mpcNamespace, -1)))
		{
			continue;
		}
		if (mpcUnit->GetInstantiator()->InstantiateFunctionFor(pcTemplate, psTarget, pcNode))
		{
			apcTemplates.Add(pcTemplate);
		}
	}
	if (apcTemplates.NumElements() > 1)
	{
		MostSpecialised(&apcTemplates, psTarget->papsParameters ? psTarget->papsParameters->NumElements() : 0, pcNode);
	}
	pcInstance = NULL;
	if (apcTemplates.NumElements() == 1)
	{
		pcInstance = mpcUnit->GetInstantiator()->InstantiateFunctionFor(apcTemplates.GetPtr(0), psTarget, pcNode);
	}
	else if (apcTemplates.NumElements() > 1)
	{
		*pbAmbiguous = true;
	}
	apcTemplates.Kill();
	return pcInstance;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcFirst is a better function for the arguments than pcSecond: it
// converts one of them better and none worse (as CCPPCandidates::Compare
// has it).  A template's instance must be, to be called rather than a
// function of the name that isn't a template's.  [over.match.best] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::IsBetterFunction(CCPPFunction* pcFirst, CCPPFunction* pcSecond, CArraySyntax* papcArguments)
{
	SCPP65Type*	apsArguments[CPP_BETTER_FUNCTION_ARGUMENTS];
	int			aiFirst[CPP_BETTER_FUNCTION_ARGUMENTS];
	int			aiSecond[CPP_BETTER_FUNCTION_ARGUMENTS];
	size		uiArguments;
	size		i;
	bool		bBetter;

	uiArguments = papcArguments ? papcArguments->NumElements() : 0;
	if (uiArguments > CPP_BETTER_FUNCTION_ARGUMENTS)
	{
		return true;
	}
	for (i = 0; i < uiArguments; i++)
	{
		apsArguments[i] = mpcUnit->GetTypeChecker()->TypeOf(papcArguments->GetPtr(i));
	}
	if (!RankArguments(pcSecond, apsArguments, papcArguments, aiSecond))
	{
		return true;
	}
	if (!RankArguments(pcFirst, apsArguments, papcArguments, aiFirst))
	{
		return false;
	}
	bBetter = false;
	for (i = 0; i < uiArguments; i++)
	{
		if (aiFirst[i] < aiSecond[i])
		{
			return false;
		}
		bBetter |= aiFirst[i] > aiSecond[i];
	}
	return bBetter;
}


//////////////////////////////////////////////////////////////////////////
//
// Of the left operand's own operator function (pcMember, of pcClass or a
// base) and an operator function that is no class's (pcFree), the one an
// operator expression means: the one whose operands convert better, none
// worse; with them as good, the one that isn't a template's instance.  The
// left operand is the member function's object, bound to its this as T&
// (const T& adding const, a base's member a conversion).  1 for the member,
// -1 for the other, 0 if neither is better: the expression is ambiguous.
// [over.match.oper] 3, [over.match.funcs] 4, [over.match.best] 1
//
//////////////////////////////////////////////////////////////////////////
int CCPPOverloads::CompareOperators(CCPPFunction* pcMember, CCPPClass* pcClass, CCPPFunction* pcFree, CCPPSyntax* pcLeft, CCPPSyntax* pcRight, ECPPCalledObject eObject)
{
	CArraySyntax	apcFree;
	CArraySyntax	apcMember;
	SCPP65Type*		apsArguments[2];
	int				aiMember[2];
	int				aiFree[2];
	size			uiOperands;
	size			i;
	bool			bMember;
	bool			bFree;
	bool			bRanked;

	uiOperands = pcRight ? 2 : 1;
	apcFree.Init();
	apcFree.Add(pcLeft);
	apcMember.Init();
	if (pcRight)
	{
		apcFree.Add(pcRight);
		apcMember.Add(pcRight);
	}
	apsArguments[0] = mpcUnit->GetTypeChecker()->TypeOf(pcLeft);
	apsArguments[1] = pcRight ? mpcUnit->GetTypeChecker()->TypeOf(pcRight) : NULL;
	bRanked = RankArguments(pcFree, apsArguments, &apcFree, aiFree) && RankArguments(pcMember, &apsArguments[1], &apcMember, &aiMember[1]);
	apcMember.Kill();
	apcFree.Kill();
	if (!bRanked)
	{
		return 0;
	}
	if (pcMember->mpcClass != pcClass)
	{
		aiMember[0] = CPPCR_Conversion;
	}
	else
	{
		aiMember[0] = (pcMember->QualifiersAdded(eObject) > 0) ? CPPCR_Qualification : CPPCR_Exact;
	}
	bMember = false;
	bFree = false;
	for (i = 0; i < uiOperands; i++)
	{
		bMember |= aiMember[i] > aiFree[i];
		bFree |= aiFree[i] > aiMember[i];
	}
	if (bMember != bFree)
	{
		return bMember ? 1 : -1;
	}
	if (bMember)
	{
		return 0;
	}
	return (pcFree->mpcTemplate ? 1 : 0) - (pcMember->mpcTemplate ? 1 : 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The function type a conversion function's result calls: operator F*()
// and operator F&() for a function type F.  NULL for any other.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPOverloads::SurrogateType(CCPPFunction* pcConversion)
{
	SCPP65Type*	psType;

	psType = pcConversion->mpsReturn ? pcConversion->mpsReturn->WithoutReference() : NULL;
	if (psType && psType->IsPointer())
	{
		psType = psType->psBase;
	}
	return (psType && psType->IsFunction()) ? psType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The rank of each argument's conversion to a function type's parameter,
// in aiRanks (an ellipsis's for those past its parameters).  False if it
// can't take them.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::RankFunctionType(SCPP65Type* psFunction, CArraySyntax* papcArguments, int* aiRanks)
{
	size		uiParameters;
	size		i;
	SCPP65Type*	psArgument;

	uiParameters = psFunction->papsParameters ? psFunction->papsParameters->NumElements() : 0;
	if ((papcArguments->NumElements() != uiParameters) && !(psFunction->bVariadic && (papcArguments->NumElements() > uiParameters)))
	{
		return false;
	}
	for (i = 0; i < papcArguments->NumElements(); i++)
	{
		if (i >= uiParameters)
		{
			aiRanks[i] = CPPCR_Ellipsis;
			continue;
		}
		psArgument = mpcUnit->GetTypeChecker()->TypeOf(papcArguments->GetPtr(i));
		aiRanks[i] = psArgument ? ConversionRank(psFunction->papsParameters->GetPtr(i), psArgument, papcArguments->GetPtr(i)) : CPPCR_None;
		if (aiRanks[i] == CPPCR_None)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The conversion function of pcClass (or a base) to a pointer or reference
// to a function whose call is an object's call with papcArguments (a
// surrogate call function), if it is better than the class's operator()
// for them (pcMember, or NULL).  The surrogate's first parameter takes the
// object by the conversion, a user-defined one, so operator() takes the
// object better; the surrogate wins only where it takes the arguments
// better and operator() takes none better.  *pbAmbiguous if neither is
// better, or two surrogates are as good.  [over.call.object] 2, 3
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::FindSurrogate(CCPPClass* pcClass, CCPPFunction* pcMember, CArraySyntax* papcArguments, bool* pbAmbiguous)
{
	CCPPFunction*	pcConversion;
	CCPPFunction*	pcBest;
	SCPP65Type*		apsArguments[CPP_BETTER_FUNCTION_ARGUMENTS];
	int				aiRanks[CPP_BETTER_FUNCTION_ARGUMENTS];
	int				aiBest[CPP_BETTER_FUNCTION_ARGUMENTS];
	int				aiMember[CPP_BETTER_FUNCTION_ARGUMENTS];
	size			uiArguments;
	size			i;
	bool			bBetter;
	bool			bWorse;

	*pbAmbiguous = false;
	uiArguments = papcArguments->NumElements();
	if (uiArguments > CPP_BETTER_FUNCTION_ARGUMENTS)
	{
		return NULL;
	}
	pcBest = NULL;
	for (pcConversion = mpcUnit->FirstFunctionOfName(CPP_CONVERSION_NAME); pcConversion; pcConversion = pcConversion->mpcNextOfName)
	{
		if (pcConversion->mpcTemplate || pcConversion->mbExplicit || !pcConversion->mpcClass || !mpcUnit->GetClasses()->IsDerived(pcClass, pcConversion->mpcClass, NULL) ||
			!SurrogateType(pcConversion) || !RankFunctionType(SurrogateType(pcConversion), papcArguments, aiRanks))
		{
			continue;
		}
		if (!pcBest)
		{
			pcBest = pcConversion;
			memcpy(aiBest, aiRanks, sizeof(int) * uiArguments);
			continue;
		}
		bBetter = false;
		bWorse = false;
		for (i = 0; i < uiArguments; i++)
		{
			bBetter |= aiRanks[i] > aiBest[i];
			bWorse |= aiRanks[i] < aiBest[i];
		}
		if (bBetter && !bWorse)
		{
			pcBest = pcConversion;
			memcpy(aiBest, aiRanks, sizeof(int) * uiArguments);
			*pbAmbiguous = false;
		}
		else if (bBetter == bWorse)
		{
			*pbAmbiguous = true;
		}
	}
	if (!pcBest || *pbAmbiguous || !pcMember)
	{
		return pcBest;
	}
	for (i = 0; i < uiArguments; i++)
	{
		apsArguments[i] = mpcUnit->GetTypeChecker()->TypeOf(papcArguments->GetPtr(i));
	}
	if (!RankArguments(pcMember, apsArguments, papcArguments, aiMember))
	{
		return pcBest;
	}
	bBetter = false;
	bWorse = false;
	for (i = 0; i < uiArguments; i++)
	{
		bBetter |= aiBest[i] > aiMember[i];
		bWorse |= aiBest[i] < aiMember[i];
	}
	if (!bBetter)
	{
		//operator() takes the object better, and the arguments no worse.
		return NULL;
	}
	*pbAmbiguous = true;
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The rank of each argument's conversion to its parameter, in aiRanks.  Returns
// false if the function cannot take the arguments.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::RankArguments(CCPPFunction* pcFunction, SCPP65Type** apsArguments, CArraySyntax* papcArguments, int* aiRanks)
{
	size	uiArguments;
	size	i;

	uiArguments = papcArguments ? papcArguments->NumElements() : 0;
	if (!pcFunction->TakesArgumentCount(uiArguments))
	{
		return false;
	}
	for (i = 0; i < uiArguments; i++)
	{
		if (!apsArguments[i] && (papcArguments->GetPtr(i)->GetKind() == CPPS_InitializerList))
		{
			aiRanks[i] = (i < pcFunction->mapsParameters.NumElements()) ? ListRank(pcFunction->mapsParameters.GetPtr(i), papcArguments->GetPtr(i)) : CPPCR_None;
			if (aiRanks[i] == CPPCR_None)
			{
				return false;
			}
			continue;
		}
		if (!apsArguments[i])
		{
			//An overloaded function's name (FindFunction) is an exact match for a pointer to one of its overloads.
			aiRanks[i] = ((i < pcFunction->mapsParameters.NumElements()) && HasOverloadOfType(mpcUnit->GetBinder()->FunctionNameOf(papcArguments->GetPtr(i)), pcFunction->mapsParameters.GetPtr(i))) ? CPPCR_Exact : CPPCR_None;
			if (aiRanks[i] == CPPCR_None)
			{
				return false;
			}
			continue;
		}
		aiRanks[i] = (i < pcFunction->mapsParameters.NumElements()) ? ConversionRank(pcFunction->mapsParameters.GetPtr(i), apsArguments[i], papcArguments->GetPtr(i)) : CPPCR_Ellipsis;
		if (aiRanks[i] == CPPCR_None)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// How well a list in braces converts to a parameter ([over.ics.list]): to
// a std::initializer_list, the worst of its elements' conversions; to
// another class (or a const reference to one), a user defined conversion if
// the class can be made from it; to anything else, its one element's
// conversion if that doesn't narrow, or an exact match for {}.
//
//////////////////////////////////////////////////////////////////////////
int CCPPOverloads::ListRank(SCPP65Type* psParameter, CCPPSyntax* pcList)
{
	SCPP65Type*		psType;
	SCPP65Type*		psElement;
	CArraySyntax*	papcElements;
	size			i;
	int				iRank;
	int				iElement;

	psType = psParameter;
	if (psParameter->IsReference())
	{
		if (!psParameter->psBase->bConst && !psParameter->IsRvalueReference())
		{
			return CPPCR_None;
		}
		psType = psParameter->psBase;
	}
	if (!mpcUnit->GetTypeChecker()->CanInitialise(psType, pcList))
	{
		return CPPCR_None;
	}
	papcElements = mpcUnit->GetInstantiator()->ListElements(pcList);
	psElement = mpcUnit->GetDesugarings()->InitializerListElement(psType);
	if (psElement)
	{
		//To a std::initializer_list, the worst of its elements' conversions.  [over.ics.list] 4
		iRank = CPPCR_Exact;
		for (i = 0; i < papcElements->NumElements(); i++)
		{
			psType = mpcUnit->GetTypeChecker()->TypeOf(papcElements->GetPtr(i));
			iElement = psType ? ConversionRank(psElement, psType, papcElements->GetPtr(i)) : CPPCR_UserDefined;
			iRank = (iElement < iRank) ? iElement : iRank;
		}
		return iRank;
	}
	if (psType->IsClass())
	{
		return CPPCR_UserDefined;
	}
	if (papcElements->NumElements() == 0)
	{
		return CPPCR_Exact;
	}
	psElement = mpcUnit->GetTypeChecker()->TypeOf(papcElements->GetPtr(0));
	return psElement ? ConversionRank(psType->Unqualified(), psElement, papcElements->GetPtr(0)) : CPPCR_None;
}


//////////////////////////////////////////////////////////////////////////
//
// How well an argument converts to a parameter: an exact match (the same type, an
// array or function to a pointer to it), a promotion (bool, the chars, short and enums
// to int as CCPP65Types::Promote has them, float to double), a conversion (between other arithmetic types, a derived class
// or its pointer to a base, a pointer to void*, 0 to a pointer, a pointer to bool),
// a user defined conversion (a constructor of the parameter's class, only when
// bUserDefined) or none.  A reference parameter matches its referred type.
//
//////////////////////////////////////////////////////////////////////////
int CCPPOverloads::ConversionRank(SCPP65Type* psParameter, SCPP65Type* psArgument, CCPPSyntax* pcArgument, bool bUserDefined)
{
	int64			lliValue;
	bool			bReference;
	bool			bRvalue;
	bool			bTemporary;
	CCPPFunction*	pcConversion;
	SCPP65Type*		psPlace;
	SCPP65Type*		psResult;
	int				iOffset;

	bReference = psParameter->IsReference();
	bRvalue = psParameter->IsRvalueReference();
	bTemporary = bReference && pcArgument && mpcUnit->GetBinder()->BindsToTemporary(psParameter, pcArgument);
	psParameter = bReference ? psParameter->psBase : psParameter;
	psArgument = psArgument->WithoutReference();
	if (bReference && psParameter->IsFunction() && pcArgument)
	{
		//A reference to a function binds to the function, which the argument's value, a pointer to it, isn't: its place.
		//[dcl.init.ref] 5
		psPlace = mpcUnit->GetTypeChecker()->PlaceOf(pcArgument);
		psArgument = psPlace ? psPlace : psArgument;
	}
	if (bRvalue)
	{
		if (pcArgument && !mpcUnit->GetTypeChecker()->IsRvalue(pcArgument) && !bTemporary)
		{
			//What has a name doesn't bind to T&&, but for the temporary it is converted to.  [dcl.init.ref] 5.2
			return CPPCR_None;
		}
	}
	else if (bReference && !psParameter->bConst && !psParameter->IsClass() && pcArgument && mpcUnit->GetTypeChecker()->IsRvalue(pcArgument))
	{
		//A value with no name only binds to a const reference or T&&.
		return CPPCR_None;
	}
	if (psParameter->IsPointer() && psArgument->IsPointer() && psParameter->psBase->Equals(psArgument->psBase) && pcArgument && mpcUnit->GetTypeChecker()->IsStringLiteral(CPPSkipParentheses(pcArgument)))
	{
		//A string literal's characters are const: char* p = "x" is allowed, but const char* is the better match.
		return psParameter->psBase->bConst ? CPPCR_Exact : CPPCR_Qualification;
	}
	if (psParameter->Equals(psArgument))
	{
		if (bRvalue)
		{
			//T&& takes what it binds to as it is, and const T&& a T.
			return psParameter->HasQualifiersOf(psArgument) ? CPPCR_Exact : CPPCR_None;
		}
		if (bReference && psArgument->IsClass() && !psArgument->bConst && pcArgument && IsTemporary(pcArgument, psArgument->pcClass))
		{
			//An object made for the call binds to a const reference (T& takes it too, as MSVC lets it).
			return psParameter->bConst ? CPPCR_RvalueToConst : CPPCR_Qualification;
		}
		//A copy doesn't care what its source's own const is; a reference does, as does what a pointer points to.
		return QualificationRank(psParameter, psArgument, bReference);
	}
	if (psParameter->IsPointer() && psArgument->IsFunction() && psParameter->psBase->Equals(psArgument))
	{
		return CPPCR_Exact;
	}
	if (psParameter->IsPointer() && psArgument->IsArray() && psParameter->psBase->Equals(psArgument->psBase))
	{
		return QualificationRank(psParameter->psBase, psArgument->psBase, true);
	}
	if ((psParameter->IsInteger() || psParameter->IsFloat()) && (psArgument->IsInteger() || psArgument->IsFloat()))
	{
		if (psParameter->pcEnum || (psArgument->IsScopedEnum() && !mpcUnit->GetObserver()->IsRecordedEnum(psArgument)))
		{
			//Only the same enum (an exact match) converts to an enum, and an enum class's value to nothing else.
			return CPPCR_None;
		}
		if (psArgument->IsInteger() && psParameter->Unqualified()->Equals(mpcUnit->GetTypes()->Promote(psArgument)))
		{
			//An integral promotion (short to int, unsigned short to unsigned int where int is 16 bits); int to long is a
			//conversion though they are as wide.  [over.ics.scs] 3, [conv.prom]
			return CPPCR_Promotion;
		}
		if (psParameter->IsFloat() && (psParameter->iBytes == 8) && !psParameter->bLongDouble && psArgument->IsFloat() && (psArgument->iBytes == 4))
		{
			return CPPCR_Promotion;
		}
		return CPPCR_Conversion;
	}
	if (psArgument->IsNullPointer())
	{
		//A std::nullptr_t is one exactly, becomes a null pointer or pointer to a member of any type ([conv.ptr] 1,
		//[conv.mem] 1), and nothing else without a cast: not a bool ([conv.bool]: direct initialisation only).
		if (psParameter->WithoutReference()->IsNullPointer())
		{
			return CPPCR_Exact;
		}
		return (psParameter->IsPointer() || psParameter->IsMemberPointer()) ? CPPCR_Conversion : CPPCR_None;
	}
	if (psParameter->IsNullPointer())
	{
		//Only a null pointer constant becomes a std::nullptr_t.
		return (pcArgument && mpcUnit->GetTypeChecker()->IsNullPointerConstant(pcArgument)) ? CPPCR_Conversion : CPPCR_None;
	}
	if (psParameter->IsPointer() && psArgument->IsPointer())
	{
		if (psParameter->psBase->HasQualifiersOf(psArgument->psBase) &&
			(psParameter->psBase->IsVoid() || (psParameter->psBase->IsClass() && psArgument->psBase->IsClass() && mpcUnit->GetClasses()->IsDerived(psArgument->psBase->pcClass, psParameter->psBase->pcClass, NULL))))
		{
			return CPPCR_Conversion;
		}
		return CPPCR_None;
	}
	if (psParameter->IsMemberPointer() && psArgument->IsMemberPointer())
	{
		//A pointer to a member of a base converts to a pointer to the same member of a derived class (not through a
		//virtual base).  [conv.mem] 2
		if (psParameter->pcClass && psArgument->pcClass && psParameter->psBase->Equals(psArgument->psBase) && mpcUnit->GetClasses()->IsDerived(psParameter->pcClass, psArgument->pcClass, &iOffset) &&
			(iOffset != CPP_VIRTUAL_PATH))
		{
			return CPPCR_Conversion;
		}
		return CPPCR_None;
	}
	if ((psParameter->IsPointer() || psParameter->IsMemberPointer()) && psArgument->IsInteger() && pcArgument && mpcUnit->GetFolder()->Constant(pcArgument, &lliValue) && (lliValue == 0))
	{
		return CPPCR_Conversion;
	}
	if ((psParameter->eKind == CPP65TK_Bool) && (psArgument->IsPointer() || psArgument->IsMemberPointer()))
	{
		return CPPCR_Boolean;
	}
	if (psParameter->IsClass() && psArgument->IsClass() && mpcUnit->GetClasses()->IsDerived(psArgument->pcClass, psParameter->pcClass, NULL))
	{
		return (!bReference || psParameter->HasQualifiersOf(psArgument)) ? CPPCR_Conversion : CPPCR_None;
	}
	if (bUserDefined && psParameter->IsClass() && HasConvertingConstructor(psParameter->pcClass, psArgument, pcArgument))
	{
		return CPPCR_UserDefinedExact;
	}
	if (bUserDefined && !psParameter->IsClass() && psArgument->IsClass())
	{
		//The conversion function's result is then converted to the parameter, which tells overloads apart.  A
		//reference binds to a temporary of it, or to what the function refers to.  [over.match.conv], [over.match.ref]
		pcConversion = FindConversion(psArgument->pcClass, psParameter, false, pcArgument);
		if (pcConversion && (!bReference || bTemporary || RefersTo(pcConversion->mpsReturn, psParameter)))
		{
			return (ConversionRank(psParameter, pcConversion->mpsReturn, NULL, false) >= CPPCR_Promotion) ? CPPCR_UserDefinedExact : CPPCR_UserDefined;
		}
	}
	if (bUserDefined && psParameter->IsClass() && psArgument->IsClass())
	{
		//An object of another class with a conversion function to the parameter's class, or to a reference to it,
		//which is all a reference that isn't const takes.  [over.match.copy], [over.match.ref]
		pcConversion = FindConversion(psArgument->pcClass, psParameter->Unqualified(), false, pcArgument);
		psResult = pcConversion ? pcConversion->mpsReturn->WithoutReference() : NULL;
		if (psResult && psResult->IsClass() && mpcUnit->GetClasses()->IsDerived(psResult->pcClass, psParameter->pcClass, NULL) && (!bReference || psParameter->bConst || RefersTo(pcConversion->mpsReturn, psParameter)))
		{
			return CPPCR_UserDefinedExact;
		}
	}
	return CPPCR_None;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a reference to psReferred binds to what a function returning
// psReturn gives, itself: the function returns T&, and what it refers to
// has no const or volatile the reference lacks.  [dcl.init.ref] 5.1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::RefersTo(SCPP65Type* psReturn, SCPP65Type* psReferred)
{
	return psReturn->IsReference() && !psReturn->bRvalue && psReferred->HasQualifiersOf(psReturn->psBase);
}


//////////////////////////////////////////////////////////////////////////
//
// How psFrom converts to psTo, the same type (Equals) but for const and
// volatile: exactly (Exact), adding const or volatile at a level with const
// on every level above it (Qualification), or not at all (None: one is
// taken away, as const T* to T*).  psTo's and psFrom's own qualifiers count
// only with bTopLevel (a reference bound, a pointer's target); a copy is
// made of a value.  [conv.qual]
//
//////////////////////////////////////////////////////////////////////////
int CCPPOverloads::QualificationRank(SCPP65Type* psTo, SCPP65Type* psFrom, bool bTopLevel)
{
	bool	bAdded;
	bool	bAllConst;

	bAdded = false;
	bAllConst = true;
	if (!bTopLevel)
	{
		if (!psTo->IsPointer() || !psFrom->IsPointer())
		{
			return CPPCR_Exact;
		}
		psTo = psTo->psBase;
		psFrom = psFrom->psBase;
	}
	for (;;)
	{
		if (!psTo->HasQualifiersOf(psFrom))
		{
			return CPPCR_None;
		}
		if ((psTo->bConst != psFrom->bConst) || (psTo->bVolatile != psFrom->bVolatile))
		{
			if (!bAllConst)
			{
				return CPPCR_None;
			}
			bAdded = true;
		}
		bAllConst &= psTo->bConst;
		if (!(psTo->IsPointer() && psFrom->IsPointer()) && !(psTo->IsArray() && psFrom->IsArray()))
		{
			return bAdded ? CPPCR_Qualification : CPPCR_Exact;
		}
		psTo = psTo->psBase;
		psFrom = psFrom->psBase;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcClass has a constructor that takes one argument of psArgument's type
// through a standard conversion (any other parameters have defaults): one it
// declares, or else the instance of a constructor template for the argument.
// It is not the class's copy constructor or explicit.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::HasConvertingConstructor(CCPPClass* pcClass, SCPP65Type* psArgument, CCPPSyntax* pcArgument)
{
	return ConvertingConstructorRank(pcClass, psArgument, pcArgument) != CPPCR_None;
}


//////////////////////////////////////////////////////////////////////////
//
// How well the best of pcClass's converting constructors (see
// HasConvertingConstructor) takes an argument of psArgument's type: the
// rank of the argument's conversion to its parameter, CPPCR_None if none
// takes it.
//
//////////////////////////////////////////////////////////////////////////
int CCPPOverloads::ConvertingConstructorRank(CCPPClass* pcClass, SCPP65Type* psArgument, CCPPSyntax* pcArgument)
{
	CArraySyntax	apcArguments;
	CCPPFunction*	pcFunction;
	size			i;
	int				iBest;
	int				iRank;

	if (!mpcUnit->GetClasses()->CompleteClass(pcClass, pcArgument) || !pcClass->mbConstructors)
	{
		return CPPCR_None;
	}
	iBest = CPPCR_None;
	for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
	{
		pcFunction = pcClass->mapcMembers.GetPtr(i);
		if ((pcFunction->meKind == CPPFK_Constructor) && !pcFunction->mpcTemplate)
		{
			iRank = ConvertRank(pcFunction, pcClass, psArgument, pcArgument);
			iBest = (iRank > iBest) ? iRank : iBest;
		}
	}
	if (!pcArgument || (iBest == CPPCR_Exact))
	{
		return iBest;
	}
	apcArguments.Init();
	apcArguments.Add(pcArgument);
	pcFunction = ConstructorInstance(pcClass, &apcArguments, pcArgument);
	apcArguments.Kill();
	iRank = pcFunction ? ConvertRank(pcFunction, pcClass, psArgument, pcArgument) : CPPCR_None;
	return (iRank > iBest) ? iRank : iBest;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the constructor converts an argument of the type to its class:
// it isn't explicit, takes one argument, which isn't an object of the
// class, and the argument becomes its parameter by a standard conversion.
// [class.conv.ctor]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::Converts(CCPPFunction* pcConstructor, CCPPClass* pcClass, SCPP65Type* psArgument, CCPPSyntax* pcArgument)
{
	return ConvertRank(pcConstructor, pcClass, psArgument, pcArgument) != CPPCR_None;
}


//////////////////////////////////////////////////////////////////////////
//
// The same, as the rank of the argument's conversion to the constructor's
// parameter: CPPCR_None if it doesn't convert it.
//
//////////////////////////////////////////////////////////////////////////
int CCPPOverloads::ConvertRank(CCPPFunction* pcConstructor, CCPPClass* pcClass, SCPP65Type* psArgument, CCPPSyntax* pcArgument)
{
	SCPP65Type*	psParameter;

	if (pcConstructor->mbExplicit || (pcConstructor->mapsParameters.NumElements() == 0) || (pcConstructor->RequiredParameters() > 1))
	{
		return CPPCR_None;
	}
	psParameter = pcConstructor->mapsParameters.GetPtr(0)->WithoutReference();
	if (psParameter->IsClass() && (psParameter->pcClass == pcClass))
	{
		return CPPCR_None;
	}
	return ConversionRank(pcConstructor->mapsParameters.GetPtr(0), psArgument, pcArgument, false);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an argument is an object of the class made for the call: what a call or an
// operator returns (IsMadeObject), or one constructed where it is passed, S() and
// S(1), itself or as the right of a comma.  It has no name to change it by
// afterwards, so it binds better to a const reference.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::IsTemporary(CCPPSyntax* pcArgument, CCPPClass* pcClass)
{
	SCPP65Type*	psType;

	pcArgument = mpcUnit->GetTypeChecker()->CommaResult(pcArgument);
	if (mpcUnit->GetTypeChecker()->IsMadeObject(pcArgument, pcClass))
	{
		return true;
	}
	if (pcArgument->GetKind() != CPPS_ConstructExpression)
	{
		return false;
	}
	psType = mpcUnit->GetTypeChecker()->TypeOf(pcArgument);
	return psType && psType->IsClass() && (psType->pcClass == pcClass);
}


//////////////////////////////////////////////////////////////////////////
//
// Which of two candidates is the better: each argument converts no worse
// for it and one better ([over.match.best] 1), where conversions of one
// rank are told apart as [over.ics.rank] 3 and 4 do (CompareArgument), and
// the object a member function is called on (eObject) is an argument too
// (CompareObjects); with all as good, the one that isn't a template's
// instance.  1 if the first is, -1 if the second is, 0 if neither.
//
//////////////////////////////////////////////////////////////////////////
int CCPPOverloads::CompareCandidates(CCPPCandidates* pcCandidates, size uiFirst, size uiSecond, CArraySyntax* papcArguments, ECPPCalledObject eObject)
{
	CCPPFunction*	pcFirst;
	CCPPFunction*	pcSecond;
	size			i;
	int				iFirst;
	int				iSecond;
	int				iCompared;
	bool			bFirst;
	bool			bSecond;

	pcFirst = pcCandidates->GetFunction(uiFirst);
	pcSecond = pcCandidates->GetFunction(uiSecond);
	iCompared = CompareObjects(pcFirst, pcSecond, eObject);
	bFirst = iCompared > 0;
	bSecond = iCompared < 0;
	for (i = 0; papcArguments && (i < papcArguments->NumElements()); i++)
	{
		iFirst = pcCandidates->Rank(uiFirst, i);
		iSecond = pcCandidates->Rank(uiSecond, i);
		iCompared = (iFirst == iSecond) ? CompareArgument(pcFirst, pcSecond, i, papcArguments->GetPtr(i)) : ((iFirst > iSecond) ? 1 : -1);
		bFirst |= iCompared > 0;
		bSecond |= iCompared < 0;
	}
	if (bFirst != bSecond)
	{
		return bFirst ? 1 : -1;
	}
	if (bFirst)
	{
		return 0;
	}
	return (pcSecond->mpcTemplate ? 1 : 0) - (pcFirst->mpcTemplate ? 1 : 0);
}


//////////////////////////////////////////////////////////////////////////
//
// How two member functions take the object they are called on (eObject):
// an object that is going is taken by f() && before f() const &, as T&&
// before const T&; else f() before f() const for one that isn't const, as
// T& before const T&.  1 if the first takes it better, -1 if the second
// does, 0 if neither.  [over.match.funcs] 4, [over.ics.rank] 3.2.3, 3.2.6
//
//////////////////////////////////////////////////////////////////////////
int CCPPOverloads::CompareObjects(CCPPFunction* pcFirst, CCPPFunction* pcSecond, ECPPCalledObject eObject)
{
	if (CPPCalledRvalue(eObject) && (pcFirst->meRefQualifier != pcSecond->meRefQualifier))
	{
		if (pcFirst->meRefQualifier == CPPRQ_Rvalue)
		{
			return 1;
		}
		return (pcSecond->meRefQualifier == CPPRQ_Rvalue) ? -1 : 0;
	}
	if (eObject == CPPCO_None)
	{
		return 0;
	}
	return pcSecond->QualifiersAdded(eObject) - pcFirst->QualifiersAdded(eObject);
}


//////////////////////////////////////////////////////////////////////////
//
// Of two conversions of an argument of one rank, to the first function's
// parameter and to the second's, the better: [over.ics.rank]
//   - a braced list to a std::initializer_list (3.1);
//   - of T* and const T*, or T& and const T&, the one without const (and
//     of const and const volatile, the one without volatile), but an object
//     made by the argument (a temporary) or a const object binds better to a
//     const reference (3.2.6);
//   - a pointer, reference or object of a derived class to its nearer base,
//     or to a class's rather than void*; a pointer to a base's member to the
//     nearer derived class's (4.2, 4.4).
// 1 if the first is better, -1 if the second is, 0 if neither.
//
//////////////////////////////////////////////////////////////////////////
int CCPPOverloads::CompareArgument(CCPPFunction* pcFirst, CCPPFunction* pcSecond, size uiArgument, CCPPSyntax* pcArgument)
{
	SCPP65Type*	psFirst;
	SCPP65Type*	psSecond;
	SCPP65Type*	psFirstBase;
	SCPP65Type*	psSecondBase;
	bool		bConstBetter;
	bool		bFirstList;
	bool		bSecondList;

	if ((uiArgument >= pcFirst->mapsParameters.NumElements()) || (uiArgument >= pcSecond->mapsParameters.NumElements()))
	{
		return 0;
	}
	psFirst = pcFirst->mapsParameters.GetPtr(uiArgument);
	psSecond = pcSecond->mapsParameters.GetPtr(uiArgument);
	if (pcArgument->GetKind() == CPPS_InitializerList)
	{
		bFirstList = mpcUnit->GetDesugarings()->InitializerListElement(psFirst->WithoutReference()) != NULL;
		bSecondList = mpcUnit->GetDesugarings()->InitializerListElement(psSecond->WithoutReference()) != NULL;
		if (bFirstList != bSecondList)
		{
			return bFirstList ? 1 : -1;
		}
	}

	if ((psFirst->IsPointer() && psSecond->IsPointer()) || (psFirst->IsReference() && psSecond->IsReference()))
	{
		if (psFirst->psBase->Equals(psSecond->psBase))
		{
			if (psFirst->psBase->bConst != psSecond->psBase->bConst)
			{
				bConstBetter = psFirst->IsReference() && ((psFirst->psBase->IsClass() && IsTemporary(pcArgument, psFirst->psBase->pcClass)) || mpcUnit->GetTypeChecker()->IsConstObject(pcArgument));
				return (psFirst->psBase->bConst == bConstBetter) ? 1 : -1;
			}
			if (psFirst->psBase->bVolatile != psSecond->psBase->bVolatile)
			{
				return psFirst->psBase->bVolatile ? -1 : 1;
			}
			return 0;
		}
		psFirstBase = psFirst->psBase;
		psSecondBase = psSecond->psBase;
	}
	else if (psFirst->IsClass() && psSecond->IsClass())
	{
		psFirstBase = psFirst;
		psSecondBase = psSecond;
	}
	else if (psFirst->IsMemberPointer() && psSecond->IsMemberPointer() && psFirst->pcClass && psSecond->pcClass && (psFirst->pcClass != psSecond->pcClass))
	{
		//int A::* to int B::* is better than to int C::* where C is derived from B.
		if (mpcUnit->GetClasses()->IsDerived(psSecond->pcClass, psFirst->pcClass, NULL))
		{
			return 1;
		}
		return mpcUnit->GetClasses()->IsDerived(psFirst->pcClass, psSecond->pcClass, NULL) ? -1 : 0;
	}
	else
	{
		return 0;
	}

	if (psFirst->IsPointer() && psFirstBase->IsClass() && psSecondBase->IsVoid())
	{
		return 1;
	}
	if (psSecond->IsPointer() && psSecondBase->IsClass() && psFirstBase->IsVoid())
	{
		return -1;
	}
	if (!psFirstBase->IsClass() || !psSecondBase->IsClass() || (psFirstBase->pcClass == psSecondBase->pcClass))
	{
		return 0;
	}
	if (mpcUnit->GetClasses()->IsDerived(psFirstBase->pcClass, psSecondBase->pcClass, NULL))
	{
		return 1;
	}
	return mpcUnit->GetClasses()->IsDerived(psSecondBase->pcClass, psFirstBase->pcClass, NULL) ? -1 : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A member template of pcClass (or of a base) named lliName that takes uiArguments
// arguments.  Of two that do, the one whose const matches the object's (eObject)
// is chosen.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPOverloads::FindMemberTemplate(CCPPClass* pcClass, int64 lliName, size uiArguments, ECPPCalledObject eObject)
{
	CArrayCPPTemplates	apcCandidates;
	CCPPTemplate*			pcTemplate;

	mpcUnit->GetStatistics()->Count(CPPCC_FindMemberTemplate);
	apcCandidates.Init();
	GetMemberTemplates(pcClass, lliName, uiArguments, &apcCandidates);
	pcTemplate = ChooseTemplate(&apcCandidates, eObject, NULL, NULL, NULL);
	apcCandidates.Kill();
	return pcTemplate;
}


//////////////////////////////////////////////////////////////////////////
//
// The member template of pcClass (or of a base) named lliName that pcCall, with
// the explicit template arguments papcExplicit, uses (ChooseTemplate).
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPOverloads::ChooseMemberTemplate(CCPPClass* pcClass, int64 lliName, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, CArraySyntax* papcExplicit, ECPPCalledObject eObject)
{
	CArrayCPPTemplates	apcCandidates;
	CCPPTemplate*			pcTemplate;

	mpcUnit->GetStatistics()->Count(CPPCC_ChooseMemberTemplate);
	apcCandidates.Init();
	GetMemberTemplates(pcClass, lliName, papcArguments->NumElements(), &apcCandidates);
	pcTemplate = ChooseTemplate(&apcCandidates, eObject, papcExplicit, pcCall, papcArguments);
	apcCandidates.Kill();
	return pcTemplate;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds pcClass's member templates named lliName that take uiArguments arguments,
// or else those of its first base that has any.
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloads::GetMemberTemplates(CCPPClass* pcClass, int64 lliName, size uiArguments, CArrayCPPTemplates* papcCandidates)
{
	CCPPTemplate*				pcTemplate;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	size						i;

	for (i = 0; i < mpcUnit->GetTemplates()->NumElements(); i++)
	{
		pcTemplate = mpcUnit->GetTemplates()->GetPtr(i);
		if (!pcTemplate->mbMemberTemplate || (pcTemplate->mpcClass != pcClass) || (pcTemplate->mlliName != lliName))
		{
			continue;
		}
		pcSuffix = pcTemplate->mpcDeclarator->GetFunctionSuffix();
		if (pcSuffix && pcSuffix->TakesArgumentCount(uiArguments))
		{
			papcCandidates->Add(pcTemplate);
		}
	}

	for (i = 0; (papcCandidates->NumElements() == 0) && (i < pcClass->macBases.NumElements()); i++)
	{
		GetMemberTemplates(pcClass->macBases.Get(i)->pcClass, lliName, uiArguments, papcCandidates);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The function template pcCall to pcSymbol, with the explicit template arguments
// papcExplicit, uses: of its overloads (the declarations chained by mpcNext) that
// take the call's arguments, the one ChooseTemplate chooses, or else
// GetTemplate's.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPOverloads::GetCallTemplate(CCPPSymbol* pcSymbol, int64 lliName, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, CArraySyntax* papcExplicit)
{
	CArrayCPPTemplates	apcCandidates;
	CCPPOverloadScopes		apcNamespaces;
	CCPPTemplate*			pcTemplate;
	size					i;

	mpcUnit->GetStatistics()->Count(CPPCC_GetCallTemplate);
	apcCandidates.Init();
	apcNamespaces.Init();
	if (pcCall->mpcCallee->GetKind() == CPPS_IdExpression)
	{
		GetCallNamespaces(((CCPPSyntaxIdExpression*)pcCall->mpcCallee)->mpcName, lliName, papcArguments, &apcNamespaces);
	}
	for (i = 0; i < apcNamespaces.NumElements(); i++)
	{
		AddCallTemplates(apcNamespaces.GetPtr(i)->FindHere(lliName), pcCall, papcArguments, &apcCandidates);
	}
	if (apcNamespaces.NumElements() == 0)
	{
		AddCallTemplates(pcSymbol, pcCall, papcArguments, &apcCandidates);
	}
	pcTemplate = NULL;
	if (apcCandidates.NumElements() > 0)
	{
		pcTemplate = ChooseTemplate(&apcCandidates, CPPCO_None, papcExplicit, pcCall, papcArguments);
	}
	else if (pcSymbol)
	{
		pcTemplate = mpcUnit->GetInstantiator()->GetTemplate(pcSymbol);
	}
	apcNamespaces.Kill();
	apcCandidates.Kill();
	return pcTemplate;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the function templates of pcSymbol's declarations that take the call's
// number of arguments.
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloads::AddCallTemplates(CCPPSymbol* pcSymbol, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, CArrayCPPTemplates* papcCandidates)
{
	CCPPTemplate*				pcTemplate;
	CCPPSymbol*					pcDeclaration;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	size						i;

	for (pcDeclaration = pcSymbol; pcDeclaration; pcDeclaration = pcDeclaration->mpcNext)
	{
		if (pcDeclaration->meKind != CPPSK_FunctionTemplate)
		{
			continue;
		}
		for (i = 0; i < mpcUnit->GetTemplates()->NumElements(); i++)
		{
			pcTemplate = mpcUnit->GetTemplates()->GetPtr(i);
			if (pcTemplate->mbMember || (pcTemplate->mpcPrimary != pcTemplate) || (pcTemplate->mpcSymbol != pcDeclaration) || !pcTemplate->IsFunction())
			{
				continue;
			}
			pcSuffix = pcTemplate->FunctionDeclarator()->GetFunctionSuffix();
			if (pcSuffix && pcSuffix->TakesArgumentCount(papcArguments->NumElements()) && (papcCandidates->Find(pcTemplate) == ARRAY_ELEMENT_NOT_FOUND))
			{
				papcCandidates->Add(pcTemplate);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Of papsCandidates, the templates that take a call's number of arguments in the
// order they were found, the one the call uses.  When there are several, the ones
// the call can use (TemplateTakes) are chosen from, if there are any, as
// SMemberCall::Init's overloads differ only in their function's type; of those,
// the ones whose arguments convert best (as plain overloads are ranked), as
// CObjects::Get<M>(OIndex) and Get<M>(const char*) differ only in their
// parameter's type, and of those the most specialised (MostSpecialised:
// f(T*) before f(T) for a pointer).  Then the first whose const and volatile match the
// object's (eObject) is chosen, or else the first.  pcCall is NULL to choose by the number
// of arguments alone.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPOverloads::ChooseTemplate(CArrayCPPTemplates* papcCandidates, ECPPCalledObject eObject, CArraySyntax* papcExplicit, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments)
{
	CCPPCandidates				cTaken;
	CArrayCPPTemplates			apcChosen;
	CArrayCPPTemplates*			papcChosen;
	CCPPTemplate*				pcTemplate;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	size						uiArguments;
	size					uiBetter;
	size					i;

	mpcUnit->GetStatistics()->Count(CPPCC_ChooseTemplate);
	apcChosen.Init();
	uiArguments = pcCall ? papcArguments->NumElements() : 0;
	cTaken.Init(uiArguments);
	for (i = 0; pcCall && (papcCandidates->NumElements() > 1) && (i < papcCandidates->NumElements()); i++)
	{
		pcTemplate = papcCandidates->GetPtr(i);
		if (TemplateTakes(pcTemplate, papcExplicit, papcArguments, pcCall, cTaken.NextRanks()))
		{
			cTaken.Add(NULL, pcTemplate);
		}
	}

	//Only the templates whose arguments no other's convert better.
	for (i = 0; (uiArguments > 0) && (i < cTaken.NumElements()); i++)
	{
		for (uiBetter = 0; uiBetter < cTaken.NumElements(); uiBetter++)
		{
			if (cTaken.Compare(uiBetter, i) > 0)
			{
				break;
			}
		}
		if (uiBetter == cTaken.NumElements())
		{
			apcChosen.Add(cTaken.GetTemplate(i));
		}
	}
	if (apcChosen.NumElements() == 0)
	{
		for (i = 0; i < cTaken.NumElements(); i++)
		{
			apcChosen.Add(cTaken.GetTemplate(i));
		}
	}
	if ((uiArguments > 0) && (apcChosen.NumElements() > 1))
	{
		MostSpecialised(&apcChosen, uiArguments, pcCall);
	}
	//Two left that take the arguments as well, neither more specialised: the call is ambiguous.  [temp.func.order] 5
	mbAmbiguousTemplate = (uiArguments > 0) && (eObject == CPPCO_None) && (apcChosen.NumElements() > 1);
	papcChosen = (apcChosen.NumElements() > 0) ? &apcChosen : papcCandidates;

	pcTemplate = (papcChosen->NumElements() > 0) ? papcChosen->GetPtr(0) : NULL;
	for (i = 0; (eObject != CPPCO_None) && (i < papcChosen->NumElements()); i++)
	{
		pcSuffix = papcChosen->GetPtr(i)->mpcDeclarator->GetFunctionSuffix();
		if (pcSuffix && (pcSuffix->IsConst() == CPPCalledConst(eObject)) && (pcSuffix->IsVolatile() == CPPCalledVolatile(eObject)))
		{
			pcTemplate = papcChosen->GetPtr(i);
			break;
		}
	}
	cTaken.Kill();
	apcChosen.Kill();
	return pcTemplate;
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps only the templates no other is more specialised than, as a call
// whose arguments they take equally well uses the most specialised.
// [temp.func.order], [over.match.best] 1.6
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloads::MostSpecialised(CArrayCPPTemplates* papcTemplates, size uiArguments, CCPPSyntax* pcNode)
{
	CArrayCPPTemplates	apcKept;
	CCPPTemplate*		pcTemplate;
	CCPPTemplate*		pcOther;
	size				i;
	size				j;
	bool				bLess;

	apcKept.Init();
	for (i = 0; i < papcTemplates->NumElements(); i++)
	{
		pcTemplate = papcTemplates->GetPtr(i);
		bLess = false;
		for (j = 0; !bLess && (j < papcTemplates->NumElements()); j++)
		{
			pcOther = papcTemplates->GetPtr(j);
			bLess = (pcOther != pcTemplate) && AtLeastAsSpecialised(pcOther, pcTemplate, uiArguments, pcNode) && !AtLeastAsSpecialised(pcTemplate, pcOther, uiArguments, pcNode);
		}
		if (!bLess)
		{
			apcKept.Add(pcTemplate);
		}
	}
	if (apcKept.NumElements() > 0)
	{
		papcTemplates->ReInit();
		for (i = 0; i < apcKept.NumElements(); i++)
		{
			papcTemplates->Add(apcKept.GetPtr(i));
		}
	}
	apcKept.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// True if pcFirst is at least as specialised as pcSecond for a call's
// first uiArguments parameters: pcSecond's template parameters can be
// deduced from pcFirst's parameter types, with pcFirst's own template
// parameters standing for types of their own (BindAsItself), so that
// pcSecond's parameters are then those types.  A pack of pcFirst's
// parameters deduces no parameter of pcSecond's that isn't one (f(T)
// before f(T...)).  Of two references deduced from each other, an lvalue
// reference is the more specialised, then the more const or volatile one
// (f(const T&) before f(T&)).  [temp.deduct.partial] 8, 9
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::AtLeastAsSpecialised(CCPPTemplate* pcFirst, CCPPTemplate* pcSecond, size uiArguments, CCPPSyntax* pcNode)
{
	CArrayCPP65Types		apsFirst;
	CArrayCPP65Types		apsSecond;
	CArrayCPP65Types		apsDeduce;
	CCPPTemplateArguments*	pcBindings;
	CCPPDeduction			cDeduction;
	CChars					szArguments;
	SCPP65Type*				psFirst;
	SCPP65Type*				psSecond;
	size					i;
	size					uiCompared;
	bool					bResult;
	bool					bFirstPack;
	bool					bSecondPack;

	mpcDiagnostics->BeginTrial();
	apsFirst.Init();
	apsSecond.Init();
	szArguments.Init();
	pcBindings = mpcUnit->GetInstantiator()->BindAsItself(pcFirst->mpcDeclaration, &szArguments, pcNode);
	bResult = pcBindings && TemplateParameterTypes(pcFirst, pcBindings, uiArguments, &apsFirst);
	szArguments.Clear();
	if (bResult)
	{
		//Deduced from the types referred to, without their own const and volatile.  [temp.deduct.partial] 5, 7
		apsDeduce.Init();
		for (i = 0; i < apsFirst.NumElements(); i++)
		{
			apsDeduce.Add(apsFirst.GetPtr(i)->WithoutReference()->Unqualified());
		}
		cDeduction.Init(mpcUnit, pcSecond);
		cDeduction.Deduce(&apsDeduce, NULL);
		apsDeduce.Kill();
		pcBindings = mpcUnit->GetInstantiator()->BindArguments(pcSecond->mpcDeclaration, NULL, cDeduction.GetTypes(), &szArguments, pcNode, cDeduction.GetValues(), cDeduction.GetPacks());
		cDeduction.Kill();
		bResult = pcBindings && TemplateParameterTypes(pcSecond, pcBindings, uiArguments, &apsSecond);
	}
	uiCompared = 0;
	if (bResult)
	{
		//TemplateParameterTypes stops at a pack.
		bFirstPack = HasPackAt(pcFirst, apsFirst.NumElements(), uiArguments);
		bSecondPack = HasPackAt(pcSecond, apsSecond.NumElements(), uiArguments);
		if (bFirstPack && (!bSecondPack || (apsSecond.NumElements() > apsFirst.NumElements())))
		{
			bResult = false;
		}
		else if (!bSecondPack && (apsSecond.NumElements() != apsFirst.NumElements()))
		{
			bResult = false;
		}
		uiCompared = (apsFirst.NumElements() < apsSecond.NumElements()) ? apsFirst.NumElements() : apsSecond.NumElements();
	}
	for (i = 0; bResult && (i < uiCompared); i++)
	{
		//A parameter taken by value takes its argument's type without its own const.  [temp.deduct.partial] 7
		psFirst = apsFirst.GetPtr(i);
		psSecond = apsSecond.GetPtr(i);
		bResult = psSecond->WithoutReference()->Unqualified()->Same(psFirst->WithoutReference()->Unqualified());
		if (bResult && psFirst->IsReference() && psSecond->IsReference())
		{
			if (psFirst->IsRvalueReference() && psSecond->IsLvalueReference())
			{
				bResult = false;
			}
			else if (psFirst->IsRvalueReference() == psSecond->IsRvalueReference())
			{
				bResult = psFirst->psBase->HasQualifiersOf(psSecond->psBase);
			}
		}
	}
	szArguments.Kill();
	apsSecond.Kill();
	apsFirst.Kill();
	return mpcDiagnostics->EndTrial() && bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function template's parameter at uiIndex is a pack, the index
// being one of a call's uiArguments.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::HasPackAt(CCPPTemplate* pcTemplate, size uiIndex, size uiArguments)
{
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;

	pcSuffix = pcTemplate->FunctionDeclarator() ? pcTemplate->FunctionDeclarator()->GetFunctionSuffix() : NULL;
	if (!pcSuffix || (uiIndex >= uiArguments) || (uiIndex >= pcSuffix->macpParameters.NumElements()))
	{
		return false;
	}
	pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(uiIndex);
	return (pcParameter->GetKind() == CPPS_Parameter) && pcParameter->mpcDeclarator && pcParameter->mpcDeclarator->mbPack;
}


//////////////////////////////////////////////////////////////////////////
//
// The types of a function template's first uiArguments parameters with
// its template parameters bound as pcBindings says.  False if one can't be
// worked out (a pack is the end of them).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::TemplateParameterTypes(CCPPTemplate* pcTemplate, CCPPTemplateArguments* pcBindings, size uiArguments, CArrayCPP65Types* papsTypes)
{
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	SCPP65Type*					psType;
	size						i;
	bool						bResult;

	pcSuffix = pcTemplate->FunctionDeclarator() ? pcTemplate->FunctionDeclarator()->GetFunctionSuffix() : NULL;
	if (!pcSuffix)
	{
		return false;
	}
	pcBindings->mpcOuter = mpcUnit->GetContext()->GetBindings();
	mpcUnit->GetContext()->EnterBindings(pcBindings);
	bResult = true;
	for (i = 0; bResult && (i < uiArguments) && (i < pcSuffix->macpParameters.NumElements()); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		if ((pcParameter->GetKind() != CPPS_Parameter) || (pcParameter->mpcDeclarator && pcParameter->mpcDeclarator->mbPack))
		{
			break;
		}
		psType = mpcUnit->GetDeclarators()->GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter);
		bResult = psType != NULL;
		if (bResult)
		{
			papsTypes->Add(psType);
		}
	}
	mpcUnit->GetContext()->LeaveBindings();
	pcBindings->mpcOuter = NULL;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a call with papcArguments (and the explicit template arguments
// papcExplicit) can use pcTemplate: its parameters can be deduced and bound, each
// parameter whose argument is a pointer to a member function is of that
// argument's type, and every other argument converts to its parameter.  aiRanks
// (one per argument, may be NULL) gets each conversion's rank (ConversionRank).
// Nothing is reported; a template the call can't use is passed over.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::TemplateTakes(CCPPTemplate* pcTemplate, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode, int* aiRanks)
{
	CArrayCPP65Types			apsArguments;
	CCPPDeduction				cDeduction;
	CChars						szArguments;
	CCPPTemplateArguments*		pcBindings;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	CCPPSyntaxParameter*		pcParameter;
	SCPP65Type*					psArgument;
	SCPP65Type*					psParameter;
	CCPPFunction*				pcInstance;
	bool						bTakes;
	bool						bPack;
	int							iRank;
	size						i;

	for (i = 0; aiRanks && (i < papcArguments->NumElements()); i++)
	{
		aiRanks[i] = CPPCR_Exact;
	}

	//A failure here is not one of the quiet code around it.
	mpcDiagnostics->BeginTrial();
	apsArguments.Init();
	pcBindings = NULL;
	bTakes = !pcTemplate->mpcClass || mpcUnit->GetInstantiator()->DefineMemberTemplate(pcTemplate, pcNode);
	if (bTakes)
	{
		for (i = 0; i < papcArguments->NumElements(); i++)
		{
			apsArguments.Add(mpcUnit->GetTypeChecker()->TypeOf(papcArguments->GetPtr(i)));
		}
		cDeduction.Init(mpcUnit, pcTemplate);
		cDeduction.Deduce(&apsArguments, papcArguments);
		bTakes = !cDeduction.HasConflict() || papcExplicit;
		szArguments.Init();
		pcBindings = mpcUnit->GetInstantiator()->BindArguments(pcTemplate->mpcDeclaration, papcExplicit, cDeduction.GetTypes(), &szArguments, pcNode, cDeduction.GetValues(), cDeduction.GetPacks());
		szArguments.Kill();
		cDeduction.Kill();
		bTakes = bTakes && (pcBindings != NULL);
	}

	if (bTakes)
	{
		//The parameters' types are worked out as InstantiateFunction declares them.
		if (pcTemplate->mpcClass)
		{
			pcBindings->mpcOuter = mpcUnit->GetInstantiator()->MemberTemplateClassBindings(pcTemplate);
		}
		else
		{
			pcBindings->mpcOuter = mpcUnit->GetContext()->GetBindings();
		}
		mpcUnit->GetContext()->EnterBindings(pcBindings);
		pcSuffix = pcTemplate->FunctionDeclarator() ? pcTemplate->FunctionDeclarator()->GetFunctionSuffix() : NULL;
		bPack = false;
		for (i = 0; bTakes && !bPack && pcSuffix && (i < pcSuffix->macpParameters.NumElements()) && (i < apsArguments.NumElements()); i++)
		{
			pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
			bPack = pcParameter->mpcDeclarator && pcParameter->mpcDeclarator->mbPack;
			psArgument = apsArguments.GetPtr(i);
			if (bPack || !psArgument)
			{
				continue;
			}
			psParameter = mpcUnit->GetDeclarators()->GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcNode);
			psArgument = psArgument->WithoutReference();
			if (psArgument->IsMemberPointer() && psArgument->psBase && psArgument->psBase->IsFunction())
			{
				bTakes = psParameter && psParameter->Equals(psArgument);
			}
			else if (psParameter)
			{
				iRank = ConversionRank(psParameter, apsArguments.GetPtr(i), papcArguments->GetPtr(i));
				bTakes = iRank != CPPCR_None;
				if (aiRanks)
				{
					aiRanks[i] = iRank;
				}
			}
		}

		//An argument f(...) takes is the worst match there is.  [over.ics.ellipsis]
		for (i = pcSuffix ? pcSuffix->macpParameters.NumElements() : 0; aiRanks && !bPack && pcSuffix && pcSuffix->mbVariadic && (i < papcArguments->NumElements()); i++)
		{
			aiRanks[i] = CPPCR_Ellipsis;
		}

		//The function's type, what it returns too, must be one with these arguments, or it is passed over (SFINAE:
		//typename EnableIf<B, int>::Type f(T) where B is false, or auto f(T t) -> decltype(t.m) for a T without
		//m).  [temp.deduct] 8
		pcSpecifiers = pcTemplate->FunctionSpecifiers();
		if (bTakes && pcSpecifiers && pcSpecifiers->mpcType && !(pcSuffix && pcSuffix->mpcTrailingReturn))
		{
			bTakes = mpcUnit->GetDeclarators()->GetType(pcSpecifiers, pcTemplate->FunctionDeclarator(), pcNode) != NULL;
		}
		mpcUnit->GetContext()->LeaveBindings();
		if (!pcTemplate->mpcClass)
		{
			pcBindings->mpcOuter = NULL;
		}
		if (bTakes && pcSuffix && pcSuffix->mpcTrailingReturn)
		{
			//A trailing return type names the parameters (decltype(t.m)): the instance is declared to work it out.
			pcInstance = mpcUnit->GetInstantiator()->InstantiateFunction(pcTemplate, papcExplicit, papcArguments, pcNode);
			bTakes = pcInstance && pcInstance->mpsReturn;
		}
	}
	apsArguments.Kill();
	bTakes = mpcDiagnostics->EndTrial() && bTakes;
	return bTakes;
}


size CCPPCandidates::NumElements(void) const { return masCandidates.NumElements(); }
