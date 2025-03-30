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
		macOperators.Add("++", CO_Increment, 0);
		macOperators.Add("--", CO_Decrement, 0);
		macOperators.Add("==", CO_EqualTo, 6);
		macOperators.Add("!=", CO_NotEqualTo, 6);
		macOperators.Add(">=", CO_GreaterThanEqualTo, 5);
		macOperators.Add("<=", CO_LessThanEqualTo, 5);
		macOperators.Add("||", CO_LogicalOr, 11);
		macOperators.Add("&&", CO_LogicalAnd, 10);
		macOperators.Add("<<", CO_LeftShift, 4);
		macOperators.Add(">>", CO_RightShift, 4);
		macOperators.Add("+", CO_Add, 3);
		macOperators.Add("-", CO_Subtract, 3);
		macOperators.Add("*", CO_Multiply, 2);
		macOperators.Add("/", CO_Divide, 2);
		macOperators.Add("%", CO_Modulus, 2);
		macOperators.Add("^", CO_Power, 2);
		macOperators.Add("!", CO_LogicalNot, 1);
		macOperators.Add("&", CO_BitwiseAnd, 7);
		macOperators.Add("|", CO_BitwiseOr, 9);
		macOperators.Add("^", CO_BitwiseXor, 8);
		macOperators.Add("<", CO_LessThan, 5);
		macOperators.Add(">", CO_GreaterThan, 5);
		macOperators.Add("~", CO_BitwiseNot, 1);
		macOperators.Add("+", CO_UnaryAdd, 1);
		macOperators.Add("-", CO_UnarySubtract, 1);
		macOperators.Add("=", CO_Assignment, 13);
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
bool CCalculatorSymbols::Validate(void)
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

