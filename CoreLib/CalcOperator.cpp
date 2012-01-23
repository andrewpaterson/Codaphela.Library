/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "CalcOperator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcOperator::Init(void)
{
	meOp = CO_invalid;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcOperator::Kill(void)
{
	meOp = CO_invalid;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcOperator::Set(ECalcOperator eOp)
{
	meOp = eOp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcOperator::Evaluate(CNumber* pcAnswer, CNumber* pcNumberLeft, CNumber* pcNumberRight)
{
	int	iLeft;
	int iRight;

	switch (meOp)
	{
		case CO_EqualTo:
			if (pcNumberLeft->Equals(pcNumberRight))
				pcAnswer->One();
			else
				pcAnswer->Zero();
			break;
		case CO_NotEqualTo:
			if (!pcNumberLeft->Equals(pcNumberRight))
				pcAnswer->One();
			else
				pcAnswer->Zero();
			break;
		case CO_GreaterThanEqualTo:
			if (pcNumberLeft->GreaterThanOrEquals(pcNumberRight))
				pcAnswer->One();
			else
				pcAnswer->Zero();
			break;
		case CO_LessThanEqualTo:
			if (pcNumberLeft->LessThanOrEquals(pcNumberRight))
				pcAnswer->One();
			else
				pcAnswer->Zero();
			break;
		case CO_LogicalOr:
			pcAnswer->Init(pcNumberLeft);
			pcAnswer->LogicalOr(pcNumberRight);
			break;
		case CO_LogicalAnd:
			pcAnswer->Init(pcNumberLeft);
			pcAnswer->LogicalAnd(pcNumberRight);
			break;
		case CO_Add:
			pcAnswer->Init(pcNumberLeft);
			pcAnswer->Add(pcNumberRight);
			break;
		case CO_Subtract:
			pcAnswer->Init(pcNumberLeft);
			pcAnswer->Subtract(pcNumberRight);
			break;
		case CO_Multiply:
			pcAnswer->Init(pcNumberLeft);
			pcAnswer->Multiply(pcNumberRight);
			break;
		case CO_Divide:
			pcAnswer->Init(pcNumberLeft);
			pcAnswer->Divide(pcNumberRight);
			break;
		case CO_Modulus:
			pcAnswer->Init(pcNumberLeft);
			pcAnswer->Modulus(pcNumberRight);
			break;
		case CO_LessThan:
			if (pcNumberLeft->LessThan(pcNumberRight))
				pcAnswer->One();
			else
				pcAnswer->Zero();
			break;
		case CO_GreaterThan:
			if (pcNumberLeft->GreaterThan(pcNumberRight))
				pcAnswer->One();
			else
				pcAnswer->Zero();
			break;
		case CO_LeftShift:
				iLeft = pcNumberLeft->IntValue();
				iRight = pcNumberRight->IntValue();
				pcAnswer->Init(iLeft << iRight);
			break;
		case CO_RightShift:
				iLeft = pcNumberLeft->IntValue();
				iRight = pcNumberRight->IntValue();
				pcAnswer->Init(iLeft >> iRight);
			break;
		case CO_BitwiseAnd:
				iLeft = pcNumberLeft->IntValue();
				iRight = pcNumberRight->IntValue();
				pcAnswer->Init(iLeft & iRight);
			break;
		case CO_BitwiseOr:
							iLeft = pcNumberLeft->IntValue();
				iRight = pcNumberRight->IntValue();
				pcAnswer->Init(iLeft | iRight);
			break;
		case CO_BitwiseXor:
				iLeft = pcNumberLeft->IntValue();
				iRight = pcNumberRight->IntValue();
				pcAnswer->Init(iLeft ^ iRight);
			break;
		default:
			gcUserError.Set("Unknown binary operator in CCalcOperator::Evaluate");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcOperator::Evaluate(CNumber* pcAnswer, CNumber* pcNumber)
{
	int	i;

	pcAnswer->Init(pcNumber);
	switch (meOp)
	{
	case CO_Increment:
		pcAnswer->Increment();
		break;
	case CO_Decrement:
		pcAnswer->Decrement();
		break;
	case CO_LogicalNot:
		pcAnswer->LogicalNot();
		break;
	case CO_BitwiseNot:
		i = pcNumber->IntValue();
		i = ~i;
		pcAnswer->Init(i);
		break;
	case CO_UnaryAdd:
		break;
	case CO_UnarySubtract:
		pcAnswer->Negate();
		break;
	default:
		gcUserError.Set("Unknown unary operator in CCalcOperator::Evaluate");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCalcOperator::IsExpression(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCalcOperator::IsOperator(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCalcOperator::IsBinary(void)
{
	switch (meOp)
	{
	case CO_EqualTo:
	case CO_NotEqualTo:
	case CO_GreaterThanEqualTo:
	case CO_LessThanEqualTo:
	case CO_LogicalOr:
	case CO_LogicalAnd:
	case CO_Add:
	case CO_Subtract:
	case CO_Multiply:
	case CO_Divide:
	case CO_Modulus:
	case CO_LessThan:
	case CO_GreaterThan:
	case CO_LeftShift:
	case CO_RightShift:
	case CO_BitwiseAnd:
	case CO_BitwiseOr:
	case CO_BitwiseXor:
		return TRUE;
	}
	return FALSE;

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCalcOperator::IsUnary(void)
{
	switch (meOp)
	{
	case CO_Increment:
	case CO_Decrement:
	case CO_LogicalNot:
	case CO_BitwiseNot:
	case CO_UnaryAdd:
	case CO_UnarySubtract:
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCalcOperator::IsAmbiguous(void)
{
	switch (meOp)
	{
	case CO_Add:
	case CO_Subtract:
	case CO_UnaryAdd:
	case CO_UnarySubtract:
		return TRUE;
	}
	return FALSE;
}