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
#include "BaseLib/Chars.h"
#include "CppTokeniser.h"
#include "CPPSyntaxTree.h"
#include "CPPUnit.h"
#include "CPPSemanticPass.h"
#include "CPP65TypeChecker.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
	mpcDiagnostics = pcUnit->GetDiagnostics();
	mpcTypes = pcUnit->GetTypes();
	mapcNone.Init();
	mcFacts.Init();
	mcFactsMemory.Init(16384);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::Kill(void)
{
	mcFacts.Kill();
	mcFactsMemory.Kill();
	mapcNone.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What is known of an expression where it is being worked out (the
// context's bindings and function): its type, worked out the first time
// it is asked for there, with what was said doing it kept (stage 4's
// keeping).  The record is the memo's: good until the next is made.
//
//////////////////////////////////////////////////////////////////////////
SCPPExpressionFacts* CCPP65TypeChecker::Facts(CCPPSyntax* pcExpression)
{
	SCPPExpressionFacts*	psFacts;
	CCPPCompileStatistics*	pcStatistics;
	SCPP65Type*				psType;
	SCPP65Type*				psDeclared;
	CCPPErrors*				pcErrors;

	pcStatistics = mpcUnit->GetStatistics();
	pcStatistics->Count(CPPCC_TypeOf);
	psFacts = FactsEntry(pcExpression);
	if (psFacts->bTyped)
	{
		return psFacts;
	}

	pcStatistics->Count(CPPCC_TypeOfWorkedOut);
	mpcDiagnostics->BeginKeeping();
	pcStatistics->Begin(CPPCP_Types);
	psType = Value(pcExpression, &psDeclared);
	pcStatistics->End(CPPCP_Types);
	pcErrors = mpcDiagnostics->EndKeeping();

	//Working it out made other records: this one is found again.
	psFacts = FactsEntry(pcExpression);
	psFacts->bTyped = true;
	psFacts->psType = psType;
	psFacts->psDeclared = psDeclared;
	psFacts->pcErrors = pcErrors;
	return psFacts;
}


//////////////////////////////////////////////////////////////////////////
//
// The record of what is known of an expression where it is being worked
// out (the context's bindings and function), nothing known yet if it is
// new there: one of other bindings or another function is begun again.
//
//////////////////////////////////////////////////////////////////////////
SCPPExpressionFacts* CCPP65TypeChecker::FactsEntry(CCPPSyntax* pcExpression)
{
	SCPPExpressionFacts**	ppsFacts;
	SCPPExpressionFacts*	psFacts;
	CCPPContext*			pcContext;
	bool					bAdded;

	pcContext = mpcUnit->GetContext();
	ppsFacts = mcFacts.FindOrAdd((int64)(size)pcExpression, &bAdded);
	if (bAdded)
	{
		*ppsFacts = (SCPPExpressionFacts*)mcFactsMemory.Add(sizeof(SCPPExpressionFacts));
	}
	else if (((*ppsFacts)->pcBindings == pcContext->GetBindings()) && ((*ppsFacts)->pcFunction == pcContext->GetFunction()))
	{
		return *ppsFacts;
	}
	psFacts = *ppsFacts;
	memset(psFacts, 0, sizeof(SCPPExpressionFacts));
	psFacts->pcBindings = pcContext->GetBindings();
	psFacts->pcFunction = pcContext->GetFunction();
	return psFacts;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of an expression's place (Place), or NULL if it has none, with
// nothing said of it (a trial): worked out once where it is evaluated.  In
// an operand that isn't evaluated a place may be what isn't one elsewhere
// (S::m in sizeof), so it is worked out each time there.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::PlaceFact(CCPPSyntax* pcExpression)
{
	SCPPExpressionFacts*	psFacts;
	CCPPCompileStatistics*	pcStatistics;
	SCPP65Type*				psPlace;
	bool					bEvaluated;

	bEvaluated = !mpcUnit->GetContext()->IsUnevaluated();
	psFacts = bEvaluated ? FactsEntry(pcExpression) : NULL;
	if (psFacts && psFacts->bPlaced)
	{
		return psFacts->psPlace;
	}

	pcStatistics = mpcUnit->GetStatistics();
	pcStatistics->Count(CPPCC_PlaceWorkedOut);
	//A failure here is not one of the quiet code around it.
	mpcDiagnostics->BeginTrial();
	pcStatistics->Begin(CPPCP_Types);
	psPlace = Place(pcExpression);
	pcStatistics->End(CPPCP_Types);
	mpcDiagnostics->EndTrial();
	if (bEvaluated)
	{
		psFacts = FactsEntry(pcExpression);
		psFacts->bPlaced = true;
		psFacts->psPlace = psPlace;
	}
	return psPlace;
}


//////////////////////////////////////////////////////////////////////////
//
// What an expression is, an lvalue, an xvalue or a prvalue ([basic.lval]):
// worked out once where it is evaluated, as its place is (in an operand
// that isn't evaluated a name may be a place that isn't one elsewhere).
//
//////////////////////////////////////////////////////////////////////////
ECPPValueCategory CCPP65TypeChecker::Category(CCPPSyntax* pcExpression)
{
	SCPPExpressionFacts*	psFacts;
	ECPPValueCategory		eCategory;
	bool					bEvaluated;

	bEvaluated = !mpcUnit->GetContext()->IsUnevaluated();
	psFacts = bEvaluated ? FactsEntry(pcExpression) : NULL;
	if (psFacts && psFacts->bCategorised)
	{
		return psFacts->eCategory;
	}
	eCategory = WorkOutCategory(pcExpression);
	if (bEvaluated)
	{
		psFacts = FactsEntry(pcExpression);
		psFacts->bCategorised = true;
		psFacts->eCategory = eCategory;
	}
	return eCategory;
}


//////////////////////////////////////////////////////////////////////////
//
// The category of each kind of expression, from its clause: a name that
// has a place, a string, *p, ++x, x = y and p->m are lvalues, and a.m is
// a's; a call, a cast and an operator function's call are what their type
// is declared to be (T& an lvalue, T&& an xvalue, T a prvalue); x, y is
// y's; c ? x : y is an lvalue or an xvalue if both are; anything else is a
// prvalue.  [expr.prim.general] 8, [expr.ref] 4, [expr.call] 10,
// [expr.cast] 1, [expr.unary.op] 1, [expr.pre.incr] 1, [expr.ass] 1,
// [expr.comma] 1, [expr.cond] 4, [expr.sub] 1, [lex.string] 8
//
//////////////////////////////////////////////////////////////////////////
ECPPValueCategory CCPP65TypeChecker::WorkOutCategory(CCPPSyntax* pcExpression)
{
	CCPPSyntaxUnaryExpression*			pcUnary;
	CCPPSyntaxBinaryExpression*			pcBinary;
	CCPPSyntaxMemberExpression*			pcMember;
	CCPPSyntaxConditionalExpression*	pcConditional;
	CCPPBoundName						cBound;
	ECPPValueCategory					eObject;
	SCPP65Type*							psType;

	pcExpression = mpcUnit->SkipParentheses(pcExpression);
	if (!pcExpression)
	{
		return CPPVC_Prvalue;
	}
	switch (pcExpression->GetKind())
	{
	case CPPS_IdExpression:
		return PlaceFact(pcExpression) ? CPPVC_Lvalue : CPPVC_Prvalue;

	case CPPS_StringExpression:
		return mpcUnit->GetDesugarings()->UserLiteralToken(pcExpression) ? DeclaredCategory(pcExpression) : CPPVC_Lvalue;

	case CPPS_LiteralExpression:
		return mpcUnit->GetDesugarings()->UserLiteralToken(pcExpression) ? DeclaredCategory(pcExpression) : CPPVC_Prvalue;

	case CPPS_MemberExpression:
		pcMember = (CCPPSyntaxMemberExpression*)pcExpression;
		if (!mpcUnit->GetBinder()->BindMemberOf(pcMember, &cBound) || (cBound.meWhat != CPPBN_Field))
		{
			//A static member is an lvalue, an enumerator a prvalue.
			return PlaceFact(pcExpression) ? CPPVC_Lvalue : CPPVC_Prvalue;
		}
		if (pcMember->mbArrow || cBound.mpsField->psType->IsReference())
		{
			return CPPVC_Lvalue;
		}
		eObject = Category(pcMember->mpcObject);
		return (eObject == CPPVC_Lvalue) ? CPPVC_Lvalue : CPPVC_Xvalue;

	case CPPS_SubscriptExpression:
		return IsOperatorCall(pcExpression) ? DeclaredCategory(pcExpression) : CPPVC_Lvalue;

	case CPPS_CallExpression:
	case CPPS_CastExpression:
	case CPPS_NamedCastExpression:
		return DeclaredCategory(pcExpression);

	case CPPS_TypeidExpression:
		return CPPVC_Lvalue;

	case CPPS_UnaryExpression:
		if (IsOperatorCall(pcExpression))
		{
			return DeclaredCategory(pcExpression);
		}
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		if ((pcUnary->meOperator == CT_Multiply) || (((pcUnary->meOperator == CT_Increment) || (pcUnary->meOperator == CT_Decrement)) && !pcUnary->mbPostfix))
		{
			return CPPVC_Lvalue;
		}
		return CPPVC_Prvalue;

	case CPPS_BinaryExpression:
		pcBinary = (CCPPSyntaxBinaryExpression*)pcExpression;
		if ((pcBinary->meOperator == CT_Comma) && !mpcUnit->GetBinder()->IsOverloadedComma(pcBinary))
		{
			return Category(pcBinary->mpcRight);
		}
		if (mpcUnit->IsMemberPointerOperator(pcExpression))
		{
			//p->*m is an lvalue, o.*m o's (an object on the left is no operator's operand); of a member function,
			//the call it is made for.  [expr.mptr.oper] 6
			if (!PlaceFact(pcExpression))
			{
				return CPPVC_Prvalue;
			}
			if (pcBinary->meOperator == CT_MemberByPointerFunctionPointerIndirection)
			{
				return CPPVC_Lvalue;
			}
			return (Category(pcBinary->mpcLeft) == CPPVC_Lvalue) ? CPPVC_Lvalue : CPPVC_Xvalue;
		}
		if (pcBinary->meOperator == CT_Assignment)
		{
			//x = y is x; an object's is its operator='s, whose implicit one returns the object (T&).  One the
			//program declares to return a value is taken for the same.  [expr.ass] 1, [class.copy] 18
			return (DeclaredCategory(pcExpression) == CPPVC_Xvalue) ? CPPVC_Xvalue : CPPVC_Lvalue;
		}
		if (IsOperatorCall(pcExpression))
		{
			return DeclaredCategory(pcExpression);
		}
		if (IsCPPAssignment(pcBinary->meOperator))
		{
			return CPPVC_Lvalue;
		}
		return CPPVC_Prvalue;

	case CPPS_ConditionalExpression:
		pcConditional = (CCPPSyntaxConditionalExpression*)pcExpression;
		if (ConditionalLvalueType(pcConditional))
		{
			return CPPVC_Lvalue;
		}
		if (ConditionalXvalueType(pcConditional))
		{
			return CPPVC_Xvalue;
		}
		break;

	default:
		break;
	}
	psType = Facts(pcExpression)->psType;
	return (psType && psType->IsClass()) ? CPPVC_ClassPrvalue : CPPVC_Prvalue;
}


//////////////////////////////////////////////////////////////////////////
//
// The category of a call, a cast or an operator function's call: what its
// type is declared to be.  T& is an lvalue, T&& an xvalue (of a function,
// an lvalue), T a prvalue.
//
//////////////////////////////////////////////////////////////////////////
ECPPValueCategory CCPP65TypeChecker::DeclaredCategory(CCPPSyntax* pcExpression)
{
	SCPPExpressionFacts*	psFacts;
	SCPP65Type*				psDeclared;

	psFacts = Facts(pcExpression);
	psDeclared = psFacts->psDeclared;
	if (psDeclared && psDeclared->IsReference())
	{
		return (psDeclared->bRvalue && !psDeclared->psBase->IsFunction()) ? CPPVC_Xvalue : CPPVC_Lvalue;
	}
	return (psFacts->psType && psFacts->psType->IsClass()) ? CPPVC_ClassPrvalue : CPPVC_Prvalue;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an operator expression is an operator function's call: one of
// its operands is an object.  [over.match.oper] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::IsOperatorCall(CCPPSyntax* pcExpression)
{
	switch (pcExpression->GetKind())
	{
	case CPPS_UnaryExpression:
		return IsObject(((CCPPSyntaxUnaryExpression*)pcExpression)->mpcOperand);
	case CPPS_BinaryExpression:
		return IsObject(((CCPPSyntaxBinaryExpression*)pcExpression)->mpcLeft) || IsObject(((CCPPSyntaxBinaryExpression*)pcExpression)->mpcRight);
	case CPPS_SubscriptExpression:
		return IsObject(((CCPPSyntaxSubscriptExpression*)pcExpression)->mpcArray);
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type of an expression's value: a reference's is what it refers to.  Arrays and
// functions named are pointers to them (see Load).  ppsDeclared is given the type
// before that: a reference where the expression is declared to give one (a call of a
// function that returns T& or T&&, a cast to one), which says what kind of value it is.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::Value(CCPPSyntax* pcExpression, SCPP65Type** ppsDeclared)
{
	SCPP65Type*		psType;

	psType = Result(pcExpression);
	if (ppsDeclared)
	{
		*ppsDeclared = psType;
	}
	if (psType && psType->IsReference())
	{
		psType = psType->psBase;
		if (!psType->IsClass() && !mpcUnit->Supported(psType, pcExpression))
		{
			return NULL;
		}
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of an expression's value where a psTo is wanted (NULL when it isn't
// known): a function's name for a pointer to a function is the overload of that
// type (&Class::f for a pointer to a member function the member of that type),
// an object is converted by its conversion function to psTo (only an
// explicit one when bExplicit; what it refers to, if it returns a reference to
// a number or a pointer), and a list in braces is its one value or zero.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ValueFor(CCPPSyntax* pcExpression, SCPP65Type* psTo, bool bExplicit)
{
	CCPPBoundConversion			cBound;
	CCPPSyntaxIdExpression*		pcFunctionName;
	SCPP65Type*					psMember;
	SCPP65Type*					psConverted;

	if (pcExpression && (pcExpression->GetKind() == CPPS_InitializerList) && psTo && !psTo->IsClass())
	{
		//{} is zero and {value} the value, not narrowed.  [dcl.init.list] 3
		return Construct(psTo, NULL, pcExpression, pcExpression) ? psTo : NULL;
	}
	pcFunctionName = mpcUnit->FunctionNameFor(pcExpression, psTo);
	if (pcFunctionName)
	{
		return FunctionAddress(pcFunctionName, pcFunctionName->mpcName->mpcSymbol->mlliName, psTo->psBase) ? psTo : NULL;
	}
	pcFunctionName = mpcUnit->MemberFunctionNameFor(pcExpression, psTo);
	if (pcFunctionName && MemberAddress(pcFunctionName, psTo->psBase, &psMember))
	{
		return psMember;
	}

	mpcUnit->GetBinder()->BindConversion(pcExpression, psTo, bExplicit, &cBound);
	if (cBound.meHow == CPPBV_Function)
	{
		psConverted = ObjectCall(pcExpression, cBound.mpcFunction, NULL, pcExpression);
		return (psConverted && psConverted->IsReference() && psConverted->psBase->IsScalar()) ? psConverted->psBase : psConverted;
	}
	return Value(pcExpression);
}


//////////////////////////////////////////////////////////////////////////
//
// The type of what an expression gives: as Value, but a call to a function that
// returns a reference gives the reference's type.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::Result(CCPPSyntax* pcExpression)
{
	if (!pcExpression)
	{
		return NULL;
	}

	//The node's own Visit works its type out, the types of what is in it on the way, and leaves it here last.
	mpsResult = NULL;
	pcExpression->Accept(this);
	mpcUnit->GetCheck()->Typed(pcExpression, mpsResult);
	return mpsResult;
}


//////////////////////////////////////////////////////////////////////////
//
// What Result gives for each kind of expression.  A kind that has no function here isn't an expression the
// compiler does.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitDefault(CCPPSyntax* pcNode)
{
	Error(pcNode, "This expression is not supported by the compiler yet");
	mpsResult = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// object[index] is a call to the object's operator[], which may return a value.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitSubscriptExpression(CCPPSyntaxSubscriptExpression* pcNode)
{
	CArraySyntax	apcIndex;
	SCPP65Type*		psType;

	psType = TypeOf(pcNode->mpcArray);
	if (psType && psType->IsClass())
	{
		apcIndex.Init();
		apcIndex.Add(pcNode->mpcIndex);
		psType = OperatorCall(pcNode->mpcArray, CT_OpenSquareBracket, &apcIndex, pcNode);
		apcIndex.Kill();
		mpsResult = psType;
		return;
	}
	mpsResult = Load(Place(pcNode), pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitLiteralExpression(CCPPSyntaxLiteralExpression* pcNode)
{
	mpsResult = mpcUnit->GetDesugarings()->UserLiteralToken(pcNode) ? LiteralCall(pcNode) : ConstantValue(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitStringExpression(CCPPSyntaxStringExpression* pcNode)
{
	mpsResult = mpcUnit->GetDesugarings()->UserLiteralToken(pcNode) ? LiteralCall(pcNode) : StringLiteral(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// A user-defined literal: the call of its literal operator
// (CCPPUnit::GetLiteralCall).  [lex.ext]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::LiteralCall(CCPPSyntax* pcLiteral)
{
	SCPPLiteralCall*	psCall;
	CCPPFunction*		pcFunction;

	psCall = mpcUnit->GetDesugarings()->GetLiteralCall(pcLiteral);
	if (!psCall)
	{
		Error(pcLiteral, "No literal operator takes the literal");
		return NULL;
	}
	pcFunction = psCall->pcFunction;
	if (!Arguments(&pcFunction->mapsParameters, &pcFunction->mapcDefaults, pcFunction->mbVariadic, &psCall->pcArguments->macpElements, pcLiteral, pcFunction->mpsReturn))
	{
		return NULL;
	}
	return pcFunction->mpsReturn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitIdExpression(CCPPSyntaxIdExpression* pcNode)
{
	mpsResult = IdExpression(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitParenthesisedExpression(CCPPSyntaxParenthesisedExpression* pcNode)
{
	mpsResult = Result(pcNode->mpcExpression);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitUnaryExpression(CCPPSyntaxUnaryExpression* pcNode)
{
	mpsResult = UnaryExpression(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitBinaryExpression(CCPPSyntaxBinaryExpression* pcNode)
{
	mpsResult = BinaryExpression(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitConditionalExpression(CCPPSyntaxConditionalExpression* pcNode)
{
	mpsResult = ConditionalExpression(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitCallExpression(CCPPSyntaxCallExpression* pcNode)
{
	mpsResult = CallExpression(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitMemberExpression(CCPPSyntaxMemberExpression* pcNode)
{
	CCPPSymbol*	pcEnumerator;

	//object.Enumerator is the enumerator, the object worked out first; anything else is a field or a static member.
	pcEnumerator = mpcUnit->GetBinder()->MemberEnumerator(pcNode);
	if (pcEnumerator)
	{
		mpsResult = Value(pcNode->mpcObject) ? mpcUnit->EnumeratorType(pcEnumerator) : NULL;
		return;
	}
	mpsResult = Load(Place(pcNode), pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitThisExpression(CCPPSyntaxThisExpression* pcNode)
{
	mpsResult = ThisExpression(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// A lambda: an object of its closure's class.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitLambdaExpression(CCPPSyntaxLambdaExpression* pcNode)
{
	if (mpcUnit->IsInUnevaluatedOperand(pcNode))
	{
		//[expr.prim.lambda] 2
		Error(pcNode, "A lambda can't be written in the operand of sizeof, decltype, noexcept or typeid");
		mpsResult = NULL;
		return;
	}
	if (((pcNode->meDefault != CPPLC_None) || (pcNode->macpCaptures.NumElements() > 0)) && IsInDefaultArgument(pcNode))
	{
		//[expr.prim.lambda] 13
		Error(pcNode, "A lambda in a default argument captures nothing");
		mpsResult = NULL;
		return;
	}
	mpsResult = mpcUnit->GetClosures()->LambdaType(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a node is in a parameter's default argument.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::IsInDefaultArgument(CCPPSyntax* pcNode)
{
	CCPPSyntax*	pcChild;
	CCPPSyntax*	pcParent;

	for (pcChild = pcNode, pcParent = pcNode->mpcParent; pcParent; pcChild = pcParent, pcParent = pcParent->mpcParent)
	{
		if ((pcParent->GetKind() == CPPS_Parameter) && (((CCPPSyntaxParameter*)pcParent)->mpcDefault == pcChild))
		{
			return true;
		}
		if ((pcParent->GetKind() == CPPS_FunctionDefinition) || (pcParent->GetKind() == CPPS_CompoundStatement))
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitCastExpression(CCPPSyntaxCastExpression* pcNode)
{
	mpsResult = CastExpression(pcNode->mpcType, pcNode->mpcOperand, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitNamedCastExpression(CCPPSyntaxNamedCastExpression* pcNode)
{
	SCPP65Type*	psResult;

	if (pcNode->meCast == CT_dynamic_cast)
	{
		//Its pointer or reference type (CCPP65IRFrontEnd::DynamicCastExpression checks what the operand is).
		psResult = mpcUnit->GetDeclarators()->GetTypeIdType(pcNode->mpcType);
		mpsResult = (psResult && DynamicCastAllowed(pcNode, psResult)) ? psResult : NULL;
		return;
	}
	//(The check types the operand again, which leaves its type in mpsResult.)
	psResult = CastExpression(pcNode->mpcType, pcNode->mpcOperand, pcNode);
	mpsResult = (psResult && NamedCastAllowed(pcNode, psResult)) ? psResult : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitConstructExpression(CCPPSyntaxConstructExpression* pcNode)
{
	mpsResult = ConstructExpression(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitSizeofExpression(CCPPSyntaxSizeofExpression* pcNode)
{
	mpsResult = SizeofExpression(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitImplicitArgument(CCPPSyntaxImplicitArgument* pcNode)
{
	mpsResult = pcNode->mpsType;
}


//////////////////////////////////////////////////////////////////////////
//
// noexcept(expression): a bool, the expression checked but not worked out.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitNoexceptExpression(CCPPSyntaxNoexceptExpression* pcNode)
{
	SCPP65Type*	psOperand;

	mpcUnit->GetContext()->EnterUnevaluated();
	psOperand = Value(pcNode->mpcOperand);
	mpcUnit->GetContext()->LeaveUnevaluated();
	mpsResult = psOperand ? &mpcTypes->msBool : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// throw, and throw with an operand.  A program built without exceptions
// (CCPPCompileOptions::mbExceptions) can't have one; with them it is void
// ([expr.throw] 1) and what is thrown must be something that can be
// (CCPPUnit::CheckThrown).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitThrowExpression(CCPPSyntaxThrowExpression* pcNode)
{
	if (mpcUnit->GetOptions()->mbExceptions)
	{
		mpsResult = (!pcNode->mpcOperand || (Value(pcNode->mpcOperand) && mpcUnit->GetExceptions()->CheckThrown(pcNode))) ? &mpcTypes->msVoid : NULL;
	}
	else
	{
		Error(pcNode, CPP_EXCEPTIONS_OFF_THROW);
		mpsResult = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// typeid(type) and typeid(expression): a const std::type_info&, of
// <typeinfo>'s std::type_info.  [expr.typeid] 1
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitTypeidExpression(CCPPSyntaxTypeidExpression* pcNode)
{
	CCPPClass*	pcTypeInfo;
	SCPP65Type*	psOf;
	bool		bDynamic;

	mpsResult = NULL;
	pcTypeInfo = TypeInfoClass();
	if (!pcTypeInfo)
	{
		Error(pcNode, "typeid needs std::type_info, from <typeinfo>");
		return;
	}
	if (pcNode->mpcOperand && !Value(pcNode->mpcOperand))
	{
		return;
	}
	psOf = TypeidType(pcNode, &bDynamic);
	if (!psOf)
	{
		Error(pcNode, "typeid of this is not supported by the compiler yet");
		return;
	}
	if (psOf->WithoutReference()->IsClass() && !mpcUnit->CompleteClass(psOf->WithoutReference()->pcClass, pcNode))
	{
		//[expr.typeid] 4
		return;
	}
	mpsResult = mpcTypes->Reference(mpcTypes->Const(pcTypeInfo->mpsType));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitTypeTraitExpression(CCPPSyntaxTypeTraitExpression* pcNode)
{
	mpsResult = SizeofExpression(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitNewExpression(CCPPSyntaxNewExpression* pcNode)
{
	mpsResult = NewExpression(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitDeleteExpression(CCPPSyntaxDeleteExpression* pcNode)
{
	mpsResult = DeleteExpression(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the value read from a place of type psPlace (NULL after an error): an
// array is a pointer to its first element, a function a pointer to it and an object
// its address (so its type is known; using it as a number fails in Convert).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::Load(SCPP65Type* psPlace, CCPPSyntax* pcNode)
{
	if (!psPlace)
	{
		return NULL;
	}
	if (psPlace->IsArray())
	{
		return mpcTypes->Pointer(psPlace->psBase);
	}
	if (psPlace->IsClass())
	{
		return psPlace;
	}
	if (psPlace->IsFunction())
	{
		return mpcTypes->Pointer(psPlace);
	}
	if (!psPlace->IsScalar())
	{
		Error(pcNode, "This type of value is not supported by the compiler yet");
		return NULL;
	}
	return mpcUnit->Supported(psPlace, pcNode) ? psPlace : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A literal (a number, a character, true, false or nullptr) or an enumerator: its
// type, once its value is known to be a constant.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ConstantValue(CCPPSyntax* pcExpression)
{
	SCPP65Type*		psType;
	CCPPSymbol*		pcSymbol;
	CCPPClass*	pcClass;
	int64			lliValue;
	double			dValue;
	size			uiErrors;

	pcExpression = mpcUnit->SkipParentheses(pcExpression);
	if (pcExpression->GetKind() == CPPS_IdExpression)
	{
		pcSymbol = ((CCPPSyntaxIdExpression*)pcExpression)->mpcName->mpcSymbol;
		if (!pcSymbol)
		{
			pcSymbol = mpcUnit->QualifiedEnumerator(((CCPPSyntaxIdExpression*)pcExpression)->mpcName, pcExpression, &pcClass);
		}
		psType = mpcUnit->EnumeratorType(pcSymbol);
	}
	else
	{
		psType = mpcUnit->Literal((CCPPSyntaxLiteralExpression*)pcExpression);
		if (!psType)
		{
			return NULL;
		}
		if (psType->IsFloat() && mpcUnit->GetFolder()->FloatConstant(pcExpression, &dValue))
		{
			return psType;
		}
	}
	if (!mpcUnit->GetFolder()->Constant(pcExpression, &lliValue))
	{
		//A class its name is qualified by that couldn't be made (Byte<300>::V) says why itself, asked for here.
		uiErrors = mpcDiagnostics->GetErrors()->NumErrors();
		if ((pcExpression->GetKind() == CPPS_IdExpression) && ((CCPPSyntaxIdExpression*)pcExpression)->mpcName->IsQualifiedByClass())
		{
			mpcUnit->QualifiersClass(((CCPPSyntaxIdExpression*)pcExpression)->mpcName, pcExpression);
		}
		if (mpcDiagnostics->GetErrors()->NumErrors() == uiErrors)
		{
			mpcUnit->GetFolder()->ConstantError(pcExpression, "The value is not a constant");
		}
		return NULL;
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// A string literal's value is a pointer to its first character, which is
// const as all of them are.  [lex.string] 8
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::StringLiteral(CCPPSyntaxStringExpression* pcString)
{
	CArrayUint8		auiBytes;
	bool			bDecoded;

	auiBytes.Init();
	bDecoded = mpcUnit->GetStrings()->DecodeString(pcString, &auiBytes);
	auiBytes.Kill();
	return bDecoded ? mpcTypes->Pointer(mpcTypes->Const(mpcUnit->GetStrings()->StringElementType(pcString))) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A name: the enclosing function's name (__func__ ...), an enumerator, a non-type
// template parameter (its argument, at the parameter's type; a pointer or a
// reference parameter's argument expression), a function or a
// function template (their address), a static const member initialised in its
// class (its value), or else a variable or a field.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::IdExpression(CCPPSyntaxIdExpression* pcId)
{
	CCPPBoundName			cBound;
	CCPPSyntaxParameter*	pcParameter;
	SCPP65Type*				psType;
	int64					lliValue;

	if (mpcUnit->IsAmbiguousName(pcId->mpcName))
	{
		mpcDiagnostics->ReportOnce(pcId, "The name is ambiguous: it finds more than one thing");
		return NULL;
	}
	mpcUnit->GetBinder()->BindName(pcId, &cBound);
	mpcUnit->WarnDeprecatedSymbol(cBound.mpcSymbol, pcId);
	switch (cBound.meWhat)
	{
	case CPPBN_FunctionName:
		return mpcTypes->Pointer(&mpcTypes->msChar);
	case CPPBN_EnclosingConstant:
		//A constant of the function the lambda is in, which it doesn't capture: its value.  [basic.def.odr] 3
		return mpcUnit->GetClosures()->EnclosingConstantType(cBound.mpcSymbol);
	case CPPBN_Constant:
		if (cBound.mpcSymbol->meKind == CPPSK_Enumerator)
		{
			return ConstantValue(pcId);
		}
		if (!mpcUnit->GetInstantiator()->FindBinding(cBound.mpcSymbol))
		{
			Error(pcId, "A template parameter used outside an instantiation");
			return NULL;
		}
		pcParameter = (CCPPSyntaxParameter*)cBound.mpcSymbol->mpcDeclaration;
		psType = (pcParameter && (pcParameter->GetKind() == CPPS_Parameter)) ? mpcUnit->GetDeclarators()->GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter) : NULL;
		return (psType && (psType->IsInteger() || psType->IsNullPointer())) ? psType : &mpcTypes->msInt;
	case CPPBN_Function:
		return FunctionAddress(pcId, cBound.mpcSymbol->mlliName, NULL);
	case CPPBN_FunctionTemplate:
		return FunctionTemplateAddress(pcId);
	case CPPBN_TemplateArgument:
		//nullptr for a pointer parameter is a null pointer of its type.
		psType = Value(cBound.mpcArgument);
		return (psType && psType->IsNullPointer()) ? mpcUnit->GetInstantiator()->ValueParameterType(cBound.mpcSymbol, psType) : psType;
	case CPPBN_Variable:
		//SIZE::Max's value is the constant its class gives it: it needs no definition.  [class.static.data] 4
		if (mpcUnit->IsConstantMember(cBound.mpcVariable) && (cBound.mpcVariable->mpsType->IsFloat() || mpcUnit->GetFolder()->Constant(pcId, &lliValue)))
		{
			return Load(cBound.mpcVariable->mpsType, pcId);
		}
		return Load(Place(pcId), pcId);
	default:
		return Load(Place(pcId), pcId);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A function's name used as a value is its address.  An overloaded name needs the
// type of the function wanted (psTarget, from a cast to a pointer to a function).
// Class::f and, in a member function, f may be a static member function (of the
// class or its bases); an unqualified name is looked for in the class first.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::FunctionAddress(CCPPSyntax* pcNode, int64 lliName, SCPP65Type* psTarget)
{
	CCPPFunction*		pcFound;
	CCPPSyntaxName*		pcName;
	bool				bOverloaded;

	pcName = (pcNode && (pcNode->GetKind() == CPPS_IdExpression)) ? ((CCPPSyntaxIdExpression*)pcNode)->mpcName : NULL;
	pcFound = mpcUnit->GetBinder()->FindAddressedFunction(pcName, lliName, psTarget, pcNode, &bOverloaded);
	if (bOverloaded)
	{
		Error(pcNode, "The address of an overloaded function needs the type of the function it is for");
		return NULL;
	}
	if (!pcFound)
	{
		Error(pcNode, "The function has not been declared");
		return NULL;
	}
	return mpcTypes->Pointer(mpcUnit->GetFunctionDeclarer()->FunctionType(pcFound));
}


//////////////////////////////////////////////////////////////////////////
//
// The address of a function template's instance for explicit arguments:
// &ComparePrimitive<int> or ComparePrimitive<int>.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::FunctionTemplateAddress(CCPPSyntaxIdExpression* pcId)
{
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPTemplate*				pcTemplate;
	CCPPFunction*				pcFunction;

	pcComponent = pcId->mpcName->GetLast();
	pcTemplate = mpcUnit->GetInstantiator()->GetTemplate(pcId->mpcName->mpcSymbol);
	if (!pcTemplate || !pcComponent->mbTemplateArguments)
	{
		Error(pcId, "The address of a function template needs its template arguments in the compiler yet");
		return NULL;
	}
	pcFunction = mpcUnit->GetInstantiator()->InstantiateFunction(pcTemplate, &pcComponent->macpTemplateArguments, NULL, pcId);
	if (!pcFunction)
	{
		return NULL;
	}
	return mpcTypes->Pointer(mpcUnit->GetFunctionDeclarer()->FunctionType(pcFunction));
}


//////////////////////////////////////////////////////////////////////////
//
// &Class::member: a pointer to a member function (the one of psTarget's
// type if it is overloaded) or to a field, its type in *ppsType (NULL after
// an error).  A member of a base is a pointer to a member of the base.
// Returns false if the name is not a non-static member of the class or its
// bases.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::MemberAddress(CCPPSyntaxIdExpression* pcId, SCPP65Type* psTarget, SCPP65Type** ppsType)
{
	CCPPSyntaxName*		pcName;
	CCPPClass*			pcClass;
	CCPPClass*			pcFoundClass;
	CCPPFunction*		pcFunction;
	SCPPField*			psField;
	int64				lliName;
	int					iOffset;
	bool				bOverloaded;

	pcName = pcId->mpcName;
	mpcDiagnostics->BeginQuiet();
	pcClass = mpcUnit->NameQualifierClass(pcName, pcId);
	mpcDiagnostics->EndQuiet();
	lliName = mpcUnit->GetFunctionName(pcName->GetLast());
	*ppsType = NULL;
	if (!pcClass || (lliName == -1))
	{
		return false;
	}
	if (!mpcUnit->CompleteClass(pcClass, pcId))
	{
		return true;
	}
	pcFunction = mpcUnit->GetBinder()->FindMemberFunction(pcClass, lliName, psTarget, &pcFoundClass, &bOverloaded);
	if (bOverloaded)
	{
		Error(pcId, "The address of an overloaded function needs the type of the function it is for");
		return true;
	}
	if (pcFunction)
	{
		*ppsType = mpcTypes->MemberPointer(pcFoundClass, mpcUnit->GetFunctionDeclarer()->FunctionType(pcFunction));
		return true;
	}

	psField = pcClass->FindField(lliName, &iOffset);
	if (!psField)
	{
		return false;
	}
	if (psField->pcSymbol && !mpcUnit->GetBinder()->ReachesMember(psField->pcSymbol, pcClass, pcId))
	{
		//&B::m is named as a member of B.  [expr.unary.op] 3, [class.protected] 1
		return true;
	}
	*ppsType = mpcTypes->MemberPointer(pcClass, psField->psType);
	return true;
}

//////////////////////////////////////////////////////////////////////////
//
// this, in a member function: a pointer to its class, const in a const one.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ThisExpression(CCPPSyntax* pcThis)
{
	CCPPClass*	pcClass;

	pcClass = mpcUnit->GetContext()->HasThis() ? mpcUnit->GetClosures()->ThisClass() : NULL;
	if (!pcClass)
	{
		Error(pcThis, mpcUnit->GetClosures()->WritingLambda() ? "The lambda doesn't capture this" : "this is only in member functions");
		return NULL;
	}
	return mpcUnit->ThisType(pcClass);
}


//////////////////////////////////////////////////////////////////////////
//
// ++ -- ! * & - + ~.  An object's own operator is called for &, -, + and ~ (and
// for ! in Condition).  -1 and ~0 are constants of the promoted operand's type.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::UnaryExpression(CCPPSyntaxUnaryExpression* pcUnary)
{
	SCPP65Type*				psType;
	CCPPSyntax*				pcOperand;
	CCPPSyntaxIdExpression*	pcId;
	CCPPSymbol*				pcSymbol;
	CCPPBoundCall			cBound;
	int64					lliValue;

	switch (pcUnary->meOperator)
	{
	case CT_Increment:
	case CT_Decrement:
		return IncrementExpression(pcUnary);
	case CT_LogicalNot:
		if (mpcUnit->GetBinder()->IsOperatorOperand(pcUnary->mpcOperand, CT_LogicalNot) && mpcUnit->GetBinder()->BindOperator(CT_LogicalNot, pcUnary->mpcOperand, NULL, pcUnary, &cBound, false))
		{
			//!object: its operator! (the object's or one that is no class's), whatever it returns.  [over.match.oper] 2
			return OperatorExpression(CT_LogicalNot, pcUnary->mpcOperand, NULL, pcUnary);
		}
		return ConditionValue(pcUnary);
	case CT_Multiply:
		if (IsObject(pcUnary->mpcOperand))
		{
			//*object: the object's operator.
			return OperatorExpression(CT_Multiply, pcUnary->mpcOperand, NULL, pcUnary);
		}
		return Load(Place(pcUnary), pcUnary);
	case CT_BitwiseAnd:
		pcOperand = mpcUnit->SkipParentheses(pcUnary->mpcOperand);
		mpcUnit->GetCheck()->Changed(pcOperand, CPPPC_ADDRESS);
		if (IsObject(pcOperand) && mpcUnit->HasMember(ObjectClass(pcOperand), mpcUnit->GetOperatorName(CT_BitwiseAnd, false)))
		{
			return OperatorExpression(CT_BitwiseAnd, pcOperand, NULL, pcUnary);
		}
		if (pcOperand->GetKind() == CPPS_IdExpression)
		{
			pcId = (CCPPSyntaxIdExpression*)pcOperand;
			pcSymbol = pcId->mpcName->mpcSymbol;
			if (mpcUnit->MemberPointerName(pcUnary) && MemberAddress(pcId, NULL, &psType))
			{
				return psType;
			}
			if (!mpcUnit->MemberPointerName(pcUnary) && !mpcUnit->GetClosures()->ThisClass() && mpcUnit->QualifiedFieldType(pcId))
			{
				//&(Class::field), which is no pointer to a member, where there is no object for the field to be in.
				Error(pcUnary, "A pointer to a member is &Class::member, with no brackets about the name");
				return NULL;
			}
			if (pcSymbol && (pcSymbol->meKind == CPPSK_FunctionTemplate))
			{
				return FunctionTemplateAddress(pcId);
			}
			if (pcSymbol && (pcSymbol->meKind == CPPSK_Function))
			{
				return FunctionAddress(pcId, pcSymbol->mlliName, NULL);
			}
		}
		psType = Place(pcUnary->mpcOperand);
		if (psType && IsRvalue(pcOperand))
		{
			//[expr.unary.op] 3
			Error(pcUnary, "The address of a temporary can't be taken");
			return NULL;
		}
		if (psType && IsBitField(pcOperand))
		{
			//[expr.unary.op] 4, [class.bit] 3
			BitFieldError(pcUnary);
			return NULL;
		}
		return psType ? mpcTypes->Pointer(psType) : NULL;
	default:
		break;
	}

	if ((pcUnary->meOperator != CT_Subtract) && (pcUnary->meOperator != CT_Add) && (pcUnary->meOperator != CT_BitwiseNot))
	{
		Error(pcUnary, "This operator is not supported by the compiler yet");
		return NULL;
	}
	if (!IsObject(pcUnary->mpcOperand) && mpcUnit->GetBinder()->IsOperatorOperand(pcUnary->mpcOperand, pcUnary->meOperator) && mpcUnit->GetBinder()->BindOperator(pcUnary->meOperator, pcUnary->mpcOperand, NULL, pcUnary, &cBound, false))
	{
		//An enum's operand with an operator function that takes it.
		return OperatorExpression(pcUnary->meOperator, pcUnary->mpcOperand, NULL, pcUnary);
	}
	if (IsObject(pcUnary->mpcOperand))
	{
		if (mpcUnit->GetBinder()->BindOperator(pcUnary->meOperator, pcUnary->mpcOperand, NULL, pcUnary, &cBound, false))
		{
			return OperatorExpression(pcUnary->meOperator, pcUnary->mpcOperand, NULL, pcUnary);
		}
		//No operator function takes the object: the built-in operator, on what the object's conversion function gives.
		psType = BuiltinOperand(pcUnary->mpcOperand, CPPUnaryOperandTypes(pcUnary->meOperator));
	}
	else
	{
		if (mpcUnit->GetFolder()->Constant(pcUnary, &lliValue))
		{
			psType = TypeOf(pcUnary->mpcOperand);
			if (psType && psType->IsInteger() && !psType->IsScopedEnum())
			{
				return mpcTypes->Promote(OperandType(psType, pcUnary->mpcOperand));
			}
		}
		psType = Operand(pcUnary->mpcOperand);
	}
	if (!psType)
	{
		return NULL;
	}
	if ((psType->IsFloat() && ((pcUnary->meOperator == CT_Add) || (pcUnary->meOperator == CT_Subtract))) || (psType->IsPointer() && (pcUnary->meOperator == CT_Add)))
	{
		//-float and +float; +pointer is the pointer.  [expr.unary.op] 7, 8
		return psType;
	}
	if (!psType->IsInteger())
	{
		Error(pcUnary, (pcUnary->meOperator == CT_BitwiseNot) ? "The operand of ~ must be an integer" : "The operand must be a number");
		return NULL;
	}
	if (psType->IsScopedEnum() && !mpcUnit->RecordEnumCast(pcUnary, psType, psType, CPPEC_ARITHMETIC))
	{
		Error(pcUnary, "An enum class has no arithmetic: cast its value to an integer");
		return NULL;
	}
	return mpcTypes->Promote(psType);
}


//////////////////////////////////////////////////////////////////////////
//
// ++x, --x, x++ and x--: the type of x, which must be a number or a pointer.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::IncrementExpression(CCPPSyntaxUnaryExpression* pcUnary)
{
	SCPP65Type*	psType;

	if (IsObject(pcUnary->mpcOperand))
	{
		//++object, --object, and object++ and object-- with an int 0: the object's operator.  [over.inc] 1
		return OperatorExpression(pcUnary->meOperator, pcUnary->mpcOperand, pcUnary->mbPostfix ? mpcUnit->GetDesugarings()->ImplicitZero() : NULL, pcUnary);
	}
	psType = Place(pcUnary->mpcOperand);
	if (!psType || !CanChange(psType, pcUnary))
	{
		return NULL;
	}
	if (!psType->IsScalar() || (psType->IsScopedEnum() && !mpcUnit->RecordEnumCast(pcUnary, psType, psType, CPPEC_INCREMENT)))
	{
		Error(pcUnary, "Only integers and pointers can be incremented");
		return NULL;
	}
	if (psType->pcEnum && !psType->IsScopedEnum() && !mpcUnit->IsCSource())
	{
		//The next value is a number, which an enum isn't given.  [expr.pre.incr] 1, [expr.post.incr] 1
		Error(pcUnary, "An enum can't be incremented: it is given only its own values, or a cast");
		return NULL;
	}
	if ((psType->eKind == CPP65TK_Bool) && (pcUnary->meOperator == CT_Decrement))
	{
		//++b makes it true; there is no --b.  [expr.pre.incr] 2
		Error(pcUnary, "A bool can't be decremented");
		return NULL;
	}
	if (psType->IsPointer() && !CompletePointee(psType, pcUnary))
	{
		return NULL;
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// left op right.  The comparisons and logical operators are bool, an object's
// operator is the operator function's result.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::BinaryExpression(CCPPSyntaxBinaryExpression* pcBinary)
{
	SCPP65Type*		psLeft;
	SCPP65Type*		psRight;

	if (IsCPPAssignment(pcBinary->meOperator))
	{
		if ((pcBinary->meOperator == CT_Assignment) && !mpcUnit->GetExceptions()->CheckAssignedExceptions(pcBinary))
		{
			return NULL;
		}
		return Assignment(pcBinary);
	}
	if (IsCPPComparison(pcBinary->meOperator))
	{
		return ConditionValue(pcBinary);
	}
	switch (pcBinary->meOperator)
	{
	case CT_MemberByPointerFunctionPointerIndirection:
		if (IsObject(pcBinary->mpcLeft))
		{
			return OperatorExpression(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, pcBinary);
		}
		return Load(Place(pcBinary), pcBinary);
	case CT_MemberFunctionPointerIndirection:
		return Load(Place(pcBinary), pcBinary);
	case CT_Comma:
		if (mpcUnit->GetBinder()->IsOverloadedComma(pcBinary))
		{
			return OperatorExpression(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, pcBinary);
		}
		if (!Value(pcBinary->mpcLeft))
		{
			return NULL;
		}
		return Value(pcBinary->mpcRight);
	case CT_LogicalAnd:
	case CT_LogicalOr:
		return ConditionValue(pcBinary);
	default:
		if (HasOperatorFunction(pcBinary))
		{
			return OperatorExpression(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, pcBinary);
		}
		if (!Operands(pcBinary, &psLeft, &psRight))
		{
			return NULL;
		}
		return Operate(pcBinary->meOperator, psLeft, psRight, pcBinary);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// x = y and x op= y: the type of x.  An object's own operator= (or op=) is called if
// it has one; otherwise an object is copied.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::Assignment(CCPPSyntaxBinaryExpression* pcBinary)
{
	SCPP65Type*		psPlace;
	SCPP65Type*		psRight;
	SCPP65Type*		psResult;
	ECppTokens		eOperator;
	CArraySyntax	apcRight;

	psResult = TypeOf(pcBinary->mpcLeft);
	if (psResult && psResult->IsClass() && !mpcUnit->CompleteClass(psResult->pcClass, pcBinary))
	{
		//Its operator= (or its using Base::operator=) is known once it is laid out.
		return NULL;
	}
	if (psResult && psResult->IsClass() && mpcUnit->HasOperator(psResult->pcClass, mpcUnit->GetOperatorName(pcBinary->meOperator, false), 1))
	{
		apcRight.Init();
		apcRight.Add(pcBinary->mpcRight);
		psResult = OperatorCall(pcBinary->mpcLeft, pcBinary->meOperator, &apcRight, pcBinary);
		apcRight.Kill();
		return psResult;
	}

	psPlace = Place(pcBinary->mpcLeft);
	if (!psPlace || !CanChange(psPlace, pcBinary))
	{
		return NULL;
	}
	if (psPlace->IsClass() && (pcBinary->meOperator == CT_Assignment))
	{
		if (psPlace->pcClass->IsAssignmentDeleted())
		{
			//The compiler's own operator= isn't written for it.  [class.copy] 23, [expr.prim.lambda] 20
			Error(pcBinary, "The object cannot be assigned: its class has a const or reference member, or declares a move and no copy");
			return NULL;
		}
		if (mpcUnit->SkipParentheses(pcBinary->mpcRight)->GetKind() == CPPS_InitializerList)
		{
			//x = {1, 2} is x = X{1, 2}: a temporary made from the list.  [expr.ass] 9
			return TemporaryObject(psPlace->Unqualified(), pcBinary->mpcRight, pcBinary) ? psPlace : NULL;
		}
		if (!IsObject(pcBinary->mpcRight) && mpcUnit->GetBinder()->ConvertsByConstructor(psPlace->pcClass, pcBinary->mpcRight))
		{
			//x = 6 is x = X(6): a temporary made by a constructor that isn't explicit.  [over.match.oper] 3, [class.conv.ctor] 1
			return TemporaryObject(psPlace->Unqualified(), pcBinary->mpcRight, pcBinary) ? psPlace : NULL;
		}
		if (!IsObject(pcBinary->mpcRight) && Value(pcBinary->mpcRight))
		{
			Error(pcBinary->mpcRight, "No constructor of the class that isn't explicit makes an object of it from the value assigned");
			return NULL;
		}
		return CopyObject(psPlace->pcClass, pcBinary->mpcRight, pcBinary) ? psPlace : NULL;
	}
	if (psPlace->IsArray() || psPlace->IsFunction())
	{
		//An array or a function is no modifiable lvalue.  [basic.lval] 6, [expr.ass] 1
		Error(pcBinary, psPlace->IsArray() ? "An array can't be assigned: its elements can" : "A function can't be assigned");
		return NULL;
	}
	if (!psPlace->IsScalar())
	{
		Error(pcBinary, "Only integers, pointers and objects can be assigned by the compiler yet");
		return NULL;
	}

	if (pcBinary->meOperator == CT_Assignment)
	{
		psRight = ValueFor(pcBinary->mpcRight, psPlace, false);
		return ImplicitConvert(psRight, psPlace, pcBinary->mpcRight, pcBinary->mpcRight) ? psPlace : NULL;
	}

	//x op= y is x = x op y, and an object for y what it converts to for the operator.  [expr.ass] 7
	eOperator = CPPCompoundOperation(pcBinary->meOperator);
	psRight = IsObject(pcBinary->mpcRight) ? BuiltinOperand(pcBinary->mpcRight, CPPBinaryOperandTypes(eOperator)) : Value(pcBinary->mpcRight);
	if (!psRight)
	{
		return NULL;
	}
	psResult = Operate(eOperator, psPlace, psRight, pcBinary);
	return ImplicitConvert(psResult, psPlace, pcBinary, NULL) ? psPlace : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The types of a binary operator's operands' values, left then right.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::Operands(CCPPSyntax* pcLeft, CCPPSyntax* pcRight, SCPP65Type** ppsLeft, SCPP65Type** ppsRight)
{
	*ppsLeft = Operand(pcLeft);
	if (!*ppsLeft)
	{
		return false;
	}
	*ppsRight = Operand(pcRight);
	return *ppsRight != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a binary operator with an object for an operand is an operator
// function's call (the object's own or a free one).  Where it is none, the
// operator is the built-in one, on what the object converts to
// (BuiltinOperand).  [over.match.oper] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::HasOperatorFunction(CCPPSyntaxBinaryExpression* pcBinary)
{
	CCPPBoundCall	cBound;

	return (mpcUnit->GetBinder()->IsOperatorOperand(pcBinary->mpcLeft, pcBinary->meOperator) || mpcUnit->GetBinder()->IsOperatorOperand(pcBinary->mpcRight, pcBinary->meOperator)) &&
		   mpcUnit->GetBinder()->BindOperator(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, pcBinary, &cBound, false);
}


//////////////////////////////////////////////////////////////////////////
//
// The types of a built-in binary operator's operands' values.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::Operands(CCPPSyntaxBinaryExpression* pcBinary, SCPP65Type** ppsLeft, SCPP65Type** ppsRight)
{
	ECPPContextualTypes	eWanted;

	eWanted = CPPBinaryOperandTypes(pcBinary->meOperator);
	*ppsLeft = BuiltinOperand(pcBinary->mpcLeft, eWanted);
	*ppsRight = *ppsLeft ? BuiltinOperand(pcBinary->mpcRight, eWanted) : NULL;
	return *ppsRight != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the value of a built-in operator's operand: as Operand gives
// it, and for an object what its class's one conversion function to a type
// the operator takes (eWanted) gives, +lambda the pointer to its function
// and Count + 1 an integer.  NULL, with why, for an object with none or with
// more than one.  [over.built]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::BuiltinOperand(CCPPSyntax* pcOperand, ECPPContextualTypes eWanted)
{
	SCPP65Type*	psType;

	if (!IsObject(pcOperand))
	{
		return Operand(pcOperand);
	}
	psType = mpcUnit->GetBinder()->ContextualType(pcOperand, eWanted);
	return psType ? ValueFor(pcOperand, psType, false) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the value of an operand that integral promotion is applied
// to (CCPPUnit::OperandType: a bit-field's is promoted by its width).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::Operand(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = Value(pcExpression);
	return psType ? OperandType(psType, pcExpression) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of left op right for the arithmetic, bitwise and shift operators
// (CCPP65Types::Operation), or NULL with the reason the operands don't fit.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::Operate(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Type* psRight, CCPPSyntax* pcNode)
{
	SCPP65Type*	psType;
	bool		bFloatOperator;

	if (!psLeft || !psRight)
	{
		return NULL;
	}
	if ((psLeft->IsScopedEnum() || psRight->IsScopedEnum()) && !RecordsEnumOperands(pcNode, psLeft, psRight, CPPEC_ARITHMETIC))
	{
		Error(pcNode, "An enum class has no arithmetic: cast its value to an integer");
		return NULL;
	}
	psType = mpcTypes->Operation(eOperator, psLeft, psRight);
	if (psType)
	{
		if ((psLeft->IsPointer() && !CompletePointee(psLeft, pcNode)) || (psRight->IsPointer() && !CompletePointee(psRight, pcNode)))
		{
			return NULL;
		}
		if (psLeft->IsPointer() && psRight->IsPointer() && !psLeft->psBase->Equals(psRight->psBase))
		{
			//[expr.add] 2
			Error(pcNode, "Pointers to different types can't be subtracted");
			return NULL;
		}
		return psType;
	}

	bFloatOperator = (eOperator == CT_Add) || (eOperator == CT_Subtract) || (eOperator == CT_Multiply) || (eOperator == CT_Divide);
	if ((psLeft->IsFloat() || psRight->IsFloat()) && bFloatOperator)
	{
		Error(pcNode, "Only numbers can be used with floating point");
	}
	else if (!(psLeft->IsFloat() || psRight->IsFloat()) && (psLeft->IsPointer() || psRight->IsPointer()))
	{
		Error(pcNode, "This operator does not work with pointers");
	}
	else if (!psLeft->IsInteger() || !psRight->IsInteger())
	{
		Error(pcNode, "The operands must be integers");
	}
	else
	{
		Error(pcNode, "This operator is not supported by the compiler yet");
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether what a pointer points to can be stepped over: pointer arithmetic and ++
// and -- step by its size, so a class (or an array of them) must be laid out,
// and void and a function have no size.  [expr.add] 1, [expr.pre.incr] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::CompletePointee(SCPP65Type* psPointer, CCPPSyntax* pcNode)
{
	SCPP65Type*	psPointee;

	psPointee = psPointer->psBase;
	while (psPointee->IsArray())
	{
		psPointee = psPointee->psBase;
	}
	if (psPointee->IsVoid())
	{
		return Error(pcNode, "A pointer to void doesn't step: void has no size");
	}
	if (psPointee->IsFunction())
	{
		return Error(pcNode, "A pointer to a function doesn't step: a function has no size");
	}
	return !psPointee->IsClass() || mpcUnit->CompleteClass(psPointee->pcClass, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcBinary's operands, a psLeft and a psRight, can be compared:
// numbers, and pointers, or for equality pointers to members, that have a
// composite pointer type (with each other, or for equality with 0 or
// nullptr: no other number, and a null pointer is before or after nothing).
// [expr.rel] 2, [expr.eq] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::Compare(CCPPSyntaxBinaryExpression* pcBinary, SCPP65Type* psLeft, SCPP65Type* psRight)
{
	CCPPSyntax*	pcNode;
	ECppTokens	eOperator;
	bool		bEquality;
	bool		bLeftPointer;
	bool		bRightPointer;

	pcNode = pcBinary;
	eOperator = pcBinary->meOperator;
	bEquality = (eOperator == CT_EqualTo) || (eOperator == CT_NotEqualTo);
	bLeftPointer = psLeft->IsPointer() || psLeft->IsMemberPointer();
	bRightPointer = psRight->IsPointer() || psRight->IsMemberPointer();
	if ((bLeftPointer && psRight->IsInteger()) || (bRightPointer && psLeft->IsInteger()))
	{
		if (!mpcUnit->IsNullPointerConstant(bLeftPointer ? pcBinary->mpcRight : pcBinary->mpcLeft))
		{
			return Error(pcNode, "A pointer is compared with a pointer, 0 or nullptr: not with a number");
		}
	}
	if (!bEquality && (bLeftPointer != bRightPointer) && !psLeft->IsFloat() && !psRight->IsFloat() && !mpcUnit->IsCSource())
	{
		//(C compilers let p > 0 by.)
		return Error(pcNode, "A pointer is compared with a null pointer only for equality");
	}
	if ((psLeft->IsScopedEnum() || psRight->IsScopedEnum()) && (psLeft->Unqualified() != psRight->Unqualified()) && !RecordsEnumOperands(pcNode, psLeft, psRight, CPPEC_COMPARE))
	{
		return Error(pcNode, "An enum class's value is compared only with its own values");
	}
	if (psLeft->IsNullPointer() || psRight->IsNullPointer())
	{
		//A std::nullptr_t is compared with another, with a pointer, and for equality with a pointer to a member: not
		//with a number.  [expr.rel] 4, [expr.eq] 2
		if ((psLeft->IsNullPointer() || psLeft->IsPointer()) && (psRight->IsNullPointer() || psRight->IsPointer()))
		{
			return true;
		}
		if ((psLeft->IsMemberPointer() || psRight->IsMemberPointer()) && ((eOperator == CT_EqualTo) || (eOperator == CT_NotEqualTo)))
		{
			return true;
		}
		return Error(pcNode, "nullptr is compared only with a pointer, a pointer to a member or another nullptr");
	}
	if (psLeft->IsFloat() || psRight->IsFloat())
	{
		if (psLeft->IsPointer() || psRight->IsPointer() || !psLeft->IsScalar() || !psRight->IsScalar())
		{
			return Error(pcNode, "Only numbers can be used with floating point");
		}
		return true;
	}
	if (psLeft->IsInteger() && psRight->IsInteger())
	{
		return true;
	}
	if ((psLeft->IsMemberPointer() || psRight->IsMemberPointer()) && (eOperator != CT_EqualTo) && (eOperator != CT_NotEqualTo))
	{
		return Error(pcNode, "Pointers to members are only compared for equality");
	}
	if ((psLeft->IsPointer() || psLeft->IsMemberPointer()) && (psRight->IsPointer() || psRight->IsMemberPointer()) && !CompositePointerType(psLeft, psRight))
	{
		//int* with long*, a pointer to a function with one to an object, pointers to members of unrelated classes.
		return Error(pcNode, "These pointers cannot be compared: neither's type converts to the other's");
	}
	if (!CompositePointerType(psLeft, psRight))
	{
		return Error(pcNode, "These operands cannot be compared");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// c ? x : y.  With an object for x or y it is ObjectConditional's; two values
// about to go of one type are one (CCPPUnit::ConditionalXvalueType); scalars are
// the type CCPPUnit::ConditionalType gives.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ConditionalExpression(CCPPSyntaxConditionalExpression* pcConditional)
{
	SCPP65Type*		psTrue;
	SCPP65Type*		psFalse;
	SCPP65Type*		psType;
	bool			bThrowTrue;
	bool			bThrowFalse;

	psTrue = TypeOf(pcConditional->mpcTrue);
	psFalse = TypeOf(pcConditional->mpcFalse);
	if (!psTrue || !psFalse)
	{
		//TypeOf says nothing of what is wrong: the arm that can't be typed is typed again here, for its error.
		if (!psTrue)
		{
			Value(pcConditional->mpcTrue);
		}
		if (!psFalse)
		{
			Value(pcConditional->mpcFalse);
		}
		return NULL;
	}
	bThrowTrue = pcConditional->mpcTrue->WithoutParentheses()->GetKind() == CPPS_ThrowExpression;
	bThrowFalse = pcConditional->mpcFalse->WithoutParentheses()->GetKind() == CPPS_ThrowExpression;
	if (bThrowTrue || bThrowFalse)
	{
		//With a throw for one operand the result is the other's, as a value; with two it is void.  [expr.cond] 2
		if (!Condition(pcConditional->mpcCondition))
		{
			return NULL;
		}
		if (bThrowTrue && bThrowFalse)
		{
			return &mpcTypes->msVoid;
		}
		psType = (bThrowTrue ? psFalse : psTrue)->WithoutReference()->Unqualified();
		if (psType->IsArray())
		{
			psType = mpcTypes->Pointer(psType->psBase);
		}
		return Value(bThrowTrue ? pcConditional->mpcFalse : pcConditional->mpcTrue) ? psType : NULL;
	}
	psType = ConditionalXvalueType(pcConditional);
	if (psType)
	{
		return (Condition(pcConditional->mpcCondition) && Value(pcConditional->mpcTrue) && Value(pcConditional->mpcFalse)) ? psType : NULL;
	}
	if (IsObject(pcConditional->mpcTrue) || IsObject(pcConditional->mpcFalse))
	{
		return ObjectConditional(pcConditional);
	}

	psType = ConditionalType(psTrue, psFalse);
	if (!psType || !ConditionalResultsFit(pcConditional, psTrue, psFalse))
	{
		Error(pcConditional, "The two results of ? : have no type in common");
		return NULL;
	}
	if (!Condition(pcConditional->mpcCondition) ||
		!Convert(Value(pcConditional->mpcTrue), psType, pcConditional->mpcTrue) ||
		!Convert(Value(pcConditional->mpcFalse), psType, pcConditional->mpcFalse))
	{
		return NULL;
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether c ? x : y's results, a psTrue and a psFalse that have a type in
// common (CCPPUnit::ConditionalType, which sees only their types), are
// what the type is for: one that is void goes with another, or with a
// throw; and a number with a pointer is 0.  [expr.cond] 2, 6
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::ConditionalResultsFit(CCPPSyntaxConditionalExpression* pcConditional, SCPP65Type* psTrue, SCPP65Type* psFalse)
{
	CCPPSyntax*	pcOther;

	if (psTrue->IsVoid() != psFalse->IsVoid())
	{
		pcOther = mpcUnit->SkipParentheses(psTrue->IsVoid() ? pcConditional->mpcTrue : pcConditional->mpcFalse);
		return pcOther->GetKind() == CPPS_ThrowExpression;
	}
	if ((psTrue->IsPointer() || psTrue->IsMemberPointer()) && psFalse->IsInteger())
	{
		return mpcUnit->IsNullPointerConstant(pcConditional->mpcFalse);
	}
	if ((psFalse->IsPointer() || psFalse->IsMemberPointer()) && psTrue->IsInteger())
	{
		return mpcUnit->IsNullPointerConstant(pcConditional->mpcTrue);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// c ? x : y as a place: x and y are lvalues of one type, or objects of a
// class and of one derived from it (CCPPUnit::ConditionalLvalueType, which
// gives the place's type).  NULL, with why, if they aren't.  [expr.cond] 3, 4
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ConditionalPlace(CCPPSyntaxConditionalExpression* pcConditional)
{
	SCPP65Type*	psType;

	psType = ConditionalLvalueType(pcConditional);
	if (!psType)
	{
		Error(pcConditional, "This is not an lvalue: the two results of ? : are not lvalues of one type");
		return NULL;
	}
	if (!Condition(pcConditional->mpcCondition) || !Place(pcConditional->mpcTrue) || !Place(pcConditional->mpcFalse))
	{
		return NULL;
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// c ? x : y where x or y is an object.  Two objects that are where they are
// give the one picked, itself (ConditionalPlace).  Otherwise it is a value of
// the type CCPPUnit::ConditionalObjectType gives: an object of the class made
// from the operand the condition picks, or what the object picked converts
// to.  [expr.cond] 3-6
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ObjectConditional(CCPPSyntaxConditionalExpression* pcConditional)
{
	SCPP65Type*		psType;
	bool			bAmbiguous;

	if (ConditionalLvalueType(pcConditional))
	{
		return ConditionalPlace(pcConditional);
	}
	psType = ConditionalObjectType(pcConditional, &bAmbiguous);
	if (!psType)
	{
		Error(pcConditional, bAmbiguous ? "Each of the two results of ? : converts to the other's type" : "The two results of ? : have no type in common");
		return NULL;
	}
	if (!Condition(pcConditional->mpcCondition))
	{
		return NULL;
	}
	if (psType->IsClass())
	{
		return (mpcUnit->CompleteClass(psType->pcClass, pcConditional) &&
				Construct(psType, NULL, pcConditional->mpcTrue, pcConditional->mpcTrue) &&
				Construct(psType, NULL, pcConditional->mpcFalse, pcConditional->mpcFalse)) ? psType : NULL;
	}
	return (Convert(ValueFor(pcConditional->mpcTrue, psType, false), psType, pcConditional->mpcTrue) &&
			Convert(ValueFor(pcConditional->mpcFalse, psType, false), psType, pcConditional->mpcFalse)) ? psType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A condition used as a value (!x, x && y, x < y ...) is a bool.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ConditionValue(CCPPSyntax* pcExpression)
{
	return Condition(pcExpression) ? &mpcTypes->msBool : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an expression can be a condition: !, && and || of conditions, a
// comparison, or a number or a pointer.  An object is a condition through its
// operator! (for !object), its comparison operator, or its conversion to bool.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::Condition(CCPPSyntax* pcExpression)
{
	CCPPBoundCall				cOperatorCall;
	CCPPSyntaxBinaryExpression*	pcBinary;
	CCPPSyntaxUnaryExpression*	pcUnary;
	SCPP65Type*					psLeft;
	SCPP65Type*					psRight;
	CCPPBoundConversion		cBound;

	pcExpression = mpcUnit->SkipParentheses(pcExpression);
	if (!pcExpression)
	{
		return false;
	}

	if (pcExpression->GetKind() == CPPS_UnaryExpression)
	{
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		if ((pcUnary->meOperator == CT_LogicalNot) && mpcUnit->GetBinder()->IsOperatorOperand(pcUnary->mpcOperand, CT_LogicalNot) && mpcUnit->GetBinder()->BindOperator(CT_LogicalNot, pcUnary->mpcOperand, NULL, pcUnary, &cOperatorCall, false))
		{
			return Truth(OperatorExpression(CT_LogicalNot, pcUnary->mpcOperand, NULL, pcUnary), pcUnary);
		}
		if (pcUnary->meOperator == CT_LogicalNot)
		{
			return Condition(pcUnary->mpcOperand);
		}
	}
	else if (pcExpression->GetKind() == CPPS_BinaryExpression)
	{
		pcBinary = (CCPPSyntaxBinaryExpression*)pcExpression;
		if ((pcBinary->meOperator == CT_LogicalAnd) || (pcBinary->meOperator == CT_LogicalOr))
		{
			return Condition(pcBinary->mpcLeft) && Condition(pcBinary->mpcRight);
		}
		if (IsCPPComparison(pcBinary->meOperator))
		{
			if (HasOperatorFunction(pcBinary))
			{
				return Truth(OperatorExpression(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, pcBinary), pcBinary);
			}
			mpcUnit->GetWarner()->ComparisonRange(pcBinary);
			return Operands(pcBinary, &psLeft, &psRight) && Compare(pcBinary, psLeft, psRight);
		}
	}

	//A condition is a conversion to bool that an explicit conversion function may make.
	mpcUnit->GetBinder()->BindConversion(pcExpression, &mpcTypes->msBool, true, &cBound);
	if (cBound.meHow == CPPBV_Function)
	{
		return Truth(ObjectCall(pcExpression, cBound.mpcFunction, NULL, pcExpression), pcExpression);
	}
	return Truth(Value(pcExpression), pcExpression);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a value of psType (NULL after an error) can be true or false: a number or
// a pointer.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::Truth(SCPP65Type* psType, CCPPSyntax* pcNode)
{
	if (!psType)
	{
		return false;
	}
	if (!psType->IsScalar())
	{
		return Error(pcNode, "A condition must be an integer or a pointer");
	}
	if (psType->IsScopedEnum() && !mpcUnit->RecordEnumCast(pcNode, psType, psType, CPPEC_CONDITION))
	{
		return Error(pcNode, "An enum class's value is not a condition: compare it with one of its values");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// f(...): the function's result.  The overload is chosen by the arguments.
// Function templates are instantiated.  Calls to member functions are in
// MemberCall, calls through pointers in CallThroughPointer.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::CallExpression(CCPPSyntaxCallExpression* pcCall)
{
	CCPPBoundCall	cBound;
	SCPP65Type*		psType;
	size			uiErrors;

	if (mpcUnit->GetDesugarings()->ImplicitAssignment(pcCall))
	{
		return Assignment(mpcUnit->GetDesugarings()->ImplicitAssignment(pcCall));
	}
	if (!mpcUnit->GetBinder()->Bind(pcCall, &cBound, true))
	{
		return NULL;
	}
	switch (cBound.meHow)
	{
	case CPPBC_Member:
		return MemberFunctionCall(pcCall, &cBound);
	case CPPBC_Destructor:
		return cBound.mpcFunction ? MemberFunctionCall(pcCall, &cBound) : &mpcTypes->msVoid;
	case CPPBC_PseudoDestructor:
		return Value(cBound.mpcObject) ? &mpcTypes->msVoid : NULL;
	case CPPBC_MemberPointer:
		return MemberPointerCall(pcCall, cBound.mpapcArguments, (CCPPSyntaxBinaryExpression*)cBound.mpcCallee);
	case CPPBC_Function:
		return Call(cBound.mpcFunction, cBound.mpapcArguments, pcCall);
	case CPPBC_InlineMemcpy:
		return InlineMemcpy(cBound.mpapcArguments);
	case CPPBC_Builtin:
		return Builtin(pcCall, cBound.mpapcArguments, cBound.mlliName);
	case CPPBC_Unchecked:
		return UncheckedCall(cBound.mpapcArguments);
	case CPPBC_Surrogate:
		//object(): its conversion function's pointer called.  [over.call.object] 2
		psType = CCPPOverloads::SurrogateType(cBound.mpcFunction);
		if (!ObjectCall(cBound.mpcObject, cBound.mpcFunction, NULL, pcCall) || !Arguments(psType->papsParameters, NULL, psType->bVariadic, cBound.mpapcArguments, pcCall, psType->psBase))
		{
			return NULL;
		}
		return psType->psBase;
	case CPPBC_Pointer:
		psType = CalleePointerType(cBound.mpcCallee);
		if (psType)
		{
			return CallThroughPointer(pcCall, cBound.mpapcArguments, psType->psBase);
		}
		uiErrors = mpcDiagnostics->GetErrors()->NumErrors();
		psType = (cBound.mpcCallee->GetKind() == CPPS_IdExpression) && !((CCPPSyntaxIdExpression*)cBound.mpcCallee)->mpcName->mpcSymbol ? NULL : Value(cBound.mpcCallee);
		if (mpcDiagnostics->GetErrors()->NumErrors() > uiErrors)
		{
			//What is wrong with the callee itself.
			return NULL;
		}
		if (psType)
		{
			//[expr.call] 1
			Error(cBound.mpcCallee, "What is called is no function, pointer to a function or object with an operator()");
			return NULL;
		}
		if ((cBound.mpcCallee->GetKind() == CPPS_IdExpression) && !((CCPPSyntaxIdExpression*)cBound.mpcCallee)->mpcName->mpcSymbol)
		{
			//No function of the name is declared where the call is, nor in its arguments' namespaces.
			Error(cBound.mpcCallee, "The function has not been declared, where the call is or in its arguments' namespaces");
			return NULL;
		}
		Error(cBound.mpcCallee, "Only calls to functions and pointers to functions are supported by the compiler yet");
		return NULL;
	default:
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A call to a function the checker knows nothing of (the host compiler's
// intrinsics): its arguments are typed, and it gives nothing.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::UncheckedCall(CArraySyntax* papcArguments)
{
	size	i;
	bool	bResult;

	bResult = true;
	for (i = 0; i < papcArguments->NumElements(); i++)
	{
		bResult &= Value(papcArguments->GetPtr(i)) != NULL;
	}
	return bResult ? &mpcTypes->msVoid : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A call through a pointer to a function of psFunctionType.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::CallThroughPointer(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, SCPP65Type* psFunctionType)
{
	if (!Arguments(psFunctionType->papsParameters, NULL, psFunctionType->bVariadic, papcArguments, pcCall, psFunctionType->psBase) ||
		!Value(pcCall->mpcCallee))
	{
		return NULL;
	}
	return psFunctionType->psBase;
}


//////////////////////////////////////////////////////////////////////////
//
// A call to pcFunction with papcArguments (NULL for none): its result.  An
// instance of a member template that is only declared is called only where
// the call isn't evaluated; a function template's may be called, as its
// definition may follow (CCPPUnit::DefineFunctionTemplate) or be another
// unit's.  [temp.point] 8, [temp] 6
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::Call(CCPPFunction* pcFunction, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	if (pcFunction->miDeprecated != -1)
	{
		//[[deprecated]]: it can still be called, and the program is told.  [dcl.attr.deprecated] 4
		mpcUnit->WarnDeprecated(pcNode, "function", pcFunction->miDeprecated);
	}
	if (pcFunction->mpcTemplate && !pcFunction->mpcDefinition && pcFunction->mpcTemplate->mbMemberTemplate && !mpcUnit->GetContext()->IsUnevaluated())
	{
		//A template only declared can be named in sizeof and decltype, not called.  [basic.def.odr] 4
		Error(pcNode, "The member template has not been defined");
		return NULL;
	}
	if (!Arguments(&pcFunction->mapsParameters, &pcFunction->mapcDefaults, pcFunction->mbVariadic, papcArguments, pcNode, pcFunction->mpsReturn, pcFunction->mpcBindings))
	{
		return NULL;
	}
	if (!mpcUnit->GetFunctionDeclarer()->CheckAbstractSignature(pcFunction, pcNode))
	{
		return NULL;
	}
	return pcFunction->mpsReturn;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a call's arguments fit its parameters (papcDefaults are the default
// arguments, NULL for none), last first.  A reference is bound to a place, an object
// is constructed from its argument, anything else is converted to its parameter.
// The arguments for ... are promoted (a float to a double).  A function that
// returns an object makes it in a temporary, so its class must be complete,
// but where the call is decltype's operand, which makes none.  [expr.call] 11
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::Arguments(CArrayCPP65Types* papsParameters, CArraySyntax* papcDefaults, bool bVariadic, CArraySyntax* papcArguments, CCPPSyntax* pcNode, SCPP65Type* psReturn, CCPPTemplateArguments* pcDefaultBindings)
{
	size			uiArguments;
	size			uiParameters;
	size			uiGiven;
	size			i;
	SCPP65Type*		psType;
	SCPP65Type*		psParameter;
	CCPPSyntax*		pcArgument;
	bool			bResult;

	uiArguments = papcArguments ? papcArguments->NumElements() : 0;
	uiParameters = papsParameters->NumElements();
	if ((uiArguments > uiParameters) && !bVariadic)
	{
		return Error(pcNode, "The number of arguments does not match the function");
	}

	for (i = uiArguments; i > uiParameters; i--)
	{
		pcArgument = papcArguments->GetPtr(i - 1);
		psType = Operand(pcArgument);
		if (!psType)
		{
			return false;
		}
		psParameter = VariadicArgumentType(psType);
		if (!psParameter)
		{
			if (psType->IsClass() && !psType->pcClass->IsComplete())
			{
				//The argument's value is read: its class must be complete.  [conv.lval] 1, [expr.call] 7
				return Error(pcArgument, "An object of a class that isn't defined yet can't be passed");
			}
			return Error(pcArgument, psType->IsClass() ? "Objects cannot be passed by value by the compiler yet; pass a pointer or a reference" : "This variable argument is not supported by the compiler yet");
		}
		if (!Convert(psType, psParameter, pcArgument))
		{
			return false;
		}
	}

	//The parameters given none take their default arguments, the last first.  Those are written where the function is
	//declared: a template's instance's with its template arguments (T b = T(6)).  [dcl.fct.default] 5
	uiGiven = (uiArguments < uiParameters) ? uiArguments : uiParameters;
	mpcUnit->GetContext()->EnterBindings(pcDefaultBindings ? pcDefaultBindings : mpcUnit->GetContext()->GetBindings());
	bResult = ParameterArguments(papsParameters, papcDefaults, uiParameters, uiGiven, pcNode);
	mpcUnit->GetContext()->LeaveBindings();
	if (!bResult || !ParameterArguments(papsParameters, papcArguments, uiGiven, 0, pcNode))
	{
		return false;
	}
	return !(psReturn && psReturn->IsClass()) || mpcUnit->IsDecltypeOperand(pcNode) || mpcUnit->CompleteClass(psReturn->pcClass, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Arguments for the parameters from uiFrom down to the one after uiTo, each
// its parameter's argument in papcArguments (the call's, or the function's
// default arguments).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::ParameterArguments(CArrayCPP65Types* papsParameters, CArraySyntax* papcArguments, size uiFrom, size uiTo, CCPPSyntax* pcNode)
{
	size			i;
	SCPP65Type*		psType;
	SCPP65Type*		psParameter;
	CCPPSyntax*		pcArgument;

	for (i = uiFrom; i > uiTo; i--)
	{
		pcArgument = papcArguments ? papcArguments->GetPtr(i - 1) : NULL;
		if (!pcArgument)
		{
			return Error(pcNode, "The number of arguments does not match the function");
		}
		psParameter = papsParameters->GetPtr(i - 1);
		if (psParameter->IsReference() && mpcUnit->GetBinder()->BindsToTemporary(psParameter, pcArgument))
		{
			//It is checked as an argument to a parameter of the type referred to.
			psParameter = psParameter->psBase->Unqualified();
		}
		if (psParameter->IsReference())
		{
			psType = Place(pcArgument);
			if (!psType || !CanBind(psParameter, psType, pcArgument))
			{
				return false;
			}
			if (!psParameter->psBase->bConst)
			{
				mpcUnit->GetCheck()->Changed(pcArgument, CPPPC_REFERENCE);
			}
			continue;
		}
		if (psParameter->IsClass())
		{
			if (!mpcUnit->CompleteClass(psParameter->pcClass, pcArgument) || !Construct(psParameter, NULL, pcArgument, pcArgument))
			{
				return false;
			}
			continue;
		}

		psType = ValueFor(pcArgument, psParameter, false);
		if (psType && psType->IsInteger() && (psParameter->eKind == CPP65TK_Integer) && (psParameter->iBytes < 4) && psType->IsSupported())
		{
			//An integer to a narrow integer parameter keeps its low bytes.
			continue;
		}
		if (!ImplicitConvert(psType, psParameter, pcArgument, pcArgument))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// GCC's builtin functions the compiler does (CCPPUnit::FindBuiltin):
// their argument counts, their arguments' types and their results.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::Builtin(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, int64 lliName)
{
	static const char*		aszArguments[] = { "no arguments", "one argument", "two arguments", "three arguments", "four arguments" };
	const SCPPBuiltin*	psBuiltin;
	SCPP65Type*				psType;
	SCPP65Type*				psVoidPointer;
	CChars					szError;
	double					dValue;

	if (mpcUnit->GetFolder()->FloatBuiltin(pcCall, &dValue, &psType))
	{
		return psType;
	}

	psBuiltin = mpcUnit->GetBuiltins()->FindBuiltin(lliName);
	if (!psBuiltin)
	{
		if (mpcUnit->GetSemanticPass())
		{
			//Only checking: the host compiler's other builtins (__builtin_va_start ...).
			return UncheckedCall(papcArguments);
		}
		Error(pcCall, "This builtin is not supported by the compiler yet");
		return NULL;
	}
	if (psBuiltin->szDecimalModeOnly && !mpcUnit->GetTarget()->HasDecimalMode())
	{
		Error(pcCall, (char*)psBuiltin->szDecimalModeOnly);
		return NULL;
	}
	if ((int)papcArguments->NumElements() != psBuiltin->iArguments)
	{
		szError.Init("The builtin takes ");
		szError.Append(aszArguments[psBuiltin->iArguments]);
		Error(pcCall, szError.Text());
		szError.Kill();
		return NULL;
	}

	psVoidPointer = mpcTypes->Pointer(&mpcTypes->msVoid);
	switch (psBuiltin->eBuiltin)
	{
	case CPPB_Unreachable:
	case CPPB_Pause:
		return &mpcTypes->msVoid;
	case CPPB_BCDAdd:
	case CPPB_BCDSubtract:
		return &mpcTypes->msUnsignedInt;
	case CPPB_Expect:
		psType = Value(papcArguments->GetPtr(0));
		return psType ? mpcTypes->Promote(psType) : NULL;
	case CPPB_Memcpy:
		//Libc65's memcpy (a small constant count is InlineMemcpy, before this).
		if (Convert(Value(papcArguments->GetPtr(0)), psVoidPointer, papcArguments->GetPtr(0)) &&
			Convert(Value(papcArguments->GetPtr(1)), psVoidPointer, papcArguments->GetPtr(1)) &&
			Convert(Value(papcArguments->GetPtr(2)), mpcTypes->Size(), papcArguments->GetPtr(2)))
		{
			return psVoidPointer;
		}
		return NULL;
	case CPPB_Alloca:
		return Convert(Value(papcArguments->GetPtr(0)), &mpcTypes->msUnsignedInt, pcCall) ? psVoidPointer : NULL;
	default:
		//ctz, clz and popcount count the bits of their operand type.
		return Convert(Value(papcArguments->GetPtr(0)), mpcUnit->GetBuiltins()->BuiltinOperandType(psBuiltin), pcCall) ? &mpcTypes->msInt : NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// memcpy(Dest, Source, Count) with a constant Count (IsInlineMemcpy): Dest, a void*.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::InlineMemcpy(CArraySyntax* papcArguments)
{
	SCPP65Type*	psVoidPointer;

	psVoidPointer = mpcTypes->Pointer(&mpcTypes->msVoid);
	if (!Convert(Value(papcArguments->GetPtr(0)), psVoidPointer, papcArguments->GetPtr(0)) ||
		!Convert(Value(papcArguments->GetPtr(1)), psVoidPointer, papcArguments->GetPtr(1)))
	{
		return NULL;
	}
	return psVoidPointer;
}


//////////////////////////////////////////////////////////////////////////
//
// A call to a member function on pcObject (this's when NULL): its arguments, then
// its object (a pointer for ->), and its result.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::MemberFunctionCall(CCPPSyntaxCallExpression* pcCall, CCPPBoundCall* pcBound)
{
	CCPPFunction*	pcFunction;
	bool			bObject;

	pcFunction = pcBound->mpcFunction;
	if (!Call(pcFunction, pcBound->mpapcArguments, pcCall))
	{
		return NULL;
	}
	bObject = true;
	if (!pcFunction->mbStatic && pcBound->mpcObject)
	{
		bObject = pcBound->mbArrow ? (ArrowPointer(pcBound->mpcObject, pcBound->mpcCallee) != NULL) : (Place(pcBound->mpcObject) != NULL);
		if (!pcBound->mbArrow)
		{
			ObjectChanged(pcBound->mpcObject, pcFunction);
		}
	}
	return bObject ? pcFunction->mpsReturn : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// pcFunction is called on pcObject (not through a pointer): unless it is
// a const function, the object can be changed by it, which a kept check
// remembers (CCPPSemanticPass::FindChanges).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::ObjectChanged(CCPPSyntax* pcObject, CCPPFunction* pcFunction)
{
	if (pcObject && !pcFunction->mbStatic && !pcFunction->mbConstThis)
	{
		mpcUnit->GetCheck()->Changed(pcObject, CPPPC_MEMBER_CALL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A call to an object's member function (an operator or a conversion function):
// its arguments (NULL for none), the object's place, and its result.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ObjectCall(CCPPSyntax* pcObject, CCPPFunction* pcFunction, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	if (!Call(pcFunction, papcArguments, pcNode) || !Place(pcObject))
	{
		return NULL;
	}
	ObjectChanged(pcObject, pcFunction);
	return pcFunction->mpsReturn;
}


//////////////////////////////////////////////////////////////////////////
//
// object op arguments, a call to the object's operator function of eOperator.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::OperatorCall(CCPPSyntax* pcObject, ECppTokens eOperator, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CCPPBoundCall	cBound;

	if (!mpcUnit->GetBinder()->BindOwnOperator(eOperator, pcObject, (papcArguments->NumElements() > 0) ? papcArguments->GetPtr(0) : NULL, pcNode, &cBound, true))
	{
		return NULL;
	}
	return ObjectCall(pcObject, cBound.mpcFunction, papcArguments, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// left op right (pcRight NULL for a unary operator) with an object: the left object's
// operator function if it has one that matches, else a free operator function
// (declared, or from a function template).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::OperatorExpression(ECppTokens eOperator, CCPPSyntax* pcLeft, CCPPSyntax* pcRight, CCPPSyntax* pcNode)
{
	CCPPBoundCall	cBound;
	SCPP65Type*		psType;
	CArraySyntax	apcArguments;

	if (!mpcUnit->GetBinder()->BindOperator(eOperator, pcLeft, pcRight, pcNode, &cBound, true))
	{
		Error(pcNode, "No operator matches the operands");
		return NULL;
	}
	apcArguments.Init();
	if (cBound.meHow == CPPBC_Function)
	{
		apcArguments.Add(pcLeft);
	}
	if (pcRight)
	{
		apcArguments.Add(pcRight);
	}
	psType = (cBound.meHow == CPPBC_Function) ? Call(cBound.mpcFunction, &apcArguments, pcNode) : ObjectCall(pcLeft, cBound.mpcFunction, &apcArguments, pcNode);
	apcArguments.Kill();
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The pointer that object-> goes through: the object's own (a pointer), or what its
// operator-> returns: a pointer, or an object whose operator-> gives one.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ArrowPointer(CCPPSyntax* pcObject, CCPPSyntax* pcNode)
{
	SCPP65Type*		psType;
	CCPPFunction*	pcFunction;
	int				i;

	psType = TypeOf(pcObject);
	if (!psType || !psType->IsClass())
	{
		return Value(pcObject);
	}
	psType = OperatorCall(pcObject, CT_MemberByPointer, &mapcNone, pcNode);
	psType = (psType && psType->IsReference() && psType->psBase->IsClass()) ? psType->psBase : psType;
	for (i = 0; psType && psType->IsClass() && (i < CPP_MAX_ARROWS); i++)
	{
		//An operator-> that returns an object (or a reference to one): its operator-> is applied, until one returns a
		//pointer.  [over.ref] 1
		pcFunction = mpcUnit->CompleteClass(psType->pcClass, pcNode) ? mpcUnit->GetBinder()->FindArrowOperator(psType->pcClass, NULL, pcNode) : NULL;
		psType = pcFunction ? Call(pcFunction, NULL, pcNode) : NULL;
		psType = (psType && psType->IsReference() && psType->psBase->IsClass()) ? psType->psBase : psType;
	}
	if (psType && !psType->IsPointer())
	{
		Error(pcNode, "operator-> must return a pointer, or an object whose operator-> does");
		return NULL;
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// A field's type in an object that is const or volatile: a member of one is
// too, but for a mutable one; a reference field is what it refers to.
// [expr.ref] 6.2
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::FieldType(SCPPField* psField, bool bConst, bool bVolatile)
{
	if (psField->psType->IsReference() || psField->bMutable)
	{
		return psField->psType->WithoutReference();
	}
	return mpcTypes->Qualified(psField->psType, psField->psType->bConst || bConst, psField->psType->bVolatile || bVolatile);
}


//////////////////////////////////////////////////////////////////////////
//
// object.member and pointer->member: a field (a reference field is what it refers
// to; a const object's fields are const) or a static data member.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::MemberPlace(CCPPSyntaxMemberExpression* pcMember)
{
	CCPPBoundName	cBound;
	SCPP65Type*		psType;
	size			uiErrors;

	if (pcMember->mbArrow)
	{
		psType = ArrowPointer(pcMember->mpcObject, pcMember);
		if (!psType)
		{
			return NULL;
		}
		if (!psType->IsPointer() || !psType->psBase->IsClass())
		{
			Error(pcMember, "-> needs a pointer to an object");
			return NULL;
		}
		psType = psType->psBase;
	}
	else
	{
		psType = Place(pcMember->mpcObject);
		if (!psType)
		{
			return NULL;
		}
		if (!psType->IsClass())
		{
			Error(pcMember, ". needs an object");
			return NULL;
		}
	}

	uiErrors = mpcDiagnostics->GetErrors()->NumErrors();
	if (!mpcUnit->GetBinder()->BindMember(psType->pcClass, pcMember->mpcName, pcMember, &cBound))
	{
		if (mpcDiagnostics->GetErrors()->NumErrors() == uiErrors)
		{
			Error(pcMember, "The class has no member of that name");
		}
		return NULL;
	}
	mpcUnit->GetCheck()->Resolved(pcMember->mpcName, cBound.mpcSymbol);
	mpcUnit->WarnDeprecatedSymbol(cBound.mpcSymbol, pcMember->mpcName);
	return (cBound.meWhat == CPPBN_Field) ? FieldType(cBound.mpsField, psType->bConst, psType->bVolatile) : cBound.mpcVariable->mpsType;
}


//////////////////////////////////////////////////////////////////////////
//
// The pointer to member's type of object.*member or pointer->*member, once the
// object is an object of the member's class or of a class derived from it,
// where that base is once and may be reached.  *ppsObject (if not NULL) is
// the object's type, with its const and volatile.  [expr.mptr.oper] 2, 3
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::MemberPointerObject(CCPPSyntaxBinaryExpression* pcBinary, SCPP65Type** ppsObject)
{
	SCPP65Type*		psMember;
	SCPP65Type*		psObject;
	CCPPClass*		pcClass;

	psMember = TypeOf(pcBinary->mpcRight);
	if (!psMember)
	{
		//Its errors.
		Value(pcBinary->mpcRight);
		return NULL;
	}
	if (!psMember->IsMemberPointer())
	{
		Error(pcBinary, "The right of .* and ->* must be a pointer to a member");
		return NULL;
	}
	if (pcBinary->meOperator == CT_MemberByPointerFunctionPointerIndirection)
	{
		psObject = Value(pcBinary->mpcLeft);
		if (!psObject)
		{
			return NULL;
		}
		if (!psObject->IsPointer() || !psObject->psBase->IsClass())
		{
			Error(pcBinary, "The left of ->* must be a pointer to an object");
			return NULL;
		}
		psObject = psObject->psBase;
	}
	else
	{
		psObject = Place(pcBinary->mpcLeft);
		if (!psObject)
		{
			return NULL;
		}
		if (!psObject->IsClass())
		{
			Error(pcBinary, "The left of .* must be an object");
			return NULL;
		}
	}
	pcClass = psObject->pcClass;
	if (psMember->pcClass && (pcClass != psMember->pcClass))
	{
		//A base further on in the object is where the member is.
		if (!mpcUnit->CompleteClass(pcClass, pcBinary))
		{
			return NULL;
		}
		if (!mpcUnit->IsDerived(pcClass, psMember->pcClass, NULL))
		{
			Error(pcBinary, "The object isn't of the class the pointer to a member is of, or of a class derived from it");
			return NULL;
		}
		if (!mpcUnit->GetBinder()->ReachesBase(pcClass, psMember->pcClass, pcBinary))
		{
			return NULL;
		}
	}
	if (ppsObject)
	{
		*ppsObject = psObject;
	}
	return psMember;
}


//////////////////////////////////////////////////////////////////////////
//
// object.*field and pointer->*field: the field's type, with the object's
// const and volatile (a mutable field's too: no pointer to a member says
// that it is one).  [expr.mptr.oper] 5
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::MemberPointerPlace(CCPPSyntaxBinaryExpression* pcBinary)
{
	SCPP65Type*	psMember;
	SCPP65Type*	psObject;
	SCPP65Type*	psField;

	psMember = MemberPointerObject(pcBinary, &psObject);
	if (!psMember)
	{
		return NULL;
	}
	psField = psMember->psBase;
	if (psField->IsFunction())
	{
		Error(pcBinary, "A pointer to a member function can only be called");
		return NULL;
	}
	if (!Value(pcBinary->mpcRight))
	{
		return NULL;
	}
	return mpcTypes->Qualified(psField->Unqualified(), psField->bConst || psObject->bConst, psField->bVolatile || psObject->bVolatile);
}


//////////////////////////////////////////////////////////////////////////
//
// (object.*function)(...) and (pointer->*function)(...): the member function's
// result.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::MemberPointerCall(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, CCPPSyntaxBinaryExpression* pcBinary)
{
	SCPP65Type*	psMember;
	SCPP65Type*	psFunctionType;
	SCPP65Type*	psObject;

	psMember = TypeOf(pcBinary->mpcRight);
	if (!psMember || !psMember->IsMemberPointer() || !psMember->psBase->IsFunction())
	{
		Error(pcBinary, "Only a pointer to a member function can be called");
		return NULL;
	}
	psFunctionType = psMember->psBase;
	if (!Arguments(psFunctionType->papsParameters, NULL, psFunctionType->bVariadic, papcArguments, pcCall, psFunctionType->psBase) ||
		!MemberPointerObject(pcBinary, &psObject) ||
		!Value(pcBinary->mpcRight))
	{
		return NULL;
	}
	if ((psObject->bConst && !psFunctionType->bConstFunction) || (psObject->bVolatile && !psFunctionType->bVolatileFunction))
	{
		//[expr.mptr.oper] 5
		Error(pcBinary, (psObject->bConst && !psFunctionType->bConstFunction) ? "A const object's member function is called through a pointer to one that is const" : "A volatile object's member function is called through a pointer to one that is volatile");
		return NULL;
	}
	return psFunctionType->psBase;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the place an lvalue is (NULL after an error).  A reference is the
// place it refers to; an array stays an array.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::Place(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = PlaceType(pcExpression);
	mpcUnit->GetCheck()->PlaceTyped(pcExpression, psType);
	mpcUnit->GetCheck()->PlaceTyped(mpcUnit->SkipParentheses(pcExpression), psType);
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// What Place gives for each kind of expression that can be a place.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::PlaceType(CCPPSyntax* pcExpression)
{
	CCPPSyntaxIdExpression*		pcId;
	CCPPBoundName					cBound;
	CCPPSyntaxUnaryExpression*	pcUnary;
	CCPPSyntaxBinaryExpression*	pcBinary;
	SCPP65Type*					psType;
	CCPPSyntax*					pcLeft;

	pcExpression = mpcUnit->SkipParentheses(pcExpression);
	switch (pcExpression->GetKind())
	{
	case CPPS_IdExpression:
		//A variable, or a field of this.
		pcId = (CCPPSyntaxIdExpression*)pcExpression;
		if (mpcUnit->IsAmbiguousName(pcId->mpcName))
		{
			mpcDiagnostics->ReportOnce(pcId, "The name is ambiguous: it finds more than one thing");
			return NULL;
		}
		mpcUnit->GetBinder()->BindName(pcId, &cBound);
		if (cBound.meWhat == CPPBN_Variable)
		{
			if ((cBound.mpcVariable->meStorage != CPPVS_Global) && !mpcUnit->GetContext()->IsUnevaluated() && mpcUnit->DefaultArgumentOf(pcId))
			{
				//A default argument is worked out where the function is called.  [dcl.fct.default] 7, 9
				mpcDiagnostics->ReportOnce(pcId, "A default argument can't use a variable or a parameter of a function");
				return NULL;
			}
			return cBound.mpcVariable->mpsType->WithoutReference();
		}
		if (cBound.meWhat == CPPBN_TemplateArgument)
		{
			return Place(cBound.mpcArgument);
		}
		if (cBound.meWhat == CPPBN_Function)
		{
			//A function's name is the function, an lvalue.
			psType = FunctionAddress(pcId, cBound.mpcSymbol->mlliName, NULL);
			return psType ? psType->psBase : NULL;
		}
		if (cBound.meWhat == CPPBN_Capture)
		{
			//What the closure holds: a copy, const unless the lambda is mutable, or a reference to it.
			if (cBound.mpsField->psType->IsReference())
			{
				return cBound.mpsField->psType->psBase;
			}
			return FieldType(cBound.mpsField, mpcUnit->GetContext()->GetFunction()->mbConstThis, false);
		}
		if (cBound.meWhat != CPPBN_Field)
		{
			if (cBound.meWhat == CPPBN_FunctionName)
			{
				Error(pcExpression, "This is not an lvalue");
				return NULL;
			}
			if (cBound.meWhat == CPPBN_EnclosingConstant)
			{
				if (mpcUnit->GetContext()->IsUnevaluated())
				{
					//decltype((c)) names the variable, which nothing reads.
					return mpcUnit->GetClosures()->EnclosingConstantObjectType(cBound.mpcSymbol);
				}
				//Its object is wanted (its address, or a reference to it): that odr-uses it.  [basic.def.odr] 3
				Error(pcExpression, mpcUnit->GetClosures()->WritingLambda() ? "The lambda doesn't capture the variable" : "A local class can't use an automatic variable of the function it is in, other than a constant's value");
				return NULL;
			}
			//S::m with no object, in an operand that isn't evaluated: nothing is read.  [expr.prim.general] 13
			psType = mpcUnit->GetContext()->IsUnevaluated() ? mpcUnit->QualifiedFieldType(pcExpression) : NULL;
			if (!psType)
			{
				Error(pcExpression, NotAPlace(((CCPPSyntaxIdExpression*)pcExpression)->mpcName));
			}
			return psType;
		}
		if (!mpcUnit->GetContext()->HasThis() && !mpcUnit->GetContext()->IsUnevaluated())
		{
			Error(pcExpression, "A field needs an object");
			return NULL;
		}
		return FieldType(cBound.mpsField, mpcUnit->IsConstThis(), mpcUnit->IsVolatileThis());

	case CPPS_MemberExpression:
		return MemberPlace((CCPPSyntaxMemberExpression*)pcExpression);

	case CPPS_UnaryExpression:
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		if (((pcUnary->meOperator == CT_Increment) || (pcUnary->meOperator == CT_Decrement)) && !pcUnary->mbPostfix && !IsObject(pcUnary->mpcOperand))
		{
			//++x and --x are x, once it is stepped.  [expr.pre.incr] 1
			return IncrementExpression(pcUnary) ? Place(pcUnary->mpcOperand) : NULL;
		}
		if (pcUnary->meOperator != CT_Multiply)
		{
			break;
		}
		if (IsObject(pcUnary->mpcOperand))
		{
			//*object: what its operator's reference refers to, or the object it returns.
			return ResultPlace(Result(pcExpression), pcExpression);
		}
		psType = Value(pcUnary->mpcOperand);
		if (!psType)
		{
			return NULL;
		}
		if (!psType->IsPointer() || psType->psBase->IsVoid())
		{
			Error(pcExpression, "Only pointers can be dereferenced");
			return NULL;
		}
		return psType->psBase;

	case CPPS_SubscriptExpression:
		return SubscriptPlace((CCPPSyntaxSubscriptExpression*)pcExpression);

	case CPPS_StringExpression:
		//A string literal is the array of its characters ("text"_s is its literal operator's call, below).
		if (mpcUnit->IsStringLiteral(pcExpression))
		{
			return mpcUnit->GetStrings()->StringArrayType(pcExpression);
		}
		break;

	case CPPS_CallExpression:
		//A function that returns a reference, or an object (its temporary).
		return ResultPlace(Result(pcExpression), pcExpression);

	case CPPS_BinaryExpression:
		if (mpcUnit->IsMemberPointerOperator(pcExpression))
		{
			return MemberPointerPlace((CCPPSyntaxBinaryExpression*)pcExpression);
		}
		pcBinary = (CCPPSyntaxBinaryExpression*)pcExpression;
		pcLeft = mpcUnit->SkipParentheses(pcBinary->mpcLeft);
		if (IsCPPAssignment(pcBinary->meOperator) && !IsObject(pcLeft))
		{
			//(x = y) and (x op= y) are x, once it is assigned.  [expr.ass] 1
			return Assignment(pcBinary) ? Place(pcLeft) : NULL;
		}
		if ((pcBinary->meOperator == CT_Comma) && !mpcUnit->GetBinder()->IsOverloadedComma(pcBinary))
		{
			//(x, y) is y, once x is worked out.  [expr.comma] 1
			return Value(pcBinary->mpcLeft) ? Place(pcBinary->mpcRight) : NULL;
		}
		break;

	case CPPS_CastExpression:
	case CPPS_NamedCastExpression:
		//A cast to a reference is what it refers to.  [expr.static.cast] 1
		if (Value(pcExpression, &psType) && psType->IsReference())
		{
			return psType->psBase;
		}
		break;

	case CPPS_ConditionalExpression:
		//One of two lvalues; of objects any other is the object it makes.  What is wrong with one that has no type is
		//said as its value's error.
		if (ConditionalLvalueType((CCPPSyntaxConditionalExpression*)pcExpression) || (!IsObject(pcExpression) && TypeOf(pcExpression)))
		{
			return ConditionalPlace((CCPPSyntaxConditionalExpression*)pcExpression);
		}
		if (!IsObject(pcExpression))
		{
			Value(pcExpression);
			return NULL;
		}
		break;

	default:
		break;
	}
	if (IsObject(pcExpression))
	{
		//An expression that makes an object: its temporary.
		return ResultPlace(Result(pcExpression), pcExpression);
	}

	//What is wrong with the expression itself is said first; else it is a value, which has no place: 5 = i, &5,
	//i++ = 5, this = p, int& r = 5.  [basic.lval] 1, [expr.ass] 1, [expr.unary.op] 3, [dcl.init.ref] 5
	if (Value(pcExpression))
	{
		Error(pcExpression, "The expression is a value, not an object or a function: it has no place to assign to, take the address of or refer to");
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// array[index] (or index[array]) of a pointer or an array and an integer: the
// element.  object[index] is what the object's operator[] returns, which must be a
// reference (or an object).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::SubscriptPlace(CCPPSyntaxSubscriptExpression* pcSubscript)
{
	SCPP65Type*		psType;
	SCPP65Type*		psArray;
	SCPP65Type*		psIndex;
	CArraySyntax	apcIndex;

	psType = TypeOf(pcSubscript->mpcArray);
	if (psType && psType->IsClass())
	{
		apcIndex.Init();
		apcIndex.Add(pcSubscript->mpcIndex);
		psType = OperatorCall(pcSubscript->mpcArray, CT_OpenSquareBracket, &apcIndex, pcSubscript);
		apcIndex.Kill();
		return ResultPlace(psType, pcSubscript);
	}
	if (pcSubscript->mpcIndex->GetKind() == CPPS_InitializerList)
	{
		//[expr.sub] 1
		Error(pcSubscript->mpcIndex, "A list in braces is an index only for a class's operator[]");
		return NULL;
	}

	if (!Operands(pcSubscript->mpcArray, pcSubscript->mpcIndex, &psArray, &psIndex))
	{
		return NULL;
	}
	if (!(psIndex->IsPointer() && psArray->IsInteger()) && !(psArray->IsPointer() && psIndex->IsInteger()))
	{
		Error(pcSubscript, "A subscript needs a pointer or an array and an integer");
		return NULL;
	}
	psType = Operate(CT_Add, psArray, psIndex, pcSubscript);
	if (!psType || psType->psBase->IsVoid())
	{
		Error(pcSubscript, "A void pointer cannot be subscripted");
		return NULL;
	}
	return psType->psBase;
}


//////////////////////////////////////////////////////////////////////////
//
// The place of what a call gives (psType, NULL after an error): a reference is the
// place it refers to, an object the temporary it is made in.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ResultPlace(SCPP65Type* psType, CCPPSyntax* pcNode)
{
	if (!psType)
	{
		return NULL;
	}
	if (psType->IsReference())
	{
		return psType->psBase;
	}
	if (!psType->IsClass())
	{
		Error(pcNode, "This is not an lvalue");
		return NULL;
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// A cast to a reference, (T&)x or static_cast<T&&>(x): x itself as a T,
// which is its own type (const taken away or not) or, for an object, a
// base or a derived class of its class; any other type only by
// reinterpret_cast, or by (T&)x.  [expr.static.cast] 2, 3
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ReferenceCast(SCPP65Type* psTo, CCPPSyntax* pcOperand, CCPPSyntax* pcNode)
{
	SCPP65Type*	psPlace;
	SCPP65Type*	psReferred;

	psPlace = Place(pcOperand);
	if (!psPlace)
	{
		return NULL;
	}
	psReferred = psTo->psBase;
	if (psReferred->Equals(psPlace))
	{
		return psTo;
	}
	if (psReferred->IsClass() && psPlace->IsClass() && (mpcUnit->IsDerived(psPlace->pcClass, psReferred->pcClass, NULL) || mpcUnit->IsDerived(psReferred->pcClass, psPlace->pcClass, NULL)))
	{
		return CastReachesBase(psPlace, psReferred, pcNode) ? psTo : NULL;
	}
	if ((pcNode->GetKind() == CPPS_CastExpression) || ((pcNode->GetKind() == CPPS_NamedCastExpression) && (((CCPPSyntaxNamedCastExpression*)pcNode)->meCast == CT_reinterpret_cast)))
	{
		//reinterpret_cast<T&>(x) is *reinterpret_cast<T*>(&x), and (T&)x is that where it can be nothing else.
		//[expr.reinterpret.cast] 11, [expr.cast] 4
		return psTo;
	}
	Error(pcNode, "This cast can't make a reference to another type: reinterpret_cast does");
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// (type)x, static_cast<type>(x), reinterpret_cast<type>(x), const_cast<type>(x) and
// type(x).  To a class it is a temporary made from x; (FPointer)&f chooses the
// overload of f with the pointer's type.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::CastExpression(CCPPSyntaxTypeId* pcTypeId, CCPPSyntax* pcOperand, CCPPSyntax* pcNode)
{
	SCPP65Type*				psTo;
	SCPP65Type*				psFrom;
	CCPPSyntaxIdExpression*	pcFunction;

	psTo = mpcUnit->GetDeclarators()->GetTypeIdType(pcTypeId);
	if (!psTo)
	{
		return NULL;
	}
	if (psTo->IsClass())
	{
		return TemporaryObject(psTo, pcOperand, pcNode);
	}
	if (psTo->IsReference())
	{
		return ReferenceCast(psTo, pcOperand, pcNode);
	}

	pcFunction = mpcUnit->FunctionNameFor(pcOperand, psTo);
	if (pcFunction && !pcFunction->mpcName->IsQualifiedByClass())
	{
		return FunctionAddress(pcFunction, pcFunction->mpcName->mpcSymbol->mlliName, psTo->psBase) ? psTo : NULL;
	}

	psFrom = ValueFor(pcOperand, psTo, true);
	if (!psFrom)
	{
		return NULL;
	}
	if (psTo->IsVoid())
	{
		return psTo;
	}
	if (psTo->IsArray() || psTo->IsFunction())
	{
		//[expr.cast] 4, [expr.static.cast] 4
		Error(pcNode, psTo->IsArray() ? "A cast can't make an array" : "A cast can't make a function");
		return NULL;
	}
	if (!psTo->IsScalar())
	{
		Error(pcNode, "Only casts to integers and pointers are supported by the compiler yet");
		return NULL;
	}
	if (psFrom->IsMemberPointer() && !psTo->IsMemberPointer() && (psTo->eKind != CPP65TK_Bool))
	{
		//No cast makes a pointer to a member an address or a number.  [expr.cast] 4, [expr.reinterpret.cast] 10
		Error(pcNode, "A pointer to a member isn't an address: no cast makes it a pointer or a number");
		return NULL;
	}
	if (psTo->IsMemberPointer() && !psFrom->IsMemberPointer() && !psFrom->IsNullPointer() && !mpcUnit->IsNullPointerConstant(pcOperand))
	{
		Error(pcNode, "A pointer to a member is made from another, 0 or nullptr: no cast makes one from an address or a number");
		return NULL;
	}
	return (Convert(psFrom, psTo, pcNode) && CastReachesBase(psFrom, psTo, pcNode)) ? psTo : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the cast pcNode may take a psFrom to a psTo where they are two
// classes (a reference's cast), pointers to two or pointers to members of
// two, and one class is the other's base: the base is in the derived class
// once, and may be reached there, which only (T)x doesn't ask.  True for
// any other types, and for reinterpret_cast, which makes no use of the
// classes.  [expr.static.cast] 2, 11, 12; [expr.cast] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::CastReachesBase(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode)
{
	CCPPSyntaxNamedCastExpression*	pcNamed;
	CCPPClass*						pcFrom;
	CCPPClass*						pcTo;
	CCPPClass*						pcDerived;
	int								iOffset;

	if (psFrom->IsMemberPointer() && psTo->IsMemberPointer())
	{
		pcFrom = psFrom->pcClass;
		pcTo = psTo->pcClass;
	}
	else
	{
		if (psFrom->IsPointer() && psTo->IsPointer())
		{
			psFrom = psFrom->psBase;
			psTo = psTo->psBase;
		}
		pcFrom = psFrom->IsClass() ? psFrom->pcClass : NULL;
		pcTo = psTo->IsClass() ? psTo->pcClass : NULL;
	}
	pcNamed = (pcNode->GetKind() == CPPS_NamedCastExpression) ? (CCPPSyntaxNamedCastExpression*)pcNode : NULL;
	if (!pcFrom || !pcTo || (pcFrom == pcTo) || (pcNamed && (pcNamed->meCast == CT_reinterpret_cast)))
	{
		return true;
	}
	pcDerived = mpcUnit->IsDerived(pcFrom, pcTo, NULL) ? pcFrom : pcTo;
	if ((pcDerived == pcTo) && !psFrom->IsMemberPointer() && !(pcNamed && (pcNamed->meCast == CT_dynamic_cast)) && mpcUnit->CompleteClass(pcTo, pcNode) && mpcUnit->IsDerived(pcTo, pcFrom, &iOffset) && (iOffset == CPP_VIRTUAL_PATH))
	{
		//A virtual base is nowhere fixed in the class: only dynamic_cast goes from it to the derived class.
		//[expr.static.cast] 2, 11
		return Error(pcNode, "A pointer or reference to a virtual base can't be cast to a class derived from it");
	}
	if (pcNamed)
	{
		return mpcUnit->GetBinder()->ReachesBase(pcDerived, (pcDerived == pcFrom) ? pcTo : pcFrom, pcNode);
	}
	return mpcUnit->GetBinder()->HasOneBase(pcDerived, (pcDerived == pcFrom) ? pcTo : pcFrom, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Type(arguments) or Type{arguments}: a temporary of a class, or of any other
// type a cast of one value, or zero for none.  The type is one word:
// unsigned int(3) is the Microsoft compiler's, and an error unless it is
// asked for (CCPPCompileOptions::mbMultiWordCasts).  [expr.type.conv] 1
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ConstructExpression(CCPPSyntaxConstructExpression* pcConstruct)
{
	SCPP65Type*		psType;
	CArraySyntax*	papcElements;

	if (pcConstruct->mpcType->mpcSpecifiers && (pcConstruct->mpcType->mpcSpecifiers->NumTypeKeywords() > 1) && !mpcUnit->GetOptions()->mbMultiWordCasts)
	{
		Error(pcConstruct, "A cast written as a type and brackets has a type of one word: unsigned(3) or (unsigned int)3, not unsigned int(3)");
		return NULL;
	}
	psType = mpcUnit->GetDeclarators()->GetTypeIdType(pcConstruct->mpcType);
	if (psType && psType->IsClass())
	{
		return mpcUnit->CompleteObjectClass(psType->pcClass, pcConstruct) ? TemporaryObject(psType, pcConstruct->mpcInitializer, pcConstruct) : NULL;
	}
	papcElements = mpcUnit->ConstructElements(pcConstruct);
	if (psType && papcElements && (papcElements->NumElements() == 0) && (psType->IsScalar() || psType->IsVoid()))
	{
		return psType;
	}
	if (papcElements && (papcElements->NumElements() == 1))
	{
		if (psType && (pcConstruct->mpcInitializer->GetKind() == CPPS_InitializerList) && !NotNarrowed(papcElements->GetPtr(0), psType))
		{
			//T{value} doesn't narrow the value, as T(value) may.  [expr.type.conv] 3, [dcl.init.list] 3
			return NULL;
		}
		return CastExpression(pcConstruct->mpcType, papcElements->GetPtr(0), pcConstruct);
	}
	Error(pcConstruct, "This construction is not supported by the compiler yet");
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// sizeof is an unsigned int, a type trait (__is_trivially_constructible(T) ...) a
// bool; both are constants.  A bit-field has no size.  [expr.sizeof] 1
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::SizeofExpression(CCPPSyntax* pcSizeof)
{
	CCPPSyntax*	pcOperand;
	int64		lliBytes;

	pcOperand = (pcSizeof->GetKind() == CPPS_SizeofExpression) ? ((CCPPSyntaxSizeofExpression*)pcSizeof)->mpcOperand : NULL;
	if (pcOperand && mpcUnit->GetBinder()->BitFieldOf(pcOperand))
	{
		Error(pcSizeof, "sizeof can't be applied to a bit-field");
		return NULL;
	}
	if (mpcUnit->GetSemanticPass())
	{
		//Only checking: the value isn't needed, only the type (__builtin_va_arg(ap, T) is a T).
		return CheckedSizeofType(pcSizeof);
	}
	if (!mpcUnit->GetFolder()->Constant(pcSizeof, &lliBytes))
	{
		SizeofError(pcSizeof);
		return NULL;
	}
	return (pcSizeof->GetKind() == CPPS_SizeofExpression) ? mpcTypes->Size() : &mpcTypes->msBool;
}


//////////////////////////////////////////////////////////////////////////
//
// Why sizeof or alignof has no value: its operand is a lambda (never worked
// out, so it may not be there), a function, void or an incomplete type, or
// an expression that is wrong.  [expr.sizeof] 1, [expr.alignof] 1,
// [expr.prim.lambda] 2
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::SizeofError(CCPPSyntax* pcSizeof)
{
	CCPPSyntaxSizeofExpression*	pcExpression;
	SCPP65Type*					psType;
	size						uiErrors;

	pcExpression = (pcSizeof->GetKind() == CPPS_SizeofExpression) ? (CCPPSyntaxSizeofExpression*)pcSizeof : NULL;
	if (!pcExpression || pcExpression->mbPack || ((pcExpression->meKeyword != CT_sizeof) && (pcExpression->meKeyword != CT_alignof)))
	{
		Error(pcSizeof, "This sizeof or type trait is not supported by the compiler yet");
		return;
	}
	if (pcExpression->mpcOperand && (mpcUnit->SkipParentheses(pcExpression->mpcOperand)->GetKind() == CPPS_LambdaExpression))
	{
		Error(pcSizeof, "A lambda can't be the operand of sizeof or alignof, which isn't worked out");
		return;
	}
	uiErrors = mpcDiagnostics->GetErrors()->NumErrors();
	psType = OperandType(pcExpression->mpcType, pcExpression->mpcOperand);
	if (!psType && pcExpression->mpcOperand)
	{
		Value(pcExpression->mpcOperand);
	}
	if (mpcDiagnostics->GetErrors()->NumErrors() > uiErrors)
	{
		return;
	}
	if (psType && psType->IsFunction())
	{
		Error(pcSizeof, (pcExpression->meKeyword == CT_sizeof) ? "sizeof can't be applied to a function" : "alignof can't be applied to a function");
	}
	else if (psType && psType->IsVoid())
	{
		Error(pcSizeof, (pcExpression->meKeyword == CT_sizeof) ? "sizeof can't be applied to void" : "alignof can't be applied to void");
	}
	else if (psType && ((psType->IsArray() && (psType->iCount == 0)) || (psType->IsClass() && !psType->pcClass->IsComplete())))
	{
		Error(pcSizeof, (pcExpression->meKeyword == CT_sizeof) ? "sizeof can't be applied to an incomplete type: an array without its bound, or a class not yet defined" : "alignof can't be applied to an incomplete type");
	}
	else if (!mpcDiagnostics->HasDeferred())
	{
		//(A class that couldn't be laid out has said why itself.)
		Error(pcSizeof, "This sizeof or type trait is not supported by the compiler yet");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type of a sizeof or type trait when only checking: __builtin_va_arg
// (ap, T) is a T, __builtin_offsetof a size, and the other traits bools.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::CheckedSizeofType(CCPPSyntax* pcSizeof)
{
	CCPPSyntaxTypeTraitExpression*	pcTrait;
	CCPPSyntaxSizeofExpression*		pcExpression;
	CChars							szTrait;
	SCPP65Type*						psType;
	bool							bComplete;

	if (pcSizeof->GetKind() == CPPS_SizeofExpression)
	{
		//Only what the operand's type says is checked (SizeofError has the same): the value isn't worked out.
		pcExpression = (CCPPSyntaxSizeofExpression*)pcSizeof;
		if (pcExpression->mbPack || ((pcExpression->meKeyword != CT_sizeof) && (pcExpression->meKeyword != CT_alignof)))
		{
			return mpcTypes->Size();
		}
		if (pcExpression->mpcOperand && (mpcUnit->SkipParentheses(pcExpression->mpcOperand)->GetKind() == CPPS_LambdaExpression))
		{
			Error(pcSizeof, "A lambda can't be the operand of sizeof or alignof, which isn't worked out");
			return NULL;
		}
		mpcDiagnostics->BeginQuiet();
		psType = OperandType(pcExpression->mpcType, pcExpression->mpcOperand);
		bComplete = !psType || !psType->IsClass() || mpcUnit->CompleteClass(psType->pcClass, pcSizeof);
		mpcDiagnostics->EndQuiet();
		if (psType && psType->IsFunction())
		{
			Error(pcSizeof, (pcExpression->meKeyword == CT_sizeof) ? "sizeof can't be applied to a function" : "alignof can't be applied to a function");
			return NULL;
		}
		if (psType && psType->IsVoid())
		{
			Error(pcSizeof, (pcExpression->meKeyword == CT_sizeof) ? "sizeof can't be applied to void" : "alignof can't be applied to void");
			return NULL;
		}
		if (psType && ((psType->IsArray() && (psType->iCount == 0)) || !bComplete))
		{
			Error(pcSizeof, (pcExpression->meKeyword == CT_sizeof) ? "sizeof can't be applied to an incomplete type: an array without its bound, or a class not yet defined" : "alignof can't be applied to an incomplete type");
			return NULL;
		}
		return mpcTypes->Size();
	}
	pcTrait = (CCPPSyntaxTypeTraitExpression*)pcSizeof;
	szTrait.Init();
	mpcUnit->GetTokeniser()->GetSource(mpcUnit->GetTokeniser()->Get(pcTrait->miName))->Append(&szTrait);
	psType = &mpcTypes->msBool;
	if (szTrait.Equals("__builtin_va_arg"))
	{
		psType = NULL;
		if ((pcTrait->macpArguments.NumElements() == 2) && (pcTrait->macpArguments.GetPtr(1)->GetKind() == CPPS_TypeId))
		{
			Value(pcTrait->macpArguments.GetPtr(0));
			psType = mpcUnit->GetDeclarators()->GetTypeIdType((CCPPSyntaxTypeId*)pcTrait->macpArguments.GetPtr(1));
		}
	}
	else if (szTrait.Equals("__builtin_offsetof"))
	{
		psType = mpcTypes->Size();
	}
	szTrait.Kill();
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// new Type, new Type(...), new (placement...) Type and new Type[count]...:
// a pointer to what it makes, from the allocation function the type and
// the placement arguments choose (CCPPBinder::FindAllocation), given the
// size, which returns a void*.  [expr.new]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::NewExpression(CCPPSyntaxNewExpression* pcNew)
{
	SCPP65Type*		psType;
	SCPP65Type*		psResult;
	CCPPFunction*	pcAllocation;
	CArraySyntax	apcArguments;
	bool			bFailed;

	psType = mpcUnit->NewElementType(pcNew);
	if (!psType || (psType->IsClass() && !mpcUnit->CompleteObjectClass(psType->pcClass, pcNew)))
	{
		return NULL;
	}
	if (psType->IsVoid() || psType->IsReference() || psType->IsFunction())
	{
		//new makes an object.  [expr.new] 1
		Error(pcNew, psType->IsVoid() ? "new makes an object: void is no object's type" : (psType->IsReference() ? "new makes an object: a reference is none" : "new makes an object: a function is none"));
		return NULL;
	}

	apcArguments.Init();
	mpcUnit->NewArguments(pcNew, &apcArguments);
	pcAllocation = mpcUnit->GetBinder()->FindAllocation(pcNew, psType, &apcArguments, &bFailed);
	psResult = pcAllocation ? Call(pcAllocation, &apcArguments, pcNew) : &mpcTypes->msVoid;
	apcArguments.Kill();
	if (bFailed)
	{
		Error(pcNew, "No operator new takes the placement arguments");
		return NULL;
	}
	if (!psResult)
	{
		return NULL;
	}
	if (pcNew->mbPlacement && !pcAllocation && !Value(pcNew->macpPlacement.GetPtr(0)))
	{
		return NULL;
	}

	if (mpcUnit->IsArrayNew(pcNew))
	{
		return NewArray(pcNew, psType);
	}
	if ((psType->IsClass() || pcNew->mpcInitializer) && !Construct(psType, NULL, pcNew->mpcInitializer, pcNew))
	{
		return NULL;
	}
	return mpcTypes->Pointer(psType);
}


//////////////////////////////////////////////////////////////////////////
//
// new Type[count]...: its elements default initialised, value initialised
// by (), or the first from a list in braces and the rest value
// initialised.  [expr.new] 17
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::NewArray(CCPPSyntaxNewExpression* pcNew, SCPP65Type* psType)
{
	CArraySyntax*	papcElements;
	SCPP65Type*		psElement;
	size			i;

	if (mpcUnit->NewTypeIdArray(pcNew))
	{
		if (mpcUnit->NewTypeIdArray(pcNew)->iCount == 0)
		{
			Error(pcNew, "A new array needs its bound");
			return NULL;
		}
	}
	else if (!NewArrayBound(pcNew->macpArraySizes.GetPtr(0)))
	{
		return NULL;
	}
	papcElements = pcNew->mpcInitializer ? mpcUnit->ListElements(pcNew->mpcInitializer) : NULL;
	if (papcElements && (pcNew->mpcInitializer->GetKind() == CPPS_ParenthesisedInitializer) && (papcElements->NumElements() > 0))
	{
		Error(pcNew, "new Type[count](arguments) can only be ()");
		return NULL;
	}
	if (psType->IsArray() && papcElements && (papcElements->NumElements() > 0))
	{
		Error(pcNew, "A new array of arrays with a list of initialisers is not supported by the compiler yet");
		return NULL;
	}
	if (mpcUnit->IsStringLiteral(mpcUnit->StringInitialiser(pcNew->mpcInitializer, psType)))
	{
		//new char[n]{"abc"}: the string's characters, as an array of them is initialised.  [dcl.init.string]
		papcElements = NULL;
	}
	for (i = 0; papcElements && (i < papcElements->NumElements()); i++)
	{
		if (!Construct(psType, NULL, papcElements->GetPtr(i), pcNew))
		{
			return NULL;
		}
	}
	for (psElement = psType; psElement->IsArray(); psElement = psElement->psBase)
	{
	}
	if (psElement->IsClass() && psElement->pcClass->NeedsConstructor() && !Construct(psElement, NULL, NULL, pcNew))
	{
		return NULL;
	}
	return mpcTypes->Pointer(psType);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether new Type[bound]'s first bound is right: an integer, or an object
// that converts to one; a constant one isn't negative.  [expr.new] 6, 7
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::NewArrayBound(CCPPSyntax* pcBound)
{
	SCPP65Type*	psBound;
	int64		lliBound;

	psBound = mpcUnit->GetBinder()->ContextualType(pcBound, CPPXT_Integer);
	if (psBound && !psBound->IsInteger())
	{
		Error(pcBound, "A new array's bound must be an integer");
		return false;
	}
	if (psBound && psBound->bSigned && mpcUnit->GetFolder()->Constant(pcBound, &lliBound) && (lliBound < 0))
	{
		Error(pcBound, "A new array's bound can't be negative");
		return false;
	}
	return psBound && Convert(ValueFor(pcBound, psBound, false), &mpcTypes->msUnsignedInt, pcBound);
}


//////////////////////////////////////////////////////////////////////////
//
// delete pointer and delete[] pointer: void.  The pointer is to an object,
// whose destructor may be called here.  [expr.delete] 1, 10
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::DeleteExpression(CCPPSyntaxDeleteExpression* pcDelete)
{
	SCPP65Type*		psPointer;
	SCPP65Type*		psType;

	//A pointer, or an object that converts to one.  [expr.delete] 1
	psPointer = mpcUnit->GetBinder()->ContextualType(pcDelete->mpcOperand, CPPXT_ObjectPointer);
	psPointer = psPointer ? ValueFor(pcDelete->mpcOperand, psPointer, false) : NULL;
	if (!psPointer)
	{
		return NULL;
	}
	if (!psPointer->IsPointer() || psPointer->psBase->IsFunction())
	{
		Error(pcDelete, "delete needs a pointer to an object");
		return NULL;
	}
	psType = psPointer->psBase;
	if (psType->IsClass() && !(mpcUnit->CompleteClass(psType->pcClass, pcDelete) && mpcUnit->GetBinder()->ReachesDestructor(psType->pcClass, pcDelete)))
	{
		return NULL;
	}
	return &mpcTypes->msVoid;
}


//////////////////////////////////////////////////////////////////////////
//
// A temporary of a class made from pcInitialiser (by default when NULL).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::TemporaryObject(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode)
{
	if (!mpcUnit->CompleteClass(psType->pcClass, pcNode) || !Construct(psType, pcInitialiser ? NULL : &mapcNone, pcInitialiser, pcNode))
	{
		return NULL;
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether converting pcSource to psTo is a narrowing conversion: floating
// point to an integer; to a smaller floating type, or an integer to a
// floating type, unless the source is a constant; or an integer to an
// integer type that can't hold every value of the source's type, unless
// the source is a constant it can hold.  [dcl.init.list] 7
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::Narrows(CCPPSyntax* pcSource, SCPP65Type* psTo)
{
	SCPP65Type*	psFrom;
	int64		lliValue;
	int64		lliLimit;
	double		dValue;
	bool		bConstant;

	psFrom = ValueTypeOf(pcSource);
	if (!psFrom || psFrom->pcEnum || psTo->pcEnum || (psTo->eKind == CPP65TK_Bool) || !(psFrom->IsInteger() || psFrom->IsFloat()) || !(psTo->IsInteger() || psTo->IsFloat()))
	{
		return false;
	}

	mpcDiagnostics->BeginQuiet();
	if (psFrom->IsFloat())
	{
		bConstant = mpcUnit->GetFolder()->FloatConstant(pcSource, &dValue);
		mpcDiagnostics->EndQuiet();
		return psTo->IsInteger() || ((psTo->iBytes < psFrom->iBytes) && !bConstant);
	}
	bConstant = mpcUnit->GetFolder()->Constant(pcSource, &lliValue);
	mpcDiagnostics->EndQuiet();
	if (psTo->IsFloat())
	{
		return !bConstant;
	}
	if ((psTo->bSigned == psFrom->bSigned) ? (psTo->iBytes >= psFrom->iBytes) : (psTo->bSigned && (psTo->iBytes > psFrom->iBytes)))
	{
		return false;
	}
	if (!bConstant)
	{
		return true;
	}

	//A constant the type holds isn't narrowed.
	if (psTo->iBytes >= 8)
	{
		return !psTo->bSigned && (lliValue < 0);
	}
	lliLimit = (int64)1 << (8 * psTo->iBytes - (psTo->bSigned ? 1 : 0));
	return (lliValue >= lliLimit) || (lliValue < (psTo->bSigned ? -lliLimit : 0));
}


//////////////////////////////////////////////////////////////////////////
//
// A value in braces for a psTo: an error if converting it narrows it.
// Not in a source compiled with C's conversions (CCPPUnit::IsCSource): C has
// no such rule.  [dcl.init.list] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::NotNarrowed(CCPPSyntax* pcSource, SCPP65Type* psTo)
{
	if (!mpcUnit->IsCSource() && Narrows(pcSource, psTo))
	{
		return Error(pcSource, "The conversion could lose some of the value, which an initialiser in braces may not");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A constant converted to a psTo where the language asks for a converted
// constant expression (a case's value, as the switch's condition's type):
// an error, wherever it is asked, if converting it narrows it.
// [expr.const] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::ConstantNotNarrowed(CCPPSyntax* pcSource, SCPP65Type* psTo)
{
	return !Narrows(pcSource, psTo) || mpcDiagnostics->ReportError(pcSource, "The constant's value isn't one of the type it is converted to");
}


//////////////////////////////////////////////////////////////////////////
//
// What is wrong with a name that is used as an object and names none:
// Class::name where the class has no such member, or a member that isn't
// static, which only an object has ([expr.prim.general] 13).
//
//////////////////////////////////////////////////////////////////////////
const char* CCPP65TypeChecker::NotAPlace(CCPPSyntaxName* pcName)
{
	if (pcName->IsQualifiedByClass() && !pcName->mpcSymbol)
	{
		return "The class has no member of that name";
	}
	if (pcName->IsQualifiedByClass() && (pcName->mpcSymbol->meKind == CPPSK_Field))
	{
		return "A member that isn't static is used without an object";
	}
	if (pcName->mpcSymbol && (pcName->mpcSymbol->meKind == CPPSK_Field))
	{
		//A member of a class this function's class is nested in, which has no object here.  [class.nest] 1
		return "A member that isn't static is used without an object of its class";
	}
	if (mpcUnit->GetClosures()->IsOuterLocal(pcName->mpcSymbol))
	{
		//[class.local] 1
		return "A local class can't use an automatic variable of the function it is in, other than a constant's value";
	}
	return "Only variables are supported by the compiler here yet";
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a psType can be made from pcInitialiser (Construct), asked by
// overload resolution: what is wrong isn't an error.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::CanInitialise(SCPP65Type* psType, CCPPSyntax* pcInitialiser)
{
	bool	bResult;

	mpcDiagnostics->BeginSilentTrial();
	bResult = Construct(psType->Unqualified(), NULL, pcInitialiser, pcInitialiser);
	mpcDiagnostics->EndSilentTrial();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a psType can be made from papcArguments (Type(arguments)) or pcInitialiser
// (= value, (arguments) or {arguments}).  A reference is bound to a place, an array
// gets its elements in order, a scalar is converted from its one value, and an
// object is copied from an object of its class (or a class derived from it), or
// made by the constructor that matches (an aggregate without one by its fields).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::Construct(SCPP65Type* psType, CArraySyntax* papcArguments, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode)
{
	SCPP65Type*						psSource;
	CArraySyntax					apcArguments;
	CCPPSyntaxConstructExpression*	pcConstruct;
	CCPPClass*						pcClass;
	CCPPFunction*					pcConversion;
	CCPPFunction*					pcConstructor;
	CCPPSyntax*						pcList;
	CCPPSyntax*						pcString;
	SCPP65Type*						psElement;
	int								i;
	bool							bResult;
	bool							bBraced;

	apcArguments.Init();
	if (!papcArguments)
	{
		papcArguments = &apcArguments;
	}
	pcInitialiser = mpcUnit->GetDesugarings()->BracedFor(psType, pcInitialiser);
	bBraced = pcInitialiser && (pcInitialiser->GetKind() == CPPS_InitializerList);
	pcList = bBraced ? pcInitialiser : NULL;
	if (pcInitialiser && ((pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer) || (pcInitialiser->GetKind() == CPPS_InitializerList)))
	{
		papcArguments = mpcUnit->ListElements(pcInitialiser);
		pcInitialiser = NULL;
	}

	if (psType->IsReference())
	{
		if (!pcInitialiser && (papcArguments->NumElements() == 1))
		{
			pcInitialiser = papcArguments->GetPtr(0);
		}
		bResult = pcInitialiser ? (Place(pcInitialiser) != NULL) : Error(pcNode, "A reference must be initialised with one object");
		apcArguments.Kill();
		return bResult;
	}

	if (psType->IsArray())
	{
		//char a[4] = "abc"; and = { "abc" }: an array of characters is its string's.  [dcl.init.string]
		pcString = pcInitialiser ? pcInitialiser : ((papcArguments->NumElements() == 1) ? papcArguments->GetPtr(0) : NULL);
		if (pcString && mpcUnit->IsStringLiteral(pcString) && psType->psBase->IsInteger() && (psType->psBase->iBytes == mpcUnit->GetStrings()->StringElementBytes(pcString)))
		{
			apcArguments.Kill();
			return true;
		}
		if (!pcInitialiser && (psType->iCount > 0) && (papcArguments->NumElements() > (size)psType->iCount))
		{
			//More values than the array's elements take, with their braces left out (BracedFor gives them back).
			//[dcl.init.aggr] 7, 11
			apcArguments.Kill();
			return Error(pcNode, "More initialisers than the array's elements take");
		}
		if (pcString && mpcUnit->IsStringLiteral(pcString))
		{
			//[dcl.init.string] 1
			apcArguments.Kill();
			return Error(pcString, mpcUnit->GetCheck()->StringArrayError(pcString, psType->psBase));
		}
		if (pcInitialiser || (papcArguments->NumElements() > (size)psType->iCount))
		{
			apcArguments.Kill();
			return Error(pcNode, "This array initialiser is not supported by the compiler yet");
		}
		bResult = true;
		for (i = 0; i < psType->iCount; i++)
		{
			if ((size)i < papcArguments->NumElements())
			{
				bResult &= Construct(psType->psBase, NULL, papcArguments->GetPtr(i), pcNode);
			}
			else if (psType->psBase->IsClass())
			{
				bResult &= Construct(psType->psBase, NULL, NULL, pcNode);
			}
		}
		apcArguments.Kill();
		return bResult;
	}

	if (!psType->IsClass())
	{
		if (!pcInitialiser)
		{
			if (papcArguments->NumElements() > 1)
			{
				apcArguments.Kill();
				return Error(pcNode, "Expected one initialiser");
			}
			pcInitialiser = (papcArguments->NumElements() == 1) ? papcArguments->GetPtr(0) : NULL;
		}
		bResult = !pcInitialiser || ImplicitConvert(ValueFor(pcInitialiser, psType, false), psType, pcInitialiser, pcInitialiser);
		bResult = bResult && (!bBraced || !pcInitialiser || NotNarrowed(pcInitialiser, psType));
		apcArguments.Kill();
		return bResult;
	}

	pcClass = psType->pcClass;
	if (!mpcUnit->CompleteClass(pcClass, pcNode))
	{
		apcArguments.Kill();
		return false;
	}
	if (pcList)
	{
		//A std::initializer_list is made from the list's elements; a class with an initializer-list constructor
		//that takes the list, by it.  [dcl.init.list] 3 and 5
		psElement = mpcUnit->GetDesugarings()->InitializerListElement(psType);
		pcConstructor = psElement ? NULL : mpcUnit->GetBinder()->InitializerListConstructor(pcClass, pcList);
		if (psElement || pcConstructor)
		{
			apcArguments.ReInit();
			apcArguments.Add(pcList);
			bResult = psElement ? InitializerListElements(psType, psElement, papcArguments, pcNode) : (Call(pcConstructor, &apcArguments, pcNode) != NULL);
			apcArguments.Kill();
			return bResult;
		}
	}

	//(object) copies it, as = object does, when no copy constructor of the class's own is to be called.
	if (!pcInitialiser && (papcArguments->NumElements() == 1) && !mpcUnit->GetBinder()->CopiesByConstructor(pcClass, papcArguments->GetPtr(0)))
	{
		psSource = ObjectType(papcArguments->GetPtr(0));
		if (psSource && mpcUnit->IsDerived(psSource->pcClass, pcClass, NULL))
		{
			pcInitialiser = papcArguments->GetPtr(0);
		}
	}
	if (pcInitialiser)
	{
		//= Class(arguments) constructs in place.
		if (pcInitialiser->GetKind() == CPPS_ConstructExpression)
		{
			pcConstruct = (CCPPSyntaxConstructExpression*)pcInitialiser;
			if ((mpcUnit->GetDeclarators()->GetTypeIdType(pcConstruct->mpcType) == psType) && pcConstruct->mpcInitializer && (pcConstruct->mpcInitializer->GetKind() == CPPS_ParenthesisedInitializer))
			{
				bResult = ConstructClass(pcClass, &((CCPPSyntaxParenthesisedInitializer*)pcConstruct->mpcInitializer)->macpElements, pcNode);
				apcArguments.Kill();
				return bResult;
			}
		}

		//= object copies it (or is the object, made in place), = value calls a constructor that takes it.
		psSource = ObjectType(pcInitialiser);
		if (psSource && mpcUnit->IsDerived(psSource->pcClass, pcClass, NULL))
		{
			bResult = IsMadeObject(pcInitialiser, pcClass) ? (Value(pcInitialiser) != NULL) : CopyConstruct(pcClass, pcInitialiser, pcNode);
			apcArguments.Kill();
			return bResult;
		}

		//= an object of another class whose conversion function makes one.  [over.match.copy]
		pcConversion = mpcUnit->GetBinder()->ConversionToClass(pcClass, pcInitialiser, pcNode);
		if (pcConversion)
		{
			bResult = ObjectCall(pcInitialiser, pcConversion, NULL, pcNode) != NULL;
			apcArguments.Kill();
			return bResult;
		}
		apcArguments.Add(pcInitialiser);
		papcArguments = &apcArguments;
	}
	bResult = (pcList || mpcUnit->GetBinder()->ConstructsWithoutBraces(pcClass, papcArguments, pcNode)) && ConstructClass(pcClass, papcArguments, pcNode);
	apcArguments.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A std::initializer_list<psElement> (psType) from the elements of a list in
// braces: each copy initialises an element, without narrowing.  It must be
// laid out as the compiler fills it (CCPPUnit::InitializerListLayout).
// [dcl.init.list] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::InitializerListElements(SCPP65Type* psType, SCPP65Type* psElement, CArraySyntax* papcElements, CCPPSyntax* pcNode)
{
	CCPPSyntax*	pcElement;
	size		i;
	bool		bEnd;
	bool		bResult;

	if (!mpcUnit->GetDesugarings()->InitializerListLayout(psType->pcClass, &bEnd))
	{
		return Error(pcNode, "This std::initializer_list isn't laid out as the compiler fills one: a pointer to its elements, then their count or end");
	}
	if (psElement->IsClass() && !mpcUnit->CompleteClass(psElement->pcClass, pcNode))
	{
		return false;
	}
	bResult = true;
	for (i = 0; i < papcElements->NumElements(); i++)
	{
		pcElement = papcElements->GetPtr(i);
		bResult &= Construct(psElement->Unqualified(), NULL, pcElement, pcElement) && (psElement->IsClass() || (pcElement->GetKind() == CPPS_InitializerList) || NotNarrowed(pcElement, psElement));
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an object of pcClass can be made from papcArguments: by the constructor
// that matches them, by default when it has none, or else as an aggregate (its
// fields in order, the rest zero).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::ConstructClass(CCPPClass* pcClass, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	CCPPBoundConstruction	cBound;
	SCPPField*				psField;
	size					i;
	bool					bResult;

	papcArguments = mpcUnit->ExpandArguments(papcArguments);
	if (!papcArguments || !mpcUnit->GetBinder()->BindConstruction(pcClass, papcArguments, pcNode, &cBound, true))
	{
		return false;
	}
	if (cBound.meHow == CPPBO_Constructor)
	{
		return Call(cBound.mpcConstructor, papcArguments, pcNode) != NULL;
	}
	if (cBound.meHow == CPPBO_Nothing)
	{
		return true;
	}

	//An aggregate: its fields in order, the rest by their default member initialisers, which are written in the
	//class, so their this is the object being filled.  [dcl.init.aggr] 7
	bResult = true;
	for (i = 0; (psField = pcClass->AggregateField(i)) != NULL; i++)
	{
		if ((i < papcArguments->NumElements()) || !psField->pcInitialiser)
		{
			bResult &= Construct(psField->psType, NULL, (i < papcArguments->NumElements()) ? papcArguments->GetPtr(i) : NULL, pcNode);
			if (bResult && mpcUnit->GetSemanticPass() && (i < papcArguments->NumElements()) && psField->psType->IsScalar() && (papcArguments->GetPtr(i)->GetKind() != CPPS_InitializerList))
			{
				//A member's value from the list isn't narrowed (the semantic pass only: the IR front end checks it where
				//it writes the member, and asking twice costs a quiet trial each).  [dcl.init.aggr] 2, [dcl.init.list] 7
				bResult = NotNarrowed(papcArguments->GetPtr(i), psField->psType);
			}
			continue;
		}
		mpcUnit->GetContext()->EnterObject(pcClass);
		bResult &= Construct(psField->psType, NULL, psField->pcInitialiser, pcNode);
		mpcUnit->GetContext()->LeaveObject();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an object of pcClass can be made a copy of pcSource: by its copy
// constructor, member by member, or byte by byte.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::CopyConstruct(CCPPClass* pcClass, CCPPSyntax* pcSource, CCPPSyntax* pcNode)
{
	CArraySyntax	apcArguments;
	bool			bResult;

	if (mpcUnit->GetBinder()->CopiesByConstructor(pcClass, pcSource))
	{
		apcArguments.Init();
		apcArguments.Add(pcSource);
		bResult = ConstructClass(pcClass, &apcArguments, pcNode);
		apcArguments.Kill();
		return bResult;
	}
	if (IsRvalue(pcSource) ? mpcUnit->GetBinder()->IsMoveDeleted(pcClass) : mpcUnit->GetBinder()->IsCopyDeleted(pcClass))
	{
		return Error(pcNode, "The class's copy constructor is deleted: a member or base of it can't be copied");
	}
	if (!pcClass->IsTriviallyCopyable())
	{
		return Place(pcSource) != NULL;
	}
	return CopyObject(pcClass, pcSource, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcSource's bytes can be copied to an object of pcClass: it must be an
// object of pcClass or a class derived from it, as which it is a pcClass
// (CCPPBinder::ReachesBase).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::CopyObject(CCPPClass* pcClass, CCPPSyntax* pcSource, CCPPSyntax* pcNode)
{
	SCPP65Type*	psSource;

	psSource = Place(pcSource);
	if (!psSource)
	{
		return false;
	}
	if (!psSource->IsClass() || !mpcUnit->IsDerived(psSource->pcClass, pcClass, NULL))
	{
		return Error(pcNode, "The object cannot be copied to that class");
	}
	return mpcUnit->GetBinder()->ReachesBase(psSource->pcClass, pcClass, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// The class type of an expression that is an object (or a reference to one), else
// NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ObjectType(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = ValueTypeOf(pcExpression);
	return (psType && psType->IsClass()) ? psType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a variable of psType can be initialised with pcInitialiser
// (NULL for none), as the IR front end initialises a local: an object (or
// an array of them) is constructed, a reference is bound to its place (or
// to a temporary of its value), anything else takes the converted value
// of = value, (value) or {value}.  An array's list (or string) has each
// element's value.  A value in braces (bBraced: it is an element of a list)
// may not be narrowed.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::Initialise(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode, bool bBraced)
{
	CArraySyntax*				papcElements;
	CCPPSyntaxIdExpression*		pcFunctionName;
	SCPP65Type*					psElement;
	SCPP65Type*					psValue;
	size			i;
	bool			bResult;
	bool			bDirect;

	psElement = psType;
	while (psElement->IsArray())
	{
		psElement = psElement->psBase;
	}
	if (psElement->IsClass())
	{
		return Construct(psType, NULL, pcInitialiser, pcNode);
	}
	if (!pcInitialiser)
	{
		return true;
	}
	if (psType->IsArray())
	{
		//char a[] = { "abc" } is char a[] = "abc".  [dcl.init.string]
		pcInitialiser = mpcUnit->StringInitialiser(pcInitialiser, psType->psBase);
	}

	//T x(value) and T x{value} initialise directly: a std::nullptr_t may make a bool so.  [conv.bool]
	papcElements = NULL;
	bDirect = false;
	if (pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer)
	{
		papcElements = mpcUnit->ListElements(pcInitialiser);
		bDirect = true;
	}
	else if (pcInitialiser->GetKind() == CPPS_InitializerList)
	{
		papcElements = mpcUnit->ListElements(pcInitialiser);
		bBraced = true;
		bDirect = true;
	}
	if (papcElements && (papcElements->NumElements() == 1) && !psType->IsArray())
	{
		pcInitialiser = papcElements->GetPtr(0);
		papcElements = NULL;
	}
	if (papcElements || psType->IsArray())
	{
		if (!papcElements)
		{
			if (mpcUnit->IsStringLiteral(pcInitialiser) && !(psType->psBase->IsInteger() && (psType->psBase->eKind != CPP65TK_Bool) && (psType->psBase->iBytes == mpcUnit->GetStrings()->StringElementBytes(pcInitialiser))))
			{
				//[dcl.init.string] 1
				return Error(pcInitialiser, mpcUnit->GetCheck()->StringArrayError(pcInitialiser, psType->psBase));
			}
			return Value(pcInitialiser) != NULL;
		}
		if (!psType->IsArray() && (papcElements->NumElements() > 1))
		{
			//[dcl.init] 17.8, [dcl.init.list] 3
			return Error(pcInitialiser, "A number or a pointer is initialised by one value");
		}
		if (psType->IsArray() && !psType->psBase->IsArray() && (psType->iCount > 0) && (papcElements->NumElements() > (size)psType->iCount))
		{
			//[dcl.init.aggr] 7
			return Error(pcInitialiser, "More initialisers than the array's elements take");
		}
		bResult = true;
		for (i = 0; i < papcElements->NumElements(); i++)
		{
			bResult &= Initialise(psType->ElementType(), papcElements->GetPtr(i), pcNode, bBraced);
		}
		return bResult;
	}

	if (psType->IsReference())
	{
		pcFunctionName = psType->psBase->IsFunction() ? mpcUnit->FunctionNameOf(pcInitialiser) : NULL;
		if (pcFunctionName && pcFunctionName->mpcName->mpcSymbol)
		{
			//int (&f)(int) = F; binds to the overload of F of the type.  [over.over] 1
			return FunctionAddress(pcFunctionName, pcFunctionName->mpcName->mpcSymbol->mlliName, psType->psBase) != NULL;
		}
		if (!BindsCategory(psType, pcInitialiser, pcNode))
		{
			return false;
		}
		if (mpcUnit->GetBinder()->BindsToTemporary(psType, pcInitialiser))
		{
			//It refers to a temporary holding the value: an object made from it (const S& r = 4;), or the value converted.
			if (psType->psBase->IsClass())
			{
				return Construct(psType->psBase->Unqualified(), NULL, pcInitialiser, pcNode);
			}
			return ImplicitConvert(ValueFor(pcInitialiser, psType->psBase, false), psType->psBase, pcNode, pcInitialiser);
		}
		mpcDiagnostics->BeginQuiet();
		psValue = Place(pcInitialiser);
		mpcDiagnostics->EndQuiet();
		if (psValue && !psType->psBase->bConst)
		{
			mpcUnit->GetCheck()->Changed(pcInitialiser, CPPPC_REFERENCE);
		}
		if (psValue && (!psType->psBase->bConst || psType->psBase->bVolatile) && IsBitField(pcInitialiser))
		{
			//Only a const reference binds to a bit-field, through a temporary.  [dcl.init.ref] 5, [class.bit] 3
			return BitFieldError(pcInitialiser);
		}
		if (!psValue && !psType->bRvalue && !psType->psBase->bConst)
		{
			psValue = Value(pcInitialiser);
			if (psValue && !psValue->WithoutReference()->IsClass())
			{
				//T& binds to an lvalue.  [dcl.init.ref] 5
				return Error(pcInitialiser, "A reference that isn't const binds to an object: the value has no place to refer to");
			}
			return psValue != NULL;
		}
		return psValue ? CanBind(psType, psValue, pcNode) : (Value(pcInitialiser) != NULL);
	}
	psValue = ValueFor(pcInitialiser, psType, false);
	if (bDirect && psValue && psValue->WithoutReference()->IsNullPointer() && (psType->eKind == CPP65TK_Bool))
	{
		return true;
	}

	//What is wrong with the value is said at the value.
	return ImplicitConvert(psValue, psType, pcInitialiser, pcInitialiser) && (!bBraced || NotNarrowed(pcInitialiser, psType));
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a value of psFrom (NULL after an error) can be converted to psTo.  Any
// number or pointer converts to any other (a pointer to a derived class to a pointer
// to its base, an integer to a pointer), but not to or from an object, a void value
// or a pointer and a float.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::Convert(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode)
{
	int		iOffset;

	if (!psFrom || !psTo)
	{
		return false;
	}
	if (psTo->IsVoid())
	{
		return true;
	}
	if (psFrom->IsVoid())
	{
		return Error(pcNode, "A void value cannot be used");
	}
	if (psFrom->IsClass() && !psTo->IsClass())
	{
		//An object converts only by its class's conversion function: one that is explicit only by a cast or as a
		//condition; a lambda that captures has none to a pointer to a function.  [class.conv.fct] 2, [expr.prim.lambda] 6
		if (psFrom->pcClass->mpsLambda)
		{
			return Error(pcNode, "The lambda doesn't convert to the type: only one that captures nothing converts to a pointer to a function, and a generic one only where its call can be made for the function's parameters");
		}
		return Error(pcNode, "The object doesn't convert to the type: its class has no conversion function to it that isn't explicit");
	}
	if (psFrom->IsClass() || psTo->IsClass())
	{
		return Error(pcNode, "Objects can only be copied or used through pointers by the compiler yet");
	}
	if (!psFrom->IsScalar() || !psTo->IsScalar())
	{
		return Error(pcNode, "This conversion is not supported by the compiler yet");
	}
	if (!mpcUnit->Supported(psFrom, pcNode) || !mpcUnit->Supported(psTo, pcNode))
	{
		return false;
	}

	//A pointer to a derived class points at its base, which may be further on: the derived class is laid out.
	if (psFrom->IsPointer() && psTo->IsPointer() && psFrom->psBase->IsClass() && psTo->psBase->IsClass() && (psFrom->psBase->pcClass != psTo->psBase->pcClass) && mpcUnit->IsDerived(psFrom->psBase->pcClass, psTo->psBase->pcClass, NULL))
	{
		mpcUnit->CompleteClass(psFrom->psBase->pcClass, pcNode);
		return true;
	}

	//A pointer to a member of a base is a pointer to a member of a class derived from it, but not of a virtual base,
	//whose members are nowhere fixed in the class.  [conv.mem] 2
	if (psFrom->IsMemberPointer() && psTo->IsMemberPointer() && psFrom->pcClass && psTo->pcClass && (psFrom->pcClass != psTo->pcClass) && mpcUnit->CompleteClass(psTo->pcClass, pcNode) &&
		mpcUnit->IsDerived(psTo->pcClass, psFrom->pcClass, &iOffset) && (iOffset == CPP_VIRTUAL_PATH))
	{
		return Error(pcNode, "A pointer to a member of a virtual base can't be converted to a pointer to a member of a class derived from it");
	}
	if ((psFrom->IsFloat() || psTo->IsFloat()) && (psTo->eKind != CPP65TK_Bool) && (psFrom->IsPointer() || psTo->IsPointer()))
	{
		return Error(pcNode, "A pointer cannot be converted to or from floating point");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Convert, for a conversion no cast asks for: a pointer is made from and
// becomes only what it may (ConvertsPointer), it can't lose const or
// volatile (KeepsPointerQualifiers), an enum class's value stays one and
// an enum is given its own values (KeepsEnum), and std::nullptr_t converts
// only as it may (KeepsNullPointer).  pcValue is the expression converted,
// when there is one to look at.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::ImplicitConvert(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode, CCPPSyntax* pcValue)
{
	return Convert(psFrom, psTo, pcNode) && ConvertsPointer(psFrom, psTo, pcNode, pcValue) && KeepsPointerQualifiers(psFrom, psTo, pcValue ? pcValue : pcNode) && KeepsEnum(psFrom, psTo, pcNode) && KeepsNullPointer(psFrom, psTo, pcNode, pcValue);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psFrom converts to psTo with no cast as far as std::nullptr_t
// goes.  A nullptr_t becomes a pointer or a pointer to a member, both null
// ([conv.ptr] 1, [conv.mem] 1), and nothing else: not an integer, and a
// bool only where it is tested or directly initialises one ([conv.bool]),
// which doesn't come here.  Only a null pointer constant (nullptr, or a
// literal 0: pcValue) becomes a nullptr_t.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::KeepsNullPointer(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode, CCPPSyntax* pcValue)
{
	if (psTo->IsReference())
	{
		psTo = psTo->psBase;
	}
	if (psFrom->IsReference())
	{
		psFrom = psFrom->psBase;
	}
	if (psFrom->IsNullPointer())
	{
		if (psTo->IsNullPointer() || psTo->IsPointer() || psTo->IsMemberPointer())
		{
			return true;
		}
		return Error(pcNode, (psTo->eKind == CPP65TK_Bool) ? "nullptr is a bool only where it is tested or directly initialises one" : "nullptr converts only to a pointer or a pointer to a member");
	}
	if (psTo->IsNullPointer() && !(pcValue && mpcUnit->IsNullPointerConstant(pcValue)))
	{
		return Error(pcNode, "Only nullptr or a literal 0 converts to std::nullptr_t");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An operator with an enum class's value as an operand: recorded if every
// enum class among its operands is the one a refactoring is asking about
// (CCPPSemanticPass::SetRecordedEnum), so the check goes on.  False if
// one isn't: it is refused.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::RecordsEnumOperands(CCPPSyntax* pcNode, SCPP65Type* psLeft, SCPP65Type* psRight, uint8 uiKind)
{
	if ((psLeft->IsScopedEnum() && !mpcUnit->IsRecordedEnum(psLeft)) || (psRight->IsScopedEnum() && !mpcUnit->IsRecordedEnum(psRight)))
	{
		return false;
	}
	return mpcUnit->RecordEnumCast(pcNode, psLeft, psRight, uiKind);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psFrom converts to psTo with no cast as far as enums go: an
// enum class's value is only its own type's, and only an enum's own values
// are its (in C a number is an enum's value too).  [conv.prom] and
// [conv.integral] leave a scoped enum out, and convert nothing to an enum.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::KeepsEnum(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode)
{
	if (psTo->IsReference())
	{
		psTo = psTo->psBase;
	}
	if (psFrom->IsReference())
	{
		psFrom = psFrom->psBase;
	}
	if (psTo->IsVoid() || (psFrom->Unqualified() == psTo->Unqualified()))
	{
		return true;
	}
	if (psFrom->IsScopedEnum())
	{
		return mpcUnit->RecordEnumCast(pcNode, psFrom, psTo, CPPEC_TO_OTHER) || Error(pcNode, "An enum class's value becomes another type only by a cast");
	}
	if (psTo->IsScopedEnum())
	{
		return mpcUnit->RecordEnumCast(pcNode, psFrom, psTo, CPPEC_TO_ENUM) || Error(pcNode, "An enum class is given only its own values, or a cast");
	}
	if (psTo->pcEnum && !mpcUnit->IsCSource())
	{
		return Error(pcNode, "An enum is given only its own values, or a cast");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psFrom converts to psTo with no cast as far as pointers go (any
// other types do here).  Only a null pointer constant, pcValue, becomes a
// pointer or a pointer to a member; a pointer becomes no number but a bool;
// a pointer to an object becomes a void* and a pointer to a class one to
// its base, and no other pointer; a pointer to a base's member becomes one
// to a derived class's, and no other.  [conv.ptr], [conv.mem], [conv.bool]
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::ConvertsPointer(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode, CCPPSyntax* pcValue)
{
	bool	bFromPointer;
	bool	bToPointer;

	psFrom = psFrom->WithoutReference();
	psTo = psTo->WithoutReference();
	bFromPointer = psFrom->IsPointer() || psFrom->IsMemberPointer();
	bToPointer = psTo->IsPointer() || psTo->IsMemberPointer();
	if ((!bFromPointer && !bToPointer) || psFrom->IsNullPointer() || psTo->IsNullPointer() || !psFrom->IsScalar() || !psTo->IsScalar())
	{
		return true;
	}
	if (!bFromPointer)
	{
		return (psFrom->IsInteger() && (!pcValue || mpcUnit->IsNullPointerConstant(pcValue))) || Error(pcNode, "Only 0 or nullptr becomes a pointer without a cast");
	}
	if (!bToPointer)
	{
		return (psTo->eKind == CPP65TK_Bool) || Error(pcNode, "A pointer becomes a number only by a cast");
	}
	if (psFrom->IsPointer() && psTo->IsPointer())
	{
		if (psFrom->psBase->Equals(psTo->psBase) || (psTo->psBase->IsVoid() && !psFrom->psBase->IsFunction()))
		{
			return true;
		}
		if (psFrom->psBase->IsClass() && psTo->psBase->IsClass() && mpcUnit->IsDerived(psFrom->psBase->pcClass, psTo->psBase->pcClass, NULL))
		{
			return mpcUnit->GetBinder()->ReachesBase(psFrom->psBase->pcClass, psTo->psBase->pcClass, pcNode);
		}
		if (psFrom->psBase->IsVoid() && !psTo->psBase->IsFunction() && mpcUnit->IsCSource())
		{
			//C's void* is any object's pointer.  (C11 6.3.2.3 1)
			return true;
		}
		return ConversionError(pcNode, psFrom, psTo);
	}
	if (psFrom->IsMemberPointer() && psTo->IsMemberPointer() && psFrom->psBase->Equals(psTo->psBase) && mpcUnit->IsDerived(psTo->pcClass, psFrom->pcClass, NULL))
	{
		return mpcUnit->GetBinder()->ReachesBase(psTo->pcClass, psFrom->pcClass, pcNode);
	}
	return ConversionError(pcNode, psFrom, psTo);
}


//////////////////////////////////////////////////////////////////////////
//
// The error of a conversion that needs a cast, with the two types.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::ConversionError(CCPPSyntax* pcNode, SCPP65Type* psFrom, SCPP65Type* psTo)
{
	return TypesError(pcNode, "", psFrom, " becomes ", psTo, " only by a cast");
}


//////////////////////////////////////////////////////////////////////////
//
// An error that names two types, in the words around them.  Always false.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::TypesError(CCPPSyntax* pcNode, const char* szBefore, SCPP65Type* psFirst, const char* szBetween, SCPP65Type* psSecond, const char* szAfter)
{
	CChars			szError;
	CCPPTypeWriter	cWriter;

	cWriter.Init(mpcUnit, CPPTW_Shown);
	szError.Init(szBefore);
	cWriter.AppendType(psFirst, &szError);
	szError.Append(szBetween);
	cWriter.AppendType(psSecond, &szError);
	szError.Append(szAfter);
	Error(pcNode, szError.Text());
	szError.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a pointer psFrom as a psTo lets what is const or volatile be
// written through: what psTo points to has less of them, at some level.
// [conv.qual], [expr.const.cast] 8
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::LosesPointerQualifiers(SCPP65Type* psFrom, SCPP65Type* psTo)
{
	if (psFrom->psBase->Equals(psTo->psBase))
	{
		return !mpcUnit->GetBinder()->KeepsQualifiers(psTo, psFrom);
	}
	return !psTo->psBase->HasQualifiersOf(psFrom->psBase);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a named cast makes a conversion that is its own to make, the
// operand's value to psTo (a cast in brackets tries each of them in turn,
// so it makes them all).  const_cast changes only const and volatile;
// static_cast makes what no cast is needed for and those conversions' ways
// back, and reinterpret_cast the conversions between pointers and between
// a pointer and an integer; neither takes const away.  A cast to a
// reference or to a class, or of an object, is checked where it is made.
// [expr.const.cast], [expr.static.cast], [expr.reinterpret.cast]
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::NamedCastAllowed(CCPPSyntaxNamedCastExpression* pcCast, SCPP65Type* psTo)
{
	SCPP65Type*	psFrom;

	psFrom = ValueTypeOf(pcCast->mpcOperand);
	if (!psFrom || psTo->IsReference() || psTo->IsClass() || psTo->IsVoid() || psFrom->IsClass())
	{
		return (pcCast->meCast != CT_const_cast) || psTo->IsReference() || Error(pcCast, "const_cast makes a pointer, a pointer to a member or a reference");
	}
	psFrom = psFrom->Unqualified();
	psTo = psTo->Unqualified();
	switch (pcCast->meCast)
	{
	case CT_const_cast:
		if ((psTo->IsPointer() && psTo->psBase->IsFunction()) || (psFrom->IsPointer() && psFrom->psBase->IsFunction()))
		{
			//[expr.const.cast] 3
			return Error(pcCast, "const_cast doesn't convert a pointer to a function: a function has no const to take away");
		}
		return ConstCastAllowed(pcCast, psFrom, psTo);
	case CT_reinterpret_cast:
		return ReinterpretCastAllowed(pcCast, psFrom, psTo);
	default:
		return StaticCastAllowed(pcCast, psFrom, psTo);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether dynamic_cast<psTo>(x) may be made of an x that is a pointer to
// an object, or an object for a reference: it takes no const away, and a
// cast to a base of x's class is the conversion to it, which the base is
// in the class once and reached for.  [expr.dynamic.cast] 1, 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::DynamicCastAllowed(CCPPSyntaxNamedCastExpression* pcCast, SCPP65Type* psTo)
{
	SCPP65Type*	psFrom;

	psFrom = psTo->IsReference() ? Place(pcCast->mpcOperand) : ValueTypeOf(pcCast->mpcOperand);
	if (!psFrom)
	{
		return false;
	}
	if (!(psTo->IsPointer() && (psTo->psBase->IsClass() || psTo->psBase->IsVoid())) && !(psTo->IsReference() && psTo->psBase->IsClass()))
	{
		//[expr.dynamic.cast] 1
		return mpcDiagnostics->ErrorOnce(pcCast, "dynamic_cast needs a pointer to a class or to void");
	}
	if (psTo->IsPointer() && psFrom->IsPointer())
	{
		psFrom = psFrom->psBase;
	}
	else if (!psTo->IsReference())
	{
		return true;
	}
	psTo = psTo->psBase;
	if ((psTo->IsClass() && !mpcUnit->CompleteClass(psTo->pcClass, pcCast)) || (psFrom->IsClass() && !mpcUnit->CompleteClass(psFrom->pcClass, pcCast)))
	{
		//[expr.dynamic.cast] 1, 2
		return false;
	}
	if (psFrom->IsClass() && !(psTo->IsClass() && ((psTo->pcClass == psFrom->pcClass) || mpcUnit->IsDerived(psFrom->pcClass, psTo->pcClass, NULL))) && !psFrom->pcClass->IsPolymorphic())
	{
		//Anything but a cast to a base looks the object up by its vtable.  [expr.dynamic.cast] 6
		return mpcDiagnostics->ErrorOnce(pcCast, "The operand of dynamic_cast is not polymorphic");
	}
	if (!psTo->HasQualifiersOf(psFrom))
	{
		return Error(pcCast, "dynamic_cast doesn't take const away: const_cast does");
	}
	if (psFrom->IsClass() && psTo->IsClass() && mpcUnit->IsDerived(psFrom->pcClass, psTo->pcClass, NULL))
	{
		return mpcUnit->GetBinder()->ReachesBase(psFrom->pcClass, psTo->pcClass, pcCast);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// const_cast<T>(x): T is a pointer or a pointer to a member, and x's type
// but for const and volatile, which it changes at any level.  Not a pointer
// to a function.  [expr.const.cast] 1, 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::ConstCastAllowed(CCPPSyntaxNamedCastExpression* pcCast, SCPP65Type* psFrom, SCPP65Type* psTo)
{
	if (!(psTo->IsPointer() || psTo->IsMemberPointer()) || psTo->psBase->IsFunction())
	{
		return Error(pcCast, "const_cast makes a pointer, a pointer to a member or a reference");
	}
	if (!(psFrom->IsNullPointer() || psFrom->Equals(psTo)))
	{
		return Error(pcCast, "const_cast only changes const and volatile: the types are different");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// static_cast<T>(x) of numbers and pointers: not between a pointer and a
// number (but a null pointer constant, and a pointer as a bool), not
// between pointers to unrelated types (void* is related to every object's,
// a class's to its base's and to a class's derived from it), and not one
// that takes const away.  [expr.static.cast] 1, 4, 11, 13
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::StaticCastAllowed(CCPPSyntaxNamedCastExpression* pcCast, SCPP65Type* psFrom, SCPP65Type* psTo)
{
	SCPP65Type*	psFromBase;
	SCPP65Type*	psToBase;

	if (psFrom->IsNullPointer())
	{
		//nullptr becomes a pointer, a pointer to a member, itself, or a bool that is initialised directly from it
		//([conv.bool]): a number is reinterpret_cast's to make.  [expr.static.cast] 4
		return psTo->IsPointer() || psTo->IsMemberPointer() || psTo->IsNullPointer() || (psTo->eKind == CPP65TK_Bool) || Error(pcCast, "static_cast doesn't make a number from nullptr: reinterpret_cast does");
	}
	if ((psTo->IsPointer() || psTo->IsMemberPointer()) && !(psFrom->IsPointer() || psFrom->IsMemberPointer()))
	{
		return mpcUnit->IsNullPointerConstant(pcCast->mpcOperand) || Error(pcCast, "static_cast doesn't make a pointer from a number: reinterpret_cast does");
	}
	if ((psFrom->IsPointer() || psFrom->IsMemberPointer()) && !(psTo->IsPointer() || psTo->IsMemberPointer()))
	{
		return (psTo->eKind == CPP65TK_Bool) || Error(pcCast, "static_cast doesn't make a number from a pointer: reinterpret_cast does");
	}
	if (!psFrom->IsPointer() || !psTo->IsPointer())
	{
		return true;
	}

	psFromBase = psFrom->psBase;
	psToBase = psTo->psBase;
	if (LosesPointerQualifiers(psFrom, psTo))
	{
		return Error(pcCast, "static_cast doesn't take const away: const_cast does");
	}
	if (psFromBase->Equals(psToBase) || psFromBase->IsVoid() || (psToBase->IsVoid() && !psFromBase->IsFunction()) || ConditionalClass(psFromBase, psToBase))
	{
		return true;
	}
	return Error(pcCast, "static_cast doesn't convert between pointers to unrelated types: reinterpret_cast does");
}


//////////////////////////////////////////////////////////////////////////
//
// reinterpret_cast<T>(x) of numbers and pointers: a pointer as an integer
// big enough for it and back, a pointer as another pointer (to an object or
// to a function) and a pointer to a member as another, a std::nullptr_t as
// an integer, and a value as its own type.  Nothing else (no number as
// another number's type), and not one that takes const away.
// [expr.reinterpret.cast] 1, 2, 4 to 10
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::ReinterpretCastAllowed(CCPPSyntaxNamedCastExpression* pcCast, SCPP65Type* psFrom, SCPP65Type* psTo)
{
	bool	bAllowed;

	if (psFrom->IsPointer() && psTo->IsPointer())
	{
		return !LosesPointerQualifiers(psFrom, psTo) || Error(pcCast, "reinterpret_cast doesn't take const away: const_cast does");
	}
	if (psFrom->IsMemberPointer() && psTo->IsMemberPointer())
	{
		bAllowed = psFrom->psBase->IsFunction() == psTo->psBase->IsFunction();
	}
	else if (psFrom->IsPointer())
	{
		bAllowed = psTo->IsInteger() && (psTo->iBytes >= psFrom->iBytes) && (psTo->eKind != CPP65TK_Bool);
	}
	else if (psTo->IsPointer())
	{
		bAllowed = psFrom->IsInteger();
	}
	else if (psFrom->IsNullPointer())
	{
		bAllowed = psTo->IsInteger();
	}
	else
	{
		bAllowed = psFrom->IsInteger() && psFrom->Same(psTo);
	}
	return bAllowed || Error(pcCast, "reinterpret_cast doesn't make this conversion");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pointer psFrom converts to psTo (any other types do here) without
// losing const or volatile from what it points to, or a pointer to a member
// from the member.  A string literal's conversion to a char* is let
// through, with a warning if they are on.  [conv.qual]
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::KeepsPointerQualifiers(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode)
{
	bool	bKept;

	if (psFrom->IsMemberPointer() && psTo->IsMemberPointer())
	{
		return psTo->psBase->HasQualifiersOf(psFrom->psBase) || Error(pcNode, "This conversion takes const away from the member the pointer points to");
	}
	if (!psFrom->IsPointer() || !psTo->IsPointer())
	{
		return true;
	}
	bKept = !LosesPointerQualifiers(psFrom, psTo);
	if (!bKept && psFrom->psBase->Equals(psTo->psBase) && mpcUnit->IsStringLiteral(mpcUnit->SkipParentheses(pcNode)))
	{
		//char* p = "abc": a string literal's characters are const, and the conversion that let that be forgotten went
		//with C++11.  It is still made, as code written before is full of it, and is warned about when that is asked
		//for (CCPPCompileOptions::mbWarnStringToPointer).  [diff.cpp03.lex]
		if (mpcUnit->GetOptions()->mbWarnStringToPointer)
		{
			mpcDiagnostics->Warning(pcNode, "A string literal is const: this conversion to a pointer its characters can be written through is not C++14's");
		}
		return true;
	}
	return bKept || Error(pcNode, "This conversion takes const away from what the pointer points to");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a place can be changed: it isn't const.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::CanChange(SCPP65Type* psPlace, CCPPSyntax* pcNode)
{
	return !psPlace->bConst || Error(pcNode, "A const object cannot be changed");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcExpression is a bit-field (CCPPUnit::BitFieldOf), asked only
// of what can be one: a name the parser left as a field's (BitFieldOf binds
// a name in a quiet trial, and a variable's name is never one), a member
// after . or ->, and c ? x : y.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::IsBitField(CCPPSyntax* pcExpression)
{
	CCPPSymbol*	pcSymbol;

	pcExpression = mpcUnit->SkipParentheses(pcExpression);
	switch (pcExpression->GetKind())
	{
	case CPPS_IdExpression:
		pcSymbol = ((CCPPSyntaxIdExpression*)pcExpression)->mpcName->mpcSymbol;
		return pcSymbol && (pcSymbol->meKind == CPPSK_Field) && (mpcUnit->GetBinder()->BitFieldOf(pcExpression) != NULL);
	case CPPS_MemberExpression:
	case CPPS_ConditionalExpression:
		return mpcUnit->GetBinder()->BitFieldOf(pcExpression) != NULL;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A bit-field's address taken, or a reference that isn't const bound to it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::BitFieldError(CCPPSyntax* pcNode)
{
	return Error(pcNode, "A bit-field has no address: it can't be pointed to or bound to a reference that isn't const");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcValue is the kind of expression a reference of type
// psReference binds to: T&& only to what has no name or is about to go,
// or to the temporary a value of another type is converted to (long&& of
// an int, int&& of an object with operator int).  [dcl.init.ref] 5.2
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::BindsCategory(SCPP65Type* psReference, CCPPSyntax* pcValue, CCPPSyntax* pcNode)
{
	return !psReference->bRvalue || IsRvalue(pcValue) || mpcUnit->GetBinder()->BindsToTemporary(psReference, pcValue) || Error(pcNode, "A reference declared && cannot be bound to something with a name");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether reference psReference can bind to a place of type psPlace: the
// place is of the type referred to, or an object of a class derived from
// it (which has the base once, where it may be reached), and the reference
// has the place's const and volatile; or the place is an object whose
// conversion function gives what it binds to.  [dcl.init.ref] 4, 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::CanBind(SCPP65Type* psReference, SCPP65Type* psPlace, CCPPSyntax* pcNode)
{
	SCPP65Type*			psReferred;
	CCPPBoundConversion	cBound;

	psReferred = psReference->psBase;
	if (psPlace->IsArray() && psReferred->IsPointer())
	{
		return Error(pcNode, "An array cannot be bound to a reference to a pointer that isn't const");
	}
	if (psPlace->IsClass() && !(psReferred->IsClass() && mpcUnit->IsDerived(psPlace->pcClass, psReferred->pcClass, NULL)))
	{
		mpcUnit->GetBinder()->BindObjectConversion(psPlace->pcClass, psReference, false, pcNode, &cBound);
		return cBound.mpcFunction || TypesError(pcNode, "A reference to ", psReferred, " doesn't bind to ", psPlace, "");
	}
	if (!psReferred->HasQualifiersOf(psPlace))
	{
		return Error(pcNode, "A const object cannot be bound to a reference that is not const");
	}
	if (psPlace->IsClass())
	{
		return mpcUnit->GetBinder()->ReachesBase(psPlace->pcClass, psReferred->pcClass, pcNode);
	}
	if (psReferred->IsFunction())
	{
		//A function's name: the overload of the reference's type (FunctionAddress chooses it).
		return true;
	}
	if (psReferred->IsArray() && psPlace->IsPointer() && psReferred->psBase->Equals(psPlace->psBase))
	{
		//An array's place is typed as its first element's address.
		return true;
	}
	return psReferred->Equals(psPlace) || TypesError(pcNode, "A reference to ", psReferred, " doesn't bind to ", psPlace, "");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcValue can initialise a psTo (a variable, or what a function
// returns): as a pointer it is one that converts (ConvertsPointer), with
// the const and volatile it has.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::KeepsQualifiers(CCPPSyntax* pcValue, SCPP65Type* psTo)
{
	SCPP65Type*	psValue;

	psValue = ValueTypeOf(pcValue);
	if (!psValue)
	{
		return true;
	}
	if (psTo->IsReference())
	{
		if (!psTo->bRvalue && !psTo->psBase->bConst && !psValue->IsClass() && !psTo->psBase->IsFunction() && IsRvalue(pcValue))
		{
			//T& binds to an lvalue (or what a conversion function returns as one).  [dcl.init.ref] 5
			return Error(pcValue, "A reference that isn't const binds to an object: the value has no place to refer to");
		}
		return BindsCategory(psTo, pcValue, pcValue) && (mpcUnit->GetBinder()->BindsToTemporary(psTo, pcValue) || CanBind(psTo, psValue, pcValue));
	}
	psValue = psValue->IsArray() ? mpcTypes->Pointer(psValue->psBase) : psValue;
	if (!mpcUnit->FunctionNameFor(pcValue, psTo) && !ConvertsPointer(psValue, psTo, pcValue, pcValue))
	{
		//(A function's name for a pointer to a function is the overload of that type, whatever type it is alone.)
		return false;
	}
	return KeepsPointerQualifiers(psValue, psTo, pcValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::Error(CCPPSyntax* pcNode, const char* szMessage)
{
	return mpcDiagnostics->Error(pcNode, szMessage);
}


//////////////////////////////////////////////////////////////////////////
//
// What the types two pointers point to, or two pointers to members point
// to as members, have in common: one type with the const and volatile of
// both, where they are the same type but for those.  At the first level
// (bFirstLevel: what the pointers themselves point to) void with an object
// type is void, and a class with one it is derived from is the base.
// Pointers that differ further in point to what those have in common, and
// are const as const was added there.  NULL if they have nothing in
// common.  [expr] 13, [conv.qual] 4
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::CompositePointee(SCPP65Type* psLeft, SCPP65Type* psRight, bool bFirstLevel)
{
	SCPP65Type*	psCommon;
	SCPP65Type*	psInner;
	CCPPClass*	pcBase;
	bool		bConst;

	bConst = psLeft->bConst || psRight->bConst;
	pcBase = bFirstLevel ? ConditionalClass(psLeft, psRight) : NULL;
	psCommon = NULL;
	if (psLeft->Unqualified()->Same(psRight->Unqualified()))
	{
		psCommon = psLeft;
	}
	else if (bFirstLevel && (psLeft->IsVoid() || psRight->IsVoid()) && !psLeft->IsFunction() && !psRight->IsFunction())
	{
		psCommon = &mpcUnit->mcTypes.msVoid;
	}
	else if (pcBase)
	{
		psCommon = pcBase->mpsType;
	}
	else if (psLeft->IsPointer() && psRight->IsPointer())
	{
		psInner = CompositePointee(psLeft->psBase, psRight->psBase, false);
		psCommon = psInner ? mpcUnit->mcTypes.Pointer(psInner) : NULL;
		bConst = true;
	}
	return psCommon ? mpcUnit->mcTypes.Qualified(psCommon->Unqualified(), bConst, psLeft->bVolatile || psRight->bVolatile) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The composite pointer type of two operands: the type both are converted
// to before two pointers, or two pointers to members, are compared, and
// the type of c ? x : y of them.  A null pointer constant (a std::nullptr_t,
// or an integer, which its user has seen is one) with a pointer is the
// pointer's type and with a std::nullptr_t that; two pointers are a
// pointer to what they point to in common (CompositePointee); two
// pointers to members are the derived class's, to what the members are in
// common.  NULL if there is no such type.  [expr] 13
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::CompositePointerType(SCPP65Type* psLeft, SCPP65Type* psRight)
{
	SCPP65Type*	psPointee;
	CCPPClass*	pcDerived;
	bool		bLeftNull;
	bool		bRightNull;

	bLeftNull = psLeft->IsNullPointer() || psLeft->IsInteger();
	bRightNull = psRight->IsNullPointer() || psRight->IsInteger();
	if (bRightNull && (psLeft->IsPointer() || psLeft->IsMemberPointer() || psLeft->IsNullPointer()))
	{
		return psLeft->Unqualified();
	}
	if (bLeftNull && (psRight->IsPointer() || psRight->IsMemberPointer() || psRight->IsNullPointer()))
	{
		return psRight->Unqualified();
	}
	if (psLeft->IsPointer() && psRight->IsPointer())
	{
		psPointee = CompositePointee(psLeft->psBase, psRight->psBase, true);
		return psPointee ? mpcUnit->mcTypes.Pointer(psPointee) : NULL;
	}
	if (psLeft->IsMemberPointer() && psRight->IsMemberPointer())
	{
		//A pointer to a base's member is one to the derived class's.  [conv.mem] 2
		pcDerived = mpcUnit->IsDerived(psLeft->pcClass, psRight->pcClass, NULL) ? psLeft->pcClass : (mpcUnit->IsDerived(psRight->pcClass, psLeft->pcClass, NULL) ? psRight->pcClass : NULL);
		psPointee = CompositePointee(psLeft->psBase, psRight->psBase, false);
		return (pcDerived && psPointee) ? mpcUnit->mcTypes.MemberPointer(pcDerived, psPointee) : NULL;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of c ? x : y for x and y that aren't objects, NULL if they have
// none in common: void if either is; their own type where it is one type,
// which isn't promoted (two chars are a char, two bools a bool); the usual
// arithmetic conversions' for numbers and enums; otherwise their composite
// pointer type.  [expr.cond] 2, 6
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ConditionalType(SCPP65Type* psTrue, SCPP65Type* psFalse)
{
	if (psTrue->IsVoid() || psFalse->IsVoid())
	{
		return &mpcUnit->mcTypes.msVoid;
	}
	if (psTrue->Unqualified()->Same(psFalse->Unqualified()))
	{
		return psTrue->Unqualified();
	}
	if (psTrue->IsFloat() || psFalse->IsFloat())
	{
		return ((psTrue->IsFloat() || psTrue->IsInteger()) && (psFalse->IsFloat() || psFalse->IsInteger())) ? mpcUnit->mcTypes.FloatArithmetic(psTrue, psFalse) : NULL;
	}
	if (psTrue->IsInteger() && psFalse->IsInteger())
	{
		return mpcUnit->mcTypes.Arithmetic(psTrue, psFalse);
	}
	return CompositePointerType(psTrue, psFalse);
}


//////////////////////////////////////////////////////////////////////////
//
// The class of c ? x : y with objects: x's and y's if they are the same,
// otherwise the one the other is derived from (the derived one is converted
// to its base).  NULL if they aren't both objects, or neither class is the
// other's base.  [expr.cond] 4
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPP65TypeChecker::ConditionalClass(SCPP65Type* psTrue, SCPP65Type* psFalse)
{
	psTrue = psTrue->WithoutReference();
	psFalse = psFalse->WithoutReference();
	if (!psTrue->IsClass() || !psFalse->IsClass())
	{
		return NULL;
	}
	if (mpcUnit->IsDerived(psTrue->pcClass, psFalse->pcClass, NULL))
	{
		return psFalse->pcClass;
	}
	return mpcUnit->IsDerived(psFalse->pcClass, psTrue->pcClass, NULL) ? psTrue->pcClass : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the lvalue c ? x : y is: x and y are lvalues of one type (it
// has the const and volatile of both), or objects of a class and of one
// derived from it, the base no less const (the derived object is its base
// there).  NULL if c ? x : y is a value.  [expr.cond] 3, 4
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ConditionalLvalueType(CCPPSyntaxConditionalExpression* pcConditional)
{
	SCPP65Type*	psTrue;
	SCPP65Type*	psFalse;
	SCPP65Type*	psBase;
	CCPPClass*	pcClass;

	psTrue = PlaceOf(pcConditional->mpcTrue);
	psFalse = PlaceOf(pcConditional->mpcFalse);
	if (!psTrue || !psFalse || IsRvalue(pcConditional->mpcTrue) || IsRvalue(pcConditional->mpcFalse))
	{
		return NULL;
	}
	if (psTrue->Unqualified()->Same(psFalse->Unqualified()))
	{
		return mpcUnit->mcTypes.Qualified(psTrue->Unqualified(), psTrue->bConst || psFalse->bConst, psTrue->bVolatile || psFalse->bVolatile);
	}
	pcClass = ConditionalClass(psTrue, psFalse);
	if (!pcClass)
	{
		return NULL;
	}
	psBase = (psTrue->pcClass == pcClass) ? psTrue : psFalse;
	return psBase->HasQualifiersOf((psBase == psTrue) ? psFalse : psTrue) ? psBase : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of c ? x : y when x and y are both about to go and of one type,
// static_cast<T&&>(a) and a call that returns T&&: a T&& too.  NULL if they
// aren't.  [expr.cond] 4
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ConditionalXvalueType(CCPPSyntaxConditionalExpression* pcConditional)
{
	SCPP65Type*	psTrue;
	SCPP65Type*	psFalse;

	psTrue = DeclaredTypeOf(pcConditional->mpcTrue);
	psFalse = DeclaredTypeOf(pcConditional->mpcFalse);
	return (psTrue && psFalse && psTrue->IsRvalueReference() && psFalse->IsRvalueReference() && psTrue->psBase->Same(psFalse->psBase)) ? psTrue : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of c ? x : y when x or y is an object and the result is a value:
// the class of both; the base, when one's class is derived from the
// other's; or else the type of the one operand that the other converts to
// (an int for an object with a conversion function to it, the class for
// what its constructor takes).  NULL if neither converts, or each does
// (*pbAmbiguous).  [expr.cond] 3
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ConditionalObjectType(CCPPSyntaxConditionalExpression* pcConditional, bool* pbAmbiguous)
{
	SCPP65Type*	psTrue;
	SCPP65Type*	psFalse;
	CCPPClass*	pcClass;
	bool		bToTrue;
	bool		bToFalse;

	*pbAmbiguous = false;
	psTrue = ValueTypeOf(pcConditional->mpcTrue);
	psFalse = ValueTypeOf(pcConditional->mpcFalse);
	if (!psTrue || !psFalse || psTrue->IsVoid() || psFalse->IsVoid())
	{
		return NULL;
	}
	pcClass = ConditionalClass(psTrue, psFalse);
	if (pcClass)
	{
		return (psTrue->pcClass == pcClass) ? psTrue : psFalse;
	}
	bToTrue = CanInitialise(psTrue, pcConditional->mpcFalse);
	bToFalse = CanInitialise(psFalse, pcConditional->mpcTrue);
	*pbAmbiguous = bToTrue && bToFalse;
	if (bToTrue == bToFalse)
	{
		return NULL;
	}
	return (bToTrue ? psTrue : psFalse)->Unqualified();
}


//////////////////////////////////////////////////////////////////////////
//
// The type an argument with no parameter (one of a function's ...) is
// passed as, after the default argument promotions: a bool or an integer
// smaller than an int is promoted, a float is a double, an array or a
// function is a pointer to it and nullptr is a void*.  NULL for what isn't
// passed so (an object).  [expr.call] 7
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::VariadicArgumentType(SCPP65Type* psType)
{
	if (psType->IsFloat())
	{
		return &mpcUnit->mcTypes.msDouble;
	}
	if (psType->IsInteger())
	{
		return mpcUnit->mcTypes.Promote(psType);
	}
	if (psType->IsArray())
	{
		return mpcUnit->mcTypes.Pointer(psType->psBase);
	}
	if (psType->IsFunction())
	{
		return mpcUnit->mcTypes.Pointer(psType);
	}
	if (psType->IsNullPointer())
	{
		return mpcUnit->mcTypes.Pointer(&mpcUnit->mcTypes.msVoid);
	}
	return (psType->IsPointer() || psType->IsMemberPointer()) ? psType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of what a call is made through when it isn't a function by its
// name: a pointer to a function, which a function (what a reference to one
// refers to) is converted to.  NULL if the callee is neither.  [conv.func]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::CalleePointerType(CCPPSyntax* pcCallee)
{
	SCPP65Type*	psType;

	psType = TypeOf(pcCallee);
	if (psType && psType->IsFunction())
	{
		psType = mpcUnit->mcTypes.Pointer(psType);
	}
	return (psType && psType->IsPointer() && psType->psBase->IsFunction()) ? psType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of an expression (NULL if it is wrong), worked out without its code by
// CCPP65TypeChecker.  Its errors are dropped: they are reported where its code is
// written (ReportTypeOf).  An expression's type only depends on the template
// arguments and the function it is in, so it is remembered.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::TypeOf(CCPPSyntax* pcExpression)
{
	return Facts(pcExpression)->psType;
}


//////////////////////////////////////////////////////////////////////////
//
// TypeOf, a reference as the type it refers to (NULL if it is wrong).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ValueTypeOf(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = TypeOf(pcExpression);
	return psType ? psType->WithoutReference() : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type the expression is declared to give: TypeOf's, or the reference
// to it that a call of a function returning T& or T&&, or a cast to one,
// gives.  A variable declared as a reference is its own type here, as its
// name is the thing it refers to.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::DeclaredTypeOf(CCPPSyntax* pcExpression)
{
	return Facts(pcExpression)->psDeclared;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the place an lvalue is (an array stays an array), or NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::PlaceTypeOf(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	mpcUnit->mcDiagnostics.BeginQuiet();
	mpcUnit->mpcStatistics->Begin(CPPCP_Types);
	psType = Place(pcExpression);
	mpcUnit->mpcStatistics->End(CPPCP_Types);
	mpcUnit->mcDiagnostics.EndQuiet();
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of an operand that isn't evaluated and isn't converted as a
// value is (sizeof's and typeid's): an lvalue stays what it is, so an array
// (a variable, a field, a row of another array, a string) is not the
// pointer it would become, and a function is not a pointer to it.  A name
// is the thing it names (CCPPDeclarators::EntityType), which needs no
// object: S::m alone, and m in a static member function, are the member's
// type.  NULL if it is wrong.  [expr.sizeof] 4, [expr.typeid] 3,
// [expr.prim.general] 13
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::UnevaluatedTypeOf(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	mpcUnit->mcContext.EnterUnevaluated();
	pcExpression = mpcUnit->SkipParentheses(pcExpression);
	psType = mpcUnit->mcDeclarators.EntityType(pcExpression);
	if (!psType)
	{
		psType = PlaceTypeOf(pcExpression);
	}
	if (!psType)
	{
		psType = TypeOf(pcExpression);
	}
	mpcUnit->mcContext.LeaveUnevaluated();
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// Reports what is wrong with an expression whose type the IR front end needed
// and TypeOf couldn't work out, in quiet mode too: a call that no function
// matches, or that more than one does, is then an error where it is ("No
// function matches the arguments": an unsigned short* passed for an unsigned
// long*), not only "doesn't support yet".  The errors are the ones TypeOf
// kept when it worked the type out: nothing is worked out again.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::ReportTypeOf(CCPPSyntax* pcExpression)
{
	mpcUnit->mcDiagnostics.Report(Facts(pcExpression)->pcErrors);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the expression is an rvalue: a value with no name (3, a + b, an
// object a call or a constructor makes) or something said to be about to
// go (what a function returning T&& returns, a cast to T&&, a member of
// either).  What has a name is an lvalue, a variable declared T&& too,
// but for a local object where it is returned.  An rvalue binds to T&&
// and not to T&; an lvalue to T& and not to T&&.  The expression's
// category is the type checker's (CCPP65TypeChecker::Category).
// [basic.lval], [class.copy] 32
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::IsRvalue(CCPPSyntax* pcExpression)
{
	mpcUnit->mpcStatistics->Count(CPPCC_IsRvalue);
	pcExpression = CommaResult(pcExpression);
	if ((pcExpression->GetKind() == CPPS_IdExpression) && IsReturnedLocal((CCPPSyntaxIdExpression*)pcExpression))
	{
		return true;
	}
	return Category(pcExpression) != CPPVC_Lvalue;
}


//////////////////////////////////////////////////////////////////////////
//
// An expression of a class type that is an object where it is (named,
// pointed at, indexed, or a reference to one), not one to be made: an
// lvalue or an xvalue (CCPP65TypeChecker::Category).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::IsObjectLvalue(CCPPSyntax* pcExpression)
{
	ECPPValueCategory	eCategory;

	mpcUnit->mpcStatistics->Count(CPPCC_IsObjectLvalue);
	eCategory = Category(pcExpression);
	return (eCategory == CPPVC_Lvalue) || (eCategory == CPPVC_Xvalue);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the expression makes an object: a class prvalue (a call, a
// cast or an operator function's call that gives a class by value, a
// construction, a lambda).  CCPP65TypeChecker::Category.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::MakesObject(CCPPSyntax* pcExpression)
{
	mpcUnit->mpcStatistics->Count(CPPCC_MakesObject);
	return Category(pcExpression) == CPPVC_ClassPrvalue;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the expression makes an object of the class (a call, or an operator, that
// returns one): it can be made where the object it initialises is.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::IsMadeObject(CCPPSyntax* pcExpression, CCPPClass* pcClass)
{
	SCPP65Type*	psType;

	pcExpression = mpcUnit->SkipParentheses(pcExpression);
	if (!pcExpression || !MakesObject(pcExpression))
	{
		return false;
	}
	psType = TypeOf(pcExpression);
	return psType && psType->IsClass() && (psType->pcClass == pcClass);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the expression is an object (or a reference to one).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::IsObject(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	mpcUnit->mpcStatistics->Count(CPPCC_IsObject);
	psType = ValueTypeOf(pcExpression);
	return psType && psType->IsClass();
}


//////////////////////////////////////////////////////////////////////////
//
// The class of an object expression (see IsObject).
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPP65TypeChecker::ObjectClass(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = ValueTypeOf(pcExpression);
	return psType->pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the place a reference to the expression would refer to, or
// NULL if it is a value with no address (a literal, a sum, a call's int).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::PlaceOf(CCPPSyntax* pcExpression)
{
	mpcUnit->mpcStatistics->Count(CPPCC_PlaceOf);
	return PlaceFact(pcExpression);
}


//////////////////////////////////////////////////////////////////////////
//
// The expression whose value, and whether it is an lvalue, an expression
// has: itself out of its brackets, and of a, b (the built-in comma) b,
// whatever is done first.  [expr.comma] 1
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPP65TypeChecker::CommaResult(CCPPSyntax* pcExpression)
{
	CCPPSyntaxBinaryExpression*	pcBinary;

	for (;;)
	{
		pcExpression = mpcUnit->SkipParentheses(pcExpression);
		pcBinary = (pcExpression->GetKind() == CPPS_BinaryExpression) ? (CCPPSyntaxBinaryExpression*)pcExpression : NULL;
		if (!pcBinary || (pcBinary->meOperator != CT_Comma) || mpcUnit->GetBinder()->IsOverloadedComma(pcBinary))
		{
			return pcExpression;
		}
		pcExpression = pcBinary->mpcRight;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if pcId is return object; of one of the function's own objects (a
// local or a parameter, not a reference): it is going, so it is moved to
// the result if its class has a move.  [class.copy] 32
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::IsReturnedLocal(CCPPSyntaxIdExpression* pcId)
{
	CCPPSyntax*		pcParent;
	CCPPBoundName	cBound;

	for (pcParent = pcId->mpcParent; pcParent && (pcParent->GetKind() == CPPS_ParenthesisedExpression); pcParent = pcParent->mpcParent)
	{
	}
	if (!pcParent || (pcParent->GetKind() != CPPS_ReturnStatement))
	{
		return false;
	}
	mpcUnit->mcBinder.BindName(pcId, &cBound);
	return (cBound.meWhat == CPPBN_Variable) && ((cBound.mpcVariable->meStorage == CPPVS_Local) || (cBound.mpcVariable->meStorage == CPPVS_Parameter)) && cBound.mpcVariable->mpsType->IsClass();
}


//////////////////////////////////////////////////////////////////////////
//
// The type sizeof or typeid is asked of: its type-id's (pcType), or its
// expression's, which isn't evaluated.  decltype(d)::name, which the parser
// takes for a type where one can be written (only the type of d says which
// it is), is the member it names when it names no type.  NULL after an
// error.  [expr.sizeof] 1, [expr.typeid] 3, 4
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::OperandType(CCPPSyntaxTypeId* pcType, CCPPSyntax* pcOperand)
{
	SCPP65Type*	psType;
	CCPPSyntax*	pcMember;

	if (!pcType)
	{
		return mpcUnit->mcTypeChecker.UnevaluatedTypeOf(pcOperand);
	}
	pcMember = mpcUnit->mcDesugarings.DecltypeMember(pcType);
	if (!pcMember)
	{
		return mpcUnit->mcDeclarators.GetTypeIdType(pcType);
	}
	mpcUnit->mcDiagnostics.BeginSilent();
	psType = mpcUnit->mcDeclarators.GetTypeIdType(pcType);
	mpcUnit->mcDiagnostics.EndSilent();
	return psType ? psType : mpcUnit->mcTypeChecker.UnevaluatedTypeOf(pcMember);
}


//////////////////////////////////////////////////////////////////////////
//
// The type the value of pcExpression, a psType, has where integral
// promotion is applied to it (an operator's operand, an argument that has
// no parameter, a switch's condition): its own, but for a bit-field, which
// is promoted by its width and not by its type.  An unsigned field of
// three bits is an int there, so that field - 4 is less than 0.
// [conv.prom] 3
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::OperandType(SCPP65Type* psType, CCPPSyntax* pcExpression)
{
	SCPPField*	psField;

	psField = psType->IsInteger() ? mpcUnit->mcBinder.BitFieldOf(pcExpression) : NULL;
	return psField ? mpcUnit->mcTypes.PromoteBitField(psType, psField->iBitWidth) : psType;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a type is an aggregate: an array, or a class with no constructor
// of its own, no base and no virtual function, whose members a list in
// braces gives values to.  [dcl.init.aggr] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::IsAggregate(SCPP65Type* psType)
{
	CCPPClass*	pcClass;
	bool		bComplete;

	if (psType->IsArray())
	{
		return true;
	}
	if (!psType->IsClass())
	{
		return false;
	}
	pcClass = psType->pcClass;
	mpcUnit->mcDiagnostics.BeginQuiet();
	bComplete = mpcUnit->CompleteClass(pcClass, NULL);
	mpcUnit->mcDiagnostics.EndQuiet();
	return bComplete && !pcClass->mbConstructors && !pcClass->mpsLambda && (pcClass->macBases.NumElements() == 0) && !pcClass->IsPolymorphic();
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the member of an aggregate that its list's value number
// uiIndex is for: an array's element, a class's field in the order they are
// declared (a union's first only), bit-fields with no name left out.  NULL
// past the last.  An array of unknown bound has an element for every value.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::AggregateMember(SCPP65Type* psType, size uiIndex)
{
	SCPPField*	psField;

	if (psType->IsArray())
	{
		return ((psType->iCount == 0) || (uiIndex < (size)psType->iCount)) ? psType->psBase : NULL;
	}
	psField = psType->pcClass->AggregateField(uiIndex);
	return psField ? psField->psType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The class std::type_info, if the program has declared it (<typeinfo>), or
// NULL.  [type.info]
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPP65TypeChecker::TypeInfoClass(void)
{
	CCPPClass*	pcClass;
	size		i;

	for (i = 0; !mpcUnit->mpcTypeInfo && (i < mpcUnit->mapcClasses.NumElements()); i++)
	{
		pcClass = mpcUnit->mapcClasses.GetPtr(i);
		if (!pcClass->mpcTemplate && pcClass->mpcSymbol && mpcUnit->mcBinder.IsStdName(pcClass->mlliName, pcClass->mpcSymbol->mpcScope, "type_info"))
		{
			mpcUnit->mpcTypeInfo = pcClass;
		}
	}
	return mpcUnit->mpcTypeInfo;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function is a member of std::type_info that the program declares
// and nothing defines (<typeinfo>'s), which the IR front end works out where
// it is called (CCPP65IRFrontEnd::TypeInfoCall).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::IsTypeInfoMember(CCPPFunction* pcFunction)
{
	return pcFunction->mpcClass && (pcFunction->mpcClass == TypeInfoClass()) && !pcFunction->mpcDefinition && (pcFunction->meKind == CPPFK_Function);
}


//////////////////////////////////////////////////////////////////////////
//
// The type typeid gives the std::type_info of: a type's, or an expression's
// (its type without a reference), without const and volatile.  *pbDynamic
// when it is the object's own (an lvalue of a polymorphic class), found
// from its vtable.  [expr.typeid]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::TypeidType(CCPPSyntaxTypeidExpression* pcTypeid, bool* pbDynamic)
{
	SCPP65Type*	psType;

	*pbDynamic = false;
	psType = OperandType(pcTypeid->mpcType, pcTypeid->mpcOperand);
	psType = psType ? psType->WithoutReference() : NULL;
	if (!pcTypeid->mpcType)
	{
		*pbDynamic = psType && psType->IsClass() && mpcUnit->CompleteClass(psType->pcClass, pcTypeid) && psType->pcClass->IsPolymorphic() && mpcUnit->mcTypeChecker.IsObjectLvalue(pcTypeid->mpcOperand);
	}
	return psType ? psType->Unqualified() : NULL;
}

