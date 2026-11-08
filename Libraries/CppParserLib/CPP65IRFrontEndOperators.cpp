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
#include "CPP65Generator.h"
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
		return ConditionValue(pcUnary, psValue);
	case CT_Multiply:
		if (!Place(pcUnary, &sPlace))
		{
			return false;
		}
		return Load(&sPlace, psValue);
	case CT_BitwiseAnd:
		pcOperand = mpcGenerator->SkipParentheses(pcUnary->mpcOperand);
		bMember = false;
		if ((pcOperand->GetKind() == CPPS_IdExpression) && ((CCPPSyntaxIdExpression*)pcOperand)->mpcName->IsQualifiedByClass() &&
			MemberAddress((CCPPSyntaxIdExpression*)pcOperand, psValue, &bMember))
		{
			return true;
		}
		if (bMember)
		{
			return false;
		}
		if (mpcGenerator->IsObject(pcOperand) && mpcGenerator->HasMember(mpcGenerator->ObjectClass(pcOperand), mpcGenerator->GetOperatorName(CT_BitwiseAnd, false)))
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
			return Unsupported(__FILE__, __LINE__);
		}
		if (!Place(pcOperand, &sPlace))
		{
			return false;
		}
		psValue->sValue = sPlace.sAddress;
		psValue->psType = mpcTypes->Pointer(sPlace.psType);
		return true;
	case CT_Add:
	case CT_Subtract:
	case CT_BitwiseNot:
		break;
	default:
		return Unsupported(__FILE__, __LINE__);
	}

	//Operators on constants are worked out by the IR (the generator's Constant does its arithmetic in signed 64
	//bits, which isn't right for every type).
	if (!Value(pcUnary->mpcOperand, &sOperand))
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
// ++x and --x give the new value, x++ and x-- the old one.  Pointers step by
// the size of what they point to.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::IncrementExpression(CCPPSyntaxUnaryExpression* pcUnary, SCPP65IRValue* psValue)
{
	SCPP65IRPlace	sPlace;
	SCPP65IRValue	sOld;
	SCPP65IRValue	sOne;
	SCPP65IRValue	sNew;
	SCPP65Type*		psType;

	if (!Place(pcUnary->mpcOperand, &sPlace) || !Load(&sPlace, &sOld))
	{
		return false;
	}
	psType = sPlace.psType;
	if (!psType->IsScalar() || (psType->eKind == CPP65TK_Bool))
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (psType->IsFloat())
	{
		sOne.sValue = mpcFunction->AddConstant(IRTypeOf(psType), (uint64)mpcGenerator->FloatBits(psType, 1.0));
		sOne.psType = psType;
	}
	else
	{
		sOne.sValue = Integer(&mpcTypes->msInt, 1);
		sOne.psType = &mpcTypes->msInt;
	}
	if (!Operate((pcUnary->meOperator == CT_Increment) ? CT_Add : CT_Subtract, &sOld, &sOne, &sNew) || !Convert(&sNew, psType))
	{
		return false;
	}
	Store(&sPlace, &sNew);
	*psValue = pcUnary->mbPostfix ? sOld : sNew;
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

	switch (pcBinary->meOperator)
	{
	case CT_MemberByPointerFunctionPointerIndirection:
		if (mpcGenerator->IsObject(pcBinary->mpcLeft))
		{
			return OperatorExpression(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, psValue);
		}
		return MemberPointerPlace(pcBinary, &sPlace) && Load(&sPlace, psValue);
	case CT_MemberFunctionPointerIndirection:
		//object.*field (a member function can only be called).
		return MemberPointerPlace(pcBinary, &sPlace) && Load(&sPlace, psValue);
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
		return Assignment(pcBinary, psValue);
	case CT_Comma:
		return Value(pcBinary->mpcLeft, &sLeft) && Value(pcBinary->mpcRight, psValue);
	case CT_LogicalAnd:
	case CT_LogicalOr:
	case CT_EqualTo:
	case CT_NotEqualTo:
	case CT_LessThan:
	case CT_LessThanEqualTo:
	case CT_GreaterThan:
	case CT_GreaterThanEqualTo:
		return ConditionValue(pcBinary, psValue);
	default:
		if (mpcGenerator->IsObject(pcBinary->mpcLeft) || mpcGenerator->IsObject(pcBinary->mpcRight))
		{
			return OperatorExpression(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, psValue);
		}
		if (!Value(pcBinary->mpcLeft, &sLeft) || !Value(pcBinary->mpcRight, &sRight))
		{
			return false;
		}
		if (((pcBinary->meOperator == CT_LeftShift) || (pcBinary->meOperator == CT_RightShift)) && sLeft.psType->IsInteger())
		{
			mpcGenerator->ShiftCount(pcBinary, mpcTypes->Promote(sLeft.psType)->iBytes * 8);
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

	psLeft = mpcGenerator->TypeOf(pcBinary->mpcLeft);
	if (psLeft && psLeft->IsClass())
	{
		//object op= value calls the class's operator if it has one; object = object copies the bytes otherwise.
		if (mpcGenerator->HasOperator(psLeft->psClass, mpcGenerator->GetOperatorName(pcBinary->meOperator, false), 1))
		{
			return OperatorCall(pcBinary->mpcLeft, pcBinary->meOperator, pcBinary->mpcRight, psValue);
		}
		if (pcBinary->meOperator != CT_Assignment)
		{
			return Unsupported(__FILE__, __LINE__);
		}
		//The right before the left, as for a scalar.  [expr.ass] 1
		if (!Value(pcBinary->mpcRight, &sRight) || !Place(pcBinary->mpcLeft, &sPlace) || !sPlace.psType->IsClass() || !CopyBytes(&sPlace, &sRight))
		{
			return false;
		}
		psValue->sValue = sPlace.sAddress;
		psValue->psType = sPlace.psType;
		return true;
	}
	//f = g chooses the overload of g with the type of f (worked out without its code, as the right is first).
	if (!ValueFor(pcBinary->mpcRight, (pcBinary->meOperator == CT_Assignment) ? mpcGenerator->PlaceTypeOf(pcBinary->mpcLeft) : NULL, &sRight) || !Place(pcBinary->mpcLeft, &sPlace))
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
		mpcGenerator->ConversionOverflow(pcBinary->mpcRight, sPlace.psType);
		if (!Convert(&sRight, sPlace.psType))
		{
			return false;
		}
		Store(&sPlace, &sRight);
		*psValue = sRight;
		return true;
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
	if (!Load(&sPlace, &sOld) || !Operate(eOperator, &sOld, &sRight, psValue) || !Convert(psValue, sPlace.psType))
	{
		return false;
	}
	Store(&sPlace, psValue);
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
	switch (eOperator)
	{
	case CT_Add:			uiOpcode = IRO_Add;									break;
	case CT_Subtract:		uiOpcode = IRO_Sub;									break;
	case CT_Multiply:		uiOpcode = IRO_Mul;									break;
	case CT_Divide:			uiOpcode = psType->bSigned ? IRO_SDiv : IRO_UDiv;	break;
	case CT_Modulus:		uiOpcode = psType->bSigned ? IRO_SRem : IRO_URem;	break;
	case CT_BitwiseAnd:		uiOpcode = IRO_And;									break;
	case CT_BitwiseOr:		uiOpcode = IRO_Or;									break;
	case CT_BitwiseXor:		uiOpcode = IRO_Xor;									break;
	case CT_LeftShift:		uiOpcode = IRO_Shl;									break;
	case CT_RightShift:		uiOpcode = psType->bSigned ? IRO_AShr : IRO_LShr;	break;
	default:				return Unsupported(__FILE__, __LINE__);
	}
	sType = IRTypeOf(psType);
	psResult->sValue = mpcFunction->Add(mpcBlock, uiOpcode, sType, sLeft.sValue, sRight.sValue);
	psResult->psType = psType;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psFrom and psTo are pointers to different classes, psFrom's
// derived from psTo's (so psFrom converts to psTo, moved to the base).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRFrontEnd::IsClassPointerTo(SCPP65Type* psFrom, SCPP65Type* psTo)
{
	return psFrom->IsPointer() && psTo->IsPointer() && psFrom->psBase->IsClass() && psTo->psBase->IsClass() &&
		   (psFrom->psBase->psClass != psTo->psBase->psClass) && mpcGenerator->IsDerived(psFrom->psBase->psClass, psTo->psBase->psClass, NULL);
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
	else if (sLeft.psType->IsPointer() || sRight.psType->IsPointer())
	{
		//A pointer to a derived class and one to its base compare as the base's (the derived one moved to it).
		psType = sLeft.psType->IsPointer() ? sLeft.psType : sRight.psType;
		if (IsClassPointerTo(sLeft.psType, sRight.psType))
		{
			psType = sRight.psType;
		}
	}
	else if ((sLeft.psType->IsMemberPointer() || sRight.psType->IsMemberPointer()) && ((eOperator == CT_EqualTo) || (eOperator == CT_NotEqualTo)))
	{
		//Pointers to members (an address or an offset, 0 for none) are equal or not, a base's as the derived class's.
		psType = sLeft.psType->IsMemberPointer() ? sLeft.psType : sRight.psType;
		if (sLeft.psType->IsMemberPointer() && sRight.psType->IsMemberPointer() && (sLeft.psType->psClass != sRight.psType->psClass) &&
			mpcGenerator->IsDerived(sRight.psType->psClass, sLeft.psType->psClass, NULL))
		{
			psType = sRight.psType;
		}
	}
	else if (sLeft.psType->IsInteger() && sRight.psType->IsInteger())
	{
		psType = mpcTypes->Arithmetic(sLeft.psType, sRight.psType);
		bSigned = psType->bSigned;
	}
	else
	{
		return Unsupported(__FILE__, __LINE__);
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

	psTrue = mpcGenerator->TypeOf(pcConditional->mpcTrue);
	psFalse = mpcGenerator->TypeOf(pcConditional->mpcFalse);
	if (!psTrue || !psFalse || psTrue->IsReference() || psFalse->IsReference() || psTrue->IsClass() || psFalse->IsClass())
	{
		return Unsupported(__FILE__, __LINE__);
	}
	psType = mpcTypes->Conditional(psTrue, psFalse);
	if (!psType->IsVoid() && (psTrue->IsArray() || psFalse->IsArray()))
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
	if (!Value(pcConditional->mpcTrue, &sTrue) || !Convert(&sTrue, psType))
	{
		return false;
	}
	pcTrueEnd = mpcBlock;
	JumpTo(pcEnd);
	StartBlock(pcFalse);
	if (!Value(pcConditional->mpcFalse, &sFalse) || !Convert(&sFalse, psType))
	{
		return false;
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

	uiTemporaries = masTemporaries.NumElements();
	pcDestroyTrue = NewBlock();
	pcDestroyFalse = NewBlock();
	if (!Condition(pcExpression, pcDestroyTrue, pcDestroyFalse))
	{
		return false;
	}
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
	masTemporaries.FakeSetUsedElements(uiTemporaries);
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
	CCPPSyntaxBinaryExpression*	pcBinary;
	CCPPSyntaxUnaryExpression*	pcUnary;
	CIRBlock*					pcRight;
	SCPP65IRValue				sLeft;
	SCPP65IRValue				sRight;
	SCPP65IRValue				sValue;
	SIRReference				sCondition;
	bool						bResult;
	SCPP65Variable*				psVariable;
	SCPP65IRPlace				sPlace;

	pcExpression = mpcGenerator->SkipParentheses(pcExpression);
	if (!pcExpression)
	{
		return Unsupported(__FILE__, __LINE__);
	}
	if (pcExpression->GetKind() == CPPS_UnaryExpression)
	{
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		if (pcUnary->meOperator == CT_LogicalNot)
		{
			if (mpcGenerator->IsObject(pcUnary->mpcOperand) && mpcGenerator->HasMember(mpcGenerator->ObjectClass(pcUnary->mpcOperand), mpcGenerator->GetOperatorName(CT_LogicalNot, false)))
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
		switch (pcBinary->meOperator)
		{
		case CT_LogicalAnd:
		case CT_LogicalOr:
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
		case CT_EqualTo:
		case CT_NotEqualTo:
		case CT_LessThan:
		case CT_LessThanEqualTo:
		case CT_GreaterThan:
		case CT_GreaterThanEqualTo:
			if (mpcGenerator->IsObject(pcBinary->mpcLeft) || mpcGenerator->IsObject(pcBinary->mpcRight))
			{
				//An operator function's result.
				if (!OperatorExpression(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, &sValue) || !sValue.psType->IsScalar() || !Truth(&sValue, &sCondition))
				{
					return Unsupported(__FILE__, __LINE__);
				}
				mpcBlock->Branch(sCondition, pcTrue, pcFalse);
				return true;
			}
			mpcGenerator->ComparisonRange(pcBinary);
			if (!Value(pcBinary->mpcLeft, &sLeft) || !Value(pcBinary->mpcRight, &sRight) || !Compare(pcBinary->meOperator, &sLeft, &sRight, &sCondition))
			{
				return false;
			}
			mpcBlock->Branch(sCondition, pcTrue, pcFalse);
			return true;
		default:
			break;
		}
	}
	else if (pcExpression->GetKind() == CPPS_SimpleDeclaration)
	{
		//if (T x = value): the variable (a scalar) is the condition.
		psVariable = NULL;
		if (!Declaration((CCPPSyntaxSimpleDeclaration*)pcExpression, &psVariable))
		{
			return false;
		}
		if (!psVariable || !psVariable->psType->IsScalar() || !VariablePlace(psVariable, &sPlace) || !Load(&sPlace, &sValue) || !Truth(&sValue, &sCondition))
		{
			return Unsupported(__FILE__, __LINE__);
		}
		mpcBlock->Branch(sCondition, pcTrue, pcFalse);
		return true;
	}

	//An object is converted to bool (with its conversion function).
	if (!Value(pcExpression, &sValue) || !Truth(&sValue, &sCondition))
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
	SCPP65Type*		psType;
	SCPP65Function*	psFunction;
	CArraySyntax	apcArguments;
	int64			lliName;
	bool			bResult;

	lliName = mpcGenerator->GetOperatorName(eOperator, false);
	psType = mpcGenerator->ValueTypeOf(pcLeft);
	apcArguments.Init();
	if (psType && psType->IsClass() && mpcGenerator->CompleteClass(psType->psClass, NULL) && mpcGenerator->HasOperator(psType->psClass, lliName, pcRight ? 1 : 0))
	{
		if (pcRight)
		{
			apcArguments.Add(pcRight);
		}
		psFunction = mpcGenerator->FindOperator(psType->psClass, lliName, &apcArguments, pcLeft, mpcGenerator->IsConstObject(pcLeft));
		apcArguments.Kill();
		if (psFunction)
		{
			return OperatorCall(pcLeft, eOperator, pcRight, psValue);
		}
		apcArguments.Init();
	}

	//operator op(left, right), declared or from a function template.
	apcArguments.Add(pcLeft);
	if (pcRight)
	{
		apcArguments.Add(pcRight);
	}
	psFunction = mpcGenerator->FindFreeOperator(lliName, &apcArguments, pcLeft);
	if (!psFunction)
	{
		apcArguments.Kill();
		return Unsupported(__FILE__, __LINE__);
	}
	bResult = Call(psFunction, FunctionSymbol(psFunction), NULL, &apcArguments, psValue);
	apcArguments.Kill();
	return bResult;
}
