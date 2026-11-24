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
	mpcQualifier = NULL;
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
	mpcDiagnostics = pcUnit->GetDiagnostics();
	mcOverloads.Init(pcUnit);
	mcAccess.Init(pcUnit);
	mcNames.Init(pcUnit, this);
	mcBound.Init();
	mcOperators.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPBinder::Kill(void)
{
	mcOperators.Kill();
	mcBound.Kill();
	mcOverloads.Kill();
	mcNames.Kill();
	mpcUnit = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPNames* CCPPBinder::GetNames(void) { return &mcNames; }


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
// arguments are the bound call's: the node's own, or with their pack expansions
// expanded (CCPPUnit::ExpandArguments).  bReport when the
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
	pcBound->mpapcArguments = mpcUnit->ExpandArguments(&pcCall->macpArguments);
	if (pcBound->mpapcArguments)
	{
		Resolve(pcCall, pcBound);
		if ((pcBound->meHow != CPPBC_Failed) && pcBound->mpcFunction && pcBound->mpcFunction->mbDeleted && pcBound->mpcFunction->mpcTemplate)
		{
			//A deleted function template's instance was the best match (one that is no template's is refused where it
			//is chosen, CCPPOverloads::FindFunction).  [dcl.fct.def.delete] 2
			mpcDiagnostics->ErrorOnce(pcCall, "The function chosen is deleted");
			pcBound->meHow = CPPBC_Failed;
		}
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
	bool						bInlineMemcpy;
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
	pcMember = mpcUnit->GetClosures()->OuterFunction();

	//In a class but not in its member functions (sizeof(test<T>(0)) in an enumerator), a member's name is its member.
	if (pcName && !pcName->mbGlobal && !(pcMember && pcMember->mpcClass) && mcNames.MemberScopeClass(pcSymbol))
	{
		ResolveMember(pcCall, pcBound);
		return;
	}

	//f<Arguments>(...) and f(...) for a function template; in a member function it may be a member template.
	if (pcSymbol && (pcSymbol->meKind == CPPSK_FunctionTemplate))
	{
		pcBound->mlliName = pcSymbol->mlliName;
		if (pcMember && pcMember->mpcClass && !pcName->mbGlobal && (mcOverloads.FindMemberTemplate(pcMember->mpcClass, pcSymbol->mlliName, pcBound->mpapcArguments->NumElements()) || mcNames.HasMember(pcMember->mpcClass, pcSymbol->mlliName)))
		{
			ResolveMember(pcCall, pcBound);
			return;
		}
		papcExplicit = (pcComponent && pcComponent->mbTemplateArguments) ? &pcComponent->macpTemplateArguments : NULL;

		//A function of the name that isn't a template's, and takes the arguments as they are, is the better match
		//than any template's instance.  [over.match.best] 2.4
		pcFunction = NULL;
		if (!papcExplicit)
		{
			mpcDiagnostics->BeginSilentTrial();
			pcFunction = mcOverloads.FindNamedFunction(pcName, pcSymbol->mlliName, pcBound->mpapcArguments, pcCall);
			mpcDiagnostics->EndSilentTrial();
			if (pcFunction && TakesAsTheyAre(pcFunction, pcBound->mpapcArguments))
			{
				mpcUnit->GetCheck()->Resolved(pcName, pcFunction);
				pcBound->meHow = CPPBC_Function;
				pcBound->mpcFunction = pcFunction;
				return;
			}
		}
		mcOverloads.mbAmbiguousTemplate = false;
		pcTemplate = mcOverloads.GetCallTemplate(pcSymbol, pcSymbol->mlliName, pcCall, pcBound->mpapcArguments, papcExplicit);
		if (!pcTemplate)
		{
			mpcDiagnostics->Error(pcCallee, "The function template has not been defined");
			return;
		}
		if (mcOverloads.mbAmbiguousTemplate && (!pcFunction || !mcOverloads.TemplateTakes(pcTemplate, papcExplicit, pcBound->mpapcArguments, pcCall, NULL) || !mcOverloads.IsBetterFunction(pcFunction, mpcUnit->GetInstantiator()->InstantiateFunction(pcTemplate, papcExplicit, pcBound->mpapcArguments, pcCall), pcBound->mpapcArguments)))
		{
			mpcDiagnostics->Error(pcCallee, "The call is ambiguous: function templates that take the arguments as well as each other, neither more specialised");
			return;
		}
		if (pcFunction && !mcOverloads.TemplateTakes(pcTemplate, papcExplicit, pcBound->mpapcArguments, pcCall, NULL))
		{
			//The template can't be used with the arguments (SFINAE: a default template argument or the function's
			//type fails), so the function that takes them converted is.  [temp.deduct] 8
			mpcUnit->GetCheck()->Resolved(pcName, pcFunction);
			pcBound->meHow = CPPBC_Function;
			pcBound->mpcFunction = pcFunction;
			return;
		}
		pcInstance = mpcUnit->GetInstantiator()->InstantiateFunction(pcTemplate, papcExplicit, pcBound->mpapcArguments, pcCall);
		if (!pcInstance)
		{
			return;
		}
		if (!pcFunction || mcOverloads.IsBetterFunction(pcInstance, pcFunction, pcBound->mpapcArguments))
		{
			//The instance, unless the function that isn't a template's takes the arguments as well as it does.
			//[over.match.best] 1
			pcFunction = pcInstance;
		}
		mpcUnit->GetCheck()->Resolved(pcName, pcFunction);
		pcBound->meHow = CPPBC_Function;
		pcBound->mpcFunction = pcFunction;
		return;
	}

	//f() in a member function may be a member, even when the parser could not resolve it in a template.
	lliName = pcSymbol ? pcSymbol->mlliName : mpcUnit->GetFunctionName(pcComponent);
	pcBound->mlliName = lliName;
	if (pcName && !pcName->mbGlobal && pcMember && pcMember->mpcClass && (!pcSymbol || (pcSymbol->meKind == CPPSK_Function)) && mcNames.HasMember(pcMember->mpcClass, lliName))
	{
		ResolveMember(pcCall, pcBound);
		return;
	}
	if (pcName && !pcName->mbGlobal && pcSymbol && (pcSymbol->meKind == CPPSK_Function) && mcNames.MemberScopeClass(pcSymbol))
	{
		//A member function of a class the function's class is nested in (or the function is a friend in): one of its
		//static members, with no object.  [class.nest] 1
		ResolveMember(pcCall, pcBound);
		return;
	}

	if (!pcSymbol && mpcUnit->GetBuiltins()->IsInlineMemcpy(lliName, pcBound->mpapcArguments))
	{
		pcBound->meHow = CPPBC_InlineMemcpy;
		return;
	}

	//A __builtin_ function is the builtin whether or not something declares it.
	if (pcName && (!pcSymbol || (pcSymbol->meKind == CPPSK_Function)) && mpcUnit->GetBuiltins()->IsBuiltin(lliName))
	{
		pcBound->meHow = CPPBC_Builtin;
		return;
	}

	if (!pcSymbol && mpcUnit->GetSemanticPass() && mpcUnit->GetBuiltins()->IsIntrinsic(lliName))
	{
		//Only checking: an intrinsic of the host's compiler (__debugbreak()), declared by no header.
		pcBound->meHow = CPPBC_Unchecked;
		return;
	}
	if (!pcSymbol && pcName && mcOverloads.UsesArgumentLookup(pcName))
	{
		//Nothing declares it here: it may be in a namespace of its arguments'.
		uiErrors = mpcDiagnostics->GetErrors()->NumErrors();
		pcFunction = mcOverloads.FindAssociatedFunction(pcName, lliName, pcCall, pcBound->mpapcArguments);
		if (pcFunction)
		{
			mpcUnit->GetCheck()->Resolved(pcName, pcFunction);
			pcBound->meHow = CPPBC_Function;
			pcBound->mpcFunction = pcFunction;
			return;
		}
		if (mpcDiagnostics->GetErrors()->NumErrors() != uiErrors)
		{
			return;
		}
	}
	if (!pcSymbol && pcComponent && (pcComponent->meKind == CPPNC_Operator))
	{
		//operator delete(p), operator+(a, b): an operator function called by its name, which no symbol has.  [over.oper] 4
		pcFunction = mcOverloads.FindFreeOperator(lliName, NULL, pcBound->mpapcArguments, pcCall);
		if (!pcFunction)
		{
			mpcDiagnostics->Error(pcCallee, "No operator function of the name takes these arguments");
			return;
		}
		pcBound->meHow = CPPBC_Function;
		pcBound->mpcFunction = pcFunction;
		return;
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
	//better match when it converts an argument better than the function does and none worse; else the function, which
	//isn't a template's, is.  [over.match.best] 1
	for (pcDeclaration = pcSymbol->mpcNext; pcDeclaration && (pcDeclaration->meKind != CPPSK_FunctionTemplate); pcDeclaration = pcDeclaration->mpcNext)
	{
	}
	if (pcDeclaration)
	{
		mpcDiagnostics->BeginSilentTrial();
		pcFunction = mcOverloads.FindNamedFunction(pcName, lliName, pcBound->mpapcArguments, pcCall);
		bInlineMemcpy = pcFunction && mpcUnit->GetBuiltins()->IsInlineMemcpy(lliName, pcBound->mpapcArguments);
		if (!bInlineMemcpy && (!pcFunction || !TakesAsTheyAre(pcFunction, pcBound->mpapcArguments)))
		{
			pcTemplate = mcOverloads.GetCallTemplate(pcSymbol, lliName, pcCall, pcBound->mpapcArguments, NULL);
			pcInstance = pcTemplate ? mpcUnit->GetInstantiator()->InstantiateFunction(pcTemplate, NULL, pcBound->mpapcArguments, pcCall) : NULL;
			if (pcInstance && (!pcFunction || mcOverloads.IsBetterFunction(pcInstance, pcFunction, pcBound->mpapcArguments)))
			{
				pcFunction = pcInstance;
			}
		}
		mpcDiagnostics->EndSilentTrial();
		if (pcFunction)
		{
			mpcUnit->GetCheck()->Resolved(pcName, pcFunction);
			if (bInlineMemcpy)
			{
				//A few bytes, a constant count: copied where it is called (as below), whichever memcpy a template of
				//the name would have made for the count's type (BaseLib's on the W65C816 choose memcpy32 by it).
				pcBound->meHow = CPPBC_InlineMemcpy;
				return;
			}
			pcBound->meHow = CPPBC_Function;
			pcBound->mpcFunction = pcFunction;
			return;
		}
	}

	uiErrors = mpcDiagnostics->GetErrors()->NumErrors();
	pcFunction = mcOverloads.FindNamedFunction(pcName, lliName, pcBound->mpapcArguments, pcCall);
	if (!pcFunction)
	{
		if (mpcDiagnostics->GetErrors()->NumErrors() == uiErrors)
		{
			mpcUnit->GetFunctionDeclarer()->ReportFailed(lliName);
			mpcDiagnostics->Error(pcCallee, "The function has not been declared");
		}
		return;
	}
	mpcUnit->GetCheck()->Resolved(pcName, pcFunction);

	//memcpy of a few bytes, a constant count, is copied where it is called: any function of the name, as the
	//code has always been written (the type checker asked for C linkage, and gave the same type either way).
	if (mpcUnit->GetBuiltins()->IsInlineMemcpy(lliName, pcBound->mpapcArguments))
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
	CCPPFunction*	pcSurrogate;
	size			uiErrors;
	bool			bAmbiguous;

	mpcDiagnostics->BeginQuiet();
	psType = mpcUnit->GetTypeChecker()->ValueTypeOf(pcBound->mpcCallee);
	mpcDiagnostics->EndQuiet();
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
		mpcUnit->GetClosures()->GenericLambdaCall(psType->pcClass->mpsLambda, pcBound->mpapcArguments);
	}

	pcBound->mlliName = mpcUnit->GetOperatorName(CT_OpenRoundBracket, false);
	uiErrors = mpcDiagnostics->GetErrors()->NumErrors();
	pcFunction = NULL;
	if (mcNames.HasMember(psType->pcClass, pcBound->mlliName))
	{
		mpcDiagnostics->BeginQuiet();
		pcFunction = mcOverloads.FindOperator(psType->pcClass, pcBound->mlliName, pcBound->mpapcArguments, pcCall, CalledObject(pcBound->mpcCallee, false));
		mpcDiagnostics->EndQuiet();
	}

	//Or a surrogate call function: through the pointer to a function the object converts to.  [over.call.object] 2
	pcSurrogate = mcOverloads.FindSurrogate(psType->pcClass, pcFunction, pcBound->mpapcArguments, &bAmbiguous);
	if (bAmbiguous)
	{
		mpcDiagnostics->Error(pcCall, "The call is ambiguous: the class's operator() and a function its conversion function points to take the arguments as well as each other");
		return true;
	}
	if (pcSurrogate)
	{
		pcBound->meHow = CPPBC_Surrogate;
		pcBound->mpcFunction = pcSurrogate;
		pcBound->mpcObject = pcBound->mpcCallee;
		pcBound->mpcClass = psType->pcClass;
		mcAccess.CheckFunction(pcSurrogate, psType->pcClass, pcCall);
		return true;
	}
	if (!pcFunction && mcNames.HasMember(psType->pcClass, pcBound->mlliName))
	{
		//Said again with its errors.
		pcFunction = mcOverloads.FindOperator(psType->pcClass, pcBound->mlliName, pcBound->mpapcArguments, pcCall, CalledObject(pcBound->mpcCallee, false));
	}
	if (!pcFunction)
	{
		if (mpcDiagnostics->GetErrors()->NumErrors() == uiErrors)
		{
			mpcDiagnostics->Error(pcCall, "The class has no operator() for these arguments");
		}
		return true;
	}
	pcBound->meHow = CPPBC_Member;
	pcBound->mpcFunction = pcFunction;
	pcBound->mpcObject = pcBound->mpcCallee;
	pcBound->mpcClass = psType->pcClass;
	pcBound->mbVirtual = !IsNamedObject(pcBound->mpcCallee);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// object.~T() or pointer->~T() of a psType that isn't a class, as a
// template writes for any T: it does nothing but work the object out.  T
// is the object's type, and nothing is in the brackets.  [expr.pseudo]
//
//////////////////////////////////////////////////////////////////////////
void CCPPBinder::ResolvePseudoDestructor(CCPPSyntaxMemberExpression* pcMember, SCPP65Type* psType, CCPPBoundCall* pcBound)
{
	CCPPSymbol*	pcNamed;
	SCPP65Type*	psNamed;

	pcNamed = pcMember->mpcName->GetLast()->mpcSymbol;
	psNamed = pcNamed ? mpcUnit->GetDeclarators()->GetSymbolType(pcNamed, pcMember) : NULL;
	if (psNamed && !psNamed->Unqualified()->Same(psType->Unqualified()))
	{
		mpcDiagnostics->Error(pcMember, "The destructor's name is not the type of the object");
		return;
	}
	if (pcBound->mpapcArguments->NumElements() != 0)
	{
		mpcDiagnostics->Error(pcMember, "A destructor takes no arguments");
		return;
	}
	pcBound->meHow = CPPBC_PseudoDestructor;
	pcBound->mpcName = pcMember->mpcName;
	pcBound->mpcObject = pcMember->mpcObject;
	pcBound->mbArrow = pcMember->mbArrow;
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
	CCPPClass*					pcThis;
	CCPPFunction*				pcFunction;
	CCPPFunction*				pcInstance;
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
	pcIn = mpcUnit->GetClosures()->OuterFunction();
	pcClass = NULL;
	pcObject = NULL;
	bArrow = false;
	bVirtual = true;
	if (pcCallee->GetKind() == CPPS_MemberExpression)
	{
		pcMember = (CCPPSyntaxMemberExpression*)pcCallee;
		psType = mpcUnit->GetTypeChecker()->TypeOf(pcMember->mpcObject);
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
		if (psType && psType->IsScalar() && (pcMember->mpcName->GetLast()->meKind == CPPNC_Destructor))
		{
			ResolvePseudoDestructor(pcMember, psType, pcBound);
			return;
		}
		if (!psType || !psType->IsClass())
		{
			mpcDiagnostics->Error(pcCallee, "Member functions need an object of a class");
			return;
		}
		pcClass = psType->pcClass;
		if (!mpcUnit->CompleteClass(pcClass, pcCallee))
		{
			return;
		}
		pcObject = pcMember->mpcObject;
		pcName = pcMember->mpcName;

		bVirtual = bArrow || !IsNamedObject(pcObject);
	}
	else
	{
		pcName = ((CCPPSyntaxIdExpression*)pcCallee)->mpcName;
		if (pcName->IsQualifiedByClass())
		{
			pcClass = mcNames.NameQualifierClass(pcName, pcCallee);
			bVirtual = false;
		}
		else
		{
			pcClass = (pcIn && pcIn->mpcClass) ? pcIn->mpcClass : mcNames.MemberScopeClass(pcName->mpcSymbol);
			if (pcClass && mcNames.MemberScopeClass(pcName->mpcSymbol) && !mcNames.HasMember(pcClass, mpcUnit->GetFunctionName(pcName->GetLast())))
			{
				//The enclosing class's member, which the function's own class doesn't have.
				pcClass = mcNames.MemberScopeClass(pcName->mpcSymbol);
				bVirtual = false;
			}
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
			mpcDiagnostics->Error(pcCallee, "This destructor call is not supported by the compiler yet");
			return;
		}
		if (pcClass->mpcDestructor)
		{
			mpcUnit->GetCheck()->Resolved(pcName, pcClass->mpcDestructor);
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
		mpcDiagnostics->Error(pcCallee, "This call is not supported by the compiler yet");
		return;
	}

	pcLookup = pcClass;
	if (pcObject && pcName->IsQualifiedByClass())
	{
		pcLookup = mcNames.QualifierClass(pcClass, pcName->GetQualifier(), pcCallee);
		if (!pcLookup)
		{
			return;
		}
		pcBound->mpcQualifier = pcLookup;
		bVirtual = false;
	}

	//object.pf() calls through a field that points to a function.
	if (pcObject && !mcNames.HasMember(pcLookup, lliName) && pcLookup->FindField(lliName, &iOffset))
	{
		psType = mpcUnit->GetTypeChecker()->TypeOf(pcCallee);
		if (psType && psType->IsPointer() && psType->psBase->IsFunction())
		{
			pcBound->meHow = CPPBC_Pointer;
			return;
		}
	}

	uiErrors = mpcDiagnostics->GetErrors()->NumErrors();
	pcComponent = pcName->GetLast();
	papcExplicit = pcComponent->mbTemplateArguments ? &pcComponent->macpTemplateArguments : NULL;
	eObject = CalledObject(pcObject, bArrow);
	if (mpcUnit->CompleteClass(pcLookup, pcCall) && !HasOneMeaning(pcLookup, lliName, pcCallee))
	{
		return;
	}
	pcTemplate = mpcUnit->CompleteClass(pcLookup, pcCall) ? mcOverloads.ChooseMemberTemplate(pcLookup, lliName, pcCall, pcBound->mpapcArguments, papcExplicit, eObject) : NULL;
	pcFunction = NULL;
	if (pcComponent->meKind == CPPNC_Conversion)
	{
		//s.operator double(): the conversion function to the type it names, not the class's others.
		pcFunction = FindConversion(pcLookup, pcComponent, pcCallee);
		pcTemplate = pcFunction ? NULL : pcTemplate;
	}
	if (!pcFunction && (!pcTemplate || !papcExplicit))
	{
		//With a member template to fall back on, that no function fits is no error.
		if (pcTemplate)
		{
			mpcDiagnostics->BeginQuiet();
		}
		pcFunction = mcOverloads.FindFunction(pcLookup, lliName, CPPFK_Function, pcBound->mpapcArguments, true, pcCall, eObject);
		if (pcTemplate)
		{
			mpcDiagnostics->EndQuiet();
		}
	}
	if (!pcFunction && pcTemplate)
	{
		pcFunction = mpcUnit->GetInstantiator()->InstantiateFunction(pcTemplate, papcExplicit, pcBound->mpapcArguments, pcCall);
	}
	else if (pcFunction && pcTemplate && !TakesAsTheyAre(pcFunction, pcBound->mpapcArguments))
	{
		//The member template's instance, if it takes the arguments better than the function that converts them, as a
		//function's is chosen (Resolve).  [over.match.best] 1
		mpcDiagnostics->BeginSilentTrial();
		pcInstance = mpcUnit->GetInstantiator()->InstantiateFunction(pcTemplate, papcExplicit, pcBound->mpapcArguments, pcCall);
		mpcDiagnostics->EndSilentTrial();
		if (pcInstance && mcOverloads.IsBetterFunction(pcInstance, pcFunction, pcBound->mpapcArguments))
		{
			pcFunction = pcInstance;
		}
	}
	if (!pcFunction)
	{
		if ((mpcDiagnostics->GetErrors()->NumErrors() == uiErrors) && !mpcUnit->GetDesugarings()->ImplicitAssignment(pcCall))
		{
			mpcDiagnostics->Error(pcCallee, "The class has no member function of that name");
		}
		return;
	}
	if (!pcObject && !pcFunction->mbStatic && (!pcIn || !pcIn->mpcClass || !mpcUnit->IsDerived(pcIn->mpcClass, pcFunction->mpcClass, NULL)))
	{
		mpcDiagnostics->Error(pcCallee, "A member function needs an object");
		return;
	}
	if (!pcObject && !pcFunction->mbStatic && pcIn && pcIn->mbStatic && !mpcUnit->GetClosures()->WritingLambda())
	{
		//A static member function has no this to call it on.  [over.call.func] 3, [class.static.mfct] 2
		mpcDiagnostics->Error(pcCallee, "A member function that isn't static is called with no object: a static member function has none");
		return;
	}
	pcThis = pcObject ? pcClass : mpcUnit->GetClosures()->ThisClass();
	if (pcThis && !pcFunction->mbStatic && !HasOneMember(pcThis, pcLookup, pcFunction->mpcClass, pcCallee))
	{
		return;
	}
	if (!pcObject && pcThis && (pcLookup != pcThis) && mpcUnit->IsDerived(pcThis, pcLookup, NULL))
	{
		//C::f() on this, in a member function of a class derived from C.
		pcBound->mpcQualifier = pcLookup;
	}
	mpcUnit->GetCheck()->Resolved(pcName, pcFunction);
	pcBound->meHow = CPPBC_Member;
	pcBound->mpcFunction = pcFunction;
	pcBound->mbVirtual = bVirtual;
}


//////////////////////////////////////////////////////////////////////////
//
// The conversion function of pcClass, or of a base, to the type a
// conversion-function-id names (pcComponent): its type's name is looked up
// in the class first, then where the call is.  NULL if there is none, or
// the type isn't known.  [basic.lookup.classref] 7, [class.conv.fct] 1
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPBinder::FindConversion(CCPPClass* pcClass, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode)
{
	CCPPSyntaxTypeId*			pcTypeId;
	CCPPSyntax*					pcSpecifier;
	CCPPSyntaxName*				pcName;
	CCPPSymbol*					pcSymbol;
	SCPP65Type*					psTarget;

	pcTypeId = (CCPPSyntaxTypeId*)pcComponent->mpcType;
	if (!pcTypeId || !pcTypeId->mpcSpecifiers)
	{
		return NULL;
	}
	pcSpecifier = pcTypeId->mpcSpecifiers->mpcType;
	pcName = (pcSpecifier && (pcSpecifier->GetKind() == CPPS_NamedTypeSpecifier)) ? ((CCPPSyntaxNamedTypeSpecifier*)pcSpecifier)->mpcName : NULL;
	if (pcName && pcName->IsSimple() && pcClass->mpcSymbol && pcClass->mpcSymbol->mpcMembers)
	{
		pcSymbol = pcClass->mpcSymbol->mpcMembers->FindMember(mpcUnit->GetComponentName(pcName->GetLast()), CPPL_Type);
		if (pcSymbol)
		{
			pcName->mpcSymbol = pcSymbol;
			pcName->GetLast()->mpcSymbol = pcSymbol;
		}
	}
	psTarget = mpcUnit->GetDeclarators()->GetTypeIdType(pcTypeId);
	if (!psTarget)
	{
		return NULL;
	}
	return FindConversionTo(pcClass, psTarget);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a value that isn't an object (pcSource) converts to pcClass by a
// constructor that isn't explicit: x = 6 assigns a temporary made so.
// [class.conv.ctor] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::ConvertsByConstructor(CCPPClass* pcClass, CCPPSyntax* pcSource)
{
	SCPP65Type*	psSource;

	psSource = mpcUnit->GetTypeChecker()->TypeOf(pcSource);
	psSource = psSource ? psSource->WithoutReference() : NULL;
	return psSource && !psSource->IsClass() && mcOverloads.HasConvertingConstructor(pcClass, psSource, pcSource);
}


//////////////////////////////////////////////////////////////////////////
//
// pcClass's conversion function to psTarget, or the nearest base's.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPBinder::FindConversionTo(CCPPClass* pcClass, SCPP65Type* psTarget)
{
	CCPPFunction*	pcFunction;
	size			i;

	for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
	{
		pcFunction = pcClass->mapcMembers.GetPtr(i);
		if (pcFunction->IsConversion() && pcFunction->mpsReturn && pcFunction->mpsReturn->Same(psTarget))
		{
			return pcFunction;
		}
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		pcFunction = FindConversionTo(pcClass->macBases.Get(i)->pcClass, psTarget);
		if (pcFunction)
		{
			return pcFunction;
		}
	}
	return NULL;
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
	CCPPFunction*	pcMember;
	CCPPFunction*	pcFunction;
	CArraySyntax	apcArguments;
	int				iBetter;

	psType = mpcUnit->GetTypeChecker()->ValueTypeOf(pcBound->mpcLeft);
	apcArguments.Init();
	pcMember = NULL;
	if (psType && psType->IsClass() && mpcUnit->CompleteClass(psType->pcClass, pcNode) && mcNames.HasOperator(psType->pcClass, pcBound->mlliName, pcBound->mpcRight ? 1 : 0))
	{
		if (pcBound->mpcRight)
		{
			apcArguments.Add(pcBound->mpcRight);
		}

		//That none of its own fits is no error: one that is no class's may.
		mpcDiagnostics->BeginSilent();
		pcMember = mcOverloads.FindOperator(psType->pcClass, pcBound->mlliName, &apcArguments, pcNode, CalledObject(pcBound->mpcLeft, false));
		mpcDiagnostics->EndSilent();
		apcArguments.ReInit();
	}

	//operator op(left, right), declared or from a function template; never operator=, a member only.  [over.ass] 1
	pcFunction = NULL;
	if (!pcMember || (pcBound->mlliName != mpcUnit->GetOperatorName(CT_Assignment, false)))
	{
		apcArguments.Add(pcBound->mpcLeft);
		if (pcBound->mpcRight)
		{
			apcArguments.Add(pcBound->mpcRight);
		}
		pcFunction = mcOverloads.FindFreeOperator(pcBound->mlliName, NULL, &apcArguments, pcNode);
	}
	apcArguments.Kill();

	//Both are candidates, and the better is chosen.  [over.match.oper] 3
	iBetter = (pcMember && pcFunction) ? mcOverloads.CompareOperators(pcMember, psType->pcClass, pcFunction, pcBound->mpcLeft, pcBound->mpcRight, CalledObject(pcBound->mpcLeft, false)) : (pcMember ? 1 : -1);
	if (pcMember && pcFunction && (iBetter == 0))
	{
		mpcDiagnostics->Error(pcNode, "The operator is ambiguous: the class's own operator function and one that is no class's take the operands as well as each other");
		return;
	}
	if (iBetter > 0)
	{
		pcBound->meHow = CPPBC_Member;
		pcBound->mpcFunction = pcMember;
		pcBound->mpcObject = pcBound->mpcLeft;
		pcBound->mpcClass = psType->pcClass;
		pcBound->mbVirtual = !IsNamedObject(pcBound->mpcLeft);
		return;
	}
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

	psType = mpcUnit->GetTypeChecker()->ValueTypeOf(pcBound->mpcLeft);
	if (!psType || !psType->IsClass())
	{
		mpcDiagnostics->Error(pcNode, "An operator call needs an object");
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
	uiErrors = mpcDiagnostics->GetErrors()->NumErrors();
	pcFunction = mcOverloads.FindOperator(psType->pcClass, pcBound->mlliName, &apcArguments, pcNode, CalledObject(pcBound->mpcLeft, false));
	apcArguments.Kill();
	if (!pcFunction)
	{
		if (mpcDiagnostics->GetErrors()->NumErrors() == uiErrors)
		{
			mpcDiagnostics->Error(pcNode, "The class has no operator for this");
		}
		return;
	}
	pcBound->meHow = CPPBC_Member;
	pcBound->mpcFunction = pcFunction;
	pcBound->mpcObject = pcBound->mpcLeft;
	pcBound->mpcClass = psType->pcClass;
	pcBound->mbVirtual = !IsNamedObject(pcBound->mpcLeft);
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
// The non-static member function &Class::name means (the one of psTarget's
// type if it has several, CCPPOverloads::FindMemberFunction) and the class
// it is in.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPBinder::FindMemberFunction(CCPPClass* pcClass, int64 lliName, SCPP65Type* psTarget, CCPPClass** ppcClass, bool* pbOverloaded)
{
	return mcOverloads.FindMemberFunction(pcClass, lliName, psTarget, false, ppcClass, pbOverloaded);
}


//////////////////////////////////////////////////////////////////////////
//
// The constructor a list in braces makes an object of pcClass with before
// any other ([over.match.list] 1): one whose first parameter is a
// std::initializer_list (or a const reference to one) that the list can
// make, the others defaulted.  NULL if there is none, or for {} of a class
// that has a default constructor ([dcl.init.list] 3).
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPBinder::InitializerListConstructor(CCPPClass* pcClass, CCPPSyntax* pcList)
{
	CCPPFunction*	pcFunction;
	SCPP65Type*		psParameter;
	size			i;

	if (mpcUnit->ListElements(pcList)->NumElements() == 0)
	{
		for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
		{
			pcFunction = pcClass->mapcMembers.GetPtr(i);
			if ((pcFunction->meKind == CPPFK_Constructor) && pcFunction->TakesArgumentCount(0))
			{
				return NULL;
			}
		}
	}
	for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
	{
		pcFunction = pcClass->mapcMembers.GetPtr(i);
		if ((pcFunction->meKind != CPPFK_Constructor) || pcFunction->mbDeleted || pcFunction->mpcTemplate || (pcFunction->mapsParameters.NumElements() == 0) || !pcFunction->TakesArgumentCount(1))
		{
			continue;
		}
		psParameter = pcFunction->mapsParameters.GetPtr(0);
		if (psParameter->IsReference() && !psParameter->psBase->bConst)
		{
			continue;
		}
		if (mpcUnit->GetDesugarings()->InitializerListElement(psParameter) && ListConverts(mpcUnit->GetDesugarings()->InitializerListElement(psParameter), pcList))
		{
			return pcFunction;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an operator function of the name that is no class's is declared,
// or a function template of it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::HasFreeOperator(int64 lliName)
{
	return mcOverloads.HasFreeOperator(lliName);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether each of a braced list's elements converts to psElement, as an
// initializer_list<psElement>'s: an implicit conversion, though it may
// narrow, which is an error once the constructor is chosen.
// [over.match.list] 1, [over.ics.list] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::ListConverts(SCPP65Type* psElement, CCPPSyntax* pcList)
{
	CArraySyntax*	papcElements;
	CCPPSyntax*		pcValue;
	SCPP65Type*		psValue;
	size			i;

	papcElements = mpcUnit->ListElements(pcList);
	for (i = 0; i < papcElements->NumElements(); i++)
	{
		pcValue = papcElements->GetPtr(i);
		psValue = (pcValue->GetKind() == CPPS_InitializerList) ? NULL : mpcUnit->GetTypeChecker()->TypeOf(pcValue);
		if (psValue ? (mcOverloads.ConversionRank(psElement, psValue, pcValue) == CPPCR_None) : !mpcUnit->GetTypeChecker()->CanInitialise(psElement, pcValue))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The literal operator of lliName (CPP_LITERAL_NAME) that takes
// papcArguments, or the instance of one that is a template for
// papcExplicit; NULL, quietly, if there is none.  [over.literal]
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPBinder::FindLiteralOperator(int64 lliName, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	return mcOverloads.FindFreeOperator(lliName, papcExplicit, papcArguments, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// The allocation function a new expression of psType (an array's element)
// calls with papcArguments, its size (CCPPUnit::NewSize) then its
// placement arguments: the class's own operator new or new[] (or a
// base's), unless it is ::new, or one of the program's global ones that
// the placement arguments choose.  NULL for the runtime's operator new,
// or for new (address) Type with no operator new of the program's that
// takes a pointer, the address itself; *pbFailed when no operator new
// takes placement arguments.  [expr.new] 9
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPBinder::FindAllocation(CCPPSyntaxNewExpression* pcNew, SCPP65Type* psType, CArraySyntax* papcArguments, bool* pbFailed)
{
	CCPPFunction*	pcFunction;
	CCPPClass*		pcClass;
	int64			lliName;

	*pbFailed = false;
	lliName = mpcUnit->GetOperatorName(CT_new, mpcUnit->IsArrayNew(pcNew));
	pcClass = (psType->IsClass() && !pcNew->mbGlobal) ? psType->pcClass : NULL;
	if (pcClass && mcNames.HasMember(pcClass, lliName))
	{
		pcFunction = mcOverloads.FindFunction(pcClass, lliName, CPPFK_Function, papcArguments, true, pcNew, CPPCO_None);
		*pbFailed = !pcFunction;
		return pcFunction;
	}
	if (papcArguments->NumElements() == 1)
	{
		return NULL;
	}

	mpcDiagnostics->BeginSilentTrial();
	pcFunction = mcOverloads.FindFunction(NULL, lliName, CPPFK_Function, papcArguments, false, pcNew, CPPCO_None);
	mpcDiagnostics->EndSilentTrial();
	if (pcFunction && IsReservedPlacement(pcFunction))
	{
		//operator new(size_t, void*) is the library's, which returns the address, whoever declares it.
		pcFunction = NULL;
	}
	*pbFailed = !pcFunction && !IsBuiltinPlacement(papcArguments);
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a global operator new or new[] is the placement form the library
// reserves, which takes a void* and returns it: a program's own declaration
// of it (<new>'s, say) is the library's.  [new.delete.placement] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::IsReservedPlacement(CCPPFunction* pcFunction)
{
	SCPP65Type*	psAddress;

	if (pcFunction->mpcClass || (pcFunction->mapsParameters.NumElements() != 2))
	{
		return false;
	}
	psAddress = pcFunction->mapsParameters.GetPtr(1)->Unqualified();
	return psAddress->IsPointer() && psAddress->psBase->IsVoid();
}


//////////////////////////////////////////////////////////////////////////
//
// new (address) Type with nothing of the program's for it: the object is
// made at the address.  [new.delete.placement] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::IsBuiltinPlacement(CArraySyntax* papcArguments)
{
	SCPP65Type*	psAddress;

	if (papcArguments->NumElements() != 2)
	{
		return false;
	}
	psAddress = mpcUnit->GetTypeChecker()->TypeOf(papcArguments->GetPtr(1));
	return psAddress && psAddress->WithoutReference()->IsPointer();
}


//////////////////////////////////////////////////////////////////////////
//
// The deallocation function delete (or delete[], bArray) of an object of
// pcClass calls (NULL for what is no class's, and for ::delete), or NULL
// for the one that takes the pointer alone and is no class's, which is the
// runtime's unless the program defines it: the operator delete of the
// class, or of the nearest base that declares one, that takes the pointer
// alone, or failing that the pointer and a size_t; and for a class that
// has none, where the size freed is known (bSized), the global one that
// takes the size, if the program has declared it (<new> does), which is
// chosen before the one that doesn't.  [expr.delete] 9, 10,
// [basic.stc.dynamic.deallocation] 2
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPBinder::FindDeallocation(CCPPClass* pcClass, bool bArray, bool bSized)
{
	CCPPFunction*	pcFunction;
	CCPPFunction*	pcFound;
	int64			lliName;
	bool			bMember;

	lliName = mpcUnit->GetOperatorName(CT_delete, bArray);
	bMember = pcClass && mcNames.HasMember(pcClass, lliName);
	pcFound = NULL;
	for (pcFunction = mpcUnit->FirstFunctionOfName(lliName); (bMember || bSized) && pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if (!IsUsualDeallocation(pcFunction))
		{
			continue;
		}
		if (bMember ? (pcFunction->mpcClass && mpcUnit->IsDerived(pcClass, pcFunction->mpcClass, NULL) && IsNearerDeallocation(pcFunction, pcFound)) : IsGlobalSizedDeallocation(pcFunction))
		{
			pcFound = pcFunction;
		}
	}
	return pcFound;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a constructor copies pcSource, an object of pcClass or of a class
// derived from it, into a new one: the class's own copy or move
// constructor, or a constructor template's instance that takes the object
// better than the copy constructor the compiler declares would
// (CCPPOverloads::InstanceCopies).  Otherwise the compiler copies it, part
// by part or as its bytes.  [class.copy] 7, [over.match.best] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::CopiesByConstructor(CCPPClass* pcClass, CCPPSyntax* pcSource)
{
	return pcClass->HasCopyConstructor() || mcOverloads.InstanceCopies(pcClass, pcSource);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether new[] of elements of psElement keeps their count in front of
// them, which delete[] reads: objects that have a destructor (each is
// destroyed), and objects whose class's operator delete[] takes the size
// (only the count says it).  Every unit decides the same, from the class
// alone: the global operator delete[] that takes a size isn't asked, as a
// unit may not have its declaration.  [expr.new] 11, [expr.delete] 10
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::KeepsArrayCount(SCPP65Type* psElement)
{
	CCPPFunction*	pcDeallocation;

	if (!psElement->IsClass())
	{
		return false;
	}
	if (psElement->pcClass->mpcDestructor)
	{
		return true;
	}
	pcDeallocation = FindDeallocation(psElement->pcClass, true);
	return pcDeallocation && (pcDeallocation->mapsParameters.NumElements() == 2);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an operator delete is one a delete expression calls: it takes
// the pointer, or the pointer and the size as a size_t.
// [basic.stc.dynamic.deallocation] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::IsUsualDeallocation(CCPPFunction* pcFunction)
{
	size	uiParameters;

	uiParameters = pcFunction->mapsParameters.NumElements();
	return (uiParameters == 1) || ((uiParameters == 2) && pcFunction->mapsParameters.GetPtr(1)->Unqualified()->Equals(mpcUnit->GetTypes()->Size()));
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a usual deallocation function is the global one that takes the
// size: void operator delete(void*, std::size_t).  [new.delete.single] 16
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::IsGlobalSizedDeallocation(CCPPFunction* pcFunction)
{
	return !pcFunction->mpcClass && !pcFunction->mpcNamespace && !pcFunction->mpcTemplate && (pcFunction->mapsParameters.NumElements() == 2);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a class's operator delete, pcFunction, is chosen before pcFound
// (NULL for none yet): it is a more derived class's, which hides a base's,
// or it is the same class's and takes the pointer alone.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::IsNearerDeallocation(CCPPFunction* pcFunction, CCPPFunction* pcFound)
{
	if (!pcFound)
	{
		return true;
	}
	if (pcFunction->mpcClass == pcFound->mpcClass)
	{
		return pcFunction->mapsParameters.NumElements() == 1;
	}
	return mpcUnit->IsDerived(pcFunction->mpcClass, pcFound->mpcClass, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// The conversion function an object of pcClass is copy initialised from
// pcSource by: when pcSource is an object of another class (not derived
// from pcClass), its operator T whose T is pcClass (or derived from it, or
// a reference to one), unless one of pcClass's converting constructors
// takes the object as well or better.  The object is the function's this,
// bound as T& (or const T& for a const function: worse for an object that
// isn't const, which T& takes).  NULL with an error if the two are as good.
// [over.match.copy] 1, [over.ics.rank] 3
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPBinder::ConversionToClass(CCPPClass* pcClass, CCPPSyntax* pcSource, CCPPSyntax* pcNode)
{
	SCPP65Type*		psSource;
	SCPP65Type*		psResult;
	CCPPFunction*	pcFunction;
	int				iConstructor;
	int				iConversion;

	psSource = mpcUnit->GetTypeChecker()->TypeOf(pcSource);
	psSource = psSource ? psSource->WithoutReference() : NULL;
	if (!psSource || !psSource->IsClass() || mpcUnit->IsDerived(psSource->pcClass, pcClass, NULL))
	{
		return NULL;
	}
	iConstructor = mcOverloads.ConvertingConstructorRank(pcClass, psSource, pcSource);
	if (iConstructor == CPPCR_Exact)
	{
		return NULL;
	}
	pcFunction = mcOverloads.FindConversion(psSource->pcClass, pcClass->mpsType, false, pcNode);
	psResult = pcFunction ? pcFunction->mpsReturn->WithoutReference() : NULL;
	if (!psResult || !psResult->IsClass() || !mpcUnit->IsDerived(psResult->pcClass, pcClass, NULL))
	{
		return NULL;
	}
	if (iConstructor == CPPCR_None)
	{
		return pcFunction;
	}
	if (!pcFunction->mbConstThis && psSource->bConst)
	{
		return NULL;
	}
	iConversion = (pcFunction->mbConstThis && !psSource->bConst) ? CPPCR_Qualification : CPPCR_Exact;
	if (iConversion == iConstructor)
	{
		mpcDiagnostics->Error(pcNode, "The conversion is ambiguous: a constructor of the class and a conversion function of the value's take it as well as each other");
		return NULL;
	}
	return (iConversion > iConstructor) ? pcFunction : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The class's operator-> for the object (NULL for an object an operator->
// returned), or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPBinder::FindArrowOperator(CCPPClass* pcClass, CCPPSyntax* pcObject, CCPPSyntax* pcNode)
{
	CArraySyntax	apcNone;
	CCPPFunction*	pcFunction;

	apcNone.Init();
	pcFunction = mcOverloads.FindFunction(pcClass, mpcUnit->GetOperatorName(CT_MemberByPointer, false), CPPFK_Function, &apcNone, true, pcNode, pcObject ? CalledObject(pcObject, false) : CPPCalledObject(false));
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
// Whether the class itself (not a base) has a member template of the name
// that takes uiArguments arguments.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::HasOwnMemberTemplate(CCPPClass* pcClass, int64 lliName, size uiArguments)
{
	CCPPTemplate*	pcTemplate;

	pcTemplate = mcOverloads.FindMemberTemplate(pcClass, lliName, uiArguments);
	return pcTemplate && (pcTemplate->mpcClass == pcClass);
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
		psArgument = mpcUnit->GetTypeChecker()->ValueTypeOf(papcArguments->GetPtr(i));
		if (!psArgument || !pcFunction->mapsParameters.GetPtr(i)->WithoutReference()->Unqualified()->Equals(psArgument->Unqualified()))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if a value of psFrom converts to psTo implicitly: the type traits'
// question, of types with no expression.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::IsConvertible(SCPP65Type* psTo, SCPP65Type* psFrom)
{
	return mcOverloads.ConversionRank(psTo, psFrom, NULL, true) != CPPCR_None;
}


//////////////////////////////////////////////////////////////////////////
//
// True if an object of the type can be made from values of the argument
// types: one of its class's constructors (not deleted) takes them, or with
// no constructors none or one of its own class; a value that isn't an
// object from none or one that converts.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::IsConstructible(SCPP65Type* psType, CArrayCPP65Types* papsArguments)
{
	CCPPClass*		pcClass;
	CCPPFunction*	pcFunction;
	size			i;
	size			j;
	bool			bTakes;

	if (!psType->IsClass())
	{
		return (papsArguments->NumElements() == 0) || ((papsArguments->NumElements() == 1) && IsConvertible(psType, papsArguments->GetPtr(0)));
	}
	pcClass = psType->pcClass;
	if (!pcClass->mbConstructors)
	{
		if (papsArguments->NumElements() == 0)
		{
			return true;
		}
		return (papsArguments->NumElements() == 1) && papsArguments->GetPtr(0)->WithoutReference()->IsClass() && mpcUnit->IsDerived(papsArguments->GetPtr(0)->WithoutReference()->pcClass, pcClass, NULL) && !pcClass->IsCopyDeleted();
	}
	for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
	{
		pcFunction = pcClass->mapcMembers.GetPtr(i);
		if ((pcFunction->meKind != CPPFK_Constructor) || pcFunction->mbDeleted || (pcFunction->mapsParameters.NumElements() < papsArguments->NumElements()) || (pcFunction->RequiredParameters() > papsArguments->NumElements()))
		{
			continue;
		}
		bTakes = true;
		for (j = 0; bTakes && (j < papsArguments->NumElements()); j++)
		{
			bTakes = mcOverloads.ConversionRank(pcFunction->mapsParameters.GetPtr(j), papsArguments->GetPtr(j), NULL, true) != CPPCR_None;
		}
		if (bTakes)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if a value of psFrom can be assigned to psTo (a reference to what
// is assigned): by an operator= of its class, the one the compiler writes,
// or as a value converts.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::IsAssignable(SCPP65Type* psTo, SCPP65Type* psFrom)
{
	SCPP65Type*		psObject;
	CCPPClass*		pcClass;
	CCPPFunction*	pcFunction;
	size			i;
	bool			bOwn;

	if (!psTo->IsReference())
	{
		//What has no name can't be assigned to (but an object, whose operator= may).
		if (!psTo->IsClass())
		{
			return false;
		}
		psObject = psTo;
	}
	else
	{
		psObject = psTo->psBase;
	}
	if (psObject->bConst)
	{
		return false;
	}
	if (!psObject->IsClass())
	{
		return IsConvertible(psObject, psFrom);
	}
	pcClass = psObject->pcClass;
	bOwn = false;
	for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
	{
		pcFunction = pcClass->mapcMembers.GetPtr(i);
		if ((pcFunction->mlliName != mpcUnit->GetOperatorName(CT_Assignment, false)) || (pcFunction->mapsParameters.NumElements() != 1))
		{
			continue;
		}
		bOwn = true;
		if (!pcFunction->mbDeleted && (mcOverloads.ConversionRank(pcFunction->mapsParameters.GetPtr(0), psFrom, NULL, true) != CPPCR_None))
		{
			return true;
		}
	}
	return !bOwn && !pcClass->IsAssignmentDeleted() && psFrom->WithoutReference()->IsClass() && mpcUnit->IsDerived(psFrom->WithoutReference()->pcClass, pcClass, NULL);
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

	apcArguments.Init();
	if (pcClass)
	{
		mpcDiagnostics->BeginSilent();
		pcFunction = mcOverloads.FindFunction(pcClass, lliName, CPPFK_Function, &apcArguments, true, pcNode, CalledObject(pcRange, false));
		mpcDiagnostics->EndSilent();
	}
	else
	{
		apcArguments.Add(pcRange);
		pcFunction = mcOverloads.FindFreeOperator(lliName, NULL, &apcArguments, pcNode);
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
// True if the object a member function is called on can only be of the
// class it is declared as, so that its virtual functions are called by
// their labels: an object that is named.  A name that is a reference is
// not one: what it refers to may be of a derived class, as what a pointer
// points to may, and its virtual functions are called through its vtable.
// [expr.call] 1, [class.virtual] 7
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::IsNamedObject(CCPPSyntax* pcObject)
{
	CCPPBoundName	cBound;

	pcObject = pcObject->WithoutParentheses();
	if (pcObject->GetKind() != CPPS_IdExpression)
	{
		return false;
	}

	ResolveName((CCPPSyntaxIdExpression*)pcObject, &cBound);
	switch (cBound.meWhat)
	{
	case CPPBN_Variable:
		return !cBound.mpcVariable->mpsType->IsReference();
	case CPPBN_Field:
		return !cBound.mpsField->psType->IsReference();
	case CPPBN_Capture:
		return !cBound.mpsCapture->bReference && !cBound.mpsField->psType->IsReference();
	default:
		return false;
	}
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
	SCPP65Type*	psType;

	if (!pcObject)
	{
		return CPPCalledObject(mpcUnit->IsConstThis(), mpcUnit->IsVolatileThis());
	}
	psType = mpcUnit->GetTypeChecker()->ValueTypeOf(pcObject);
	if (bArrow)
	{
		psType = (psType && psType->IsPointer()) ? psType->psBase : NULL;
	}
	return CPPCalledObject(psType && psType->bConst, psType && psType->bVolatile, !bArrow && mpcUnit->GetTypeChecker()->IsRvalue(pcObject));
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
	if ((pcBound->meWhat != CPPBN_Nothing) && (pcBound->meWhat != CPPBN_FunctionName) && (pcBound->meWhat != CPPBN_Field))
	{
		//A static member or an enumerator of a base, by its name alone.  (A field's is HasOneMeaning's.)
		if (!mcNames.CheckBaseMemberName(pcId->mpcName, pcId))
		{
			pcBound->meWhat = CPPBN_Nothing;
		}
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
	CCPPClass*		pcLookup;
	SCPPBinding*	psBinding;
	SCPPLambda*		psLambda;

	pcBound->meWhat = CPPBN_Nothing;
	pcBound->mpcVariable = NULL;
	pcBound->mpcClass = NULL;
	pcBound->mpsField = NULL;
	pcBound->miOffset = 0;
	pcBound->mpcQualifier = NULL;
	pcBound->mpcVirtualBase = NULL;
	pcBound->mpsCapture = NULL;
	pcBound->mpcArgument = NULL;
	pcSymbol = pcId->mpcName->mpcSymbol;
	pcBound->mpcSymbol = pcSymbol;

	//In a lambda's body, what it captures is its closure's.
	pcBound->mpsCapture = (pcId->mpcName->macpComponents.NumElements() == 1) ? mpcUnit->GetClosures()->FindCapture(pcSymbol) : NULL;
	if (pcBound->mpsCapture)
	{
		pcBound->meWhat = CPPBN_Capture;
		pcBound->mpcClass = mpcUnit->GetClosures()->WritingLambda()->pcClass;
		pcBound->mpsField = pcBound->mpcClass->macFields.Get(pcBound->mpsCapture->uiField);
		pcBound->miOffset = pcBound->mpsField->iOffset;
		return;
	}

	//A constant of the function it is in that it doesn't capture is its value (its object is the type checker's error).
	psLambda = (pcSymbol && (pcId->mpcName->macpComponents.NumElements() == 1)) ? mpcUnit->GetClosures()->WritingLambda() : NULL;
	if (psLambda && mpcUnit->GetClosures()->IsEnclosingConstant(pcSymbol, psLambda))
	{
		pcBound->meWhat = CPPBN_EnclosingConstant;
		return;
	}
	if (!psLambda && pcSymbol && (pcId->mpcName->macpComponents.NumElements() == 1) && mpcUnit->GetClosures()->IsOuterConstant(pcSymbol))
	{
		//A local class's member function, and a constant of the function the class is in.  [class.local] 1
		pcBound->meWhat = CPPBN_EnclosingConstant;
		return;
	}
	if (mpcUnit->IsFunctionName(pcId))
	{
		pcBound->meWhat = CPPBN_FunctionName;
		return;
	}

	//A pointer or reference parameter of the template being generated is its argument: P is &g, R is g.
	psBinding = (pcSymbol && (pcSymbol->meKind == CPPSK_ValueParameter)) ? mpcUnit->GetInstantiator()->FindBinding(pcSymbol) : NULL;
	if (psBinding && psBinding->pcArgument)
	{
		pcBound->meWhat = CPPBN_TemplateArgument;
		pcBound->mpcArgument = psBinding->pcArgument;
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
			mcNames.QualifiedMember(pcId->mpcName, CPPSK_Field, pcId, &pcMemberClass);
		}
		else if (pcSymbol)
		{
			pcIn = mpcUnit->GetClosures()->OuterFunction();
			pcMemberClass = (pcIn && pcIn->mpcClass && pcIn->mpcClass->mpcTemplate) ? pcIn->mpcClass : NULL;
		}
		if (pcMemberClass)
		{
			pcBound->mpcVariable = mpcUnit->FindStatic(pcMemberClass, mpcUnit->GetComponentName(pcId->mpcName->GetLast()));
		}
	}

	//A local, a parameter, a global or a static member; pi<int> a variable template's instance.
	if (!pcBound->mpcVariable && pcSymbol && (pcSymbol->meKind == CPPSK_VariableTemplate))
	{
		pcBound->mpcVariable = mpcUnit->GetInstantiator()->VariableInstance(pcId->mpcName, pcId);
	}
	if (!pcBound->mpcVariable && pcSymbol)
	{
		pcBound->mpcVariable = mpcUnit->GetVariable(pcSymbol);
	}
	if (!pcBound->mpcVariable && pcSymbol && (pcSymbol->meKind == CPPSK_Variable))
	{
		//A static local of the function a local class is in, named in one of the class's member functions.
		pcBound->mpcVariable = mpcUnit->GetClosures()->EnclosingStatic(pcSymbol);
	}
	if (pcBound->mpcVariable)
	{
		pcBound->meWhat = CPPBN_Variable;
		return;
	}

	if (!pcSymbol)
	{
		pcSymbol = mcNames.QualifiedEnumerator(pcId->mpcName, pcId, &pcEnumeratorClass);
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

	//A field of this, of the base a qualifier names (B::a) if it does; a name in a template that the parser could not
	//resolve may be one too.  In a static member function it is the class's field still, which has no object there:
	//only what doesn't evaluate it (sizeof, decltype) can name it.  [expr.prim.general] 13
	pcClass = mpcUnit->GetClosures()->ThisClass();
	pcIn = mpcUnit->GetClosures()->OuterFunction();
	if (!pcClass && pcIn && pcIn->mbStatic)
	{
		pcClass = pcIn->mpcClass;
	}
	if (pcSymbol && (pcSymbol->meKind == CPPSK_Field) && pcClass)
	{
		pcLookup = pcId->mpcName->IsQualifiedByClass() ? mcNames.GetComponentClass(pcId->mpcName->GetQualifier(), NULL, pcId, pcClass) : NULL;
		if (pcLookup && (pcLookup != pcClass) && mpcUnit->IsDerived(pcClass, pcLookup, NULL))
		{
			pcBound->mpcQualifier = pcLookup;
		}
		pcBound->mpsField = (pcBound->mpcQualifier ? pcBound->mpcQualifier : pcClass)->FindFieldSymbol(pcSymbol, &pcBound->miOffset, &pcBound->mpcVirtualBase);
	}
	else if (!pcSymbol && pcClass && (pcId->mpcName->macpComponents.NumElements() == 1))
	{
		pcBound->mpsField = pcClass->FindField(mpcUnit->GetComponentName(pcId->mpcName->GetLast()), &pcBound->miOffset, &pcBound->mpcVirtualBase);
		if (pcBound->mpsField && InDependentBase(pcClass, pcClass->FindFieldClass(pcBound->mpsField)))
		{
			//A name that isn't qualified isn't looked for in a base that depends on a template parameter.  [temp.dep] 3
			mpcDiagnostics->ReportOnce(pcId, "The name is declared only in a base class that depends on a template parameter: it is named this->name there");
			pcBound->mpsField = NULL;
		}
	}
	if (pcBound->mpsField)
	{
		pcBound->meWhat = CPPBN_Field;
		pcBound->mpcClass = pcClass;
		pcLookup = pcBound->mpcQualifier ? pcBound->mpcQualifier : pcClass;
		HasOneMember(pcClass, pcLookup, pcLookup->FindFieldClass(pcBound->mpsField), pcId);
		HasOneMeaning(pcLookup, pcBound->mpsField->pcSymbol->mlliName, pcId);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcDeclaring, a base of the class template instance pcClass, is
// reached through one of its bases that its template names with its
// parameters (CCPPScope::IsDependentBase).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::InDependentBase(CCPPClass* pcClass, CCPPClass* pcDeclaring)
{
	CCPPScope*	pcScope;
	CCPPScope*	pcBaseScope;
	CCPPClass*	pcBase;
	size		i;
	size		j;

	pcScope = (pcClass->mpcTemplate && pcClass->mpcSymbol) ? pcClass->mpcSymbol->mpcMembers : NULL;
	if (!pcScope || !pcDeclaring || (pcDeclaring == pcClass) || (pcScope->muiDependentBases == 0))
	{
		return false;
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		pcBase = pcClass->macBases.Get(i)->pcClass;
		if ((pcBase != pcDeclaring) && !mpcUnit->IsDerived(pcBase, pcDeclaring, NULL))
		{
			continue;
		}
		pcBaseScope = pcBase->mpcSymbol ? pcBase->mpcSymbol->mpcMembers : NULL;
		for (j = 0; j < pcScope->mapcBases.NumElements(); j++)
		{
			if (pcScope->mapcBases.GetPtr(j) == pcBaseScope)
			{
				return pcScope->IsDependentBase(j);
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// object.name and pointer->name, once the object's class is known: the class's field
// of the name, or a base's, with where it is in the object (CPPBN_Field), or else its
// static member, which a member variable template's instance is (CPPBN_Variable).
// False if it has neither (CPPBN_Nothing), if two of its bases each have the name
// (HasOneMeaning), or the class can't be completed, which is reported at pcNode.  In the class's own
// definition it needn't be complete: a member function's trailing return type names
// the members declared before it (auto Get(void) -> decltype(this->m)), which haven't
// been placed yet.  [expr.ref] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::BindMember(CCPPClass* pcClass, CCPPSyntaxName* pcName, CCPPSyntax* pcNode, CCPPBoundName* pcBound)
{
	int64		lliName;
	CCPPClass*	pcLookup;
	CCPPClass*	pcDeclaring;
	CCPPSymbol*	pcSymbol;

	pcBound->meWhat = CPPBN_Nothing;
	pcBound->mpcSymbol = NULL;
	pcBound->mpcVariable = NULL;
	pcBound->mpcClass = pcClass;
	pcBound->mpsField = NULL;
	pcBound->miOffset = 0;
	pcBound->mpcQualifier = NULL;
	pcBound->mpcVirtualBase = NULL;
	if (!pcClass || !(pcClass->IsLayingOut() || mpcUnit->CompleteClass(pcClass, pcNode)))
	{
		return false;
	}
	if (!mpcUnit->IsDefinedBefore(pcClass, pcNode))
	{
		//[expr.ref] 2, [basic.types] 5
		return mpcDiagnostics->ReportOnce(pcNode, "The class is incomplete here: it is defined further on");
	}
	lliName = mpcUnit->GetComponentName(pcName->GetLast());
	pcLookup = pcClass;
	if (pcName->IsQualifiedByClass())
	{
		pcLookup = mcNames.QualifierClass(pcClass, pcName->GetQualifier(), pcNode);
		if (!pcLookup)
		{
			return false;
		}
		pcBound->mpcQualifier = (pcLookup != pcClass) ? pcLookup : NULL;
	}
	if (!HasOneMeaning(pcLookup, lliName, pcNode))
	{
		return false;
	}
	pcBound->mpsField = pcLookup->FindField(lliName, &pcBound->miOffset, &pcBound->mpcVirtualBase);
	if (pcBound->mpsField)
	{
		pcBound->meWhat = CPPBN_Field;
		pcBound->mpcSymbol = pcBound->mpsField->pcSymbol;
		HasOneMember(pcClass, pcLookup, pcLookup->FindFieldClass(pcBound->mpsField), pcNode);
	}
	else
	{
		//object.size<char> is its class's member variable template's instance for the arguments (each a static member
		//of the name).  [temp.variable] 1
		pcSymbol = pcName->GetLast()->mbTemplateArguments ? pcLookup->FindMemberSymbol(lliName, CPPSK_VariableTemplate, &pcDeclaring) : NULL;
		pcBound->mpcVariable = pcSymbol ? mpcUnit->GetInstantiator()->ClassVariableInstance(pcSymbol, pcName->GetLast(), pcLookup, pcNode) : mpcUnit->FindStatic(pcClass, lliName);
		if (!pcBound->mpcVariable)
		{
			return false;
		}
		pcBound->meWhat = CPPBN_Variable;
		pcBound->mpcSymbol = pcBound->mpcVariable->mpcSymbol;
	}

	//It is the member whether or not it may be named here: that it may not is an error of its own.
	if (mcAccess.CheckSymbol(pcBound->mpcSymbol, pcClass, pcNode))
	{
		mcAccess.CheckProtectedObject(pcBound->mpcSymbol, pcClass, pcNode);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an object of pcClass has one pcBase to be (the class itself, or a
// base it has once): an error (and false) for a base it has more than once,
// which a pcClass doesn't become, nor is a member of it the class's without
// saying whose.  [class.mi] 4, [class.member.lookup]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::HasOneBase(CCPPClass* pcClass, CCPPClass* pcBase, CCPPSyntax* pcNode)
{
	return (mpcUnit->CountBases(pcClass, pcBase) <= 1) || mpcDiagnostics->ReportOnce(pcNode, "The base class is in the class more than once: say which is meant with a qualified name or a cast to a class between");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a member that pcDeclaring declares (a field or a member function
// that isn't static; NULL when it isn't known), found in pcLookup, is one
// member of an object of pcClass: the object has one pcLookup (itself, or
// the base that qualifies the member's name) and that one pcDeclaring.  An
// error (and false) if not.  [class.member.lookup], [expr.ref] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::HasOneMember(CCPPClass* pcClass, CCPPClass* pcLookup, CCPPClass* pcDeclaring, CCPPSyntax* pcNode)
{
	return HasOneBase(pcClass, pcLookup, pcNode) && (!pcDeclaring || HasOneBase(pcLookup, pcDeclaring, pcNode));
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a member's name, looked up in the class, means one thing: an
// error (and false) if two of its bases each declare it and neither's is
// hidden (CCPPUnit::IsAmbiguousInBases), whatever each declares it as and
// whether or not one would fit better.  [class.member.lookup] 6
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::HasOneMeaning(CCPPClass* pcClass, int64 lliName, CCPPSyntax* pcNode)
{
	return !mcNames.IsAmbiguousInBases(pcClass, lliName) || mpcDiagnostics->ReportOnce(pcNode, "The name is a member of more than one of the class's bases");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a pcClass becomes its base pcBase at pcNode, or the base the
// class, as a pointer, a reference or a pointer to a member does: the base
// is there once and may be reached.  An error (and false) if not.
// [conv.ptr] 3, [conv.mem] 2, [expr.static.cast] 2, 11, 12
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::ReachesBase(CCPPClass* pcClass, CCPPClass* pcBase, CCPPSyntax* pcNode)
{
	return HasOneBase(pcClass, pcBase, pcNode) && mcAccess.CheckBase(pcClass, pcBase, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an object of pcClass is made from arguments that aren't a list in
// braces: by a constructor, or from none.  A class with no constructor is
// an aggregate, whose members only a list in braces gives values: S(1, 2),
// S s = 1 and (S)1 are errors (and false) where S{1, 2} isn't.
// [dcl.init] 17, [dcl.init.aggr]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::ConstructsWithoutBraces(CCPPClass* pcClass, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CCPPBoundConstruction	cBound;

	if ((papcArguments->NumElements() == 0) || !BindConstruction(pcClass, papcArguments, pcNode, &cBound, false))
	{
		//(What doesn't bind is reported where it is made.)
		return true;
	}
	return (cBound.meHow != CPPBO_Aggregate) || mpcDiagnostics->ReportOnce(pcNode, "The class has no constructor for this: its members are given values by a list in braces");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the class's copy constructor is deleted: the one it declares is,
// or it declares none but declares a move (which deletes the one the
// compiler would declare), or a base or a field can't be copied (an rvalue
// reference, or an object whose copy constructor is deleted).
// [class.copy] 7, 11
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::IsCopyDeleted(CCPPClass* pcClass)
{
	CCPPFunction*	pcCopy;
	SCPPField*		psField;
	SCPP65Type*		psType;
	size			i;

	pcCopy = pcClass->GetCopier(false, false);
	if (pcCopy)
	{
		return pcCopy->mbDeleted;
	}
	if (pcClass->GetCopier(false, true) || pcClass->GetCopier(true, true))
	{
		return true;
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		if (IsCopyDeleted(pcClass->macBases.Get(i)->pcClass))
		{
			return true;
		}
	}
	for (i = 0; !pcClass->mbUnion && (i < pcClass->macFields.NumElements()); i++)
	{
		psField = pcClass->macFields.Get(i);
		psType = psField->psType->InnermostElement();
		if ((psType->IsReference() && psType->bRvalue) || (psType->IsClass() && IsCopyDeleted(psType->pcClass)))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an object of the class can't be made from an rvalue of it: the
// move constructor it declares is deleted, or with none the copy
// constructor it declares; the compiler declares no move where the class
// declares a copy assignment, a move assignment or a destructor, and then
// the copy constructor makes it (IsCopyDeleted); else a base or a field
// can't be moved.  [class.copy] 9, 11
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::IsMoveDeleted(CCPPClass* pcClass)
{
	CCPPFunction*	pcCopier;
	SCPPField*		psField;
	SCPP65Type*		psType;
	size			i;

	pcCopier = pcClass->GetCopier(false, true);
	if (!pcCopier)
	{
		pcCopier = pcClass->GetCopier(false, false);
	}
	if (pcCopier)
	{
		return pcCopier->mbDeleted;
	}
	if (pcClass->GetCopier(true, false) || pcClass->GetCopier(true, true) || (pcClass->mpcDestructor && !pcClass->mpcDestructor->mbImplicit))
	{
		return IsCopyDeleted(pcClass);
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		if (IsMoveDeleted(pcClass->macBases.Get(i)->pcClass))
		{
			return true;
		}
	}
	for (i = 0; !pcClass->mbUnion && (i < pcClass->macFields.NumElements()); i++)
	{
		psField = pcClass->macFields.Get(i);
		psType = psField->psType->InnermostElement();
		if (psType->IsClass() && IsMoveDeleted(psType->pcClass))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a member's name (a field, a function, a type) may be named at
// pcNode as a member of pcNaming, for a pointer to member (&C::m: the
// protected rule's object is pcNaming).  An error (and false) if not.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::ReachesMember(CCPPSymbol* pcSymbol, CCPPClass* pcNaming, CCPPSyntax* pcNode)
{
	return mcAccess.CheckSymbol(pcSymbol, pcNaming, pcNode) && mcAccess.CheckProtectedObject(pcSymbol, pcNaming, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a class's using-declaration may name a base's member (it isn't
// private there; CCPPAccess::CheckUsing).  An error (and false) if not.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::UsingReaches(CCPPSymbol* pcSymbol, CCPPSyntax* pcUsing)
{
	return mcAccess.CheckUsing(pcSymbol, pcUsing);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a field of the class (a union's, say) is an object that making
// it runs code: a constructor of its class, or what NeedsConstructor says.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::HasMemberMadeByCode(CCPPClass* pcClass)
{
	SCPPField*		psField;
	SCPP65Type*		psType;
	size			i;

	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		psField = pcClass->macFields.Get(i);
		psType = psField->psType->ElementType();
		if (psType->IsClass() && psType->pcClass->NeedsConstructor())
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcInit's = value, or = { values }, may make an object of pcClass
// by the constructor that takes it: copy initialisation uses no explicit
// constructor, and the constructor is the one conversion of the program's
// own there is, so its parameter takes the value by none.  An error (and
// false) if not.  Any other initialiser may.  [dcl.init] 17,
// [over.match.copy], [over.match.list], [class.conv] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::CopyInitialises(CCPPClass* pcClass, CCPPSyntaxInitDeclarator* pcInit)
{
	CCPPBoundConstruction	cBound;
	CArraySyntax			apcValue;
	CArraySyntax*			papcArguments;
	SCPP65Type*				psValue;
	SCPP65Type*				psParameter;
	bool					bResult;

	if ((pcInit->meInitializer != CPPI_Equals) || !pcInit->mpcInitializer || !pcClass->mbConstructors)
	{
		return true;
	}
	apcValue.Init();
	apcValue.Add(pcInit->mpcInitializer);
	papcArguments = (pcInit->mpcInitializer->GetKind() == CPPS_InitializerList) ? mpcUnit->ListElements(pcInit->mpcInitializer) : &apcValue;
	if ((papcArguments == &apcValue) && !CopiesByConstructor(pcClass, pcInit->mpcInitializer))
	{
		//= an object of the class, which no constructor of its own copies: it is copied as its parts or bytes are.
		psValue = mpcUnit->GetTypeChecker()->ValueTypeOf(pcInit->mpcInitializer);
		psValue = psValue ? psValue->WithoutReference() : NULL;
		if (psValue && psValue->IsClass() && mpcUnit->IsDerived(psValue->pcClass, pcClass, NULL))
		{
			apcValue.Kill();
			return true;
		}
	}
	bResult = true;
	if ((papcArguments->NumElements() > 0) && BindConstruction(pcClass, papcArguments, pcInit, &cBound, false) && (cBound.meHow == CPPBO_Constructor))
	{
		psValue = (papcArguments == &apcValue) ? mpcUnit->GetTypeChecker()->ValueTypeOf(pcInit->mpcInitializer) : NULL;
		psParameter = (cBound.mpcConstructor->mapsParameters.NumElements() > 0) ? cBound.mpcConstructor->mapsParameters.GetPtr(0)->WithoutReference() : NULL;
		if (cBound.mpcConstructor->mbExplicit)
		{
			bResult = mpcDiagnostics->ReportOnce(pcInit, "An explicit constructor isn't used by = : write the class's name before the value");
		}
		else if (psValue && psParameter && !(psParameter->IsClass() && (psParameter->pcClass == pcClass)) && (mcOverloads.ConversionRank(psParameter, psValue, pcInit->mpcInitializer, false) == CPPCR_None))
		{
			bResult = mpcDiagnostics->ReportOnce(pcInit, "= converts the value by one constructor or conversion function of the program's, not two");
		}
	}
	apcValue.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an object of pcClass may be destroyed at pcNode: the destructor
// the class declares, if it does, isn't deleted and may be named there.  An
// error (and false) if not.  [class.dtor] 11
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::ReachesDestructor(CCPPClass* pcClass, CCPPSyntax* pcNode)
{
	if (pcClass->mpcDestructor && pcClass->mpcDestructor->mbDeleted)
	{
		return mpcDiagnostics->ReportOnce(pcNode, "The class's destructor is deleted: no object of it is made where it would be destroyed");
	}
	return !pcClass->mpcDestructor || mcAccess.CheckFunction(pcClass->mpcDestructor, pcClass, pcNode);
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
	if (psTo && !psTo->IsClass() && !psTo->IsReference() && !psTo->IsVoid() && mpcUnit->GetTypeChecker()->IsObject(pcExpression))
	{
		BindObjectConversion(mpcUnit->GetTypeChecker()->ObjectClass(pcExpression), psTo, bExplicit, pcExpression, pcBound);
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
	if (pcBound->mpcFunction && mcOverloads.mbAmbiguousConversion && pcNode)
	{
		mpcDiagnostics->ReportOnce(pcNode, "The conversion is ambiguous: two conversion functions' results convert to the type as well as each other");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type an expression is contextually implicitly converted to where one
// of the types of eWanted is wanted (an integer for a switch's condition or a
// new array's bound, a pointer to an object for delete's operand, a number
// for the built-in -): its own, or for an object the type its class's one
// fitting conversion function gives (CCPPOverloads::FindContextualConversion).
// NULL, with why, for an object of a class with none or with more than one.
// [conv] 5, [over.built]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPBinder::ContextualType(CCPPSyntax* pcExpression, ECPPContextualTypes eWanted)
{
	CCPPFunction*	pcConversion;
	const char*		szNone;
	bool			bAmbiguous;

	if (!mpcUnit->GetTypeChecker()->IsObject(pcExpression))
	{
		return mpcUnit->GetTypeChecker()->ValueTypeOf(pcExpression);
	}
	pcConversion = mcOverloads.FindContextualConversion(mpcUnit->GetTypeChecker()->ObjectClass(pcExpression), eWanted, pcExpression, &bAmbiguous);
	if (!pcConversion)
	{
		szNone = (eWanted == CPPXT_ObjectPointer) ? "The object doesn't convert to a pointer" : ((eWanted == CPPXT_Integer) ? "The object doesn't convert to an integer" : "The object doesn't convert to a number");
		if (bAmbiguous || !mpcDiagnostics->HasErrorAt(pcExpression))
		{
			//(Where its own operator was ambiguous, that has been said, and is why.)
			mpcDiagnostics->Error(pcExpression, bAmbiguous ? "The object converts to more than one type here" : szNone);
		}
		return NULL;
	}
	return pcConversion->mpsReturn->WithoutReference()->Unqualified();
}


//////////////////////////////////////////////////////////////////////////
//
// The one conversion function of a class that gives an integer, by which an
// object of it is one where an integer is wanted, or NULL.  [conv] 5
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPBinder::FindIntegerConversion(CCPPClass* pcClass, CCPPSyntax* pcNode)
{
	bool	bAmbiguous;

	return mcOverloads.FindContextualConversion(pcClass, CPPXT_Integer, pcNode, &bAmbiguous);
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
// Whether an expression is the value of a declaration's = value: what it
// is made with is copy initialised.  [dcl.init] 15
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::IsCopyInitialiser(CCPPSyntax* pcValue)
{
	CCPPSyntaxInitDeclarator*	pcInit;

	if (!pcValue->mpcParent || (pcValue->mpcParent->GetKind() != CPPS_InitDeclarator))
	{
		return false;
	}
	pcInit = (CCPPSyntaxInitDeclarator*)pcValue->mpcParent;
	return (pcInit->meInitializer == CPPI_Equals) && (pcInit->mpcInitializer == pcValue);
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
	if (pcClass->mpsLambda && (papcArguments->NumElements() == 0))
	{
		//decltype(lambda) object;  [expr.prim.lambda] 20
		return mpcDiagnostics->ReportOnce(pcNode, "A lambda's closure has no default constructor: it is made by its lambda, or copied");
	}
	if (pcClass->mbUnion && pcClass->mpcSpecifier && pcClass->mpcSpecifier->mpcName && !pcClass->mbConstructors && (papcArguments->NumElements() == 0) && HasMemberMadeByCode(pcClass))
	{
		//(An anonymous union's members are its class's, which its constructor makes or not.)  [class.union] 2, [class.ctor] 4
		return mpcDiagnostics->ReportOnce(pcNode, "The union's default constructor is deleted: a member of it is made by a constructor, which the union can't choose to call");
	}
	if (pcClass->mbConstructors || (papcArguments->NumElements() == 0))
	{
		//T x = value; has only the converting constructors as candidates.  [over.match.ctor] 1
		pcBound->mpcConstructor = mcOverloads.FindConstructor(pcClass, papcArguments, pcNode, (papcArguments->NumElements() == 1) && IsCopyInitialiser(papcArguments->GetPtr(0)));
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
			if (bReport && !mpcDiagnostics->HasErrorAt(pcNode))
			{
				//(Where choosing among the constructors said why, that is the reason.)
				mpcDiagnostics->Error(pcNode, "No constructor matches the arguments");
			}
			return false;
		}
		pcBound->meHow = CPPBO_Nothing;
		return true;
	}

	if ((pcClass->macBases.NumElements() > 0) || pcClass->IsPolymorphic() || (papcArguments->NumElements() > pcClass->NumAggregateFields()) ||
		!mcAccess.HasOnlyPublicFields(pcClass))
	{
		if (bReport)
		{
			mpcDiagnostics->Error(pcNode, "These initialisers do not match the class");
		}
		return false;
	}
	pcBound->meHow = CPPBO_Aggregate;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The class of the object in object.member or pointer->member, or NULL if
// it isn't an object of a class.  Nothing is reported.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPBinder::MemberObjectClass(CCPPSyntaxMemberExpression* pcMember)
{
	SCPP65Type*	psObject;

	mpcUnit->mcDiagnostics.BeginQuiet();
	psObject = pcMember->mbArrow ? mpcUnit->ArrowType(pcMember->mpcObject, pcMember) : mpcUnit->mcTypeChecker.ValueTypeOf(pcMember->mpcObject);
	mpcUnit->mcDiagnostics.EndQuiet();
	if (psObject && pcMember->mbArrow)
	{
		psObject = psObject->IsPointer() ? psObject->psBase : NULL;
	}
	return (psObject && psObject->IsClass()) ? psObject->pcClass : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// What the member of object.member or pointer->member is, in *pcBound
// (CCPPBinder::BindMember, in the object's class), with nothing reported:
// false if it is nothing, or the object isn't one.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::BindMemberOf(CCPPSyntaxMemberExpression* pcMember, CCPPBoundName* pcBound)
{
	CCPPClass*	pcClass;
	bool		bBound;

	pcClass = MemberObjectClass(pcMember);
	mpcUnit->mcDiagnostics.BeginQuiet();
	bBound = pcClass && mpcUnit->mcBinder.BindMember(pcClass, pcMember->mpcName, pcMember, pcBound);
	mpcUnit->mcDiagnostics.EndQuiet();
	return bBound;
}


//////////////////////////////////////////////////////////////////////////
//
// The enumerator that object.name or pointer->name names, a member of the
// object's class (s.E is S::E, once s is worked out), or NULL.
// [expr.ref] 4
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPBinder::MemberEnumerator(CCPPSyntaxMemberExpression* pcMember)
{
	CCPPClass*	pcClass;
	CCPPClass*	pcDeclaring;

	pcClass = MemberObjectClass(pcMember);
	return (pcClass && mpcUnit->CompleteClass(pcClass, pcMember)) ? pcClass->FindMemberSymbol(mpcUnit->GetComponentName(pcMember->mpcName->GetLast()), CPPSK_Enumerator, &pcDeclaring) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The bit-field an expression names, or NULL: a field of this named alone,
// or one after . or ->, that is declared with a width, and c ? x : y that
// picks one (its first).
//
//////////////////////////////////////////////////////////////////////////
SCPPField* CCPPBinder::BitFieldOf(CCPPSyntax* pcExpression)
{
	CCPPSyntaxConditionalExpression*	pcConditional;
	CCPPBoundName						cBound;
	SCPPField*							psField;

	pcExpression = mpcUnit->SkipParentheses(pcExpression);
	cBound.meWhat = CPPBN_Nothing;
	if (pcExpression->GetKind() == CPPS_ConditionalExpression)
	{
		pcConditional = (CCPPSyntaxConditionalExpression*)pcExpression;
		psField = BitFieldOf(pcConditional->mpcTrue);
		return psField ? psField : BitFieldOf(pcConditional->mpcFalse);
	}
	if (pcExpression->GetKind() == CPPS_IdExpression)
	{
		mpcUnit->mcDiagnostics.BeginQuiet();
		mpcUnit->mcBinder.BindName((CCPPSyntaxIdExpression*)pcExpression, &cBound);
		mpcUnit->mcDiagnostics.EndQuiet();
	}
	else if ((pcExpression->GetKind() == CPPS_MemberExpression) && !BindMemberOf((CCPPSyntaxMemberExpression*)pcExpression, &cBound))
	{
		cBound.meWhat = CPPBN_Nothing;
	}
	return ((cBound.meWhat == CPPBN_Field) && cBound.mpsField->IsBitField()) ? cBound.mpsField : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// What a reference is initialised from: a list in braces with one element
// the reference binds to as it is (its type the reference's, or a class
// derived from it) is that element, int& r{x}; any other list makes a
// temporary the reference is bound to (BindsToTemporary).
// [dcl.init.list] 3
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPBinder::BoundInitialiser(SCPP65Type* psReference, CCPPSyntax* pcInitialiser)
{
	CArraySyntax*	papcElements;
	CCPPSyntax*		pcElement;
	SCPP65Type*		psElement;
	SCPP65Type*		psReferred;

	if (!pcInitialiser || (pcInitialiser->GetKind() != CPPS_InitializerList))
	{
		return pcInitialiser;
	}
	papcElements = mpcUnit->ListElements(pcInitialiser);
	pcElement = (papcElements->NumElements() == 1) ? papcElements->GetPtr(0) : NULL;
	if (!pcElement || (pcElement->GetKind() == CPPS_InitializerList))
	{
		return pcInitialiser;
	}
	mpcUnit->mcDiagnostics.BeginQuiet();
	psElement = mpcUnit->mcTypeChecker.PlaceOf(pcElement);
	psElement = psElement ? psElement : mpcUnit->mcTypeChecker.ValueTypeOf(pcElement);
	mpcUnit->mcDiagnostics.EndQuiet();
	psElement = psElement ? psElement->WithoutReference()->Unqualified() : NULL;
	psReferred = psReference->psBase->Unqualified();
	if (psElement && (psElement->Equals(psReferred) || (psElement->IsClass() && psReferred->IsClass() && mpcUnit->IsDerived(psElement->pcClass, psReferred->pcClass, NULL))))
	{
		return pcElement;
	}
	return pcInitialiser;
}


//////////////////////////////////////////////////////////////////////////
//
// True if a reference gets a temporary holding the value, not the value's
// address: a const reference or T&&, and a value that has no address or
// is of another type (const long& of a char, const char* const& of an
// array, const Class& of what a Class is made from, a list in braces).  An
// object of the class, or of one derived from it, is referred to itself.
// [dcl.init.ref]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::BindsToTemporary(SCPP65Type* psReference, CCPPSyntax* pcArgument)
{
	SCPP65Type*			psPlace;
	SCPP65Type*			psValue;
	CCPPBoundConversion	cConversion;

	if (!(psReference->psBase->bConst || psReference->bRvalue))
	{
		return false;
	}
	if (pcArgument->GetKind() == CPPS_InitializerList)
	{
		//const S& s = {1, 2}: what the list makes.  [dcl.init.list] 3
		return true;
	}
	if (psReference->psBase->IsClass())
	{
		psValue = mpcUnit->mcTypeChecker.ValueTypeOf(pcArgument);
		return psValue && !(psValue->IsClass() && ((psValue->pcClass == psReference->psBase->pcClass) || mpcUnit->IsDerived(psValue->pcClass, psReference->psBase->pcClass, NULL)));
	}
	psValue = mpcUnit->mcTypeChecker.ValueTypeOf(pcArgument);
	if (psValue && psValue->IsClass() && !psReference->bRvalue)
	{
		//const T& binds to what the object's operator T& returns, itself; a temporary only holds what another
		//conversion function's result converts to.  [dcl.init.ref] 5.1.2, [over.match.ref]
		mpcUnit->mcBinder.BindObjectConversion(psValue->pcClass, psReference, false, NULL, &cConversion);
		return !cConversion.mpcFunction || !cConversion.mpcFunction->mpsReturn->IsLvalueReference() ||
			   !psReference->psBase->Unqualified()->Equals(cConversion.mpcFunction->mpsReturn->psBase->Unqualified()) || !psReference->psBase->HasQualifiersOf(cConversion.mpcFunction->mpsReturn->psBase);
	}
	if (IsUncapturedConstant(pcArgument))
	{
		//A constant a lambda doesn't capture has no object in it to refer to: binding a reference to it odr-uses it.
		//[basic.def.odr] 3
		mpcUnit->mcDiagnostics.ReportOnce(pcArgument, "The lambda doesn't capture the variable");
		return true;
	}
	psPlace = mpcUnit->mcTypeChecker.PlaceOf(pcArgument);
	return !psPlace || !psReference->psBase->Equals(psPlace->WithoutReference());
}


//////////////////////////////////////////////////////////////////////////
//
// True if the expression names a constant of the function a lambda being
// written is in, which the lambda doesn't capture (CPPBN_EnclosingConstant):
// only its value can be used.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::IsUncapturedConstant(CCPPSyntax* pcExpression)
{
	CCPPBoundName	cBound;

	pcExpression = mpcUnit->SkipParentheses(pcExpression);
	if (!pcExpression || (pcExpression->GetKind() != CPPS_IdExpression))
	{
		return false;
	}
	mpcUnit->mcBinder.BindName((CCPPSyntaxIdExpression*)pcExpression, &cBound);
	return cBound.meWhat == CPPBN_EnclosingConstant;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether left, right is a call of an operator, because an operand is an
// object and one matches.  Other operators with an object need one; a comma
// without one is the built in comma.  [over.match.oper] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::IsOverloadedComma(CCPPSyntaxBinaryExpression* pcBinary)
{
	CCPPBoundCall	cBound;
	bool			bOverloaded;

	if (!mpcUnit->mcTypeChecker.IsObject(pcBinary->mpcLeft) && !mpcUnit->mcTypeChecker.IsObject(pcBinary->mpcRight))
	{
		return false;
	}
	mpcUnit->mcDiagnostics.BeginSilent();
	bOverloaded = mpcUnit->mcBinder.BindOperator(CT_Comma, pcBinary->mpcLeft, pcBinary->mpcRight, pcBinary, &cBound, false);
	mpcUnit->mcDiagnostics.EndSilent();
	return bOverloaded;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an operator's operand may make it an operator function's call: an
// object, or an enumerator or enum value where an operator function of the
// operator's name that is no class's is declared (else it is only ever the
// built-in operator, and isn't looked for).  [over.match.oper] 1, 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::IsOperatorOperand(CCPPSyntax* pcExpression, ECppTokens eOperator)
{
	SCPP65Type*	psType;

	psType = mpcUnit->mcTypeChecker.ValueTypeOf(pcExpression);
	if (!psType)
	{
		return false;
	}
	if (psType->IsClass())
	{
		return true;
	}
	return psType->pcEnum && mpcUnit->mcBinder.HasFreeOperator(mpcUnit->GetOperatorName(eOperator, false));
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an identifier (lliName) is szName.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::IsNamed(int64 lliName, const char* szName)
{
	char*	szIdentifier;
	int		iLength;

	if (lliName < 0)
	{
		return false;
	}
	szIdentifier = mpcUnit->mpcTokeniser->GetIdentifier(lliName, &iLength);
	return ((size)iLength == strlen(szName)) && (memcmp(szIdentifier, szName, iLength) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a name declared in pcScope is std::szName: the library's, which
// the compiler knows (std::initializer_list, std::type_info).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBinder::IsStdName(int64 lliName, CCPPScope* pcScope, const char* szName)
{
	return pcScope && pcScope->mpcOwner && IsNamed(lliName, szName) && IsNamed(pcScope->mpcOwner->mlliName, "std") && (!pcScope->mpcParent || !pcScope->mpcParent->mpcOwner);
}

