/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
	mszOperators.Init();
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

	maiPrecedence.Init();
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
	bool				bResult;
	CChars				sz;

	mszError.Reinit();

	mcParser.Init(szText);
	bResult = Expression(&pcExpression);
	mcParser.Kill();

	if (bResult)
	{
		cAnswer = pcExpression->Evaluate();
		cAnswer.Clean();
		SafeKill(pcExpression);
	}
	else
	{
		if (!HasError())
		{
			sz.Init("Cannot evaluate expression [");
			sz.Append(szText);
			sz.Append("]\n");
			SetError(sz.Text());
			sz.Kill();
		}
		cAnswer.NotANumber();
	}

	return cAnswer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCalculator::Expression(CCalcExpression** ppcExpression)
{
	bool					bOperator;
	bool					bOperand;
	bool					bFirst;
	CArrayIntAndPointer		cArray;
	CCalcOperator*			pcOperator;
	CCalcExpression*		pcOperand;

	cArray.Init();
	bFirst = true;
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
		if (HasError())
		{
			cArray.Kill();
			return false;
		}

		if (!bOperand && !bOperator)
		{
			if (bFirst)
			{
				cArray.Kill();
				return false;
			}
			else
			{
				BuildExpression(ppcExpression, &cArray);

				cArray.Kill();

				if (*ppcExpression == NULL)
				{
					return false;
				}
				else
				{
					return true;
				}
			}
		}
		cArray.Add(pcOperand, 0);
		bFirst = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCalculator::Operand(CCalcExpression** ppcExpression)
{
	bool					bReturn;
	CCalcParentheses*		pcParentheses;
	CCalcConstExpression*	pcConst;

	bReturn = Parentheses(&pcParentheses);
	if (bReturn)
	{
		*ppcExpression = pcParentheses;
		return true;
	}

	bReturn = Value(&pcConst);
	if (bReturn)
	{
		*ppcExpression = pcConst;
		return true;
	}

	bReturn = Identifier(&pcConst);
	if (bReturn)
	{
		*ppcExpression = pcConst;
		return true;
	}

	*ppcExpression = NULL;
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCalculator::Identifier(CCalcConstExpression** ppcConst)
{
	TRISTATE				tResult;
	CNumber					cNumber;
	size					iLength;
	char*					sz;

	tResult = mcParser.GetIdentifier(NULL, &iLength);
	if (tResult == TRITRUE)
	{
		sz = (char*)malloc(iLength+1);
		mcParser.GetIdentifier(sz);
		SafeFree(sz);
		*ppcConst = NewMalloc<CCalcConstExpression>();
		(*ppcConst)->SetValue(cNumber.Zero());
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCalculator::Value(CCalcConstExpression** ppcConst)
{
	TRISTATE	tResult;
	CNumber		cNumber;
	uint64		ulli;

	tResult = mcParser.GetHexadecimal(&ulli);
	if (tResult == TRITRUE)
	{
		*ppcConst = NewMalloc<CCalcConstExpression>();
		(*ppcConst)->SetValue(cNumber.Init((int32)ulli));  //This needs to properly take an int64.
		return true;
	}

	tResult = mcParser.GetNumber(&cNumber);
	if (tResult == TRITRUE)
	{
		tResult = mcParser.GetExactCharacter('L', false);
		if (tResult == false)
		{
			tResult = mcParser.GetExactCharacter('l', false);
		}
		*ppcConst = NewMalloc<CCalcConstExpression>();
		(*ppcConst)->SetValue(&cNumber);
		return true;
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCalculator::Operator(CCalcOperator** pcOperator)
{
	TRISTATE		tResult;
	size			i;
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
			*pcOperator = NewMalloc<CCalcOperator>();
			(*pcOperator)->Set(eOp);
			return true;
		}
		else if (tResult == TRIERROR)
		{
			eOp = CO_invalid;
			mcParser.PopPosition();
			*pcOperator = NULL;
			return false;
		}
	}

	eOp = CO_invalid;
	mcParser.PopPosition();
	*pcOperator = NULL;
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCalculator::Parentheses(CCalcParentheses** ppcParentheses)
{
	TRISTATE			tResult;
	bool				bReturn;
	CCalcExpression*	pcExpression;

	mcParser.PushPosition();
	tResult = mcParser.GetExactCharacter('(');
	if (tResult == TRITRUE)
	{
		tResult = mcParser.GetExactCharacter(')');
		if (tResult == TRITRUE)
		{
			mcParser.PassPosition();
			*ppcParentheses = NewMalloc<CCalcParentheses>();
			(*ppcParentheses)->SetExpression(NULL);
			return true;
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
					*ppcParentheses = NewMalloc<CCalcParentheses>();
					(*ppcParentheses)->SetExpression(pcExpression);
					return true;
				}
			}
			SafeKill(pcExpression);
			mcParser.PopPosition();
			*ppcParentheses = NULL;
			return false;
		}
	}
	else
	{
		mcParser.PopPosition();
		*ppcParentheses = NULL;
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCalculator::BuildExpression(CCalcExpression** ppcExpression, CArrayIntAndPointer* pcArray)
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
	Print(&szStart, pcArray);

	iOldUsedElements = pcArray->NumElements();
	while (pcArray->NumElements() > 1)
	{
		if (pcArray->NumElements() > iOldUsedElements)
		{
			return SetError(&szStart, pcArray, ppcExpression, "Number of elements in expression INCREASED from [", "] to [", "] which should not be possible.");
		}
		iOldUsedElements = pcArray->NumElements();

		iIndex = GetMinPrecedence(pcArray);
		if (iIndex == -1)
		{
			return SetError(&szStart, pcArray, ppcExpression, "Confused trying to find order of precedence for inital [", "] with current [", "].");
		}
		pcOperator = (CCalcOperator*)pcArray->GetPtr(iIndex);

		if (pcOperator->IsAmbiguous())
		{
			pcObject = (CCalcObject*)pcArray->SafeGetPtr(iIndex - 1);
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
			ResolveAmbiguity(pcOperator, bUnary);
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
					pcUnary = NewMalloc<CCalcUnaryExpression>();
					pcUnary->Set(pcOperand, pcOperator);
					pcArray->RemoveAt(iIndex+1);
					pcArray->Set(iIndex, pcUnary, 0);
				}
				else
				{
					return SetError(&szStart, pcArray, ppcExpression, "Unary operator only works on expressions given inital [", "] and current [", "].");
				}
			}
			else
			{
				return SetError(&szStart, pcArray, ppcExpression, "Unary operator needs right hand operand for inital [", "] with current [", "].");
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
						pcBinary = NewMalloc<CCalcBinaryExpression>();
						pcBinary->Set(pcOperandLeft, pcOperator, pcOperandRight);
						pcArray->RemoveAt(iIndex+1);
						pcArray->Set(iIndex, pcBinary, 0);
						pcArray->RemoveAt(iIndex-1);
					}
					else
					{
						return SetError(&szStart, pcArray, ppcExpression, "Binary operator only works on expressions given inital [", "] and current [", "].");
					}

				}
				else
				{
					return SetError(&szStart, pcArray, ppcExpression, "Binary operator needs right hand operand for inital [", "] with current [", "].");
				}
			}
			else
			{
				return SetError(&szStart, pcArray, ppcExpression, "Binary operator needs left hand operand for inital [", "] with current [", "].");
			}
		}
		else
		{
			return SetError(&szStart, pcArray, ppcExpression, "Don't know what style of operator this is [", "] with current [", "].");
		}
	}
	*ppcExpression = (CCalcExpression*)pcArray->GetPtr(0);
	szStart.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCalculator::GetMinPrecedence(CArrayIntAndPointer* pcArray)
{
	size				i;
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
void CCalculator::ResolveAmbiguity(CCalcOperator* pcOperator, bool bIsUnary)
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCalculator::SetError(CChars* pszStart, CArrayIntAndPointer* pcArray, CCalcExpression** ppcExpression, char* szLeft, char* szMiddle, char* szRight)
{
	CChars	szCurrent;
	CChars	sz;

	szCurrent.Init();
	Print(&szCurrent, pcArray);
	sz.InitList(szLeft, pszStart->Text(), szMiddle, szCurrent.Text(), szRight, NULL);
	SetError(sz.Text());
	sz.Kill();
	szCurrent.Kill();
	pszStart->Kill();
	*ppcExpression = NULL;
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
void CCalculator::Print(CChars* psz, CArrayIntAndPointer* pcArray)
{
	size			i;
	CCalcObject*	pcObject;

	for (i = 0; i < pcArray->NumElements(); i++)
	{
		pcObject = (CCalcObject*)pcArray->GetPtr(i);
		pcObject->Print(psz);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculator::Dump(CArrayIntAndPointer* pcArray)
{
	CChars sz;

	sz.Init();
	Print(&sz, pcArray);
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCalculator::GetError(void)
{
	return mszError.Text();
}

