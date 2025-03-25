#include "CalcObjectArray.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcuObjectArray::Kill(void)
{
	size			i;
	size			uiNumElements;
	CCalcObject*	pcExpression;

	uiNumElements = NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcExpression = __CCalcObjectArray::GetPtr(i);
		pcExpression->Kill();
	}

	__CCalcObjectArray::Kill();
}

