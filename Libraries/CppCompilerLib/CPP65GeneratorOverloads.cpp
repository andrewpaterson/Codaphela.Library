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
#include "CPP65Generator.h"


//The generator's overload resolution: finding the functions a call may mean, argument dependent lookup and ranking conversions.


//////////////////////////////////////////////////////////////////////////
//
// The error for a call no function of the name matches: the arguments' types
// and, when there is only one function of the name, its parameters'.  An
// unsigned short* passed for an unsigned long* (a 16 bit size where a 32 bit
// lsize is wanted) says so.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::NoFunctionMatches(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, SCPP65Type** apsArguments, size uiArguments, CCPPSyntax* pcNode)
{
	CCPPFunction*		pcFunction;
	CCPPFunction*		pcOnly;
	CChars				szMessage;
	size				i;
	size				uiCandidates;

	pcOnly = NULL;
	uiCandidates = 0;
	for (pcFunction = FirstFunctionOfName(lliName); pcFunction; pcFunction = pcFunction->mpcNextOfName)
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
	Error(pcNode, szMessage.Text());
	szMessage.Kill();
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
CCPPFunction* CCPP65Generator::FindFunction(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, CArraySyntax* papcArguments, bool bBases, CCPPSyntax* pcNode, int iConstObject, CArrayScopes* papcNamespaces)
{
	CCPPFunction*		pcFunction;
	CCPPFunction*		pcBest;
	SCPP65Type*			apsArguments[32];
	int					aiRanks[32];
	int					aiBest[32];
	size				uiArguments;
	size				i;
	bool				bAmbiguous;
	bool				bFound;

	mpcStatistics->Count(CPPCC_FindFunction);
	if (pcClass && !CompleteClass(pcClass, pcNode))
	{
		return NULL;
	}
	uiArguments = papcArguments ? papcArguments->NumElements() : 0;
	if (uiArguments > 32)
	{
		Error(pcNode, "Too many arguments");
		return NULL;
	}
	for (i = 0; i < uiArguments; i++)
	{
		//An overloaded function's name has no type of its own (NULL): RankArguments matches it to a parameter
		//that points to one of its overloads.
		apsArguments[i] = TypeOf(papcArguments->GetPtr(i));
		if (!apsArguments[i] && !FunctionNameOf(papcArguments->GetPtr(i)))
		{
			//Its errors.
			mcTypeChecker.Value(papcArguments->GetPtr(i));
			return NULL;
		}
	}

	//The best is found by comparing each viable function with the best so far, then
	//checked to be better than every other.
	pcBest = NULL;
	bFound = false;
	for (pcFunction = FirstFunctionOfName(lliName); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if ((pcFunction->meKind != eKind) || pcFunction->mpcTemplate || !IsOverloadOf(pcClass, pcFunction, lliName) || (papcNamespaces && (papcNamespaces->Find(pcFunction->mpcNamespace) == ARRAY_ELEMENT_NOT_FOUND)))
		{
			continue;
		}
		bFound = true;
		if (!pcFunction->TakesObject(iConstObject) || !RankArguments(pcFunction, apsArguments, papcArguments, aiRanks))
		{
			continue;
		}
		if (!pcBest || (CompareRanks(aiRanks, aiBest, uiArguments) > 0) || ((CompareRanks(aiRanks, aiBest, uiArguments) == 0) && (CompareQualifiers(pcFunction, pcBest, papcArguments, iConstObject) > 0)))
		{
			pcBest = pcFunction;
			memcpy(aiBest, aiRanks, sizeof(int) * uiArguments);
		}
	}
	bAmbiguous = false;
	for (pcFunction = pcBest ? FirstFunctionOfName(lliName) : NULL; pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if ((pcFunction == pcBest) || (pcFunction->meKind != eKind) || pcFunction->mpcTemplate || !IsOverloadOf(pcClass, pcFunction, lliName) || (papcNamespaces && (papcNamespaces->Find(pcFunction->mpcNamespace) == ARRAY_ELEMENT_NOT_FOUND)))
		{
			continue;
		}
		if (pcFunction->TakesObject(iConstObject) && RankArguments(pcFunction, apsArguments, papcArguments, aiRanks) && (CompareRanks(aiBest, aiRanks, uiArguments) <= 0))
		{
			if ((CompareRanks(aiBest, aiRanks, uiArguments) < 0) || (CompareQualifiers(pcBest, pcFunction, papcArguments, iConstObject) <= 0))
			{
				bAmbiguous = true;
			}
		}
	}

	if (pcBest && !bAmbiguous)
	{
		return pcBest;
	}
	if (bAmbiguous)
	{
		Error(pcNode, "The call is ambiguous");
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
			pcFunction = FindFunction(pcClass->macBases.Get(i)->pcClass, lliName, eKind, papcArguments, true, pcNode, iConstObject);
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
void CCPP65Generator::NormaliseNamespaces(CArrayScopes* papcNamespaces)
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
void CCPP65Generator::GetOverloadNamespaces(CCPPSyntaxName* pcName, int64 lliName, CArrayScopes* papcNamespaces)
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
void CCPP65Generator::AddNamespaceOf(CCPPScope* pcScope, CArrayScopes* papcNamespaces)
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
void CCPP65Generator::AddAssociatedNamespaces(SCPP65Type* psType, CArrayScopes* papcNamespaces, int iDepth)
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
void CCPP65Generator::AddArgumentNamespaces(CArraySyntax* papcArguments, CArrayScopes* papcNamespaces)
{
	size	i;

	for (i = 0; papcArguments && (i < papcArguments->NumElements()); i++)
	{
		if (!FunctionNameOf(papcArguments->GetPtr(i)))
		{
			AddAssociatedNamespaces(TypeOf(papcArguments->GetPtr(i)), papcNamespaces, 0);
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
void CCPP65Generator::GetCallNamespaces(CCPPSyntaxName* pcName, int64 lliName, CArraySyntax* papcArguments, CArrayScopes* papcNamespaces)
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
CCPPFunction* CCPP65Generator::FindNamedFunction(CCPPSyntaxName* pcName, int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CArrayScopes	apcNamespaces;
	CCPPFunction*	pcFunction;

	mpcStatistics->Count(CPPCC_FindNamedFunction);
	apcNamespaces.Init();
	GetCallNamespaces(pcName, lliName, papcArguments, &apcNamespaces);
	NormaliseNamespaces(&apcNamespaces);
	pcFunction = FindFunction(NULL, lliName, CPPFK_Function, papcArguments, false, pcNode, -1, &apcNamespaces);
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
CCPPFunction* CCPP65Generator::FindAssociatedFunction(CCPPSyntaxName* pcName, int64 lliName, CCPPSyntaxCallExpression* pcCall)
{
	CCPPFunction*	pcFunction;
	CCPPTemplate*	pcTemplate;
	size			uiErrors;

	mpcStatistics->Count(CPPCC_FindAssociatedFunction);
	if ((lliName == -1) || pcName->mbGlobal || pcName->GetQualifier() || (pcCall->macpArguments.NumElements() == 0))
	{
		return NULL;
	}
	uiErrors = mcErrors.NumErrors();
	pcFunction = FindNamedFunction(pcName, lliName, &pcCall->macpArguments, pcCall);
	if (pcFunction || (mcErrors.NumErrors() != uiErrors))
	{
		return pcFunction;
	}
	pcTemplate = GetCallTemplate(NULL, lliName, pcCall, NULL);
	return pcTemplate ? InstantiateFunction(pcTemplate, NULL, &pcCall->macpArguments, pcCall) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The function a name's address is of: a static member of the class that
// qualifies it or of the class of the function being written, or a function
// it can name.  The one of psTarget's type, else the only one; *pbOverloaded is
// set when the name has several and psTarget doesn't pick one.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPP65Generator::FindAddressedFunction(CCPPSyntaxName* pcName, int64 lliName, SCPP65Type* psTarget, CCPPSyntax* pcNode, bool* pbOverloaded)
{
	CCPPFunction*	pcFunction;
	CCPPFunction*	pcFound;
	CCPPClass*	pcClass;
	CArrayScopes	apcNamespaces;
	bool			bQualified;

	mpcStatistics->Count(CPPCC_FindAddressedFunction);
	pcFound = NULL;
	*pbOverloaded = false;
	pcClass = NULL;
	bQualified = pcName && pcName->IsQualifiedByClass();
	if (bQualified)
	{
		BeginQuiet();
		pcClass = GetComponentClass(pcName->GetQualifier(), NULL, pcNode);
		EndQuiet();
	}
	else if (mcContext.GetFunction() && mcContext.GetFunction()->mpcClass)
	{
		pcClass = mcContext.GetFunction()->mpcClass;
	}
	if (pcClass && CompleteClass(pcClass, pcNode))
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
	for (pcFunction = FirstFunctionOfName(lliName); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if (!pcFunction->mpcClass && !pcFunction->mpcTemplate && (!pcName || (apcNamespaces.Find(pcFunction->mpcNamespace) != ARRAY_ELEMENT_NOT_FOUND)))
		{
			if (psTarget && psTarget->Equals(mcTypes.Function(pcFunction->mpsReturn, &pcFunction->mapsParameters, pcFunction->mbVariadic)))
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
bool CCPP65Generator::RankArguments(CCPPFunction* pcFunction, SCPP65Type** apsArguments, CArraySyntax* papcArguments, int* aiRanks)
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
			aiRanks[i] = ((i < pcFunction->mapsParameters.NumElements()) && HasOverloadOfType(FunctionNameOf(papcArguments->GetPtr(i)), pcFunction->mapsParameters.GetPtr(i))) ? CPPCR_Exact : CPPCR_None;
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
int CCPP65Generator::ConversionRank(SCPP65Type* psParameter, SCPP65Type* psArgument, CCPPSyntax* pcArgument, bool bUserDefined)
{
	int64			lliValue;
	bool			bReference;
	CCPPFunction*	pcConversion;

	bReference = psParameter->IsReference();
	psParameter = bReference ? psParameter->psBase : psParameter;
	psArgument = psArgument->WithoutReference();
	if (bReference && !psParameter->bConst && !psParameter->IsClass() && pcArgument && !PlaceOf(pcArgument))
	{
		//A value with no address only binds to a const reference.
		return CPPCR_None;
	}
	if (psParameter->IsPointer() && psArgument->IsPointer() && psParameter->psBase->Equals(psArgument->psBase) && pcArgument && (SkipParentheses(pcArgument)->GetKind() == CPPS_StringExpression))
	{
		//A string literal's characters are const: char* p = "x" is allowed, but const char* is the better match.
		return psParameter->psBase->bConst ? CPPCR_Exact : CPPCR_Qualification;
	}
	if (psParameter->Equals(psArgument))
	{
		if (bReference && psArgument->IsClass() && !psArgument->bConst && pcArgument && IsMadeObject(pcArgument, psArgument->pcClass))
		{
			//An object made for the call binds best to a const reference (T& takes it too, as MSVC lets it).
			return psParameter->bConst ? CPPCR_Exact : CPPCR_Qualification;
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
		if (psParameter->pcEnum)
		{
			//Only the same enum (an exact match) converts to an enum.
			return CPPCR_None;
		}
		if ((psParameter == &mcTypes.msInt) && psArgument->IsInteger() && ((psArgument->iBytes < 4) || psArgument->pcEnum))
		{
			return CPPCR_Promotion;
		}
		if (psParameter->IsFloat() && (psParameter->iBytes == 8) && !psParameter->bLongDouble && psArgument->IsFloat() && (psArgument->iBytes == 4))
		{
			return CPPCR_Promotion;
		}
		return CPPCR_Conversion;
	}
	if (psParameter->IsPointer() && psArgument->IsPointer())
	{
		if (psParameter->psBase->HasQualifiersOf(psArgument->psBase) &&
			(psParameter->psBase->IsVoid() || (psParameter->psBase->IsClass() && psArgument->psBase->IsClass() && IsDerived(psArgument->psBase->pcClass, psParameter->psBase->pcClass, NULL))))
		{
			return CPPCR_Conversion;
		}
		return CPPCR_None;
	}
	if (psParameter->IsMemberPointer() && psArgument->IsMemberPointer())
	{
		//A pointer to a member of a base converts to a pointer to the same member of a derived class.
		if (psParameter->pcClass && psArgument->pcClass && psParameter->psBase->Equals(psArgument->psBase) && IsDerived(psParameter->pcClass, psArgument->pcClass, NULL))
		{
			return CPPCR_Conversion;
		}
		return CPPCR_None;
	}
	if ((psParameter->IsPointer() || psParameter->IsMemberPointer()) && psArgument->IsInteger() && pcArgument && Constant(pcArgument, &lliValue) && (lliValue == 0))
	{
		return CPPCR_Conversion;
	}
	if ((psParameter->eKind == CPP65TK_Bool) && (psArgument->IsPointer() || psArgument->IsMemberPointer()))
	{
		return CPPCR_Boolean;
	}
	if (psParameter->IsClass() && psArgument->IsClass() && IsDerived(psArgument->pcClass, psParameter->pcClass, NULL))
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
int CCPP65Generator::QualificationRank(SCPP65Type* psTo, SCPP65Type* psFrom, bool bTopLevel)
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
bool CCPP65Generator::HasConvertingConstructor(CCPPClass* pcClass, SCPP65Type* psArgument, CCPPSyntax* pcArgument)
{
	CCPPFunction*	pcFunction;
	SCPP65Type*		psParameter;
	size			i;

	if (!CompleteClass(pcClass, pcArgument) || !pcClass->mbConstructors)
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
// 1 if the first function's arguments convert better (none worse, one better), -1
// if the second's do, 0 if neither.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::CompareRanks(int* aiFirst, int* aiSecond, size uiArguments)
{
	size	i;
	bool	bFirst;
	bool	bSecond;

	bFirst = false;
	bSecond = false;
	for (i = 0; i < uiArguments; i++)
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
// Between two functions whose arguments convert equally well, which qualifies them
// less: of T* and const T*, or T& and const T&, the one without const, but an object
// made by the argument (a temporary) or a const object binds better to a const
// reference.  The object a member function is called on (iConstObject 1 if it is const,
// 0 if not, -1 for none) is bound to this the same way.  1 if the first does, -1 if
// the second does, 0 if neither.  [over.ics.rank] 3.2.6
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::CompareQualifiers(CCPPFunction* pcFirst, CCPPFunction* pcSecond, CArraySyntax* papcArguments, int iConstObject)
{
	size		i;
	bool		bFirst;
	bool		bSecond;
	bool		bConstBetter;
	SCPP65Type*	psParameter;
	SCPP65Type*	psOther;

	bFirst = false;
	bSecond = false;
	if ((iConstObject >= 0) && (pcFirst->mbConstThis != pcSecond->mbConstThis))
	{
		if (pcFirst->mbConstThis == (iConstObject == 1))
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
		bConstBetter = psParameter->IsReference() && ((psParameter->psBase->IsClass() && IsMadeObject(papcArguments->GetPtr(i), psParameter->psBase->pcClass)) || IsConstObject(papcArguments->GetPtr(i)));
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
// Whether an expression is an object that is const.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsConstObject(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = ValueTypeOf(pcExpression);
	return psType && psType->bConst;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an expression is a pointer to const.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsConstPointer(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = ValueTypeOf(pcExpression);
	return psType && psType->IsPointer() && psType->psBase->bConst;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the function being generated is a const member function.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsConstThis(void)
{
	return mcContext.GetFunction() && mcContext.GetFunction()->mbConstThis;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcClass is psBase or derives from it.  *piOffset (if not NULL) is set to
// where psBase is in pcClass.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsDerived(CCPPClass* pcClass, CCPPClass* pcBase, int* piOffset)
{
	SCPPBase*	psDirect;
	size		i;
	int			iOffset;

	if ((pcClass != pcBase) && !pcClass->IsComplete())
	{
		//The bases are enough to know; their offsets need the class laid out.
		BeginQuiet();
		ResolveBases(pcClass, NULL);
		EndQuiet();
	}
	if (pcClass == pcBase)
	{
		if (piOffset)
		{
			*piOffset = 0;
		}
		return true;
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psDirect = pcClass->macBases.Get(i);
		if (IsDerived(psDirect->pcClass, pcBase, &iOffset))
		{
			if (piOffset)
			{
				*piOffset = psDirect->iOffset + iOffset;
			}
			return true;
		}
	}
	return false;
}
