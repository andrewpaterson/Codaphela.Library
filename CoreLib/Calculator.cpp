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
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/ErrorHandler.h"
#include "Calculator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculator::Init(void)
{
	mszOperators.Init(22);
	mszOperators.Add("++");
	mszOperators.Add("--");
	mszOperators.Add("==");
	mszOperators.Add("!=");
	mszOperators.Add(">=");
	mszOperators.Add("<=");
	mszOperators.Add("||");
	mszOperators.Add("&&");
	mszOperators.Add("<<");
	mszOperators.Add(">>");
	mszOperators.Add("+");
	mszOperators.Add("-");
	mszOperators.Add("*");
	mszOperators.Add("/");
	mszOperators.Add("%");
	mszOperators.Add("!");
	mszOperators.Add("&");
	mszOperators.Add("|");
	mszOperators.Add("^");
	mszOperators.Add("<");
	mszOperators.Add(">");
	mszOperators.Add("~");

	maiPrecedence.Init(22);
	maiPrecedence.Add(0);	// CO_Increment,
	maiPrecedence.Add(0);	// CO_Decrement,
	maiPrecedence.Add(6);	// CO_EqualTo,
	maiPrecedence.Add(6);	// CO_NotEqualTo,
	maiPrecedence.Add(5);	// CO_GreaterThanEqualTo,
	maiPrecedence.Add(5);	// CO_LessThanEqualTo,
	maiPrecedence.Add(11);	// CO_LogicalOr,
	maiPrecedence.Add(10);  // CO_LogicalAnd,
	maiPrecedence.Add(4);	// CO_LeftShift,
	maiPrecedence.Add(4);	// CO_RightShift,
	maiPrecedence.Add(3);	// CO_Add,
	maiPrecedence.Add(3);	// CO_Subtract,
	maiPrecedence.Add(2);	// CO_Multiply,
	maiPrecedence.Add(2);	// CO_Divide,
	maiPrecedence.Add(2);	// CO_Modulus,
	maiPrecedence.Add(1);	// CO_LogicalNot,
	maiPrecedence.Add(7);	// CO_BitwiseAnd,
	maiPrecedence.Add(9);	// CO_BitwiseOr,
	maiPrecedence.Add(8);	// CO_BitwiseXor,
	maiPrecedence.Add(5);	// CO_LessThan,
	maiPrecedence.Add(5);	// CO_GreaterThan,
	maiPrecedence.Add(1);	// CO_BitwiseNot,
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculator::Kill(void)
{
	maiPrecedence.Kill();
	mszOperators.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber CCalculator::Eval(char* szText)
{
	CCalcExpression*	pcExpression;
	CNumber				cAnswer;
	BOOL				bResult;
	CChars				sz;

	mcParser.Init(szText);
	bResult = Expression(&pcExpression);
	mcParser.Kill();

	if (bResult)
	{
		cAnswer = pcExpression->Evaluate();
		SafeKill(pcExpression);
	}
	else
	{
		sz.Init("Cannot evaluate expression [");
		sz.Append(szText);
		sz.Append("]\n");
		gcUserError.Set(sz.Text());
		sz.Kill();
		cAnswer.Zero();
	}

	return cAnswer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCalculator::Expression(CCalcExpression** ppcExpression)
{
	BOOL				bOperator;
	BOOL				bOperand;
	BOOL				bFirst;
	CArrayPointer		cArray;
	CCalcOperator*		pcOperator;
	CCalcExpression*	pcOperand;

	cArray.Init(16);
	bFirst = TRUE;
	for (;;)
	{
		for (;;)
		{
			bOperator = Operator(&pcOperator);
			if (!bOperator)
			{
				break;
			}
			cArray.Add(pcOperator, 0);
		}

		bOperand = Operand(&pcOperand);
		if (!bOperand && !bOperator)
		{
			if (bFirst)
			{
				cArray.Kill();
				return FALSE;
			}
			else
			{
				BuildExpression(ppcExpression, &cArray);

				cArray.Kill();

				if (*ppcExpression == NULL)
				{
					return FALSE;
				}
				else
				{
					return TRUE;
				}
			}
		}
		cArray.Add(pcOperand, 0);
		bFirst = FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCalculator::Operand(CCalcExpression** ppcExpression)
{
	BOOL					bReturn;
	CCalcParentheses*		pcParentheses;
	CCalcConstExpression*	pcConst;

	bReturn = Parentheses(&pcParentheses);
	if (bReturn)
	{
		*ppcExpression = pcParentheses;
		return TRUE;
	}

	bReturn = Value(&pcConst);
	if (bReturn)
	{
		*ppcExpression = pcConst;
		return TRUE;
	}

	bReturn = Identifier(&pcConst);
	if (bReturn)
	{
		*ppcExpression = pcConst;
		return TRUE;
	}

	*ppcExpression = NULL;
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCalculator::Identifier(CCalcConstExpression** ppcConst)
{
	TRISTATE				tResult;
	CNumber					cNumber;
	int						iLength;
	char*					sz;

	tResult = mcParser.GetIdentifier(NULL, &iLength);
	if (tResult == TRITRUE)
	{
		sz = (char*)malloc(iLength+1);
		mcParser.GetIdentifier(sz);
		free(sz);
		*ppcConst = Malloc(CCalcConstExpression);
		(*ppcConst)->SetValue(cNumber.Zero());
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCalculator::Value(CCalcConstExpression** ppcConst)
{
	TRISTATE				tResult;
	CNumber					cNumber;
	unsigned long long int	ulli;

	tResult = mcParser.GetHexadecimal(&ulli);
	if (tResult == TRITRUE)
	{
		*ppcConst = Malloc(CCalcConstExpression);
		(*ppcConst)->SetValue(cNumber.Init((int)ulli));
		return TRUE;
	}

	tResult = mcParser.GetNumber(&cNumber);
	if (tResult == TRITRUE)
	{
		mcParser.GetExactCharacter('L', FALSE);
		*ppcConst = Malloc(CCalcConstExpression);
		(*ppcConst)->SetValue(&cNumber);
		return TRUE;
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCalculator::Operator(CCalcOperator** pcOperator)
{
	TRISTATE		tResult;
	int				i;
	char*			szSimpleOp;
	ECalcOperator	eOp;

	mcParser.PushPosition();

	for (i = 0; i < mszOperators.NumElements(); i++)
	{
		szSimpleOp = mszOperators.Get(i)->Text();
		tResult = mcParser.GetExactCharacterSequence(szSimpleOp);
		if (tResult == TRITRUE)
		{
			eOp = (ECalcOperator)i;
			mcParser.PassPosition();
			*pcOperator = Malloc(CCalcOperator);
			(*pcOperator)->Set(eOp);
			return TRUE;
		}
		else if (tResult == TRIERROR)
		{
			eOp = CO_invalid;
			mcParser.PopPosition();
			*pcOperator = NULL;
			return FALSE;
		}
	}

	eOp = CO_invalid;
	mcParser.PopPosition();
	*pcOperator = NULL;
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCalculator::Parentheses(CCalcParentheses** ppcParentheses)
{
	TRISTATE			tResult;
	BOOL				bReturn;
	CCalcExpression*	pcExpression;

	mcParser.PushPosition();
	tResult = mcParser.GetExactCharacter('(');
	if (tResult == TRITRUE)
	{
		tResult = mcParser.GetExactCharacter(')');
		if (tResult == TRITRUE)
		{
			mcParser.PassPosition();
			*ppcParentheses = Malloc(CCalcParentheses);
			(*ppcParentheses)->SetExpression(NULL);
			return TRUE;
		}
		else
		{
			bReturn = Expression(&pcExpression);
			if (bReturn)
			{
				tResult = mcParser.GetExactCharacter(')');
				if (tResult == TRITRUE)
				{
					mcParser.PassPosition();
					*ppcParentheses = Malloc(CCalcParentheses);
					(*ppcParentheses)->SetExpression(pcExpression);
					return TRUE;
				}
			}
			SafeKill(pcExpression);
			mcParser.PopPosition();
			*ppcParentheses = NULL;
			return FALSE;
		}
	}
	else
	{
		mcParser.PopPosition();
		*ppcParentheses = NULL;
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculator::BuildExpression(CCalcExpression** ppcExpression, CArrayPointer* pcArray)
{
	int						iIndex;
	CCalcOperator*			pcOperator;
	CCalcExpression*		pcOperand;
	CCalcObject*			pcObject;
	CCalcObject*			pcObjectLeft;
	CCalcObject*			pcObjectRight;
	CCalcUnaryExpression*	pcUnary;
	CCalcBinaryExpression*	pcBinary;
	CCalcExpression*		pcOperandLeft;
	CCalcExpression*		pcOperandRight;
	int						iOldUsedElements;

	iOldUsedElements = pcArray->NumElements();
	while (pcArray->NumElements() > 1)
	{
		if (pcArray->NumElements() > iOldUsedElements)
		{
			gcUserError.Set("Number of elements in expression INCREASED?");
			*ppcExpression = NULL;					
			return;
		}
		iOldUsedElements = pcArray->NumElements();

		iIndex = GetMinPrecedence(pcArray);
		if (iIndex == -1)
		{
			gcUserError.Set("Confused trying to find order of precedence");
			*ppcExpression = NULL;					
			return;
		}
		pcOperator = (CCalcOperator*)pcArray->GetPtr(iIndex);

		if (pcOperator->IsAmbiguous())
		{
			ResolveAmbiguity(pcOperator, pcArray->SafeGetPtr(iIndex-1) == NULL);
		}

		if (pcOperator->IsUnary())
		{
			//For the time being always assume Right-to-Left associativity.
			pcObject = (CCalcObject*)pcArray->SafeGetPtr(iIndex+1);
			if (pcObject)
			{
				if (pcObject->IsExpression())
				{
					pcOperand = (CCalcExpression*)pcObject;
					pcUnary = Malloc(CCalcUnaryExpression);
					pcUnary->Set(pcOperand, pcOperator);
					pcArray->RemoveAt(iIndex+1);
					pcArray->Set(iIndex, pcUnary, 0);
				}
				else
				{
					gcUserError.Set("Unary operator only works on expressions.");
					*ppcExpression = NULL;					
					return;
				}
			}
			else
			{
				*ppcExpression = NULL;
				gcUserError.Set("Unary operator needs right hand operand.");
			}
		}
		else if (pcOperator->IsBinary())
		{
			pcObjectLeft = (CCalcObject*)pcArray->SafeGetPtr(iIndex-1);
			if (pcObjectLeft)
			{
				pcObjectRight = (CCalcObject*)pcArray->SafeGetPtr(iIndex+1);
				if (pcObjectRight)
				{
					if (pcObjectLeft->IsExpression() && pcObjectRight->IsExpression())
					{
						pcOperandLeft = (CCalcExpression*)pcObjectLeft;
						pcOperandRight = (CCalcExpression*)pcObjectRight;
						pcBinary = Malloc(CCalcBinaryExpression);
						pcBinary->Set(pcOperandLeft, pcOperator, pcOperandRight);
						pcArray->RemoveAt(iIndex+1);
						pcArray->Set(iIndex, pcBinary, 0);
						pcArray->RemoveAt(iIndex-1);
					}
					else
					{
						gcUserError.Set("Binary operator only works on expressions.");
						*ppcExpression = NULL;					
						return;
					}

				}
				else
				{
					gcUserError.Set("Binary operator needs right hand operand.");
					*ppcExpression = NULL;					
					return;
				}
			}
			else
			{
				gcUserError.Set("Binary operator needs left hand operand.");
				*ppcExpression = NULL;					
				return;
			}
		}
		else
		{
			gcUserError.Set("Don't know what style of operator this is.");
			*ppcExpression = NULL;					
			return;
		}
	}
	*ppcExpression = (CCalcExpression*)pcArray->GetPtr(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCalculator::GetMinPrecedence(CArrayPointer* pcArray)
{
	int					i;
	CCalcObject*		pcObject;
	CCalcOperator*		pcOperator;
	int					iMinPrecedence;
	int					iMinIndex;
	int					iPrecedence;

	iMinPrecedence = 12;
	iMinIndex = -1;
	for (i = 0; i < pcArray->NumElements(); i++)
	{
		pcObject = (CCalcObject*)pcArray->GetPtr(i);
		if (pcObject->IsOperator())
		{
			pcOperator = (CCalcOperator*)pcObject;
			iPrecedence = maiPrecedence.GetValue(pcOperator->meOp);
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
void CCalculator::ResolveAmbiguity(CCalcOperator* pcOperator, BOOL bIsUnary)
{
	if (bIsUnary)
	{
		if (pcOperator->meOp == CO_Add)
		{
			pcOperator->meOp = CO_UnaryAdd;
		}
		else if (pcOperator->meOp == CO_Subtract)
		{
			pcOperator->meOp = CO_UnarySubtract;
		}
	}
	else
	{
		if (pcOperator->meOp == CO_UnaryAdd)
		{
			pcOperator->meOp = CO_Add;
		}
		else if (pcOperator->meOp == CO_UnarySubtract)
		{
			pcOperator->meOp = CO_Subtract;
		}
	}
}

