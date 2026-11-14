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
#include "CPPBoundCall.h"


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
void CCPPCallBinder::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
	mcBound.Init(true);
	mcOperators.Init(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPCallBinder::Kill(void)
{
	mcOperators.Kill();
	mcBound.Kill();
	mpcUnit = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// What the call is, in *pcBound.  False if it can't be made (CPPBC_Failed).  A call
// whose arguments have a pack to expand is CPPBC_PackExpansion every time: it is
// bound again, as what it is, once its caller has expanded them.  bReport when the
// errors of a call that failed before are wanted again (the type checker's, which
// ReportTypeOf reports); without it a failure is as it was remembered.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPCallBinder::Bind(CCPPSyntaxCallExpression* pcCall, CCPPBoundCall* pcBound, bool bReport)
{
	CCPPBoundCall*	pcRemembered;

	if (mpcUnit->HasPackExpansion(&pcCall->macpArguments))
	{
		pcBound->Init();
		pcBound->meHow = CPPBC_PackExpansion;
		return true;
	}

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
	Resolve(pcCall, pcBound);
	mcBound.Put((int64)(size)pcCall, pcBound);
	return pcBound->meHow != CPPBC_Failed;
}


//////////////////////////////////////////////////////////////////////////
//
// The cascade.  pcBound->meHow is CPPBC_Failed until something else is found.
//
//////////////////////////////////////////////////////////////////////////
void CCPPCallBinder::Resolve(CCPPSyntaxCallExpression* pcCall, CCPPBoundCall* pcBound)
{
	CCPPSyntax*					pcCallee;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	CArraySyntax*				papcExplicit;
	CCPPSymbol*					pcSymbol;
	CCPPFunction*				pcFunction;
	CCPPFunction*				pcMember;
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
	pcMember = mpcUnit->GetContext()->GetFunction();

	//f<Arguments>(...) and f(...) for a function template; in a member function it may be a member template.
	if (pcSymbol && (pcSymbol->meKind == CPPSK_FunctionTemplate))
	{
		pcBound->mlliName = pcSymbol->mlliName;
		if (pcMember && pcMember->mpcClass && !pcName->mbGlobal && (mpcUnit->GetOverloads()->FindMemberTemplate(pcMember->mpcClass, pcSymbol->mlliName, pcCall->macpArguments.NumElements()) || mpcUnit->HasMember(pcMember->mpcClass, pcSymbol->mlliName)))
		{
			ResolveMember(pcCall, pcBound);
			return;
		}
		papcExplicit = (pcComponent && pcComponent->mbTemplateArguments) ? &pcComponent->macpTemplateArguments : NULL;
		pcTemplate = mpcUnit->GetOverloads()->GetCallTemplate(pcSymbol, pcSymbol->mlliName, pcCall, papcExplicit);
		if (!pcTemplate)
		{
			mpcUnit->Error(pcCallee, "The function template has not been defined");
			return;
		}
		pcFunction = mpcUnit->InstantiateFunction(pcTemplate, papcExplicit, &pcCall->macpArguments, pcCall);
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

	if (!pcSymbol && mpcUnit->IsInlineMemcpy(lliName, &pcCall->macpArguments))
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
		pcFunction = mpcUnit->GetOverloads()->FindAssociatedFunction(pcName, lliName, pcCall);
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
		//A variable, or an expression: a pointer to a function if it is anything.
		pcBound->meHow = CPPBC_Pointer;
		return;
	}

	uiErrors = mpcUnit->GetErrors()->NumErrors();
	pcFunction = mpcUnit->GetOverloads()->FindNamedFunction(pcName, lliName, &pcCall->macpArguments, pcCall);
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
	if (mpcUnit->IsInlineMemcpy(lliName, &pcCall->macpArguments))
	{
		pcBound->meHow = CPPBC_InlineMemcpy;
		return;
	}
	pcBound->meHow = CPPBC_Function;
	pcBound->mpcFunction = pcFunction;
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
void CCPPCallBinder::ResolveMember(CCPPSyntaxCallExpression* pcCall, CCPPBoundCall* pcBound)
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
	pcIn = mpcUnit->GetContext()->GetFunction();
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
			pcClass = mpcUnit->GetComponentClass(pcName->GetQualifier(), NULL, pcCallee);
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
		if (!pcClass || (pcCall->macpArguments.NumElements() != 0) || !mpcUnit->CompleteClass(pcClass, pcCallee))
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
	eObject = CPPCalledObject(pcObject ? (bArrow ? mpcUnit->IsConstPointer(pcObject) : mpcUnit->IsConstObject(pcObject)) : mpcUnit->IsConstThis());
	pcTemplate = mpcUnit->CompleteClass(pcLookup, pcCall) ? mpcUnit->GetOverloads()->ChooseMemberTemplate(pcLookup, lliName, pcCall, papcExplicit, eObject) : NULL;
	pcFunction = NULL;
	if (!pcTemplate || !papcExplicit)
	{
		//With a member template to fall back on, that no function fits is no error.
		if (pcTemplate)
		{
			mpcUnit->BeginQuiet();
		}
		pcFunction = mpcUnit->GetOverloads()->FindFunction(pcLookup, lliName, CPPFK_Function, &pcCall->macpArguments, true, pcCall, eObject);
		if (pcTemplate)
		{
			mpcUnit->EndQuiet();
		}
	}
	if (!pcFunction && pcTemplate)
	{
		pcFunction = mpcUnit->InstantiateFunction(pcTemplate, papcExplicit, &pcCall->macpArguments, pcCall);
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
bool CCPPCallBinder::BindOperator(ECppTokens eOperator, CCPPSyntax* pcLeft, CCPPSyntax* pcRight, CCPPSyntax* pcNode, CCPPBoundCall* pcBound, bool bReport)
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
bool CCPPCallBinder::BindOwnOperator(ECppTokens eOperator, CCPPSyntax* pcObject, CCPPSyntax* pcArgument, CCPPSyntax* pcNode, CCPPBoundCall* pcBound, bool bReport)
{
	return BindAnOperator(eOperator, pcObject, pcArgument, pcNode, pcBound, bReport, true);
}


//////////////////////////////////////////////////////////////////////////
//
// Either, remembered by its left operand for the same question asked again.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPCallBinder::BindAnOperator(ECppTokens eOperator, CCPPSyntax* pcLeft, CCPPSyntax* pcRight, CCPPSyntax* pcNode, CCPPBoundCall* pcBound, bool bReport, bool bOwnOnly)
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
	mcOperators.Put((int64)(size)pcLeft, pcBound);
	return pcBound->meHow != CPPBC_Failed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPCallBinder::ResolveOperator(CCPPSyntax* pcNode, CCPPBoundCall* pcBound)
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
		pcFunction = mpcUnit->GetOverloads()->FindOperator(psType->pcClass, pcBound->mlliName, &apcArguments, pcNode, CPPCalledObject(mpcUnit->IsConstObject(pcBound->mpcLeft)));
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
	pcFunction = mpcUnit->GetOverloads()->FindFreeOperator(pcBound->mlliName, &apcArguments, pcNode);
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
void CCPPCallBinder::ResolveOwnOperator(CCPPSyntax* pcNode, CCPPBoundCall* pcBound)
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
	pcFunction = mpcUnit->GetOverloads()->FindOperator(psType->pcClass, pcBound->mlliName, &apcArguments, pcNode, CPPCalledObject(mpcUnit->IsConstObject(pcBound->mpcLeft)));
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

