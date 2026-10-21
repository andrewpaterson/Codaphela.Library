#include "PointerFunctions.h"
#include "CalcObjectArray.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcObjectArray::Kill(void)
{
	size			i;
	size			uiNumElements;
	CCalcObject*	pcExpression;

	uiNumElements = NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		//The objects are owned by the array.
		pcExpression = __CCalcObjectArray::GetPtr(i);
		SafeKill(pcExpression);
	}

	__CCalcObjectArray::Kill();
}

