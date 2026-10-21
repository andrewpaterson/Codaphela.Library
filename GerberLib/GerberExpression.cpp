#include <math.h>
#include "BaseLib/PointerFunctions.h"
#include "GerberMacroVariables.h"
#include "GerberExpression.h"


bool EvaluateGerberCalcExpression(CCalcExpression* pcExpression, CGerberMacroVariables* pcVariables, CNumber* pcResult);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberExpression::Init(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace, bool bUseUserError)
{
	mcCalculator.Init(pcSymbols, bSkipWhitespace, bUseUserError);
	mpcExpression = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberExpression::Kill(void)
{
	mcCalculator.Kill();
	SafeKill(mpcExpression);
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CGerberExpression::GetConstNumber(void)
{
	CCalcConstExpression*	pcConst;

	if (mpcExpression->IsConstExpression())
	{
		pcConst = (CCalcConstExpression*)mpcExpression;
		return pcConst->GetNumber();
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CGerberExpression::DoubleValue(void)
{
	CCalcConstExpression*	pcConst;
	CNumber*				pcNumber;

	if (mpcExpression->IsConstExpression())
	{
		pcConst = (CCalcConstExpression*)mpcExpression;
		pcNumber = pcConst->GetNumber();
		if (pcNumber != NULL)
		{
			return pcNumber->DoubleValue();
		}
	}
	return NAN;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberExpression::Evaluate(CGerberMacroVariables* pcVariables, CNumber* pcResult)
{
	//An optional parameter that was omitted, such as the rotation of a circle, is 0.
	if (mpcExpression == NULL)
	{
		pcResult->Zero();
		return true;
	}
	return EvaluateGerberCalcExpression(mpcExpression, pcVariables, pcResult);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool EvaluateGerberCalcExpression(CCalcExpression* pcExpression, CGerberMacroVariables* pcVariables, CNumber* pcResult)
{
	CCalcConstExpression*	pcConst;
	CCalcParentheses*		pcParentheses;
	CCalcUnaryExpression*	pcUnary;
	CCalcBinaryExpression*	pcBinary;
	CCalcVariable*			pcVariable;
	CNumber					cLeft;
	CNumber					cRight;
	char*					szName;
	uint32					uiVariable;

	//The calculator binds variables to its own definitions when parsing so the tree is walked here with the macro's variables.
	if (pcExpression == NULL)
	{
		return false;
	}

	if (pcExpression->IsConstExpression())
	{
		pcConst = (CCalcConstExpression*)pcExpression;
		pcResult->Init(&pcConst->mcValue);
	}
	else if (pcExpression->IsParentheses())
	{
		pcParentheses = (CCalcParentheses*)pcExpression;
		if (!EvaluateGerberCalcExpression(pcParentheses->mpcExpression, pcVariables, pcResult))
		{
			return false;
		}
	}
	else if (pcExpression->IsUnaryExpression())
	{
		pcUnary = (CCalcUnaryExpression*)pcExpression;
		if (!EvaluateGerberCalcExpression(pcUnary->mpcExpression, pcVariables, &cRight))
		{
			return false;
		}
		pcUnary->mpcOp->Evaluate(pcResult, &cRight);
	}
	else if (pcExpression->IsBinaryExpression())
	{
		pcBinary = (CCalcBinaryExpression*)pcExpression;
		if (!EvaluateGerberCalcExpression(pcBinary->mpcLeft, pcVariables, &cLeft))
		{
			return false;
		}
		if (!EvaluateGerberCalcExpression(pcBinary->mpcRight, pcVariables, &cRight))
		{
			return false;
		}
		pcBinary->mpcOp->Evaluate(pcResult, &cLeft, &cRight);
	}
	else if (pcExpression->IsVariable())
	{
		//Macro variables are $ followed by digits.
		pcVariable = (CCalcVariable*)pcExpression;
		szName = pcVariable->Get();
		if (szName[0] != '$')
		{
			return false;
		}
		uiVariable = 0;
		szName++;
		while ((*szName >= '0') && (*szName <= '9'))
		{
			uiVariable = uiVariable * 10 + (*szName - '0');
			szName++;
		}
		pcVariables->Get(uiVariable, pcResult);
	}
	else
	{
		return false;
	}

	if (pcResult->IsNAN() || pcResult->IsDivisionByZero())
	{
		return false;
	}
	return true;
}

