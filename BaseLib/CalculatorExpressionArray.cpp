#include "CalculatorExpressionArray.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculatorExpressionArray::Kill(void)
{
	size			i;
	size			uiNumElements;
	CCalcObject*	pcExpression;

	uiNumElements = NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcExpression = __CCalculatorExpressionArrays::GetPtr(i);
		pcExpression->Kill();
	}

	__CCalculatorExpressionArrays::Kill();
}

