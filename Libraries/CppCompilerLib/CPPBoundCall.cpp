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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPCallBinder::Kill(void)
{
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
		pcBound->meHow = CPPBC_Member;
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
			pcBound->meHow = CPPBC_Member;
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
			pcBound->meHow = CPPBC_Member;
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
		pcBound->meHow = CPPBC_Member;
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
