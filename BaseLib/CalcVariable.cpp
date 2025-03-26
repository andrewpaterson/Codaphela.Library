#include "CalculatorVariables.h"
#include "CalculatorError.h"
#include "CalcVariable.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcVariable::Init(CCalculatorError* pcError)
{
	mpcError = pcError;
	mszName.Init();
	mpcVariableDefinitions = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcVariable::Kill(void)
{
	mpcVariableDefinitions = NULL;
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcVariable::Set(char* szName, CCalculatorVariables* pcVariableDefinitions)
{
	mszName.Set(szName);
	mpcVariableDefinitions = pcVariableDefinitions;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber	CCalcVariable::Evaluate(void)
{
	CCalcVariableDefinition*	pcVariableDefinition;
	CCalcExpression*			pcExpression;
	CNumber						cNumber;
	CChars						sz;

	pcVariableDefinition = mpcVariableDefinitions->Get(mszName.Text());
	if (pcVariableDefinition)
	{
		pcExpression = pcVariableDefinition->GetExpression();

		cNumber = pcExpression->Evaluate();
		return cNumber;
	}
	else
	{
		//AssertString("Variable [x] is not assigned.", cCalculator.GetError());
		sz.Init("Variable [");
		sz.Append(mszName.Text());
		sz.Append("] is not assigned.");
		mpcError->SetError(sz.Text());
		sz.Kill();
		cNumber.NotANumber();
		return cNumber;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcVariable::Print(CChars* psz)
{
	psz->Append(&mszName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCalcVariable::Get(void)
{
	return mszName.Text();
}

