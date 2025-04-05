#include "Logger.h"
#include "CalculatorSymbols.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculatorSymbols::Init(bool bEmpty)
{
	macOperators.Init();

	macOperators.Add();
	if (!bEmpty)
	{
		macOperators.Add("++", CO_Increment, 2);
		macOperators.Add("--", CO_Decrement, 2);
		macOperators.Add("==", CO_EqualTo, 8);
		macOperators.Add("!=", CO_NotEqualTo, 8);
		macOperators.Add(">=", CO_GreaterThanEqualTo, 7);
		macOperators.Add("<=", CO_LessThanEqualTo, 7);
		macOperators.Add("||", CO_LogicalOr, 13);
		macOperators.Add("&&", CO_LogicalAnd, 12);
		macOperators.Add("<<", CO_LeftShift, 6);
		macOperators.Add(">>", CO_RightShift, 6);
		macOperators.Add("^^", CO_Power, 1);
		macOperators.Add("+", CO_Add, 5);
		macOperators.Add("-", CO_Subtract, 5);
		macOperators.Add("*", CO_Multiply, 4);
		macOperators.Add("/", CO_Divide, 3);
		macOperators.Add("%", CO_Modulus, 3);
		macOperators.Add("!", CO_LogicalNot, 2);
		macOperators.Add("&", CO_BitwiseAnd, 9);
		macOperators.Add("|", CO_BitwiseOr, 11);
		macOperators.Add("^", CO_BitwiseXor, 10);
		macOperators.Add("<", CO_LessThan, 7);
		macOperators.Add(">", CO_GreaterThan, 7);
		macOperators.Add("~", CO_BitwiseNot, 2);
		macOperators.Add("+", CO_UnaryAdd, 0);
		macOperators.Add("-", CO_UnarySubtract, 0);
		macOperators.Add("=", CO_Assignment, 14);
	}
	else
	{
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
		macOperators.Add();
	}

	mpcAssignment = macOperators.Get(CO_Assignment);

	mbValidated = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCalculatorSymbols::ValidateSymbols(void)
{
	size					uiNumElments;
	size					i;
	CCalculatorOperator*	pcOperator;
	ECalcOperator			eOp;
	uint16					uiMaxPrecedence;

	uiMaxPrecedence = 0;
	if (!mbValidated)
	{
		uiNumElments = macOperators.NumElements();
		for (i = CO_Invalid+1; i <= CO_Assignment; i++)
		{
			pcOperator = macOperators.SafeGet(i);
			eOp = pcOperator->GetOperator();
			if (eOp != CO_Invalid)
			{
				if (eOp != i)
				{
					gcLogger.Error2(__METHOD__, " Operator [", SizeToString(eOp), "] does not match operator expected index [", SizeToString(i), "].", NULL);
					return false;
				}
			}
		}
		mbValidated = true;

		for (i = CO_Invalid + 1; i <= CO_Assignment; i++)
		{
			pcOperator = macOperators.SafeGet(i);
			eOp = pcOperator->GetOperator();
			if (eOp != CO_Invalid)
			{
				if (uiMaxPrecedence < pcOperator->GetPrecedence())
				{
					uiMaxPrecedence = pcOperator->GetPrecedence();
				}
			}
		}

		muiMaxPrecedence = uiMaxPrecedence;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculatorSymbols::Kill(void)
{
	macOperators.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCalculatorOperator* CCalculatorSymbols::GetOperator(ECalcOperator eOp)
{
	return macOperators.Get(eOp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCalculatorSymbols::SetOperator(char* szSymbol, ECalcOperator eOp, uint16 uiPrecedence)
{
	CCalculatorOperator*	pcOperator;

	pcOperator = macOperators.SafeGet(eOp);
	if (pcOperator)
	{
		pcOperator->Init(szSymbol, eOp, uiPrecedence);
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
CArrayCalculatorOperators* CCalculatorSymbols::GetOperators(void)
{
	return &macOperators;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCalculatorOperator* CCalculatorSymbols::GetAssignment(void)
{
	return mpcAssignment;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CCalculatorSymbols::GetMaxPrecedence(void)
{
	return muiMaxPrecedence;
}

