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
#include "CPP65IRFrontEnd.h"


//The IR front end's operators: unary, increments, binary, assignments, comparisons, the conditional operator and
//conditions (branched on, or as a value).


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::UnaryExpression(CCPPSyntaxUnaryExpression* pcUnary, SCPP65IRValue* psValue)
{
	SCPP65IRPlace	sPlace;
	SCPP65IRValue	sOperand;
	CCPPBoundName	cBound;
	CCPPBoundCall	cCall;
	CCPPSyntax*		pcOperand;
	SCPP65Type*		psType;
	SIRType			sType;
	bool			bMember;

	switch (pcUnary->meOperator)
	{
	case CT_Increment:
	case CT_Decrement:
		return IncrementExpression(pcUnary, psValue);
	case CT_LogicalNot:
		if (mpcUnit->IsOperatorOperand(pcUnary->mpcOperand, CT_LogicalNot) && mpcUnit->GetBinder()->BindOperator(CT_LogicalNot, pcUnary->mpcOperand, NULL, pcUnary, &cCall, false))
		{
			//!object: its operator!, whatever it returns.
			return OperatorExpression(CT_LogicalNot, pcUnary->mpcOperand, NULL, psValue);
		}
		return ConditionValue(pcUnary, psValue);
	case CT_Multiply:
		if (mpcUnit->IsObject(pcUnary->mpcOperand))
		{
			//*object: the object's operator, and what a reference it returns refers to.
			if (!OperatorExpression(CT_Multiply, pcUnary->mpcOperand, NULL, psValue))
			{
				return false;
			}
			if (!psValue->psType->IsReference())
			{
				return true;
			}
			sPlace.sAddress = psValue->sValue;
			sPlace.psType = psValue->psType->psBase;
			return Load(&sPlace, psValue);
		}
		if (!Place(pcUnary, &sPlace))
		{
			return false;
		}
		return Load(&sPlace, psValue);
	case CT_BitwiseAnd:
		pcOperand = mpcUnit->SkipParentheses(pcUnary->mpcOperand);
		bMember = false;
		if (mpcUnit->MemberPointerName(pcUnary) && MemberAddress((CCPPSyntaxIdExpression*)pcOperand, NULL, psValue, &bMember))
		{
			return true;
		}
		if (bMember)
		{
			return false;
		}
		if (mpcUnit->IsObject(pcOperand) && mpcUnit->HasMember(mpcUnit->ObjectClass(pcOperand), mpcUnit->GetOperatorName(CT_BitwiseAnd, false)))
		{
			//&object with the object's operator&.
			return OperatorExpression(CT_BitwiseAnd, pcOperand, NULL, psValue);
		}
		if ((pcOperand->GetKind() == CPPS_IdExpression) && ((CCPPSyntaxIdExpression*)pcOperand)->mpcName->mpcSymbol && (((CCPPSyntaxIdExpression*)pcOperand)->mpcName->mpcSymbol->meKind == CPPSK_Function))
		{
			return FunctionAddress((CCPPSyntaxIdExpression*)pcOperand, NULL, psValue);
		}
		if ((pcOperand->GetKind() == CPPS_IdExpression) && ((CCPPSyntaxIdExpression*)pcOperand)->mpcName->mpcSymbol && (((CCPPSyntaxIdExpression*)pcOperand)->mpcName->mpcSymbol->meKind == CPPSK_FunctionTemplate))
		{
			return FunctionTemplateAddress((CCPPSyntaxIdExpression*)pcOperand, psValue);
		}
		if ((pcOperand->GetKind() == CPPS_IdExpression) && ((CCPPSyntaxIdExpression*)pcOperand)->mpcName->IsQualifiedByClass())
		{
			//&Class::member of a static data member is its address, a pointer like any variable's ([expr.unary.op] 3),
			//and &(Class::field) the address of this's field.  Anything else a class qualifies that isn't a field or a
			//member function isn't done.
			mpcUnit->GetBinder()->BindName((CCPPSyntaxIdExpression*)pcOperand, &cBound);
			if (((cBound.meWhat != CPPBN_Variable) || !cBound.mpcVariable) && (cBound.meWhat != CPPBN_Field))
			{
				return Unsupported(__FILE__, __LINE__);
			}
		}
		if (!Place(pcOperand, &sPlace))
		{
			return false;
		}
		if (sPlace.IsBitField())
		{
			return BitFieldAddress(__FILE__, __LINE__);
		}
		psValue->sValue = sPlace.sAddress;
		psValue->psType = mpcTypes->Pointer(sPlace.psType);
		return true;
	case CT_Add:
	case CT_Subtract:
	case CT_BitwiseNot:
		if (mpcUnit->IsOperatorOperand(pcUnary->mpcOperand, pcUnary->meOperator) && mpcUnit->GetBinder()->BindOperator(pcUnary->meOperator, pcUnary->mpcOperand, NULL, pcUnary, &cCall, false))
		{
			//+object, -object, ~object: the object's operator, whatever it returns.
			return OperatorExpression(pcUnary->meOperator, pcUnary->mpcOperand, NULL, psValue);
		}
		break;
	default:
		return Unsupported(__FILE__, __LINE__);
	}

	//Operators on constants are worked out by the IR (the generator's Constant does its arithmetic in signed 64
	//bits, which isn't right for every type).  An object no operator function takes is what its conversion function
	//gives: +lambda is the pointer to its function.  [over.built]
	if (!ContextualValue(pcUnary->mpcOperand, CPPUnaryOperandTypes(pcUnary->meOperator), &sOperand))
	{
		return false;
	}
	if (sOperand.psType->IsFloat() && (pcUnary->meOperator != CT_BitwiseNot))
	{
		*psValue = sOperand;
		if (pcUnary->meOperator == CT_Subtract)
		{
			psValue->sValue = mpcFunction->Add(mpcBlock, IRO_FNeg, IRTypeOf(sOperand.psType), sOperand.sValue);
		}
		return true;
	}
	if (sOperand.psType->IsPointer() && (pcUnary->meOperator == CT_Add))
	{
		//+pointer is the pointer (an array's, or a function's, is the pointer to it).  [expr.unary.op] 7
		*psValue = sOperand;
		return true;
	}
	if (!sOperand.psType->IsInteger())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psType = mpcTypes->Promote(sOperand.psType);
	if (!Convert(&sOperand, psType))
	{
		return false;
	}
	sType = IRTypeOf(psType);
	psValue->psType = psType;
	switch (pcUnary->meOperator)
	{
	case CT_Subtract:
		psValue->sValue = mpcFunction->Add(mpcBlock, IRO_Neg, sType, sOperand.sValue);
		MarkSigned(psType);
		break;
	case CT_BitwiseNot:
		psValue->sValue = mpcFunction->Add(mpcBlock, IRO_Not, sType, sOperand.sValue);
		break;
	default:
		psValue->sValue = sOperand.sValue;
		break;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of an operand that integral promotion is applied to: a
// bit-field's becomes the type its width promotes it to
// (CCPPUnit::OperandType).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Operand(CCPPSyntax* pcExpression, SCPP65IRValue* psValue)
{
	return Value(pcExpression, psValue) && Convert(psValue, mpcUnit->OperandType(psValue->psType, pcExpression));
}


//////////////////////////////////////////////////////////////////////////
//
// ++x and --x give the new value, x++ and x-- the old one.  Pointers step by
// the size of what they point to.  psStored (if not NULL) gets the place
// that was stepped: what ++x and --x are as lvalues.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::IncrementExpression(CCPPSyntaxUnaryExpression* pcUnary, SCPP65IRValue* psValue, SCPP65IRPlace* psStored)
{
	SCPP65IRPlace	sPlace;
	SCPP65IRValue	sOld;
	SCPP65IRValue	sOne;
	SCPP65IRValue	sNew;
	SCPP65Type*		psType;

	if (mpcUnit->IsObject(pcUnary->mpcOperand))
	{
		//++object, --object, and object++ and object-- with an int 0: the object's operator.  [over.inc] 1
		return OperatorExpression(pcUnary->meOperator, pcUnary->mpcOperand, pcUnary->mbPostfix ? mpcUnit->GetDesugarings()->ImplicitZero() : NULL, psValue);
	}
	if (!Place(pcUnary->mpcOperand, &sPlace) || !Load(&sPlace, &sOld))
	{
		return false;
	}
	psType = sPlace.psType;
	if (!psType->IsScalar())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (psType->IsFloat())
	{
		sOne.sValue = mpcFunction->AddConstant(IRTypeOf(psType), (uint64)mpcUnit->GetFolder()->FloatBits(psType, 1.0));
		sOne.psType = psType;
	}
	else
	{
		sOne.sValue = Integer(&mpcTypes->msInt, 1);
		sOne.psType = &mpcTypes->msInt;
	}
	if (psType->eKind == CPP65TK_Bool)
	{
		//++b and b++ make a bool true: deprecated, and C++14's still.  [expr.pre.incr] 1
		sNew = sOne;
	}
	else if (!Operate((pcUnary->meOperator == CT_Increment) ? CT_Add : CT_Subtract, &sOld, &sOne, &sNew))
	{
		return false;
	}
	if (!Convert(&sNew, psType))
	{
		return false;
	}
	Store(&sPlace, &sNew);
	BitFieldResult(&sPlace, &sNew);
	*psValue = pcUnary->mbPostfix ? sOld : sNew;
	if (psStored)
	{
		*psStored = sPlace;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::BinaryExpression(CCPPSyntaxBinaryExpression* pcBinary, SCPP65IRValue* psValue)
{
	SCPP65IRValue	sLeft;
	SCPP65IRValue	sRight;
	SCPP65IRPlace	sPlace;

	if (IsCPPAssignment(pcBinary->meOperator))
	{
		return Assignment(pcBinary, psValue);
	}
	if (IsCPPComparison(pcBinary->meOperator))
	{
		return ConditionValue(pcBinary, psValue);
	}
	switch (pcBinary->meOperator)
	{
	case CT_MemberByPointerFunctionPointerIndirection:
		if (mpcUnit->IsObject(pcBinary->mpcLeft))
		{
			return OperatorExpression(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, psValue);
		}
		return MemberPointerPlace(pcBinary, &sPlace) && Load(&sPlace, psValue);
	case CT_MemberFunctionPointerIndirection:
		//object.*field (a member function can only be called).
		return MemberPointerPlace(pcBinary, &sPlace) && Load(&sPlace, psValue);
	case CT_Comma:
		if (mpcUnit->IsOverloadedComma(pcBinary))
		{
			return OperatorExpression(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, psValue);
		}
		return Value(pcBinary->mpcLeft, &sLeft) && Value(pcBinary->mpcRight, psValue);
	case CT_LogicalAnd:
	case CT_LogicalOr:
		return ConditionValue(pcBinary, psValue);
	default:
		if (HasOperatorFunction(pcBinary))
		{
			return OperatorExpression(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, psValue);
		}
		if (!Operands(pcBinary, &sLeft, &sRight))
		{
			return false;
		}
		if (((pcBinary->meOperator == CT_LeftShift) || (pcBinary->meOperator == CT_RightShift)) && sLeft.psType->IsInteger())
		{
			mpcUnit->GetWarner()->ShiftCount(pcBinary, mpcTypes->Promote(sLeft.psType)->iBytes * 8);
		}
		return Operate(pcBinary->meOperator, &sLeft, &sRight, psValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// x = y and x op= y.  The value is the one stored, and psStored (if not
// NULL) gets the place stored to (of a scalar).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Assignment(CCPPSyntaxBinaryExpression* pcBinary, SCPP65IRValue* psValue, SCPP65IRPlace* psStored)
{
	SCPP65IRPlace	sPlace;
	SCPP65IRValue	sRight;
	SCPP65IRValue	sOld;
	ECppTokens		eOperator;
	SCPP65Type*		psLeft;
	bool			bBytes;
	bool			bResult;

	psLeft = mpcUnit->TypeOf(pcBinary->mpcLeft);
	if (psLeft && psLeft->IsClass())
	{
		//object op= value calls the class's operator if it has one; object = object assigns each part otherwise, as
		//the compiler's own operator= does: the bytes, unless a part's class has an operator=.
		if (mpcUnit->HasOperator(psLeft->pcClass, mpcUnit->GetOperatorName(pcBinary->meOperator, false), 1))
		{
			return OperatorCall(pcBinary->mpcLeft, pcBinary->meOperator, pcBinary->mpcRight, psValue);
		}
		if (pcBinary->meOperator != CT_Assignment)
		{
			return Unsupported(__FILE__, __LINE__);
		}
		//The right before the left, as for a scalar.  [expr.ass] 1
		if (psLeft->pcClass->IsAssignmentDeleted())
		{
			return AssignmentDeleted(pcBinary);
		}
		if (!AssignedObject(pcBinary->mpcRight, psLeft->Unqualified(), &sRight) || !Place(pcBinary->mpcLeft, &sPlace) || !sPlace.psType->IsClass())
		{
			return false;
		}
		//A polymorphic object's vtable pointers stay: it may be a base subobject of another class's.
		bBytes = sPlace.psType->pcClass->IsTriviallyAssignable() && !sPlace.psType->pcClass->IsPolymorphic();
		if (!(bBytes ? CopyBytes(&sPlace, &sRight) : AssignMembers(&sPlace, &sRight, pcBinary->mpcRight)))
		{
			return false;
		}
		psValue->sValue = sPlace.sAddress;
		psValue->psType = sPlace.psType;
		return true;
	}
	//f = g chooses the overload of g with the type of f (worked out without its code, as the right is first).  x op= y
	//with an object for y takes what it converts to for the operator.
	eOperator = CPPCompoundOperation(pcBinary->meOperator);
	if (pcBinary->meOperator == CT_Assignment)
	{
		bResult = ValueFor(pcBinary->mpcRight, mpcUnit->PlaceTypeOf(pcBinary->mpcLeft), &sRight);
	}
	else
	{
		bResult = mpcUnit->IsObject(pcBinary->mpcRight) ? ContextualValue(pcBinary->mpcRight, CPPBinaryOperandTypes(eOperator), &sRight) : ValueFor(pcBinary->mpcRight, NULL, &sRight);
	}
	if (!bResult || !Place(pcBinary->mpcLeft, &sPlace))
	{
		return false;
	}
	if (!sPlace.psType->IsScalar() || (sPlace.psType->IsMemberPointer() && (pcBinary->meOperator != CT_Assignment)))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (psStored)
	{
		*psStored = sPlace;
	}
	if (pcBinary->meOperator == CT_Assignment)
	{
		mpcUnit->GetWarner()->ConversionOverflow(pcBinary->mpcRight, sPlace.psType);
		if (!ConvertImplicit(&sRight, sPlace.psType, pcBinary->mpcRight))
		{
			return false;
		}
		Store(&sPlace, &sRight);
		BitFieldResult(&sPlace, &sRight);
		*psValue = sRight;
		return true;
	}

	//x op= y is x = x op y: both are operands, a bit-field promoted as one is.  [expr.ass] 7
	if (!Load(&sPlace, &sOld) || !Convert(&sOld, mpcUnit->OperandType(sOld.psType, pcBinary->mpcLeft)) || !Convert(&sRight, mpcUnit->OperandType(sRight.psType, pcBinary->mpcRight)))
	{
		return false;
	}
	if (!Operate(eOperator, &sOld, &sRight, psValue) || !Convert(psValue, sPlace.psType))
	{
		return false;
	}
	Store(&sPlace, psValue);
	BitFieldResult(&sPlace, psValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// What an object of psClass is assigned: the right operand's object, or
// for a list in braces a temporary of the class made from the list, as
// x = {1, 2} is x = X{1, 2}.  [expr.ass] 9
//
//////////////////////////////////////////////////////////////////////////
//
// The IR opcode of an integer arithmetic operator (+ - * / % & | ^ << >>),
// signed or unsigned as its operands' type is; IRO_None for any other.
// The front end writes with it, and a constant's signed overflow is
// checked with it (CCPPUnit::IsSignedOverflow).
//
//////////////////////////////////////////////////////////////////////////
uint16 CPPArithmeticOpcode(ECppTokens eOperator, bool bSigned)
{
	switch (eOperator)
	{
	case CT_Add:			return IRO_Add;
	case CT_Subtract:		return IRO_Sub;
	case CT_Multiply:		return IRO_Mul;
	case CT_Divide:			return bSigned ? IRO_SDiv : IRO_UDiv;
	case CT_Modulus:		return bSigned ? IRO_SRem : IRO_URem;
	case CT_BitwiseAnd:		return IRO_And;
	case CT_BitwiseOr:		return IRO_Or;
	case CT_BitwiseXor:		return IRO_Xor;
	case CT_LeftShift:		return IRO_Shl;
	case CT_RightShift:		return bSigned ? IRO_AShr : IRO_LShr;
	default:				return IRO_None;
	}
}


//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::AssignedObject(CCPPSyntax* pcRight, SCPP65Type* psClass, SCPP65IRValue* psValue)
{
	SCPP65IRPlace	sTemporary;
	SCPP65IRMade	sMade;

	if ((mpcUnit->SkipParentheses(pcRight)->GetKind() != CPPS_InitializerList) && (mpcUnit->IsObject(pcRight) || !psClass->IsClass() || !mpcUnit->GetBinder()->ConvertsByConstructor(psClass->pcClass, pcRight)))
	{
		return Value(pcRight, psValue);
	}
	if (!Temporary(psClass, &sTemporary, &sMade) || !Construct(&sTemporary, NULL, pcRight))
	{
		return false;
	}
	TemporaryMade(&sMade);
	psValue->sValue = sTemporary.sAddress;
	psValue->psType = psClass;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// left op right for the arithmetic, bitwise and shift operators.  Pointer +
// integer and pointer - integer are scaled by the size pointed to; pointer -
// pointer is divided by it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Operate(ECppTokens eOperator, SCPP65IRValue* psLeft, SCPP65IRValue* psRight, SCPP65IRValue* psResult)
{
	SCPP65Type*		psType;
	SCPP65IRValue	sLeft;
	SCPP65IRValue	sRight;
	SCPP65IRValue	sIndex;
	SCPP65IRValue	sPointer;
	SIRType			sType;
	SIRType			sOffset;
	SIRReference	sScaled;
	SIRReference	sDifference;
	uint16			uiOpcode;
	int				iBytes;

	sLeft = *psLeft;
	sRight = *psRight;
	if (sLeft.psType->IsFloat() || sRight.psType->IsFloat())
	{
		psType = mpcTypes->Operation(eOperator, sLeft.psType, sRight.psType);
		if (!psType)
		{
			return Unsupported(__FILE__, __LINE__);
		}
		switch (eOperator)
		{
		case CT_Add:		uiOpcode = IRO_FAdd;	break;
		case CT_Subtract:	uiOpcode = IRO_FSub;	break;
		case CT_Multiply:	uiOpcode = IRO_FMul;	break;
		default:			uiOpcode = IRO_FDiv;	break;
		}
		if (!Convert(&sLeft, psType) || !Convert(&sRight, psType))
		{
			return false;
		}
		psResult->sValue = mpcFunction->Add(mpcBlock, uiOpcode, IRTypeOf(psType), sLeft.sValue, sRight.sValue);
		psResult->psType = psType;
		return true;
	}

	if (sLeft.psType->IsPointer() || sRight.psType->IsPointer())
	{
		sOffset = IRIntegerType(mcModule.GetPointerSize(0));
		if (((eOperator == CT_Add) || (eOperator == CT_Subtract)) && sLeft.psType->IsPointer() != sRight.psType->IsPointer())
		{
			if ((eOperator == CT_Subtract) && !sLeft.psType->IsPointer())
			{
				return Unsupported(__FILE__, __LINE__);
			}
			sPointer = sLeft.psType->IsPointer() ? sLeft : sRight;
			sIndex = sLeft.psType->IsPointer() ? sRight : sLeft;
			if (!sIndex.psType->IsInteger() || sPointer.psType->psBase->IsVoid() || sPointer.psType->psBase->IsFunction())
			{
				return Unsupported(__FILE__, __LINE__);
			}
			//The index widens to the pointer's size (an int is narrower on the W65C816).
			if (!Convert(&sIndex, mpcTypes->Integer(mcModule.GetPointerSize(0), sIndex.psType->bSigned)))
			{
				return false;
			}
			iBytes = sPointer.psType->GetElementBytes();
			sScaled = (iBytes == 1) ? sIndex.sValue : mpcFunction->Add(mpcBlock, IRO_Mul, sOffset, sIndex.sValue, mpcFunction->AddInteger(sOffset, iBytes));
			if (eOperator == CT_Subtract)
			{
				sScaled = mpcFunction->Add(mpcBlock, IRO_Neg, sOffset, sScaled);
			}
			psResult->sValue = mpcFunction->Add(mpcBlock, IRO_PtrAdd, IRType(IRTK_Pointer), sPointer.sValue, sScaled);
			psResult->psType = sPointer.psType;
			return true;
		}
		if ((eOperator == CT_Subtract) && sLeft.psType->IsPointer() && sRight.psType->IsPointer())
		{
			sDifference = mpcFunction->Add(mpcBlock, IRO_Sub, sOffset,
				mpcFunction->Add(mpcBlock, IRO_PtrToI, sOffset, sLeft.sValue),
				mpcFunction->Add(mpcBlock, IRO_PtrToI, sOffset, sRight.sValue));
			iBytes = sLeft.psType->GetElementBytes();
			if (iBytes != 1)
			{
				sDifference = mpcFunction->Add(mpcBlock, IRO_SDiv, sOffset, sDifference, mpcFunction->AddInteger(sOffset, iBytes));
			}
			psResult->sValue = sDifference;
			psResult->psType = mpcTypes->PointerDifference();
			return true;
		}
		return Unsupported(__FILE__, __LINE__);
	}

	psType = mpcTypes->Operation(eOperator, sLeft.psType, sRight.psType);
	if (!psType)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (!Convert(&sLeft, psType) || !Convert(&sRight, psType))
	{
		return false;
	}
	uiOpcode = CPPArithmeticOpcode(eOperator, psType->bSigned);
	if (uiOpcode == IRO_None)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	sType = IRTypeOf(psType);
	psResult->sValue = mpcFunction->Add(mpcBlock, uiOpcode, sType, sLeft.sValue, sRight.sValue);
	psResult->psType = psType;
	if (IsIRSignedWrapping(uiOpcode))
	{
		MarkSigned(psType);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The add, sub, mul, neg or shl just written is of psType: in a constant's
// IR one of a signed type is marked, so that an overflow is no constant
// (IRF_NoSignedWrap).  [expr] 4
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRFrontEnd::MarkSigned(SCPP65Type* psType)
{
	if (mbConstants && psType->IsInteger() && psType->bSigned && (mpcBlock->masInstructions.NumElements() > 0))
	{
		mpcBlock->masInstructions.Tail()->uiFlags |= IRF_NoSignedWrap;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An i1: the comparison of left and right.  Pointers compare unsigned;
// integers after the usual arithmetic conversions; floats as floats.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Compare(ECppTokens eOperator, SCPP65IRValue* psLeft, SCPP65IRValue* psRight, SIRReference* psCondition)
{
	SCPP65Type*		psType;
	SCPP65IRValue	sLeft;
	SCPP65IRValue	sRight;
	bool			bSigned;
	bool			bFloat;
	bool			bMemberOrder;
	uint16			uiOpcode;

	sLeft = *psLeft;
	sRight = *psRight;
	bFloat = false;
	bSigned = false;
	if (sLeft.psType->IsFloat() || sRight.psType->IsFloat())
	{
		if (sLeft.psType->IsPointer() || sRight.psType->IsPointer())
		{
			return Unsupported(__FILE__, __LINE__);
		}
		psType = mpcTypes->FloatArithmetic(sLeft.psType, sRight.psType);
		bFloat = true;
	}
	else if (sLeft.psType->IsInteger() && sRight.psType->IsInteger())
	{
		psType = mpcTypes->Arithmetic(sLeft.psType, sRight.psType);
		bSigned = psType->bSigned;
	}
	else
	{
		//Pointers, std::nullptr_t and pointers to members (which are only equal or not) are compared as their composite
		//pointer type: a pointer to a derived class is moved to the base the other points to.
		bMemberOrder = (sLeft.psType->IsMemberPointer() || sRight.psType->IsMemberPointer()) && (eOperator != CT_EqualTo) && (eOperator != CT_NotEqualTo);
		psType = bMemberOrder ? NULL : mpcUnit->GetTypeChecker()->CompositePointerType(sLeft.psType, sRight.psType);
		if (!psType)
		{
			return Unsupported(__FILE__, __LINE__);
		}
	}
	if (!Convert(&sLeft, psType) || !Convert(&sRight, psType))
	{
		return false;
	}

	switch (eOperator)
	{
	case CT_EqualTo:				uiOpcode = bFloat ? IRO_FEq : IRO_Eq;									break;
	case CT_NotEqualTo:				uiOpcode = bFloat ? IRO_FNe : IRO_Ne;									break;
	case CT_LessThan:				uiOpcode = bFloat ? IRO_FLt : (bSigned ? IRO_SLt : IRO_ULt);			break;
	case CT_LessThanEqualTo:		uiOpcode = bFloat ? IRO_FLe : (bSigned ? IRO_SLe : IRO_ULe);			break;
	case CT_GreaterThan:			uiOpcode = bFloat ? IRO_FGt : (bSigned ? IRO_SGt : IRO_UGt);			break;
	case CT_GreaterThanEqualTo:		uiOpcode = bFloat ? IRO_FGe : (bSigned ? IRO_SGe : IRO_UGe);			break;
	default:						return Unsupported(__FILE__, __LINE__);
	}
	*psCondition = mpcFunction->Add(mpcBlock, uiOpcode, IRTypeOf(psType), sLeft.sValue, sRight.sValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// c ? x : y.  The two values meet in a phi.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ConditionalExpression(CCPPSyntaxConditionalExpression* pcConditional, SCPP65IRValue* psValue)
{
	SCPP65Type*		psTrue;
	SCPP65Type*		psFalse;
	SCPP65Type*		psType;
	CIRBlock*		pcTrue;
	CIRBlock*		pcFalse;
	CIRBlock*		pcEnd;
	CIRBlock*		pcTrueEnd;
	SCPP65IRValue	sTrue;
	SCPP65IRValue	sFalse;
	bool			bThrowTrue;
	bool			bThrowFalse;
	bool			bObject;

	psTrue = mpcUnit->TypeOf(pcConditional->mpcTrue);
	psFalse = mpcUnit->TypeOf(pcConditional->mpcFalse);
	if (!psTrue || !psFalse || psTrue->IsReference() || psFalse->IsReference())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	bObject = psTrue->IsClass() || psFalse->IsClass();

	//A throw for one operand: the result is the other's, and that way never comes to the end.  [expr.cond] 2
	bThrowTrue = pcConditional->mpcTrue->WithoutParentheses()->GetKind() == CPPS_ThrowExpression;
	bThrowFalse = pcConditional->mpcFalse->WithoutParentheses()->GetKind() == CPPS_ThrowExpression;
	if (bThrowTrue || bThrowFalse || bObject)
	{
		//An object for one operand that converts to the other's type: the type checker says which.  [expr.cond] 3
		psType = mpcUnit->TypeOf(pcConditional);
		if (!psType || psType->IsClass())
		{
			return Unsupported(__FILE__, __LINE__);
		}
	}
	else
	{
		psType = mpcUnit->GetTypeChecker()->ConditionalType(psTrue, psFalse);
	}
	if (!psType || (!psType->IsVoid() && (psTrue->IsArray() || psFalse->IsArray())))
	{
		return Unsupported(__FILE__, __LINE__);
	}

	pcTrue = NewBlock();
	pcFalse = NewBlock();
	pcEnd = NewBlock();
	if (!Condition(pcConditional->mpcCondition, pcTrue, pcFalse))
	{
		return false;
	}
	//A temporary made by only one of them is destroyed only if it was made (Temporary).
	miConditional++;
	StartBlock(pcTrue);
	if (!(bObject ? ValueFor(pcConditional->mpcTrue, psType, &sTrue) : Value(pcConditional->mpcTrue, &sTrue)) || (!bThrowTrue && !Convert(&sTrue, psType)))
	{
		return false;
	}
	if (bThrowTrue && !psType->IsVoid())
	{
		//Where the throw's block (which isn't reached) joins: a value for the phi.
		sTrue.sValue = mpcFunction->AddConstant(IRTypeOf(psType), 0);
	}
	pcTrueEnd = mpcBlock;
	JumpTo(pcEnd);
	StartBlock(pcFalse);
	if (!(bObject ? ValueFor(pcConditional->mpcFalse, psType, &sFalse) : Value(pcConditional->mpcFalse, &sFalse)) || (!bThrowFalse && !Convert(&sFalse, psType)))
	{
		return false;
	}
	if (bThrowFalse && !psType->IsVoid())
	{
		sFalse.sValue = mpcFunction->AddConstant(IRTypeOf(psType), 0);
	}
	miConditional--;
	JumpTo(pcEnd);
	psValue->psType = psType;
	if (!psType->IsVoid())
	{
		psValue->sValue = Phi(pcEnd, IRTypeOf(psType), pcTrueEnd, sTrue.sValue, mpcBlock, sFalse.sValue);
	}
	else
	{
		psValue->sValue = IRNone();
	}
	StartBlock(pcEnd);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a binary operator with an object for an operand is an operator
// function's call: the type checker's question (CCPP65TypeChecker::
// HasOperatorFunction).  Otherwise it is the built-in operator.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::HasOperatorFunction(CCPPSyntaxBinaryExpression* pcBinary)
{
	CCPPBoundCall	cBound;

	return (mpcUnit->IsOperatorOperand(pcBinary->mpcLeft, pcBinary->meOperator) || mpcUnit->IsOperatorOperand(pcBinary->mpcRight, pcBinary->meOperator)) &&
		   mpcUnit->GetBinder()->BindOperator(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, pcBinary, &cBound, false);
}


//////////////////////////////////////////////////////////////////////////
//
// A built-in binary operator's operands' values, the left first: an object
// is what its conversion function gives for the operator.  [over.built]
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Operands(CCPPSyntaxBinaryExpression* pcBinary, SCPP65IRValue* psLeft, SCPP65IRValue* psRight)
{
	ECPPContextualTypes	eWanted;

	eWanted = CPPBinaryOperandTypes(pcBinary->meOperator);
	return ContextualValue(pcBinary->mpcLeft, eWanted, psLeft) && ContextualValue(pcBinary->mpcRight, eWanted, psRight);
}


//////////////////////////////////////////////////////////////////////////
//
// c ? x : y of two lvalues of one type, as the place it is: x's or y's,
// their addresses meeting in a phi.  Where one is a bit-field, which bits of
// the unit there the place is meets in a phi too (SCPP65IRPlace::bEither).
// [expr.cond] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ConditionalPlace(CCPPSyntaxConditionalExpression* pcConditional, SCPP65IRPlace* psPlace)
{
	CIRBlock*		pcTrue;
	CIRBlock*		pcFalse;
	CIRBlock*		pcEnd;
	CIRBlock*		pcTrueEnd;
	SCPP65IRPlace	sTrue;
	SCPP65IRPlace	sFalse;
	SIRType			sType;

	psPlace->psType = mpcUnit->PlaceTypeOf(pcConditional);
	psPlace->psBitField = NULL;
	psPlace->bEither = false;
	if (!psPlace->psType)
	{
		return Unsupported(__FILE__, __LINE__);
	}

	pcTrue = NewBlock();
	pcFalse = NewBlock();
	pcEnd = NewBlock();
	if (!Condition(pcConditional->mpcCondition, pcTrue, pcFalse))
	{
		return false;
	}
	//A temporary made by only one of them is destroyed only if it was made (Temporary).
	miConditional++;
	StartBlock(pcTrue);
	if (!Place(pcConditional->mpcTrue, &sTrue))
	{
		return false;
	}
	pcTrueEnd = mpcBlock;
	JumpTo(pcEnd);
	StartBlock(pcFalse);
	if (!Place(pcConditional->mpcFalse, &sFalse))
	{
		return false;
	}
	miConditional--;
	JumpTo(pcEnd);
	if (sTrue.bEither || sFalse.bEither)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psPlace->sAddress = Phi(pcEnd, IRType(IRTK_Pointer), pcTrueEnd, sTrue.sAddress, mpcBlock, sFalse.sAddress);
	if (sTrue.psBitField || sFalse.psBitField)
	{
		sType = IRTypeOf(psPlace->psType);
		psPlace->bEither = true;
		psPlace->psBitField = sTrue.psBitField;
		psPlace->psElseBitField = sFalse.psBitField;
		psPlace->sFirst = Phi(pcEnd, sType, pcTrueEnd, mpcFunction->AddInteger(sType, -1), mpcBlock, mpcFunction->AddInteger(sType, 0));
	}
	StartBlock(pcEnd);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A condition as a bool: 1 or 0 from a phi.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::ConditionValue(CCPPSyntax* pcExpression, SCPP65IRValue* psValue)
{
	CIRBlock*	pcTrue;
	CIRBlock*	pcFalse;
	CIRBlock*	pcEnd;
	SIRType		sType;

	pcTrue = NewBlock();
	pcFalse = NewBlock();
	pcEnd = NewBlock();
	if (!Condition(pcExpression, pcTrue, pcFalse))
	{
		return false;
	}
	pcTrue->Jump(pcEnd);
	pcFalse->Jump(pcEnd);
	sType = IRTypeOf(&mpcTypes->msBool);
	psValue->sValue = Phi(pcEnd, sType, pcTrue, mpcFunction->AddConstant(sType, 1), pcFalse, mpcFunction->AddConstant(sType, 0));
	psValue->psType = &mpcTypes->msBool;
	StartBlock(pcEnd);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A statement's condition, a full expression: branches to pcTrue or pcFalse
// after destroying the temporaries made working it out (on both ways out).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::FullCondition(CCPPSyntax* pcExpression, CIRBlock* pcTrue, CIRBlock* pcFalse)
{
	size		uiTemporaries;
	CIRBlock*	pcDestroyTrue;
	CIRBlock*	pcDestroyFalse;

	uiTemporaries = mcCleanups.NumTemporaries();
	pcDestroyTrue = NewBlock();
	pcDestroyFalse = NewBlock();
	if (!Condition(pcExpression, pcDestroyTrue, pcDestroyFalse))
	{
		return false;
	}
	MarkFullExpression();
	StartBlock(pcDestroyTrue);
	if (!DestroyTemporaryPlaces(uiTemporaries))
	{
		return false;
	}
	JumpTo(pcTrue);
	StartBlock(pcDestroyFalse);
	if (!DestroyTemporaryPlaces(uiTemporaries))
	{
		return false;
	}
	JumpTo(pcFalse);
	mcCleanups.CutTemporaries(uiTemporaries);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Branches to pcTrue or pcFalse.  && and || only work out their right
// operand when they need to.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Condition(CCPPSyntax* pcExpression, CIRBlock* pcTrue, CIRBlock* pcFalse)
{
	CCPPBoundCall				cOperatorCall;
	CCPPSyntaxBinaryExpression*	pcBinary;
	CCPPSyntaxUnaryExpression*	pcUnary;
	CIRBlock*					pcRight;
	SCPP65IRValue				sLeft;
	SCPP65IRValue				sRight;
	SCPP65IRValue				sValue;
	SIRReference				sCondition;
	bool						bResult;
	CCPPVariable*				pcVariable;
	SCPP65IRPlace				sPlace;

	pcExpression = mpcUnit->SkipParentheses(pcExpression);
	if (!pcExpression)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (pcExpression->GetKind() == CPPS_UnaryExpression)
	{
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		if (pcUnary->meOperator == CT_LogicalNot)
		{
			if (mpcUnit->IsOperatorOperand(pcUnary->mpcOperand, CT_LogicalNot) && mpcUnit->GetBinder()->BindOperator(CT_LogicalNot, pcUnary->mpcOperand, NULL, pcUnary, &cOperatorCall, false))
			{
				//!object with the object's operator!.
				if (!OperatorExpression(CT_LogicalNot, pcUnary->mpcOperand, NULL, &sValue) || !Truth(&sValue, &sCondition))
				{
					return false;
				}
				mpcBlock->Branch(sCondition, pcTrue, pcFalse);
				return true;
			}
			return Condition(pcUnary->mpcOperand, pcFalse, pcTrue);
		}
	}
	else if (pcExpression->GetKind() == CPPS_BinaryExpression)
	{
		pcBinary = (CCPPSyntaxBinaryExpression*)pcExpression;
		if ((pcBinary->meOperator == CT_LogicalAnd) || (pcBinary->meOperator == CT_LogicalOr))
		{
			pcRight = NewBlock();
			if (!Condition(pcBinary->mpcLeft, (pcBinary->meOperator == CT_LogicalAnd) ? pcRight : pcTrue, (pcBinary->meOperator == CT_LogicalAnd) ? pcFalse : pcRight))
			{
				return false;
			}
			StartBlock(pcRight);
			miConditional++;
			bResult = Condition(pcBinary->mpcRight, pcTrue, pcFalse);
			miConditional--;
			return bResult;
		}
		if (IsCPPComparison(pcBinary->meOperator))
		{
			if (HasOperatorFunction(pcBinary))
			{
				//An operator function's result.
				if (!OperatorExpression(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, &sValue) || !sValue.psType->IsScalar() || !Truth(&sValue, &sCondition))
				{
					return Unsupported(__FILE__, __LINE__);
				}
				mpcBlock->Branch(sCondition, pcTrue, pcFalse);
				return true;
			}
			mpcUnit->GetWarner()->ComparisonRange(pcBinary);
			if (!Operands(pcBinary, &sLeft, &sRight) || !Compare(pcBinary->meOperator, &sLeft, &sRight, &sCondition))
			{
				return false;
			}
			mpcBlock->Branch(sCondition, pcTrue, pcFalse);
			return true;
		}
	}
	else if (pcExpression->GetKind() == CPPS_SimpleDeclaration)
	{
		//if (T x = value): the variable is the condition, an object converted to bool, by an explicit conversion
		//function too.  [stmt.select] 4
		pcVariable = NULL;
		if (!Declaration((CCPPSyntaxSimpleDeclaration*)pcExpression, &pcVariable))
		{
			return false;
		}
		if (!pcVariable || !VariablePlace(pcVariable, &sPlace))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		if (pcVariable->mpsType->IsArray())
		{
			//[stmt.select] 2
			return mpcDiagnostics->ReportOnce(pcExpression, "A condition's variable isn't an array");
		}
		if (pcVariable->mpsType->IsClass())
		{
			sValue.psType = pcVariable->mpsType;
			sValue.sValue = sPlace.sAddress;
			if (!ConvertObject(&sValue, &mpcTypes->msBool, true))
			{
				return false;
			}
		}
		else if (!pcVariable->mpsType->IsScalar() || !Load(&sPlace, &sValue))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		if (!Truth(&sValue, &sCondition))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		mpcBlock->Branch(sCondition, pcTrue, pcFalse);
		return true;
	}

	//An object is converted to bool (with its conversion function).
	if (!Value(pcExpression, &sValue))
	{
		return false;
	}
	if (sValue.psType && sValue.psType->IsScopedEnum())
	{
		return mpcDiagnostics->ReportError(pcExpression, "An enum class's value is not a condition: compare it with one of its values");
	}
	if (!Truth(&sValue, &sCondition))
	{
		return false;
	}
	mpcBlock->Branch(sCondition, pcTrue, pcFalse);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An i1 that is 1 when a scalar is not zero (+0 and -0 are zero).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::Truth(SCPP65IRValue* psValue, SIRReference* psCondition)
{
	SIRType	sType;

	if (IsObjectValue(psValue) && !ConvertObject(psValue, &mpcTypes->msBool))
	{
		return false;
	}
	if (!psValue->psType->IsScalar())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (psValue->psType->IsScopedEnum())
	{
		//Where the expression isn't to hand (!e, e && x, e ? x : y).
		return mpcDiagnostics->ReportError(NULL, "An enum class's value is not a condition: compare it with one of its values");
	}
	sType = IRTypeOf(psValue->psType);
	if (psValue->psType->IsFloat())
	{
		*psCondition = mpcFunction->Add(mpcBlock, IRO_FNe, sType, psValue->sValue, mpcFunction->AddConstant(sType, 0));
	}
	else
	{
		*psCondition = mpcFunction->Add(mpcBlock, IRO_Ne, sType, psValue->sValue, mpcFunction->AddConstant(sType, 0));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// left op right, or op operand when pcRight is NULL, with an object
// operand: the left's member
// operator if one matches, otherwise a free operator function.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::OperatorExpression(ECppTokens eOperator, CCPPSyntax* pcLeft, CCPPSyntax* pcRight, SCPP65IRValue* psValue)
{
	CCPPBoundCall	cBound;

	if (!mpcUnit->GetBinder()->BindOperator(eOperator, pcLeft, pcRight, pcLeft, &cBound, false))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	return BoundOperatorCall(&cBound, eOperator, psValue);
}
