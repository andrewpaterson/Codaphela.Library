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
#include "CPPBinder.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPBoundCall::Init(void)
{
	meHow = CPPBC_Failed;
	mpcCallee = NULL;
	mpcName = NULL;
	mlliName = -1;
	mpcFunction = NULL;
	mpapcArguments = NULL;
	mpcObject = NULL;
	mpcClass = NULL;
	mbArrow = false;
	mbVirtual = false;
	mpcLeft = NULL;
	mpcRight = NULL;
	mbOwnOnly = false;
	mpcBindings = NULL;
	mpcIn = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPBinder::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
	mcOverloads.Init(pcUnit);
	mcAccess.Init(pcUnit);
	mcBound.Init(true);
	mcOperators.Init(true);
	mapapcExpanded.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPBinder::Kill(void)
{
	size	i;

	for (i = 0; i < mapapcExpanded.NumElements(); i++)
	{
		mapapcExpanded.GetPtr(i)->Kill();
		free(mapapcExpanded.GetPtr(i));
	}
	mapapcExpanded.Kill();
	mcOperators.Kill();
	mcBound.Kill();
	mcOverloads.Kill();
	mpcUnit = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The function chosen for a call or an operator must be one that may be
// named there.  If it isn't the call has failed, so that it is bound
// again, and the error reported again, when its errors are wanted.
//
//////////////////////////////////////////////////////////////////////////
void CCPPBinder::CheckAccess(CCPPBoundCall* pcBound, CCPPSyntax* pcNode)
{
	if ((pcBound->meHow != CPPBC_Failed) && !mcAccess.CheckFunction(pcBound->mpcFunction, pcBound->mpcClass, pcNode))
	{
		pcBound->meHow = CPPBC_Failed;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What the call is, in *pcBound.  False if it can't be made (CPPBC_Failed).  Its
// arguments are the bound call's: the node's own, or with a function parameter
// pack's args... expanded, which the binder makes once and keeps.  bReport when the
// errors of a call that failed before are wanted again (the type checker's, which
// ReportTypeOf reports); without it a failure is as it was remembered.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::Bind(CCPPSyntaxCallExpression* pcCall, CCPPBoundCall* pcBound, bool bReport)
{
	CCPPBoundCall*	pcRemembered;

	mpcUnit->GetStatistics()->Count(CPPCC_BoundCalls);
	pcRemembered = mcBound.Get((int64)(size)pcCall);
	if (pcRemembered && (pcRemembered->mpcBindings == mpcUnit->GetContext()->GetBindings()) && (pcRemembered->mpcIn == mpcUnit->GetContext()->GetFunction()))
	{
		if ((pcRemembered->meHow != CPPBC_Failed) || !bReport)
		{
			*pcBound = *pcRemembered;
			return pcBound->meHow != CPPBC_Failed;
		}
	}

	mpcUnit->GetStatistics()->Count(CPPCC_BoundCallsResolved);
	pcBound->Init();
	pcBound->mpcBindings = mpcUnit->GetContext()->GetBindings();
	pcBound->mpcIn = mpcUnit->GetContext()->GetFunction();
	pcBound->mpapcArguments = mpcUnit->HasPackExpansion(&pcCall->macpArguments) ? ExpandPack(pcCall) : &pcCall->macpArguments;
	if (pcBound->mpapcArguments)
	{
		Resolve(pcCall, pcBound);
		CheckAccess(pcBound, pcCall);
	}
	else
	{
		pcBound->mpapcArguments = &pcCall->macpArguments;
	}
	mcBound.Put((int64)(size)pcCall, pcBound);
	return pcBound->meHow != CPPBC_Failed;
}


//////////////////////////////////////////////////////////////////////////
//
// The cascade.  pcBound->meHow is CPPBC_Failed until something else is found.
//
//////////////////////////////////////////////////////////////////////////
void CCPPBinder::Resolve(CCPPSyntaxCallExpression* pcCall, CCPPBoundCall* pcBound)
{
	CCPPSyntax*					pcCallee;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	CArraySyntax*				papcExplicit;
	CCPPSymbol*					pcSymbol;
	CCPPFunction*				pcFunction;
	CCPPFunction*				pcMember;
	CCPPFunction*				pcInstance;
	CCPPSymbol*					pcDeclaration;
	CCPPTemplate*				pcTemplate;
	size						uiErrors;
	int64						lliName;

	pcCallee = pcCall->mpcCallee->WithoutParentheses();
	pcBound->mpcCallee = pcCallee;
	if (pcCallee->GetKind() == CPPS_MemberExpression)
	{
		ResolveMember(pcCall, pcBound);
		return;
	}
	if (mpcUnit->IsMemberPointerOperator(pcCallee))
	{
		pcBound->meHow = CPPBC_MemberPointer;
		return;
	}
	pcSymbol = NULL;
	pcName = NULL;
	pcComponent = NULL;
	if (pcCallee->GetKind() == CPPS_IdExpression)
	{
		pcName = ((CCPPSyntaxIdExpression*)pcCallee)->mpcName;
		pcSymbol = pcName->mpcSymbol;
		pcComponent = pcName->GetLast();
		pcBound->mpcName = pcName;
		if (pcName->IsQualifiedByClass())
		{
			ResolveMember(pcCall, pcBound);
			return;
		}
	}
	pcMember = mpcUnit->OuterFunction();

	//f<Arguments>(...) and f(...) for a function template; in a member function it may be a member template.
	if (pcSymbol && (pcSymbol->meKind == CPPSK_FunctionTemplate))
	{
		pcBound->mlliName = pcSymbol->mlliName;
		if (pcMember && pcMember->mpcClass && !pcName->mbGlobal && (mcOverloads.FindMemberTemplate(pcMember->mpcClass, pcSymbol->mlliName, pcBound->mpapcArguments->NumElements()) || mpcUnit->HasMember(pcMember->mpcClass, pcSymbol->mlliName)))
		{
			ResolveMember(pcCall, pcBound);
			return;
		}
		papcExplicit = (pcComponent && pcComponent->mbTemplateArguments) ? &pcComponent->macpTemplateArguments : NULL;

		//A function of the name that isn't a template's, and takes the arguments as they are, is the better match
		//than any template's instance.  [over.match.best] 2.4
		if (!papcExplicit)
		{
			uiErrors = mpcUnit->GetErrors()->NumErrors();
			mpcUnit->BeginTrial();
			pcFunction = mcOverloads.FindNamedFunction(pcName, pcSymbol->mlliName, pcBound->mpapcArguments, pcCall);
			mpcUnit->EndTrial();
			mpcUnit->GetErrors()->Truncate(uiErrors);
			if (pcFunction && TakesAsTheyAre(pcFunction, pcBound->mpapcArguments))
			{
				mpcUnit->Resolved(pcName, pcFunction);
				pcBound->meHow = CPPBC_Function;
				pcBound->mpcFunction = pcFunction;
				return;
			}
		}
		pcTemplate = mcOverloads.GetCallTemplate(pcSymbol, pcSymbol->mlliName, pcCall, pcBound->mpapcArguments, papcExplicit);
		if (!pcTemplate)
		{
			mpcUnit->Error(pcCallee, "The function template has not been defined");
			return;
		}
		pcFunction = mpcUnit->InstantiateFunction(pcTemplate, papcExplicit, pcBound->mpapcArguments, pcCall);
		if (!pcFunction)
		{
			return;
		}
		mpcUnit->Resolved(pcName, pcFunction);
		pcBound->meHow = CPPBC_Function;
		pcBound->mpcFunction = pcFunction;
		return;
	}

	//f() in a member function may be a member, even when the parser could not resolve it in a template.
	lliName = pcSymbol ? pcSymbol->mlliName : mpcUnit->GetFunctionName(pcComponent);
	pcBound->mlliName = lliName;
	if (pcName && !pcName->mbGlobal && pcMember && pcMember->mpcClass && (!pcSymbol || (pcSymbol->meKind == CPPSK_Function)) && mpcUnit->HasMember(pcMember->mpcClass, lliName))
	{
		ResolveMember(pcCall, pcBound);
		return;
	}

	if (!pcSymbol && mpcUnit->IsInlineMemcpy(lliName, pcBound->mpapcArguments))
	{
		pcBound->meHow = CPPBC_InlineMemcpy;
		return;
	}

	//A __builtin_ function is the builtin whether or not something declares it.
	if (pcName && (!pcSymbol || (pcSymbol->meKind == CPPSK_Function)) && mpcUnit->IsBuiltin(lliName))
	{
		pcBound->meHow = CPPBC_Builtin;
		return;
	}

	if (!pcSymbol && mpcUnit->GetSemanticPass() && mpcUnit->IsIntrinsic(lliName))
	{
		//Only checking: an intrinsic of the host's compiler (__debugbreak()), declared by no header.
		pcBound->meHow = CPPBC_Unchecked;
		return;
	}
	if (!pcSymbol && pcName)
	{
		//Nothing declares it here: it may be in a namespace of its arguments'.
		uiErrors = mpcUnit->GetErrors()->NumErrors();
		pcFunction = mcOverloads.FindAssociatedFunction(pcName, lliName, pcCall, pcBound->mpapcArguments);
		if (pcFunction)
		{
			mpcUnit->Resolved(pcName, pcFunction);
			pcBound->meHow = CPPBC_Function;
			pcBound->mpcFunction = pcFunction;
			return;
		}
		if (mpcUnit->GetErrors()->NumErrors() != uiErrors)
		{
			return;
		}
	}
	if (!pcSymbol || (pcSymbol->meKind != CPPSK_Function))
	{
		//A variable, or an expression: an object with an operator(), or else a pointer to a function if it is anything.
		if (!ResolveCallOperator(pcCall, pcBound))
		{
			pcBound->meHow = CPPBC_Pointer;
		}
		return;
	}

	//The name may be a template's as well (declared after the function the name was first): its instance is the
	//better match unless a function takes the arguments as they are.
	for (pcDeclaration = pcSymbol->mpcNext; pcDeclaration && (pcDeclaration->meKind != CPPSK_FunctionTemplate); pcDeclaration = pcDeclaration->mpcNext)
	{
	}
	if (pcDeclaration)
	{
		uiErrors = mpcUnit->GetErrors()->NumErrors();
		mpcUnit->BeginTrial();
		pcFunction = mcOverloads.FindNamedFunction(pcName, lliName, pcBound->mpapcArguments, pcCall);
		if (!pcFunction || !TakesAsTheyAre(pcFunction, pcBound->mpapcArguments))
		{
			pcTemplate = mcOverloads.GetCallTemplate(pcSymbol, lliName, pcCall, pcBound->mpapcArguments, NULL);
			pcInstance = pcTemplate ? mpcUnit->InstantiateFunction(pcTemplate, NULL, pcBound->mpapcArguments, pcCall) : NULL;
			pcFunction = pcInstance ? pcInstance : pcFunction;
		}
		mpcUnit->EndTrial();
		mpcUnit->GetErrors()->Truncate(uiErrors);
		if (pcFunction)
		{
			mpcUnit->Resolved(pcName, pcFunction);
			pcBound->meHow = CPPBC_Function;
			pcBound->mpcFunction = pcFunction;
			return;
		}
	}

	uiErrors = mpcUnit->GetErrors()->NumErrors();
	pcFunction = mcOverloads.FindNamedFunction(pcName, lliName, pcBound->mpapcArguments, pcCall);
	if (!pcFunction)
	{
		if (mpcUnit->GetErrors()->NumErrors() == uiErrors)
		{
			mpcUnit->ReportFailedDeclaration(lliName);
			mpcUnit->Error(pcCallee, "The function has not been declared");
		}
		return;
	}
	mpcUnit->Resolved(pcName, pcFunction);

	//memcpy of a few bytes, a constant count, is copied where it is called: any function of the name, as the
	//code has always been written (the type checker asked for C linkage, and gave the same type either way).
	if (mpcUnit->IsInlineMemcpy(lliName, pcBound->mpapcArguments))
	{
		pcBound->meHow = CPPBC_InlineMemcpy;
		return;
	}
	pcBound->meHow = CPPBC_Function;
	pcBound->mpcFunction = pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// object(arguments) for an object of a class: its operator(), a member
// called on it.  False if the callee isn't an object of a class; true with
// the call still failed if no operator() takes the arguments.  [over.call]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::ResolveCallOperator(CCPPSyntaxCallExpression* pcCall, CCPPBoundCall* pcBound)
{
	SCPP65Type*		psType;
	CCPPFunction*	pcFunction;
	size			uiErrors;

	mpcUnit->BeginQuiet();
	psType = mpcUnit->ValueTypeOf(pcBound->mpcCallee);
	mpcUnit->EndQuiet();
	if (!psType || !psType->IsClass())
	{
		return false;
	}
	if (!mpcUnit->CompleteClass(psType->pcClass, pcCall))
	{
		return true;
	}
	if (psType->pcClass->mpsLambda && psType->pcClass->mpsLambda->bGeneric)
	{
		//A generic lambda's operator() for these arguments.
		mpcUnit->GenericLambdaCall(psType->pcClass->mpsLambda, pcBound->mpapcArguments);
	}

	pcBound->mlliName = mpcUnit->GetOperatorName(CT_OpenRoundBracket, false);
	uiErrors = mpcUnit->GetErrors()->NumErrors();
	pcFunction = mcOverloads.FindOperator(psType->pcClass, pcBound->mlliName, pcBound->mpapcArguments, pcCall, CalledObject(pcBound->mpcCallee, false));
	if (!pcFunction)
	{
		if (mpcUnit->GetErrors()->NumErrors() == uiErrors)
		{
			mpcUnit->Error(pcCall, "The class has no operator() for these arguments");
		}
		return true;
	}
	pcBound->meHow = CPPBC_Member;
	pcBound->mpcFunction = pcFunction;
	pcBound->mpcObject = pcBound->mpcCallee;
	pcBound->mpcClass = psType->pcClass;
	pcBound->mbVirtual = pcBound->mpcCallee->GetKind() != CPPS_IdExpression;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A call on an object, or one in a member function that means a member: the object
// and its class, then the destructor, a field that points to a function, or the member
// function of the name chosen for the arguments and for whether the object is const
// (a member template's instance if no function fits).  pcBound->mpcCallee is the
// callee: a member expression, or a name.
//
//////////////////////////////////////////////////////////////////////////
void CCPPBinder::ResolveMember(CCPPSyntaxCallExpression* pcCall, CCPPBoundCall* pcBound)
{
	CCPPSyntaxMemberExpression*	pcMember;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	CArraySyntax*				papcExplicit;
	CCPPSyntax*					pcCallee;
	CCPPSyntax*					pcObject;
	CCPPClass*					pcClass;
	CCPPClass*					pcLookup;
	CCPPFunction*				pcFunction;
	CCPPFunction*				pcIn;
	CCPPTemplate*				pcTemplate;
	SCPP65Type*					psType;
	int64						lliName;
	int							iOffset;
	ECPPCalledObject			eObject;
	size						uiErrors;
	bool						bArrow;
	bool						bVirtual;

	pcCallee = pcBound->mpcCallee;
	pcIn = mpcUnit->OuterFunction();
	pcClass = NULL;
	pcObject = NULL;
	bArrow = false;
	bVirtual = true;
	if (pcCallee->GetKind() == CPPS_MemberExpression)
	{
		pcMember = (CCPPSyntaxMemberExpression*)pcCallee;
		psType = mpcUnit->TypeOf(pcMember->mpcObject);
		if (!psType)
		{
			//Its errors.
			mpcUnit->GetTypeChecker()->Value(pcMember->mpcObject);
			return;
		}
		bArrow = pcMember->mbArrow;
		if (bArrow)
		{
			psType = mpcUnit->ArrowType(pcMember->mpcObject, pcCallee);
			psType = (psType && psType->IsPointer()) ? psType->psBase : NULL;
		}
		else
		{
			psType = psType->WithoutReference();
		}
		if (!psType || !psType->IsClass())
		{
			mpcUnit->Error(pcCallee, "Member functions need an object of a class");
			return;
		}
		pcClass = psType->pcClass;
		if (!mpcUnit->CompleteClass(pcClass, pcCallee))
		{
			return;
		}
		pcObject = pcMember->mpcObject;
		pcName = pcMember->mpcName;

		//A named object is what it is declared as: its virtual functions are called by their labels.
		if (!bArrow && (pcObject->WithoutParentheses()->GetKind() == CPPS_IdExpression))
		{
			bVirtual = false;
		}
	}
	else
	{
		pcName = ((CCPPSyntaxIdExpression*)pcCallee)->mpcName;
		if (pcName->IsQualifiedByClass())
		{
			//TEXT::SizePolicy::Copy(...): a qualifier the parser couldn't resolve is found in the class before it.
			pcClass = pcName->GetQualifier()->mpcSymbol ? mpcUnit->GetComponentClass(pcName->GetQualifier(), NULL, pcCallee) : mpcUnit->QualifiersClass(pcName, pcCallee);
			bVirtual = false;
		}
		else if (pcIn)
		{
			pcClass = pcIn->mpcClass;
		}
	}
	pcBound->mpcName = pcName;
	pcBound->mpcObject = pcObject;
	pcBound->mpcClass = pcClass;
	pcBound->mbArrow = bArrow;

	//p->~X(), o.~X(), p->X::~X() or ~X() (this's): nothing if the class has no destructor.
	if (pcName->GetLast()->meKind == CPPNC_Destructor)
	{
		if (!pcClass || (pcBound->mpapcArguments->NumElements() != 0) || !mpcUnit->CompleteClass(pcClass, pcCallee))
		{
			mpcUnit->Error(pcCallee, "This destructor call is not supported by the compiler yet");
			return;
		}
		if (pcClass->mpcDestructor)
		{
			mpcUnit->Resolved(pcName, pcClass->mpcDestructor);
		}
		pcBound->meHow = CPPBC_Destructor;
		pcBound->mpcFunction = pcClass->mpcDestructor;
		pcBound->mbVirtual = pcName->IsQualifiedByClass() ? false : bVirtual;
		return;
	}

	lliName = mpcUnit->GetFunctionName(pcName->GetLast());
	pcBound->mlliName = lliName;
	if (!pcClass || (lliName == -1))
	{
		mpcUnit->Error(pcCallee, "This call is not supported by the compiler yet");
		return;
	}

	pcLookup = pcClass;
	if (pcObject && pcName->IsQualifiedByClass())
	{
		pcLookup = mpcUnit->QualifierClass(pcClass, pcName->GetQualifier(), pcCallee);
		if (!pcLookup)
		{
			return;
		}
		bVirtual = false;
	}

	//object.pf() calls through a field that points to a function.
	if (pcObject && !mpcUnit->HasMember(pcLookup, lliName) && pcLookup->FindField(lliName, &iOffset))
	{
		psType = mpcUnit->TypeOf(pcCallee);
		if (psType && psType->IsPointer() && psType->psBase->IsFunction())
		{
			pcBound->meHow = CPPBC_Pointer;
			return;
		}
	}

	uiErrors = mpcUnit->GetErrors()->NumErrors();
	pcComponent = pcName->GetLast();
	papcExplicit = pcComponent->mbTemplateArguments ? &pcComponent->macpTemplateArguments : NULL;
	eObject = CalledObject(pcObject, bArrow);
	pcTemplate = mpcUnit->CompleteClass(pcLookup, pcCall) ? mcOverloads.ChooseMemberTemplate(pcLookup, lliName, pcCall, pcBound->mpapcArguments, papcExplicit, eObject) : NULL;
	pcFunction = NULL;
	if (!pcTemplate || !papcExplicit)
	{
		//With a member template to fall back on, that no function fits is no error.
		if (pcTemplate)
		{
			mpcUnit->BeginQuiet();
		}
		pcFunction = mcOverloads.FindFunction(pcLookup, lliName, CPPFK_Function, pcBound->mpapcArguments, true, pcCall, eObject);
		if (pcTemplate)
		{
			mpcUnit->EndQuiet();
		}
	}
	if (!pcFunction && pcTemplate)
	{
		pcFunction = mpcUnit->InstantiateFunction(pcTemplate, papcExplicit, pcBound->mpapcArguments, pcCall);
	}
	if (!pcFunction)
	{
		if (mpcUnit->GetErrors()->NumErrors() == uiErrors)
		{
			mpcUnit->Error(pcCallee, "The class has no member function of that name");
		}
		return;
	}
	if (!pcObject && !pcFunction->mbStatic && (!pcIn || !pcIn->mpcClass || !mpcUnit->IsDerived(pcIn->mpcClass, pcFunction->mpcClass, NULL)))
	{
		mpcUnit->Error(pcCallee, "A member function needs an object");
		return;
	}
	mpcUnit->Resolved(pcName, pcFunction);
	pcBound->meHow = CPPBC_Member;
	pcBound->mpcFunction = pcFunction;
	pcBound->mbVirtual = bVirtual;
}


//////////////////////////////////////////////////////////////////////////
//
// left op right, or op left when pcRight is NULL, where an operand is an object:
// the left object's own operator function if it has one that fits (CPPBC_Member, on
// pcBound->mpcObject), or else an operator function that is no class's, declared or a
// template's instance (CPPBC_Function, whose arguments are the operands).  False,
// with no error of its own, if neither fits.  [over.match.oper]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::BindOperator(ECppTokens eOperator, CCPPSyntax* pcLeft, CCPPSyntax* pcRight, CCPPSyntax* pcNode, CCPPBoundCall* pcBound, bool bReport)
{
	return BindAnOperator(eOperator, pcLeft, pcRight, pcNode, pcBound, bReport, false);
}


//////////////////////////////////////////////////////////////////////////
//
// object op argument (pcArgument NULL for none) as a call of the object's own
// operator function: object[index], object->, and an operator that must be a member.
// False with an error if it has none that fits.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::BindOwnOperator(ECppTokens eOperator, CCPPSyntax* pcObject, CCPPSyntax* pcArgument, CCPPSyntax* pcNode, CCPPBoundCall* pcBound, bool bReport)
{
	return BindAnOperator(eOperator, pcObject, pcArgument, pcNode, pcBound, bReport, true);
}


//////////////////////////////////////////////////////////////////////////
//
// Either, remembered by its left operand for the same question asked again.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::BindAnOperator(ECppTokens eOperator, CCPPSyntax* pcLeft, CCPPSyntax* pcRight, CCPPSyntax* pcNode, CCPPBoundCall* pcBound, bool bReport, bool bOwnOnly)
{
	CCPPBoundCall*	pcRemembered;
	int64			lliName;

	lliName = mpcUnit->GetOperatorName(eOperator, false);
	mpcUnit->GetStatistics()->Count(CPPCC_BoundCalls);
	pcRemembered = mcOperators.Get((int64)(size)pcLeft);
	if (pcRemembered && (pcRemembered->mpcLeft == pcLeft) && (pcRemembered->mpcRight == pcRight) && (pcRemembered->mlliName == lliName) && (pcRemembered->mbOwnOnly == bOwnOnly) &&
		(pcRemembered->mpcBindings == mpcUnit->GetContext()->GetBindings()) && (pcRemembered->mpcIn == mpcUnit->GetContext()->GetFunction()))
	{
		if ((pcRemembered->meHow != CPPBC_Failed) || !bReport)
		{
			*pcBound = *pcRemembered;
			return pcBound->meHow != CPPBC_Failed;
		}
	}

	mpcUnit->GetStatistics()->Count(CPPCC_BoundCallsResolved);
	pcBound->Init();
	pcBound->mpcBindings = mpcUnit->GetContext()->GetBindings();
	pcBound->mpcIn = mpcUnit->GetContext()->GetFunction();
	pcBound->mpcLeft = pcLeft;
	pcBound->mpcRight = pcRight;
	pcBound->mlliName = lliName;
	pcBound->mbOwnOnly = bOwnOnly;
	if (bOwnOnly)
	{
		ResolveOwnOperator(pcNode, pcBound);
	}
	else
	{
		ResolveOperator(pcNode, pcBound);
	}
	CheckAccess(pcBound, pcNode);
	mcOperators.Put((int64)(size)pcLeft, pcBound);
	return pcBound->meHow != CPPBC_Failed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPBinder::ResolveOperator(CCPPSyntax* pcNode, CCPPBoundCall* pcBound)
{
	SCPP65Type*		psType;
	CCPPFunction*	pcFunction;
	CArraySyntax	apcArguments;
	size			uiErrors;

	psType = mpcUnit->ValueTypeOf(pcBound->mpcLeft);
	apcArguments.Init();
	if (psType && psType->IsClass() && mpcUnit->CompleteClass(psType->pcClass, pcNode) && mpcUnit->HasOperator(psType->pcClass, pcBound->mlliName, pcBound->mpcRight ? 1 : 0))
	{
		if (pcBound->mpcRight)
		{
			apcArguments.Add(pcBound->mpcRight);
		}

		//That none of its own fits is no error: one that is no class's may.
		uiErrors = mpcUnit->GetErrors()->NumErrors();
		mpcUnit->BeginQuiet();
		pcFunction = mcOverloads.FindOperator(psType->pcClass, pcBound->mlliName, &apcArguments, pcNode, CalledObject(pcBound->mpcLeft, false));
		mpcUnit->EndQuiet();
		mpcUnit->GetErrors()->Truncate(uiErrors);
		if (pcFunction)
		{
			apcArguments.Kill();
			pcBound->meHow = CPPBC_Member;
			pcBound->mpcFunction = pcFunction;
			pcBound->mpcObject = pcBound->mpcLeft;
			pcBound->mpcClass = psType->pcClass;
			pcBound->mbVirtual = pcBound->mpcLeft->WithoutParentheses()->GetKind() != CPPS_IdExpression;
			return;
		}
		apcArguments.ReInit();
	}

	//operator op(left, right), declared or from a function template.
	apcArguments.Add(pcBound->mpcLeft);
	if (pcBound->mpcRight)
	{
		apcArguments.Add(pcBound->mpcRight);
	}
	pcFunction = mcOverloads.FindFreeOperator(pcBound->mlliName, &apcArguments, pcNode);
	apcArguments.Kill();
	if (pcFunction)
	{
		pcBound->meHow = CPPBC_Function;
		pcBound->mpcFunction = pcFunction;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPBinder::ResolveOwnOperator(CCPPSyntax* pcNode, CCPPBoundCall* pcBound)
{
	SCPP65Type*		psType;
	CCPPFunction*	pcFunction;
	CArraySyntax	apcArguments;
	size			uiErrors;

	psType = mpcUnit->ValueTypeOf(pcBound->mpcLeft);
	if (!psType || !psType->IsClass())
	{
		mpcUnit->Error(pcNode, "An operator call needs an object");
		return;
	}
	if (!mpcUnit->CompleteClass(psType->pcClass, pcNode))
	{
		return;
	}
	apcArguments.Init();
	if (pcBound->mpcRight)
	{
		apcArguments.Add(pcBound->mpcRight);
	}
	uiErrors = mpcUnit->GetErrors()->NumErrors();
	pcFunction = mcOverloads.FindOperator(psType->pcClass, pcBound->mlliName, &apcArguments, pcNode, CalledObject(pcBound->mpcLeft, false));
	apcArguments.Kill();
	if (!pcFunction)
	{
		if (mpcUnit->GetErrors()->NumErrors() == uiErrors)
		{
			mpcUnit->Error(pcNode, "The class has no operator for this");
		}
		return;
	}
	pcBound->meHow = CPPBC_Member;
	pcBound->mpcFunction = pcFunction;
	pcBound->mpcObject = pcBound->mpcLeft;
	pcBound->mpcClass = psType->pcClass;
	pcBound->mbVirtual = pcBound->mpcLeft->WithoutParentheses()->GetKind() != CPPS_IdExpression;
}


//////////////////////////////////////////////////////////////////////////
//
// The function a name used for its address means: the only one of the name, or the
// one of psTarget's type.  *pbOverloaded if there are several and none is chosen.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPBinder::FindAddressedFunction(CCPPSyntaxName* pcName, int64 lliName, SCPP65Type* psTarget, CCPPSyntax* pcNode, bool* pbOverloaded)
{
	return mcOverloads.FindAddressedFunction(pcName, lliName, psTarget, pcNode, pbOverloaded);
}


//////////////////////////////////////////////////////////////////////////
//
// The class's operator-> for the object, or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPBinder::FindArrowOperator(CCPPClass* pcClass, CCPPSyntax* pcObject, CCPPSyntax* pcNode)
{
	CArraySyntax	apcNone;
	CCPPFunction*	pcFunction;

	apcNone.Init();
	pcFunction = mcOverloads.FindFunction(pcClass, mpcUnit->GetOperatorName(CT_MemberByPointer, false), CPPFK_Function, &apcNone, true, pcNode, CPPCalledObject(mpcUnit->IsConstObject(pcObject)));
	apcNone.Kill();
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the class (or a base) has a member template of the name that takes
// uiArguments arguments.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::HasMemberTemplate(CCPPClass* pcClass, int64 lliName, size uiArguments)
{
	return mcOverloads.FindMemberTemplate(pcClass, lliName, uiArguments) != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether converting psFrom to psTo, the same type but for const and volatile,
// takes none away.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::KeepsQualifiers(SCPP65Type* psTo, SCPP65Type* psFrom)
{
	return mcOverloads.QualificationRank(psTo, psFrom, false) != CPPCR_None;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the function takes each of the arguments as its parameter's own type, but
// for const and a reference: no argument is converted.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::TakesAsTheyAre(CCPPFunction* pcFunction, CArraySyntax* papcArguments)
{
	SCPP65Type*	psArgument;
	size		i;

	if (pcFunction->mbVariadic || (pcFunction->mapsParameters.NumElements() != papcArguments->NumElements()))
	{
		return false;
	}
	for (i = 0; i < papcArguments->NumElements(); i++)
	{
		psArgument = mpcUnit->ValueTypeOf(papcArguments->GetPtr(i));
		if (!psArgument || !pcFunction->mapsParameters.GetPtr(i)->WithoutReference()->Unqualified()->Equals(psArgument->Unqualified()))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// begin or end of a range-for's range (pcRange, an object): the member of
// that name of pcClass that takes nothing, or with no class the function
// of that name that takes the range, of the namespaces around and those
// its type is of.  NULL, quietly, if there is none.  [stmt.ranged] 1
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPBinder::BindRangeFunction(int64 lliName, CCPPClass* pcClass, CCPPSyntax* pcRange, CCPPSyntax* pcNode)
{
	CArraySyntax	apcArguments;
	CCPPFunction*	pcFunction;
	size			uiErrors;

	apcArguments.Init();
	if (pcClass)
	{
		uiErrors = mpcUnit->GetErrors()->NumErrors();
		mpcUnit->BeginQuiet();
		pcFunction = mcOverloads.FindFunction(pcClass, lliName, CPPFK_Function, &apcArguments, true, pcNode, CalledObject(pcRange, false));
		mpcUnit->EndQuiet();
		mpcUnit->GetErrors()->Truncate(uiErrors);
	}
	else
	{
		apcArguments.Add(pcRange);
		pcFunction = mcOverloads.FindFreeOperator(lliName, &apcArguments, pcNode);
	}
	apcArguments.Kill();
	if (pcFunction && !mcAccess.CheckFunction(pcFunction, pcClass, pcNode))
	{
		return NULL;
	}
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// The object a member function is called on, for the choice among const
// and & and && overloads: pcObject (what a pointer points to with
// bArrow), or the function's own when there is none.
//
//////////////////////////////////////////////////////////////////////////
ECPPCalledObject CCPPBinder::CalledObject(CCPPSyntax* pcObject, bool bArrow)
{
	if (!pcObject)
	{
		return CPPCalledObject(mpcUnit->IsConstThis());
	}
	if (bArrow)
	{
		return CPPCalledObject(mpcUnit->IsConstPointer(pcObject));
	}
	return CPPCalledObject(mpcUnit->IsConstObject(pcObject), mpcUnit->IsRvalue(pcObject));
}


//////////////////////////////////////////////////////////////////////////
//
// f(args...): the call's arguments with args... replaced by a name for each of the
// function parameter pack's elements, in an array the binder keeps.  The call node's
// own arguments are left as they are: the same call is in instances with other
// packs.  Only the function parameter pack itself can be expanded: NULL, with an
// error, for another.
//
//////////////////////////////////////////////////////////////////////////
CArraySyntax* CCPPBinder::ExpandPack(CCPPSyntaxCallExpression* pcCall)
{
	CArraySyntax*				papcArguments;
	CArrayCPPSymbols*			papcElements;
	CCPPSyntax*					pcArgument;
	CCPPSyntax*					pcPattern;
	CCPPSyntaxIdExpression*		pcId;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPSyntaxNameComponent*	pcLast;
	CCPPSyntaxMemory*			pcMemory;
	CCPPSymbol*					pcPack;
	size						i;
	size						j;

	mpcUnit->GetStatistics()->Count(CPPCC_PackExpansions);
	papcArguments = NewMalloc<CArraySyntax>();
	papcArguments->Init();
	mapapcExpanded.Add(papcArguments);
	pcMemory = mpcUnit->GetTree()->GetMemory();
	pcPack = mpcUnit->GetContext()->GetPackParameter();
	papcElements = mpcUnit->GetPackElements();
	for (i = 0; i < pcCall->macpArguments.NumElements(); i++)
	{
		pcArgument = pcCall->macpArguments.GetPtr(i);
		if (pcArgument->GetKind() != CPPS_PackExpansionExpression)
		{
			papcArguments->Add(pcArgument);
			continue;
		}
		pcPattern = ((CCPPSyntaxPackExpansionExpression*)pcArgument)->mpcPattern->WithoutParentheses();
		if (!pcPack || (pcPattern->GetKind() != CPPS_IdExpression) || (((CCPPSyntaxIdExpression*)pcPattern)->mpcName->mpcSymbol != pcPack))
		{
			mpcUnit->Error(pcArgument, "Only a function parameter pack can be expanded by the compiler yet");
			return NULL;
		}
		pcLast = ((CCPPSyntaxIdExpression*)pcPattern)->mpcName->GetLast();
		for (j = 0; j < papcElements->NumElements(); j++)
		{
			mpcUnit->GetStatistics()->Count(CPPCC_PackExpansionNodes, 3);
			pcId = pcMemory->Create<CCPPSyntaxIdExpression>(pcCall);
			pcId->SetTokens(pcPattern->miFirstToken, pcPattern->miLastToken);
			pcName = pcMemory->Create<CCPPSyntaxName>(pcId);
			pcName->SetTokens(pcPattern->miFirstToken, pcPattern->miLastToken);
			pcName->mpcSymbol = papcElements->GetPtr(j);
			pcComponent = pcMemory->Create<CCPPSyntaxNameComponent>(pcName);
			pcComponent->SetTokens(pcPattern->miFirstToken, pcPattern->miLastToken);
			pcComponent->meKind = pcLast->meKind;
			pcComponent->miName = pcLast->miName;
			pcComponent->mpcSymbol = pcName->mpcSymbol;
			pcName->macpComponents.Add(pcComponent);
			pcId->mpcName = pcName;
			papcArguments->Add(pcId);
		}
	}
	return papcArguments;
}


//////////////////////////////////////////////////////////////////////////
//
// What a name used as an expression is (ResolveName), which must be what
// may be named there: a field or a static member of a class, an
// enumerator or a member function.
//
//////////////////////////////////////////////////////////////////////////
void CCPPBinder::BindName(CCPPSyntaxIdExpression* pcId, CCPPBoundName* pcBound)
{
	ResolveName(pcId, pcBound);
	if ((pcBound->meWhat != CPPBN_Nothing) && (pcBound->meWhat != CPPBN_FunctionName))
	{
		mcAccess.CheckSymbol(pcBound->mpcSymbol, pcBound->mpcClass, pcId);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What the name is, in *pcBound: the one cascade, in the order a name is looked at.
// It reports nothing and remembers nothing (a local's variable is made again each
// time its function is written): what a name can't be used as is its user's error.
//
//////////////////////////////////////////////////////////////////////////
void CCPPBinder::ResolveName(CCPPSyntaxIdExpression* pcId, CCPPBoundName* pcBound)
{
	CCPPSymbol*		pcSymbol;
	CCPPFunction*	pcIn;
	CCPPClass*		pcClass;
	CCPPClass*		pcEnumeratorClass;
	CCPPClass*		pcMemberClass;

	pcBound->meWhat = CPPBN_Nothing;
	pcBound->mpcVariable = NULL;
	pcBound->mpcClass = NULL;
	pcBound->mpsField = NULL;
	pcBound->miOffset = 0;
	pcBound->mpsCapture = NULL;
	pcSymbol = pcId->mpcName->mpcSymbol;
	pcBound->mpcSymbol = pcSymbol;

	//In a lambda's body, what it captures is its closure's.
	pcBound->mpsCapture = (pcId->mpcName->macpComponents.NumElements() == 1) ? mpcUnit->FindCapture(pcSymbol) : NULL;
	if (pcBound->mpsCapture)
	{
		pcBound->meWhat = CPPBN_Capture;
		pcBound->mpcClass = mpcUnit->WritingLambda()->pcClass;
		pcBound->mpsField = pcBound->mpcClass->macFields.Get(pcBound->mpsCapture->uiField);
		pcBound->miOffset = pcBound->mpsField->iOffset;
		return;
	}
	if (mpcUnit->IsFunctionName(pcId))
	{
		pcBound->meWhat = CPPBN_FunctionName;
		return;
	}

	//A static member is the variable of the class the name means: every instance of a class template has its own
	//under the template's symbol.  SIZE::Max with SIZE bound to a class, STwice<7>::Value, and in a member function
	//of an instance its own Max.
	pcBound->mpcVariable = NULL;
	if (!pcSymbol || (pcSymbol->meKind == CPPSK_Field))
	{
		pcMemberClass = NULL;
		if (pcId->mpcName->macpComponents.NumElements() > 1)
		{
			mpcUnit->QualifiedMember(pcId->mpcName, CPPSK_Field, pcId, &pcMemberClass);
		}
		else if (pcSymbol)
		{
			pcIn = mpcUnit->OuterFunction();
			pcMemberClass = (pcIn && pcIn->mpcClass && pcIn->mpcClass->mpcTemplate) ? pcIn->mpcClass : NULL;
		}
		if (pcMemberClass)
		{
			pcBound->mpcVariable = mpcUnit->FindStatic(pcMemberClass, mpcUnit->GetComponentName(pcId->mpcName->GetLast()));
		}
	}

	//A local, a parameter, a global or a static member.
	if (!pcBound->mpcVariable && pcSymbol)
	{
		pcBound->mpcVariable = mpcUnit->GetVariable(pcSymbol);
	}
	if (pcBound->mpcVariable)
	{
		pcBound->meWhat = CPPBN_Variable;
		return;
	}

	if (!pcSymbol)
	{
		pcSymbol = mpcUnit->QualifiedEnumerator(pcId->mpcName, pcId, &pcEnumeratorClass);
		pcBound->mpcSymbol = pcSymbol;
	}
	if (pcSymbol && ((pcSymbol->meKind == CPPSK_Enumerator) || (pcSymbol->meKind == CPPSK_ValueParameter)))
	{
		pcBound->meWhat = CPPBN_Constant;
		return;
	}
	if (pcSymbol && (pcSymbol->meKind == CPPSK_Function))
	{
		pcBound->meWhat = CPPBN_Function;
		return;
	}
	if (pcSymbol && (pcSymbol->meKind == CPPSK_FunctionTemplate))
	{
		pcBound->meWhat = CPPBN_FunctionTemplate;
		return;
	}

	//A field of this; a name in a template that the parser could not resolve may be one too.
	pcClass = mpcUnit->ThisClass();
	if (pcSymbol && (pcSymbol->meKind == CPPSK_Field) && pcClass)
	{
		pcBound->mpsField = pcClass->FindFieldSymbol(pcSymbol, &pcBound->miOffset);
	}
	else if (!pcSymbol && pcClass && (pcId->mpcName->macpComponents.NumElements() == 1))
	{
		pcBound->mpsField = pcClass->FindField(mpcUnit->GetComponentName(pcId->mpcName->GetLast()), &pcBound->miOffset);
	}
	if (pcBound->mpsField)
	{
		pcBound->meWhat = CPPBN_Field;
		pcBound->mpcClass = pcClass;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// object.name and pointer->name, once the object's class is known: the class's field
// of the name, or a base's, with where it is in the object (CPPBN_Field), or else its
// static member (CPPBN_Variable).  False if it has neither (CPPBN_Nothing), or the
// class can't be completed, which is reported at pcNode.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::BindMember(CCPPClass* pcClass, CCPPSyntaxName* pcName, CCPPSyntax* pcNode, CCPPBoundName* pcBound)
{
	int64	lliName;

	pcBound->meWhat = CPPBN_Nothing;
	pcBound->mpcSymbol = NULL;
	pcBound->mpcVariable = NULL;
	pcBound->mpcClass = pcClass;
	pcBound->mpsField = NULL;
	pcBound->miOffset = 0;
	if (!pcClass || !mpcUnit->CompleteClass(pcClass, pcNode))
	{
		return false;
	}
	lliName = mpcUnit->GetComponentName(pcName->GetLast());
	pcBound->mpsField = pcClass->FindField(lliName, &pcBound->miOffset);
	if (pcBound->mpsField)
	{
		pcBound->meWhat = CPPBN_Field;
		pcBound->mpcSymbol = pcBound->mpsField->pcSymbol;
	}
	else
	{
		pcBound->mpcVariable = mpcUnit->FindStatic(pcClass, lliName);
		if (!pcBound->mpcVariable)
		{
			return false;
		}
		pcBound->meWhat = CPPBN_Variable;
		pcBound->mpcSymbol = pcBound->mpcVariable->mpcSymbol;
	}

	//It is the member whether or not it may be named here: that it may not is an error of its own.
	mcAccess.CheckSymbol(pcBound->mpcSymbol, pcClass, pcNode);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// How the expression's value becomes psTo (NULL when no type is known): an object
// wanted as a type that isn't a class or a reference is converted by its class's
// conversion function, if one converts to it; an explicit one only when bExplicit (a
// cast, or a condition).  Anything else is used as it is.  [class.conv.fct]
//
//////////////////////////////////////////////////////////////////////////
void CCPPBinder::BindConversion(CCPPSyntax* pcExpression, SCPP65Type* psTo, bool bExplicit, CCPPBoundConversion* pcBound)
{
	pcBound->meHow = CPPBV_AsItIs;
	pcBound->mpcFunction = NULL;
	pcBound->mpcClass = NULL;
	if (psTo && !psTo->IsClass() && !psTo->IsReference() && !psTo->IsVoid() && mpcUnit->IsObject(pcExpression))
	{
		BindObjectConversion(mpcUnit->ObjectClass(pcExpression), psTo, bExplicit, pcExpression, pcBound);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The same for an object of pcClass that is already a value.
//
//////////////////////////////////////////////////////////////////////////
void CCPPBinder::BindObjectConversion(CCPPClass* pcClass, SCPP65Type* psTo, bool bExplicit, CCPPSyntax* pcNode, CCPPBoundConversion* pcBound)
{
	pcBound->mpcClass = pcClass;
	pcBound->mpcFunction = mcOverloads.FindConversion(pcClass, psTo, bExplicit, pcNode);
	pcBound->meHow = pcBound->mpcFunction ? CPPBV_Function : CPPBV_AsItIs;
}


//////////////////////////////////////////////////////////////////////////
//
// A type named in a declaration (Class::Nested, or a name alone): if it is
// a class's member (a nested class or enum, or a typedef) it must be one
// that may be named there.
//
//////////////////////////////////////////////////////////////////////////
void CCPPBinder::CheckTypeName(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode)
{
	mcAccess.CheckSymbol(pcSymbol, NULL, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// How an object of pcClass is made from papcArguments: by the constructor that
// matches them; by nothing, when it has no constructor and there are none; or else as
// an aggregate, its fields in order, which a class with bases or virtual functions
// can't be.  False if it can't be made; its own two errors are reported only when
// bReport (the type checker's: the front end is asked what was already checked).
// [dcl.init]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::BindConstruction(CCPPClass* pcClass, CArraySyntax* papcArguments, CCPPSyntax* pcNode, CCPPBoundConstruction* pcBound, bool bReport)
{
	pcBound->meHow = CPPBO_Failed;
	pcBound->mpcConstructor = NULL;
	if (!mpcUnit->CompleteClass(pcClass, pcNode))
	{
		return false;
	}
	if (pcClass->mbConstructors || (papcArguments->NumElements() == 0))
	{
		pcBound->mpcConstructor = mcOverloads.FindFunction(pcClass, pcClass->mlliName, CPPFK_Constructor, papcArguments, false, pcNode);
		if (pcBound->mpcConstructor)
		{
			if (!mcAccess.CheckFunction(pcBound->mpcConstructor, pcClass, pcNode))
			{
				return false;
			}
			pcBound->meHow = CPPBO_Constructor;
			return true;
		}
		if (pcClass->mbConstructors)
		{
			if (bReport)
			{
				mpcUnit->Error(pcNode, "No constructor matches the arguments");
			}
			return false;
		}
		pcBound->meHow = CPPBO_Nothing;
		return true;
	}

	if ((pcClass->macBases.NumElements() > 0) || pcClass->IsPolymorphic() || (papcArguments->NumElements() > pcClass->NumAggregateFields()))
	{
		if (bReport)
		{
			mpcUnit->Error(pcNode, "These initialisers do not match the class");
		}
		return false;
	}
	pcBound->meHow = CPPBO_Aggregate;
	return true;
}

