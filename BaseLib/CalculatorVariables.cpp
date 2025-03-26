#include "CalculatorVariables.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculatorVariables::Init(void)
{
	mapcExpressionDefintions.Init();
	mmszpcVariables.Init(true, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculatorVariables::Kill(void)
{
	size			uiNumElements;
	CCalcObject*	pcCalcObject;
	size			i;

	mmszpcVariables.Kill();

	uiNumElements = mapcExpressionDefintions.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcCalcObject = mapcExpressionDefintions.GetPtr(i);
		SafeKill(pcCalcObject);
	}
	mapcExpressionDefintions.RemoveAll();
	mapcExpressionDefintions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculatorVariables::Add(CCalcVariableDefinition* pcVariableDefinition)
{
	mapcExpressionDefintions.Add(pcVariableDefinition);
	mmszpcVariables.Put(pcVariableDefinition->GetName(), &pcVariableDefinition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculatorVariables::Add(CCalcExpression* pcExpression)
{
	mapcExpressionDefintions.Add(pcExpression);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCalcVariableDefinition* CCalculatorVariables::Get(char* szName)
{
	CCalcVariableDefinition** ppcVariableDefinition;

	ppcVariableDefinition = mmszpcVariables.Get(szName);
	if (ppcVariableDefinition)
	{
		return *ppcVariableDefinition;
	}
	else
	{
		return NULL;
	}
}

