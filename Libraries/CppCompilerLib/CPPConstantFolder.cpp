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
#include <string.h>
#include <math.h>
#include "CTInt.h"
#include "CTShort.h"
#include "CTLongLong.h"
#include "CTChar.h"
#include "CTBoolean.h"
#include "CTString.h"
#include "CTIdentifier.h"
#include "CTUserLiteral.h"
#include "CppTokeniser.h"
#include "CPP65IRFrontEnd.h"
#include "CPPSyntaxTree.h"
#include "CPPConstants.h"
#include "CPPSemanticPass.h"
#include "CPPTypeWriter.h"
#include "LiteralHelper.h"
#include "CTFloat.h"
#include "CTDouble.h"
#include "CTLongDouble.h"
#include "CPPUnit.h"
#include "CPPConstantFolder.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPConstantFolder::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPConstantFolder::Kill(void)
{
	mpcUnit = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of a constant expression, of an integer or a floating point
// type: what the syntax tree folds to (Fold), or else what running it
// gives (RunConstant), which is an integer's.  False if it isn't a
// constant.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstantFolder::Evaluate(CCPPSyntax* pcExpression, SCPPConstant* psValue)
{
	mpcUnit->mpcConstants->ClearReason();
	mpcUnit->mpcStatistics->Count(CPPCC_Constants);
	if (Fold(pcExpression, psValue))
	{
		mpcUnit->mpcStatistics->Count(CPPCC_ConstantsFolded);
		return true;
	}
	psValue->psFloat = NULL;
	if (RunConstant(pcExpression, &psValue->lliValue))
	{
		mpcUnit->mpcStatistics->Count(CPPCC_ConstantsRun);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of a constant expression of an integer type (Evaluate).
// False if it isn't a constant, or is a floating point one.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstantFolder::Constant(CCPPSyntax* pcExpression, int64* plliValue)
{
	SCPPConstant	sValue;

	if (!Evaluate(pcExpression, &sValue) || sValue.psFloat)
	{
		return false;
	}
	*plliValue = sValue.lliValue;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of a constant expression of a floating point type, worked out
// as the target would: each float result is rounded to float.  Rounding
// a double result of float operands to float rounds correctly, as a
// double has more than twice a float's precision.  False if it isn't a
// constant, or is an integer one.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstantFolder::FloatConstant(CCPPSyntax* pcExpression, double* pdValue)
{
	SCPPConstant	sValue;

	mpcUnit->mpcStatistics->Count(CPPCC_FloatConstants);
	if (!Fold(pcExpression, &sValue) || !sValue.psFloat)
	{
		return false;
	}
	mpcUnit->mpcStatistics->Count(CPPCC_FloatConstantsFolded);
	*pdValue = sValue.dValue;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Works out a constant expression from its syntax tree, an integer or a
// floating point one (SCPPConstant), the one walk for both.  An integer
// is kept as its type keeps it (ConstantOfType): unsigned and narrow
// arithmetic wraps.  What it is made of is worked out by Evaluate: what
// the tree can't give may be run.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstantFolder::Fold(CCPPSyntax* pcExpression, SCPPConstant* psValue)
{
	CCPPSyntaxUnaryExpression*		pcUnary;
	CCPPSyntaxBinaryExpression*		pcBinary;
	CCPPSyntaxConditionalExpression*	pcConditional;
	CCPPSyntaxSizeofExpression*		pcSizeof;
	CCPPSyntaxCastExpression*		pcCast;
	CCPPSyntaxNamedCastExpression*	pcNamedCast;
	CCPPSyntaxConstructExpression*	pcConstruct;
	CCPPSyntaxImplicitArgument*		pcImplicit;
	CArraySyntax*					papcElements;
	SCPP65Type*						psType;
	SCPPLiteralCall*				psCall;
	SCPPConstant					sLeft;
	SCPPConstant					sRight;
	CCPPToken*						pcToken;
	double							dValue;

	pcExpression = mpcUnit->SkipParentheses(pcExpression);
	if (!pcExpression)
	{
		return false;
	}
	psValue->SetInteger(0);

	switch (pcExpression->GetKind())
	{
	case CPPS_ImplicitArgument:
		pcImplicit = (CCPPSyntaxImplicitArgument*)pcExpression;
		if (!pcImplicit->mbConstant)
		{
			return false;
		}
		if (pcImplicit->mpsType->IsFloat())
		{
			psValue->psFloat = pcImplicit->mpsType;
			psValue->dValue = pcImplicit->mdValue;
			return true;
		}
		psValue->lliValue = pcImplicit->mlliValue;
		return true;

	case CPPS_LiteralExpression:
	case CPPS_StringExpression:
		if (mpcUnit->GetDesugarings()->UserLiteralToken(pcExpression))
		{
			//12_km and "abc"_len are their literal operators' calls.  [lex.ext] 2
			psCall = mpcUnit->GetDesugarings()->GetLiteralCall(pcExpression);
			return psCall && ConstexprResult(psCall->pcFunction, pcExpression, &psValue->lliValue);
		}
		if (pcExpression->GetKind() == CPPS_StringExpression)
		{
			return false;
		}
		pcToken = mpcUnit->mpcTokeniser->Get(((CCPPSyntaxLiteralExpression*)pcExpression)->miLiteral);
		if (TokenFloat(pcToken, &dValue, &psType))
		{
			psValue->psFloat = psType;
			psValue->dValue = dValue;
			return true;
		}
		return TokenConstant(pcToken, &psValue->lliValue);

	case CPPS_IdExpression:
		if (NameConstant((CCPPSyntaxIdExpression*)pcExpression, &psValue->lliValue))
		{
			return true;
		}
		return ConstexprFloat((CCPPSyntaxIdExpression*)pcExpression, psValue);

	case CPPS_UnaryExpression:
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		if (!Evaluate(pcUnary->mpcOperand, &sLeft))
		{
			return false;
		}
		if (sLeft.psFloat)
		{
			if ((pcUnary->meOperator != CT_Subtract) && (pcUnary->meOperator != CT_Add))
			{
				return false;
			}
			psValue->SetFloat(sLeft.psFloat, (pcUnary->meOperator == CT_Subtract) ? -sLeft.dValue : sLeft.dValue);
			return true;
		}
		return UnaryConstant(pcUnary, sLeft.lliValue, &psValue->lliValue);

	case CPPS_BinaryExpression:
		pcBinary = (CCPPSyntaxBinaryExpression*)pcExpression;
		if (!Evaluate(pcBinary->mpcLeft, &sLeft))
		{
			return false;
		}
		if (!sLeft.psFloat && (((pcBinary->meOperator == CT_LogicalAnd) && (sLeft.lliValue == 0)) || ((pcBinary->meOperator == CT_LogicalOr) && (sLeft.lliValue != 0))))
		{
			//The right isn't worked out, so needn't be a constant.  [expr.const] 2
			psValue->lliValue = (sLeft.lliValue != 0);
			return true;
		}
		if (!Evaluate(pcBinary->mpcRight, &sRight))
		{
			return false;
		}
		if (sLeft.psFloat || sRight.psFloat)
		{
			return BinaryFloat(pcBinary->meOperator, &sLeft, &sRight, psValue);
		}
		return BinaryConstant(pcBinary, sLeft.lliValue, sRight.lliValue, &psValue->lliValue);

	case CPPS_ConditionalExpression:
		pcConditional = (CCPPSyntaxConditionalExpression*)pcExpression;
		if (!Evaluate(pcConditional->mpcCondition, &sLeft) || sLeft.psFloat)
		{
			return false;
		}
		return Evaluate(sLeft.lliValue ? pcConditional->mpcTrue : pcConditional->mpcFalse, psValue);

	case CPPS_CastExpression:
		pcCast = (CCPPSyntaxCastExpression*)pcExpression;
		return CastConstant(pcCast->mpcType, pcCast->mpcOperand, psValue);

	case CPPS_NamedCastExpression:
		//static_cast<int>(3 > 2), as (int)(3 > 2) is.  No other named cast of a number is a constant ([expr.const] 2).
		pcNamedCast = (CCPPSyntaxNamedCastExpression*)pcExpression;
		return (pcNamedCast->meCast == CT_static_cast) && CastConstant(pcNamedCast->mpcType, pcNamedCast->mpcOperand, psValue);

	case CPPS_ConstructExpression:
		pcConstruct = (CCPPSyntaxConstructExpression*)pcExpression;
		psType = mpcUnit->mcDeclarators.GetTypeIdType(pcConstruct->mpcType);
		papcElements = mpcUnit->ConstructElements(pcConstruct);
		if (!psType || !(psType->IsInteger() || psType->IsPointer()) || !papcElements || (papcElements->NumElements() > 1))
		{
			return false;
		}
		if ((papcElements->NumElements() == 1) && (!Evaluate(papcElements->GetPtr(0), &sLeft) || sLeft.psFloat))
		{
			return false;
		}
		psValue->lliValue = ConstantOfType(psType, (papcElements->NumElements() == 1) ? sLeft.lliValue : 0);
		return true;

	case CPPS_TypeTraitExpression:
		return mpcUnit->TypeTrait((CCPPSyntaxTypeTraitExpression*)pcExpression, &psValue->lliValue);

	case CPPS_NoexceptExpression:
		psValue->lliValue = !mpcUnit->GetExceptions()->MayThrow(((CCPPSyntaxNoexceptExpression*)pcExpression)->mpcOperand);
		return true;

	case CPPS_CallExpression:
		if (FloatBuiltin(pcExpression, &dValue, &psType))
		{
			psValue->psFloat = psType;
			psValue->dValue = dValue;
			return true;
		}
		return ConstexprCall((CCPPSyntaxCallExpression*)pcExpression, &psValue->lliValue);

	case CPPS_SizeofExpression:
		pcSizeof = (CCPPSyntaxSizeofExpression*)pcExpression;
		if (pcSizeof->mbPack)
		{
			return mpcUnit->PackSize(pcSizeof->mpcPackSymbol, &psValue->lliValue);
		}
		if ((pcSizeof->meKeyword != CT_sizeof) && (pcSizeof->meKeyword != CT_alignof))
		{
			return false;
		}
		psType = mpcUnit->OperandType(pcSizeof->mpcType, pcSizeof->mpcOperand);
		if (!psType || psType->IsVoid() || psType->IsFunction() || (psType->IsArray() && (psType->iCount == 0)))
		{
			//Nothing that has no size: void, a function, an array of unknown bound.  [expr.sizeof] 1
			return false;
		}
		psType = psType->WithoutReference();
		if (psType->ElementType()->IsClass() && !mpcUnit->CompleteClass(psType->ElementType()->pcClass, pcExpression))
		{
			return false;
		}
		psValue->lliValue = (pcSizeof->meKeyword == CT_alignof) ? mpcUnit->ObjectAlignment(psType) : psType->iBytes;
		return true;

	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// left op right where either is a floating point constant: + - * / in
// their common floating point type, rounded as it keeps it.  Any other
// operator isn't one the folder works out.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstantFolder::BinaryFloat(ECppTokens eOperator, SCPPConstant* psLeft, SCPPConstant* psRight, SCPPConstant* psValue)
{
	double	dLeft;
	double	dRight;

	dLeft = psLeft->AsDouble();
	dRight = psRight->AsDouble();
	switch (eOperator)
	{
	case CT_Add:		psValue->SetFloat(mpcUnit->mcTypes.FloatArithmetic(psLeft->psFloat, psRight->psFloat), dLeft + dRight);	return true;
	case CT_Subtract:	psValue->SetFloat(mpcUnit->mcTypes.FloatArithmetic(psLeft->psFloat, psRight->psFloat), dLeft - dRight);	return true;
	case CT_Multiply:	psValue->SetFloat(mpcUnit->mcTypes.FloatArithmetic(psLeft->psFloat, psRight->psFloat), dLeft * dRight);	return true;
	case CT_Divide:		psValue->SetFloat(mpcUnit->mcTypes.FloatArithmetic(psLeft->psFloat, psRight->psFloat), dLeft / dRight);	return true;
	default:			return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The constant a cast of a constant is: to an integer or a pointer type,
// (int)(3 > 2), static_cast<char>(300), and (int)1.5, a floating point
// constant with the part after its point taken away ([conv.fpint] 1); to
// a floating point type, (float)1.1 rounded to a float.  False if it
// isn't one the folder works out.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstantFolder::CastConstant(CCPPSyntaxTypeId* pcType, CCPPSyntax* pcOperand, SCPPConstant* psValue)
{
	SCPP65Type*		psType;
	SCPPConstant	sOperand;

	psType = mpcUnit->mcDeclarators.GetTypeIdType(pcType);
	if (!psType || !Evaluate(pcOperand, &sOperand))
	{
		return false;
	}
	if (psType->IsFloat())
	{
		psValue->SetFloat(psType, sOperand.AsDouble());
		return true;
	}
	if (!(psType->IsInteger() || psType->IsPointer()) || (sOperand.psFloat && !psType->IsInteger()))
	{
		return false;
	}
	psValue->SetInteger(ConstantOfType(psType, sOperand.psFloat ? (int64)sOperand.dValue : sOperand.lliValue));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of a name that is a floating point variable declared constexpr
// (a const one isn't a constant: [expr.const] 2), which is its
// initialiser's as its type.  False for any other name.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstantFolder::ConstexprFloat(CCPPSyntaxIdExpression* pcId, SCPPConstant* psValue)
{
	CCPPSymbol*		pcSymbol;
	CCPPVariable*	pcVariable;
	CCPPSyntax*		pcInitialiser;
	SCPPConstant	sInitialiser;
	CCPPBoundName	cBound;
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;

	//S::d, a static member of a class (a field's name in it), is the variable its name is bound to.
	pcSymbol = pcId->mpcName->mpcSymbol;
	if (pcSymbol && (pcSymbol->meKind == CPPSK_Variable))
	{
		pcVariable = mpcUnit->GetVariable(pcSymbol);
	}
	else
	{
		mpcUnit->mcBinder.BindName(pcId, &cBound);
		pcVariable = (cBound.meWhat == CPPBN_Variable) ? cBound.mpcVariable : NULL;
	}
	if (!pcVariable || !pcVariable->mpsType->IsFloat())
	{
		return false;
	}
	//Its declaration says constexpr; the initialiser is there or, for a static member, in its class.
	pcInit = mpcUnit->GetInstantiator()->VariableDeclarator(pcVariable);
	pcDeclaration = pcInit ? pcInit->GetDeclaration() : NULL;
	if (!pcDeclaration || !pcDeclaration->mpcSpecifiers || !(pcDeclaration->mpcSpecifiers->muiFlags & CPPDS_CONSTEXPR))
	{
		return false;
	}
	pcInitialiser = pcInit->mpcInitializer ? pcInit->mpcInitializer : pcVariable->mpcInClassInitialiser;
	if (!pcInitialiser)
	{
		return false;
	}
	if ((pcInitialiser->GetKind() == CPPS_InitializerList) && (mpcUnit->ListElements(pcInitialiser)->NumElements() == 1))
	{
		pcInitialiser = mpcUnit->ListElements(pcInitialiser)->GetPtr(0);
	}
	if (!Evaluate(pcInitialiser, &sInitialiser))
	{
		return false;
	}
	psValue->SetFloat(pcVariable->mpsType->Unqualified(), sInitialiser.AsDouble());
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The integer a name is the constant of: an enumerator, a static const
// member, a variable template's instance, a template's value parameter,
// or a const integer variable initialised with a constant.  False for any
// other name.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstantFolder::NameConstant(CCPPSyntaxIdExpression* pcId, int64* plliValue)
{
	CCPPSymbol*						pcSymbol;
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSyntax*						pcInitialiser;
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	CCPPVariable*					pcVariable;
	SCPPBinding*					psBinding;
	CCPPTemplateArguments*			pcBindings;
	CCPPClass*						pcClass;
	CCPPSymbol*						pcEnumerator;
	CCPPSymbol*						pcMember;
	bool							bResult;

	pcSymbol = pcId->mpcName->mpcSymbol;
	if (!pcSymbol || (pcSymbol->meKind == CPPSK_Enumerator))
	{
		mpcUnit->mbValueArgumentRefused = false;
		pcEnumerator = mpcUnit->QualifiedEnumerator(pcId->mpcName, pcId, &pcClass);
		if (pcEnumerator)
		{
			mpcUnit->mcContext.EnterBindings(pcClass ? pcClass->mpcBindings : mpcUnit->mcContext.GetBindings());
			bResult = mpcUnit->EnumeratorValue(pcEnumerator, plliValue);
			mpcUnit->mcContext.LeaveBindings();
			return bResult;
		}
		if (mpcUnit->mbValueArgumentRefused)
		{
			//Count<-1>::V: there is no such class for V to be the enumerator of.
			return false;
		}
	}

	//A static const member: SNarrow::Max, SIZE::Max with SIZE bound to a class, or Max in its class.  Its
	//initialiser is worked out with the bindings of the class that declares it.
	pcClass = NULL;
	if (!pcSymbol || (pcSymbol->meKind == CPPSK_Field))
	{
		pcMember = mpcUnit->QualifiedMember(pcId->mpcName, CPPSK_Field, pcId, &pcClass);
		if (pcMember)
		{
			pcSymbol = pcMember;
		}
		else
		{
			pcClass = NULL;
		}
	}
	if (!pcSymbol)
	{
		return false;
	}
	if (pcSymbol->meKind == CPPSK_Enumerator)
	{
		return mpcUnit->EnumeratorValue(pcSymbol, plliValue);
	}
	if (pcSymbol->meKind == CPPSK_VariableTemplate)
	{
		return mpcUnit->VariableTemplateValue(pcId, plliValue);
	}
	if (pcSymbol->meKind == CPPSK_ValueParameter)
	{
		psBinding = mpcUnit->GetInstantiator()->FindBinding(pcSymbol);
		if (!psBinding || psBinding->pcArgument)
		{
			return false;
		}
		*plliValue = psBinding->lliValue;
		return true;
	}

	//A const integer initialised with a constant, or a static const member initialised in its class, or after it
	//by its definition (const int S::k = 8;) once that is read.
	pcInit = (CCPPSyntaxInitDeclarator*)pcSymbol->mpcDeclaration;
	if (((pcSymbol->meKind != CPPSK_Variable) && (pcSymbol->meKind != CPPSK_Field)) || !pcInit || (pcInit->GetKind() != CPPS_InitDeclarator))
	{
		return false;
	}
	pcInitialiser = pcInit->mpcInitializer;
	pcBindings = pcClass ? pcClass->mpcBindings : NULL;
	if (!pcInitialiser && (pcSymbol->meKind == CPPSK_Field))
	{
		pcVariable = pcClass ? mpcUnit->FindStatic(pcClass, pcSymbol->mlliName) : mpcUnit->GetVariable(pcSymbol);
		pcInitialiser = pcVariable ? pcVariable->mpcOutsideInitialiser : NULL;
		if (!pcInitialiser && pcClass && pcClass->mpcTemplate)
		{
			//template<class T> const int W<T>::k = 3; is each instance's, defined with the unit's data.
			pcInitialiser = mpcUnit->GetInstantiator()->InstanceStaticInitialiser(pcClass, pcSymbol, &pcBindings);
		}
	}
	if (!pcInitialiser)
	{
		return false;
	}
	pcDeclaration = pcInit->GetDeclaration();
	if (!pcDeclaration || !pcDeclaration->mpcSpecifiers || !(pcDeclaration->mpcSpecifiers->muiFlags & (CPPDS_CONST | CPPDS_CONSTEXPR)))
	{
		return false;
	}
	if ((pcSymbol->meKind == CPPSK_Field) && !(pcDeclaration->mpcSpecifiers->muiFlags & CPPDS_STATIC))
	{
		return false;
	}
	pcVariable = (pcSymbol->meKind == CPPSK_Variable) ? mpcUnit->GetVariable(pcSymbol) : NULL;
	if (pcVariable && (!pcVariable->mpsType->IsInteger() || pcVariable->mpsType->bVolatile))
	{
		//(A volatile const's value is read when the program runs.  [expr.const] 2)
		return false;
	}
	//The variable's value is its initialiser's, which can't be the variable's again (constexpr int a = a + 1).
	mpcUnit->mcContext.EnterBindings(pcClass ? pcBindings : mpcUnit->mcContext.GetBindings());
	bResult = mpcUnit->mcContext.EnterWorking(CPPWK_Initialiser, pcInitialiser);
	if (bResult)
	{
		bResult = Constant(pcInitialiser, plliValue);
		mpcUnit->mcContext.LeaveWorking();
	}
	else
	{
		mpcUnit->mpcConstants->SetReason("It needs its own value");
	}
	mpcUnit->mcContext.LeaveBindings();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// op operand, of an integer constant: ! gives 0 or 1, and + - ~ work in
// the expression's type, the operand promoted to it.  A signed negation
// that overflows is no constant.  [expr] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstantFolder::UnaryConstant(CCPPSyntaxUnaryExpression* pcUnary, int64 lliOperand, int64* plliValue)
{
	SCPP65Type*	psType;

	if (pcUnary->meOperator == CT_LogicalNot)
	{
		*plliValue = !lliOperand;
		return true;
	}
	//Otherwise the operand is promoted to the expression's type, and the result kept in it.
	psType = ConstantTypeOf(pcUnary);
	lliOperand = ConstantOfType(psType, lliOperand);
	switch (pcUnary->meOperator)
	{
	case CT_Add:			*plliValue = lliOperand;																	break;
	case CT_Subtract:		*plliValue = (int64)(0 - (uint64)lliOperand);	ConstantOverflow(pcUnary, *plliValue, true);
		//A signed one that overflows is no constant.  [expr] 4
		if (psType && psType->IsInteger() && psType->bSigned && IRSignedOverflows(IRO_Neg, psType->iBytes * 8, (uint64)lliOperand, 0, (uint64)*plliValue))
		{
			mpcUnit->mpcConstants->SetReason("Its signed arithmetic overflows");
			return false;
		}
		break;
	case CT_BitwiseNot:		*plliValue = ~lliOperand;																	break;
	default:				return false;
	}
	*plliValue = ConstantOfType(psType, *plliValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The error that what must be a constant isn't (szMessage), with why if
// running it said why: "An array size must be a positive constant: It
// divides by zero, in f(int)".  Always false.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstantFolder::ConstantError(CCPPSyntax* pcNode, const char* szMessage)
{
	CChars	szError;
	bool	bResult;

	szError.Init();
	ConstantMessage(szMessage, &szError);
	bResult = mpcUnit->mcDiagnostics.Error(pcNode, szError.Text());
	szError.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// szMessage, and the reason the last constant asked for had no value if
// running it gave one.
//
//////////////////////////////////////////////////////////////////////////
void CCPPConstantFolder::ConstantMessage(const char* szMessage, CChars* pszDest)
{
	pszDest->Append(szMessage);
	if (mpcUnit->mpcConstants->GetReason()[0] != '\0')
	{
		pszDest->Append(": ");
		pszDest->Append(mpcUnit->mpcConstants->GetReason());
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What the tree folder can't give: a member or an element of a constant
// object, what a pointer or reference to one is, a cast of a floating
// point constant, a comparison or a difference of two that aren't integers
// (floating point constants, addresses), and an object where an integer is
// wanted, which is what its constexpr conversion function gives
// (ObjectConstantType).  Only an expression that needs running
// (NamesConstexpr) is run; its value is the interpreter's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstantFolder::RunConstant(CCPPSyntax* pcExpression, int64* plliValue)
{
	SCPP65Type*	psType;

	pcExpression = mpcUnit->SkipParentheses(pcExpression);
	if (!pcExpression)
	{
		return false;
	}
	switch (pcExpression->GetKind())
	{
	case CPPS_IdExpression:
	case CPPS_MemberExpression:
	case CPPS_SubscriptExpression:
	case CPPS_CastExpression:
		break;
	case CPPS_NamedCastExpression:
		if (((CCPPSyntaxNamedCastExpression*)pcExpression)->meCast == CT_dynamic_cast)
		{
			return false;
		}
		break;
	case CPPS_UnaryExpression:
		if (((CCPPSyntaxUnaryExpression*)pcExpression)->meOperator != CT_Multiply)
		{
			return false;
		}
		break;
	case CPPS_BinaryExpression:
		if (!IsCPPComparison(((CCPPSyntaxBinaryExpression*)pcExpression)->meOperator) && (((CCPPSyntaxBinaryExpression*)pcExpression)->meOperator != CT_Subtract))
		{
			return false;
		}
		break;
	case CPPS_ConstructExpression:
		break;
	default:
		return false;
	}
	psType = ObjectConstantType(pcExpression);
	if (psType)
	{
		return mpcUnit->mpcConstants->Value(pcExpression, psType, plliValue);
	}
	if (!NamesConstexpr(pcExpression))
	{
		return false;
	}
	//Its type, which may ask for its constant (a static member's, to see that it needs no definition): that asking
	//is answered no, and the type comes back.
	if (!mpcUnit->mcContext.EnterWorking(CPPWK_RunType, pcExpression))
	{
		return false;
	}
	psType = mpcUnit->TypeOf(pcExpression);
	mpcUnit->mcContext.LeaveWorking();
	if (!psType || !(psType->IsInteger() || (psType->eKind == CPP65TK_Bool)))
	{
		return false;
	}
	return mpcUnit->mpcConstants->Value(pcExpression, psType, plliValue);
}


//////////////////////////////////////////////////////////////////////////
//
// The integer type an object is where an integer constant is wanted: what
// its class's one conversion function to an integer gives, when that is
// constexpr.  NULL for anything else.  [expr.const] 5
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPConstantFolder::ObjectConstantType(CCPPSyntax* pcExpression)
{
	CCPPFunction*	pcConversion;
	SCPP65Type*		psType;

	if (!mpcUnit->mcContext.EnterWorking(CPPWK_ObjectConstant, pcExpression))
	{
		//Its type is being worked out, which asked whether it is a constant: it isn't known to be an object.
		return NULL;
	}
	mpcUnit->mcDiagnostics.BeginQuiet();
	psType = mpcUnit->ValueTypeOf(pcExpression);
	pcConversion = (psType && psType->IsClass()) ? mpcUnit->mcBinder.FindIntegerConversion(psType->pcClass, pcExpression) : NULL;
	mpcUnit->mcDiagnostics.EndQuiet();
	mpcUnit->mcContext.LeaveWorking();
	return (pcConversion && pcConversion->mbConstexpr) ? pcConversion->mpsReturn->WithoutReference()->Unqualified() : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the expression names a variable or function declared constexpr,
// has a string literal, or takes an address: what only running it can give
// the value of.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstantFolder::NamesConstexpr(CCPPSyntax* pcNode)
{
	CCPPSymbol*						pcSymbol;
	CCPPSyntaxDeclSpecifiers*		pcSpecifiers;
	size							i;

	if (!pcNode)
	{
		return false;
	}
	if (mpcUnit->IsStringLiteral(pcNode) || ((pcNode->GetKind() == CPPS_UnaryExpression) && (((CCPPSyntaxUnaryExpression*)pcNode)->meOperator == CT_BitwiseAnd)))
	{
		return true;
	}
	if (pcNode->GetKind() == CPPS_IdExpression)
	{
		pcSymbol = ((CCPPSyntaxIdExpression*)pcNode)->mpcName->mpcSymbol;
		pcSpecifiers = pcSymbol ? pcSymbol->DeclSpecifiers() : NULL;
		return pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_CONSTEXPR);
	}
	for (i = 0; i < pcNode->NumChildren(); i++)
	{
		if (NamesConstexpr(pcNode->GetChild(i)))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The type Constant works pcExpression out in, or NULL (64 bit) if that is
// being found already: TypeOf can ask for the constant whose type it is
// finding.  Other constants on the way have their own types.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPConstantFolder::ConstantTypeOf(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	if (!mpcUnit->mcContext.EnterWorking(CPPWK_ConstantType, pcExpression))
	{
		return NULL;
	}
	psType = mpcUnit->TypeOf(pcExpression);
	mpcUnit->mcContext.LeaveWorking();
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of a call of a constexpr function that gives an integer, if
// it has one: worked out by running it (CCPPConstants).  A call of any
// other function is no constant, and isn't looked at.  [expr.const] 2.2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstantFolder::ConstexprCall(CCPPSyntaxCallExpression* pcCall, int64* plliValue)
{
	CCPPBoundCall	cBound;
	bool			bBound;

	mpcUnit->mcDiagnostics.BeginSilent();
	bBound = mpcUnit->mcBinder.Bind(pcCall, &cBound, false);
	mpcUnit->mcDiagnostics.EndSilent();
	return bBound && ConstexprResult(cBound.mpcFunction, pcCall, plliValue);
}


//////////////////////////////////////////////////////////////////////////
//
// The integer an expression that is a call of pcFunction gives (a call, or
// a literal with a suffix, which is its literal operator's), if the
// function is constexpr: the call is run.  False if it isn't a constant.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstantFolder::ConstexprResult(CCPPFunction* pcFunction, CCPPSyntax* pcExpression, int64* plliValue)
{
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	SCPP65Type*						psType;

	pcDefinition = pcFunction ? pcFunction->mpcDefinition : NULL;
	if (!pcDefinition || !pcDefinition->mpcSpecifiers || !(pcDefinition->mpcSpecifiers->muiFlags & CPPDS_CONSTEXPR))
	{
		return false;
	}
	psType = mpcUnit->TypeOf(pcExpression);
	if (!psType || !(psType->IsInteger() || (psType->eKind == CPP65TK_Bool)))
	{
		return false;
	}
	return mpcUnit->mpcConstants->Value(pcExpression, psType, plliValue);
}


//////////////////////////////////////////////////////////////////////////
//
// lliValue as an integer (or pointer) of psType holds it: cut to its width,
// then sign or zero extended.  Unchanged for other types and NULL.
//
//////////////////////////////////////////////////////////////////////////
int64 CCPPConstantFolder::ConstantOfType(SCPP65Type* psType, int64 lliValue)
{
	if (!psType || !(psType->IsInteger() || psType->IsPointer()))
	{
		return lliValue;
	}
	switch (psType->iBytes)
	{
	case 1:		return psType->bSigned ? (int64)(int8)lliValue : (int64)(uint8)lliValue;
	case 2:		return psType->bSigned ? (int64)(int16)lliValue : (int64)(uint16)lliValue;
	case 8:		return lliValue;
	default:	return psType->bSigned ? (int64)(int32)lliValue : (int64)(uint32)lliValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// left op right, of constants: the operands converted to their common type
// (the left's promoted for a shift), the arithmetic on that type (unsigned
// divides, shifts and compares as unsigned) and the result kept in the
// expression's type.  The sum's, product's and shift's exact values are
// checked for overflow.  False (not a constant) for a divide by 0, a shift
// by a negative count or by the type's width or more, and the most
// negative 64 bit value divided by -1.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstantFolder::BinaryConstant(CCPPSyntaxBinaryExpression* pcBinary, int64 lliLeft, int64 lliRight, int64* plliValue)
{
	SCPP65Type*	psResult;
	SCPP65Type*	psOperands;
	SCPP65Type*	psLeft;
	SCPP65Type*	psRight;
	bool		bUnsigned;
	int			iBits;
	uint64		a;
	uint64		b;
	int64		lliResult;

	psResult = ConstantTypeOf(pcBinary);
	psOperands = psResult;
	if (pcBinary->meOperator == CT_LogicalAnd)
	{
		*plliValue = lliLeft && lliRight;
		return true;
	}
	if (pcBinary->meOperator == CT_LogicalOr)
	{
		*plliValue = lliLeft || lliRight;
		return true;
	}
	if (IsCPPComparison(pcBinary->meOperator))
	{
		psLeft = ConstantTypeOf(pcBinary->mpcLeft);
		psRight = ConstantTypeOf(pcBinary->mpcRight);
		psLeft = psLeft ? psLeft->WithoutReference() : NULL;
		psRight = psRight ? psRight->WithoutReference() : NULL;
		psOperands = (psLeft && psRight && psLeft->IsInteger() && psRight->IsInteger()) ? mpcUnit->mcTypes.Arithmetic(psLeft, psRight) : NULL;
	}
	bUnsigned = psOperands && psOperands->IsInteger() && !psOperands->bSigned;
	lliLeft = ConstantOfType(psOperands, lliLeft);
	if ((pcBinary->meOperator == CT_LeftShift) || (pcBinary->meOperator == CT_RightShift))
	{
		iBits = (psOperands && psOperands->IsInteger()) ? psOperands->iBytes * 8 : 64;
		if ((lliRight < 0) || (lliRight >= iBits))
		{
			return false;
		}
	}
	else
	{
		lliRight = ConstantOfType(psOperands, lliRight);
	}
	a = (uint64)lliLeft;
	b = (uint64)lliRight;

	switch (pcBinary->meOperator)
	{
	case CT_Add:				lliResult = (int64)(a + b);		ConstantOverflow(pcBinary, lliResult, false);	break;
	case CT_Subtract:			lliResult = (int64)(a - b);		ConstantOverflow(pcBinary, lliResult, true);	break;
	case CT_Multiply:			lliResult = (int64)(a * b);		ConstantOverflow(pcBinary, lliResult, false);	break;
	case CT_LeftShift:			lliResult = (int64)(a << b);	ConstantOverflow(pcBinary, lliResult, false);	break;
	case CT_RightShift:			lliResult = bUnsigned ? (int64)(a >> b) : (lliLeft >> b);	break;
	case CT_BitwiseAnd:			lliResult = (int64)(a & b);		break;
	case CT_BitwiseOr:			lliResult = (int64)(a | b);		break;
	case CT_BitwiseXor:			lliResult = (int64)(a ^ b);		break;
	case CT_EqualTo:			*plliValue = a == b;													return true;
	case CT_NotEqualTo:			*plliValue = a != b;													return true;
	case CT_LessThan:			*plliValue = bUnsigned ? (a < b) : (lliLeft < lliRight);				return true;
	case CT_LessThanEqualTo:	*plliValue = bUnsigned ? (a <= b) : (lliLeft <= lliRight);			return true;
	case CT_GreaterThan:		*plliValue = bUnsigned ? (a > b) : (lliLeft > lliRight);				return true;
	case CT_GreaterThanEqualTo:	*plliValue = bUnsigned ? (a >= b) : (lliLeft >= lliRight);			return true;
	case CT_Divide:
	case CT_Modulus:
		if ((b == 0) || (!bUnsigned && (lliLeft == (int64)MIN_INT64) && (lliRight == -1)))
		{
			return false;
		}
		if (pcBinary->meOperator == CT_Divide)
		{
			lliResult = bUnsigned ? (int64)(a / b) : (lliLeft / lliRight);
		}
		else
		{
			lliResult = bUnsigned ? (int64)(a % b) : (lliLeft % lliRight);
		}
		break;
	default:
		return false;
	}

	//A signed sum, difference, product, left shift or quotient that overflows is no constant: the least number
	//divided by -1 is one.  [expr] 4, [expr.shift] 2
	if (!bUnsigned && psOperands && psOperands->IsInteger() && psOperands->bSigned && IsSignedOverflow(pcBinary->meOperator, psOperands->iBytes * 8, lliLeft, lliRight, lliResult))
	{
		mpcUnit->mpcConstants->SetReason(((pcBinary->meOperator == CT_Divide) || (pcBinary->meOperator == CT_Modulus)) ? "It divides by zero, or the least number by -1" : "Its signed arithmetic overflows");
		return false;
	}
	*plliValue = ConstantOfType(psResult, lliResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if a signed constant's left op right (iBits wide) that gave
// lliResult overflows (IRSignedOverflows), or is the least number divided
// by -1.  The operator's opcode is the front end's (CPPArithmeticOpcode).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstantFolder::IsSignedOverflow(ECppTokens eOperator, int iBits, int64 lliLeft, int64 lliRight, int64 lliResult)
{
	uint16	uiOpcode;

	if ((eOperator == CT_Divide) || (eOperator == CT_Modulus))
	{
		return (lliRight == -1) && IRSignedOverflows(IRO_Neg, iBits, (uint64)lliLeft, 0, 0);
	}
	//The sum, difference, product and left shift: the operators whose opcodes wrap.
	uiOpcode = CPPArithmeticOpcode(eOperator, true);
	return (uiOpcode != IRO_None) && IsIRSignedWrapping(uiOpcode) && IRSignedOverflows(uiOpcode, iBits, (uint64)lliLeft, (uint64)lliRight, (uint64)lliResult);
}


//////////////////////////////////////////////////////////////////////////
//
// Warns if lliValue, an integer constant expression's exact value, doesn't
// fit its type: an int expression that is right with a 32 bit int can
// silently be wrong with the W65C816's 16 bit one ("12000 * 6").  Unsigned
// arithmetic wraps by definition; bUnsignedWraps says if this one is meant to
// (subtraction and negation are, a sum, product or shift that loses bits is
// taken as a mistake).
//
//////////////////////////////////////////////////////////////////////////
void CCPPConstantFolder::ConstantOverflow(CCPPSyntax* pcExpression, int64 lliValue, bool bUnsignedWraps)
{
	SCPP65Type*	psType;
	int64		lliMin;
	int64		lliMax;
	CChars		szMessage;

	if (!mpcUnit->mcContext.EnterWorking(CPPWK_Warning, NULL))
	{
		return;
	}
	psType = mpcUnit->TypeOf(pcExpression);
	mpcUnit->mcContext.LeaveWorking();
	if (!psType || !psType->IsInteger() || (psType->iBytes >= 8))
	{
		return;
	}
	if (!psType->bSigned && bUnsignedWraps)
	{
		return;
	}
	lliMin = psType->Least();
	lliMax = psType->Most();
	if ((lliValue >= lliMin) && (lliValue <= lliMax))
	{
		return;
	}

	szMessage.Init("Integer overflow: the constant expression is ");
	szMessage.Append(lliValue)->Append(", which doesn't fit its ")->Append(psType->iBytes * 8)->Append(" bit ")->Append(psType->bSigned ? "signed" : "unsigned")->Append(" type.");
	mpcUnit->mcDiagnostics.Warning(pcExpression, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A literal token's value: a number's, a character's, a bool's, nullptr's
// (0).  False for a floating point number and a user-defined literal.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstantFolder::TokenConstant(CCPPToken* pcToken, int64* plliValue)
{
	if (pcToken->IsNumber())
	{
		switch (((CCTNumber*)pcToken)->meType)
		{
		case CTNT_Int:
			*plliValue = ((CCTInt*)pcToken)->mcValue;
			if (((CCTInt*)pcToken)->IsSigned())
			{
				*plliValue = (int32)((CCTInt*)pcToken)->mcValue;
			}
			return true;
		case CTNT_Short:
			*plliValue = ((CCTShort*)pcToken)->mcValue;
			return true;
		case CTNT_LongLong:
			*plliValue = (int64)((CCTLongLong*)pcToken)->mcValue;
			return true;
		default:
			return false;
		}
	}
	if (pcToken->IsChar())
	{
		*plliValue = (int8)((CCTChar*)pcToken)->mcChar;
		return true;
	}
	if (pcToken->IsBool())
	{
		*plliValue = ((CCTBoolean*)pcToken)->mcBool ? 1 : 0;
		return true;
	}
	if (pcToken->IsUserLiteral())
	{
		return false;
	}
	*plliValue = 0;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A floating point number token's value and type (float, double or long
// double).  False for any other token.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstantFolder::TokenFloat(CCPPToken* pcToken, double* pdValue, SCPP65Type** ppsType)
{
	if (!pcToken->IsNumber())
	{
		return false;
	}
	switch (((CCTNumber*)pcToken)->meType)
	{
	case CTNT_Float:
		*pdValue = ((CCTFloat*)pcToken)->mcValue;
		*ppsType = mpcUnit->mcTypes.Float(4);
		return true;
	case CTNT_Double:
		*pdValue = ((CCTDouble*)pcToken)->mcValue;
		*ppsType = mpcUnit->mcTypes.Float(8);
		return true;
	case CTNT_LongDouble:
		*pdValue = (double)((CCTLongDouble*)pcToken)->mcValue;
		*ppsType = mpcUnit->mcTypes.LongDouble();
		return true;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A call of one of GCC's floating point constant builtins
// (CCPPBuiltins::FloatBuiltinValue): its value and type.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstantFolder::FloatBuiltin(CCPPSyntax* pcExpression, double* pdValue, SCPP65Type** ppsType)
{
	CCPPSyntaxCallExpression*	pcCall;
	CCPPSyntax*					pcCallee;
	CCPPSyntaxName*				pcName;
	int64						lliName;
	bool						bFloat;

	pcCall = (CCPPSyntaxCallExpression*)pcExpression;
	pcCallee = mpcUnit->SkipParentheses(pcCall->mpcCallee);
	if (!pcCallee || (pcCallee->GetKind() != CPPS_IdExpression))
	{
		return false;
	}
	pcName = ((CCPPSyntaxIdExpression*)pcCallee)->mpcName;
	if (!pcName || (pcName->macpComponents.NumElements() != 1))
	{
		return false;
	}
	lliName = mpcUnit->GetFunctionName(pcName->GetLast());
	if (!mpcUnit->GetBuiltins()->FloatBuiltinValue(lliName, pcCall->macpArguments.NumElements(), pdValue, &bFloat))
	{
		return false;
	}
	*ppsType = bFloat ? &mpcUnit->mcTypes.msFloat : &mpcUnit->mcTypes.msDouble;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The bits of a floating point value of psType (as a 32 bit value for a float).
//
//////////////////////////////////////////////////////////////////////////
int64 CCPPConstantFolder::FloatBits(SCPP65Type* psType, double dValue)
{
	float	fValue;
	uint32	uiBits;
	uint64	ulliBits;

	if (psType->iBytes == 4)
	{
		fValue = (float)dValue;
		memcpy(&uiBits, &fValue, sizeof(uint32));
		return (int64)(int32)uiBits;
	}
	memcpy(&ulliBits, &dValue, sizeof(uint64));
	return (int64)ulliBits;
}


//////////////////////////////////////////////////////////////////////////
//
// A constant's value as a floating point number.
//
//////////////////////////////////////////////////////////////////////////
double SCPPConstant::AsDouble(void) const
{
	return psFloat ? dValue : (double)lliValue;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes the constant a floating point one of psType (float, double or
// long double), dValue rounded as the type keeps it.
//
//////////////////////////////////////////////////////////////////////////
void SCPPConstant::SetFloat(SCPP65Type* psType, double d)
{
	psFloat = psType;
	dValue = (psType->iBytes == 4) ? (double)(float)d : d;
	lliValue = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes the constant an integer one.
//
//////////////////////////////////////////////////////////////////////////
void SCPPConstant::SetInteger(int64 lli)
{
	psFloat = NULL;
	dValue = 0;
	lliValue = lli;
}

