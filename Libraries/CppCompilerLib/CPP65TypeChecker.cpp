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
	mpcTypes = pcUnit->GetTypes();
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
// explicit one when bExplicit), and a list in braces is its one value or zero.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ValueFor(CCPPSyntax* pcExpression, SCPP65Type* psTo, bool bExplicit)
{
	CCPPBoundConversion			cBound;
	CCPPSyntaxIdExpression*		pcFunctionName;
	SCPP65Type*					psMember;

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
		return ObjectCall(pcExpression, cBound.mpcFunction, NULL, pcExpression);
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
	mpcUnit->Typed(pcExpression, mpsResult);
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

	psType = mpcUnit->TypeOf(pcNode->mpcArray);
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
	mpsResult = mpcUnit->UserLiteralToken(pcNode) ? LiteralCall(pcNode) : ConstantValue(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65TypeChecker::VisitStringExpression(CCPPSyntaxStringExpression* pcNode)
{
	mpsResult = mpcUnit->UserLiteralToken(pcNode) ? LiteralCall(pcNode) : StringLiteral(pcNode);
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

	psCall = mpcUnit->GetLiteralCall(pcLiteral);
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
	pcEnumerator = mpcUnit->MemberEnumerator(pcNode);
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
	mpsResult = mpcUnit->LambdaType(pcNode);
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
	if (pcNode->meCast == CT_dynamic_cast)
	{
		//Its pointer or reference type, whatever the operand (CCPP65IRFrontEnd::DynamicCastExpression checks it).
		mpsResult = mpcUnit->GetDeclarators()->GetTypeIdType(pcNode->mpcType);
		return;
	}
	mpsResult = CastExpression(pcNode->mpcType, pcNode->mpcOperand, pcNode);
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

	mpcUnit->BeginUnevaluated();
	psOperand = Value(pcNode->mpcOperand);
	mpcUnit->EndUnevaluated();
	mpsResult = psOperand ? &mpcTypes->msBool : NULL;
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
	bool		bDynamic;

	mpsResult = NULL;
	pcTypeInfo = mpcUnit->TypeInfoClass();
	if (!pcTypeInfo)
	{
		Error(pcNode, "typeid needs std::type_info, from <typeinfo>");
		return;
	}
	if (pcNode->mpcOperand && !Value(pcNode->mpcOperand))
	{
		return;
	}
	if (!mpcUnit->TypeidType(pcNode, &bDynamic))
	{
		Error(pcNode, "typeid of this is not supported by the compiler yet");
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
		if (psType->IsFloat() && mpcUnit->FloatConstant(pcExpression, &dValue))
		{
			return psType;
		}
	}
	if (!mpcUnit->Constant(pcExpression, &lliValue))
	{
		mpcUnit->ConstantError(pcExpression, "The value is not a constant");
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
	bDecoded = mpcUnit->DecodeString(pcString, &auiBytes);
	auiBytes.Kill();
	return bDecoded ? mpcTypes->Pointer(mpcTypes->Const(mpcUnit->StringElementType(pcString))) : NULL;
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
		mpcUnit->ReportOnce(pcId, "The name is ambiguous: it finds more than one thing");
		return NULL;
	}
	mpcUnit->GetBinder()->BindName(pcId, &cBound);
	switch (cBound.meWhat)
	{
	case CPPBN_FunctionName:
		return mpcTypes->Pointer(&mpcTypes->msChar);
	case CPPBN_EnclosingConstant:
		//A constant of the function the lambda is in, which it doesn't capture: its value.  [basic.def.odr] 3
		return mpcUnit->EnclosingConstantType(cBound.mpcSymbol);
	case CPPBN_Constant:
		if (cBound.mpcSymbol->meKind == CPPSK_Enumerator)
		{
			return ConstantValue(pcId);
		}
		if (!mpcUnit->FindBinding(cBound.mpcSymbol))
		{
			Error(pcId, "A template parameter used outside an instantiation");
			return NULL;
		}
		pcParameter = (CCPPSyntaxParameter*)cBound.mpcSymbol->mpcDeclaration;
		psType = (pcParameter && (pcParameter->GetKind() == CPPS_Parameter)) ? mpcUnit->GetDeclarators()->GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter) : NULL;
		return (psType && psType->IsInteger()) ? psType : &mpcTypes->msInt;
	case CPPBN_Function:
		return FunctionAddress(pcId, cBound.mpcSymbol->mlliName, NULL);
	case CPPBN_FunctionTemplate:
		return FunctionTemplateAddress(pcId);
	case CPPBN_TemplateArgument:
		return Value(cBound.mpcArgument);
	case CPPBN_Variable:
		//SIZE::Max's value is the constant its class gives it: it needs no definition.  [class.static.data] 4
		if (mpcUnit->IsConstantMember(cBound.mpcVariable) && mpcUnit->Constant(pcId, &lliValue))
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
	return mpcTypes->Pointer(mpcUnit->FunctionType(pcFound));
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
	pcTemplate = mpcUnit->GetTemplate(pcId->mpcName->mpcSymbol);
	if (!pcTemplate || !pcComponent->mbTemplateArguments)
	{
		Error(pcId, "The address of a function template needs its template arguments in the compiler yet");
		return NULL;
	}
	pcFunction = mpcUnit->InstantiateFunction(pcTemplate, &pcComponent->macpTemplateArguments, NULL, pcId);
	if (!pcFunction)
	{
		return NULL;
	}
	return mpcTypes->Pointer(mpcUnit->FunctionType(pcFunction));
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
	mpcUnit->BeginQuiet();
	pcClass = mpcUnit->GetComponentClass(pcName->GetQualifier(), NULL, pcId);
	mpcUnit->EndQuiet();
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
		*ppsType = mpcTypes->MemberPointer(pcFoundClass, mpcUnit->FunctionType(pcFunction));
		return true;
	}

	psField = pcClass->FindField(lliName, &iOffset);
	if (!psField)
	{
		return false;
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

	pcClass = mpcUnit->GetContext()->HasThis() ? mpcUnit->ThisClass() : NULL;
	if (!pcClass)
	{
		Error(pcThis, mpcUnit->WritingLambda() ? "The lambda doesn't capture this" : "this is only in member functions");
		return NULL;
	}
	return mpcTypes->Pointer(mpcTypes->Qualified(pcClass->mpsType, mpcUnit->IsConstThis(), false));
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
		if (mpcUnit->IsObject(pcUnary->mpcOperand))
		{
			//*object: the object's operator.
			return OperatorExpression(CT_Multiply, pcUnary->mpcOperand, NULL, pcUnary);
		}
		return Load(Place(pcUnary), pcUnary);
	case CT_BitwiseAnd:
		pcOperand = mpcUnit->SkipParentheses(pcUnary->mpcOperand);
		mpcUnit->Changed(pcOperand, CPPPC_ADDRESS);
		if (mpcUnit->IsObject(pcOperand) && mpcUnit->HasMember(mpcUnit->ObjectClass(pcOperand), mpcUnit->GetOperatorName(CT_BitwiseAnd, false)))
		{
			return OperatorExpression(CT_BitwiseAnd, pcOperand, NULL, pcUnary);
		}
		if (pcOperand->GetKind() == CPPS_IdExpression)
		{
			pcId = (CCPPSyntaxIdExpression*)pcOperand;
			pcSymbol = pcId->mpcName->mpcSymbol;
			if (pcId->mpcName->IsQualifiedByClass() && MemberAddress(pcId, NULL, &psType))
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

	if (((pcUnary->meOperator == CT_Subtract) || (pcUnary->meOperator == CT_Add) || (pcUnary->meOperator == CT_BitwiseNot)) && mpcUnit->IsObject(pcUnary->mpcOperand))
	{
		return OperatorExpression(pcUnary->meOperator, pcUnary->mpcOperand, NULL, pcUnary);
	}
	if (((pcUnary->meOperator == CT_Subtract) || (pcUnary->meOperator == CT_BitwiseNot) || (pcUnary->meOperator == CT_Add)) && mpcUnit->Constant(pcUnary, &lliValue))
	{
		psType = mpcUnit->TypeOf(pcUnary->mpcOperand);
		if (psType && psType->IsInteger())
		{
			return mpcTypes->Promote(mpcUnit->OperandType(psType, pcUnary->mpcOperand));
		}
	}

	psType = Operand(pcUnary->mpcOperand);
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

	if (mpcUnit->IsObject(pcUnary->mpcOperand))
	{
		//++object, --object, and object++ and object-- with an int 0: the object's operator.  [over.inc] 1
		return OperatorExpression(pcUnary->meOperator, pcUnary->mpcOperand, pcUnary->mbPostfix ? mpcUnit->ImplicitZero() : NULL, pcUnary);
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

	switch (pcBinary->meOperator)
	{
	case CT_MemberByPointerFunctionPointerIndirection:
		if (mpcUnit->IsObject(pcBinary->mpcLeft))
		{
			return OperatorExpression(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, pcBinary);
		}
		return Load(Place(pcBinary), pcBinary);
	case CT_MemberFunctionPointerIndirection:
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
		if (mpcUnit->IsOverloadedComma(pcBinary))
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
	case CT_EqualTo:
	case CT_NotEqualTo:
	case CT_LessThan:
	case CT_LessThanEqualTo:
	case CT_GreaterThan:
	case CT_GreaterThanEqualTo:
		return ConditionValue(pcBinary);
	default:
		if (mpcUnit->IsObject(pcBinary->mpcLeft) || mpcUnit->IsObject(pcBinary->mpcRight))
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

	psResult = mpcUnit->TypeOf(pcBinary->mpcLeft);
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
		if (mpcUnit->SkipParentheses(pcBinary->mpcRight)->GetKind() == CPPS_InitializerList)
		{
			//x = {1, 2} is x = X{1, 2}: a temporary made from the list.  [expr.ass] 9
			return TemporaryObject(psPlace->Unqualified(), pcBinary->mpcRight, pcBinary) ? psPlace : NULL;
		}
		return CopyObject(psPlace->pcClass, pcBinary->mpcRight, pcBinary) ? psPlace : NULL;
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
// The type of the value of an operand that integral promotion is applied
// to (CCPPUnit::OperandType: a bit-field's is promoted by its width).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::Operand(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = Value(pcExpression);
	return psType ? mpcUnit->OperandType(psType, pcExpression) : NULL;
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
	return !psPointee->IsClass() || mpcUnit->CompleteClass(psPointee->pcClass, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether left and right can be compared: numbers, and pointers, or for equality
// pointers to members, that have a composite pointer type (with each other or 0).
// [expr.rel] 2, [expr.eq] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::Compare(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Type* psRight, CCPPSyntax* pcNode)
{
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
	if ((psLeft->IsPointer() || psLeft->IsMemberPointer()) && (psRight->IsPointer() || psRight->IsMemberPointer()) && !mpcUnit->CompositePointerType(psLeft, psRight))
	{
		//int* with long*, a pointer to a function with one to an object, pointers to members of unrelated classes.
		return Error(pcNode, "These pointers cannot be compared: neither's type converts to the other's");
	}
	if (!mpcUnit->CompositePointerType(psLeft, psRight))
	{
		return Error(pcNode, "These operands cannot be compared");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// c ? x : y.  Objects of a class make an object of it (ObjectConditional), scalars
// the type CCPPUnit::ConditionalType gives.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ConditionalExpression(CCPPSyntaxConditionalExpression* pcConditional)
{
	SCPP65Type*		psTrue;
	SCPP65Type*		psFalse;
	SCPP65Type*		psType;

	psTrue = mpcUnit->TypeOf(pcConditional->mpcTrue);
	psFalse = mpcUnit->TypeOf(pcConditional->mpcFalse);
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
	if (mpcUnit->IsObject(pcConditional->mpcTrue) || mpcUnit->IsObject(pcConditional->mpcFalse))
	{
		return ObjectConditional(pcConditional, psTrue, psFalse);
	}

	psType = mpcUnit->ConditionalType(psTrue, psFalse);
	if (!psType)
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
// c ? x : y as a place: x and y are lvalues of one type, which the result
// is, with the const and volatile of both.  NULL, with why, if they aren't.
// (One that picks between bit-fields isn't done.)  [expr.cond] 3, 4
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ConditionalPlace(CCPPSyntaxConditionalExpression* pcConditional)
{
	SCPP65Type*	psTrue;
	SCPP65Type*	psFalse;

	psTrue = mpcUnit->PlaceOf(pcConditional->mpcTrue);
	psFalse = mpcUnit->PlaceOf(pcConditional->mpcFalse);
	if (!psTrue || !psFalse || mpcUnit->IsRvalue(pcConditional->mpcTrue) || mpcUnit->IsRvalue(pcConditional->mpcFalse) || !psTrue->Unqualified()->Same(psFalse->Unqualified()))
	{
		Error(pcConditional, "This is not an lvalue: the two results of ? : are not lvalues of one type");
		return NULL;
	}
	if (mpcUnit->BitFieldOf(pcConditional->mpcTrue) || mpcUnit->BitFieldOf(pcConditional->mpcFalse))
	{
		Error(pcConditional, "c ? x : y that picks a bit-field to change is not supported by the compiler yet");
		return NULL;
	}
	if (!Condition(pcConditional->mpcCondition) || !Place(pcConditional->mpcTrue) || !Place(pcConditional->mpcFalse))
	{
		return NULL;
	}
	return mpcTypes->Qualified(psTrue->Unqualified(), psTrue->bConst || psFalse->bConst, psTrue->bVolatile || psFalse->bVolatile);
}


//////////////////////////////////////////////////////////////////////////
//
// c ? x : y with objects of the same class, or one derived from the other's
// (CCPPUnit::ConditionalClass): an object of the class made from the
// operand the condition picks.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ObjectConditional(CCPPSyntaxConditionalExpression* pcConditional, SCPP65Type* psTrue, SCPP65Type* psFalse)
{
	CCPPClass*		pcClass;
	SCPP65Type*		psType;

	pcClass = mpcUnit->ConditionalClass(psTrue, psFalse);
	if (!pcClass)
	{
		Error(pcConditional, "c ? x : y with objects of different types is not supported by the compiler yet");
		return NULL;
	}
	psTrue = psTrue->WithoutReference();
	psType = (psTrue->pcClass == pcClass) ? psTrue : psFalse->WithoutReference();
	if (!mpcUnit->CompleteClass(pcClass, pcConditional) ||
		!Condition(pcConditional->mpcCondition) ||
		!Construct(psType, NULL, pcConditional->mpcTrue, pcConditional->mpcTrue) ||
		!Construct(psType, NULL, pcConditional->mpcFalse, pcConditional->mpcFalse))
	{
		return NULL;
	}
	return psType;
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
	CCPPBoundConversion		cBound;

	pcExpression = mpcUnit->SkipParentheses(pcExpression);
	if (!pcExpression)
	{
		return false;
	}

	if (pcExpression->GetKind() == CPPS_UnaryExpression)
	{
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		if ((pcUnary->meOperator == CT_LogicalNot) && mpcUnit->IsObject(pcUnary->mpcOperand) && mpcUnit->HasMember(mpcUnit->ObjectClass(pcUnary->mpcOperand), mpcUnit->GetOperatorName(CT_LogicalNot, false)))
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
			if (mpcUnit->IsObject(pcBinary->mpcLeft) || mpcUnit->IsObject(pcBinary->mpcRight))
			{
				return Truth(OperatorExpression(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, pcBinary), pcBinary);
			}
			mpcUnit->ComparisonRange(pcBinary);
			return Operands(pcBinary->mpcLeft, pcBinary->mpcRight, &psLeft, &psRight) && Compare(pcBinary->meOperator, psLeft, psRight, pcBinary);
		default:
			break;
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
	case CPPBC_Pointer:
		psType = mpcUnit->CalleePointerType(cBound.mpcCallee);
		if (psType)
		{
			return CallThroughPointer(pcCall, cBound.mpapcArguments, psType->psBase);
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
// the call isn't evaluated.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::Call(CCPPFunction* pcFunction, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	if (pcFunction->mpcTemplate && !pcFunction->mpcDefinition && pcFunction->mpcTemplate->mbMemberTemplate && !mpcUnit->IsUnevaluated())
	{
		//A member template only declared can be named in sizeof and decltype, not called.  [basic.def.odr] 4
		Error(pcNode, "The member template has not been defined");
		return NULL;
	}
	if (!Arguments(&pcFunction->mapsParameters, &pcFunction->mapcDefaults, pcFunction->mbVariadic, papcArguments, pcNode, pcFunction->mpsReturn))
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
		psType = Operand(pcArgument);
		if (!psType)
		{
			return false;
		}
		psParameter = mpcUnit->VariadicArgumentType(psType);
		if (!psParameter)
		{
			return Error(pcArgument, psType->IsClass() ? "Objects cannot be passed by value by the compiler yet; pass a pointer or a reference" : "This variable argument is not supported by the compiler yet");
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
		if (psParameter->IsReference() && mpcUnit->BindsToTemporary(psParameter, pcArgument))
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
				mpcUnit->Changed(pcArgument, CPPPC_REFERENCE);
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

	return !(psReturn && psReturn->IsClass()) || mpcUnit->CompleteClass(psReturn->pcClass, pcNode);
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

	if (mpcUnit->FloatBuiltin(pcCall, &dValue, &psType))
	{
		return psType;
	}

	psBuiltin = mpcUnit->FindBuiltin(lliName);
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
		return Convert(Value(papcArguments->GetPtr(0)), mpcUnit->BuiltinOperandType(psBuiltin), pcCall) ? &mpcTypes->msInt : NULL;
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
		mpcUnit->Changed(pcObject, CPPPC_MEMBER_CALL);
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

	psType = mpcUnit->TypeOf(pcObject);
	if (!psType || !psType->IsClass())
	{
		return Value(pcObject);
	}
	psType = OperatorCall(pcObject, CT_MemberByPointer, &mapcNone, pcNode);
	for (i = 0; psType && psType->IsClass() && (i < CPP_MAX_ARROWS); i++)
	{
		//An operator-> that returns an object: its operator-> is applied, until one returns a pointer.  [over.ref] 1
		pcFunction = mpcUnit->CompleteClass(psType->pcClass, pcNode) ? mpcUnit->GetBinder()->FindArrowOperator(psType->pcClass, NULL, pcNode) : NULL;
		psType = pcFunction ? Call(pcFunction, NULL, pcNode) : NULL;
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

	uiErrors = mpcUnit->GetErrors()->NumErrors();
	if (!mpcUnit->GetBinder()->BindMember(psType->pcClass, pcMember->mpcName, pcMember, &cBound))
	{
		if (mpcUnit->GetErrors()->NumErrors() == uiErrors)
		{
			Error(pcMember, "The class has no member of that name");
		}
		return NULL;
	}
	mpcUnit->Resolved(pcMember->mpcName, cBound.mpcSymbol);
	return (cBound.meWhat == CPPBN_Field) ? FieldType(cBound.mpsField, psType->bConst, psType->bVolatile) : cBound.mpcVariable->mpsType;
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
	CCPPClass*	pcClass;

	psMember = mpcUnit->TypeOf(pcBinary->mpcRight);
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
		pcClass = psObject->psBase->pcClass;
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
		pcClass = psObject->pcClass;
	}
	if (psMember->pcClass && (pcClass != psMember->pcClass))
	{
		//A base further on in the object is where the member is.
		mpcUnit->CompleteClass(pcClass, pcBinary);
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
SCPP65Type* CCPP65TypeChecker::MemberPointerCall(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, CCPPSyntaxBinaryExpression* pcBinary)
{
	SCPP65Type*	psMember;
	SCPP65Type*	psFunctionType;

	psMember = mpcUnit->TypeOf(pcBinary->mpcRight);
	if (!psMember || !psMember->IsMemberPointer() || !psMember->psBase->IsFunction())
	{
		Error(pcBinary, "Only a pointer to a member function can be called");
		return NULL;
	}
	psFunctionType = psMember->psBase;
	if (!Arguments(psFunctionType->papsParameters, NULL, psFunctionType->bVariadic, papcArguments, pcCall, psFunctionType->psBase) ||
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
	SCPP65Type*	psType;

	psType = PlaceType(pcExpression);
	mpcUnit->PlaceTyped(pcExpression, psType);
	mpcUnit->PlaceTyped(mpcUnit->SkipParentheses(pcExpression), psType);
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
			mpcUnit->ReportOnce(pcId, "The name is ambiguous: it finds more than one thing");
			return NULL;
		}
		mpcUnit->GetBinder()->BindName(pcId, &cBound);
		if (cBound.meWhat == CPPBN_Variable)
		{
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
				//Its object is wanted (its address, or a reference to it): that odr-uses it.  [basic.def.odr] 3
				Error(pcExpression, "The lambda doesn't capture the variable");
				return NULL;
			}
			Error(pcExpression, "Only variables are supported by the compiler here yet");
			return NULL;
		}
		if (!mpcUnit->GetContext()->HasThis())
		{
			Error(pcExpression, "A field needs an object");
			return NULL;
		}
		return FieldType(cBound.mpsField, mpcUnit->IsConstThis(), false);

	case CPPS_MemberExpression:
		return MemberPlace((CCPPSyntaxMemberExpression*)pcExpression);

	case CPPS_UnaryExpression:
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		if (((pcUnary->meOperator == CT_Increment) || (pcUnary->meOperator == CT_Decrement)) && !pcUnary->mbPostfix && !mpcUnit->IsObject(pcUnary->mpcOperand))
		{
			//++x and --x are x, once it is stepped.  [expr.pre.incr] 1
			return IncrementExpression(pcUnary) ? Place(pcUnary->mpcOperand) : NULL;
		}
		if (pcUnary->meOperator != CT_Multiply)
		{
			break;
		}
		if (mpcUnit->IsObject(pcUnary->mpcOperand))
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
			return mpcUnit->StringArrayType(pcExpression);
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
		if (mpcUnit->IsAssignmentOperator(pcBinary->meOperator) && !mpcUnit->IsObject(pcLeft))
		{
			//(x = y) and (x op= y) are x, once it is assigned.  [expr.ass] 1
			return Assignment(pcBinary) ? Place(pcLeft) : NULL;
		}
		if ((pcBinary->meOperator == CT_Comma) && !mpcUnit->IsOverloadedComma(pcBinary))
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
		if (!mpcUnit->IsObject(pcExpression))
		{
			return ConditionalPlace((CCPPSyntaxConditionalExpression*)pcExpression);
		}
		break;

	default:
		break;
	}
	if (mpcUnit->IsObject(pcExpression))
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

	psType = mpcUnit->TypeOf(pcSubscript->mpcArray);
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
		return psTo;
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
	if (!psTo->IsScalar())
	{
		Error(pcNode, "Only casts to integers and pointers are supported by the compiler yet");
		return NULL;
	}
	return Convert(psFrom, psTo, pcNode) ? psTo : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Type(arguments) or Type{arguments}: a temporary of a class, or of any other
// type a cast of one value, or zero for none.  [expr.type.conv]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::ConstructExpression(CCPPSyntaxConstructExpression* pcConstruct)
{
	SCPP65Type*		psType;
	CArraySyntax*	papcElements;

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
		return CastExpression(pcConstruct->mpcType, papcElements->GetPtr(0), pcConstruct);
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

	if (mpcUnit->GetSemanticPass())
	{
		//Only checking: the value isn't needed, only the type (__builtin_va_arg(ap, T) is a T).
		return CheckedSizeofType(pcSizeof);
	}
	if (!mpcUnit->Constant(pcSizeof, &lliBytes))
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

	if (pcNew->macpArraySizes.NumElements() > 0)
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

	//The bound is an integer, or an object that converts to one.  [expr.new] 6
	psElement = mpcUnit->GetBinder()->ContextualType(pcNew->macpArraySizes.GetPtr(0), false);
	if (psElement && !psElement->IsInteger())
	{
		Error(pcNew->macpArraySizes.GetPtr(0), "A new array's bound must be an integer");
		return NULL;
	}
	if (!psElement || !Convert(ValueFor(pcNew->macpArraySizes.GetPtr(0), psElement, false), &mpcTypes->msUnsignedInt, pcNew))
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
// delete pointer and delete[] pointer: void.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65TypeChecker::DeleteExpression(CCPPSyntaxDeleteExpression* pcDelete)
{
	SCPP65Type*		psPointer;
	SCPP65Type*		psType;

	//A pointer, or an object that converts to one.  [expr.delete] 1
	psPointer = mpcUnit->GetBinder()->ContextualType(pcDelete->mpcOperand, true);
	psPointer = psPointer ? ValueFor(pcDelete->mpcOperand, psPointer, false) : NULL;
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
	if (psType->IsClass() && !mpcUnit->CompleteClass(psType->pcClass, pcDelete))
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

	psFrom = mpcUnit->ValueTypeOf(pcSource);
	if (!psFrom || psFrom->pcEnum || psTo->pcEnum || (psTo->eKind == CPP65TK_Bool) || !(psFrom->IsInteger() || psFrom->IsFloat()) || !(psTo->IsInteger() || psTo->IsFloat()))
	{
		return false;
	}

	mpcUnit->BeginQuiet();
	if (psFrom->IsFloat())
	{
		bConstant = mpcUnit->FloatConstant(pcSource, &dValue);
		mpcUnit->EndQuiet();
		return psTo->IsInteger() || ((psTo->iBytes < psFrom->iBytes) && !bConstant);
	}
	bConstant = mpcUnit->Constant(pcSource, &lliValue);
	mpcUnit->EndQuiet();
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
// [dcl.init.list] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::NotNarrowed(CCPPSyntax* pcSource, SCPP65Type* psTo)
{
	if (Narrows(pcSource, psTo))
	{
		return Error(pcSource, "The conversion could lose some of the value, which an initialiser in braces may not");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a psType can be made from pcInitialiser (Construct), asked by
// overload resolution: what is wrong isn't an error.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::CanInitialise(SCPP65Type* psType, CCPPSyntax* pcInitialiser)
{
	size	uiErrors;
	bool	bResult;

	uiErrors = mpcUnit->GetErrors()->NumErrors();
	mpcUnit->BeginTrial();
	bResult = Construct(psType->Unqualified(), NULL, pcInitialiser, pcInitialiser);
	mpcUnit->EndTrial();
	mpcUnit->GetErrors()->Truncate(uiErrors);
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
	SCPP65Type*						psElement;
	int								i;
	bool							bResult;
	bool							bBraced;

	apcArguments.Init();
	if (!papcArguments)
	{
		papcArguments = &apcArguments;
	}
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
		psElement = mpcUnit->InitializerListElement(psType);
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
	if (!pcInitialiser && (papcArguments->NumElements() == 1) && !pcClass->HasCopyConstructor())
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
			bResult = mpcUnit->IsMadeObject(pcInitialiser, pcClass) ? (Value(pcInitialiser) != NULL) : CopyConstruct(pcClass, pcInitialiser, pcNode);
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
	bResult = ConstructClass(pcClass, papcArguments, pcNode);
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

	if (!mpcUnit->InitializerListLayout(psType->pcClass, &bEnd))
	{
		return Error(pcNode, "This std::initializer_list isn't laid out as the compiler fills one: a pointer to its elements, then their count or end");
	}
	if (psElement->IsClass() && !mpcUnit->CompleteClass(psElement->pcClass, pcNode))
	{
		return false;
	}
	if (psElement->IsClass() && psElement->pcClass->NeedsDestructor())
	{
		return Error(pcNode, "A std::initializer_list of objects that are destroyed is not supported by the compiler yet");
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
	size					i;
	size					uiArgument;
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

	bResult = true;
	uiArgument = 0;
	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		if (pcClass->macFields.Get(i)->IsUnnamedBitField())
		{
			continue;
		}
		bResult &= Construct(pcClass->macFields.Get(i)->psType, NULL, (uiArgument < papcArguments->NumElements()) ? papcArguments->GetPtr(uiArgument) : pcClass->macFields.Get(i)->pcInitialiser, pcNode);
		uiArgument++;
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

	if (pcClass->HasCopyConstructor())
	{
		apcArguments.Init();
		apcArguments.Add(pcSource);
		bResult = ConstructClass(pcClass, &apcArguments, pcNode);
		apcArguments.Kill();
		return bResult;
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
// object of pcClass or a class derived from it.
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

	psType = mpcUnit->ValueTypeOf(pcExpression);
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
		papcElements = mpcUnit->ListElements(pcInitialiser);
	}
	else if (pcInitialiser->GetKind() == CPPS_InitializerList)
	{
		papcElements = mpcUnit->ListElements(pcInitialiser);
		bBraced = true;
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
			bResult &= Initialise(psType->IsArray() ? psType->psBase : psType, papcElements->GetPtr(i), pcNode, bBraced);
		}
		return bResult;
	}

	if (psType->IsReference())
	{
		if (!BindsCategory(psType, pcInitialiser, pcNode))
		{
			return false;
		}
		if (mpcUnit->BindsToTemporary(psType, pcInitialiser))
		{
			//It refers to a temporary holding the value.
			return ImplicitConvert(ValueFor(pcInitialiser, psType->psBase, false), psType->psBase, pcNode, pcInitialiser);
		}
		mpcUnit->BeginQuiet();
		psValue = Place(pcInitialiser);
		mpcUnit->EndQuiet();
		if (psValue && !psType->psBase->bConst)
		{
			mpcUnit->Changed(pcInitialiser, CPPPC_REFERENCE);
		}
		return psValue ? CanBind(psType, psValue, pcNode) : (Value(pcInitialiser) != NULL);
	}
	return ImplicitConvert(ValueFor(pcInitialiser, psType, false), psType, pcNode, pcInitialiser) && (!bBraced || NotNarrowed(pcInitialiser, psType));
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
	if (psFrom->IsMemberPointer() && psTo->IsMemberPointer() && psFrom->pcClass && psTo->pcClass && (psFrom->pcClass != psTo->pcClass) && mpcUnit->CompleteClass(psTo->pcClass, pcNode) && mpcUnit->IsDerived(psTo->pcClass, psFrom->pcClass, &iOffset) && (iOffset != 0) && psFrom->psBase->IsFunction())
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
// volatile (KeepsPointerQualifiers), an enum class's value stays one
// (KeepsScopedEnum), and std::nullptr_t converts only as it may
// (KeepsNullPointer).  pcValue is the expression converted, when there is
// one to look at.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::ImplicitConvert(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode, CCPPSyntax* pcValue)
{
	return Convert(psFrom, psTo, pcNode) && KeepsPointerQualifiers(psFrom, psTo, pcNode) && KeepsScopedEnum(psFrom, psTo, pcNode) && KeepsNullPointer(psFrom, psTo, pcNode, pcValue);
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
// Whether psFrom converts to psTo with no cast as far as an enum class
// goes: its value is only its own type's, and only its own values are its.
// [conv.prom] and [conv.integral] leave a scoped enum out.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::KeepsScopedEnum(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode)
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
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pointer psFrom converts to psTo (any other types do here) without
// losing const or volatile from what it points to.  A string literal's
// conversion to a char* is let through, with a warning if they are on.
// [conv.qual]
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::KeepsPointerQualifiers(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode)
{
	bool	bKept;

	if (!psFrom->IsPointer() || !psTo->IsPointer())
	{
		return true;
	}
	bKept = psFrom->psBase->Equals(psTo->psBase) ? mpcUnit->GetBinder()->KeepsQualifiers(psTo, psFrom) : psTo->psBase->HasQualifiersOf(psFrom->psBase);
	if (!bKept && psFrom->psBase->Equals(psTo->psBase) && mpcUnit->IsStringLiteral(mpcUnit->SkipParentheses(pcNode)))
	{
		//char* p = "abc": a string literal's characters are const, and the conversion that let that be forgotten went
		//with C++11.  It is still made, as code written before is full of it, and is warned about when that is asked
		//for (CCPPCompileOptions::mbWarnStringToPointer).  [diff.cpp03.lex]
		if (mpcUnit->GetOptions()->mbWarnStringToPointer)
		{
			mpcUnit->Warning(pcNode, "A string literal is const: this conversion to a pointer its characters can be written through is not C++14's");
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
// Whether pcValue is the kind of expression a reference of type
// psReference binds to: T&& only to what has no name or is about to go.
// [dcl.init.ref] 5.2
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::BindsCategory(SCPP65Type* psReference, CCPPSyntax* pcValue, CCPPSyntax* pcNode)
{
	return !psReference->bRvalue || mpcUnit->IsRvalue(pcValue) || Error(pcNode, "A reference declared && cannot be bound to something with a name");
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
	return psReference->psBase->HasQualifiersOf(psPlace) || Error(pcNode, "A const object cannot be bound to a reference that is not const");
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

	psValue = mpcUnit->ValueTypeOf(pcValue);
	if (!psValue)
	{
		return true;
	}
	if (psTo->IsReference())
	{
		return BindsCategory(psTo, pcValue, pcValue) && (mpcUnit->BindsToTemporary(psTo, pcValue) || CanBind(psTo, psValue, pcValue));
	}
	return KeepsPointerQualifiers(psValue->IsArray() ? mpcTypes->Pointer(psValue->psBase) : psValue, psTo, pcValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65TypeChecker::Error(CCPPSyntax* pcNode, const char* szMessage)
{
	return mpcUnit->Error(pcNode, szMessage);
}
