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
#include <string.h>
#include "CTInt.h"
#include "CTShort.h"
#include "CTLongLong.h"
#include "CTChar.h"
#include "CTBoolean.h"
#include "CTString.h"
#include "CTIdentifier.h"
#include "CppTokeniser.h"
#include "CPPSyntaxTree.h"
#include "CPP65Generator.h"


//////////////////////////////////////////////////////////////////////////
//
// Generates code that leaves the expression's value in A and returns its type, or
// NULL after an error.  Arrays become pointers to their first element.  Registers A,
// B, W, X and Y may change; S is the same afterwards.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::Expression(CCPPSyntax* pcExpression)
{
	SCPP65Type*		psType;
	SCPP65Place		sPlace;

	psType = Evaluate(pcExpression);
	if (psType && psType->IsReference())
	{
		//A reference's value is what it refers to.
		ResultPlace(psType, &sPlace, pcExpression);
		if (sPlace.psType->IsClass())
		{
			return sPlace.psType;
		}
		if (!Supported(sPlace.psType, pcExpression))
		{
			return NULL;
		}
		Load(&sPlace, "A", "A");
		return sPlace.psType;
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// Expression, but a function returning a reference leaves its address in A and
// the reference type.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::Evaluate(CCPPSyntax* pcExpression)
{
	CCPPSyntaxNamedCastExpression*	pcNamedCast;
	CCPPSyntaxConstructExpression*	pcConstruct;
	CArraySyntax*					papcElements;

	if (!pcExpression)
	{
		return NULL;
	}

	switch (pcExpression->GetKind())
	{
	case CPPS_LiteralExpression:
		return SimpleValue(pcExpression, "A");
	case CPPS_StringExpression:
		return StringLiteral((CCPPSyntaxStringExpression*)pcExpression);
	case CPPS_IdExpression:
		return IdExpression((CCPPSyntaxIdExpression*)pcExpression);
	case CPPS_ParenthesisedExpression:
		return Evaluate(((CCPPSyntaxParenthesisedExpression*)pcExpression)->mpcExpression);
	case CPPS_UnaryExpression:
		return UnaryExpression((CCPPSyntaxUnaryExpression*)pcExpression);
	case CPPS_BinaryExpression:
		return BinaryExpression((CCPPSyntaxBinaryExpression*)pcExpression);
	case CPPS_ConditionalExpression:
		return ConditionalExpression((CCPPSyntaxConditionalExpression*)pcExpression);
	case CPPS_CallExpression:
		return CallExpression((CCPPSyntaxCallExpression*)pcExpression);
	case CPPS_SubscriptExpression:
	case CPPS_MemberExpression:
		return Value(pcExpression);
	case CPPS_ThisExpression:
		return ThisExpression(pcExpression);
	case CPPS_CastExpression:
		return CastExpression(((CCPPSyntaxCastExpression*)pcExpression)->mpcType, ((CCPPSyntaxCastExpression*)pcExpression)->mpcOperand, pcExpression);
	case CPPS_NamedCastExpression:
		pcNamedCast = (CCPPSyntaxNamedCastExpression*)pcExpression;
		if (pcNamedCast->meCast == CT_dynamic_cast)
		{
			Error(pcExpression, "dynamic_cast is not supported by the 65C64 generator");
			return NULL;
		}
		return CastExpression(pcNamedCast->mpcType, pcNamedCast->mpcOperand, pcExpression);
	case CPPS_ConstructExpression:
		pcConstruct = (CCPPSyntaxConstructExpression*)pcExpression;
		if (pcConstruct->mpcInitializer && (pcConstruct->mpcInitializer->GetKind() == CPPS_ParenthesisedInitializer))
		{
			papcElements = &((CCPPSyntaxParenthesisedInitializer*)pcConstruct->mpcInitializer)->macpElements;
			if (papcElements->NumElements() == 1)
			{
				return CastExpression(pcConstruct->mpcType, papcElements->GetPtr(0), pcExpression);
			}
		}
		Error(pcExpression, "This construction is not supported by the 65C64 generator yet");
		return NULL;
	case CPPS_SizeofExpression:
	case CPPS_TypeTraitExpression:
		return SizeofExpression(pcExpression);
	case CPPS_NewExpression:
		return NewExpression((CCPPSyntaxNewExpression*)pcExpression);
	default:
		Error(pcExpression, "This expression is not supported by the 65C64 generator yet");
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Loads the value of an lvalue into A.  An array gives the address of its first
// element and an object its address.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::Value(CCPPSyntax* pcExpression)
{
	SCPP65Place		sPlace;

	if (!Place(pcExpression, &sPlace))
	{
		return NULL;
	}
	if (sPlace.psType->IsArray())
	{
		LoadAddress(&sPlace);
		return mcTypes.Pointer(sPlace.psType->psBase);
	}
	if (sPlace.psType->IsClass())
	{
		//An object's value is its address, so its type is known; using it as a number fails in Convert.
		LoadAddress(&sPlace);
		return sPlace.psType;
	}
	if (sPlace.psType->IsFunction())
	{
		LoadAddress(&sPlace);
		return mcTypes.Pointer(sPlace.psType);
	}
	if (!sPlace.psType->IsScalar())
	{
		Error(pcExpression, "This type of value is not supported by the 65C64 generator yet");
		return NULL;
	}
	if (!Supported(sPlace.psType, pcExpression))
	{
		return NULL;
	}
	Load(&sPlace, "A", "A");
	return sPlace.psType;
}


//////////////////////////////////////////////////////////////////////////
//
// Numbers, characters, true, false and nullptr.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::Literal(CCPPSyntaxLiteralExpression* pcLiteral)
{
	CCPPToken*		pcToken;

	pcToken = mpcTokeniser->Get(pcLiteral->miLiteral);
	if (pcToken->IsNumber())
	{
		switch (((CCTNumber*)pcToken)->meType)
		{
		case CTNT_Int:
			return ((CCTInt*)pcToken)->IsSigned() ? &mcTypes.msInt : &mcTypes.msUnsignedInt;
		case CTNT_Short:
			return &mcTypes.msInt;
		case CTNT_LongLong:
			return ((CCTLongLong*)pcToken)->IsSigned() ? &mcTypes.msLongLong : &mcTypes.msUnsignedLongLong;
		default:
			Error(pcLiteral, "Floating point is not supported by the 65C64 generator yet");
			return NULL;
		}
	}
	if (pcToken->IsChar())
	{
		return &mcTypes.msChar;
	}
	if (pcToken->IsBool())
	{
		return &mcTypes.msBool;
	}
	return mcTypes.Pointer(&mcTypes.msVoid);
}


//////////////////////////////////////////////////////////////////////////
//
// Loads a simple value (see IsSimple) into szRegister.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::SimpleValue(CCPPSyntax* pcExpression, const char* szRegister)
{
	SCPP65Type*		psType;
	SCPP65Place		sPlace;
	int64			lliValue;
	CCPPSymbol*		pcSymbol;

	pcExpression = SkipParentheses(pcExpression);
	if (pcExpression->GetKind() == CPPS_IdExpression)
	{
		pcSymbol = ((CCPPSyntaxIdExpression*)pcExpression)->mpcName->mpcSymbol;
		if (pcSymbol->meKind != CPPSK_Enumerator)
		{
			if (!Place(pcExpression, &sPlace))
			{
				return NULL;
			}
			Load(&sPlace, szRegister, NULL);
			return sPlace.psType;
		}
		psType = &mcTypes.msInt;
	}
	else
	{
		psType = Literal((CCPPSyntaxLiteralExpression*)pcExpression);
		if (!psType)
		{
			return NULL;
		}
	}

	if (!Constant(pcExpression, &lliValue))
	{
		Error(pcExpression, "The value is not a constant");
		return NULL;
	}
	LoadConstant(szRegister, psType, lliValue);
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// String literals are data.  A gets the address of the first character.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::StringLiteral(CCPPSyntaxStringExpression* pcString)
{
	CArrayUint8		auiBytes;

	auiBytes.Init();
	if (!DecodeString(pcString, &auiBytes))
	{
		auiBytes.Kill();
		return NULL;
	}

	StringData((char*)auiBytes.GetData(), auiBytes.NumElements());
	auiBytes.Kill();
	return mcTypes.Pointer(&mcTypes.msChar);
}


//////////////////////////////////////////////////////////////////////////
//
// Adds uiLength bytes and a zero as string data.  A gets its address.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::StringData(char* szBytes, size uiLength)
{
	CChars*	pszBytes;
	size	i;

	pszBytes = maszStrings.Add();
	pszBytes->Init();
	for (i = 0; i < uiLength; i++)
	{
		pszBytes->Append((int)(uint8)szBytes[i]);
		pszBytes->Append(", ");
	}
	pszBytes->Append('0');

	Line("cpy A, ")->Append(&mszUnit)->Append("Str")->Append((int)maszStrings.NumElements() - 1)->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// __func__ and __FUNCTION__ are the function's name.  __PRETTY_FUNCTION__ and
// __FUNCSIG__ are its class and name ("CClass::Name"), not the whole signature.
// Returns false if lliName is not one of them.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::FunctionNameText(int64 lliName, CChars* pszText)
{
	char*	szName;
	int		iLength;
	bool	bPretty;

	if (lliName < 0)
	{
		return false;
	}
	szName = mpcTokeniser->GetIdentifier(lliName, &iLength);
	if (!szName)
	{
		return false;
	}
	bPretty = ((iLength == 19) && (memcmp(szName, "__PRETTY_FUNCTION__", 19) == 0)) || ((iLength == 10) && (memcmp(szName, "__FUNCSIG__", 10) == 0));
	if (!bPretty && !((iLength == 8) && (memcmp(szName, "__func__", 8) == 0)) && !((iLength == 12) && (memcmp(szName, "__FUNCTION__", 12) == 0)))
	{
		return false;
	}

	if (mpsFunction)
	{
		if (bPretty && mpsFunction->psClass)
		{
			szName = mpcTokeniser->GetIdentifier(mpsFunction->psClass->lliName, &iLength);
			pszText->Append(szName, iLength);
			pszText->Append("::");
		}
		if (mpsFunction->lliName >= 0)
		{
			szName = mpcTokeniser->GetIdentifier(mpsFunction->lliName, &iLength);
			pszText->Append(szName, iLength);
		}
		else
		{
			pszText->Append("operator");
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::IdExpression(CCPPSyntaxIdExpression* pcId)
{
	CCPPSymbol*		pcSymbol;
	CChars			szName;

	pcSymbol = pcId->mpcName->mpcSymbol;
	if (IsFunctionName(pcId))
	{
		szName.Init();
		FunctionNameText(GetComponentName(pcId->mpcName->GetLast()), &szName);
		StringData(szName.Text(), szName.Length());
		szName.Kill();
		return mcTypes.Pointer(&mcTypes.msChar);
	}
	if (pcSymbol && (pcSymbol->meKind == CPPSK_Enumerator))
	{
		return SimpleValue(pcId, "A");
	}
	if (pcSymbol && (pcSymbol->meKind == CPPSK_Function) && (pcId->mpcName->macpComponents.NumElements() == 1))
	{
		return FunctionAddress(pcId, pcSymbol->mlliName);
	}
	return Value(pcId);
}


//////////////////////////////////////////////////////////////////////////
//
// True for __func__ and the other names of the enclosing function, which the parser
// declares as globals.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsFunctionName(CCPPSyntaxIdExpression* pcId)
{
	CCPPSymbol*	pcSymbol;
	CChars		szName;
	bool		bName;

	pcSymbol = pcId->mpcName->mpcSymbol;
	if ((pcId->mpcName->macpComponents.NumElements() != 1) || (pcSymbol && ((pcSymbol->meKind != CPPSK_Variable) || GetVariable(pcSymbol))))
	{
		return false;
	}
	szName.Init();
	bName = FunctionNameText(GetComponentName(pcId->mpcName->GetLast()), &szName);
	szName.Kill();
	return bName;
}


//////////////////////////////////////////////////////////////////////////
//
// A function's name used as a value is its address.  It must not be overloaded.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::FunctionAddress(CCPPSyntax* pcNode, int64 lliName)
{
	SCPP65Function*		psFunction;
	SCPP65Function*		psFound;
	size				i;

	psFound = NULL;
	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		if (!psFunction->psClass && (psFunction->lliName == lliName) && !psFunction->psTemplate)
		{
			if (psFound)
			{
				Error(pcNode, "The address of an overloaded function is not supported by the 65C64 generator yet");
				return NULL;
			}
			psFound = psFunction;
		}
	}
	if (!psFound)
	{
		Error(pcNode, "The function has not been declared");
		return NULL;
	}

	RequireFunction(psFound);
	Line("cpy A, ");
	AppendFunctionLabel(mpszOut, psFound);
	mpszOut->AppendNewLine();
	return mcTypes.Pointer(mcTypes.Function(psFound->psReturn, &psFound->mapsParameters, psFound->bVariadic));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::UnaryExpression(CCPPSyntaxUnaryExpression* pcUnary)
{
	SCPP65Type*		psType;
	SCPP65Place		sPlace;
	CCPPSyntax*		pcOperand;
	int64			lliValue;

	switch (pcUnary->meOperator)
	{
	case CT_Increment:
	case CT_Decrement:
		return IncrementExpression(pcUnary);
	case CT_LogicalNot:
		return ConditionValue(pcUnary);
	case CT_Multiply:
		return Value(pcUnary);
	case CT_BitwiseAnd:
		pcOperand = SkipParentheses(pcUnary->mpcOperand);
		if ((pcOperand->GetKind() == CPPS_IdExpression) && ((CCPPSyntaxIdExpression*)pcOperand)->mpcName->mpcSymbol && (((CCPPSyntaxIdExpression*)pcOperand)->mpcName->mpcSymbol->meKind == CPPSK_Function))
		{
			return FunctionAddress(pcOperand, ((CCPPSyntaxIdExpression*)pcOperand)->mpcName->mpcSymbol->mlliName);
		}
		if (!Place(pcUnary->mpcOperand, &sPlace))
		{
			return NULL;
		}
		LoadAddress(&sPlace);
		return mcTypes.Pointer(sPlace.psType);
	default:
		break;
	}

	//-1 and ~0 are constants.
	if (((pcUnary->meOperator == CT_Subtract) || (pcUnary->meOperator == CT_BitwiseNot) || (pcUnary->meOperator == CT_Add)) && Constant(pcUnary, &lliValue))
	{
		psType = TypeOf(pcUnary->mpcOperand);
		if (psType && psType->IsInteger())
		{
			psType = mcTypes.Promote(psType);
			LoadConstant("A", psType, lliValue);
			return psType;
		}
	}

	psType = Expression(pcUnary->mpcOperand);
	if (!psType)
	{
		return NULL;
	}
	if (!psType->IsInteger())
	{
		Error(pcUnary, "The operand must be an integer");
		return NULL;
	}
	psType = mcTypes.Promote(psType);
	switch (pcUnary->meOperator)
	{
	case CT_Add:
		return psType;
	case CT_Subtract:
		Line(psType->IsWide() ? "wth A, 8\n\t\tneg A, A\n\t\twth A, 4" : "neg A, A")->AppendNewLine();
		return psType;
	case CT_BitwiseNot:
		Line(psType->IsWide() ? "wth A, 8\n\t\txor A, -1\n\t\twth A, 4" : "xor A, -1")->AppendNewLine();
		return psType;
	default:
		Error(pcUnary, "This operator is not supported by the 65C64 generator yet");
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// ++x and --x give the new value, x++ and x-- the old one.  Pointers step by the size
// of what they point to.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::IncrementExpression(CCPPSyntaxUnaryExpression* pcUnary)
{
	SCPP65Place		sPlace;
	const char*		szAddress;
	const char*		szOperation;
	int				iStep;

	if (!Place(pcUnary->mpcOperand, &sPlace))
	{
		return NULL;
	}
	if (!sPlace.psType->IsScalar())
	{
		Error(pcUnary, "Only integers and pointers can be incremented");
		return NULL;
	}

	szAddress = NULL;
	if (sPlace.eKind == CPP65PK_Address)
	{
		Line("cpy X, A")->AppendNewLine();
		ChangeX();
		szAddress = "X";
	}
	iStep = sPlace.psType->IsPointer() ? sPlace.psType->GetElementBytes() : 1;
	szOperation = (pcUnary->meOperator == CT_Increment) ? "add " : "sub ";

	Load(&sPlace, "A", szAddress);
	if (pcUnary->mbPostfix)
	{
		MoveAToB(sPlace.psType);
		if (sPlace.psType->IsWide())
		{
			Line("wth B, 8")->AppendNewLine();
		}
		Line(szOperation)->Append("B, ")->Append(iStep)->AppendNewLine();
		if (sPlace.psType->IsWide())
		{
			Line("wth B, 4")->AppendNewLine();
		}
		Store(&sPlace, "B", szAddress);
	}
	else
	{
		if (sPlace.psType->IsWide())
		{
			Line("wth A, 8")->AppendNewLine();
		}
		Line(szOperation)->Append("A, ")->Append(iStep)->AppendNewLine();
		if (sPlace.psType->IsWide())
		{
			Line("wth A, 4")->AppendNewLine();
		}
		if (sPlace.psType->iBytes < 4)
		{
			Truncate("A", sPlace.psType->iBytes, sPlace.psType->bSigned);
		}
		Store(&sPlace, "A", szAddress);
	}
	return sPlace.psType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::BinaryExpression(CCPPSyntaxBinaryExpression* pcBinary)
{
	SCPP65Type*		psLeft;
	SCPP65Right		sRight;

	switch (pcBinary->meOperator)
	{
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
		if (!Expression(pcBinary->mpcLeft))
		{
			return NULL;
		}
		return Expression(pcBinary->mpcRight);
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
		psLeft = Operands(pcBinary->mpcLeft, pcBinary->mpcRight, &sRight);
		if (!psLeft)
		{
			return NULL;
		}
		return Operate(pcBinary->meOperator, psLeft, &sRight, pcBinary);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// x = y and x op= y.  The value is the one stored.  An address on the stack is
// read again after the operation because a signed divide changes X.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::Assignment(CCPPSyntaxBinaryExpression* pcBinary)
{
	SCPP65Place		sPlace;
	SCPP65Type*		psRight;
	SCPP65Right		sRight;
	SCPP65Type*		psResult;
	ECppTokens		eOperator;
	CArraySyntax	apcRight;
	bool			bAddress;

	//object op= value calls the object's operator if it has one.
	psResult = TypeOf(pcBinary->mpcLeft);
	if (psResult && psResult->IsClass() && HasMember(psResult->psClass, GetOperatorName(pcBinary->meOperator, false)))
	{
		apcRight.Init();
		apcRight.Add(pcBinary->mpcRight);
		psResult = OperatorCall(pcBinary->mpcLeft, pcBinary->meOperator, &apcRight, pcBinary);
		apcRight.Kill();
		return psResult;
	}

	if (!Place(pcBinary->mpcLeft, &sPlace))
	{
		return NULL;
	}
	if (sPlace.psType->IsClass() && (pcBinary->meOperator == CT_Assignment))
	{
		return CopyObject(&sPlace, pcBinary->mpcRight, pcBinary) ? sPlace.psType : NULL;
	}
	if (!sPlace.psType->IsScalar())
	{
		Error(pcBinary, "Only integers, pointers and objects can be assigned by the 65C64 generator yet");
		return NULL;
	}
	bAddress = sPlace.eKind == CPP65PK_Address;
	if (bAddress)
	{
		Push("A");
	}

	psRight = Expression(pcBinary->mpcRight);
	if (!psRight)
	{
		return NULL;
	}

	if (pcBinary->meOperator == CT_Assignment)
	{
		if (!Convert(psRight, sPlace.psType, pcBinary->mpcRight))
		{
			return NULL;
		}
	}
	else
	{
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

		MoveAToB(psRight);
		if (bAddress)
		{
			Line("cpy X, [1 + S]")->AppendNewLine();
			ChangeX();
		}
		Load(&sPlace, "A", "X");
		sRight.psType = psRight;
		sRight.eKind = CPP65RK_B;
		sRight.lliValue = 0;
		psResult = Operate(eOperator, sPlace.psType, &sRight, pcBinary);
		if (!Convert(psResult, sPlace.psType, pcBinary))
		{
			return NULL;
		}
	}

	if (bAddress)
	{
		Pull("X");
		ChangeX();
	}
	Store(&sPlace, "A", "X");
	return sPlace.psType;
}


//////////////////////////////////////////////////////////////////////////
//
// c ? x : y.  Integer results take the usual arithmetic conversions.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::ConditionalExpression(CCPPSyntaxConditionalExpression* pcConditional)
{
	SCPP65Type*		psTrue;
	SCPP65Type*		psFalse;
	SCPP65Type*		psType;
	int				iFalse;
	int				iEnd;

	psTrue = TypeOf(pcConditional->mpcTrue);
	psFalse = TypeOf(pcConditional->mpcFalse);
	if (!psTrue || !psFalse)
	{
		return NULL;
	}
	if (psTrue->IsInteger() && psFalse->IsInteger())
	{
		psType = mcTypes.Arithmetic(psTrue, psFalse);
	}
	else if (psTrue->IsVoid() || psFalse->IsVoid())
	{
		psType = &mcTypes.msVoid;
	}
	else
	{
		psType = psTrue->IsPointer() ? psTrue : psFalse;
	}

	iFalse = NewLabel();
	iEnd = NewLabel();
	if (!Condition(pcConditional->mpcCondition, false, iFalse))
	{
		return NULL;
	}
	if (!Convert(Expression(pcConditional->mpcTrue), psType, pcConditional->mpcTrue))
	{
		return NULL;
	}
	Line("bra ");
	AppendLabel(mpszOut, iEnd);
	mpszOut->AppendNewLine();
	Label(iFalse);
	if (!Convert(Expression(pcConditional->mpcFalse), psType, pcConditional->mpcFalse))
	{
		return NULL;
	}
	Label(iEnd);
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// A condition as a bool value: 0 or 1 in A.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::ConditionValue(CCPPSyntax* pcExpression)
{
	int		iFalse;
	int		iEnd;

	iFalse = NewLabel();
	iEnd = NewLabel();
	if (!Condition(pcExpression, false, iFalse))
	{
		return NULL;
	}
	Line("cpy A, 1")->AppendNewLine();
	Line("bra ");
	AppendLabel(mpszOut, iEnd);
	mpszOut->AppendNewLine();
	Label(iFalse);
	Line("cpy A, 0")->AppendNewLine();
	Label(iEnd);
	return &mcTypes.msBool;
}


//////////////////////////////////////////////////////////////////////////
//
// True for the names of GCC's builtin functions, which are not declared.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsBuiltin(int64 lliName)
{
	char*	szName;
	int		iLength;

	if (lliName < 0)
	{
		return false;
	}
	szName = mpcTokeniser->GetIdentifier(lliName, &iLength);
	return szName && (iLength > 10) && (memcmp(szName, "__builtin_", 10) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// GCC's builtin functions: __builtin_ctz, __builtin_clz and __builtin_popcount
// (with their l and ll forms), __builtin_expect, __builtin_unreachable and
// __builtin_trap.  The 65C64's find set bit does ctz and clz; ctz(0) and clz(0) are
// the width in bits.  The result is in A.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::Builtin(CCPPSyntaxCallExpression* pcCall, int64 lliName)
{
	CChars			szName;
	char*			szIdentifier;
	int				iLength;
	SCPP65Type*		psArgument;
	SCPP65Type*		psType;
	bool			bWide;
	int				iArguments;

	szIdentifier = mpcTokeniser->GetIdentifier(lliName, &iLength);
	szName.Init();
	szName.Append(szIdentifier + 10, iLength - 10);
	iArguments = (int)pcCall->macpArguments.NumElements();

	if (szName.Equals("unreachable") || szName.Equals("trap"))
	{
		szName.Kill();
		if (iArguments != 0)
		{
			Error(pcCall, "The builtin takes no arguments");
			return NULL;
		}
		Line("stp")->AppendNewLine();
		return &mcTypes.msVoid;
	}
	if (szName.Equals("expect"))
	{
		szName.Kill();
		if (iArguments != 2)
		{
			Error(pcCall, "__builtin_expect takes two arguments");
			return NULL;
		}
		psType = Expression(pcCall->macpArguments.GetPtr(0));
		return psType ? mcTypes.Promote(psType) : NULL;
	}

	bWide = szName.EndsWith("ll");
	if (bWide)
	{
		szName.SetLength(szName.Length() - 2);
	}
	else if (szName.EndsWith('l'))
	{
		szName.SetLength(szName.Length() - 1);
	}
	if (!szName.Equals("ctz") && !szName.Equals("clz") && !szName.Equals("popcount"))
	{
		Error(pcCall, "This builtin is not supported by the 65C64 generator yet");
		szName.Kill();
		return NULL;
	}
	if (iArguments != 1)
	{
		Error(pcCall, "The builtin takes one argument");
		szName.Kill();
		return NULL;
	}

	psType = bWide ? &mcTypes.msUnsignedLongLong : &mcTypes.msUnsignedInt;
	psArgument = Expression(pcCall->macpArguments.GetPtr(0));
	if (!psArgument || !Convert(psArgument, psType, pcCall))
	{
		szName.Kill();
		return NULL;
	}

	if (bWide)
	{
		Line("wtth (A, B), 8")->AppendNewLine();
	}
	if (szName.Equals("ctz"))
	{
		Line("cpy B, 0")->AppendNewLine();
		Line("cep c")->AppendNewLine();
		Line("fsbi A, B, +1")->AppendNewLine();
		Line("cpy A, B")->AppendNewLine();
	}
	else if (szName.Equals("clz"))
	{
		//The highest set bit's index; -1 when there is none.
		Line("cpy B, ")->Append(bWide ? 63 : 31)->AppendNewLine();
		Line("cep c")->AppendNewLine();
		Line("fsbi A, B, -1")->AppendNewLine();
		Line("cpy A, ")->Append(bWide ? 63 : 31)->AppendNewLine();
		Line("sub A, B")->AppendNewLine();
	}
	else
	{
		//Bits counted in pairs, then nibbles, then the bytes are added together.
		Line("cpy B, A")->AppendNewLine();
		Line("shr B, 1")->AppendNewLine();
		Line("and B, ")->Append(bWide ? (int64)0x5555555555555555LL : (int64)0x55555555)->AppendNewLine();
		Line("sub A, B")->AppendNewLine();
		Line("cpy B, A")->AppendNewLine();
		Line("shr B, 2")->AppendNewLine();
		Line("and B, ")->Append(bWide ? (int64)0x3333333333333333LL : (int64)0x33333333)->AppendNewLine();
		Line("and A, ")->Append(bWide ? (int64)0x3333333333333333LL : (int64)0x33333333)->AppendNewLine();
		Line("add A, B")->AppendNewLine();
		Line("cpy B, A")->AppendNewLine();
		Line("shr B, 4")->AppendNewLine();
		Line("add A, B")->AppendNewLine();
		Line("and A, ")->Append(bWide ? (int64)0x0F0F0F0F0F0F0F0FLL : (int64)0x0F0F0F0F)->AppendNewLine();
		Line("cpy B, A")->AppendNewLine();
		Line("shr B, 8")->AppendNewLine();
		Line("add A, B")->AppendNewLine();
		Line("cpy B, A")->AppendNewLine();
		Line("shr B, 16")->AppendNewLine();
		Line("add A, B")->AppendNewLine();
		if (bWide)
		{
			Line("cpy B, A")->AppendNewLine();
			Line("shr B, 32")->AppendNewLine();
			Line("add A, B")->AppendNewLine();
		}
		Line("and A, 127")->AppendNewLine();
	}
	if (bWide)
	{
		Line("wtth (A, B), 4")->AppendNewLine();
	}
	szName.Kill();
	return &mcTypes.msInt;
}


//////////////////////////////////////////////////////////////////////////
//
// Arguments are pushed last first, each at its own size, and removed by the caller.
// The result is in A.  The overload is chosen by the arguments.  Function templates
// are instantiated, calls to member functions are in MemberCall and calls through
// pointers to functions use jsr A.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::CallExpression(CCPPSyntaxCallExpression* pcCall)
{
	CCPPSyntax*					pcCallee;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPSymbol*					pcSymbol;
	SCPP65Function*				psFunction;
	SCPP65Template*				psTemplate;
	SCPP65Type*					psType;
	int							iBytes;
	size						uiErrors;
	int64						lliName;

	pcCallee = SkipParentheses(pcCall->mpcCallee);
	if (pcCallee->GetKind() == CPPS_MemberExpression)
	{
		return MemberCall(pcCall, pcCallee);
	}
	pcSymbol = NULL;
	pcName = NULL;
	pcComponent = NULL;
	if (pcCallee->GetKind() == CPPS_IdExpression)
	{
		pcName = ((CCPPSyntaxIdExpression*)pcCallee)->mpcName;
		pcSymbol = pcName->mpcSymbol;
		pcComponent = pcName->GetLast();
		if (pcName->macpComponents.NumElements() >= 2)
		{
			return MemberCall(pcCall, pcCallee);
		}
	}

	//f<Arguments>(...) and f(...) for a function template.
	if (pcSymbol && (pcSymbol->meKind == CPPSK_FunctionTemplate))
	{
		psTemplate = GetTemplate(pcSymbol);
		if (!psTemplate)
		{
			Error(pcCallee, "The function template has not been defined");
			return NULL;
		}
		psFunction = InstantiateFunction(psTemplate, (pcComponent && pcComponent->mbTemplateArguments) ? &pcComponent->macpTemplateArguments : NULL, &pcCall->macpArguments, pcCall);
		if (!psFunction)
		{
			return NULL;
		}
		iBytes = PushArguments(psFunction, &pcCall->macpArguments, pcCall);
		if (iBytes < 0)
		{
			return NULL;
		}
		Call(psFunction, false, iBytes);
		return psFunction->psReturn;
	}

	//f() in a member function may be a member, even when the parser could not resolve it in a template.
	lliName = pcSymbol ? pcSymbol->mlliName : GetFunctionName(pcComponent);
	if (pcName && !pcName->mbGlobal && mpsFunction && mpsFunction->psClass && (!pcSymbol || (pcSymbol->meKind == CPPSK_Function)) && HasMember(mpsFunction->psClass, lliName))
	{
		return MemberCall(pcCall, pcCallee);
	}

	if (!pcSymbol && IsBuiltin(lliName))
	{
		return Builtin(pcCall, lliName);
	}

	if (!pcSymbol || (pcSymbol->meKind != CPPSK_Function))
	{
		psType = TypeOf(pcCallee);
		if (psType && psType->IsPointer() && psType->psBase->IsFunction())
		{
			return PointerCall(pcCall, psType->psBase);
		}
		Error(pcCallee, "Only calls to functions and pointers to functions are supported by the 65C64 generator yet");
		return NULL;
	}

	uiErrors = mcErrors.NumErrors();
	psFunction = FindFunction(NULL, lliName, CPP65FK_Function, &pcCall->macpArguments, false, pcCall);
	if (!psFunction)
	{
		if (mcErrors.NumErrors() == uiErrors)
		{
			ReportFailedDeclaration(lliName);
			Error(pcCallee, "The function has not been declared");
		}
		return NULL;
	}

	iBytes = PushArguments(psFunction, &pcCall->macpArguments, pcCall);
	if (iBytes < 0)
	{
		return NULL;
	}
	Call(psFunction, false, iBytes);
	return psFunction->psReturn;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the class or a base has a member function of the name.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::HasMember(SCPP65Class* psClass, int64 lliName)
{
	SCPP65Function*	psFunction;
	size			i;

	if (!CompleteClass(psClass, NULL))
	{
		return false;
	}
	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		if ((psFunction->psClass == psClass) && (psFunction->lliName == lliName) && (psFunction->eKind == CPP65FK_Function))
		{
			return true;
		}
	}
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		if (HasMember(psClass->macBases.Get(i)->psClass, lliName))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A call through a pointer to a function: the arguments are pushed, then the
// pointer is worked out into A and called.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::PointerCall(CCPPSyntaxCallExpression* pcCall, SCPP65Type* psFunctionType)
{
	int		iBytes;

	iBytes = PushParameters(psFunctionType->papsParameters, NULL, psFunctionType->bVariadic, &pcCall->macpArguments, pcCall);
	if (iBytes < 0)
	{
		return NULL;
	}
	if (!Expression(pcCall->mpcCallee))
	{
		return NULL;
	}
	Line("jsr A")->AppendNewLine();
	ChangeX();
	if (iBytes > 0)
	{
		Line("add S, ")->Append(iBytes)->AppendNewLine();
		miDepth -= iBytes;
	}
	return psFunctionType->psBase;
}


//////////////////////////////////////////////////////////////////////////
//
// A call to a function whose declaration could not be generated: the declaration's
// errors are reported now.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::ReportFailedDeclaration(int64 lliName)
{
	CCPPSyntaxDeclarator*		pcDeclarator;
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	CCPPSyntax*					pcParent;
	size						i;

	for (i = 0; i < mapcFailed.NumElements(); i++)
	{
		pcDeclarator = (CCPPSyntaxDeclarator*)mapcFailed.GetPtr(i);
		if (!pcDeclarator->GetName() || (GetFunctionName(pcDeclarator->GetName()->GetLast()) != lliName))
		{
			continue;
		}
		pcParent = pcDeclarator->mpcParent;
		pcSpecifiers = NULL;
		if (pcParent && (pcParent->GetKind() == CPPS_InitDeclarator) && pcParent->mpcParent && (pcParent->mpcParent->GetKind() == CPPS_SimpleDeclaration))
		{
			pcSpecifiers = ((CCPPSyntaxSimpleDeclaration*)pcParent->mpcParent)->mpcSpecifiers;
		}
		else if (pcParent && (pcParent->GetKind() == CPPS_FunctionDefinition))
		{
			pcSpecifiers = ((CCPPSyntaxFunctionDefinition*)pcParent)->mpcSpecifiers;
		}
		DeclareFunction(pcSpecifiers, pcDeclarator, false, NULL, false);
		return;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::ThisExpression(CCPPSyntax* pcThis)
{
	if ((miThisOffset == -1) || !mpsFunction || !mpsFunction->psClass)
	{
		Error(pcThis, "this is only in member functions");
		return NULL;
	}
	EnsureThis();
	Line("cpy A, X")->AppendNewLine();
	return mcTypes.Pointer(mpsFunction->psClass->psType);
}


//////////////////////////////////////////////////////////////////////////
//
// (type)x, static_cast<type>(x), reinterpret_cast<type>(x), const_cast<type>(x) and
// type(x).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::CastExpression(CCPPSyntaxTypeId* pcTypeId, CCPPSyntax* pcOperand, CCPPSyntax* pcNode)
{
	SCPP65Type*		psTo;
	SCPP65Type*		psFrom;

	psTo = GetTypeIdType(pcTypeId);
	if (!psTo)
	{
		return NULL;
	}
	psFrom = Expression(pcOperand);
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
		Error(pcNode, "Only casts to integers and pointers are supported by the 65C64 generator yet");
		return NULL;
	}
	if (!Convert(psFrom, psTo, pcNode))
	{
		return NULL;
	}
	return psTo;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::SizeofExpression(CCPPSyntax* pcSizeof)
{
	int64	lliBytes;

	//sizeof and type traits (__is_trivially_constructible(T) ...) are constants.
	if (!Constant(pcSizeof, &lliBytes))
	{
		Error(pcSizeof, "This sizeof or type trait is not supported by the 65C64 generator yet");
		return NULL;
	}
	Line("cpy A, ")->Append(lliBytes)->AppendNewLine();
	return (pcSizeof->GetKind() == CPPS_SizeofExpression) ? &mcTypes.msUnsignedInt : &mcTypes.msBool;
}


//////////////////////////////////////////////////////////////////////////
//
// new(address) Type constructs at address and gives it as a Type*.  There is no
// heap yet, so new without a placement is not supported.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::NewExpression(CCPPSyntaxNewExpression* pcNew)
{
	SCPP65Type*		psType;
	SCPP65Type*		psAddress;
	SCPP65Place		sPlace;

	if (!pcNew->mbPlacement || (pcNew->macpPlacement.NumElements() != 1) || (pcNew->macpArraySizes.NumElements() != 0))
	{
		Error(pcNew, "Only placement new is supported by the 65C64 generator yet");
		return NULL;
	}
	psType = GetTypeIdType(pcNew->mpcType);
	if (!psType)
	{
		return NULL;
	}
	psAddress = Expression(pcNew->macpPlacement.GetPtr(0));
	if (!psAddress)
	{
		return NULL;
	}
	if (!psAddress->IsPointer())
	{
		Error(pcNew, "Placement new needs an address");
		return NULL;
	}

	sPlace.eKind = CPP65PK_Address;
	sPlace.psType = psType;
	sPlace.iOffset = 0;
	sPlace.lliName = -1;
	sPlace.iString = -1;
	sPlace.psClass = NULL;
	if (psType->IsClass() || pcNew->mpcInitializer)
	{
		//The address is kept on the stack: constructing uses A.
		Push("A");
		if (!Construct(&sPlace, NULL, pcNew->mpcInitializer, pcNew))
		{
			return NULL;
		}
		Pull("A");
	}
	return mcTypes.Pointer(psType);
}


//////////////////////////////////////////////////////////////////////////
//
// The type of an expression without generating its code.  Its errors are reported
// when it is generated.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::TypeOf(CCPPSyntax* pcExpression)
{
	CChars			szDiscard;
	CChars*			pszOut;
	SCPP65Type*		psType;
	int				iDepth;
	int				iTemporaries;
	bool			bThisInX;
	SCPP65TypeOf*	psTypeOf;
	SCPP65TypeOf	sTypeOf;
	size			uiErrors;
	size			uiStrings;

	//An expression's type only depends on the template arguments and the function it is in.
	psTypeOf = mcTypeOfs.Get((int64)(size)pcExpression);
	if (psTypeOf && (psTypeOf->psBindings == mpsBindings) && (psTypeOf->psFunction == mpsFunction))
	{
		return psTypeOf->psType;
	}

	pszOut = mpszOut;
	iDepth = miDepth;
	iTemporaries = miTemporaries;
	bThisInX = mbThisInX;
	uiErrors = mcErrors.NumErrors();
	uiStrings = maszStrings.NumElements();

	szDiscard.Init();
	mpszOut = &szDiscard;
	psType = Expression(pcExpression);
	mpszOut = pszOut;
	szDiscard.Kill();

	miDepth = iDepth;
	miTemporaries = iTemporaries;
	mbThisInX = bThisInX;
	mcErrors.Truncate(uiErrors);
	while (maszStrings.NumElements() > uiStrings)
	{
		maszStrings.Tail()->Kill();
		maszStrings.Pop();
	}

	sTypeOf.psBindings = mpsBindings;
	sTypeOf.psFunction = mpsFunction;
	sTypeOf.psType = psType;
	mcTypeOfs.Put((int64)(size)pcExpression, &sTypeOf);
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// Leaves the left operand in A and the right in B, a constant or a pseudo register.
// A simple operand (a constant or a variable) is loaded straight into its register,
// as loading it only changes that register and Y.  Otherwise the right is worked out
// first and kept in a pseudo register while the left is, or pushed if the left calls
// a function (which may use the pseudo registers too).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::Operands(CCPPSyntax* pcLeft, CCPPSyntax* pcRight, SCPP65Right* psRight)
{
	SCPP65Type*		psLeft;
	int				iTemporary;
	bool			bWide;

	psRight->eKind = CPP65RK_B;
	psRight->lliValue = 0;
	if (IsSimple(pcRight))
	{
		psLeft = Expression(pcLeft);
		if (!psLeft)
		{
			return NULL;
		}
		if (SkipParentheses(pcRight)->GetKind() == CPPS_LiteralExpression)
		{
			psRight->psType = Literal((CCPPSyntaxLiteralExpression*)SkipParentheses(pcRight));
			if (Constant(pcRight, &psRight->lliValue))
			{
				psRight->eKind = CPP65RK_Constant;
			}
		}
		else if (Constant(pcRight, &psRight->lliValue))
		{
			psRight->psType = &mcTypes.msInt;
			psRight->eKind = CPP65RK_Constant;
		}
		else
		{
			psRight->psType = SimpleValue(pcRight, "B");
		}
	}
	else if (IsSimple(pcLeft))
	{
		psRight->psType = Expression(pcRight);
		if (!psRight->psType)
		{
			return NULL;
		}
		MoveAToB(psRight->psType);
		psLeft = SimpleValue(pcLeft, "A");
	}
	else if (!ContainsCall(pcLeft) && (miTemporaries + 4 <= CPP65_TEMPORARY_BYTES))
	{
		psRight->psType = Expression(pcRight);
		if (!psRight->psType)
		{
			return NULL;
		}
		bWide = psRight->psType->IsWide();
		iTemporary = miTemporaries;
		miTemporaries += bWide ? 8 : 4;
		if (bWide)
		{
			Line("wth A, 8")->AppendNewLine();
		}
		Line("cpy [")->Append(iTemporary)->Append(" + W], A")->AppendNewLine();
		if (bWide)
		{
			Line("wth A, 4")->AppendNewLine();
		}
		psLeft = Expression(pcLeft);
		miTemporaries -= bWide ? 8 : 4;

		//Nothing else uses the pseudo register before the operator does.
		psRight->eKind = CPP65RK_Temporary;
		psRight->lliValue = iTemporary;
	}
	else
	{
		psRight->psType = Expression(pcRight);
		if (!psRight->psType)
		{
			return NULL;
		}
		PushWide("A", psRight->psType->IsWide());
		psLeft = Expression(pcLeft);
		PullWide("B", psRight->psType->IsWide());
	}

	if (!psLeft || !psRight->psType)
	{
		return NULL;
	}
	return psLeft;
}


//////////////////////////////////////////////////////////////////////////
//
// Loads a constant or pseudo register right operand into B for the instructions
// that need it there.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::RightInB(SCPP65Right* psRight)
{
	if (psRight->eKind != CPP65RK_B)
	{
		if (psRight->psType->IsWide())
		{
			Line("wth B, 8")->AppendNewLine();
		}
		Line("cpy B, ");
		AppendRight(mpszOut, psRight)->AppendNewLine();
		if (psRight->psType->IsWide())
		{
			Line("wth B, 4")->AppendNewLine();
		}
		psRight->eKind = CPP65RK_B;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Appends B, the constant or the pseudo register.
//
//////////////////////////////////////////////////////////////////////////
CChars* CCPP65Generator::AppendRight(CChars* psz, SCPP65Right* psRight)
{
	switch (psRight->eKind)
	{
	case CPP65RK_Constant:
		return psz->Append(psRight->psType->IsWide() ? psRight->lliValue : (int64)(int32)psRight->lliValue);
	case CPP65RK_Temporary:
		return psz->Append('[')->Append((int)psRight->lliValue)->Append(" + W]");
	default:
		return psz->Append('B');
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A = A op right for the arithmetic, bitwise and shift operators.  Pointer + integer,
// pointer - integer and pointer - pointer are scaled by the size of what is pointed
// to.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::Operate(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Right* psRight, CCPPSyntax* pcNode)
{
	SCPP65Type*		psType;
	int				iBytes;
	int				iPositive;
	int				iEnd;
	const char*		szInstruction;

	if (!psLeft || !psRight->psType)
	{
		return NULL;
	}

	if (psLeft->IsPointer() || psRight->psType->IsPointer())
	{
		if (((eOperator == CT_Add) || (eOperator == CT_Subtract)) && psLeft->IsPointer() && psRight->psType->IsInteger())
		{
			iBytes = psLeft->GetElementBytes();
			if (psRight->eKind == CPP65RK_Constant)
			{
				psRight->lliValue *= iBytes;
			}
			else
			{
				RightInB(psRight);
				Scale("B", iBytes);
			}
			Line((eOperator == CT_Add) ? "add A, " : "sub A, ");
			AppendRight(mpszOut, psRight)->AppendNewLine();
			return psLeft;
		}
		if ((eOperator == CT_Add) && psLeft->IsInteger() && psRight->psType->IsPointer())
		{
			RightInB(psRight);
			Scale("A", psRight->psType->GetElementBytes());
			Line("add A, B")->AppendNewLine();
			return psRight->psType;
		}
		if ((eOperator == CT_Subtract) && psLeft->IsPointer() && psRight->psType->IsPointer())
		{
			RightInB(psRight);
			Line("sub A, B")->AppendNewLine();
			iBytes = psLeft->GetElementBytes();
			if (iBytes != 1)
			{
				Line("cpy B, ")->Append(iBytes)->AppendNewLine();
				Line("jsr R_DivideSigned")->AppendNewLine();
				ChangeX();
				mbSignedDivide = true;
			}
			return &mcTypes.msInt;
		}
		Error(pcNode, "This operator does not work with pointers");
		return NULL;
	}

	if (!psLeft->IsInteger() || !psRight->psType->IsInteger())
	{
		Error(pcNode, "The operands must be integers");
		return NULL;
	}

	psType = mcTypes.Arithmetic(psLeft, psRight->psType);
	if ((eOperator == CT_LeftShift) || (eOperator == CT_RightShift))
	{
		psType = mcTypes.Promote(psLeft);
	}
	if (psType->IsWide())
	{
		return OperateWide(eOperator, psLeft, psRight, psType, pcNode);
	}
	szInstruction = NULL;
	switch (eOperator)
	{
	case CT_Add:
		szInstruction = "add A, ";
		break;
	case CT_Subtract:
		szInstruction = "sub A, ";
		break;
	case CT_BitwiseAnd:
		szInstruction = "and A, ";
		break;
	case CT_BitwiseOr:
		szInstruction = "or A, ";
		break;
	case CT_BitwiseXor:
		szInstruction = "xor A, ";
		break;
	case CT_Multiply:
		//A x B -> Y.
		RightInB(psRight);
		Multiply("mul");
		Line("cpy A, Y")->AppendNewLine();
		break;
	case CT_Divide:
	case CT_Modulus:
		RightInB(psRight);
		if (psType->bSigned)
		{
			Line("jsr R_DivideSigned")->AppendNewLine();
			ChangeX();
			mbSignedDivide = true;
		}
		else
		{
			//Y / A -> Y, the remainder in B.  B must start at 0.
			Line("cpy Y, A")->AppendNewLine();
			Line("cpy A, B")->AppendNewLine();
			Line("cpy B, 0")->AppendNewLine();
			Multiply("div");
			Line("cpy A, Y")->AppendNewLine();
		}
		if (eOperator == CT_Modulus)
		{
			Line("cpy A, B")->AppendNewLine();
		}
		break;
	case CT_LeftShift:
		psType = mcTypes.Promote(psLeft);
		szInstruction = "shl A, ";
		break;
	case CT_RightShift:
		psType = mcTypes.Promote(psLeft);
		if (!psType->bSigned)
		{
			szInstruction = "shr A, ";
			break;
		}

		//shr is a logical shift.  A negative value is inverted, shifted and inverted back.
		iPositive = NewLabel();
		iEnd = NewLabel();
		Line("cmp A, 0")->AppendNewLine();
		Line("brac n, ");
		AppendLabel(mpszOut, iPositive);
		mpszOut->AppendNewLine();
		Line("xor A, -1")->AppendNewLine();
		Line("shr A, ");
		AppendRight(mpszOut, psRight)->AppendNewLine();
		Line("xor A, -1")->AppendNewLine();
		Line("bra ");
		AppendLabel(mpszOut, iEnd);
		mpszOut->AppendNewLine();
		Label(iPositive);
		Line("shr A, ");
		AppendRight(mpszOut, psRight)->AppendNewLine();
		Label(iEnd);
		break;
	default:
		Error(pcNode, "This operator is not supported by the 65C64 generator yet");
		return NULL;
	}

	if (szInstruction)
	{
		Line(szInstruction);
		AppendRight(mpszOut, psRight)->AppendNewLine();
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// A = A op B for 64 bit integers.  The narrower operand is widened, then the
// operation runs with A and B (and Y for mul and div) 64 bits wide.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::OperateWide(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Right* psRight, SCPP65Type* psType, CCPPSyntax* pcNode)
{
	bool	bShift;
	int		iPositive;
	int		iEnd;

	bShift = (eOperator == CT_LeftShift) || (eOperator == CT_RightShift);
	RightInB(psRight);
	if (!psLeft->IsWide())
	{
		Widen("A", psLeft->bSigned);
	}
	if (!psRight->psType->IsWide())
	{
		Widen("B", !bShift && psRight->psType->bSigned);
	}

	Line("wtth (A, B), 8")->AppendNewLine();
	switch (eOperator)
	{
	case CT_Add:			Line("add A, B")->AppendNewLine();	break;
	case CT_Subtract:		Line("sub A, B")->AppendNewLine();	break;
	case CT_BitwiseAnd:		Line("and A, B")->AppendNewLine();	break;
	case CT_BitwiseOr:		Line("or A, B")->AppendNewLine();	break;
	case CT_BitwiseXor:		Line("xor A, B")->AppendNewLine();	break;
	case CT_LeftShift:		Line("shl A, B")->AppendNewLine();	break;
	case CT_Multiply:
		Line("wth Y, 8")->AppendNewLine();
		Multiply("mul");
		Line("cpy A, Y")->AppendNewLine();
		Line("wth Y, 4")->AppendNewLine();
		break;
	case CT_Divide:
	case CT_Modulus:
		if (psType->bSigned)
		{
			Line("jsr R_DivideSigned64")->AppendNewLine();
			ChangeX();
			mbSignedDivide64 = true;
		}
		else
		{
			Line("wth Y, 8")->AppendNewLine();
			Line("cpy Y, A")->AppendNewLine();
			Line("cpy A, B")->AppendNewLine();
			Line("cpy B, 0")->AppendNewLine();
			Multiply("div");
			Line("cpy A, Y")->AppendNewLine();
			Line("wth Y, 4")->AppendNewLine();
		}
		if (eOperator == CT_Modulus)
		{
			Line("cpy A, B")->AppendNewLine();
		}
		break;
	case CT_RightShift:
		if (!psType->bSigned)
		{
			Line("shr A, B")->AppendNewLine();
			break;
		}
		iPositive = NewLabel();
		iEnd = NewLabel();
		Line("cmp A, 0")->AppendNewLine();
		Line("brac n, ");
		AppendLabel(mpszOut, iPositive);
		mpszOut->AppendNewLine();
		Line("xor A, -1")->AppendNewLine();
		Line("shr A, B")->AppendNewLine();
		Line("xor A, -1")->AppendNewLine();
		Line("bra ");
		AppendLabel(mpszOut, iEnd);
		mpszOut->AppendNewLine();
		Label(iPositive);
		Line("shr A, B")->AppendNewLine();
		Label(iEnd);
		break;
	default:
		Line("wtth (A, B), 4")->AppendNewLine();
		Error(pcNode, "This operator is not supported by the 65C64 generator yet");
		return NULL;
	}
	Line("wtth (A, B), 4")->AppendNewLine();
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// Copies A to B, all 64 bits for a 64 bit value.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::MoveAToB(SCPP65Type* psType)
{
	if (psType && psType->IsWide())
	{
		Line("wtth (A, B), 8")->AppendNewLine();
		Line("cpy B, A")->AppendNewLine();
		Line("wtth (A, B), 4")->AppendNewLine();
	}
	else
	{
		Line("cpy B, A")->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Loads a constant into a register, all 64 bits for a 64 bit type.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::LoadConstant(const char* szRegister, SCPP65Type* psType, int64 lliValue)
{
	if (psType->IsWide())
	{
		Line("wth ")->Append(szRegister)->Append(", 8")->AppendNewLine();
		Line("cpy ")->Append(szRegister)->Append(", ")->Append(lliValue)->AppendNewLine();
		Line("wth ")->Append(szRegister)->Append(", 4")->AppendNewLine();
	}
	else
	{
		Line("cpy ")->Append(szRegister)->Append(", ")->Append((int64)(int32)lliValue)->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Compares A (left) with B (right) and branches to iLabel when the comparison's
// result is bJumpIf.  Pointers compare unsigned.
//
// After cmp X, Y: z is X = Y, c clear is X < Y unsigned and n != v is X < Y signed.
// > and <= swap the operands.  There is no branch on n = v, so it branches round
// a bra.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Compare(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Right* psRight, bool bJumpIf, int iLabel, CCPPSyntax* pcNode)
{
	bool			bSigned;
	bool			bSwap;
	bool			bLess;
	int				iSkip;
	SCPP65Type*		psCompare;

	if (!psLeft || !psRight->psType)
	{
		return false;
	}
	if (psLeft->IsPointer() || psRight->psType->IsPointer())
	{
		bSigned = false;
	}
	else if (psLeft->IsInteger() && psRight->psType->IsInteger())
	{
		bSigned = mcTypes.Arithmetic(psLeft, psRight->psType)->bSigned;
	}
	else
	{
		return Error(pcNode, "These operands cannot be compared");
	}

	bSwap = (eOperator == CT_GreaterThan) || (eOperator == CT_LessThanEqualTo);
	psCompare = (psLeft->IsInteger() && psRight->psType->IsInteger()) ? mcTypes.Arithmetic(psLeft, psRight->psType) : NULL;
	if (psCompare && psCompare->IsWide())
	{
		//64 bits: both are widened and compared with the registers 64 bits wide.
		RightInB(psRight);
		if (!psLeft->IsWide())
		{
			Widen("A", psLeft->bSigned);
		}
		if (!psRight->psType->IsWide())
		{
			Widen("B", psRight->psType->bSigned);
		}
		if (bSwap)
		{
			eOperator = (eOperator == CT_GreaterThan) ? CT_LessThan : CT_GreaterThanEqualTo;
		}
		Line("wtth (A, B), 8")->AppendNewLine();
		Line(bSwap ? "cmp B, A" : "cmp A, B")->AppendNewLine();
		Line("wtth (A, B), 4")->AppendNewLine();
	}
	else if (bSwap)
	{
		RightInB(psRight);
		eOperator = (eOperator == CT_GreaterThan) ? CT_LessThan : CT_GreaterThanEqualTo;
		Line("cmp B, A")->AppendNewLine();
	}
	else
	{
		Line("cmp A, ");
		AppendRight(mpszOut, psRight)->AppendNewLine();
	}

	if ((eOperator == CT_EqualTo) || (eOperator == CT_NotEqualTo))
	{
		Line(((eOperator == CT_EqualTo) == bJumpIf) ? "bras z, " : "brac z, ");
		AppendLabel(mpszOut, iLabel);
		mpszOut->AppendNewLine();
		return true;
	}

	//bLess: branch when the left is less than the right.
	bLess = (eOperator == CT_LessThan) == bJumpIf;
	if (!bSigned)
	{
		Line(bLess ? "brac c, " : "bras c, ");
		AppendLabel(mpszOut, iLabel);
		mpszOut->AppendNewLine();
	}
	else if (bLess)
	{
		Line("braso (n, v), ");
		AppendLabel(mpszOut, iLabel);
		mpszOut->AppendNewLine();
	}
	else
	{
		iSkip = NewLabel();
		Line("braso (n, v), ");
		AppendLabel(mpszOut, iSkip);
		mpszOut->AppendNewLine();
		Line("bra ");
		AppendLabel(mpszOut, iLabel);
		mpszOut->AppendNewLine();
		Label(iSkip);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Branches to iLabel when the condition is bJumpIf.  && and || only work out their
// right operand when they need to.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Condition(CCPPSyntax* pcExpression, bool bJumpIf, int iLabel)
{
	CCPPSyntaxBinaryExpression*	pcBinary;
	CCPPSyntaxUnaryExpression*	pcUnary;
	SCPP65Type*					psLeft;
	SCPP65Right					sRight;
	SCPP65Type*					psType;
	SCPP65Variable*				psVariable;
	SCPP65Place					sPlace;
	int							iSkip;
	bool						bResult;

	pcExpression = SkipParentheses(pcExpression);
	if (!pcExpression)
	{
		return false;
	}

	if (pcExpression->GetKind() == CPPS_UnaryExpression)
	{
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		if (pcUnary->meOperator == CT_LogicalNot)
		{
			return Condition(pcUnary->mpcOperand, !bJumpIf, iLabel);
		}
	}
	else if (pcExpression->GetKind() == CPPS_BinaryExpression)
	{
		pcBinary = (CCPPSyntaxBinaryExpression*)pcExpression;
		switch (pcBinary->meOperator)
		{
		case CT_LogicalAnd:
		case CT_LogicalOr:
			if ((pcBinary->meOperator == CT_LogicalAnd) != bJumpIf)
			{
				bResult = Condition(pcBinary->mpcLeft, bJumpIf, iLabel);
				return bResult && Condition(pcBinary->mpcRight, bJumpIf, iLabel);
			}
			iSkip = NewLabel();
			bResult = Condition(pcBinary->mpcLeft, !bJumpIf, iSkip);
			bResult = bResult && Condition(pcBinary->mpcRight, bJumpIf, iLabel);
			Label(iSkip);
			return bResult;
		case CT_EqualTo:
		case CT_NotEqualTo:
		case CT_LessThan:
		case CT_LessThanEqualTo:
		case CT_GreaterThan:
		case CT_GreaterThanEqualTo:
			psLeft = Operands(pcBinary->mpcLeft, pcBinary->mpcRight, &sRight);
			return Compare(pcBinary->meOperator, psLeft, &sRight, bJumpIf, iLabel, pcBinary);
		default:
			break;
		}
	}
	else if (pcExpression->GetKind() == CPPS_SimpleDeclaration)
	{
		if (!ConditionDeclaration(pcExpression))
		{
			return Error(pcExpression, "This condition is not supported by the 65C64 generator yet");
		}
		psVariable = GetVariable(((CCPPSyntaxInitDeclarator*)((CCPPSyntaxSimpleDeclaration*)pcExpression)->macpDeclarators.GetPtr(0))->mpcSymbol);
		VariablePlace(psVariable, &sPlace);
		Load(&sPlace, "A", NULL);
		CompareZero(sPlace.psType);
		Line(bJumpIf ? "brac z, " : "bras z, ");
		AppendLabel(mpszOut, iLabel);
		mpszOut->AppendNewLine();
		return true;
	}

	psType = Expression(pcExpression);
	if (!psType)
	{
		return false;
	}
	if (!psType->IsScalar())
	{
		return Error(pcExpression, "A condition must be an integer or a pointer");
	}
	CompareZero(psType);
	Line(bJumpIf ? "brac z, " : "bras z, ");
	AppendLabel(mpszOut, iLabel);
	mpszOut->AppendNewLine();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Converts the value in A from psFrom to psTo.  Narrow integers are kept sign or
// zero extended to 32 bits in registers, so only narrowing changes A.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Convert(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode)
{
	int		iZero;
	int		iOffset;
	bool	bExtended;

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
		return Error(pcNode, "Objects can only be copied or used through pointers by the 65C64 generator yet");
	}
	if (!psFrom->IsScalar() || !psTo->IsScalar())
	{
		return Error(pcNode, "This conversion is not supported by the 65C64 generator yet");
	}
	if (!Supported(psFrom, pcNode) || !Supported(psTo, pcNode))
	{
		return false;
	}

	//A pointer to a derived class points at its base, which may be further on.
	if (psFrom->IsPointer() && psTo->IsPointer() && psFrom->psBase->IsClass() && psTo->psBase->IsClass() && (psFrom->psBase->psClass != psTo->psBase->psClass) && psFrom->psBase->psClass->bComplete && IsDerived(psFrom->psBase->psClass, psTo->psBase->psClass, &iOffset) && (iOffset != 0))
	{
		iZero = NewLabel();
		Line("cmp A, 0")->AppendNewLine();
		Line("bras z, ");
		AppendLabel(mpszOut, iZero);
		mpszOut->AppendNewLine();
		Line("add A, ")->Append(iOffset)->AppendNewLine();
		Label(iZero);
		return true;
	}

	//To 64 bits: a narrower value is extended by its own signedness.  From 64 bits: the low bits are already in A.
	if (psTo->IsWide() && !psFrom->IsWide())
	{
		Widen("A", psFrom->bSigned && !psFrom->IsPointer());
		return true;
	}

	if (psTo->eKind == CPP65TK_Bool)
	{
		if (psFrom->eKind != CPP65TK_Bool)
		{
			iZero = NewLabel();
			CompareZero(psFrom);
			Line("bras z, ");
			AppendLabel(mpszOut, iZero);
			mpszOut->AppendNewLine();
			Line("cpy A, 1")->AppendNewLine();
			Label(iZero);
		}
		return true;
	}

	if (psTo->iBytes < 4)
	{
		//A is already right when it holds a narrower or equal integer extended the same way.
		bExtended = false;
		if (psFrom->IsInteger())
		{
			if (psFrom->iBytes < psTo->iBytes)
			{
				bExtended = !psFrom->bSigned || psTo->bSigned;
			}
			else if (psFrom->iBytes == psTo->iBytes)
			{
				bExtended = psFrom->bSigned == psTo->bSigned;
			}
		}
		if (!bExtended)
		{
			Truncate("A", psTo->iBytes, psTo->bSigned);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// cmp A, 0 at the value's width.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::CompareZero(SCPP65Type* psType)
{
	if (psType->IsWide())
	{
		Line("wth A, 8")->AppendNewLine();
		Line("cmp A, 0")->AppendNewLine();
		Line("wth A, 4")->AppendNewLine();
	}
	else
	{
		Line("cmp A, 0")->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Floating point can be declared but not used yet.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Supported(SCPP65Type* psType, CCPPSyntax* pcNode)
{
	if (psType->IsFloat())
	{
		return Error(pcNode, "Floating point is not supported by the 65C64 generator yet");
	}
	if (psType->IsScalar() && (psType->iBytes > 8))
	{
		return Error(pcNode, "Integers wider than 64 bits are not supported by the 65C64 generator");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps the low iBytes of a register, zero extended, or sign extended by flipping
// and subtracting the sign bit.  The register stays 32 bits wide.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::Truncate(const char* szRegister, int iBytes, bool bSigned)
{
	int64	lliSign;

	lliSign = (int64)1 << (8 * iBytes - 1);
	Line("and ")->Append(szRegister)->Append(", ")->Append(2 * lliSign - 1)->AppendNewLine();
	if (bSigned)
	{
		Line("xor ")->Append(szRegister)->Append(", ")->Append(lliSign)->AppendNewLine();
		Line("sub ")->Append(szRegister)->Append(", ")->Append(lliSign)->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Multiplies A or B by iBytes, for pointer arithmetic.  The other of A and B is
// kept.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::Scale(const char* szRegister, int iBytes)
{
	int		iShift;
	bool	bA;

	if (iBytes == 1)
	{
		return;
	}
	for (iShift = 1; iShift < 31; iShift++)
	{
		if (iBytes == (1 << iShift))
		{
			Line("shl ")->Append(szRegister)->Append(", ")->Append(iShift)->AppendNewLine();
			return;
		}
	}

	//mul is A x B -> Y.
	bA = szRegister[0] == 'A';
	Push(bA ? "B" : "A");
	Line(bA ? "cpy B, " : "cpy A, ")->Append(iBytes)->AppendNewLine();
	Multiply("mul");
	Line("cpy ")->Append(szRegister)->Append(", Y")->AppendNewLine();
	Pull(bA ? "B" : "A");
}


//////////////////////////////////////////////////////////////////////////
//
// Finds where an lvalue is.  Its address may be worked out into A.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Place(CCPPSyntax* pcExpression, SCPP65Place* psPlace)
{
	CCPPSyntaxIdExpression*			pcId;
	CCPPSyntaxUnaryExpression*		pcUnary;
	CCPPSyntaxSubscriptExpression*	pcSubscript;
	SCPP65Variable*					psVariable;
	SCPP65Field*					psField;
	CCPPSymbol*						pcSymbol;
	SCPP65Type*						psType;
	SCPP65Right						sIndex;
	CCPPSyntax*						pcArray;
	CCPPSyntax*						pcIndex;
	CArraySyntax					apcIndex;
	int64							lliIndex;
	int								iOffset;

	pcExpression = SkipParentheses(pcExpression);
	switch (pcExpression->GetKind())
	{
	case CPPS_IdExpression:
		pcId = (CCPPSyntaxIdExpression*)pcExpression;
		pcSymbol = pcId->mpcName->mpcSymbol;
		psVariable = pcSymbol ? GetVariable(pcSymbol) : NULL;
		if (psVariable)
		{
			VariablePlace(psVariable, psPlace);
			if (psPlace->psType->IsReference())
			{
				DerefPlace(psPlace);
			}
			return true;
		}
		psField = (pcSymbol && (pcSymbol->meKind == CPPSK_Field) && mpsFunction && mpsFunction->psClass) ? FindFieldSymbol(mpsFunction->psClass, pcSymbol, &iOffset) : NULL;
		if (!psField && !pcSymbol && mpsFunction && mpsFunction->psClass && (pcId->mpcName->macpComponents.NumElements() == 1))
		{
			//A name in a template the parser could not resolve.
			psField = FindField(mpsFunction->psClass, GetComponentName(pcId->mpcName->GetLast()), &iOffset);
		}
		if (!psField)
		{
			if (IsFunctionName(pcId))
			{
				return Error(pcExpression, "This is not an lvalue");
			}
			return Error(pcExpression, "Only variables are supported by the 65C64 generator here yet");
		}
		if (miThisOffset == -1)
		{
			return Error(pcExpression, "A field needs an object");
		}
		psPlace->eKind = CPP65PK_This;
		psPlace->psType = psField->psType;
		psPlace->iOffset = iOffset;
		psPlace->lliName = -1;
		psPlace->iString = -1;
		psPlace->psClass = NULL;
		if (psPlace->psType->IsReference())
		{
			DerefPlace(psPlace);
		}
		return true;

	case CPPS_MemberExpression:
		return MemberPlace((CCPPSyntaxMemberExpression*)pcExpression, psPlace);

	case CPPS_UnaryExpression:
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		if (pcUnary->meOperator != CT_Multiply)
		{
			break;
		}
		psType = Expression(pcUnary->mpcOperand);
		if (!psType)
		{
			return false;
		}
		if (!psType->IsPointer() || psType->psBase->IsVoid())
		{
			return Error(pcExpression, "Only pointers can be dereferenced");
		}
		psPlace->eKind = CPP65PK_Address;
		psPlace->psType = psType->psBase;
		psPlace->iOffset = 0;
		psPlace->lliName = -1;
		psPlace->iString = -1;
		psPlace->psClass = NULL;
		return true;

	case CPPS_SubscriptExpression:
		pcSubscript = (CCPPSyntaxSubscriptExpression*)pcExpression;
		pcArray = pcSubscript->mpcArray;
		pcIndex = pcSubscript->mpcIndex;

		//object[index] is a call to the object's operator[].
		psType = TypeOf(pcArray);
		if (psType && psType->IsClass())
		{
			apcIndex.Init();
			apcIndex.Add(pcIndex);
			psType = OperatorCall(pcArray, CT_OpenSquareBracket, &apcIndex, pcExpression);
			apcIndex.Kill();
			return ResultPlace(psType, psPlace, pcExpression);
		}

		//An element of an array variable at a constant index is the variable's place moved on.
		if (Constant(pcIndex, &lliIndex) && (SkipParentheses(pcArray)->GetKind() == CPPS_IdExpression))
		{
			pcId = (CCPPSyntaxIdExpression*)SkipParentheses(pcArray);
			psVariable = pcId->mpcName->mpcSymbol ? GetVariable(pcId->mpcName->mpcSymbol) : NULL;
			if (psVariable && psVariable->psType->IsArray())
			{
				VariablePlace(psVariable, psPlace);
				psPlace->psType = psVariable->psType->psBase;
				psPlace->iOffset += (int)lliIndex * psPlace->psType->iBytes;
				return true;
			}
		}

		psType = Operands(pcArray, pcIndex, &sIndex);
		if (!psType)
		{
			return false;
		}
		if ((sIndex.psType->IsPointer() && psType->IsInteger()) || (psType->IsPointer() && sIndex.psType->IsInteger()))
		{
			psType = Operate(CT_Add, psType, &sIndex, pcExpression);
		}
		else
		{
			return Error(pcExpression, "A subscript needs a pointer or an array and an integer");
		}
		if (!psType || psType->psBase->IsVoid())
		{
			return Error(pcExpression, "A void pointer cannot be subscripted");
		}
		psPlace->eKind = CPP65PK_Address;
		psPlace->psType = psType->psBase;
		psPlace->iOffset = 0;
		psPlace->lliName = -1;
		psPlace->iString = -1;
		psPlace->psClass = NULL;
		return true;

	case CPPS_CallExpression:
		//A function that returns a reference.
		return ResultPlace(Evaluate(pcExpression), psPlace, pcExpression);

	default:
		break;
	}
	return Error(pcExpression, "This lvalue is not supported by the 65C64 generator yet");
}


//////////////////////////////////////////////////////////////////////////
//
// The place a reference left in A refers to.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ResultPlace(SCPP65Type* psType, SCPP65Place* psPlace, CCPPSyntax* pcNode)
{
	if (!psType)
	{
		return false;
	}
	if (!psType->IsReference())
	{
		return Error(pcNode, "This is not an lvalue");
	}
	psPlace->eKind = CPP65PK_Address;
	psPlace->psType = psType->psBase;
	psPlace->iOffset = 0;
	psPlace->lliName = -1;
	psPlace->iString = -1;
	psPlace->psClass = NULL;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A reference's place holds an address: the place becomes what it refers to, with
// the address in A.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::DerefPlace(SCPP65Place* psPlace)
{
	SCPP65Type*	psBase;

	psBase = psPlace->psType->psBase;
	Load(psPlace, "A", "A");
	psPlace->eKind = CPP65PK_Address;
	psPlace->psType = psBase;
	psPlace->iOffset = 0;
	psPlace->lliName = -1;
	psPlace->iString = -1;
	psPlace->psClass = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// V points just below the frame.  Locals are at the bottom of the frame, then the
// caller's V, the return address and the arguments:
//
//	V + 1						locals
//	V + 1 + frame				the caller's V
//	V + 1 + frame + 4			the return address
//	V + 1 + frame + 8 + n		the argument n bytes after the first
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::VariablePlace(SCPP65Variable* psVariable, SCPP65Place* psPlace)
{
	psPlace->psType = psVariable->psType;
	psPlace->lliName = -1;
	psPlace->iString = -1;
	psPlace->psClass = psVariable->psClass;
	switch (psVariable->eStorage)
	{
	case CPP65S_Local:
		psPlace->eKind = CPP65PK_Frame;
		psPlace->iOffset = psVariable->iOffset;
		break;
	case CPP65S_Parameter:
		psPlace->eKind = CPP65PK_Frame;
		psPlace->iOffset = miFrameBytes + 8 + psVariable->iOffset;
		break;
	case CPP65S_Static:
		psPlace->eKind = CPP65PK_Global;
		psPlace->iOffset = 0;
		psPlace->lliName = psVariable->pcSymbol->mlliName;
		psPlace->iString = psVariable->iOffset;
		break;
	default:
		psPlace->eKind = CPP65PK_Global;
		psPlace->iOffset = 0;
		psPlace->lliName = psVariable->pcSymbol->mlliName;
		psVariable->bUsed = true;
		break;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The memory operand for a place.  szAddress is the register holding the address
// of an Address place.  X must already hold this for a This place.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendPlace(CChars* psz, SCPP65Place* psPlace, const char* szAddress)
{
	psz->Append('[');
	switch (psPlace->eKind)
	{
	case CPP65PK_Frame:
		psz->Append(1 + psPlace->iOffset);
		psz->Append(" + V");
		break;
	case CPP65PK_Global:
		AppendPlaceLabel(psz, psPlace);
		if (psPlace->iOffset != 0)
		{
			psz->Append(" + ");
			psz->Append(psPlace->iOffset);
		}
		break;
	case CPP65PK_This:
		psz->Append(psPlace->iOffset);
		psz->Append(" + X");
		break;
	default:
		if (psPlace->iOffset != 0)
		{
			psz->Append(psPlace->iOffset);
			psz->Append(" + ");
		}
		psz->Append(szAddress);
		break;
	}
	psz->Append(']');
}


//////////////////////////////////////////////////////////////////////////
//
// Loads a place into a register, sign or zero extended to 32 bits.  Narrow values
// are read through Y; 8 byte values with the register 64 bits wide (it is left 32
// bits wide, holding all 64).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::Load(SCPP65Place* psPlace, const char* szRegister, const char* szAddress)
{
	int		iBytes;

	if (psPlace->eKind == CPP65PK_This)
	{
		EnsureThis();
	}
	iBytes = psPlace->psType->iBytes;
	if (iBytes == 8)
	{
		Line("wth ")->Append(szRegister)->Append(", 8")->AppendNewLine();
		Line("cpy ")->Append(szRegister)->Append(", ");
		AppendPlace(mpszOut, psPlace, szAddress);
		mpszOut->AppendNewLine();
		Line("wth ")->Append(szRegister)->Append(", 4")->AppendNewLine();
		return;
	}
	if (iBytes >= 4)
	{
		Line("cpy ")->Append(szRegister)->Append(", ");
		AppendPlace(mpszOut, psPlace, szAddress);
		mpszOut->AppendNewLine();
		return;
	}

	Line("wth Y, ")->Append(iBytes)->AppendNewLine();
	Line("cpy Y, ");
	AppendPlace(mpszOut, psPlace, szAddress);
	mpszOut->AppendNewLine();
	Line("cpy ")->Append(szRegister)->Append(psPlace->psType->bSigned ? ", Y:s" : ", Y")->AppendNewLine();
	Line("wth Y, 4")->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// Stores a register to a place.  Narrow values are written through Y.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::Store(SCPP65Place* psPlace, const char* szRegister, const char* szAddress)
{
	int		iBytes;

	if (psPlace->eKind == CPP65PK_This)
	{
		EnsureThis();
	}
	iBytes = psPlace->psType->iBytes;
	if (iBytes == 8)
	{
		Line("wth ")->Append(szRegister)->Append(", 8")->AppendNewLine();
		Line("cpy ");
		AppendPlace(mpszOut, psPlace, szAddress);
		mpszOut->Append(", ")->Append(szRegister)->AppendNewLine();
		Line("wth ")->Append(szRegister)->Append(", 4")->AppendNewLine();
		return;
	}
	if (iBytes >= 4)
	{
		Line("cpy ");
		AppendPlace(mpszOut, psPlace, szAddress);
		mpszOut->Append(", ")->Append(szRegister)->AppendNewLine();
		return;
	}

	Line("wth Y, ")->Append(iBytes)->AppendNewLine();
	Line("cpy Y, ")->Append(szRegister)->AppendNewLine();
	Line("cpy ");
	AppendPlace(mpszOut, psPlace, szAddress);
	mpszOut->Append(", Y")->AppendNewLine();
	Line("wth Y, 4")->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// Puts the address of a place in A.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::LoadAddress(SCPP65Place* psPlace)
{
	switch (psPlace->eKind)
	{
	case CPP65PK_Frame:
		Line("cpy A, V")->AppendNewLine();
		Line("add A, ")->Append(1 + psPlace->iOffset)->AppendNewLine();
		break;
	case CPP65PK_This:
		EnsureThis();
		Line("cpy A, X")->AppendNewLine();
		if (psPlace->iOffset != 0)
		{
			Line("add A, ")->Append(psPlace->iOffset)->AppendNewLine();
		}
		break;
	case CPP65PK_Global:
		Line("cpy A, ");
		AppendPlaceLabel(mpszOut, psPlace);
		if (psPlace->iOffset != 0)
		{
			mpszOut->Append(" + ")->Append(psPlace->iOffset);
		}
		mpszOut->AppendNewLine();
		break;
	default:
		if (psPlace->iOffset != 0)
		{
			Line("add A, ")->Append(psPlace->iOffset)->AppendNewLine();
		}
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::Push(const char* szRegister)
{
	Line("psh ")->Append(szRegister)->AppendNewLine();
	miDepth += 4;
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes A at the size of psType.  Narrow values are pushed from Y.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::PushValue(SCPP65Type* psType)
{
	if (psType->iBytes == 8)
	{
		PushWide("A", true);
		return;
	}
	if (psType->iBytes >= 4)
	{
		Push("A");
		return;
	}
	Line("wth Y, ")->Append(psType->iBytes)->AppendNewLine();
	Line("cpy Y, A")->AppendNewLine();
	Line("psh Y")->AppendNewLine();
	Line("wth Y, 4")->AppendNewLine();
	miDepth += psType->iBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes or pulls a register at 8 bytes when bWide, or 4.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::PushWide(const char* szRegister, bool bWide)
{
	if (!bWide)
	{
		Push(szRegister);
		return;
	}
	Line("wth ")->Append(szRegister)->Append(", 8")->AppendNewLine();
	Line("psh ")->Append(szRegister)->AppendNewLine();
	Line("wth ")->Append(szRegister)->Append(", 4")->AppendNewLine();
	miDepth += 8;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::PullWide(const char* szRegister, bool bWide)
{
	if (!bWide)
	{
		Pull(szRegister);
		return;
	}
	Line("wth ")->Append(szRegister)->Append(", 8")->AppendNewLine();
	Line("pul ")->Append(szRegister)->AppendNewLine();
	Line("wth ")->Append(szRegister)->Append(", 4")->AppendNewLine();
	miDepth -= 8;
}


//////////////////////////////////////////////////////////////////////////
//
// Widens a register holding a 32 bit (or narrower, extended) integer to 64 bits,
// sign extended if bSigned.  The register is left 32 bits wide holding all 64.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::Widen(const char* szRegister, bool bSigned)
{
	Line("wthx ")->Append(szRegister)->Append(bSigned ? ", 8:s" : ", 8")->AppendNewLine();
	Line("wth ")->Append(szRegister)->Append(", 4")->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// mul and div count their steps in W, from 0.  W points at the pseudo registers, so
// it is kept on the stack.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::Multiply(const char* szInstruction)
{
	Push("W");
	Line("cpy W, 0")->AppendNewLine();
	Line(szInstruction)->AppendNewLine();
	Pull("W");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::Pull(const char* szRegister)
{
	Line("pul ")->Append(szRegister)->AppendNewLine();
	miDepth -= 4;
}


//////////////////////////////////////////////////////////////////////////
//
// Works out an integer constant expression.  The arithmetic is 64 bit.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Constant(CCPPSyntax* pcExpression, int64* plliValue)
{
	CCPPToken*						pcToken;
	CCPPSyntaxUnaryExpression*		pcUnary;
	CCPPSyntaxBinaryExpression*		pcBinary;
	CCPPSyntaxConditionalExpression*	pcConditional;
	CCPPSyntaxSizeofExpression*		pcSizeof;
	CCPPSyntaxCastExpression*		pcCast;
	CCPPSyntaxIdExpression*			pcId;
	CCPPSyntax*						pcOperand;
	CCPPSymbol*						pcSymbol;
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	SCPP65Variable*					psVariable;
	SCPP65Type*						psType;
	SCPP65Binding*					psBinding;
	int64							lliLeft;
	int64							lliRight;

	pcExpression = SkipParentheses(pcExpression);
	if (!pcExpression)
	{
		return false;
	}

	switch (pcExpression->GetKind())
	{
	case CPPS_LiteralExpression:
		pcToken = mpcTokeniser->Get(((CCPPSyntaxLiteralExpression*)pcExpression)->miLiteral);
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
		*plliValue = 0;
		return true;

	case CPPS_IdExpression:
		pcId = (CCPPSyntaxIdExpression*)pcExpression;
		pcSymbol = pcId->mpcName->mpcSymbol;
		if (!pcSymbol)
		{
			return false;
		}
		if (pcSymbol->meKind == CPPSK_Enumerator)
		{
			return EnumeratorValue(pcSymbol, plliValue);
		}
		if (pcSymbol->meKind == CPPSK_ValueParameter)
		{
			psBinding = FindBinding(pcSymbol);
			if (!psBinding)
			{
				return false;
			}
			*plliValue = psBinding->lliValue;
			return true;
		}

		//A const integer initialised with a constant.
		pcInit = (CCPPSyntaxInitDeclarator*)pcSymbol->mpcDeclaration;
		if ((pcSymbol->meKind != CPPSK_Variable) || !pcInit || (pcInit->GetKind() != CPPS_InitDeclarator) || !pcInit->mpcInitializer)
		{
			return false;
		}
		pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcInit->mpcParent;
		if (!pcDeclaration || (pcDeclaration->GetKind() != CPPS_SimpleDeclaration) || !(pcDeclaration->mpcSpecifiers->muiFlags & (CPPDS_CONST | CPPDS_CONSTEXPR)))
		{
			return false;
		}
		psVariable = GetVariable(pcSymbol);
		if (psVariable && !psVariable->psType->IsInteger())
		{
			return false;
		}
		return Constant(pcInit->mpcInitializer, plliValue);

	case CPPS_UnaryExpression:
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		if (!Constant(pcUnary->mpcOperand, &lliLeft))
		{
			return false;
		}
		switch (pcUnary->meOperator)
		{
		case CT_Add:			*plliValue = lliLeft;			return true;
		case CT_Subtract:		*plliValue = -lliLeft;			return true;
		case CT_BitwiseNot:		*plliValue = ~lliLeft;			return true;
		case CT_LogicalNot:		*plliValue = !lliLeft;			return true;
		default:				return false;
		}

	case CPPS_BinaryExpression:
		pcBinary = (CCPPSyntaxBinaryExpression*)pcExpression;
		if (!Constant(pcBinary->mpcLeft, &lliLeft) || !Constant(pcBinary->mpcRight, &lliRight))
		{
			return false;
		}
		switch (pcBinary->meOperator)
		{
		case CT_Add:				*plliValue = lliLeft + lliRight;		return true;
		case CT_Subtract:			*plliValue = lliLeft - lliRight;		return true;
		case CT_Multiply:			*plliValue = lliLeft * lliRight;		return true;
		case CT_Divide:				if (lliRight == 0) return false;	*plliValue = lliLeft / lliRight;	return true;
		case CT_Modulus:			if (lliRight == 0) return false;	*plliValue = lliLeft % lliRight;	return true;
		case CT_LeftShift:			*plliValue = lliLeft << lliRight;		return true;
		case CT_RightShift:			*plliValue = lliLeft >> lliRight;		return true;
		case CT_BitwiseAnd:			*plliValue = lliLeft & lliRight;		return true;
		case CT_BitwiseOr:			*plliValue = lliLeft | lliRight;		return true;
		case CT_BitwiseXor:			*plliValue = lliLeft ^ lliRight;		return true;
		case CT_LogicalAnd:			*plliValue = lliLeft && lliRight;		return true;
		case CT_LogicalOr:			*plliValue = lliLeft || lliRight;		return true;
		case CT_EqualTo:			*plliValue = lliLeft == lliRight;		return true;
		case CT_NotEqualTo:			*plliValue = lliLeft != lliRight;		return true;
		case CT_LessThan:			*plliValue = lliLeft < lliRight;		return true;
		case CT_LessThanEqualTo:	*plliValue = lliLeft <= lliRight;		return true;
		case CT_GreaterThan:		*plliValue = lliLeft > lliRight;		return true;
		case CT_GreaterThanEqualTo:	*plliValue = lliLeft >= lliRight;		return true;
		default:					return false;
		}

	case CPPS_ConditionalExpression:
		pcConditional = (CCPPSyntaxConditionalExpression*)pcExpression;
		if (!Constant(pcConditional->mpcCondition, &lliLeft))
		{
			return false;
		}
		return Constant(lliLeft ? pcConditional->mpcTrue : pcConditional->mpcFalse, plliValue);

	case CPPS_CastExpression:
		pcCast = (CCPPSyntaxCastExpression*)pcExpression;
		psType = GetTypeIdType(pcCast->mpcType);
		if (!psType || !(psType->IsInteger() || psType->IsPointer()) || !Constant(pcCast->mpcOperand, &lliLeft))
		{
			return false;
		}
		switch (psType->iBytes)
		{
		case 1:		*plliValue = psType->bSigned ? (int64)(int8)lliLeft : (int64)(uint8)lliLeft;		break;
		case 2:		*plliValue = psType->bSigned ? (int64)(int16)lliLeft : (int64)(uint16)lliLeft;	break;
		default:	*plliValue = psType->bSigned ? (int64)(int32)lliLeft : (int64)(uint32)lliLeft;	break;
		}
		return true;

	case CPPS_TypeTraitExpression:
		return TypeTrait((CCPPSyntaxTypeTraitExpression*)pcExpression, plliValue);

	case CPPS_SizeofExpression:
		pcSizeof = (CCPPSyntaxSizeofExpression*)pcExpression;
		if (pcSizeof->meKeyword != CT_sizeof)
		{
			return false;
		}
		if (pcSizeof->mpcType)
		{
			psType = GetTypeIdType(pcSizeof->mpcType);
		}
		else
		{
			//sizeof an array variable is the array's size, not a pointer's.
			pcOperand = SkipParentheses(pcSizeof->mpcOperand);
			psType = NULL;
			if (pcOperand && (pcOperand->GetKind() == CPPS_IdExpression) && ((CCPPSyntaxIdExpression*)pcOperand)->mpcName->mpcSymbol)
			{
				psVariable = GetVariable(((CCPPSyntaxIdExpression*)pcOperand)->mpcName->mpcSymbol);
				psType = psVariable ? psVariable->psType : NULL;
			}
			if (!psType)
			{
				psType = TypeOf(pcOperand);
			}
		}
		if (!psType || psType->IsVoid() || psType->IsFunction())
		{
			return false;
		}
		if (psType->IsReference())
		{
			psType = psType->psBase;
		}
		if (psType->IsClass() && !CompleteClass(psType->psClass, pcExpression))
		{
			return false;
		}
		*plliValue = psType->iBytes;
		return true;

	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The compiler's type traits: __is_class, __is_enum, __is_union, __is_empty,
// __is_polymorphic, __is_abstract, __is_pod, __is_trivial, __is_trivially_copyable,
// __is_trivially_constructible, __is_trivially_destructible and
// __has_trivial_destructor of one type.  Enums are ints.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::TypeTrait(CCPPSyntaxTypeTraitExpression* pcTrait, int64* plliValue)
{
	SCPP65Type*		psType;
	SCPP65Class*	psClass;
	char*			szName;
	CChars			szTrait;
	bool			bResult;

	if ((pcTrait->macpArguments.NumElements() < 1) || (pcTrait->macpArguments.GetPtr(0)->GetKind() != CPPS_TypeId))
	{
		return false;
	}
	psType = GetTypeIdType((CCPPSyntaxTypeId*)pcTrait->macpArguments.GetPtr(0));
	if (!psType)
	{
		return false;
	}
	psClass = psType->IsClass() ? psType->psClass : NULL;
	if (psClass && !CompleteClass(psClass, pcTrait))
	{
		return false;
	}

	szTrait.Init();
	mpcTokeniser->GetSource(mpcTokeniser->Get(pcTrait->miName))->Append(&szTrait);
	szName = szTrait.Text();
	bResult = true;
	if (strcmp(szName, "__is_class") == 0)
	{
		*plliValue = psClass && !psClass->bUnion;
	}
	else if (strcmp(szName, "__is_union") == 0)
	{
		*plliValue = psClass && psClass->bUnion;
	}
	else if (strcmp(szName, "__is_enum") == 0)
	{
		*plliValue = 0;
	}
	else if (strcmp(szName, "__is_empty") == 0)
	{
		*plliValue = psClass && psClass->bEmpty;
	}
	else if ((strcmp(szName, "__is_polymorphic") == 0) || (strcmp(szName, "__is_abstract") == 0))
	{
		*plliValue = psClass && IsPolymorphic(psClass);
	}
	else if ((strcmp(szName, "__is_trivially_constructible") == 0) || (strcmp(szName, "__has_trivial_constructor") == 0))
	{
		*plliValue = !psClass || !NeedsConstructor(psClass);
	}
	else if ((strcmp(szName, "__is_trivially_destructible") == 0) || (strcmp(szName, "__has_trivial_destructor") == 0))
	{
		*plliValue = !psClass || !psClass->psDestructor;
	}
	else if ((strcmp(szName, "__is_pod") == 0) || (strcmp(szName, "__is_trivial") == 0) || (strcmp(szName, "__is_trivially_copyable") == 0))
	{
		*plliValue = !psClass || (!NeedsConstructor(psClass) && !psClass->psDestructor);
	}
	else
	{
		bResult = false;
	}
	szTrait.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// An enumerator is one more than the one before it unless it is given a value.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::EnumeratorValue(CCPPSymbol* pcSymbol, int64* plliValue)
{
	CCPPSyntaxEnumerator*		pcEnumerator;
	CCPPSyntaxEnumerator*		pcOther;
	CCPPSyntaxEnumSpecifier*	pcEnum;
	size						i;
	int64						lliValue;

	pcEnumerator = (CCPPSyntaxEnumerator*)pcSymbol->mpcDeclaration;
	if (!pcEnumerator || !pcEnumerator->mpcParent || (pcEnumerator->mpcParent->GetKind() != CPPS_EnumSpecifier))
	{
		return false;
	}
	pcEnum = (CCPPSyntaxEnumSpecifier*)pcEnumerator->mpcParent;

	lliValue = 0;
	for (i = 0; i < pcEnum->macpEnumerators.NumElements(); i++)
	{
		pcOther = (CCPPSyntaxEnumerator*)pcEnum->macpEnumerators.GetPtr(i);
		if (pcOther->mpcValue && !Constant(pcOther->mpcValue, &lliValue))
		{
			return false;
		}
		if (pcOther == pcEnumerator)
		{
			*plliValue = lliValue;
			return true;
		}
		lliValue++;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A constant or a scalar variable: loading it only changes A and Y.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsSimple(CCPPSyntax* pcExpression)
{
	CCPPSymbol*			pcSymbol;
	SCPP65Variable*		psVariable;
	SCPP65Field*		psField;
	CCPPToken*			pcToken;
	int					iOffset;

	pcExpression = SkipParentheses(pcExpression);
	if (!pcExpression)
	{
		return false;
	}
	if (pcExpression->GetKind() == CPPS_LiteralExpression)
	{
		pcToken = mpcTokeniser->Get(((CCPPSyntaxLiteralExpression*)pcExpression)->miLiteral);
		return !pcToken->IsNumber() || (((CCTNumber*)pcToken)->meType == CTNT_Int) || (((CCTNumber*)pcToken)->meType == CTNT_Short) || (((CCTNumber*)pcToken)->meType == CTNT_LongLong);
	}
	if (pcExpression->GetKind() == CPPS_IdExpression)
	{
		pcSymbol = ((CCPPSyntaxIdExpression*)pcExpression)->mpcName->mpcSymbol;
		if (!pcSymbol)
		{
			return false;
		}
		if (pcSymbol->meKind == CPPSK_Enumerator)
		{
			return true;
		}
		psVariable = GetVariable(pcSymbol);
		if (psVariable)
		{
			return psVariable->psType->IsScalar();
		}
		psField = ((pcSymbol->meKind == CPPSK_Field) && mpsFunction && mpsFunction->psClass && (miThisOffset != -1)) ? FindFieldSymbol(mpsFunction->psClass, pcSymbol, &iOffset) : NULL;
		return psField && psField->psType->IsScalar();
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether working out an expression calls a function.  Runtime routines do not use
// the pseudo registers so they don't count.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ContainsCall(CCPPSyntax* pcNode)
{
	size	i;

	if (!pcNode)
	{
		return false;
	}
	if (pcNode->GetKind() == CPPS_CallExpression)
	{
		return true;
	}
	for (i = 0; i < pcNode->NumChildren(); i++)
	{
		if (ContainsCall(pcNode->GetChild(i)))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPP65Generator::SkipParentheses(CCPPSyntax* pcExpression)
{
	while (pcExpression && (pcExpression->GetKind() == CPPS_ParenthesisedExpression))
	{
		pcExpression = ((CCPPSyntaxParenthesisedExpression*)pcExpression)->mpcExpression;
	}
	return pcExpression;
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes of a narrow string literal, without the zero at the end.  Adjacent
// literals are joined.  MSVC's __FUNCTION__, __FUNCSIG__ and __FUNCDNAME__ can be
// parts; they are the function's name, its class and name, and its label.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::DecodeString(CCPPSyntax* pcString, CArrayUint8* pauiBytes)
{
	CCPPToken*	pcToken;
	int			iToken;
	CChars		szName;
	char*		szIdentifier;
	int			iLength;
	size		i;

	for (iToken = pcString->miFirstToken; iToken <= pcString->miLastToken; iToken++)
	{
		pcToken = mpcTokeniser->Get(iToken);
		if (pcToken->IsString())
		{
			if (!DecodeStringToken(pcString, (CCTString*)pcToken, pauiBytes))
			{
				return false;
			}
			continue;
		}

		szIdentifier = pcToken->IsIdentifier() ? mpcTokeniser->GetIdentifier(((CCTIdentifier*)pcToken)->mlliIdentifier, &iLength) : NULL;
		szName.Init();
		if (szIdentifier && (iLength == 11) && (memcmp(szIdentifier, "__FUNCDNAME", 11) == 0))
		{
			if (mpsFunction)
			{
				AppendFunctionLabel(&szName, mpsFunction);
			}
		}
		else if (!szIdentifier || !FunctionNameText(((CCTIdentifier*)pcToken)->mlliIdentifier, &szName))
		{
			szName.Kill();
			return Error(pcString, "This part of a string literal is not supported by the 65C64 generator yet");
		}
		for (i = 0; i < szName.Length(); i++)
		{
			pauiBytes->Add((uint8)szName.GetChar(i));
		}
		szName.Kill();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes of one narrow string literal token.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::DecodeStringToken(CCPPSyntax* pcString, CCTString* pcToken, CArrayUint8* pauiBytes)
{
	const char*		sz;
	const char*		szEnd;
	uint32			uiValue;
	int				iDigits;

	if ((pcToken->meEncoding != CTE_Narrow) && (pcToken->meEncoding != CTE_UTF8))
	{
		return Error(pcString, "Wide string literals are not supported by the 65C64 generator yet");
	}

	sz = pcToken->mszStart;
	while (*sz != '"')
	{
		if (*sz == 'R')
		{
			return Error(pcString, "Raw string literals are not supported by the 65C64 generator yet");
		}
		sz++;
	}
	sz++;
	szEnd = pcToken->mszEnd;

	while (sz < szEnd)
	{
		if (*sz != '\\')
		{
			pauiBytes->Add((uint8)*sz);
			sz++;
			continue;
		}

		sz++;
		switch (*sz)
		{
		case 'n':	uiValue = '\n';		sz++;	break;
		case 't':	uiValue = '\t';		sz++;	break;
		case 'r':	uiValue = '\r';		sz++;	break;
		case 'a':	uiValue = '\a';		sz++;	break;
		case 'b':	uiValue = '\b';		sz++;	break;
		case 'f':	uiValue = '\f';		sz++;	break;
		case 'v':	uiValue = '\v';		sz++;	break;
		case 'x':
			sz++;
			uiValue = 0;
			while (sz < szEnd)
			{
				if ((*sz >= '0') && (*sz <= '9'))			uiValue = uiValue * 16 + (*sz - '0');
				else if ((*sz >= 'a') && (*sz <= 'f'))		uiValue = uiValue * 16 + (*sz - 'a' + 10);
				else if ((*sz >= 'A') && (*sz <= 'F'))		uiValue = uiValue * 16 + (*sz - 'A' + 10);
				else										break;
				sz++;
			}
			break;
		default:
			if ((*sz >= '0') && (*sz <= '7'))
			{
				uiValue = 0;
				for (iDigits = 0; (iDigits < 3) && (sz < szEnd) && (*sz >= '0') && (*sz <= '7'); iDigits++)
				{
					uiValue = uiValue * 8 + (*sz - '0');
					sz++;
				}
			}
			else
			{
				//\\ \' \" \? and anything else stand for themselves.
				uiValue = (uint8)*sz;
				sz++;
			}
			break;
		}
		pauiBytes->Add((uint8)uiValue);
	}
	return true;
}
