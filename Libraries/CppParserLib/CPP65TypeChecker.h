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
#ifndef __C_P_P_65_TYPE_CHECKER_H__
#define __C_P_P_65_TYPE_CHECKER_H__
#include "CPPSyntaxes.h"
#include "CPP65Type.h"


class CCPP65Generator;
struct SCPP65Class;
struct SCPP65Function;


//The type of a C++ expression, and what is wrong with it, worked out without writing any code: the generator's
//TypeOf, PlaceTypeOf and ReportTypeOf.  It walks an expression the way CCPP65IRFrontEnd writes it, and its functions
//have the IR front end's names (Value, Place, CallExpression, Construct ...), so an expression's type and its code are
//found in the same place in both.  An error is reported with the generator's Error (in quiet mode it is only noted),
//and the function that found it returns NULL (or false); so do the functions that called it.
class CCPP65TypeChecker
{
protected:
	CCPP65Generator*	mpcGenerator;
	CCPP65Types*		mpcTypes;
	CArraySyntax		mapcNone;		//No arguments.

public:
	void			Init(CCPP65Generator* pcGenerator);
	void			Kill(void);

	SCPP65Type*		Value(CCPPSyntax* pcExpression);
	SCPP65Type*		ValueFor(CCPPSyntax* pcExpression, SCPP65Type* psTo, bool bExplicit);
	SCPP65Type*		Place(CCPPSyntax* pcExpression);
	bool			Convert(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode);

protected:
	SCPP65Type*		Result(CCPPSyntax* pcExpression);
	SCPP65Type*		Load(SCPP65Type* psPlace, CCPPSyntax* pcNode);
	SCPP65Type*		ConstantValue(CCPPSyntax* pcExpression);
	SCPP65Type*		StringLiteral(CCPPSyntaxStringExpression* pcString);
	SCPP65Type*		IdExpression(CCPPSyntaxIdExpression* pcId);
	SCPP65Type*		FunctionAddress(CCPPSyntax* pcNode, int64 lliName, SCPP65Type* psTarget);
	SCPP65Type*		FunctionTemplateAddress(CCPPSyntaxIdExpression* pcId);
	bool			MemberAddress(CCPPSyntaxIdExpression* pcId, SCPP65Type** ppsType);
	bool			MemberFunctionAddress(SCPP65Class* psClass, int64 lliName, SCPP65Type** ppsType, CCPPSyntax* pcNode);
	SCPP65Type*		ThisExpression(CCPPSyntax* pcThis);

	SCPP65Type*		UnaryExpression(CCPPSyntaxUnaryExpression* pcUnary);
	SCPP65Type*		IncrementExpression(CCPPSyntaxUnaryExpression* pcUnary);
	SCPP65Type*		BinaryExpression(CCPPSyntaxBinaryExpression* pcBinary);
	SCPP65Type*		Assignment(CCPPSyntaxBinaryExpression* pcBinary);
	SCPP65Type*		Operate(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Type* psRight, CCPPSyntax* pcNode);
	bool			Compare(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Type* psRight, CCPPSyntax* pcNode);
	bool			CompletePointee(SCPP65Type* psPointer, CCPPSyntax* pcNode);
	bool			Operands(CCPPSyntax* pcLeft, CCPPSyntax* pcRight, SCPP65Type** ppsLeft, SCPP65Type** ppsRight);
	SCPP65Type*		ConditionalExpression(CCPPSyntaxConditionalExpression* pcConditional);
	SCPP65Type*		ObjectConditional(CCPPSyntaxConditionalExpression* pcConditional, SCPP65Type* psTrue, SCPP65Type* psFalse);
	SCPP65Type*		ConditionValue(CCPPSyntax* pcExpression);
	bool			Condition(CCPPSyntax* pcExpression);
	bool			Truth(SCPP65Type* psType, CCPPSyntax* pcNode);

	SCPP65Type*		CallExpression(CCPPSyntaxCallExpression* pcCall);
	SCPP65Type*		ExpandPackCall(CCPPSyntaxCallExpression* pcCall);
	SCPP65Type*		CallThroughPointer(CCPPSyntaxCallExpression* pcCall, SCPP65Type* psFunctionType);
	SCPP65Type*		Call(SCPP65Function* psFunction, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	bool			Arguments(CArrayCPP65Types* papsParameters, CArraySyntax* papcDefaults, bool bVariadic, CArraySyntax* papcArguments, CCPPSyntax* pcNode, SCPP65Type* psReturn);
	SCPP65Type*		Builtin(CCPPSyntaxCallExpression* pcCall, int64 lliName);
	SCPP65Type*		InlineMemcpy(CArraySyntax* papcArguments);

	SCPP65Type*		MemberCall(CCPPSyntaxCallExpression* pcCall, CCPPSyntax* pcCallee);
	SCPP65Type*		MemberFunctionCall(CCPPSyntaxCallExpression* pcCall, CCPPSyntax* pcCallee, CCPPSyntax* pcObject, SCPP65Function* psFunction);
	SCPP65Type*		ObjectCall(CCPPSyntax* pcObject, SCPP65Function* psFunction, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	SCPP65Type*		OperatorCall(CCPPSyntax* pcObject, ECppTokens eOperator, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	SCPP65Type*		OperatorExpression(ECppTokens eOperator, CCPPSyntax* pcLeft, CCPPSyntax* pcRight, CCPPSyntax* pcNode);
	SCPP65Type*		ArrowPointer(CCPPSyntax* pcObject, CCPPSyntax* pcNode);
	SCPP65Type*		MemberPlace(CCPPSyntaxMemberExpression* pcMember);
	SCPP65Type*		MemberPointerObject(CCPPSyntaxBinaryExpression* pcBinary);
	SCPP65Type*		MemberPointerPlace(CCPPSyntaxBinaryExpression* pcBinary);
	SCPP65Type*		MemberPointerCall(CCPPSyntaxCallExpression* pcCall, CCPPSyntaxBinaryExpression* pcBinary);
	SCPP65Type*		SubscriptPlace(CCPPSyntaxSubscriptExpression* pcSubscript);
	SCPP65Type*		ResultPlace(SCPP65Type* psType, CCPPSyntax* pcNode);

	SCPP65Type*		CastExpression(CCPPSyntaxTypeId* pcTypeId, CCPPSyntax* pcOperand, CCPPSyntax* pcNode);
	SCPP65Type*		ConstructExpression(CCPPSyntaxConstructExpression* pcConstruct);
	SCPP65Type*		SizeofExpression(CCPPSyntax* pcSizeof);
	SCPP65Type*		NewExpression(CCPPSyntaxNewExpression* pcNew);
	SCPP65Type*		NewArray(CCPPSyntaxNewExpression* pcNew, SCPP65Type* psType);
	SCPP65Type*		DeleteExpression(CCPPSyntaxDeleteExpression* pcDelete);

	SCPP65Type*		TemporaryObject(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode);
	bool			Construct(SCPP65Type* psType, CArraySyntax* papcArguments, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode);
	bool			ConstructClass(SCPP65Class* psClass, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	bool			CopyConstruct(SCPP65Class* psClass, CCPPSyntax* pcSource, CCPPSyntax* pcNode);
	bool			CopyObject(SCPP65Class* psClass, CCPPSyntax* pcSource, CCPPSyntax* pcNode);
	SCPP65Type*		ObjectType(CCPPSyntax* pcExpression);

	bool			Error(CCPPSyntax* pcNode, const char* szMessage);
};


#endif // __C_P_P_65_TYPE_CHECKER_H__
