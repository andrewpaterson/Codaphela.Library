#include "CalculatorError.h"
#include "CalcVariableDefinition.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcVariableDefinition::Init(CCalculatorError* pcError)
{
	mpcError = pcError;
	mpcVariable = NULL;
	mpcExpression = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcVariableDefinition::Kill(void)
{
	SafeKill(mpcVariable);
	SafeKill(mpcExpression);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcVariableDefinition::Set(CCalcVariable* pcVariable, CCalcExpression* pcExpression)
{
	mpcVariable = pcVariable;
	mpcExpression = pcExpression;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcVariableDefinition::Print(CChars* psz)
{
	mpcVariable->Print(psz);
	psz->Append(" ");
	psz->Append("=");
	psz->Append(" ");
	mpcExpression->Print(psz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCalcVariableDefinition::GetName(void)
{
	return mpcVariable->Get();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCalcExpression* CCalcVariableDefinition::GetExpression(void)
{
	return mpcExpression;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCalcVariableDefinition::IsExpression(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCalcVariableDefinition::IsOperator(void)
{
	return false;
}

