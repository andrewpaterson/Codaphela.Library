/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Chars.h"
#include "CppTokeniser.h"
#include "CPPSyntaxTree.h"
#include "CPP65Generator.h"
#include "CPP65TypeChecker.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::Init(CCPP65Generator* pcGenerator)
{
	mpcGenerator = pcGenerator;
	mpcTypes = &pcGenerator->mcTypes;
	mapcNone.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::Kill(void)
{
	mapcNone.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The type of an expression's value: a reference's is what it refers to.  Arrays and
// functions named are pointers to them (see Load).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::Value(CCPPSyntax* pcExpression)
{
	SCPP65Type*		psType;

	psType = Result(pcExpression);
	if (psType && psType->IsReference())
	{
		psType = psType->psBase;
		if (!psType->IsClass() && !mpcGenerator->Supported(psType, pcExpression))
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
// type, and an object is converted by its conversion function to psTo (only an
// explicit one when bExplicit).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ValueFor(CCPPSyntax* pcExpression, SCPP65Type* psTo, bool bExplicit)
{
	SCPP65Function*				psFunction;
	SCPP65Class*				psClass;
	CCPPSyntaxIdExpression*		pcFunction;

	pcFunction = mpcGenerator->FunctionNameFor(pcExpression, psTo);
	if (pcFunction)
	{
		return FunctionAddress(pcFunction, pcFunction->mpcName->mpcSymbol->mlliName, psTo->psBase) ? psTo : NULL;
	}

	if (psTo && !psTo->IsClass() && !psTo->IsReference() && !psTo->IsVoid() && mpcGenerator->IsObject(pcExpression))
	{
		psClass = mpcGenerator->ObjectClass(pcExpression);
		psFunction = mpcGenerator->FindConversion(psClass, psTo, bExplicit, pcExpression);
		if (psFunction)
		{
			return ObjectCall(pcExpression, psFunction, NULL, pcExpression);
		}
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
	CCPPSyntaxNamedCastExpression*	pcNamedCast;
	CCPPSyntaxSubscriptExpression*	pcSubscript;
	CArraySyntax					apcIndex;
	SCPP65Type*						psType;

	if (!pcExpression)
	{
		return NULL;
	}

	switch (pcExpression->GetKind())
	{
	case CPPS_LiteralExpression:
		return ConstantValue(pcExpression);
	case CPPS_StringExpression:
		return StringLiteral((CCPPSyntaxStringExpression*)pcExpression);
	case CPPS_IdExpression:
		return IdExpression((CCPPSyntaxIdExpression*)pcExpression);
	case CPPS_ParenthesisedExpression:
		return Result(((CCPPSyntaxParenthesisedExpression*)pcExpression)->mpcExpression);
	case CPPS_UnaryExpression:
		return UnaryExpression((CCPPSyntaxUnaryExpression*)pcExpression);
	case CPPS_BinaryExpression:
		return BinaryExpression((CCPPSyntaxBinaryExpression*)pcExpression);
	case CPPS_ConditionalExpression:
		return ConditionalExpression((CCPPSyntaxConditionalExpression*)pcExpression);
	case CPPS_CallExpression:
		return CallExpression((CCPPSyntaxCallExpression*)pcExpression);
	case CPPS_SubscriptExpression:
		//object[index] is a call to the object's operator[], which may return a value.
		pcSubscript = (CCPPSyntaxSubscriptExpression*)pcExpression;
		psType = mpcGenerator->TypeOf(pcSubscript->mpcArray);
		if (psType && psType->IsClass())
		{
			apcIndex.Init();
			apcIndex.Add(pcSubscript->mpcIndex);
			psType = OperatorCall(pcSubscript->mpcArray, CT_OpenSquareBracket, &apcIndex, pcExpression);
			apcIndex.Kill();
			return psType;
		}
		return Load(Place(pcExpression), pcExpression);
	case CPPS_MemberExpression:
		return Load(Place(pcExpression), pcExpression);
	case CPPS_ThisExpression:
		return ThisExpression(pcExpression);
	case CPPS_CastExpression:
		return CastExpression(((CCPPSyntaxCastExpression*)pcExpression)->mpcType, ((CCPPSyntaxCastExpression*)pcExpression)->mpcOperand, pcExpression);
	case CPPS_NamedCastExpression:
		pcNamedCast = (CCPPSyntaxNamedCastExpression*)pcExpression;
		if (pcNamedCast->meCast == CT_dynamic_cast)
		{
			//Its pointer or reference type, whatever the operand (CCPP65IRFrontEnd::DynamicCastExpression checks it).
			return mpcGenerator->GetTypeIdType(pcNamedCast->mpcType);
		}
		return CastExpression(pcNamedCast->mpcType, pcNamedCast->mpcOperand, pcExpression);
	case CPPS_ConstructExpression:
		return ConstructExpression((CCPPSyntaxConstructExpression*)pcExpression);
	case CPPS_SizeofExpression:
	case CPPS_TypeTraitExpression:
		return SizeofExpression(pcExpression);
	case CPPS_NewExpression:
		return NewExpression((CCPPSyntaxNewExpression*)pcExpression);
	case CPPS_DeleteExpression:
		return DeleteExpression((CCPPSyntaxDeleteExpression*)pcExpression);
	default:
		Error(pcExpression, "This expression is not supported by the compiler yet");
		return NULL;
	}
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
	return mpcGenerator->Supported(psPlace, pcNode) ? psPlace : NULL;
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
	int64			lliValue;
	double			dValue;

	pcExpression = mpcGenerator->SkipParentheses(pcExpression);
	if (pcExpression->GetKind() == CPPS_IdExpression)
	{
		psType = mpcGenerator->EnumeratorType(((CCPPSyntaxIdExpression*)pcExpression)->mpcName->mpcSymbol);
	}
	else
	{
		psType = mpcGenerator->Literal((CCPPSyntaxLiteralExpression*)pcExpression);
		if (!psType)
		{
			return NULL;
		}
		if (psType->IsFloat() && mpcGenerator->FloatConstant(pcExpression, &dValue))
		{
			return psType;
		}
	}
	if (!mpcGenerator->Constant(pcExpression, &lliValue))
	{
		Error(pcExpression, "The value is not a constant");
		return NULL;
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// A string literal is a pointer to its first character.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::StringLiteral(CCPPSyntaxStringExpression* pcString)
{
	CArrayUint8		auiBytes;
	bool			bDecoded;

	auiBytes.Init();
	bDecoded = mpcGenerator->DecodeString(pcString, &auiBytes);
	auiBytes.Kill();
	return bDecoded ? mpcTypes->Pointer(&mpcTypes->msChar) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A name: the enclosing function's name (__func__ ...), an enumerator, a non-type
// template parameter (its argument, at the parameter's type), a function or a
// function template (their address), or else a variable or a field.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::IdExpression(CCPPSyntaxIdExpression* pcId)
{
	CCPPSymbol*				pcSymbol;
	CCPPSyntaxParameter*	pcParameter;
	SCPP65Type*				psType;

	pcSymbol = pcId->mpcName->mpcSymbol;
	if (mpcGenerator->IsFunctionName(pcId))
	{
		return mpcTypes->Pointer(&mpcTypes->msChar);
	}
	if (pcSymbol && (pcSymbol->meKind == CPPSK_Enumerator))
	{
		return ConstantValue(pcId);
	}
	if (pcSymbol && (pcSymbol->meKind == CPPSK_ValueParameter))
	{
		if (!mpcGenerator->FindBinding(pcSymbol))
		{
			Error(pcId, "A template parameter used outside an instantiation");
			return NULL;
		}
		pcParameter = (CCPPSyntaxParameter*)pcSymbol->mpcDeclaration;
		psType = (pcParameter && (pcParameter->GetKind() == CPPS_Parameter)) ? mpcGenerator->GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter) : NULL;
		return (psType && psType->IsInteger()) ? psType : &mpcTypes->msInt;
	}
	if (pcSymbol && (pcSymbol->meKind == CPPSK_Function))
	{
		return FunctionAddress(pcId, pcSymbol->mlliName, NULL);
	}
	if (pcSymbol && (pcSymbol->meKind == CPPSK_FunctionTemplate))
	{
		return FunctionTemplateAddress(pcId);
	}
	return Load(Place(pcId), pcId);
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
	SCPP65Function*		psFound;
	CCPPSyntaxName*		pcName;
	bool				bOverloaded;

	pcName = (pcNode && (pcNode->GetKind() == CPPS_IdExpression)) ? ((CCPPSyntaxIdExpression*)pcNode)->mpcName : NULL;
	psFound = mpcGenerator->FindAddressedFunction(pcName, lliName, psTarget, pcNode, &bOverloaded);
	if (bOverloaded)
	{
		Error(pcNode, "The address of an overloaded function needs the type of the function it is for");
		return NULL;
	}
	if (!psFound)
	{
		Error(pcNode, "The function has not been declared");
		return NULL;
	}
	return mpcTypes->Pointer(mpcTypes->Function(psFound->psReturn, &psFound->mapsParameters, psFound->bVariadic));
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
	SCPP65Template*				psTemplate;
	SCPP65Function*				psFunction;

	pcComponent = pcId->mpcName->GetLast();
	psTemplate = mpcGenerator->GetTemplate(pcId->mpcName->mpcSymbol);
	if (!psTemplate || !pcComponent->mbTemplateArguments)
	{
		Error(pcId, "The address of a function template needs its template arguments in the compiler yet");
		return NULL;
	}
	psFunction = mpcGenerator->InstantiateFunction(psTemplate, &pcComponent->macpTemplateArguments, NULL, pcId);
	if (!psFunction)
	{
		return NULL;
	}
	return mpcTypes->Pointer(mpcTypes->Function(psFunction->psReturn, &psFunction->mapsParameters, psFunction->bVariadic));
}


//////////////////////////////////////////////////////////////////////////
//
// &Class::member: a pointer to a member function or to a field, its type in *ppsType
// (NULL after an error).  A member of a base is a pointer to a member of the base.
// Returns false if the name is not a non-static member of the class or its bases.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::MemberAddress(CCPPSyntaxIdExpression* pcId, SCPP65Type** ppsType)
{
	CCPPSyntaxName*		pcName;
	SCPP65Class*		psClass;
	SCPP65Field*		psField;
	int64				lliName;
	int					iOffset;

	pcName = pcId->mpcName;
	mpcGenerator->BeginQuiet();
	psClass = mpcGenerator->GetComponentClass(pcName->GetQualifier(), NULL, pcId);
	mpcGenerator->EndQuiet();
	lliName = mpcGenerator->GetFunctionName(pcName->GetLast());
	*ppsType = NULL;
	if (!psClass || (lliName == -1))
	{
		return false;
	}
	if (!mpcGenerator->CompleteClass(psClass, pcId))
	{
		return true;
	}
	if (MemberFunctionAddress(psClass, lliName, ppsType, pcId))
	{
		return true;
	}

	psField = mpcGenerator->FindField(psClass, lliName, &iOffset);
	if (!psField)
	{
		return false;
	}
	*ppsType = mpcTypes->MemberPointer(psClass, psField->psType);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The pointer to the member function of the name in the class, or else in its bases
// (see MemberAddress).  Returns false if there is none.  The address of an
// overloaded one is not supported yet.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::MemberFunctionAddress(SCPP65Class* psClass, int64 lliName, SCPP65Type** ppsType, CCPPSyntax* pcNode)
{
	SCPP65Function*		psFunction;
	SCPP65Function*		psFound;
	size				i;

	psFound = NULL;
	for (psFunction = mpcGenerator->FirstFunctionOfName(lliName); psFunction; psFunction = psFunction->psNextOfName)
	{
		if ((psFunction->psClass == psClass) && (psFunction->eKind == CPP65FK_Function) && !psFunction->psTemplate)
		{
			if (psFound)
			{
				Error(pcNode, "The address of an overloaded function is not supported by the compiler yet");
				return true;
			}
			psFound = psFunction;
		}
	}
	if (!psFound)
	{
		for (i = 0; i < psClass->macBases.NumElements(); i++)
		{
			if (MemberFunctionAddress(psClass->macBases.Get(i)->psClass, lliName, ppsType, pcNode))
			{
				return true;
			}
		}
		return false;
	}
	if (psFound->bStatic)
	{
		return false;
	}
	*ppsType = mpcTypes->MemberPointer(psClass, mpcTypes->Function(psFound->psReturn, &psFound->mapsParameters, psFound->bVariadic));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// this, in a member function: a pointer to its class, const in a const one.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ThisExpression(CCPPSyntax* pcThis)
{
	if (!mpcGenerator->mbThis || !mpcGenerator->mpsFunction || !mpcGenerator->mpsFunction->psClass)
	{
		Error(pcThis, "this is only in member functions");
		return NULL;
	}
	return mpcTypes->Pointer(mpcTypes->Qualified(mpcGenerator->mpsFunction->psClass->psType, mpcGenerator->IsConstThis(), false));
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
	int64					lliValue;

	switch (pcUnary->meOperator)
	{
	case CT_Increment:
	case CT_Decrement:
		return IncrementExpression(pcUnary);
	case CT_LogicalNot:
		return ConditionValue(pcUnary);
	case CT_Multiply:
		return Load(Place(pcUnary), pcUnary);
	case CT_BitwiseAnd:
		pcOperand = mpcGenerator->SkipParentheses(pcUnary->mpcOperand);
		if (mpcGenerator->IsObject(pcOperand) && mpcGenerator->HasMember(mpcGenerator->ObjectClass(pcOperand), mpcGenerator->GetOperatorName(CT_BitwiseAnd, false)))
		{
			return OperatorExpression(CT_BitwiseAnd, pcOperand, NULL, pcUnary);
		}
		if (pcOperand->GetKind() == CPPS_IdExpression)
		{
			pcId = (CCPPSyntaxIdExpression*)pcOperand;
			pcSymbol = pcId->mpcName->mpcSymbol;
			if (pcId->mpcName->IsQualifiedByClass() && MemberAddress(pcId, &psType))
			{
				return psType;
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
		return psType ? mpcTypes->Pointer(psType) : NULL;
	default:
		break;
	}

	if (((pcUnary->meOperator == CT_Subtract) || (pcUnary->meOperator == CT_Add) || (pcUnary->meOperator == CT_BitwiseNot)) && mpcGenerator->IsObject(pcUnary->mpcOperand))
	{
		return OperatorExpression(pcUnary->meOperator, pcUnary->mpcOperand, NULL, pcUnary);
	}
	if (((pcUnary->meOperator == CT_Subtract) || (pcUnary->meOperator == CT_BitwiseNot) || (pcUnary->meOperator == CT_Add)) && mpcGenerator->Constant(pcUnary, &lliValue))
	{
		psType = mpcGenerator->TypeOf(pcUnary->mpcOperand);
		if (psType && psType->IsInteger())
		{
			return mpcTypes->Promote(psType);
		}
	}

	psType = Value(pcUnary->mpcOperand);
	if (!psType)
	{
		return NULL;
	}
	if (psType->IsFloat() && ((pcUnary->meOperator == CT_Add) || (pcUnary->meOperator == CT_Subtract)))
	{
		return psType;
	}
	if (!psType->IsInteger())
	{
		Error(pcUnary, "The operand must be an integer");
		return NULL;
	}
	switch (pcUnary->meOperator)
	{
	case CT_Add:
	case CT_Subtract:
	case CT_BitwiseNot:
		return mpcTypes->Promote(psType);
	default:
		Error(pcUnary, "This operator is not supported by the compiler yet");
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// ++x, --x, x++ and x--: the type of x, which must be a number or a pointer.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::IncrementExpression(CCPPSyntaxUnaryExpression* pcUnary)
{
	SCPP65Type*	psType;

	psType = Place(pcUnary->mpcOperand);
	if (!psType || !CanChange(psType, pcUnary))
	{
		return NULL;
	}
	if (!psType->IsScalar())
	{
		Error(pcUnary, "Only integers and pointers can be incremented");
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

	switch (pcBinary->meOperator)
	{
	case CT_MemberFunctionPointerIndirection:
	case CT_MemberByPointerFunctionPointerIndirection:
		return Load(Place(pcBinary), pcBinary);
	case CT_Assignment:
	case CT_AssignmentAdd:
	case CT_AssignmentSubtract:
	case CT_AssignmentMultiply:
	case CT_AssignmentDivide:
	case CT_AssignmentModulus:
	case CT_AssignmentBitwiseAnd:
	case CT_AssignmentBitwiseOr:
	case CT_AssignmentBitwiseXor:
	case CT_AssignmentLeftShift:
	case CT_AssignmentRightShift:
		return Assignment(pcBinary);
	case CT_Comma:
		if (!Value(pcBinary->mpcLeft))
		{
			return NULL;
		}
		return Value(pcBinary->mpcRight);
	case CT_LogicalAnd:
	case CT_LogicalOr:
	case CT_EqualTo:
	case CT_NotEqualTo:
	case CT_LessThan:
	case CT_LessThanEqualTo:
	case CT_GreaterThan:
	case CT_GreaterThanEqualTo:
		return ConditionValue(pcBinary);
	default:
		if (mpcGenerator->IsObject(pcBinary->mpcLeft) || mpcGenerator->IsObject(pcBinary->mpcRight))
		{
			return OperatorExpression(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, pcBinary);
		}
		if (!Operands(pcBinary->mpcLeft, pcBinary->mpcRight, &psLeft, &psRight))
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

	psResult = mpcGenerator->TypeOf(pcBinary->mpcLeft);
	if (psResult && psResult->IsClass() && mpcGenerator->HasMember(psResult->psClass, mpcGenerator->GetOperatorName(pcBinary->meOperator, false)))
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
		return CopyObject(psPlace->psClass, pcBinary->mpcRight, pcBinary) ? psPlace : NULL;
	}
	if (!psPlace->IsScalar())
	{
		Error(pcBinary, "Only integers, pointers and objects can be assigned by the compiler yet");
		return NULL;
	}

	if (pcBinary->meOperator == CT_Assignment)
	{
		psRight = ValueFor(pcBinary->mpcRight, psPlace, false);
		return ImplicitConvert(psRight, psPlace, pcBinary->mpcRight) ? psPlace : NULL;
	}

	psRight = Value(pcBinary->mpcRight);
	if (!psRight)
	{
		return NULL;
	}
	switch (pcBinary->meOperator)
	{
	case CT_AssignmentAdd:			eOperator = CT_Add;			break;
	case CT_AssignmentSubtract:		eOperator = CT_Subtract;	break;
	case CT_AssignmentMultiply:		eOperator = CT_Multiply;	break;
	case CT_AssignmentDivide:		eOperator = CT_Divide;		break;
	case CT_AssignmentModulus:		eOperator = CT_Modulus;		break;
	case CT_AssignmentBitwiseAnd:	eOperator = CT_BitwiseAnd;	break;
	case CT_AssignmentBitwiseOr:	eOperator = CT_BitwiseOr;	break;
	case CT_AssignmentBitwiseXor:	eOperator = CT_BitwiseXor;	break;
	case CT_AssignmentLeftShift:	eOperator = CT_LeftShift;	break;
	default:						eOperator = CT_RightShift;	break;
	}
	psResult = Operate(eOperator, psPlace, psRight, pcBinary);
	return Convert(psResult, psPlace, pcBinary) ? psPlace : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The types of a binary operator's operands' values, left then right.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::Operands(CCPPSyntax* pcLeft, CCPPSyntax* pcRight, SCPP65Type** ppsLeft, SCPP65Type** ppsRight)
{
	*ppsLeft = Value(pcLeft);
	if (!*ppsLeft)
	{
		return false;
	}
	*ppsRight = Value(pcRight);
	return *ppsRight != NULL;
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
	psType = mpcTypes->Operation(eOperator, psLeft, psRight);
	if (psType)
	{
		if ((psLeft->IsPointer() && !CompletePointee(psLeft, pcNode)) || (psRight->IsPointer() && !CompletePointee(psRight, pcNode)))
		{
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
// and -- step by its size, so a class (or an array of them) must be laid out.
// [expr.add] 1
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
	return !psPointee->IsClass() || mpcGenerator->CompleteClass(psPointee->psClass, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether left and right can be compared: numbers, pointers (with each other or 0),
// and pointers to members for equality.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::Compare(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Type* psRight, CCPPSyntax* pcNode)
{
	if (psLeft->IsFloat() || psRight->IsFloat())
	{
		if (psLeft->IsPointer() || psRight->IsPointer() || !psLeft->IsScalar() || !psRight->IsScalar())
		{
			return Error(pcNode, "Only numbers can be used with floating point");
		}
		return true;
	}
	if (psLeft->IsPointer() || psRight->IsPointer())
	{
		return true;
	}
	if ((psLeft->IsMemberPointer() || psRight->IsMemberPointer()) && ((eOperator == CT_EqualTo) || (eOperator == CT_NotEqualTo)))
	{
		return true;
	}
	if (psLeft->IsInteger() && psRight->IsInteger())
	{
		return true;
	}
	return Error(pcNode, "These operands cannot be compared");
}


//////////////////////////////////////////////////////////////////////////
//
// c ? x : y.  Objects of a class make an object of it (ObjectConditional), scalars
// the type CCPP65Types::Conditional gives.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ConditionalExpression(CCPPSyntaxConditionalExpression* pcConditional)
{
	SCPP65Type*		psTrue;
	SCPP65Type*		psFalse;
	SCPP65Type*		psType;

	psTrue = mpcGenerator->TypeOf(pcConditional->mpcTrue);
	psFalse = mpcGenerator->TypeOf(pcConditional->mpcFalse);
	if (!psTrue || !psFalse)
	{
		return NULL;
	}
	if (mpcGenerator->IsObject(pcConditional->mpcTrue) || mpcGenerator->IsObject(pcConditional->mpcFalse))
	{
		return ObjectConditional(pcConditional, psTrue, psFalse);
	}

	psType = mpcTypes->Conditional(psTrue, psFalse);
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
// c ? x : y with objects of the same class: an object made from the operand the
// condition picks.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ObjectConditional(CCPPSyntaxConditionalExpression* pcConditional, SCPP65Type* psTrue, SCPP65Type* psFalse)
{
	psTrue = psTrue->WithoutReference();
	psFalse = psFalse->WithoutReference();
	if (!psTrue->IsClass() || !psFalse->IsClass() || (psTrue->psClass != psFalse->psClass))
	{
		Error(pcConditional, "c ? x : y with objects of different types is not supported by the compiler yet");
		return NULL;
	}
	if (!mpcGenerator->CompleteClass(psTrue->psClass, pcConditional) ||
		!Condition(pcConditional->mpcCondition) ||
		!Construct(psTrue, NULL, pcConditional->mpcTrue, pcConditional->mpcTrue) ||
		!Construct(psTrue, NULL, pcConditional->mpcFalse, pcConditional->mpcFalse))
	{
		return NULL;
	}
	return psTrue;
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
	CCPPSyntaxBinaryExpression*	pcBinary;
	CCPPSyntaxUnaryExpression*	pcUnary;
	SCPP65Type*					psLeft;
	SCPP65Type*					psRight;
	SCPP65Function*				psFunction;

	pcExpression = mpcGenerator->SkipParentheses(pcExpression);
	if (!pcExpression)
	{
		return false;
	}

	if (pcExpression->GetKind() == CPPS_UnaryExpression)
	{
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		if ((pcUnary->meOperator == CT_LogicalNot) && mpcGenerator->IsObject(pcUnary->mpcOperand) && mpcGenerator->HasMember(mpcGenerator->ObjectClass(pcUnary->mpcOperand), mpcGenerator->GetOperatorName(CT_LogicalNot, false)))
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
		switch (pcBinary->meOperator)
		{
		case CT_LogicalAnd:
		case CT_LogicalOr:
			return Condition(pcBinary->mpcLeft) && Condition(pcBinary->mpcRight);
		case CT_EqualTo:
		case CT_NotEqualTo:
		case CT_LessThan:
		case CT_LessThanEqualTo:
		case CT_GreaterThan:
		case CT_GreaterThanEqualTo:
			if (mpcGenerator->IsObject(pcBinary->mpcLeft) || mpcGenerator->IsObject(pcBinary->mpcRight))
			{
				return Truth(OperatorExpression(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, pcBinary), pcBinary);
			}
			mpcGenerator->ComparisonRange(pcBinary);
			return Operands(pcBinary->mpcLeft, pcBinary->mpcRight, &psLeft, &psRight) && Compare(pcBinary->meOperator, psLeft, psRight, pcBinary);
		default:
			break;
		}
	}

	psFunction = mpcGenerator->IsObject(pcExpression) ? mpcGenerator->FindConversion(mpcGenerator->ObjectClass(pcExpression), &mpcTypes->msBool, true, pcExpression) : NULL;
	if (psFunction)
	{
		return Truth(ObjectCall(pcExpression, psFunction, NULL, pcExpression), pcExpression);
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
	CCPPSyntax*					pcCallee;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPSymbol*					pcSymbol;
	SCPP65Function*				psFunction;
	SCPP65Function*				psMember;
	SCPP65Template*				psTemplate;
	SCPP65Type*					psType;
	size						uiErrors;
	int64						lliName;

	if (mpcGenerator->HasPackExpansion(&pcCall->macpArguments))
	{
		return ExpandPackCall(pcCall);
	}
	pcCallee = mpcGenerator->SkipParentheses(pcCall->mpcCallee);
	if (pcCallee->GetKind() == CPPS_MemberExpression)
	{
		return MemberCall(pcCall, pcCallee);
	}
	if (mpcGenerator->IsMemberPointerOperator(pcCallee))
	{
		return MemberPointerCall(pcCall, (CCPPSyntaxBinaryExpression*)pcCallee);
	}
	pcSymbol = NULL;
	pcName = NULL;
	pcComponent = NULL;
	if (pcCallee->GetKind() == CPPS_IdExpression)
	{
		pcName = ((CCPPSyntaxIdExpression*)pcCallee)->mpcName;
		pcSymbol = pcName->mpcSymbol;
		pcComponent = pcName->GetLast();
		if (pcName->IsQualifiedByClass())
		{
			return MemberCall(pcCall, pcCallee);
		}
	}
	psMember = mpcGenerator->mpsFunction;

	//f<Arguments>(...) and f(...) for a function template; in a member function it may be a member template.
	if (pcSymbol && (pcSymbol->meKind == CPPSK_FunctionTemplate))
	{
		if (psMember && psMember->psClass && !pcName->mbGlobal && (mpcGenerator->FindMemberTemplate(psMember->psClass, pcSymbol->mlliName, pcCall->macpArguments.NumElements()) || mpcGenerator->HasMember(psMember->psClass, pcSymbol->mlliName)))
		{
			return MemberCall(pcCall, pcCallee);
		}
		psTemplate = mpcGenerator->GetCallTemplate(pcSymbol, pcSymbol->mlliName, pcCall, (pcComponent && pcComponent->mbTemplateArguments) ? &pcComponent->macpTemplateArguments : NULL);
		if (!psTemplate)
		{
			Error(pcCallee, "The function template has not been defined");
			return NULL;
		}
		psFunction = mpcGenerator->InstantiateFunction(psTemplate, (pcComponent && pcComponent->mbTemplateArguments) ? &pcComponent->macpTemplateArguments : NULL, &pcCall->macpArguments, pcCall);
		if (!psFunction)
		{
			return NULL;
		}
		mpcGenerator->Resolved(pcName, psFunction);
		return Call(psFunction, &pcCall->macpArguments, pcCall);
	}

	//f() in a member function may be a member, even when the parser could not resolve it in a template.
	lliName = pcSymbol ? pcSymbol->mlliName : mpcGenerator->GetFunctionName(pcComponent);
	if (pcName && !pcName->mbGlobal && psMember && psMember->psClass && (!pcSymbol || (pcSymbol->meKind == CPPSK_Function)) && mpcGenerator->HasMember(psMember->psClass, lliName))
	{
		return MemberCall(pcCall, pcCallee);
	}

	if (!pcSymbol && mpcGenerator->IsInlineMemcpy(lliName, &pcCall->macpArguments))
	{
		return InlineMemcpy(&pcCall->macpArguments);
	}
	if (!pcSymbol && mpcGenerator->IsBuiltin(lliName))
	{
		return Builtin(pcCall, lliName);
	}

	if (!pcSymbol && mpcGenerator->mpcSemanticPass && mpcGenerator->IsIntrinsic(lliName))
	{
		//Only checking: an intrinsic of the host's compiler (__debugbreak()), declared by no header.
		return UncheckedCall(&pcCall->macpArguments);
	}
	if (!pcSymbol && pcName)
	{
		//Nothing declares it here: it may be in a namespace of its arguments'.
		uiErrors = mpcGenerator->mcErrors.NumErrors();
		psFunction = mpcGenerator->FindAssociatedFunction(pcName, lliName, pcCall);
		if (psFunction)
		{
			mpcGenerator->Resolved(pcName, psFunction);
			return Call(psFunction, &pcCall->macpArguments, pcCall);
		}
		if (mpcGenerator->mcErrors.NumErrors() != uiErrors)
		{
			return NULL;
		}
	}
	if (!pcSymbol || (pcSymbol->meKind != CPPSK_Function))
	{
		psType = mpcGenerator->TypeOf(pcCallee);
		if (psType && psType->IsPointer() && psType->psBase->IsFunction())
		{
			return CallThroughPointer(pcCall, psType->psBase);
		}
		Error(pcCallee, "Only calls to functions and pointers to functions are supported by the compiler yet");
		return NULL;
	}

	uiErrors = mpcGenerator->mcErrors.NumErrors();
	psFunction = mpcGenerator->FindNamedFunction(pcName, lliName, &pcCall->macpArguments, pcCall);
	if (!psFunction)
	{
		if (mpcGenerator->mcErrors.NumErrors() == uiErrors)
		{
			mpcGenerator->ReportFailedDeclaration(lliName);
			Error(pcCallee, "The function has not been declared");
		}
		return NULL;
	}

	mpcGenerator->Resolved(pcName, psFunction);

	//memcpy of a few bytes, a constant count, is copied where it is called.
	if (psFunction->bCLinkage && mpcGenerator->IsInlineMemcpy(lliName, &pcCall->macpArguments))
	{
		return InlineMemcpy(&pcCall->macpArguments);
	}
	return Call(psFunction, &pcCall->macpArguments, pcCall);
}


//////////////////////////////////////////////////////////////////////////
//
// f(args...): the call with the function parameter pack's elements for args...
// (CCPP65Generator::ExpandPackArguments).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ExpandPackCall(CCPPSyntaxCallExpression* pcCall)
{
	CArraySyntax	apcArguments;
	CCPPSyntax*		pcExpansion;
	SCPP65Type*		psType;

	pcExpansion = mpcGenerator->ExpandPackArguments(pcCall, &apcArguments);
	if (pcExpansion)
	{
		Error(pcExpansion, "Only a function parameter pack can be expanded by the compiler yet");
		return NULL;
	}
	psType = CallExpression(pcCall);
	mpcGenerator->RestoreArguments(pcCall, &apcArguments);
	return psType;
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
SCPP65Type* CCPP65TypeChecker::CallThroughPointer(CCPPSyntaxCallExpression* pcCall, SCPP65Type* psFunctionType)
{
	if (!Arguments(psFunctionType->papsParameters, NULL, psFunctionType->bVariadic, &pcCall->macpArguments, pcCall, psFunctionType->psBase) ||
		!Value(pcCall->mpcCallee))
	{
		return NULL;
	}
	return psFunctionType->psBase;
}


//////////////////////////////////////////////////////////////////////////
//
// A call to psFunction with papcArguments (NULL for none): its result.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::Call(SCPP65Function* psFunction, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	if (!Arguments(&psFunction->mapsParameters, &psFunction->mapcDefaults, psFunction->bVariadic, papcArguments, pcNode, psFunction->psReturn))
	{
		return NULL;
	}
	return psFunction->psReturn;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a call's arguments fit its parameters (papcDefaults are the default
// arguments, NULL for none), last first.  A reference is bound to a place, an object
// is constructed from its argument, anything else is converted to its parameter.
// The arguments for ... are promoted (a float to a double).  A function that
// returns an object makes it in a temporary, so its class must be complete.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::Arguments(CArrayCPP65Types* papsParameters, CArraySyntax* papcDefaults, bool bVariadic, CArraySyntax* papcArguments, CCPPSyntax* pcNode, SCPP65Type* psReturn)
{
	size			uiArguments;
	size			uiParameters;
	size			i;
	SCPP65Type*		psType;
	SCPP65Type*		psParameter;
	CCPPSyntax*		pcArgument;

	uiArguments = papcArguments ? papcArguments->NumElements() : 0;
	uiParameters = papsParameters->NumElements();
	if ((uiArguments > uiParameters) && !bVariadic)
	{
		return Error(pcNode, "The number of arguments does not match the function");
	}

	for (i = uiArguments; i > uiParameters; i--)
	{
		pcArgument = papcArguments->GetPtr(i - 1);
		psType = Value(pcArgument);
		if (!psType)
		{
			return false;
		}
		if (psType->eKind == CPP65TK_Bool)
		{
			psParameter = &mpcTypes->msInt;
		}
		else if (psType->IsFloat())
		{
			psParameter = &mpcTypes->msDouble;
		}
		else if (psType->IsInteger())
		{
			psParameter = mpcTypes->Promote(psType);
		}
		else if (psType->IsArray())
		{
			psParameter = mpcTypes->Pointer(psType->psBase);
		}
		else if (psType->IsFunction())
		{
			psParameter = mpcTypes->Pointer(psType);
		}
		else if (psType->IsPointer())
		{
			psParameter = psType;
		}
		else if (psType->IsClass())
		{
			return Error(pcArgument, "Objects cannot be passed by value by the compiler yet; pass a pointer or a reference");
		}
		else
		{
			return Error(pcArgument, "This variable argument is not supported by the compiler yet");
		}
		if (!Convert(psType, psParameter, pcArgument))
		{
			return false;
		}
	}

	for (i = uiParameters; i > 0; i--)
	{
		pcArgument = (i <= uiArguments) ? papcArguments->GetPtr(i - 1) : (papcDefaults ? papcDefaults->GetPtr(i - 1) : NULL);
		if (!pcArgument)
		{
			return Error(pcNode, "The number of arguments does not match the function");
		}
		psParameter = papsParameters->GetPtr(i - 1);
		if (psParameter->IsReference() && mpcGenerator->BindsToTemporary(psParameter, pcArgument))
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
			continue;
		}
		if (psParameter->IsClass())
		{
			if (!mpcGenerator->CompleteClass(psParameter->psClass, pcArgument) || !Construct(psParameter, NULL, pcArgument, pcArgument))
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
		if (!ImplicitConvert(psType, psParameter, pcArgument))
		{
			return false;
		}
	}

	return !(psReturn && psReturn->IsClass()) || mpcGenerator->CompleteClass(psReturn->psClass, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// GCC's builtin functions the compiler does (CCPP65Generator::FindBuiltin):
// their argument counts, their arguments' types and their results.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::Builtin(CCPPSyntaxCallExpression* pcCall, int64 lliName)
{
	static const char*		aszArguments[] = { "no arguments", "one argument", "two arguments", "three arguments", "four arguments" };
	const SCPP65Builtin*	psBuiltin;
	SCPP65Type*				psType;
	SCPP65Type*				psVoidPointer;
	CArraySyntax*			papcArguments;
	CChars					szError;
	double					dValue;

	if (mpcGenerator->FloatBuiltin(pcCall, &dValue, &psType))
	{
		return psType;
	}

	papcArguments = &pcCall->macpArguments;
	psBuiltin = mpcGenerator->FindBuiltin(lliName);
	if (!psBuiltin)
	{
		if (mpcGenerator->mpcSemanticPass)
		{
			//Only checking: the host compiler's other builtins (__builtin_va_start ...).
			return UncheckedCall(papcArguments);
		}
		Error(pcCall, "This builtin is not supported by the compiler yet");
		return NULL;
	}
	if (psBuiltin->szW65C816Only && (mpcGenerator->mpsTarget->eTarget != CPPT_W65C816))
	{
		Error(pcCall, (char*)psBuiltin->szW65C816Only);
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
	case CPP65B_Unreachable:
	case CPP65B_Pause:
		return &mpcTypes->msVoid;
	case CPP65B_BCDAdd:
	case CPP65B_BCDSubtract:
		return &mpcTypes->msUnsignedInt;
	case CPP65B_Expect:
		psType = Value(papcArguments->GetPtr(0));
		return psType ? mpcTypes->Promote(psType) : NULL;
	case CPP65B_Memcpy:
		//Libc65's memcpy (a small constant count is InlineMemcpy, before this).
		if (Convert(Value(papcArguments->GetPtr(0)), psVoidPointer, papcArguments->GetPtr(0)) &&
			Convert(Value(papcArguments->GetPtr(1)), psVoidPointer, papcArguments->GetPtr(1)) &&
			Convert(Value(papcArguments->GetPtr(2)), mpcTypes->Long(false), papcArguments->GetPtr(2)))
		{
			return psVoidPointer;
		}
		return NULL;
	case CPP65B_Alloca:
		return Convert(Value(papcArguments->GetPtr(0)), &mpcTypes->msUnsignedInt, pcCall) ? psVoidPointer : NULL;
	default:
		//ctz, clz and popcount count the bits of their operand type.
		return Convert(Value(papcArguments->GetPtr(0)), mpcGenerator->BuiltinOperandType(psBuiltin), pcCall) ? &mpcTypes->msInt : NULL;
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
// object.f(...), pointer->f(...), Class::f(...) and, in a member function, f(...):
// the member function's result.  A destructor's call is void.  object.Base::f() looks
// f up in Base.  A member template is used when it has template arguments or no
// other function matches.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::MemberCall(CCPPSyntaxCallExpression* pcCall, CCPPSyntax* pcCallee)
{
	CCPPSyntaxMemberExpression*	pcMember;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPSyntax*					pcObject;
	SCPP65Class*				psClass;
	SCPP65Class*				psLookup;
	SCPP65Function*				psFunction;
	SCPP65Template*				psTemplate;
	SCPP65Type*					psType;
	int64						lliName;
	int							iOffset;
	int							iConstObject;
	size						uiErrors;
	bool						bArrow;

	psClass = NULL;
	pcObject = NULL;
	bArrow = false;
	if (pcCallee->GetKind() == CPPS_MemberExpression)
	{
		pcMember = (CCPPSyntaxMemberExpression*)pcCallee;
		psType = mpcGenerator->TypeOf(pcMember->mpcObject);
		if (!psType)
		{
			//Its errors.
			Value(pcMember->mpcObject);
			return NULL;
		}
		bArrow = pcMember->mbArrow;
		if (bArrow)
		{
			psType = mpcGenerator->ArrowType(pcMember->mpcObject, pcCallee);
			psType = (psType && psType->IsPointer()) ? psType->psBase : NULL;
		}
		if (!psType || !psType->IsClass())
		{
			Error(pcCallee, "Member functions need an object of a class");
			return NULL;
		}
		psClass = psType->psClass;
		if (!mpcGenerator->CompleteClass(psClass, pcCallee))
		{
			return NULL;
		}
		pcObject = pcMember->mpcObject;
		pcName = pcMember->mpcName;
	}
	else
	{
		pcName = ((CCPPSyntaxIdExpression*)pcCallee)->mpcName;
		if (pcName->IsQualifiedByClass())
		{
			psClass = mpcGenerator->GetComponentClass(pcName->GetQualifier(), NULL, pcCallee);
		}
		else if (mpcGenerator->mpsFunction)
		{
			psClass = mpcGenerator->mpsFunction->psClass;
		}
	}

	//p->~X(), o.~X(), p->X::~X() or ~X() (this's): nothing if the class has no destructor.
	if (pcName->GetLast()->meKind == CPPNC_Destructor)
	{
		if (!psClass || (pcCall->macpArguments.NumElements() != 0) || !mpcGenerator->CompleteClass(psClass, pcCallee))
		{
			Error(pcCallee, "This destructor call is not supported by the compiler yet");
			return NULL;
		}
		if (!psClass->psDestructor)
		{
			return &mpcTypes->msVoid;
		}
		mpcGenerator->Resolved(pcName, psClass->psDestructor);
		return MemberFunctionCall(pcCall, pcCallee, pcObject, psClass->psDestructor);
	}

	lliName = mpcGenerator->GetFunctionName(pcName->GetLast());
	if (!psClass || (lliName == -1))
	{
		Error(pcCallee, "This call is not supported by the compiler yet");
		return NULL;
	}

	psLookup = psClass;
	if (pcObject && pcName->IsQualifiedByClass())
	{
		psLookup = mpcGenerator->QualifierClass(psClass, pcName->GetQualifier(), pcCallee);
		if (!psLookup)
		{
			return NULL;
		}
	}
	//object.pf() calls through a field that points to a function.
	if (pcObject && !mpcGenerator->HasMember(psLookup, lliName) && mpcGenerator->FindField(psLookup, lliName, &iOffset))
	{
		psType = mpcGenerator->TypeOf(pcCallee);
		if (psType && psType->IsPointer() && psType->psBase->IsFunction())
		{
			return CallThroughPointer(pcCall, psType->psBase);
		}
	}

	uiErrors = mpcGenerator->mcErrors.NumErrors();
	pcComponent = pcName->GetLast();
	iConstObject = pcObject ? (bArrow ? mpcGenerator->IsConstPointer(pcObject) : mpcGenerator->IsConstObject(pcObject)) : mpcGenerator->IsConstThis();
	psTemplate = mpcGenerator->CompleteClass(psLookup, pcCall) ? mpcGenerator->ChooseMemberTemplate(psLookup, lliName, pcCall, pcComponent->mbTemplateArguments ? &pcComponent->macpTemplateArguments : NULL, iConstObject) : NULL;
	psFunction = NULL;
	if (!psTemplate || !pcComponent->mbTemplateArguments)
	{
		if (psTemplate)
		{
			mpcGenerator->BeginQuiet();
		}
		psFunction = mpcGenerator->FindFunction(psLookup, lliName, CPP65FK_Function, &pcCall->macpArguments, true, pcCall, iConstObject);
		if (psTemplate)
		{
			mpcGenerator->EndQuiet();
		}
	}
	if (!psFunction && psTemplate)
	{
		psFunction = mpcGenerator->InstantiateFunction(psTemplate, pcComponent->mbTemplateArguments ? &pcComponent->macpTemplateArguments : NULL, &pcCall->macpArguments, pcCall);
	}
	if (!psFunction)
	{
		if (mpcGenerator->mcErrors.NumErrors() == uiErrors)
		{
			Error(pcCallee, "The class has no member function of that name");
		}
		return NULL;
	}
	if (!pcObject && !psFunction->bStatic && (!mpcGenerator->mpsFunction || !mpcGenerator->mpsFunction->psClass || !mpcGenerator->IsDerived(mpcGenerator->mpsFunction->psClass, psFunction->psClass, NULL)))
	{
		Error(pcCallee, "A member function needs an object");
		return NULL;
	}
	mpcGenerator->Resolved(pcName, psFunction);
	return MemberFunctionCall(pcCall, pcCallee, pcObject, psFunction);
}


//////////////////////////////////////////////////////////////////////////
//
// A call to a member function on pcObject (this's when NULL): its arguments, then
// its object (a pointer for ->), and its result.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::MemberFunctionCall(CCPPSyntaxCallExpression* pcCall, CCPPSyntax* pcCallee, CCPPSyntax* pcObject, SCPP65Function* psFunction)
{
	bool	bObject;

	if (!Arguments(&psFunction->mapsParameters, &psFunction->mapcDefaults, psFunction->bVariadic, &pcCall->macpArguments, pcCall, psFunction->psReturn))
	{
		return NULL;
	}
	bObject = true;
	if (!psFunction->bStatic && pcObject)
	{
		bObject = ((CCPPSyntaxMemberExpression*)pcCallee)->mbArrow ? (ArrowPointer(pcObject, pcCallee) != NULL) : (Place(pcObject) != NULL);
	}
	return bObject ? psFunction->psReturn : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A call to an object's member function (an operator or a conversion function):
// its arguments (NULL for none), the object's place, and its result.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ObjectCall(CCPPSyntax* pcObject, SCPP65Function* psFunction, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	if (!Call(psFunction, papcArguments, pcNode) || !Place(pcObject))
	{
		return NULL;
	}
	return psFunction->psReturn;
}


//////////////////////////////////////////////////////////////////////////
//
// object op arguments, a call to the object's operator function of eOperator.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::OperatorCall(CCPPSyntax* pcObject, ECppTokens eOperator, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	SCPP65Type*		psType;
	SCPP65Function*	psFunction;
	size			uiErrors;

	psType = mpcGenerator->TypeOf(pcObject);
	if (!psType || !psType->IsClass())
	{
		Error(pcNode, "An operator call needs an object");
		return NULL;
	}
	uiErrors = mpcGenerator->mcErrors.NumErrors();
	psFunction = mpcGenerator->FindFunction(psType->psClass, mpcGenerator->GetOperatorName(eOperator, false), CPP65FK_Function, papcArguments, true, pcNode, mpcGenerator->IsConstObject(pcObject));
	if (!psFunction)
	{
		if (mpcGenerator->mcErrors.NumErrors() == uiErrors)
		{
			Error(pcNode, "The class has no operator for this");
		}
		return NULL;
	}
	return ObjectCall(pcObject, psFunction, papcArguments, pcNode);
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
	SCPP65Type*			psType;
	SCPP65Function*		psFunction;
	CArraySyntax		apcArguments;
	int64				lliName;
	size				uiErrors;

	lliName = mpcGenerator->GetOperatorName(eOperator, false);
	psType = mpcGenerator->ValueTypeOf(pcLeft);
	apcArguments.Init();
	if (psType && psType->IsClass() && mpcGenerator->CompleteClass(psType->psClass, pcNode) && mpcGenerator->HasMember(psType->psClass, lliName))
	{
		if (pcRight)
		{
			apcArguments.Add(pcRight);
		}
		uiErrors = mpcGenerator->mcErrors.NumErrors();
		mpcGenerator->BeginQuiet();
		psFunction = mpcGenerator->FindFunction(psType->psClass, lliName, CPP65FK_Function, &apcArguments, true, pcNode, mpcGenerator->IsConstObject(pcLeft));
		mpcGenerator->EndQuiet();
		mpcGenerator->mcErrors.Truncate(uiErrors);
		if (psFunction)
		{
			psType = OperatorCall(pcLeft, eOperator, &apcArguments, pcNode);
			apcArguments.Kill();
			return psType;
		}
		apcArguments.ReInit();
	}

	apcArguments.Add(pcLeft);
	if (pcRight)
	{
		apcArguments.Add(pcRight);
	}
	psFunction = mpcGenerator->FindFreeOperator(lliName, &apcArguments, pcNode);
	if (!psFunction)
	{
		Error(pcNode, "No operator matches the operands");
		apcArguments.Kill();
		return NULL;
	}
	psType = Call(psFunction, &apcArguments, pcNode);
	apcArguments.Kill();
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The pointer that object-> goes through: the object's own (a pointer), or what its
// operator-> returns, which must be a pointer.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ArrowPointer(CCPPSyntax* pcObject, CCPPSyntax* pcNode)
{
	SCPP65Type*		psType;

	psType = mpcGenerator->TypeOf(pcObject);
	if (!psType || !psType->IsClass())
	{
		return Value(pcObject);
	}
	psType = OperatorCall(pcObject, CT_MemberByPointer, &mapcNone, pcNode);
	if (psType && !psType->IsPointer())
	{
		Error(pcNode, "operator-> must return a pointer for the compiler");
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
SCPP65Type* CCPP65TypeChecker::FieldType(SCPP65Field* psField, bool bConst, bool bVolatile)
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
	SCPP65Type*		psType;
	SCPP65Field*	psField;
	SCPP65Variable*	psVariable;
	SCPP65Class*	psClass;
	int64			lliName;
	int				iOffset;

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

	psClass = psType->psClass;
	if (!mpcGenerator->CompleteClass(psClass, pcMember))
	{
		return NULL;
	}
	lliName = mpcGenerator->GetComponentName(pcMember->mpcName->GetLast());
	psField = mpcGenerator->FindField(psClass, lliName, &iOffset);
	if (psField)
	{
		mpcGenerator->Resolved(pcMember->mpcName, psField->pcSymbol);
		return FieldType(psField, psType->bConst, psType->bVolatile);
	}
	psVariable = mpcGenerator->FindStatic(psClass, lliName);
	if (psVariable)
	{
		mpcGenerator->Resolved(pcMember->mpcName, psVariable->pcSymbol);
		return psVariable->psType;
	}
	Error(pcMember, "The class has no member of that name");
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The pointer to member's type of object.*member or pointer->*member, once the
// object is an object of a class.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::MemberPointerObject(CCPPSyntaxBinaryExpression* pcBinary)
{
	SCPP65Type*		psMember;
	SCPP65Type*		psObject;
	SCPP65Class*	psClass;

	psMember = mpcGenerator->TypeOf(pcBinary->mpcRight);
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
		psClass = psObject->psBase->psClass;
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
		psClass = psObject->psClass;
	}
	if (psMember->psClass && (psClass != psMember->psClass))
	{
		//A base further on in the object is where the member is.
		mpcGenerator->CompleteClass(psClass, pcBinary);
	}
	return psMember;
}


//////////////////////////////////////////////////////////////////////////
//
// object.*field and pointer->*field: the field's type.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::MemberPointerPlace(CCPPSyntaxBinaryExpression* pcBinary)
{
	SCPP65Type*	psMember;

	psMember = MemberPointerObject(pcBinary);
	if (!psMember)
	{
		return NULL;
	}
	if (psMember->psBase->IsFunction())
	{
		Error(pcBinary, "A pointer to a member function can only be called");
		return NULL;
	}
	return Value(pcBinary->mpcRight) ? psMember->psBase : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// (object.*function)(...) and (pointer->*function)(...): the member function's
// result.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::MemberPointerCall(CCPPSyntaxCallExpression* pcCall, CCPPSyntaxBinaryExpression* pcBinary)
{
	SCPP65Type*	psMember;
	SCPP65Type*	psFunctionType;

	psMember = mpcGenerator->TypeOf(pcBinary->mpcRight);
	if (!psMember || !psMember->IsMemberPointer() || !psMember->psBase->IsFunction())
	{
		Error(pcBinary, "Only a pointer to a member function can be called");
		return NULL;
	}
	psFunctionType = psMember->psBase;
	if (!Arguments(psFunctionType->papsParameters, NULL, psFunctionType->bVariadic, &pcCall->macpArguments, pcCall, psFunctionType->psBase) ||
		!MemberPointerObject(pcBinary) ||
		!Value(pcBinary->mpcRight))
	{
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
	CCPPSyntaxIdExpression*		pcId;
	CCPPSyntaxUnaryExpression*	pcUnary;
	CCPPSyntaxBinaryExpression*	pcBinary;
	SCPP65Variable*				psVariable;
	SCPP65Field*				psField;
	CCPPSymbol*					pcSymbol;
	SCPP65Type*					psType;
	SCPP65Class*				psClass;
	CCPPSyntax*					pcLeft;
	int							iOffset;

	pcExpression = mpcGenerator->SkipParentheses(pcExpression);
	switch (pcExpression->GetKind())
	{
	case CPPS_IdExpression:
		//A variable, or a field of this.
		pcId = (CCPPSyntaxIdExpression*)pcExpression;
		pcSymbol = pcId->mpcName->mpcSymbol;
		psVariable = pcSymbol ? mpcGenerator->GetVariable(pcSymbol) : NULL;
		if (psVariable)
		{
			return psVariable->psType->WithoutReference();
		}
		psClass = mpcGenerator->mpsFunction ? mpcGenerator->mpsFunction->psClass : NULL;
		psField = (pcSymbol && (pcSymbol->meKind == CPPSK_Field) && psClass) ? mpcGenerator->FindFieldSymbol(psClass, pcSymbol, &iOffset) : NULL;
		if (!psField && !pcSymbol && psClass && (pcId->mpcName->macpComponents.NumElements() == 1))
		{
			//A name in a template the parser could not resolve.
			psField = mpcGenerator->FindField(psClass, mpcGenerator->GetComponentName(pcId->mpcName->GetLast()), &iOffset);
		}
		if (!psField)
		{
			if (mpcGenerator->IsFunctionName(pcId))
			{
				Error(pcExpression, "This is not an lvalue");
				return NULL;
			}
			Error(pcExpression, "Only variables are supported by the compiler here yet");
			return NULL;
		}
		if (!mpcGenerator->mbThis)
		{
			Error(pcExpression, "A field needs an object");
			return NULL;
		}
		return FieldType(psField, mpcGenerator->IsConstThis(), false);

	case CPPS_MemberExpression:
		return MemberPlace((CCPPSyntaxMemberExpression*)pcExpression);

	case CPPS_UnaryExpression:
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		if (pcUnary->meOperator != CT_Multiply)
		{
			break;
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

	case CPPS_CallExpression:
		//A function that returns a reference, or an object (its temporary).
		return ResultPlace(Result(pcExpression), pcExpression);

	case CPPS_BinaryExpression:
		if (mpcGenerator->IsMemberPointerOperator(pcExpression))
		{
			return MemberPointerPlace((CCPPSyntaxBinaryExpression*)pcExpression);
		}
		pcBinary = (CCPPSyntaxBinaryExpression*)pcExpression;
		pcLeft = mpcGenerator->SkipParentheses(pcBinary->mpcLeft);
		if (mpcGenerator->IsAssignmentOperator(pcBinary->meOperator) && (pcLeft->GetKind() == CPPS_IdExpression) && !mpcGenerator->IsObject(pcLeft))
		{
			//&(c = 'A'): the assignment, then its variable's place.
			return Assignment(pcBinary) ? Place(pcLeft) : NULL;
		}
		break;

	default:
		break;
	}
	if (mpcGenerator->IsObject(pcExpression))
	{
		//An expression that makes an object: its temporary.
		return ResultPlace(Result(pcExpression), pcExpression);
	}
	Error(pcExpression, "This lvalue is not supported by the compiler yet");
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

	psType = mpcGenerator->TypeOf(pcSubscript->mpcArray);
	if (psType && psType->IsClass())
	{
		apcIndex.Init();
		apcIndex.Add(pcSubscript->mpcIndex);
		psType = OperatorCall(pcSubscript->mpcArray, CT_OpenSquareBracket, &apcIndex, pcSubscript);
		apcIndex.Kill();
		return ResultPlace(psType, pcSubscript);
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

	psTo = mpcGenerator->GetTypeIdType(pcTypeId);
	if (!psTo)
	{
		return NULL;
	}
	if (psTo->IsClass())
	{
		return TemporaryObject(psTo, pcOperand, pcNode);
	}

	pcFunction = mpcGenerator->FunctionNameFor(pcOperand, psTo);
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
	if (!psTo->IsScalar())
	{
		Error(pcNode, "Only casts to integers and pointers are supported by the compiler yet");
		return NULL;
	}
	return Convert(psFrom, psTo, pcNode) ? psTo : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Type(arguments): a temporary of a class, or a cast of one value.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ConstructExpression(CCPPSyntaxConstructExpression* pcConstruct)
{
	SCPP65Type*		psType;
	CArraySyntax*	papcElements;

	psType = mpcGenerator->GetTypeIdType(pcConstruct->mpcType);
	if (psType && psType->IsClass())
	{
		return TemporaryObject(psType, pcConstruct->mpcInitializer, pcConstruct);
	}
	if (pcConstruct->mpcInitializer && (pcConstruct->mpcInitializer->GetKind() == CPPS_ParenthesisedInitializer))
	{
		papcElements = &((CCPPSyntaxParenthesisedInitializer*)pcConstruct->mpcInitializer)->macpElements;
		if (papcElements->NumElements() == 1)
		{
			return CastExpression(pcConstruct->mpcType, papcElements->GetPtr(0), pcConstruct);
		}
	}
	Error(pcConstruct, "This construction is not supported by the compiler yet");
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// sizeof is an unsigned int, a type trait (__is_trivially_constructible(T) ...) a
// bool; both are constants.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::SizeofExpression(CCPPSyntax* pcSizeof)
{
	int64	lliBytes;

	if (mpcGenerator->mpcSemanticPass)
	{
		//Only checking: the value isn't needed, only the type (__builtin_va_arg(ap, T) is a T).
		return CheckedSizeofType(pcSizeof);
	}
	if (!mpcGenerator->Constant(pcSizeof, &lliBytes))
	{
		Error(pcSizeof, "This sizeof or type trait is not supported by the compiler yet");
		return NULL;
	}
	return (pcSizeof->GetKind() == CPPS_SizeofExpression) ? mpcTypes->Size() : &mpcTypes->msBool;
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
	CChars							szTrait;
	SCPP65Type*						psType;

	if (pcSizeof->GetKind() == CPPS_SizeofExpression)
	{
		return mpcTypes->Size();
	}
	pcTrait = (CCPPSyntaxTypeTraitExpression*)pcSizeof;
	szTrait.Init();
	mpcGenerator->mpcTokeniser->GetSource(mpcGenerator->mpcTokeniser->Get(pcTrait->miName))->Append(&szTrait);
	psType = &mpcTypes->msBool;
	if (szTrait.Equals("__builtin_va_arg"))
	{
		psType = NULL;
		if ((pcTrait->macpArguments.NumElements() == 2) && (pcTrait->macpArguments.GetPtr(1)->GetKind() == CPPS_TypeId))
		{
			Value(pcTrait->macpArguments.GetPtr(0));
			psType = mpcGenerator->GetTypeIdType((CCPPSyntaxTypeId*)pcTrait->macpArguments.GetPtr(1));
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
// new Type, new Type(...), new(address) Type and new Type[count]: a Type*.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::NewExpression(CCPPSyntaxNewExpression* pcNew)
{
	SCPP65Type*		psType;
	SCPP65Type*		psAddress;

	if (pcNew->mbPlacement && ((pcNew->macpPlacement.NumElements() != 1) || (pcNew->macpArraySizes.NumElements() != 0)))
	{
		Error(pcNew, "Only new(address) Type is supported for placement new by the compiler yet");
		return NULL;
	}
	psType = mpcGenerator->GetTypeIdType(pcNew->mpcType);
	if (!psType || (psType->IsClass() && !mpcGenerator->CompleteClass(psType->psClass, pcNew)))
	{
		return NULL;
	}
	if (pcNew->macpArraySizes.NumElements() > 0)
	{
		return NewArray(pcNew, psType);
	}

	if (pcNew->mbPlacement)
	{
		psAddress = Value(pcNew->macpPlacement.GetPtr(0));
		if (!psAddress)
		{
			return NULL;
		}
		if (!psAddress->IsPointer())
		{
			Error(pcNew, "Placement new needs an address");
			return NULL;
		}
	}
	if ((psType->IsClass() || pcNew->mpcInitializer) && !Construct(psType, NULL, pcNew->mpcInitializer, pcNew))
	{
		return NULL;
	}
	return mpcTypes->Pointer(psType);
}


//////////////////////////////////////////////////////////////////////////
//
// new Type[count]: one dimension, its elements constructed by default.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::NewArray(CCPPSyntaxNewExpression* pcNew, SCPP65Type* psType)
{
	if ((pcNew->macpArraySizes.NumElements() != 1) || (pcNew->mpcInitializer && (pcNew->mpcInitializer->GetKind() != CPPS_ParenthesisedInitializer)))
	{
		Error(pcNew, "Only new Type[count] (one dimension, no initialiser list) is supported by the compiler yet");
		return NULL;
	}
	if (pcNew->mpcInitializer && psType->IsScalar())
	{
		Error(pcNew, "new Type[count]() is not supported by the compiler yet");
		return NULL;
	}
	if (!Convert(Value(pcNew->macpArraySizes.GetPtr(0)), &mpcTypes->msUnsignedInt, pcNew))
	{
		return NULL;
	}
	if (psType->IsClass() && mpcGenerator->NeedsConstructor(psType->psClass) && !Construct(psType, NULL, NULL, pcNew))
	{
		return NULL;
	}
	return mpcTypes->Pointer(psType);
}


//////////////////////////////////////////////////////////////////////////
//
// delete pointer and delete[] pointer: void.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::DeleteExpression(CCPPSyntaxDeleteExpression* pcDelete)
{
	SCPP65Type*		psPointer;
	SCPP65Type*		psType;

	psPointer = Value(pcDelete->mpcOperand);
	if (!psPointer)
	{
		return NULL;
	}
	if (!psPointer->IsPointer())
	{
		Error(pcDelete, "delete needs a pointer");
		return NULL;
	}
	psType = psPointer->psBase;
	if (psType->IsClass() && !mpcGenerator->CompleteClass(psType->psClass, pcDelete))
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
	if (!mpcGenerator->CompleteClass(psType->psClass, pcNode) || !Construct(psType, pcInitialiser ? NULL : &mapcNone, pcInitialiser, pcNode))
	{
		return NULL;
	}
	return psType;
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
	SCPP65Class*					psClass;
	int								i;
	bool							bResult;

	apcArguments.Init();
	if (!papcArguments)
	{
		papcArguments = &apcArguments;
	}
	if (pcInitialiser && ((pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer) || (pcInitialiser->GetKind() == CPPS_InitializerList)))
	{
		papcArguments = (pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer) ? &((CCPPSyntaxParenthesisedInitializer*)pcInitialiser)->macpElements : &((CCPPSyntaxInitializerList*)pcInitialiser)->macpElements;
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
		bResult = !pcInitialiser || ImplicitConvert(ValueFor(pcInitialiser, psType, false), psType, pcInitialiser);
		apcArguments.Kill();
		return bResult;
	}

	psClass = psType->psClass;
	if (!mpcGenerator->CompleteClass(psClass, pcNode))
	{
		apcArguments.Kill();
		return false;
	}

	//(object) copies it, as = object does.
	if (!pcInitialiser && (papcArguments->NumElements() == 1) && !psClass->bConstructors)
	{
		psSource = ObjectType(papcArguments->GetPtr(0));
		if (psSource && mpcGenerator->IsDerived(psSource->psClass, psClass, NULL))
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
			if ((mpcGenerator->GetTypeIdType(pcConstruct->mpcType) == psType) && pcConstruct->mpcInitializer && (pcConstruct->mpcInitializer->GetKind() == CPPS_ParenthesisedInitializer))
			{
				bResult = ConstructClass(psClass, &((CCPPSyntaxParenthesisedInitializer*)pcConstruct->mpcInitializer)->macpElements, pcNode);
				apcArguments.Kill();
				return bResult;
			}
		}

		//= object copies it (or is the object, made in place), = value calls a constructor that takes it.
		psSource = ObjectType(pcInitialiser);
		if (psSource && mpcGenerator->IsDerived(psSource->psClass, psClass, NULL))
		{
			bResult = mpcGenerator->IsMadeObject(pcInitialiser, psClass) ? (Value(pcInitialiser) != NULL) : CopyConstruct(psClass, pcInitialiser, pcNode);
			apcArguments.Kill();
			return bResult;
		}
		apcArguments.Add(pcInitialiser);
		papcArguments = &apcArguments;
	}
	bResult = ConstructClass(psClass, papcArguments, pcNode);
	apcArguments.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an object of psClass can be made from papcArguments: by the constructor
// that matches them, by default when it has none, or else as an aggregate (its
// fields in order, the rest zero).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::ConstructClass(SCPP65Class* psClass, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	SCPP65Function*	psConstructor;
	size			i;
	bool			bResult;

	if (!mpcGenerator->CompleteClass(psClass, pcNode))
	{
		return false;
	}
	if (psClass->bConstructors)
	{
		psConstructor = mpcGenerator->FindFunction(psClass, psClass->lliName, CPP65FK_Constructor, papcArguments, false, pcNode);
		if (!psConstructor)
		{
			return Error(pcNode, "No constructor matches the arguments");
		}
		return Call(psConstructor, papcArguments, pcNode) != NULL;
	}

	if (papcArguments->NumElements() == 0)
	{
		psConstructor = mpcGenerator->FindFunction(psClass, psClass->lliName, CPP65FK_Constructor, papcArguments, false, pcNode);
		return !psConstructor || (Call(psConstructor, papcArguments, pcNode) != NULL);
	}

	if ((psClass->macBases.NumElements() > 0) || mpcGenerator->IsPolymorphic(psClass) || (papcArguments->NumElements() > psClass->macFields.NumElements()))
	{
		return Error(pcNode, "These initialisers do not match the class");
	}
	bResult = true;
	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		bResult &= Construct(psClass->macFields.Get(i)->psType, NULL, (i < papcArguments->NumElements()) ? papcArguments->GetPtr(i) : NULL, pcNode);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an object of psClass can be made a copy of pcSource: by its copy
// constructor, member by member, or byte by byte.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::CopyConstruct(SCPP65Class* psClass, CCPPSyntax* pcSource, CCPPSyntax* pcNode)
{
	CArraySyntax	apcArguments;
	bool			bResult;

	if (mpcGenerator->HasCopyConstructor(psClass))
	{
		apcArguments.Init();
		apcArguments.Add(pcSource);
		bResult = ConstructClass(psClass, &apcArguments, pcNode);
		apcArguments.Kill();
		return bResult;
	}
	if (!mpcGenerator->IsTriviallyCopyable(psClass))
	{
		return Place(pcSource) != NULL;
	}
	return CopyObject(psClass, pcSource, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcSource's bytes can be copied to an object of psClass: it must be an
// object of psClass or a class derived from it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::CopyObject(SCPP65Class* psClass, CCPPSyntax* pcSource, CCPPSyntax* pcNode)
{
	SCPP65Type*	psSource;

	psSource = Place(pcSource);
	if (!psSource)
	{
		return false;
	}
	if (!psSource->IsClass() || !mpcGenerator->IsDerived(psSource->psClass, psClass, NULL))
	{
		return Error(pcNode, "The object cannot be copied to that class");
	}
	return true;
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

	psType = mpcGenerator->ValueTypeOf(pcExpression);
	return (psType && psType->IsClass()) ? psType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a variable of psType can be initialised with pcInitialiser
// (NULL for none), as the IR front end initialises a local: an object (or
// an array of them) is constructed, a reference is bound to its place (or
// to a temporary of its value), anything else takes the converted value
// of = value, (value) or {value}.  An array's list (or string) has each
// element's value.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::Initialise(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode)
{
	CArraySyntax*	papcElements;
	SCPP65Type*		psElement;
	SCPP65Type*		psValue;
	size			i;
	bool			bResult;

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

	papcElements = NULL;
	if (pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer)
	{
		papcElements = &((CCPPSyntaxParenthesisedInitializer*)pcInitialiser)->macpElements;
	}
	else if (pcInitialiser->GetKind() == CPPS_InitializerList)
	{
		papcElements = &((CCPPSyntaxInitializerList*)pcInitialiser)->macpElements;
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
			return Value(pcInitialiser) != NULL;
		}
		bResult = true;
		for (i = 0; i < papcElements->NumElements(); i++)
		{
			bResult &= Initialise(psType->IsArray() ? psType->psBase : psType, papcElements->GetPtr(i), pcNode);
		}
		return bResult;
	}

	if (psType->IsReference())
	{
		mpcGenerator->BeginQuiet();
		psValue = Place(pcInitialiser);
		mpcGenerator->EndQuiet();
		return psValue ? CanBind(psType, psValue, pcNode) : (Value(pcInitialiser) != NULL);
	}
	return ImplicitConvert(ValueFor(pcInitialiser, psType, false), psType, pcNode);
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
	if (psFrom->IsClass() || psTo->IsClass())
	{
		return Error(pcNode, "Objects can only be copied or used through pointers by the compiler yet");
	}
	if (!psFrom->IsScalar() || !psTo->IsScalar())
	{
		return Error(pcNode, "This conversion is not supported by the compiler yet");
	}
	if (!mpcGenerator->Supported(psFrom, pcNode) || !mpcGenerator->Supported(psTo, pcNode))
	{
		return false;
	}

	//A pointer to a derived class points at its base, which may be further on: the derived class is laid out.
	if (psFrom->IsPointer() && psTo->IsPointer() && psFrom->psBase->IsClass() && psTo->psBase->IsClass() && (psFrom->psBase->psClass != psTo->psBase->psClass) && mpcGenerator->IsDerived(psFrom->psBase->psClass, psTo->psBase->psClass, NULL))
	{
		mpcGenerator->CompleteClass(psFrom->psBase->psClass, pcNode);
		return true;
	}

	//A pointer to a member of a base is a pointer to a member of a class derived from it ([conv.mem]).
	if (psFrom->IsMemberPointer() && psTo->IsMemberPointer() && psFrom->psClass && psTo->psClass && (psFrom->psClass != psTo->psClass) && mpcGenerator->CompleteClass(psTo->psClass, pcNode) && mpcGenerator->IsDerived(psTo->psClass, psFrom->psClass, &iOffset) && (iOffset != 0) && psFrom->psBase->IsFunction())
	{
		return Error(pcNode, "A pointer to a member function of a base further on in the class is not supported by the compiler yet");
	}

	if ((psFrom->IsFloat() || psTo->IsFloat()) && (psTo->eKind != CPP65TK_Bool) && (psFrom->IsPointer() || psTo->IsPointer()))
	{
		return Error(pcNode, "A pointer cannot be converted to or from floating point");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Convert, for a conversion no cast asks for: a pointer can't lose const or
// volatile (KeepsPointerQualifiers).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::ImplicitConvert(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode)
{
	return Convert(psFrom, psTo, pcNode) && KeepsPointerQualifiers(psFrom, psTo, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pointer psFrom converts to psTo (any other types do here) without
// losing const or volatile from what it points to.  [conv.qual]
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::KeepsPointerQualifiers(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode)
{
	bool	bKept;

	if (!psFrom->IsPointer() || !psTo->IsPointer())
	{
		return true;
	}
	bKept = psFrom->psBase->Equals(psTo->psBase) ? (mpcGenerator->QualificationRank(psTo, psFrom, false) != CPP65CR_None) : mpcGenerator->AddsQualifiers(psTo->psBase, psFrom->psBase);
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
// Whether reference psReference can bind to a place of type psPlace: it has
// the place's const and volatile.  [dcl.init.ref]
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::CanBind(SCPP65Type* psReference, SCPP65Type* psPlace, CCPPSyntax* pcNode)
{
	if (psPlace->IsArray() && psReference->psBase->IsPointer())
	{
		return Error(pcNode, "An array cannot be bound to a reference to a pointer that isn't const");
	}
	return mpcGenerator->AddsQualifiers(psReference->psBase, psPlace) || Error(pcNode, "A const object cannot be bound to a reference that is not const");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcValue can initialise a psTo (a variable, or what a function
// returns) with the const and volatile it has.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::KeepsQualifiers(CCPPSyntax* pcValue, SCPP65Type* psTo)
{
	SCPP65Type*	psValue;

	psValue = mpcGenerator->ValueTypeOf(pcValue);
	if (!psValue)
	{
		return true;
	}
	if (psTo->IsReference())
	{
		return CanBind(psTo, psValue, pcValue);
	}
	return KeepsPointerQualifiers(psValue->IsArray() ? mpcTypes->Pointer(psValue->psBase) : psValue, psTo, pcValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::Error(CCPPSyntax* pcNode, const char* szMessage)
{
	return mpcGenerator->Error(pcNode, szMessage);
}
