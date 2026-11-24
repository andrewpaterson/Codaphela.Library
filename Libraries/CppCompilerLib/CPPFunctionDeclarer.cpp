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
#include "CPPFunctionDeclarer.h"


//////////////////////////////////////////////////////////////////////////
//
// The unit's types, diagnostics, declarators and context are its own,
// asked for once.
//
//////////////////////////////////////////////////////////////////////////
void CCPPFunctionDeclarer::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
	mpcTypes = pcUnit->GetTypes();
	mpcDiagnostics = pcUnit->GetDiagnostics();
	mpcDeclarators = pcUnit->GetDeclarators();
	mpcContext = pcUnit->GetContext();
	masFailed.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPFunctionDeclarer::Kill(void)
{
	masFailed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Records a function's signature (DeclareSignature) and, where its return
// type is written after it or left to its body (auto), works that out.
// Returns NULL after an error.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPFunctionDeclarer::Declare(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, bool bDefinition, CCPPClass* pcClass, bool bPure, CCPPTemplate* pcTemplate, CChars* pszArguments)
{
	CCPPFunction*	pcFunction;

	pcFunction = DeclareSignature(pcSpecifiers, pcDeclarator, bDefinition, pcClass, bPure, pcTemplate, pszArguments);
	if (pcFunction && (pcFunction->mpsReturn == &mpcTypes->msUndeduced) && !ResolveReturn(pcFunction, pcSpecifiers, pcDeclarator))
	{
		return NULL;
	}
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// Records a function's signature, a member of pcClass if it is not NULL, or an
// instance of pcTemplate.  A function may be declared any number of times but
// defined once.  Functions with the same name and different parameters are
// overloads.  Returns NULL after an error.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPFunctionDeclarer::DeclareSignature(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, bool bDefinition, CCPPClass* pcClass, bool bPure, CCPPTemplate* pcTemplate, CChars* pszArguments)
{
	int64						lliName;
	CCPPFunction*				pcFunction;
	CCPPFunction*				pcOther;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	SCPP65Type*					psType;
	SCPP65Type*					psReturn;
	ECPPFunctionKind			eKind;
	CArrayCPP65Types			apsParameters;
	CArraySyntax				apcDefaults;
	CArrayCPPBindingPtrs		apsPacks;
	CCPPPackElements			cElements;
	size						i;
	size						j;
	size						uiPack;
	uint32						uiFlags;
	bool						bConstThis;
	bool						bVolatileThis;
	bool						bUndeduced;
	bool						bSameParameters;
	SCPP65Type*					psAbstract;
	ECPPRefQualifier			eRefQualifier;
	bool						bFriend;
	CCPPScope*					pcNamespace;
	CCPPSyntaxDeclSpecifiers*	pcTypeSpecifiers;

	pcName = pcDeclarator->GetName();
	pcComponent = pcName ? pcName->GetLast() : NULL;
	if (!pcComponent || ((pcComponent->meKind == CPPNC_Conversion) && !pcClass) || (!pcClass && !pcTemplate && pcName->IsQualifiedByClass()))
	{
		mpcDiagnostics->Error(pcDeclarator, "Conversion functions outside classes and names qualified by what isn't a namespace are not supported by the compiler yet");
		return NULL;
	}
	lliName = mpcUnit->GetFunctionName(pcComponent);
	if (lliName == -1)
	{
		mpcDiagnostics->Error(pcDeclarator, "This function name is not supported by the compiler yet");
		return NULL;
	}
	if (!pcClass && !pcTemplate && bDefinition && !IsDefinedWhereDeclared(pcName))
	{
		mpcDiagnostics->Error(pcDeclarator, "The function is defined in a namespace that doesn't enclose it");
		return NULL;
	}
	eKind = CPPFK_Function;
	if (pcComponent->meKind == CPPNC_Destructor)
	{
		eKind = CPPFK_Destructor;
	}
	else if (pcClass && (lliName == pcClass->mlliName))
	{
		eKind = CPPFK_Constructor;
	}

	//typedef int Binary(int, int); Binary Add; declares a function by the name of its type: the typedef's declarator
	//has its parameters, and the typedef's specifiers what it returns.  [dcl.fct] 10
	pcTypeSpecifiers = pcSpecifiers;
	if (!pcDeclarator->GetFunctionSuffix() && !(pcSpecifiers && pcSpecifiers->NamesFunctionType(&pcTypeSpecifiers, &pcDeclarator)))
	{
		mpcDiagnostics->Error(pcDeclarator, "This function declarator is not supported by the compiler yet");
		return NULL;
	}
	pcSuffix = pcDeclarator->GetFunctionSuffix();
	if (!pcDeclarator->mpcNested && (pcDeclarator->macpSuffixes.NumElements() != 1))
	{
		mpcDiagnostics->Error(pcDeclarator, "This function declarator is not supported by the compiler yet");
		return NULL;
	}
	bUndeduced = mpcDeclarators->IsAuto(pcTypeSpecifiers) || mpcDeclarators->IsDecltypeAuto(pcTypeSpecifiers);
	if (pcSuffix->mpcTrailingReturn && (!mpcDeclarators->IsAuto(pcTypeSpecifiers) || pcDeclarator->mpcNested || (pcDeclarator->NumPointers() != 0)))
	{
		mpcDiagnostics->Error(pcDeclarator, "A function whose return type is written after it is declared auto, and nothing more");
		return NULL;
	}

	psReturn = &mpcTypes->msVoid;
	if (lliName == CPP_CONVERSION_NAME)
	{
		//operator T () returns T.
		psReturn = pcComponent->mpcType ? mpcDeclarators->GetTypeIdType(pcComponent->mpcType) : NULL;
		if (!psReturn)
		{
			return NULL;
		}
	}
	else if (eKind == CPPFK_Function)
	{
		//auto: the type is worked out once the function is known (ResolveReturn).
		psReturn = bUndeduced ? &mpcTypes->msUndeduced : ReturnDeclaratorType(mpcDeclarators->GetType(pcTypeSpecifiers, NULL, pcDeclarator, NULL), pcDeclarator);
		if (!psReturn || !mpcDeclarators->IsReturnable(psReturn, pcDeclarator))
		{
			return NULL;
		}
	}
	if (!pcClass && (pcSuffix->IsConst() || pcSuffix->IsVolatile()))
	{
		//[dcl.fct] 6
		mpcDiagnostics->Error(pcDeclarator, pcSuffix->IsConst() ? "Only a member function is const" : "Only a member function is volatile");
		return NULL;
	}

	apsParameters.Init();
	apcDefaults.Init();
	bConstThis = pcClass && pcSuffix->IsConst();
	bVolatileThis = pcClass && pcSuffix->IsVolatile();
	eRefQualifier = pcClass ? mpcUnit->GetInstantiator()->RefQualifier(pcSuffix) : CPPRQ_None;
	apsPacks.Init();
	for (i = 0; i < pcSuffix->macpParameters.NumElements(); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		//Args... args, and any pack expansion a parameter is declared with (Pair<Count<Ts...>, Ts>... p), is a parameter
		//for each of its packs' elements.  [temp.variadic] 5
		apsPacks.ReInit();
		uiPack = 1;
		if (pcParameter->mpcDeclarator && pcParameter->mpcDeclarator->mbPack && pcParameter->mpcSpecifiers)
		{
			mpcUnit->FindPacks(pcParameter->mpcSpecifiers, &apsPacks, NULL);
			if ((apsPacks.NumElements() > 0) && !mpcUnit->PackExpansionLength(&apsPacks, NULL, pcParameter, &uiPack))
			{
				apsPacks.Kill();
				apcDefaults.Kill();
				apsParameters.Kill();
				return NULL;
			}
		}
		cElements.Init(&apsPacks);
		for (j = 0; j < uiPack; j++)
		{
			if (apsPacks.NumElements() > 0)
			{
				cElements.Enter(j);
			}
			psType = mpcDeclarators->GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter);
			if (!psType || (psType->IsVoid() && !mpcDeclarators->IsWholeVoid(pcSuffix, pcParameter)))
			{
				cElements.Kill();
				apsPacks.Kill();
				apcDefaults.Kill();
				apsParameters.Kill();
				return NULL;
			}
			if (psType->IsVoid())
			{
				break;
			}
			apsParameters.Add(ParameterType(psType));
			apcDefaults.Add(pcParameter->mpcDefault);
		}
		cElements.Kill();
	}
	apsPacks.Kill();
	if (!pcTemplate && (!CheckAllocationFunction(lliName, psReturn, &apsParameters, pcDeclarator) || !CheckOperatorSignature(pcName->GetLast(), lliName, &apsParameters, &apcDefaults, pcClass, pcDeclarator)))
	{
		apcDefaults.Kill();
		apsParameters.Kill();
		return NULL;
	}

	//A declaration of a function declared before.
	uiFlags = pcSpecifiers ? pcSpecifiers->muiFlags : 0;
	pcNamespace = NULL;
	if (!pcClass)
	{
		//A friend with a simple name is of the namespace its class is in, whatever the name finds.  [namespace.memdef] 3
		bFriend = pcName->IsSimple() && (uiFlags & CPPDS_FRIEND);
		pcNamespace = (pcName->mpcSymbol && !bFriend) ? pcName->mpcSymbol->mpcScope->GetNamespace() : mpcContext->GetNamespace();
		if (!pcTemplate && ((lliName == mpcUnit->GetOperatorName(CT_new, false)) || (lliName == mpcUnit->GetOperatorName(CT_new, true)) || (lliName == mpcUnit->GetOperatorName(CT_delete, false)) || (lliName == mpcUnit->GetOperatorName(CT_delete, true))))
		{
			//An allocation or deallocation function is a class's or the global namespace's, and not static.  [basic.stc.dynamic] 1
			if (pcNamespace && pcNamespace->mpcParent)
			{
				mpcDiagnostics->ReportOnce(pcDeclarator, "An operator new or delete is declared in the global namespace or in a class");
			}
			else if (uiFlags & CPPDS_STATIC)
			{
				mpcDiagnostics->ReportOnce(pcDeclarator, "An operator new or delete in the global namespace can't be static");
			}
		}
	}
	for (i = 0; !pcTemplate && (i <= apsParameters.NumElements()); i++)
	{
		psAbstract = (i < apsParameters.NumElements()) ? apsParameters.GetPtr(i) : psReturn;
		if (psAbstract && psAbstract->IsClass() && psAbstract->pcClass->IsComplete() && psAbstract->pcClass->IsAbstract())
		{
			//[class.abstract] 3
			mpcDiagnostics->ReportOnce(pcDeclarator, "An abstract class is no function's parameter or result: no object of it can be made");
			break;
		}
	}
	for (pcOther = mpcUnit->FirstFunctionOfName(lliName); pcOther; pcOther = pcOther->mpcNextOfName)
	{
		if ((pcOther->mpcClass != pcClass) || (pcOther->mpcNamespace != pcNamespace) || (pcOther->meKind != eKind) || (pcOther->mapsParameters.NumElements() != apsParameters.NumElements()) || (pcOther->mpcTemplate != pcTemplate) ||
			(pcOther->mbVariadic != pcSuffix->mbVariadic))
		{
			continue;
		}
		if (pcTemplate && !pcOther->mszArguments.Equals(pszArguments))
		{
			continue;
		}
		if ((pcOther->IsConversion() && !pcOther->mpsReturn->Equals(psReturn)) || (pcOther->mbConstThis != bConstThis) || (pcOther->mbVolatileThis != bVolatileThis) || (pcOther->meRefQualifier != eRefQualifier))
		{
			continue;
		}
		if (!pcOther->TakesParameters(&apsParameters))
		{
			continue;
		}
		pcFunction = pcOther;
		if (!pcTemplate && !pcFunction->IsConversion() && !pcFunction->mpsReturn->Same(psReturn) && (pcFunction->mpsReturn != &mpcTypes->msUndeduced) && (psReturn != &mpcTypes->msUndeduced))
		{
			//What a function returns doesn't tell it from another: one declared twice returns one type.  [dcl.fct] 5, [over.load] 2
			mpcDiagnostics->ReportOnce(pcDeclarator, "The function is declared again with another return type");
		}
		for (j = 0; j < apcDefaults.NumElements(); j++)
		{
			if (apcDefaults.GetPtr(j) && pcFunction->mapcDefaults.GetPtr(j) && (apcDefaults.GetPtr(j) != pcFunction->mapcDefaults.GetPtr(j)) && !pcTemplate)
			{
				//[dcl.fct.default] 4
				mpcDiagnostics->Error(apcDefaults.GetPtr(j), "The parameter's default argument is given once: an earlier declaration has it");
			}
			if (apcDefaults.GetPtr(j))
			{
				pcFunction->mapcDefaults.SetPtr(j, apcDefaults.GetPtr(j));
			}
		}
		if (!pcTemplate)
		{
			CheckDefaultsFollow(pcFunction, pcDeclarator);
		}
		apcDefaults.Kill();
		apsParameters.Kill();
		pcFunction->mbVirtual |= (uiFlags & CPPDS_VIRTUAL) != 0;
		pcFunction->mbPure |= bPure;
		pcFunction->mbCLinkage |= mpcContext->IsCLinkage() && !pcClass;
		pcFunction->mbExplicit |= (uiFlags & CPPDS_EXPLICIT) != 0;
		pcFunction->mbConstexpr |= (uiFlags & CPPDS_CONSTEXPR) != 0;
		mpcUnit->GetExceptions()->CheckRedeclaredExceptions(pcFunction, pcDeclarator);
		return pcFunction;
	}

	if (mpcContext->IsCLinkage() && !pcClass && !pcTemplate)
	{
		for (pcOther = mpcUnit->FirstFunctionOfName(lliName); pcOther; pcOther = pcOther->mpcNextOfName)
		{
			if (!pcOther->mbCLinkage || pcOther->mpcClass)
			{
				continue;
			}
			bSameParameters = (pcOther->mbVariadic == pcSuffix->mbVariadic) && pcOther->TakesParameters(&apsParameters);
			if (!bSameParameters)
			{
				//Two extern "C" functions of a name are one function, whatever namespace each is in.  [dcl.link] 6
				mpcDiagnostics->ReportOnce(pcDeclarator, "Two extern \"C\" functions of one name are one function: it can't be overloaded");
				break;
			}
		}
	}

	for (pcOther = pcClass ? mpcUnit->FirstFunctionOfName(lliName) : NULL; pcOther; pcOther = pcOther->mpcNextOfName)
	{
		if ((pcOther->mpcClass != pcClass) || (pcOther->meKind != eKind) || pcOther->mpcTemplate || pcTemplate || ((pcOther->meRefQualifier == CPPRQ_None) == (eRefQualifier == CPPRQ_None)))
		{
			continue;
		}
		bSameParameters = (pcOther->mbVariadic == pcSuffix->mbVariadic) && pcOther->TakesParameters(&apsParameters);
		if (bSameParameters)
		{
			//Overloads that take the same parameters are all declared & or && or none is.  [over.load] 2
			mpcDiagnostics->ReportOnce(pcDeclarator, "Where one overload taking these parameters is declared & or &&, each is");
			break;
		}
	}

	pcFunction = mpcUnit->AddFunction(pcClass, lliName, eKind, psReturn);
	pcFunction->mpcNamespace = pcNamespace;
	pcFunction->mpcDeclarator = pcDeclarator;
	for (i = 0; i < apsParameters.NumElements(); i++)
	{
		pcFunction->mapsParameters.Add(apsParameters.GetPtr(i));
		pcFunction->mapcDefaults.Add(apcDefaults.GetPtr(i));
	}
	pcFunction->mbConstThis = bConstThis;
	pcFunction->mbVolatileThis = bVolatileThis;
	pcFunction->meRefQualifier = eRefQualifier;
	apcDefaults.Kill();
	apsParameters.Kill();
	if (!pcTemplate)
	{
		CheckDefaultsFollow(pcFunction, pcDeclarator);
	}
	pcFunction->mbVariadic = pcSuffix->mbVariadic;
	pcFunction->mbStatic = ((uiFlags & CPPDS_STATIC) != 0) || (pcClass && IsAllocationFunction(lliName));
	pcFunction->mbVirtual = (uiFlags & CPPDS_VIRTUAL) != 0;
	pcFunction->mbPure = bPure;
	pcFunction->mbInline = (uiFlags & (CPPDS_INLINE | CPPDS_CONSTEXPR)) != 0;
	pcFunction->mbConstexpr = (uiFlags & CPPDS_CONSTEXPR) != 0;
	pcFunction->mpcTemplate = pcTemplate;
	pcFunction->mbCLinkage = mpcContext->IsCLinkage() && !pcClass;
	pcFunction->mbExplicit = (uiFlags & CPPDS_EXPLICIT) != 0;
	pcFunction->miDeprecated = (pcFunction->miDeprecated != -1) ? pcFunction->miDeprecated : mpcUnit->FindAttribute(pcDeclarator, "deprecated");
	mpcUnit->GetExceptions()->SetDeclaredExceptions(pcFunction, pcDeclarator);
	pcFunction->mbOverride |= mpcUnit->HasVirtSpecifier(pcSuffix, mpcUnit->GetTokeniser()->mlliOverride);
	pcFunction->mbFinal |= mpcUnit->HasVirtSpecifier(pcSuffix, mpcUnit->GetTokeniser()->mlliFinal);
	if (pszArguments)
	{
		pcFunction->mszArguments.Append(pszArguments);
	}
	if (pcTemplate)
	{
		pcTemplate->mapcInstances.Add(pcFunction);
	}
	if (eKind == CPPFK_Constructor)
	{
		pcClass->mbConstructors = true;
	}
	else if (eKind == CPPFK_Destructor)
	{
		pcClass->mpcDestructor = pcFunction;
	}
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of what a function declared auto f(...) -> type, auto f(...)
// { } or decltype(auto) f(...) { } returns.  Its parameters are in scope
// for the type written after it, and its locals too for the type its
// first return gives, which the others must give as well: they are made
// as they are to write the function, in a function scope of their own,
// as this may be asked while another function is being written.  A
// declaration with neither leaves the type until the definition.
// [dcl.fct] 2, [dcl.spec.auto] 2, 7-9
//
//////////////////////////////////////////////////////////////////////////
bool CCPPFunctionDeclarer::ResolveReturn(CCPPFunction* pcFunction, CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPSyntaxFunctionSuffix*		pcSuffix;
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPSyntaxTypeId*				pcTrailing;
	SCPP65Type*						psReturn;
	CArraySyntax					apcReturns;
	size							i;
	bool							bWritten;

	pcSuffix = pcDeclarator->GetFunctionSuffix();
	pcDefinition = (pcDeclarator->mpcParent && (pcDeclarator->mpcParent->GetKind() == CPPS_FunctionDefinition)) ? (CCPPSyntaxFunctionDefinition*)pcDeclarator->mpcParent : NULL;
	if (pcDefinition && (pcDefinition->meBody != CPPFB_Body))
	{
		pcDefinition = NULL;
	}

	//-> auto and -> decltype(auto) are the returns' type still, as the specifiers before the function's name make it.
	pcTrailing = pcSuffix->mpcTrailingReturn;
	bWritten = pcTrailing && !mpcDeclarators->IsAuto(pcTrailing->mpcSpecifiers) && !mpcDeclarators->IsDecltypeAuto(pcTrailing->mpcSpecifiers);
	if (!bWritten && !pcDefinition)
	{
		return true;
	}

	mpcUnit->EnterFunction(pcFunction);
	mpcUnit->AddParameters(pcFunction, pcDeclarator);
	if (pcTrailing && !bWritten)
	{
		pcSpecifiers = pcTrailing->mpcSpecifiers;
		pcDeclarator = pcTrailing->mpcDeclarator;
	}
	if (bWritten)
	{
		psReturn = mpcDeclarators->GetTypeIdType(pcTrailing);
	}
	else
	{
		mpcDiagnostics->BeginQuiet();
		mpcUnit->AllocateLocals(pcDefinition->mpcBody);
		mpcDiagnostics->EndQuiet();
		apcReturns.Init();
		FindReturns(pcDefinition->mpcBody, &apcReturns);
		psReturn = &mpcTypes->msVoid;
		for (i = 0; psReturn && (i < apcReturns.NumElements()); i++)
		{
			psReturn = ReturnedType(pcSpecifiers, pcDeclarator, (CCPPSyntaxReturnStatement*)apcReturns.GetPtr(i));
			if (i == 0)
			{
				//Known from here on, so the function can call itself after its first return.
				pcFunction->mpsReturn = psReturn ? psReturn : &mpcTypes->msUndeduced;
			}
			else if (psReturn && !psReturn->Same(pcFunction->mpsReturn))
			{
				mpcDiagnostics->Error(apcReturns.GetPtr(i), "Each return of a function declared auto must give the same type");
				psReturn = NULL;
			}
		}
		apcReturns.Kill();
	}
	mpcUnit->LeaveFunction();
	if (psReturn)
	{
		pcFunction->mpsReturn = psReturn;
	}
	return psReturn != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type return value; gives a function declared auto (as it gives a
// variable declared so, with the function's & or *) or decltype(auto):
// by the specifiers and declarator before its name, or those after its ->
// (pcDeclarator is NULL when there is none there).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPFunctionDeclarer::ReturnedType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntaxReturnStatement* pcReturn)
{
	SCPP65Type*	psType;

	if (!pcReturn->mpcValue)
	{
		return &mpcTypes->msVoid;
	}
	if (mpcDeclarators->IsDecltypeAuto(pcSpecifiers))
	{
		return mpcDeclarators->DecltypeType(pcReturn->mpcValue, pcReturn);
	}
	psType = mpcDeclarators->DeduceAuto(pcSpecifiers, pcDeclarator, pcReturn->mpcValue, pcReturn);
	if (psType && (pcSpecifiers->muiFlags & (CPPDS_CONST | CPPDS_VOLATILE)))
	{
		psType = mpcTypes->Qualified(psType, psType->bConst || (pcSpecifiers->muiFlags & CPPDS_CONST), psType->bVolatile || (pcSpecifiers->muiFlags & CPPDS_VOLATILE));
	}
	return pcDeclarator ? ReturnDeclaratorType(psType, pcDeclarator) : psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The return statements of a function's body, in order: not those of a
// class or a lambda inside it.
//
//////////////////////////////////////////////////////////////////////////
void CCPPFunctionDeclarer::FindReturns(CCPPSyntax* pcNode, CArraySyntax* papcReturns)
{
	size	i;

	if (!pcNode || (pcNode->GetKind() == CPPS_ClassSpecifier) || (pcNode->GetKind() == CPPS_LambdaExpression))
	{
		return;
	}
	if (pcNode->GetKind() == CPPS_ReturnStatement)
	{
		papcReturns->Add(pcNode);
		return;
	}
	for (i = 0; i < pcNode->NumChildren(); i++)
	{
		FindReturns(pcNode->GetChild(i), papcReturns);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What a function's declarator makes of the type before it: int (*Pick())(int)
// returns what the declarators around its name make of the int, and int*& f()
// a reference to a pointer.  NULL after an error.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPFunctionDeclarer::ReturnDeclaratorType(SCPP65Type* psReturn, CCPPSyntaxDeclarator* pcDeclarator)
{
	size		i;
	CCPPClass*	pcClass;

	if (psReturn && pcDeclarator->mpcNested)
	{
		psReturn = mpcDeclarators->GetDeclaratorType(psReturn, pcDeclarator, pcDeclarator, NULL, pcDeclarator->GetFunctionSuffix());
	}
	for (i = 0; psReturn && !pcDeclarator->mpcNested && (i < pcDeclarator->NumPointers()); i++)
	{
		switch (pcDeclarator->GetPointer(i)->meKind)
		{
		case CPPP_Pointer:
			psReturn = mpcUnit->QualifiedPointer(mpcTypes->Pointer(psReturn), pcDeclarator->GetPointer(i));
			break;
		case CPPP_Reference:
		case CPPP_RValueReference:
			psReturn = mpcTypes->Reference(psReturn, pcDeclarator->GetPointer(i)->meKind == CPPP_RValueReference);
			break;
		case CPPP_MemberPointer:
			//int B::* Get() returns a pointer to an int member of B.
			pcClass = pcDeclarator->GetPointer(i)->mpcClass ? mpcUnit->GetNameClass(pcDeclarator->GetPointer(i)->mpcClass, pcDeclarator) : NULL;
			psReturn = pcClass ? mpcTypes->MemberPointer(pcClass, psReturn) : NULL;
			if (!pcClass)
			{
				mpcDiagnostics->Error(pcDeclarator, "The class of a pointer to a member is not supported by the compiler yet");
			}
			break;
		default:
			mpcDiagnostics->Error(pcDeclarator, "This declarator is not supported by the compiler yet");
			psReturn = NULL;
			break;
		}
	}
	return psReturn;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a qualified name (A::f) is defined in a namespace that encloses
// its declaration, as it must be: not namespace C { int A::f() { ... } }.
// [namespace.memdef] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPFunctionDeclarer::IsDefinedWhereDeclared(CCPPSyntaxName* pcName)
{
	CCPPScope*	pcHere;

	if ((pcName->macpComponents.NumElements() < 2) || !pcName->mpcSymbol || !pcName->mpcSymbol->mpcScope)
	{
		return true;
	}
	pcHere = mpcContext->GetNamespace();
	return !pcHere || pcHere->Encloses(pcName->mpcSymbol->mpcScope);
}


//////////////////////////////////////////////////////////////////////////
//
// Declare in quiet code: a declaration the compiler can't do is an error
// only where the function is used.  If it fails, its declarator is kept
// with what declaring it said quietly, which is said where it is called
// (ReportFailed), and that is given to *ppcErrors too; what it said loudly
// is said now.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPFunctionDeclarer::DeclareQuietly(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, bool bDefinition, CCPPClass* pcClass, bool bPure, CCPPErrors** ppcErrors)
{
	CCPPFunction*			pcFunction;
	CCPPErrors*				pcErrors;
	SCPPFailedDeclaration*	psFailed;

	mpcDiagnostics->BeginQuiet();
	mpcDiagnostics->BeginKeeping(false);
	pcFunction = Declare(pcSpecifiers, pcDeclarator, bDefinition, pcClass, bPure);
	pcErrors = mpcDiagnostics->EndKeeping();
	mpcDiagnostics->EndQuiet();
	if (!pcFunction)
	{
		psFailed = masFailed.Add();
		psFailed->pcDeclarator = pcDeclarator;
		psFailed->pcErrors = pcErrors;
	}
	if (ppcErrors)
	{
		*ppcErrors = pcErrors;
	}
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// int F(int a); in a block declares a function of the namespace around the
// block, as if it was declared there.  [basic.link] 6
//
//////////////////////////////////////////////////////////////////////////
void CCPPFunctionDeclarer::DeclareBlockFunction(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator)
{
	DeclareQuietly(pcSpecifiers, pcDeclarator, false, NULL, false);
}


//////////////////////////////////////////////////////////////////////////
//
// A function of pcClass the compiler declares, not the program (an
// implicit destructor or constructor, a closure's conversion), defined by
// the compiler too where bDefined.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPFunctionDeclarer::AddImplicit(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, SCPP65Type* psReturn, bool bDefined)
{
	CCPPFunction*	pcFunction;

	pcFunction = mpcUnit->AddFunction(pcClass, lliName, eKind, psReturn);
	pcFunction->mbImplicit = true;
	pcFunction->mbDefined = bDefined;
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// A function's type: its return type, parameters, whether it is variadic,
// and whether it is a member function that is const or volatile.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPFunctionDeclarer::FunctionType(CCPPFunction* pcFunction)
{
	bool	bMember;

	bMember = pcFunction->mpcClass && !pcFunction->mbStatic;
	return mpcTypes->Function(pcFunction->mpsReturn, &pcFunction->mapsParameters, pcFunction->mbVariadic, bMember && pcFunction->mbConstThis, bMember && pcFunction->mbVolatileThis);
}


//////////////////////////////////////////////////////////////////////////
//
// A parameter's type in its function's type: an array is a pointer to its
// element, a function a pointer to it, and const and volatile on the
// parameter itself go (f(const int) is f(int)).  [dcl.fct] 5
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPFunctionDeclarer::ParameterType(SCPP65Type* psType)
{
	if (psType->IsArray())
	{
		return mpcTypes->Pointer(psType->psBase);
	}
	if (psType->IsFunction())
	{
		return mpcTypes->Pointer(psType);
	}
	return psType->Unqualified();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function's label is weak, so that another object's definition
// takes its place: __attribute__((weak)) on its definition, or on the
// declaration it is known by (Libc65's replaceable operator new and delete
// say it where they are defined, so that a program's own, declared by the
// same header, isn't).  [replacement.functions]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPFunctionDeclarer::IsWeak(CCPPFunction* pcFunction)
{
	return mpcUnit->HasAttribute(pcFunction->mpcDeclarator, "weak") || (pcFunction->mpcDefinition && mpcUnit->HasAttribute(pcFunction->mpcDefinition->mpcDeclarator, "weak"));
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function's name is operator new, new[], delete or delete[],
// which as a class's members are static whether or not they are declared
// so.  [class.free] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPFunctionDeclarer::IsAllocationFunction(int64 lliName)
{
	return (lliName == mpcUnit->GetOperatorName(CT_new, false)) || (lliName == mpcUnit->GetOperatorName(CT_new, true)) || (lliName == mpcUnit->GetOperatorName(CT_delete, false)) || (lliName == mpcUnit->GetOperatorName(CT_delete, true));
}


//////////////////////////////////////////////////////////////////////////
//
// True if the declarator declares an operator new, new[], delete or
// delete[].
//
//////////////////////////////////////////////////////////////////////////
bool CCPPFunctionDeclarer::DeclaresAllocation(CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPSyntaxName*	pcName;
	int64			lliName;

	pcName = pcDeclarator->GetName();
	lliName = (pcName && (pcName->GetLast()->meKind == CPPNC_Operator)) ? mpcUnit->GetFunctionName(pcName->GetLast()) : -1;
	return (lliName != -1) && ((lliName == mpcUnit->GetOperatorName(CT_new, false)) || (lliName == mpcUnit->GetOperatorName(CT_new, true)) || (lliName == mpcUnit->GetOperatorName(CT_delete, false)) || (lliName == mpcUnit->GetOperatorName(CT_delete, true)));
}


//////////////////////////////////////////////////////////////////////////
//
// What an operator new or delete must be to be an allocation or a
// deallocation function: operator new returns void* and its first
// parameter is a std::size_t; operator delete returns nothing and its first
// parameter is a void*.  True for any other function.
// [basic.stc.dynamic.allocation] 1, [basic.stc.dynamic.deallocation] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPFunctionDeclarer::CheckAllocationFunction(int64 lliName, SCPP65Type* psReturn, CArrayCPP65Types* papsParameters, CCPPSyntax* pcNode)
{
	SCPP65Type*	psVoidPointer;
	SCPP65Type*	psFirst;

	psVoidPointer = mpcTypes->Pointer(&mpcTypes->msVoid);
	psFirst = (papsParameters->NumElements() > 0) ? papsParameters->GetPtr(0)->Unqualified() : NULL;
	if ((lliName == mpcUnit->GetOperatorName(CT_new, false)) || (lliName == mpcUnit->GetOperatorName(CT_new, true)))
	{
		if (!psReturn->Equals(psVoidPointer))
		{
			return mpcDiagnostics->Error(pcNode, "operator new returns void*");
		}
		return (psFirst && psFirst->Equals(mpcTypes->Size())) || mpcDiagnostics->Error(pcNode, "operator new's first parameter is a std::size_t");
	}
	if ((lliName == mpcUnit->GetOperatorName(CT_delete, false)) || (lliName == mpcUnit->GetOperatorName(CT_delete, true)))
	{
		if (!psReturn->IsVoid())
		{
			return mpcDiagnostics->Error(pcNode, "operator delete returns void");
		}
		return (psFirst && psFirst->Equals(psVoidPointer)) || mpcDiagnostics->Error(pcNode, "operator delete's first parameter is a void*");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An operator function's parameters: none has a default, but operator()'s;
// one that is no class's member has a class or an enum among them (a
// reference to one too); postfix ++'s and --'s extra one is an int.  A
// literal operator's are one of the lists [over.literal] 3 gives.  An error
// (and false) if not.  [over.oper] 6, 8, [over.inc] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPFunctionDeclarer::CheckOperatorSignature(CCPPSyntaxNameComponent* pcComponent, int64 lliName, CArrayCPP65Types* papsParameters, CArraySyntax* papcDefaults, CCPPClass* pcClass, CCPPSyntax* pcNode)
{
	SCPP65Type*	psType;
	SCPP65Type*	psFirst;
	size		i;
	bool		bClass;

	if (pcComponent->meKind == CPPNC_Literal)
	{
		psFirst = (papsParameters->NumElements() > 0) ? papsParameters->GetPtr(0)->Unqualified() : NULL;
		if (papsParameters->NumElements() == 1)
		{
			bClass = psFirst->Equals(mpcTypes->Pointer(mpcTypes->Const(&mpcTypes->msChar))) || psFirst->Equals(&mpcTypes->msUnsignedLongLong) || psFirst->Equals(&mpcTypes->msLongDouble) ||
					 psFirst->Equals(&mpcTypes->msChar) || psFirst->Equals(&mpcTypes->msWChar) || psFirst->Equals(&mpcTypes->msChar16) || psFirst->Equals(&mpcTypes->msChar32);
		}
		else if (papsParameters->NumElements() == 2)
		{
			bClass = psFirst->IsPointer() && psFirst->psBase->bConst && (psFirst->psBase->Unqualified()->Equals(&mpcTypes->msChar) || psFirst->psBase->Unqualified()->Equals(&mpcTypes->msWChar) ||
					 psFirst->psBase->Unqualified()->Equals(&mpcTypes->msChar16) || psFirst->psBase->Unqualified()->Equals(&mpcTypes->msChar32)) && papsParameters->GetPtr(1)->Unqualified()->Equals(mpcTypes->Size());
		}
		else
		{
			bClass = false;
		}
		return bClass || mpcDiagnostics->Error(pcNode, "A literal operator's parameters are one of: const char*; unsigned long long; long double; a character type; a pointer to const characters and a size_t");
	}
	if ((pcComponent->meKind != CPPNC_Operator) || (lliName == mpcUnit->GetOperatorName(CT_new, false)) || (lliName == mpcUnit->GetOperatorName(CT_new, true)) ||
		(lliName == mpcUnit->GetOperatorName(CT_delete, false)) || (lliName == mpcUnit->GetOperatorName(CT_delete, true)))
	{
		return true;
	}
	if (lliName != mpcUnit->GetOperatorName(CT_OpenRoundBracket, false))
	{
		for (i = 0; i < papcDefaults->NumElements(); i++)
		{
			if (papcDefaults->GetPtr(i))
			{
				return OperatorError(pcClass, pcNode, "An operator function has no default arguments (but operator())");
			}
		}
	}
	if (!pcClass)
	{
		bClass = false;
		for (i = 0; !bClass && (i < papsParameters->NumElements()); i++)
		{
			psType = papsParameters->GetPtr(i)->WithoutReference();
			bClass = psType->IsClass() || (psType->pcEnum != NULL);
		}
		if (!bClass)
		{
			return mpcDiagnostics->Error(pcNode, "An operator function that is no class's member has a parameter of a class or an enum");
		}
	}
	if (((lliName == mpcUnit->GetOperatorName(CT_Increment, false)) || (lliName == mpcUnit->GetOperatorName(CT_Decrement, false))) && (papsParameters->NumElements() == (pcClass ? 1 : 2)) &&
		!papsParameters->GetPtr(papsParameters->NumElements() - 1)->Unqualified()->Equals(&mpcTypes->msInt))
	{
		return OperatorError(pcClass, pcNode, "A postfix ++ or --'s second parameter is an int");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An error in a function's declaration: a member's is a class error, kept
// though the class is laid out quietly (Defer).  False.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPFunctionDeclarer::OperatorError(CCPPClass* pcClass, CCPPSyntax* pcNode, const char* szMessage)
{
	return pcClass ? mpcDiagnostics->Defer(pcNode->miFirstToken, szMessage) : mpcDiagnostics->Error(pcNode, szMessage);
}


//////////////////////////////////////////////////////////////////////////
//
// After a parameter with a default argument every parameter has one, from
// this declaration or one before it: an error at pcNode if not.
// [dcl.fct.default] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPFunctionDeclarer::CheckDefaultsFollow(CCPPFunction* pcFunction, CCPPSyntax* pcNode)
{
	size	i;
	bool	bDefault;

	bDefault = false;
	for (i = 0; i < pcFunction->mapcDefaults.NumElements(); i++)
	{
		if (bDefault && !pcFunction->mapcDefaults.GetPtr(i))
		{
			return mpcDiagnostics->Error(pcNode, "A parameter after one with a default argument needs one too");
		}
		bDefault |= pcFunction->mapcDefaults.GetPtr(i) != NULL;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A function defined or called takes no parameter of an abstract class
// and returns no object of one (its declaration said so where the class
// was complete then; DeclareSignature).  An error at pcNode if it does.
// [class.abstract] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPFunctionDeclarer::CheckAbstractSignature(CCPPFunction* pcFunction, CCPPSyntax* pcNode)
{
	SCPP65Type*	psType;
	size		i;

	for (i = 0; i <= pcFunction->mapsParameters.NumElements(); i++)
	{
		psType = (i < pcFunction->mapsParameters.NumElements()) ? pcFunction->mapsParameters.GetPtr(i) : pcFunction->mpsReturn;
		if (psType && psType->IsClass() && psType->pcClass->IsComplete() && psType->pcClass->IsAbstract())
		{
			return mpcDiagnostics->ReportOnce(pcNode, "An abstract class is no function's parameter or result: no object of it can be made");
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// What = default and = delete ask of the function they define.  = default
// is of a special member function: a constructor that takes nothing or
// its own class (a copy or a move), a destructor, or an operator= that
// takes its own class; and it has no default argument.  = delete is where
// the function is first declared.  An error (and false) if not; true for
// any other definition.  [dcl.fct.def.default] 1, [dcl.fct.def.delete] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPFunctionDeclarer::CheckDefaultedOrDeleted(CCPPFunction* pcFunction, CCPPSyntaxFunctionDefinition* pcDefinition)
{
	SCPP65Type*	psParameter;
	size		i;
	bool		bOwnClass;

	if (pcDefinition->meBody == CPPFB_Delete)
	{
		return (pcFunction->mpcDeclarator == pcDefinition->mpcDeclarator) || pcFunction->mpcTemplate || mpcDiagnostics->ReportError(pcDefinition, "A function is deleted where it is first declared");
	}
	if (pcDefinition->meBody != CPPFB_Default)
	{
		return true;
	}
	psParameter = (pcFunction->mapsParameters.NumElements() == 1) ? pcFunction->mapsParameters.GetPtr(0)->WithoutReference() : NULL;
	bOwnClass = psParameter && psParameter->IsClass() && (psParameter->pcClass == pcFunction->mpcClass);
	if (!pcFunction->mpcClass || ((pcFunction->meKind == CPPFK_Constructor) && (pcFunction->mapsParameters.NumElements() != 0) && !bOwnClass) ||
		((pcFunction->meKind == CPPFK_Function) && !((pcFunction->mlliName == CPP_OPERATOR_NAME(CT_Assignment)) && bOwnClass)))
	{
		return mpcDiagnostics->ReportError(pcDefinition, "Only a constructor that takes nothing or its own class, a destructor or an operator= that takes its own class is = default");
	}
	for (i = 0; i < pcFunction->mapcDefaults.NumElements(); i++)
	{
		if (pcFunction->mapcDefaults.GetPtr(i))
		{
			return mpcDiagnostics->ReportError(pcDefinition, "A function that is = default has no default argument");
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A constructor's member initialisers: each names a member or a base of
// its class, once; a delegating one (naming the class) is the only one;
// and a reference member, or a const one of a type with no constructor of
// its own, that no default member initialiser gives a value is named.
// [class.base.init] 2, 4, 6, 8
//
//////////////////////////////////////////////////////////////////////////
bool CCPPFunctionDeclarer::CheckMemberInitialisers(CCPPFunction* pcFunction)
{
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CArraySyntax*					papcInitialisers;
	CCPPSyntaxMemberInitializer*	pcMember;
	CCPPSyntaxMemberInitializer*	pcOther;
	CCPPClass*						pcClass;
	CCPPClass*						pcNamed;
	CCPPClass*						pcOtherNamed;
	SCPPField*						psField;
	SCPP65Type*						psType;
	int64							lliName;
	size							i;
	size							j;
	int								iOffset;
	bool							bDelegating;
	bool							bNamed;

	pcDefinition = pcFunction->mpcDefinition;
	pcClass = pcFunction->mpcClass;
	if ((pcFunction->meKind != CPPFK_Constructor) || !pcDefinition || !pcClass || (pcDefinition->meBody != CPPFB_Body) || !mpcUnit->CompleteClass(pcClass, pcDefinition))
	{
		return true;
	}
	papcInitialisers = mpcUnit->ExpandArguments(&pcDefinition->macpMemberInitializers);
	if (!papcInitialisers)
	{
		return true;
	}

	bDelegating = false;
	for (i = 0; i < papcInitialisers->NumElements(); i++)
	{
		pcMember = (CCPPSyntaxMemberInitializer*)papcInitialisers->GetPtr(i);
		if (pcMember->GetKind() != CPPS_MemberInitializer)
		{
			return true;
		}
		lliName = mpcUnit->GetComponentName(pcMember->mpcName->GetLast());
		pcNamed = (pcMember->mpcName->mpcSymbol && (pcMember->mpcName->mpcSymbol->meKind == CPPSK_Field)) ? NULL : mpcUnit->GetNameClass(pcMember->mpcName, pcMember);
		if (pcNamed == pcClass)
		{
			bDelegating = true;
			if (papcInitialisers->NumElements() != 1)
			{
				return mpcDiagnostics->Error(pcMember, "A constructor that names its own class in its member initialisers has no other");
			}
		}
		else if (!(pcNamed && mpcUnit->IsDerived(pcClass, pcNamed, NULL)) && !pcClass->FindField(lliName, &iOffset))
		{
			return mpcDiagnostics->Error(pcMember, "The member initialiser names no member or base of the class");
		}
		for (j = 0; j < i; j++)
		{
			pcOther = (CCPPSyntaxMemberInitializer*)papcInitialisers->GetPtr(j);
			pcOtherNamed = (pcOther->mpcName->mpcSymbol && (pcOther->mpcName->mpcSymbol->meKind == CPPSK_Field)) ? NULL : mpcUnit->GetNameClass(pcOther->mpcName, pcOther);
			if (pcNamed ? (pcOtherNamed == pcNamed) : (!pcOtherNamed && (mpcUnit->GetComponentName(pcOther->mpcName->GetLast()) == lliName)))
			{
				return mpcDiagnostics->Error(pcMember, "The member or base is named twice in the member initialisers");
			}
		}
	}
	if (bDelegating || pcClass->mbUnion)
	{
		return true;
	}

	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		psField = pcClass->macFields.Get(i);
		if (!psField->pcSymbol || psField->pcInitialiser)
		{
			continue;
		}
		psType = psField->psType;
		if (!(psType->IsReference() || (psType->bConst && !(psType->IsClass() && psType->pcClass->mbConstructors))))
		{
			continue;
		}
		bNamed = false;
		for (j = 0; !bNamed && (j < papcInitialisers->NumElements()); j++)
		{
			pcMember = (CCPPSyntaxMemberInitializer*)papcInitialisers->GetPtr(j);
			bNamed = (pcMember->mpcName->mpcSymbol == psField->pcSymbol) || (mpcUnit->GetComponentName(pcMember->mpcName->GetLast()) == psField->lliName);
		}
		if (!bNamed)
		{
			return mpcDiagnostics->Error(pcDefinition->mpcDeclarator, "The constructor leaves a reference or const member without a value: name it in the member initialisers");
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A call to a function whose declaration could not be generated: what
// declaring it said is said now (as if said here: in quiet code it only
// fails).
//
//////////////////////////////////////////////////////////////////////////
void CCPPFunctionDeclarer::ReportFailed(int64 lliName)
{
	CCPPSyntaxDeclarator*	pcDeclarator;
	size					i;

	for (i = 0; i < masFailed.NumElements(); i++)
	{
		pcDeclarator = masFailed.Get(i)->pcDeclarator;
		if (pcDeclarator->GetName() && (mpcUnit->GetFunctionName(pcDeclarator->GetName()->GetLast()) == lliName))
		{
			mpcDiagnostics->Repeat(masFailed.Get(i)->pcErrors);
			return;
		}
	}
}


