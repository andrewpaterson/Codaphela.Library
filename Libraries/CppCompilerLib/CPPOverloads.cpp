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
	mcResolved.Init(true);
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
	size			j;
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
		for (j = 0; j < pcOwn->mapsParameters.NumElements(); j++)
		{
			if (!pcOwn->mapsParameters.GetPtr(j)->Equals(pcFunction->mapsParameters.GetPtr(j)))
			{
				break;
			}
		}
		if (j == pcOwn->mapsParameters.NumElements())
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The conversion function (operator T, its class's own or a base's) that converts
// an object of pcClass to psTo best: by its result's conversion to psTo (see
// ConversionRank; a condition is a conversion to bool).  explicit ones only when
// bExplicit (a cast or a condition).  NULL, quietly, if none converts.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::FindConversion(CCPPClass* pcClass, SCPP65Type* psTo, bool bExplicit, CCPPSyntax* pcNode)
{
	CCPPFunction*	pcFunction;
	CCPPFunction*	pcBest;
	int				iRank;
	int				iBest;

	mpcUnit->GetStatistics()->Count(CPPCC_FindConversion);
	if (!mpcUnit->CompleteClass(pcClass, pcNode))
	{
		return NULL;
	}
	pcBest = NULL;
	iBest = CPPCR_None;
	for (pcFunction = mpcUnit->FirstFunctionOfName(CPP_CONVERSION_NAME); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if (pcFunction->mpcTemplate || (pcFunction->mbExplicit && !bExplicit) || !mpcUnit->IsDerived(pcClass, pcFunction->mpcClass, NULL))
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
	return pcBest;
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
	return pcFound && psPointer->psBase->Equals(mpcUnit->GetTypes()->Function(pcFound->mpsReturn, &pcFound->mapsParameters, pcFound->mbVariadic));
}


//////////////////////////////////////////////////////////////////////////
//
// The static member function of the name in the class, or else in its bases (the
// first base that has one).  *pbOverloaded is set when the name has several and
// psTarget does not pick one.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::FindStaticFunction(CCPPClass* pcClass, int64 lliName, SCPP65Type* psTarget, bool* pbOverloaded)
{
	CCPPFunction*		pcFunction;
	CCPPFunction*		pcFound;
	size				i;

	mpcUnit->GetStatistics()->Count(CPPCC_FindStaticFunction);
	pcFound = NULL;
	*pbOverloaded = false;
	for (pcFunction = mpcUnit->FirstFunctionOfName(lliName); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if ((pcFunction->mpcClass == pcClass) && pcFunction->mbStatic && (pcFunction->meKind == CPPFK_Function) && !pcFunction->mpcTemplate)
		{
			if (psTarget && psTarget->Equals(mpcUnit->GetTypes()->Function(pcFunction->mpsReturn, &pcFunction->mapsParameters, pcFunction->mbVariadic)))
			{
				*pbOverloaded = false;
				return pcFunction;
			}
			*pbOverloaded |= (pcFound != NULL);
			pcFound = pcFunction;
		}
	}
	if (pcFound)
	{
		return pcFound;
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		pcFound = FindStaticFunction(pcClass->macBases.Get(i)->pcClass, lliName, psTarget, pbOverloaded);
		if (pcFound || *pbOverloaded)
		{
			return pcFound;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The operator function of pcClass (or a base) for the arguments: a member function that takes them, or else an
// instance of a member template.  A class without a member template reports errors as FindFunction does.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::FindOperator(CCPPClass* pcClass, int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode, ECPPCalledObject eObject)
{
	CCPPTemplate*	pcTemplate;
	CCPPFunction*	pcFunction;
	size			uiErrors;

	mpcUnit->GetStatistics()->Count(CPPCC_FindOperator);
	pcTemplate = FindMemberTemplate(pcClass, lliName, papcArguments ? papcArguments->NumElements() : 0, eObject);
	if (!pcTemplate)
	{
		return FindFunction(pcClass, lliName, CPPFK_Function, papcArguments, true, pcNode, eObject);
	}

	uiErrors = mpcUnit->GetErrors()->NumErrors();
	mpcUnit->BeginQuiet();
	pcFunction = FindFunction(pcClass, lliName, CPPFK_Function, papcArguments, true, pcNode, eObject);
	if (!pcFunction)
	{
		pcFunction = mpcUnit->InstantiateFunction(pcTemplate, NULL, papcArguments, pcNode);
	}
	mpcUnit->EndQuiet();
	mpcUnit->GetErrors()->Truncate(uiErrors);
	return pcFunction;
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

	pcFunction = mpcUnit->OuterFunction();
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
// The operator function (not a member) for the arguments: a declared one, or an
// instance of a function template of that name.  Those of the namespaces around
// the function being written (without the hiding names have) and those the
// arguments are associated with.  NULL, quietly, if none matches.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::FindFreeOperator(int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CCPPFunction*					pcFunction;
	CCPPTemplate*					pcTemplate;
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPSyntaxName*					pcName;
	CCPPScope*						pcLookup;
	CArrayScopes					apcNamespaces;
	size							uiErrors;
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

	uiErrors = mpcUnit->GetErrors()->NumErrors();
	pcFunction = NULL;
	mpcUnit->BeginQuiet();
	apcNamespaces.Init();
	pcLookup = CurrentLookupScope();
	if (pcLookup)
	{
		pcLookup->GatherEnclosingNamespaces(&apcNamespaces);
	}
	AddArgumentNamespaces(papcArguments, &apcNamespaces);
	NormaliseNamespaces(&apcNamespaces);
	if (apcNamespaces.Find(NULL) == ARRAY_ELEMENT_NOT_FOUND)
	{
		apcNamespaces.Add(NULL);
	}
	if (bDeclared)
	{
		pcFunction = FindFunction(NULL, lliName, CPPFK_Function, papcArguments, false, pcNode, CPPCO_None, &apcNamespaces);
	}
	for (i = 0; !pcFunction && (i < mpcUnit->GetTemplates()->NumElements()); i++)
	{
		pcTemplate = mpcUnit->GetTemplates()->GetPtr(i);
		if (pcTemplate->mbMember || (pcTemplate->mpcPrimary != pcTemplate) || !pcTemplate->mpcDefinition || (pcTemplate->mpcDefinition->GetKind() != CPPS_FunctionDefinition))
		{
			continue;
		}
		pcDefinition = (CCPPSyntaxFunctionDefinition*)pcTemplate->mpcDefinition;
		pcName = pcDefinition->mpcDeclarator ? pcDefinition->mpcDeclarator->GetName() : NULL;
		if (pcName && (pcName->macpComponents.NumElements() == 1) && (mpcUnit->GetFunctionName(pcName->GetLast()) == lliName) && (apcNamespaces.Find(pcTemplate->mpcNamespace) != ARRAY_ELEMENT_NOT_FOUND))
		{
			pcFunction = mpcUnit->InstantiateFunction(pcTemplate, NULL, papcArguments, pcNode);
		}
	}
	apcNamespaces.Kill();
	mpcUnit->EndQuiet();
	mpcUnit->GetErrors()->Truncate(uiErrors);
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
	mpcUnit->Error(pcNode, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The best of the candidates: the one whose every argument converts at least as well as
// for any other, and one better, or between two that convert equally well the one that
// qualifies them less (CompareQualifiers).  It is found by comparing each with the best
// so far, then checked to be better than every other: *pbAmbiguous if it is not.  NULL
// if there are no candidates.  [over.match.best]
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::ChooseFunction(CCPPCandidates* pcCandidates, CArraySyntax* papcArguments, ECPPCalledObject eObject, bool* pbAmbiguous)
{
	size	uiBest;
	size	i;
	int		iRanks;

	*pbAmbiguous = false;
	if (pcCandidates->NumElements() == 0)
	{
		return NULL;
	}
	uiBest = 0;
	for (i = 1; i < pcCandidates->NumElements(); i++)
	{
		iRanks = pcCandidates->Compare(i, uiBest);
		if ((iRanks > 0) || ((iRanks == 0) && (CompareQualifiers(pcCandidates->GetFunction(i), pcCandidates->GetFunction(uiBest), papcArguments, eObject) > 0)))
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
		iRanks = pcCandidates->Compare(uiBest, i);
		if ((iRanks < 0) || ((iRanks == 0) && (CompareQualifiers(pcCandidates->GetFunction(uiBest), pcCandidates->GetFunction(i), papcArguments, eObject) <= 0)))
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
// better (see ConversionRank).  Returns NULL with an error if no one is best, or
// none matches functions of the name; NULL without an error if there are none.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::FindFunction(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, CArraySyntax* papcArguments, bool bBases, CCPPSyntax* pcNode, ECPPCalledObject eObject, CArrayScopes* papcNamespaces)
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
	if (pcClass && !mpcUnit->CompleteClass(pcClass, pcNode))
	{
		return NULL;
	}
	uiArguments = papcArguments ? papcArguments->NumElements() : 0;
	if (uiArguments > 32)
	{
		mpcUnit->Error(pcNode, "Too many arguments");
		return NULL;
	}
	for (i = 0; i < uiArguments; i++)
	{
		//An overloaded function's name has no type of its own (NULL): RankArguments matches it to a parameter
		//that points to one of its overloads.
		apsArguments[i] = mpcUnit->TypeOf(papcArguments->GetPtr(i));
		if (!apsArguments[i] && !mpcUnit->FunctionNameOf(papcArguments->GetPtr(i)))
		{
			//Its errors.
			mpcUnit->GetTypeChecker()->Value(papcArguments->GetPtr(i));
			return NULL;
		}
	}

	//The candidates: the functions of the name that the call can see, that take its arguments.
	cCandidates.Init(uiArguments);
	uiLooked = 0;
	for (pcFunction = mpcUnit->FirstFunctionOfName(lliName); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if ((pcFunction->meKind != eKind) || pcFunction->mpcTemplate || !IsOverloadOf(pcClass, pcFunction, lliName) || (papcNamespaces && (papcNamespaces->Find(pcFunction->mpcNamespace) == ARRAY_ELEMENT_NOT_FOUND)))
		{
			continue;
		}
		uiLooked++;
		if (pcFunction->TakesObject(eObject) && RankArguments(pcFunction, apsArguments, papcArguments, cCandidates.NextRanks()))
		{
			cCandidates.Add(pcFunction, NULL);
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
			mpcUnit->Error(pcNode, "The function is declared auto and what it returns isn't known until it is defined");
			return NULL;
		}
		if (pcBest->mbDeleted)
		{
			//It was the best match, and so the call is wrong: that is what deleting it is for.  [dcl.fct.def.delete]
			mpcUnit->Error(pcNode, "The function chosen is deleted");
			return NULL;
		}
		return pcBest;
	}
	if (bAmbiguous)
	{
		mpcUnit->Error(pcNode, "The call is ambiguous");
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
			pcFunction = FindFunction(pcClass->macBases.Get(i)->pcClass, lliName, eKind, papcArguments, true, pcNode, eObject);
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
// The namespaces as functions have them: NULL for the global one.
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloads::NormaliseNamespaces(CArrayScopes* papcNamespaces)
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
void CCPPOverloads::GetOverloadNamespaces(CCPPSyntaxName* pcName, int64 lliName, CArrayScopes* papcNamespaces)
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
// Adds the namespace pcScope is in, if it isn't there already.
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloads::AddNamespaceOf(CCPPScope* pcScope, CArrayScopes* papcNamespaces)
{
	CCPPScope*	pcNamespace;

	pcNamespace = pcScope->GetEnclosingNamespace();
	if (papcNamespaces->Find(pcNamespace) == ARRAY_ELEMENT_NOT_FOUND)
	{
		papcNamespaces->Add(pcNamespace);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the namespaces associated with a type: a class's, its bases' and its
// template arguments', an enum's, and those of what a pointer, reference, array
// or function type is made from.  [basic.lookup.argdep] 2
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloads::AddAssociatedNamespaces(SCPP65Type* psType, CArrayScopes* papcNamespaces, int iDepth)
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
// Adds the namespaces associated with the types of a call's arguments.
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloads::AddArgumentNamespaces(CArraySyntax* papcArguments, CArrayScopes* papcNamespaces)
{
	size	i;

	for (i = 0; papcArguments && (i < papcArguments->NumElements()); i++)
	{
		if (!mpcUnit->FunctionNameOf(papcArguments->GetPtr(i)))
		{
			AddAssociatedNamespaces(mpcUnit->TypeOf(papcArguments->GetPtr(i)), papcNamespaces, 0);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The namespaces a call's function can be in: what the name finds, and for
// a name that isn't qualified the namespaces its arguments are associated with.
// [basic.lookup.argdep] 3
//
//////////////////////////////////////////////////////////////////////////
void CCPPOverloads::GetCallNamespaces(CCPPSyntaxName* pcName, int64 lliName, CArraySyntax* papcArguments, CArrayScopes* papcNamespaces)
{
	GetOverloadNamespaces(pcName, lliName, papcNamespaces);
	if (!pcName->mbGlobal && !pcName->GetQualifier())
	{
		AddArgumentNamespaces(papcArguments, papcNamespaces);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The function a call of a name that isn't a member's means.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPOverloads::FindNamedFunction(CCPPSyntaxName* pcName, int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CArrayScopes	apcNamespaces;
	CCPPFunction*	pcFunction;

	mpcUnit->GetStatistics()->Count(CPPCC_FindNamedFunction);
	apcNamespaces.Init();
	GetCallNamespaces(pcName, lliName, papcArguments, &apcNamespaces);
	NormaliseNamespaces(&apcNamespaces);
	pcFunction = FindFunction(NULL, lliName, CPPFK_Function, papcArguments, false, pcNode, CPPCO_None, &apcNamespaces);
	apcNamespaces.Kill();
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
	uiErrors = mpcUnit->GetErrors()->NumErrors();
	pcFunction = FindNamedFunction(pcName, lliName, papcArguments, pcCall);
	if (pcFunction || (mpcUnit->GetErrors()->NumErrors() != uiErrors))
	{
		return pcFunction;
	}
	pcTemplate = GetCallTemplate(NULL, lliName, pcCall, papcArguments, NULL);
	return pcTemplate ? mpcUnit->InstantiateFunction(pcTemplate, NULL, papcArguments, pcCall) : NULL;
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
	CArrayScopes	apcNamespaces;
	bool			bQualified;

	mpcUnit->GetStatistics()->Count(CPPCC_FindAddressedFunction);
	pcFound = NULL;
	*pbOverloaded = false;
	pcClass = NULL;
	bQualified = pcName && pcName->IsQualifiedByClass();
	if (bQualified)
	{
		mpcUnit->BeginQuiet();
		pcClass = mpcUnit->GetComponentClass(pcName->GetQualifier(), NULL, pcNode);
		mpcUnit->EndQuiet();
	}
	else if (mpcUnit->OuterFunction() && mpcUnit->OuterFunction()->mpcClass)
	{
		pcClass = mpcUnit->OuterFunction()->mpcClass;
	}
	if (pcClass && mpcUnit->CompleteClass(pcClass, pcNode))
	{
		pcFound = FindStaticFunction(pcClass, lliName, psTarget, pbOverloaded);
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
		if (!pcFunction->mpcClass && !pcFunction->mpcTemplate && (!pcName || (apcNamespaces.Find(pcFunction->mpcNamespace) != ARRAY_ELEMENT_NOT_FOUND)))
		{
			if (psTarget && psTarget->Equals(mpcUnit->GetTypes()->Function(pcFunction->mpsReturn, &pcFunction->mapsParameters, pcFunction->mbVariadic)))
			{
				pcFound = pcFunction;
				*pbOverloaded = false;
				break;
			}
			*pbOverloaded |= (pcFound != NULL);
			pcFound = pcFunction;
		}
	}
	apcNamespaces.Kill();
	return pcFound;
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
	if ((uiArguments < pcFunction->RequiredParameters()) || ((uiArguments > pcFunction->mapsParameters.NumElements()) && !pcFunction->mbVariadic))
	{
		return false;
	}
	for (i = 0; i < uiArguments; i++)
	{
		if (!apsArguments[i])
		{
			//An overloaded function's name (FindFunction) is an exact match for a pointer to one of its overloads.
			aiRanks[i] = ((i < pcFunction->mapsParameters.NumElements()) && HasOverloadOfType(mpcUnit->FunctionNameOf(papcArguments->GetPtr(i)), pcFunction->mapsParameters.GetPtr(i))) ? CPPCR_Exact : CPPCR_None;
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
// How well an argument converts to a parameter: an exact match (the same type, an
// array or function to a pointer to it), a promotion (bool, char, short and enums to
// int, float to double), a conversion (between other arithmetic types, a derived class
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
	CCPPFunction*	pcConversion;

	bReference = psParameter->IsReference();
	bRvalue = psParameter->IsRvalueReference();
	psParameter = bReference ? psParameter->psBase : psParameter;
	psArgument = psArgument->WithoutReference();
	if (bRvalue)
	{
		if (pcArgument && !mpcUnit->IsRvalue(pcArgument))
		{
			//What has a name doesn't bind to T&&.  [dcl.init.ref] 5.2
			return CPPCR_None;
		}
	}
	else if (bReference && !psParameter->bConst && !psParameter->IsClass() && pcArgument && mpcUnit->IsRvalue(pcArgument))
	{
		//A value with no name only binds to a const reference or T&&.
		return CPPCR_None;
	}
	if (psParameter->IsPointer() && psArgument->IsPointer() && psParameter->psBase->Equals(psArgument->psBase) && pcArgument && (mpcUnit->SkipParentheses(pcArgument)->GetKind() == CPPS_StringExpression))
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
		if (psParameter->pcEnum || (psArgument->IsScopedEnum() && !mpcUnit->IsRecordedEnum(psArgument)))
		{
			//Only the same enum (an exact match) converts to an enum, and an enum class's value to nothing else.
			return CPPCR_None;
		}
		if ((psParameter == &mpcUnit->GetTypes()->msInt) && psArgument->IsInteger() && ((psArgument->iBytes < 4) || psArgument->pcEnum))
		{
			return CPPCR_Promotion;
		}
		if (psParameter->IsFloat() && (psParameter->iBytes == 8) && !psParameter->bLongDouble && psArgument->IsFloat() && (psArgument->iBytes == 4))
		{
			return CPPCR_Promotion;
		}
		return CPPCR_Conversion;
	}
	if ((psParameter->IsPointer() || psParameter->IsMemberPointer()) && pcArgument && mpcUnit->IsNullPointerLiteral(pcArgument))
	{
		//nullptr is a null pointer of any type.  [conv.ptr] 1
		return CPPCR_Conversion;
	}
	if (psParameter->IsPointer() && psArgument->IsPointer())
	{
		if (psParameter->psBase->HasQualifiersOf(psArgument->psBase) &&
			(psParameter->psBase->IsVoid() || (psParameter->psBase->IsClass() && psArgument->psBase->IsClass() && mpcUnit->IsDerived(psArgument->psBase->pcClass, psParameter->psBase->pcClass, NULL))))
		{
			return CPPCR_Conversion;
		}
		return CPPCR_None;
	}
	if (psParameter->IsMemberPointer() && psArgument->IsMemberPointer())
	{
		//A pointer to a member of a base converts to a pointer to the same member of a derived class.
		if (psParameter->pcClass && psArgument->pcClass && psParameter->psBase->Equals(psArgument->psBase) && mpcUnit->IsDerived(psParameter->pcClass, psArgument->pcClass, NULL))
		{
			return CPPCR_Conversion;
		}
		return CPPCR_None;
	}
	if ((psParameter->IsPointer() || psParameter->IsMemberPointer()) && psArgument->IsInteger() && pcArgument && mpcUnit->Constant(pcArgument, &lliValue) && (lliValue == 0))
	{
		return CPPCR_Conversion;
	}
	if ((psParameter->eKind == CPP65TK_Bool) && (psArgument->IsPointer() || psArgument->IsMemberPointer()))
	{
		return CPPCR_Boolean;
	}
	if (psParameter->IsClass() && psArgument->IsClass() && mpcUnit->IsDerived(psArgument->pcClass, psParameter->pcClass, NULL))
	{
		return (!bReference || psParameter->HasQualifiersOf(psArgument)) ? CPPCR_Conversion : CPPCR_None;
	}
	if (bUserDefined && psParameter->IsClass() && HasConvertingConstructor(psParameter->pcClass, psArgument, pcArgument))
	{
		return CPPCR_UserDefinedExact;
	}
	if (bUserDefined && !bReference && !psParameter->IsClass() && psArgument->IsClass())
	{
		//The conversion function's result is then converted to the parameter, which tells overloads apart.
		pcConversion = FindConversion(psArgument->pcClass, psParameter, false, pcArgument);
		if (pcConversion)
		{
			return (ConversionRank(psParameter, pcConversion->mpsReturn, NULL, false) >= CPPCR_Promotion) ? CPPCR_UserDefinedExact : CPPCR_UserDefined;
		}
	}
	return CPPCR_None;
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
// through a standard conversion (any other parameters have defaults).  It is not
// the class's copy constructor or explicit.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::HasConvertingConstructor(CCPPClass* pcClass, SCPP65Type* psArgument, CCPPSyntax* pcArgument)
{
	CCPPFunction*	pcFunction;
	SCPP65Type*		psParameter;
	size			i;

	if (!mpcUnit->CompleteClass(pcClass, pcArgument) || !pcClass->mbConstructors)
	{
		return false;
	}
	for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
	{
		pcFunction = pcClass->mapcMembers.GetPtr(i);
		if ((pcFunction->meKind != CPPFK_Constructor) || pcFunction->mpcTemplate || pcFunction->mbExplicit || (pcFunction->mapsParameters.NumElements() == 0))
		{
			continue;
		}
		if (pcFunction->RequiredParameters() > 1)
		{
			continue;
		}
		psParameter = pcFunction->mapsParameters.GetPtr(0);
		psParameter = psParameter->WithoutReference();
		if (psParameter->IsClass() && (psParameter->pcClass == pcClass))
		{
			continue;
		}
		if (ConversionRank(pcFunction->mapsParameters.GetPtr(0), psArgument, pcArgument, false) != CPPCR_None)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an argument is an object of the class made for the call: what a call or an
// operator returns (IsMadeObject), or one constructed where it is passed, S() and
// S(1).  It has no name to change it by afterwards, so it binds better to a const
// reference.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::IsTemporary(CCPPSyntax* pcArgument, CCPPClass* pcClass)
{
	SCPP65Type*	psType;

	if (mpcUnit->IsMadeObject(pcArgument, pcClass))
	{
		return true;
	}
	pcArgument = pcArgument->WithoutParentheses();
	if (!pcArgument || (pcArgument->GetKind() != CPPS_ConstructExpression))
	{
		return false;
	}
	psType = mpcUnit->TypeOf(pcArgument);
	return psType && psType->IsClass() && (psType->pcClass == pcClass);
}


//////////////////////////////////////////////////////////////////////////
//
// Between two functions whose arguments convert equally well, which qualifies them
// less: of T* and const T*, or T& and const T&, the one without const, but an object
// made by the argument (a temporary) or a const object binds better to a const
// reference.  The object a member function is called on (eObject) is bound to this
// the same way.  1 if the first does, -1 if the second does, 0 if neither.
// [over.ics.rank] 3.2.6
//
//////////////////////////////////////////////////////////////////////////
int CCPPOverloads::CompareQualifiers(CCPPFunction* pcFirst, CCPPFunction* pcSecond, CArraySyntax* papcArguments, ECPPCalledObject eObject)
{
	size		i;
	bool		bFirst;
	bool		bSecond;
	bool		bConstBetter;
	SCPP65Type*	psParameter;
	SCPP65Type*	psOther;

	bFirst = false;
	bSecond = false;
	if (CPPCalledRvalue(eObject) && (pcFirst->meRefQualifier != pcSecond->meRefQualifier))
	{
		//An object that is going is taken by f() && before f() const &, as T&& before const T&.
		if (pcFirst->meRefQualifier == CPPRQ_Rvalue)
		{
			bFirst = true;
		}
		else if (pcSecond->meRefQualifier == CPPRQ_Rvalue)
		{
			bSecond = true;
		}
	}
	else if ((eObject != CPPCO_None) && (pcFirst->mbConstThis != pcSecond->mbConstThis))
	{
		if (pcFirst->mbConstThis == CPPCalledConst(eObject))
		{
			bFirst = true;
		}
		else
		{
			bSecond = true;
		}
	}
	for (i = 0; papcArguments && (i < papcArguments->NumElements()) && (i < pcFirst->mapsParameters.NumElements()) && (i < pcSecond->mapsParameters.NumElements()); i++)
	{
		psParameter = pcFirst->mapsParameters.GetPtr(i);
		psOther = pcSecond->mapsParameters.GetPtr(i);
		if (!((psParameter->IsPointer() && psOther->IsPointer()) || (psParameter->IsReference() && psOther->IsReference())) || (psParameter->psBase->bConst == psOther->psBase->bConst) || !psParameter->psBase->Equals(psOther->psBase))
		{
			continue;
		}
		bConstBetter = psParameter->IsReference() && ((psParameter->psBase->IsClass() && IsTemporary(papcArguments->GetPtr(i), psParameter->psBase->pcClass)) || mpcUnit->IsConstObject(papcArguments->GetPtr(i)));
		if (psParameter->psBase->bConst == bConstBetter)
		{
			bFirst = true;
		}
		else
		{
			bSecond = true;
		}
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
// Whether a function with these parameters takes uiArguments arguments: defaults
// make up the rest, and ... or a function parameter pack (Args ... args) take any
// number more.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::TakesArguments(CCPPSyntaxFunctionSuffix* pcSuffix, size uiArguments)
{
	CCPPSyntaxParameter*	pcLast;
	size					uiParameters;
	size					uiRequired;

	uiParameters = pcSuffix->NumParameters();
	uiRequired = pcSuffix->RequiredParameters();
	pcLast = (uiParameters > 0) ? (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(uiParameters - 1) : NULL;
	if (pcLast && pcLast->mpcDeclarator && pcLast->mpcDeclarator->mbPack)
	{
		return uiArguments + 1 >= uiRequired;
	}
	return (uiArguments >= uiRequired) && ((uiArguments <= uiParameters) || pcSuffix->mbVariadic);
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
		if (pcSuffix && TakesArguments(pcSuffix, uiArguments))
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
	CArrayScopes			apcNamespaces;
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
		pcTemplate = mpcUnit->GetTemplate(pcSymbol);
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
			if (pcTemplate->mbMember || (pcTemplate->mpcPrimary != pcTemplate) || (pcTemplate->mpcSymbol != pcDeclaration) || !pcTemplate->mpcDefinition || (pcTemplate->mpcDefinition->GetKind() != CPPS_FunctionDefinition))
			{
				continue;
			}
			pcSuffix = ((CCPPSyntaxFunctionDefinition*)pcTemplate->mpcDefinition)->mpcDeclarator->GetFunctionSuffix();
			if (pcSuffix && TakesArguments(pcSuffix, papcArguments->NumElements()))
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
// f(T*) before f(T) for a pointer).  Then the first whose const matches the object's
// (eObject) is chosen, or else the first.  pcCall is NULL to choose by the number
// of arguments alone.
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPOverloads::ChooseTemplate(CArrayCPPTemplates* papcCandidates, ECPPCalledObject eObject, CArraySyntax* papcExplicit, CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments)
{
	CCPPCandidates			cTaken;
	CArrayCPPTemplates		apcChosen;
	CArrayCPPTemplates*		papcChosen;
	CCPPTemplate*			pcTemplate;
	size					uiArguments;
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
	papcChosen = (apcChosen.NumElements() > 0) ? &apcChosen : papcCandidates;

	pcTemplate = (papcChosen->NumElements() > 0) ? papcChosen->GetPtr(0) : NULL;
	for (i = 0; (eObject != CPPCO_None) && (i < papcChosen->NumElements()); i++)
	{
		if (mpcUnit->IsConstSuffix(papcChosen->GetPtr(i)->mpcDeclarator->GetFunctionSuffix()) == CPPCalledConst(eObject))
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
// pcSecond's parameters are then those types.  [temp.deduct.partial]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPOverloads::AtLeastAsSpecialised(CCPPTemplate* pcFirst, CCPPTemplate* pcSecond, size uiArguments, CCPPSyntax* pcNode)
{
	CArrayCPP65Types		apsFirst;
	CArrayCPP65Types		apsSecond;
	CCPPTemplateArguments*	pcBindings;
	CCPPDeduction			cDeduction;
	CChars					szArguments;
	size					i;
	bool					bResult;

	mpcUnit->BeginTrial();
	apsFirst.Init();
	apsSecond.Init();
	szArguments.Init();
	pcBindings = mpcUnit->BindAsItself(pcFirst->mpcDeclaration, &szArguments, pcNode);
	bResult = pcBindings && TemplateParameterTypes(pcFirst, pcBindings, uiArguments, &apsFirst);
	szArguments.Clear();
	if (bResult)
	{
		cDeduction.Init(mpcUnit, pcSecond);
		cDeduction.Deduce(&apsFirst, NULL);
		pcBindings = mpcUnit->BindArguments(pcSecond->mpcDeclaration, NULL, cDeduction.GetTypes(), &szArguments, pcNode, cDeduction.GetValues());
		cDeduction.Kill();
		bResult = pcBindings && TemplateParameterTypes(pcSecond, pcBindings, uiArguments, &apsSecond) && (apsSecond.NumElements() == apsFirst.NumElements());
	}
	for (i = 0; bResult && (i < apsFirst.NumElements()); i++)
	{
		//A parameter taken by value takes its argument's type without its own const.  [temp.deduct.partial] 7
		bResult = apsSecond.GetPtr(i)->WithoutReference()->Unqualified()->Same(apsFirst.GetPtr(i)->WithoutReference()->Unqualified());
	}
	szArguments.Kill();
	apsSecond.Kill();
	apsFirst.Kill();
	return mpcUnit->EndTrial() && bResult;
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
	bool						bTakes;
	bool						bPack;
	int							iRank;
	size						i;

	for (i = 0; aiRanks && (i < papcArguments->NumElements()); i++)
	{
		aiRanks[i] = CPPCR_Exact;
	}

	//A failure here is not one of the quiet code around it.
	mpcUnit->BeginTrial();
	apsArguments.Init();
	pcBindings = NULL;
	bTakes = !pcTemplate->mpcClass || mpcUnit->DefineMemberTemplate(pcTemplate, pcNode);
	if (bTakes)
	{
		for (i = 0; i < papcArguments->NumElements(); i++)
		{
			apsArguments.Add(mpcUnit->TypeOf(papcArguments->GetPtr(i)));
		}
		cDeduction.Init(mpcUnit, pcTemplate);
		cDeduction.Deduce(&apsArguments, papcArguments);
		szArguments.Init();
		pcBindings = mpcUnit->BindArguments(pcTemplate->mpcDeclaration, papcExplicit, cDeduction.GetTypes(), &szArguments, pcNode, cDeduction.GetValues());
		szArguments.Kill();
		cDeduction.Kill();
		bTakes = pcBindings != NULL;
	}

	if (bTakes)
	{
		//The parameters' types are worked out as InstantiateFunction declares them.
		if (pcTemplate->mpcClass)
		{
			pcBindings->mpcOuter = pcTemplate->mpcClassDeclaration ? mpcUnit->RenameBindings(pcTemplate->mpcClass, pcTemplate->mpcClassDeclaration) : pcTemplate->mpcClass->mpcBindings;
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
		//typename EnableIf<B, int>::Type f(T) where B is false).  [temp.deduct] 8
		pcSpecifiers = pcTemplate->FunctionSpecifiers();
		if (bTakes && pcSpecifiers && pcSpecifiers->mpcType)
		{
			bTakes = mpcUnit->GetDeclarators()->GetType(pcSpecifiers, pcTemplate->FunctionDeclarator(), pcNode) != NULL;
		}
		mpcUnit->GetContext()->LeaveBindings();
		if (!pcTemplate->mpcClass)
		{
			pcBindings->mpcOuter = NULL;
		}
	}
	apsArguments.Kill();
	bTakes = mpcUnit->EndTrial() && bTakes;
	return bTakes;
}

