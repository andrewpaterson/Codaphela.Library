#include <math.h>
#include "BaseLib/PointerFunctions.h"
#include "GerberMacroVariables.h"
#include "GerberExpression.h"


bool EvaluateGerberCalcExpression(CCalcExpression* pcExpression, CGerberMacroVariables* pcVariables, CNumber* pcResult);
bool PrintGerberCalcExpression(CCalcExpression* pcExpression, CChars* psz);


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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberExpression::IsSet(void)
{
	return mpcExpression != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberExpression::Print(CChars* psz)
{
	return PrintGerberCalcExpression(mpcExpression, psz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char GetGerberOperatorSymbol(CCalcOperator* pcOperator)
{
	switch (pcOperator->meOp)
	{
		case CO_Add:
		case CO_UnaryAdd:
			return '+';
		case CO_Subtract:
		case CO_UnarySubtract:
			return '-';
		case CO_Multiply:
			return 'x';
		case CO_Divide:
			return '/';
		default:
			return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int GetGerberOperatorPrecedence(CCalcOperator* pcOperator)
{
	//Higher binds tighter.  The Gerber calculator symbols bind / tighter than x (2 and 3) so the same order is used
	//here to print a tree that parses back to itself.
	switch (pcOperator->meOp)
	{
		case CO_Add:
		case CO_Subtract:
			return 1;
		case CO_Multiply:
			return 2;
		case CO_Divide:
			return 3;
		default:
			return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrintGerberCalcOperand(CCalcExpression* pcOperand, int iPrecedence, bool bRight, CChars* psz)
{
	CCalcBinaryExpression*	pcBinary;
	bool					bParentheses;
	int						iOperandPrecedence;

	//Parentheses are added where the operand would otherwise bind differently.  A unary operand on the right
	//is also parenthesised; "a+-b" is not in the Gerber expression grammar.
	bParentheses = false;
	if (pcOperand->IsBinaryExpression())
	{
		pcBinary = (CCalcBinaryExpression*)pcOperand;
		iOperandPrecedence = GetGerberOperatorPrecedence(pcBinary->mpcOp);
		bParentheses = bRight ? (iOperandPrecedence <= iPrecedence) : (iOperandPrecedence < iPrecedence);
	}
	else if (pcOperand->IsUnaryExpression())
	{
		bParentheses = bRight;
	}

	if (bParentheses)
	{
		psz->Append('(');
	}
	if (!PrintGerberCalcExpression(pcOperand, psz))
	{
		return false;
	}
	if (bParentheses)
	{
		psz->Append(')');
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrintGerberCalcExpression(CCalcExpression* pcExpression, CChars* psz)
{
	CCalcParentheses*		pcParentheses;
	CCalcUnaryExpression*	pcUnary;
	CCalcBinaryExpression*	pcBinary;
	char					cSymbol;
	int						iPrecedence;

	if (pcExpression == NULL)
	{
		return false;
	}

	if (pcExpression->IsConstExpression())
	{
		((CCalcConstExpression*)pcExpression)->mcValue.Print(psz);
		return true;
	}
	else if (pcExpression->IsVariable())
	{
		psz->Append(((CCalcVariable*)pcExpression)->Get());
		return true;
	}
	else if (pcExpression->IsParentheses())
	{
		pcParentheses = (CCalcParentheses*)pcExpression;
		psz->Append('(');
		if (!PrintGerberCalcExpression(pcParentheses->mpcExpression, psz))
		{
			return false;
		}
		psz->Append(')');
		return true;
	}
	else if (pcExpression->IsUnaryExpression())
	{
		pcUnary = (CCalcUnaryExpression*)pcExpression;
		cSymbol = GetGerberOperatorSymbol(pcUnary->mpcOp);
		if (cSymbol == 0)
		{
			return false;
		}
		psz->Append(cSymbol);
		if (pcUnary->mpcExpression->IsBinaryExpression() || pcUnary->mpcExpression->IsUnaryExpression())
		{
			psz->Append('(');
			if (!PrintGerberCalcExpression(pcUnary->mpcExpression, psz))
			{
				return false;
			}
			psz->Append(')');
			return true;
		}
		return PrintGerberCalcExpression(pcUnary->mpcExpression, psz);
	}
	else if (pcExpression->IsBinaryExpression())
	{
		pcBinary = (CCalcBinaryExpression*)pcExpression;
		cSymbol = GetGerberOperatorSymbol(pcBinary->mpcOp);
		iPrecedence = GetGerberOperatorPrecedence(pcBinary->mpcOp);
		if ((cSymbol == 0) || (iPrecedence == 0))
		{
			return false;
		}
		if (!PrintGerberCalcOperand(pcBinary->mpcLeft, iPrecedence, false, psz))
		{
			return false;
		}
		psz->Append(cSymbol);
		return PrintGerberCalcOperand(pcBinary->mpcRight, iPrecedence, true, psz);
	}
	return false;
}

