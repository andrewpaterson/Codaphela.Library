/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "PointerFunctions.h"
#include "ErrorHandler.h"
#include "Calculator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculator::Init(void)
{
	Init(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculator::Init(bool bUseUserError)
{
	macOperators.Init();

	macOperators.Add("++", CO_Increment,		  0);	
	macOperators.Add("--", CO_Decrement,		  0);	
	macOperators.Add("==", CO_EqualTo,			  6);	
	macOperators.Add("!=", CO_NotEqualTo,		  6);	
	macOperators.Add(">=", CO_GreaterThanEqualTo, 5);	
	macOperators.Add("<=", CO_LessThanEqualTo,	  5);	
	macOperators.Add("||", CO_LogicalOr,		  11);	
	macOperators.Add("&&", CO_LogicalAnd,		  10);  
	macOperators.Add("<<", CO_LeftShift,		  4);	
	macOperators.Add(">>", CO_RightShift,		  4);	
	macOperators.Add( "+", CO_Add,				  3);	
	macOperators.Add( "-", CO_Subtract,			  3);	
	macOperators.Add( "*", CO_Multiply,			  2);	
	macOperators.Add( "/", CO_Divide,			  2);	
	macOperators.Add( "%", CO_Modulus,			  2);	
	macOperators.Add( "!", CO_LogicalNot,		  1);	
	macOperators.Add( "&", CO_BitwiseAnd,		  7);	
	macOperators.Add( "|", CO_BitwiseOr,		  9);	
	macOperators.Add( "^", CO_BitwiseXor,		  8);	
	macOperators.Add( "<", CO_LessThan,			  5);	
	macOperators.Add( ">", CO_GreaterThan,		  5);	
	macOperators.Add( "~", CO_BitwiseNot,		  1);	

	mcAssignment.Init("=", CO_Assignment, 12);

	mbUseUserError = bUseUserError;
	mszError.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculator::Kill(void)
{
	mszError.Kill();
	mcAssignment.Kill();
	macOperators.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber CCalculator::Eval(CCalcExpression* pcExpression)
{
	CNumber				cAnswer;

	if (!HasError())
	{
		cAnswer = pcExpression->Evaluate();
		cAnswer.Clean();
	}
	else
	{
		cAnswer.NotANumber();
	}

	return cAnswer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCalcExpression* CCalculator::BuildExpression(CCalcObjectArray* papcExpressions)
{
	size					iIndex;
	CCalcOperator*			pcOperator;
	CCalcExpression*		pcOperand;
	CCalcObject*			pcObject;
	CCalcObject*			pcObjectLeft;
	CCalcObject*			pcObjectRight;
	CCalcUnaryExpression*	pcUnary;
	CCalcBinaryExpression*	pcBinary;
	CCalcExpression*		pcOperandLeft;
	CCalcExpression*		pcOperandRight;
	size					iOldUsedElements;
	CChars					szStart;
	bool					bUnary;

	szStart.Init();

	Dump(papcExpressions);

	iOldUsedElements = papcExpressions->NumElements();
	while (papcExpressions->NumElements() > 1)
	{
		if (papcExpressions->NumElements() > iOldUsedElements)
		{
			SetError(&szStart, papcExpressions, "Number of elements in expression INCREASED from [", "] to [", "] which should not be possible.");
			return NULL;
		}
		iOldUsedElements = papcExpressions->NumElements();

		iIndex = GetMinPrecedence(papcExpressions);
		if (iIndex == ARRAY_ELEMENT_NOT_FOUND)
		{
			SetError(&szStart, papcExpressions, "Confused trying to find order of precedence for inital [", "] with current [", "].");
			return NULL;
		}
		pcOperator = (CCalcOperator*)papcExpressions->GetPtr(iIndex);

		if (pcOperator->IsAmbiguous())
		{
			pcObject = (CCalcObject*)papcExpressions->SafeGetPtr(iIndex - 1);
			bUnary = false;
			if (pcObject == NULL)
			{
				bUnary = true;
			}
			else
			{
				if (pcObject->IsOperator())
				{
					bUnary = true;
				}
			}
			pcOperator->meOp = ResolveAmbiguity(pcOperator->meOp, bUnary);
		}

		if (pcOperator->IsUnary())
		{
			//For the time being always assume Right-to-Left associativity.
			pcObject = (CCalcObject*)papcExpressions->SafeGetPtr(iIndex+1);
			if (pcObject)
			{
				if (pcObject->IsExpression())
				{
					pcOperand = (CCalcExpression*)pcObject;
					pcUnary = NewMalloc<CCalcUnaryExpression>();
					pcUnary->Init();
					pcUnary->Set(pcOperand, pcOperator);
					papcExpressions->RemoveAt(iIndex + 1);
					papcExpressions->SetPtr(iIndex, pcUnary);
				}
				else
				{
					SetError(&szStart, papcExpressions, "Unary operator only works on expressions given inital [", "] and current [", "].");
					return NULL;
				}
			}
			else
			{
				SetError(&szStart, papcExpressions, "Unary operator needs right hand operand for inital [", "] with current [", "].");
				return NULL;
			}
		}
		else if (pcOperator->IsBinary())
		{
			pcObjectLeft = (CCalcObject*)papcExpressions->SafeGetPtr(iIndex - 1);
			if (pcObjectLeft)
			{
				pcObjectRight = (CCalcObject*)papcExpressions->SafeGetPtr(iIndex + 1);
				if (pcObjectRight)
				{
					if (pcObjectLeft->IsExpression() && pcObjectRight->IsExpression())
					{
						pcOperandLeft = (CCalcExpression*)pcObjectLeft;
						pcOperandRight = (CCalcExpression*)pcObjectRight;
						pcBinary = NewMalloc<CCalcBinaryExpression>();
						pcBinary->Init();
						pcBinary->Set(pcOperandLeft, pcOperator, pcOperandRight);
						papcExpressions->RemoveAt(iIndex + 1);
						papcExpressions->SetPtr(iIndex, pcBinary);
						papcExpressions->RemoveAt(iIndex - 1);
					}
					else
					{
						SetError(&szStart, papcExpressions, "Binary operator only works on expressions given inital [", "] and current [", "].");
						return NULL;
					}

				}
				else
				{
					SetError(&szStart, papcExpressions, "Binary operator needs right hand operand for inital [", "] with current [", "].");
					return NULL;
				}
			}
			else
			{
				SetError(&szStart, papcExpressions, "Binary operator needs left hand operand for inital [", "] with current [", "].");
				return NULL;
			}
		}
		else
		{
			SetError(&szStart, papcExpressions, "Don't know what style of operator this is [", "] with current [", "].");
			return NULL;
		}
	}
	
	szStart.Kill();
	if (papcExpressions->NumElements() == 1)
	{
		return (CCalcExpression*)papcExpressions->GetPtr(0);
	}
	else if (papcExpressions->NumElements() == 0)
	{
		SetError(&szStart, papcExpressions, "No elements remain in expression [", "] with current [", "].");
		return NULL;
	}
	else
	{
		SetError(&szStart, papcExpressions, "Too many elements remain in expression [", "] with current [", "].");
		return NULL;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCalculator::GetMinPrecedence(CCalcObjectArray* papcExpressions)
{
	size					i;
	CCalcObject*			pcObject;
	CCalcOperator*			pcOperator;
	uint16					iMinPrecedence;
	size					iMinIndex;
	uint16					iPrecedence;
	size					uiNumElements;
	CCalculatorOperator*	pcDefinition;

	iMinPrecedence = 12;
	iMinIndex = ARRAY_ELEMENT_NOT_FOUND;
	uiNumElements = papcExpressions->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcObject = (CCalcObject*)papcExpressions->GetPtr(i);
		if (pcObject->IsOperator())
		{
			pcOperator = (CCalcOperator*)pcObject;
			pcDefinition = macOperators.Get(pcOperator->meOp);
			iPrecedence = pcDefinition->GetPrecedence();
			if (iPrecedence < iMinPrecedence)
			{
				iMinPrecedence = iPrecedence;
				iMinIndex = i;
			}
		}
	}
	return iMinIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECalcOperator CCalculator::ResolveAmbiguity(ECalcOperator eOperator, bool bIsUnary)
{
	if (bIsUnary)
	{
		if (eOperator == CO_Add)
		{
			return CO_UnaryAdd;
		}
		else if (eOperator == CO_Subtract)
		{
			return CO_UnarySubtract;
		}
		else
		{
			return eOperator;
		}
	}
	else
	{
		if (eOperator == CO_UnaryAdd)
		{
			return CO_Add;
		}
		else if (eOperator == CO_UnarySubtract)
		{
			return CO_Subtract;
		}
		else
		{
			return eOperator;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCalculator::SetError(CChars* pszStart, CCalcObjectArray* papcExpressions, char* szLeft, char* szMiddle, char* szRight)
{
	CChars	szCurrent;
	CChars	sz;

	szCurrent.Init();
	Print(&szCurrent, papcExpressions);
	sz.InitList(szLeft, pszStart->Text(), szMiddle, szCurrent.Text(), szRight, NULL);
	SetError(sz.Text());
	sz.Kill();
	szCurrent.Kill();
	pszStart->Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculator::SetError(char* szError)
{
	mszError.Append(szError);
	if (mbUseUserError)
	{
		gcUserError.Set(szError);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCalculator::HasError(void)
{
	return !mszError.Empty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculator::Print(CChars* psz, CCalcObjectArray* papcExpressions)
{
	size			i;
	CCalcObject*	pcObject;
	size			uiNumElements;

	uiNumElements = papcExpressions->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcObject = (CCalcObject*)papcExpressions->GetPtr(i);
		pcObject->Print(psz);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculator::Print(CChars* psz, CCalcObject* pcExpression)
{
	pcExpression->Print(psz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCalculator::GetError(void)
{
	return mszError.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayCalculatorOperators* CCalculator::GetOperators(void)
{
	return &macOperators;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCalculatorOperator* CCalculator::GetAssignment(void)
{
	return &mcAssignment;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculator::Dump(CCalcObjectArray* papcExpressions)
{
	CChars	sz;
	
	sz.Init();
	Print(&sz, papcExpressions);;
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculator::Dump(CCalcObject* pcExpression)
{
	CChars	sz;

	sz.Init();
	Print(&sz, pcExpression);;
	sz.DumpKill();
}

