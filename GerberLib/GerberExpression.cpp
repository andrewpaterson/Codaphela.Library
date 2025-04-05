#include "GerberExpression.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberExpression::Init(void)
{
	CCalculatorSymbols* pcSymbols;

	mcCalculator.Init(true, false, false);
	pcSymbols = mcCalculator.GetSymbols();
	pcSymbols->SetOperator("+", CO_Add, 4);
	pcSymbols->SetOperator("-", CO_Subtract, 4);
	pcSymbols->SetOperator("x", CO_Multiply, 3);
	pcSymbols->SetOperator("/", CO_Divide, 2);
	pcSymbols->SetOperator("=", CO_Assignment, 5);

	mpcExpression = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberExpression::Kill(void)
{
	mcCalculator.Kill();
	mpcExpression = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberExpression::SetCalcExpression(CCalcExpression* pcExpression)
{
	mpcExpression = pcExpression;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCalculator* CGerberExpression::GetCalculator(void)
{
	return &mcCalculator;
}

