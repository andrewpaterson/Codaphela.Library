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
#ifndef __C_P_P_65_TYPE_CHECKER_H__
#define __C_P_P_65_TYPE_CHECKER_H__
#include "CPPSyntaxVisitor.h"
#include "CPPSyntaxes.h"
#include "BaseLib/MapLongTemplate.h"
#include "BaseLib/HashMapPrimitiveTemplate.h"
#include "BaseLib/MemoryStackExtended.h"
#include "CPP65Type.h"


class CCPPUnit;
class CCPPDiagnostics;
struct SCPPField;
class CCPPClass;
class CCPPFunction;
class CCPPTemplateArguments;
class CCPPErrors;


//What an expression is ([basic.lval]): an lvalue (a place that stays), an xvalue (a place about to go), or a
//prvalue (a value), a class's being the object it makes.
enum ECPPValueCategory
{
	CPPVC_Lvalue,
	CPPVC_Xvalue,
	CPPVC_Prvalue,
	CPPVC_ClassPrvalue,
};


//What is known of an expression (CCPP65TypeChecker::Facts), worked out once for the bindings and the function it is
//worked out in, each part when it is first asked for: its type, the type it is declared to give, and what was said
//working them out; and the type of its place where it is evaluated.  SemanticLayerPlan.txt stage 15e: its category
//joins them.
struct SCPPExpressionFacts
{
	CCPPTemplateArguments*	pcBindings;
	CCPPFunction*			pcFunction;
	bool					bTyped;			//psType, psDeclared and pcErrors are known.
	SCPP65Type*				psType;
	SCPP65Type*				psDeclared;		//psType, or the reference to it the expression is declared to give (f() of T& f(), a cast to T&&).
	CCPPErrors*				pcErrors;		//What was said working it out (NULL for nothing), which ReportTypeOf says: the diagnostics' to free.
	bool					bPlaced;		//psPlace is known.
	SCPP65Type*				psPlace;		//Its place's type (Place), NULL for an expression that has none.
	bool					bCategorised;	//eCategory is known.
	ECPPValueCategory		eCategory;
};


typedef CHashMapLongTemplate<SCPPExpressionFacts*>	CMapCPPExpressionFacts;


//The type of a C++ expression, and what is wrong with it, worked out without writing any code: the generator's
//TypeOf, PlaceTypeOf and ReportTypeOf.  It walks an expression the way CCPP65IRFrontEnd writes it, and its functions
//have the IR front end's names (Value, Place, CallExpression, Construct ...), so an expression's type and its code are
//found in the same place in both.  An error is reported with the generator's Error (in quiet mode it is only noted),
//and the function that found it returns NULL (or false); so do the functions that called it.
class CCPP65TypeChecker : public CCPPSyntaxVisitor
{
protected:
	CCPPUnit*			mpcUnit;
	CCPPDiagnostics*	mpcDiagnostics;	//Where its errors go: the unit's.
	CCPP65Types*		mpcTypes;
	CArraySyntax		mapcNone;		//No arguments.
	CMapCPPExpressionFacts	mcFacts;	//What is known of each expression, by its node (Facts)...
	CMemoryStackExtended	mcFactsMemory;	//...kept here, where it doesn't move.

	SCPPExpressionFacts*	FactsEntry(CCPPSyntax* pcExpression);
	ECPPValueCategory	WorkOutCategory(CCPPSyntax* pcExpression);
	ECPPValueCategory	DeclaredCategory(CCPPSyntax* pcExpression);
	bool				IsOperatorCall(CCPPSyntax* pcExpression);
	SCPP65Type*			mpsResult;		//What the Visit function of the expression Result was asked for found.

public:
	bool			CanInitialise(SCPP65Type* psType, CCPPSyntax* pcInitialiser);
	const char*		NotAPlace(CCPPSyntaxName* pcName);
	void			VisitDefault(CCPPSyntax* pcNode) override;
	void			VisitLiteralExpression(CCPPSyntaxLiteralExpression* pcNode) override;
	void			VisitStringExpression(CCPPSyntaxStringExpression* pcNode) override;
	void			VisitIdExpression(CCPPSyntaxIdExpression* pcNode) override;
	void			VisitParenthesisedExpression(CCPPSyntaxParenthesisedExpression* pcNode) override;
	void			VisitUnaryExpression(CCPPSyntaxUnaryExpression* pcNode) override;
	void			VisitBinaryExpression(CCPPSyntaxBinaryExpression* pcNode) override;
	void			VisitConditionalExpression(CCPPSyntaxConditionalExpression* pcNode) override;
	void			VisitCallExpression(CCPPSyntaxCallExpression* pcNode) override;
	void			VisitSubscriptExpression(CCPPSyntaxSubscriptExpression* pcNode) override;
	void			VisitMemberExpression(CCPPSyntaxMemberExpression* pcNode) override;
	void			VisitThisExpression(CCPPSyntaxThisExpression* pcNode) override;
	void			VisitLambdaExpression(CCPPSyntaxLambdaExpression* pcNode) override;
	void			VisitCastExpression(CCPPSyntaxCastExpression* pcNode) override;
	void			VisitNamedCastExpression(CCPPSyntaxNamedCastExpression* pcNode) override;
	void			VisitConstructExpression(CCPPSyntaxConstructExpression* pcNode) override;
	void			VisitSizeofExpression(CCPPSyntaxSizeofExpression* pcNode) override;
	void			VisitImplicitArgument(CCPPSyntaxImplicitArgument* pcNode) override;
	void			VisitTypeTraitExpression(CCPPSyntaxTypeTraitExpression* pcNode) override;
	void			VisitNoexceptExpression(CCPPSyntaxNoexceptExpression* pcNode) override;
	void			VisitThrowExpression(CCPPSyntaxThrowExpression* pcNode) override;
	void			VisitTypeidExpression(CCPPSyntaxTypeidExpression* pcNode) override;
	void			VisitNewExpression(CCPPSyntaxNewExpression* pcNode) override;
	void			VisitDeleteExpression(CCPPSyntaxDeleteExpression* pcNode) override;

	void			Init(CCPPUnit* pcUnit);
	void			Kill(void);

	SCPPExpressionFacts*	Facts(CCPPSyntax* pcExpression);
	SCPP65Type*		PlaceFact(CCPPSyntax* pcExpression);
	ECPPValueCategory	Category(CCPPSyntax* pcExpression);
	SCPP65Type*		Value(CCPPSyntax* pcExpression, SCPP65Type** ppsDeclared = NULL);
	SCPP65Type*		ValueFor(CCPPSyntax* pcExpression, SCPP65Type* psTo, bool bExplicit);
	SCPP65Type*		Place(CCPPSyntax* pcExpression);
	bool			Convert(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode);
	bool			ImplicitConvert(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode, CCPPSyntax* pcValue = NULL);
	bool			KeepsNullPointer(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode, CCPPSyntax* pcValue);
	bool			KeepsEnum(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode);
	bool			Truth(SCPP65Type* psType, CCPPSyntax* pcNode);
	bool			RecordsEnumOperands(CCPPSyntax* pcNode, SCPP65Type* psLeft, SCPP65Type* psRight, uint8 uiKind);
	bool			CanBind(SCPP65Type* psReference, SCPP65Type* psPlace, CCPPSyntax* pcNode);
	bool			ConvertsPointer(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode, CCPPSyntax* pcValue);
	bool			ConversionError(CCPPSyntax* pcNode, SCPP65Type* psFrom, SCPP65Type* psTo);
	bool			TypesError(CCPPSyntax* pcNode, const char* szBefore, SCPP65Type* psFirst, const char* szBetween, SCPP65Type* psSecond, const char* szAfter);
	bool			KeepsPointerQualifiers(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode);
	bool			LosesPointerQualifiers(SCPP65Type* psFrom, SCPP65Type* psTo);
	bool			NamedCastAllowed(CCPPSyntaxNamedCastExpression* pcCast, SCPP65Type* psTo);
	bool			ConstCastAllowed(CCPPSyntaxNamedCastExpression* pcCast, SCPP65Type* psFrom, SCPP65Type* psTo);
	bool			StaticCastAllowed(CCPPSyntaxNamedCastExpression* pcCast, SCPP65Type* psFrom, SCPP65Type* psTo);
	bool			DynamicCastAllowed(CCPPSyntaxNamedCastExpression* pcCast, SCPP65Type* psTo);
	bool			CastReachesBase(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode);
	bool			ReinterpretCastAllowed(CCPPSyntaxNamedCastExpression* pcCast, SCPP65Type* psFrom, SCPP65Type* psTo);
	bool			KeepsQualifiers(CCPPSyntax* pcValue, SCPP65Type* psTo);
	bool			NotNarrowed(CCPPSyntax* pcSource, SCPP65Type* psTo);
	bool			ConstantNotNarrowed(CCPPSyntax* pcSource, SCPP65Type* psTo);
	bool			Initialise(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode, bool bBraced = false);

	SCPP65Type*				CompositePointee(SCPP65Type* psLeft, SCPP65Type* psRight, bool bFirstLevel);
	SCPP65Type*				CompositePointerType(SCPP65Type* psLeft, SCPP65Type* psRight);
	SCPP65Type*				ConditionalType(SCPP65Type* psTrue, SCPP65Type* psFalse);
	CCPPClass*				ConditionalClass(SCPP65Type* psTrue, SCPP65Type* psFalse);
	SCPP65Type*				ConditionalLvalueType(CCPPSyntaxConditionalExpression* pcConditional);
	SCPP65Type*				ConditionalXvalueType(CCPPSyntaxConditionalExpression* pcConditional);
	SCPP65Type*				ConditionalObjectType(CCPPSyntaxConditionalExpression* pcConditional, bool* pbAmbiguous);
	SCPP65Type*				VariadicArgumentType(SCPP65Type* psType);
	SCPP65Type*				CalleePointerType(CCPPSyntax* pcCallee);
	SCPP65Type*				TypeOf(CCPPSyntax* pcExpression);
	SCPP65Type*				ValueTypeOf(CCPPSyntax* pcExpression);
	SCPP65Type*				DeclaredTypeOf(CCPPSyntax* pcExpression);
	SCPP65Type*				PlaceTypeOf(CCPPSyntax* pcExpression);
	SCPP65Type*				UnevaluatedTypeOf(CCPPSyntax* pcExpression);
	void					ReportTypeOf(CCPPSyntax* pcExpression);
	bool					IsRvalue(CCPPSyntax* pcExpression);
	bool					IsObjectLvalue(CCPPSyntax* pcExpression);
	bool					MakesObject(CCPPSyntax* pcExpression);
	bool					IsMadeObject(CCPPSyntax* pcExpression, CCPPClass* pcClass);
	bool					IsObject(CCPPSyntax* pcExpression);
	CCPPClass*			ObjectClass(CCPPSyntax* pcExpression);
	SCPP65Type*				PlaceOf(CCPPSyntax* pcExpression);
	CCPPSyntax*				CommaResult(CCPPSyntax* pcExpression);
	bool					IsReturnedLocal(CCPPSyntaxIdExpression* pcId);
	SCPP65Type*				OperandType(SCPP65Type* psType, CCPPSyntax* pcExpression);
	SCPP65Type*				OperandType(CCPPSyntaxTypeId* pcType, CCPPSyntax* pcOperand);
	bool					IsAggregate(SCPP65Type* psType);
	SCPP65Type*				AggregateMember(SCPP65Type* psType, size uiIndex);
	CCPPClass*				TypeInfoClass(void);
	bool					IsTypeInfoMember(CCPPFunction* pcFunction);
	SCPP65Type*				TypeidType(CCPPSyntaxTypeidExpression* pcTypeid, bool* pbDynamic);
	bool					IsConstObject(CCPPSyntax* pcExpression);
	CCPPFunction*			ThisFunction(void);
	bool					IsConstThis(void);
	bool					IsVolatileThis(void);
	SCPP65Type*				ThisType(CCPPClass* pcClass);

protected:
	SCPP65Type*		Result(CCPPSyntax* pcExpression);
	SCPP65Type*		PlaceType(CCPPSyntax* pcExpression);
	void			ObjectChanged(CCPPSyntax* pcObject, CCPPFunction* pcFunction);
	SCPP65Type*		Load(SCPP65Type* psPlace, CCPPSyntax* pcNode);
	SCPP65Type*		ConstantValue(CCPPSyntax* pcExpression);
	SCPP65Type*		StringLiteral(CCPPSyntaxStringExpression* pcString);
	SCPP65Type*		IdExpression(CCPPSyntaxIdExpression* pcId);
	SCPP65Type*		FunctionAddress(CCPPSyntax* pcNode, int64 lliName, SCPP65Type* psTarget);
	SCPP65Type*		FunctionTemplateAddress(CCPPSyntaxIdExpression* pcId);
	bool			MemberAddress(CCPPSyntaxIdExpression* pcId, SCPP65Type* psTarget, SCPP65Type** ppsType);
	SCPP65Type*		ThisExpression(CCPPSyntax* pcThis);

	SCPP65Type*		UnaryExpression(CCPPSyntaxUnaryExpression* pcUnary);
	SCPP65Type*		IncrementExpression(CCPPSyntaxUnaryExpression* pcUnary);
	SCPP65Type*		BinaryExpression(CCPPSyntaxBinaryExpression* pcBinary);
	SCPP65Type*		Assignment(CCPPSyntaxBinaryExpression* pcBinary);
	SCPP65Type*		Operate(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Type* psRight, CCPPSyntax* pcNode);
	bool			Compare(CCPPSyntaxBinaryExpression* pcBinary, SCPP65Type* psLeft, SCPP65Type* psRight);
	bool			CompletePointee(SCPP65Type* psPointer, CCPPSyntax* pcNode);
	bool			BitFieldError(CCPPSyntax* pcNode);
	bool			IsBitField(CCPPSyntax* pcExpression);
	bool			Operands(CCPPSyntax* pcLeft, CCPPSyntax* pcRight, SCPP65Type** ppsLeft, SCPP65Type** ppsRight);
	SCPP65Type*		Operand(CCPPSyntax* pcExpression);
	SCPP65Type*		ConditionalExpression(CCPPSyntaxConditionalExpression* pcConditional);
	bool			ConditionalResultsFit(CCPPSyntaxConditionalExpression* pcConditional, SCPP65Type* psTrue, SCPP65Type* psFalse);
	SCPP65Type*		ConditionalPlace(CCPPSyntaxConditionalExpression* pcConditional);
	SCPP65Type*		ObjectConditional(CCPPSyntaxConditionalExpression* pcConditional);
	SCPP65Type*		ConditionValue(CCPPSyntax* pcExpression);
	bool			Condition(CCPPSyntax* pcExpression);

	SCPP65Type*		CallExpression(CCPPSyntaxCallExpression* pcCall);
	SCPP65Type*		UncheckedCall(CArraySyntax* papcArguments);
	SCPP65Type*		CallThroughPointer(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, SCPP65Type* psFunctionType);
	SCPP65Type*		Call(CCPPFunction* pcFunction, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	bool			Arguments(CArrayCPP65Types* papsParameters, CArraySyntax* papcDefaults, bool bVariadic, CArraySyntax* papcArguments, CCPPSyntax* pcNode, SCPP65Type* psReturn, CCPPTemplateArguments* pcDefaultBindings = NULL);
	bool			ParameterArguments(CArrayCPP65Types* papsParameters, CArraySyntax* papcArguments, size uiFrom, size uiTo, CCPPSyntax* pcNode);
	SCPP65Type*		Builtin(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, int64 lliName);
	SCPP65Type*		InlineMemcpy(CArraySyntax* papcArguments);

	SCPP65Type*		MemberFunctionCall(CCPPSyntaxCallExpression* pcCall, CCPPBoundCall* pcBound);
	SCPP65Type*		LiteralCall(CCPPSyntax* pcLiteral);
	bool			InitializerListElements(SCPP65Type* psType, SCPP65Type* psElement, CArraySyntax* papcElements, CCPPSyntax* pcNode);
	bool			Narrows(CCPPSyntax* pcSource, SCPP65Type* psTo);
	SCPP65Type*		ObjectCall(CCPPSyntax* pcObject, CCPPFunction* pcFunction, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	SCPP65Type*		OperatorCall(CCPPSyntax* pcObject, ECppTokens eOperator, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	SCPP65Type*		OperatorExpression(ECppTokens eOperator, CCPPSyntax* pcLeft, CCPPSyntax* pcRight, CCPPSyntax* pcNode);
	SCPP65Type*		ArrowPointer(CCPPSyntax* pcObject, CCPPSyntax* pcNode);
	SCPP65Type*		MemberPlace(CCPPSyntaxMemberExpression* pcMember);
	SCPP65Type*		MemberPointerObject(CCPPSyntaxBinaryExpression* pcBinary, SCPP65Type** ppsObject);
	SCPP65Type*		MemberPointerPlace(CCPPSyntaxBinaryExpression* pcBinary);
	SCPP65Type*		MemberPointerCall(CCPPSyntaxCallExpression* pcCall, CArraySyntax* papcArguments, CCPPSyntaxBinaryExpression* pcBinary);
	SCPP65Type*		SubscriptPlace(CCPPSyntaxSubscriptExpression* pcSubscript);
	SCPP65Type*		ResultPlace(SCPP65Type* psType, CCPPSyntax* pcNode);

	SCPP65Type*		CastExpression(CCPPSyntaxTypeId* pcTypeId, CCPPSyntax* pcOperand, CCPPSyntax* pcNode);
	bool			BindsCategory(SCPP65Type* psReference, CCPPSyntax* pcValue, CCPPSyntax* pcNode);
	SCPP65Type*		ReferenceCast(SCPP65Type* psTo, CCPPSyntax* pcOperand, CCPPSyntax* pcNode);
	SCPP65Type*		ConstructExpression(CCPPSyntaxConstructExpression* pcConstruct);
	SCPP65Type*		SizeofExpression(CCPPSyntax* pcSizeof);
	void			SizeofError(CCPPSyntax* pcSizeof);
	bool			IsInDefaultArgument(CCPPSyntax* pcNode);
	SCPP65Type*		CheckedSizeofType(CCPPSyntax* pcSizeof);
	SCPP65Type*		NewExpression(CCPPSyntaxNewExpression* pcNew);
	SCPP65Type*		NewArray(CCPPSyntaxNewExpression* pcNew, SCPP65Type* psType);
	bool			NewArrayBound(CCPPSyntax* pcBound);
	bool			HasOperatorFunction(CCPPSyntaxBinaryExpression* pcBinary);
	bool			Operands(CCPPSyntaxBinaryExpression* pcBinary, SCPP65Type** ppsLeft, SCPP65Type** ppsRight);
	SCPP65Type*		BuiltinOperand(CCPPSyntax* pcOperand, ECPPContextualTypes eWanted);
	SCPP65Type*		DeleteExpression(CCPPSyntaxDeleteExpression* pcDelete);

	SCPP65Type*		FieldType(SCPPField* psField, bool bConst, bool bVolatile);
	bool			CanChange(SCPP65Type* psPlace, CCPPSyntax* pcNode);
	SCPP65Type*		TemporaryObject(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode);
	bool			Construct(SCPP65Type* psType, CArraySyntax* papcArguments, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode);
	bool			ConstructClass(CCPPClass* pcClass, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	bool			CopyConstruct(CCPPClass* pcClass, CCPPSyntax* pcSource, CCPPSyntax* pcNode);
	bool			CopyObject(CCPPClass* pcClass, CCPPSyntax* pcSource, CCPPSyntax* pcNode);
	SCPP65Type*		ObjectType(CCPPSyntax* pcExpression);

	bool			Error(CCPPSyntax* pcNode, const char* szMessage);

};


#endif // __C_P_P_65_TYPE_CHECKER_H__
