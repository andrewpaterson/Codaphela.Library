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
	mcSymbols.Init(false);

	mcErrors.Init(bUseUserError);

	mcVariables.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculator::Init(bool bEmptySymbols, bool bSkipWhitespace, bool bUseUserError)
{
	mcSymbols.Init(bEmptySymbols);

	mcErrors.Init(bUseUserError);

	mcVariables.Init();

	mbSkipWhitespace = bSkipWhitespace;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculator::Kill(void)
{
	mcVariables.Kill();
	mcErrors.Kill();
	mcSymbols.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber CCalculator::Eval(CCalcExpression* pcExpression)
{
	CNumber				cAnswer;

	mcSymbols.ValidateSymbols();

	if (!HasError() && (pcExpression != NULL))
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
CNumber CCalculator::Eval(void)
{
	CNumber				cAnswer;
	size				uiNumElements;
	CCalcExpression*	pcExpression;

	uiNumElements = mcVariables.NumExpressions();
	if (uiNumElements > 0)
	{
		pcExpression = mcVariables.GetExpression(uiNumElements - 1);
		cAnswer = Eval(pcExpression);
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
void CCalculator::ConvertToUnaryOperators(CCalcObjectArray* papcExpressions)
{
	size					i;
	CCalcOperator*			pcPreviousOperator;
	CCalcObject*			pcObject;
	CCalcObject*			pcPrevious;
	ECalcOperator			eOperator;

	if (!papcExpressions->IsEmpty())
	{
		i = papcExpressions->NumElements();
		pcPrevious = NULL;
		do
		{
			i--;
			pcObject = (CCalcObject*)papcExpressions->GetPtr(i);
			if (pcPrevious != NULL)
			{
				if (pcPrevious->IsOperator() && pcObject->IsOperator())
				{
					pcPreviousOperator = (CCalcOperator*)pcPrevious;
					eOperator = pcPreviousOperator->GetOperator();
					if (eOperator == CO_Add)
					{
						pcPreviousOperator->SetOperator(CO_UnaryAdd);
					}
					else if (eOperator == CO_Subtract)
					{
						pcPreviousOperator->SetOperator(CO_UnarySubtract);
					}
				}
			}
			pcPrevious = pcObject;

		} while (i != 0);

		if (pcPrevious != NULL)
		{
			if (pcPrevious->IsOperator())
			{
				pcPreviousOperator = (CCalcOperator*)pcPrevious;
				eOperator = pcPreviousOperator->GetOperator();
				if (eOperator == CO_Add)
				{
					pcPreviousOperator->SetOperator(CO_UnaryAdd);
				}
				else if (eOperator == CO_Subtract)
				{
					pcPreviousOperator->SetOperator(CO_UnarySubtract);
				}
			}
		}
	}
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

	ConvertToUnaryOperators(papcExpressions);

	szStart.Init();
	Print(&szStart, papcExpressions);

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
					pcUnary->Init(GetErrors());
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
						pcBinary->Init(GetErrors());
						pcBinary->Set(pcOperandLeft, pcOperator, pcOperandRight, mbSkipWhitespace);
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

	iMinPrecedence = mcSymbols.GetMaxPrecedence() + 1;
	iMinIndex = ARRAY_ELEMENT_NOT_FOUND;
	uiNumElements = papcExpressions->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcObject = (CCalcObject*)papcExpressions->GetPtr(i);
		if (pcObject->IsOperator())
		{
			pcOperator = (CCalcOperator*)pcObject;
			pcDefinition = mcSymbols.GetOperator(pcOperator->meOp);
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
void CCalculator::StripParentheses(CCalcExpression** ppcExpression)
{
	CCalcParentheses*	pcChild;

	for (;;)
	{
		if ((*ppcExpression)->IsParentheses())
		{
			pcChild = (CCalcParentheses*)*ppcExpression;
			*ppcExpression = pcChild->mpcExpression;
			SafeFree(pcChild);
		}
		else
		{
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculator::RecurseCleanExpression(CCalcExpression* pcExpression)
{
	CCalcParentheses*		pcParentheses;
	CCalcBinaryExpression*	pcBinary;
	CCalcUnaryExpression*	pcUnary;

	if (pcExpression->IsParentheses())
	{
		pcParentheses = (CCalcParentheses*)pcExpression;
		StripParentheses(&pcParentheses->mpcExpression);
		CleanExpression(pcParentheses->mpcExpression);
	}
	else if (pcExpression->IsBinaryExpression())
	{
		pcBinary = (CCalcBinaryExpression*)pcExpression;
		StripParentheses(&pcBinary->mpcLeft);
		StripParentheses(&pcBinary->mpcRight);
		CleanExpression(pcBinary->mpcLeft);
		CleanExpression(pcBinary->mpcRight);
	}
	else if (pcExpression->IsUnaryExpression())
	{
		pcUnary = (CCalcUnaryExpression*)pcExpression;
		StripParentheses(&pcUnary->mpcExpression);
		CleanExpression(pcUnary->mpcExpression);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCalcExpression* CCalculator::CleanExpression(CCalcExpression* pcExpression)
{
	RecurseCleanExpression(pcExpression);
	if (pcExpression->IsParentheses())
	{
		StripParentheses(&pcExpression);
	}
	return pcExpression;
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
	mcErrors.SetError(szError);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCalculator::HasError(void)
{
	return mcErrors.HasError();
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
void CCalculator::Print(CChars* psz)
{
	mcVariables.Print(psz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCalculator::GetError(void)
{
	return mcErrors.GetError();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculator::ClearError(void)
{
	mcErrors.Kill();
	mcErrors.Init(mcErrors.UseUserError());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayCalculatorOperators* CCalculator::GetOperators(void)
{
	return mcSymbols.GetOperators();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCalculatorOperator* CCalculator::GetAssignment(void)
{
	return mcSymbols.GetAssignment();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCalculatorVariables* CCalculator::GetVariables(void)
{
	return &mcVariables;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCalculatorSymbols* CCalculator::GetSymbols(void)
{
	return &mcSymbols;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCalculator::IsSkipWhitespace(void)
{
	return mbSkipWhitespace;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCalculatorError* CCalculator::GetErrors(void)
{
	return &mcErrors;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculator::Add(CCalcVariableDefinition* pcVariableDefinition)
{
	mcVariables.Add(pcVariableDefinition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculator::Add(CCalcExpression* pcExpression)
{
	mcVariables.Add(pcExpression);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculator::Add(CCalcObject* pcObject)
{
	if (pcObject)
	{
		if (pcObject->IsExpression())
		{
			Add((CCalcExpression*)pcObject);
		}
		else if (pcObject->IsVariableDefinition())
		{
			Add((CCalcVariableDefinition*)pcObject);
		}
	}
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

