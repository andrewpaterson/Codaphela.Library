#include "CalculatorArrayExpressions.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCalculatorExpression::Kill(void)
{
	size			i;
	size			uiNumElements;
	CCalcObject*	pcExpression;

	uiNumElements = NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcExpression = __CArrayCalculatorExpressions::GetPtr(i);
		pcExpression->Kill();
	}

	__CArrayCalculatorExpressions::Kill();
}

