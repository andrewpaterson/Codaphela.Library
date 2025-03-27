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
void CCalculatorVariables::ReInit(void)
{
	Kill();
	Init();
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculatorVariables::Print(CChars* psz)
{
	size			i;
	CCalcObject*	pcObject;
	size			uiNumElements;

	uiNumElements = mapcExpressionDefintions.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcObject = mapcExpressionDefintions.GetPtr(i);
		pcObject->Print(psz);
		psz->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCalcExpression* CCalculatorVariables::GetExpression(size uiIndex)
{
	size			i;
	CCalcObject*	pcObject;
	size			uiNumElements;
	size			uiCount;

	uiCount = 0;
	uiNumElements = mapcExpressionDefintions.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcObject = mapcExpressionDefintions.GetPtr(i);
		if (pcObject->IsExpression())
		{
			if (uiIndex == uiCount)
			{
				return (CCalcExpression*)pcObject;
			}
			uiCount++;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCalculatorVariables::NumExpressions(void)
{
	size			i;
	CCalcObject*	pcObject;
	size			uiNumElements;
	size			uiCount;

	uiCount = 0;
	uiNumElements = mapcExpressionDefintions.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcObject = mapcExpressionDefintions.GetPtr(i);
		if (pcObject->IsExpression())
		{
			uiCount++;
		}
	}
	return uiCount;
}

