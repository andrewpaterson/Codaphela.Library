#include "W65Address.h"
#include "W65OperationArray.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65OperationArray::Kill(void)
{
	size			i;
	size			uiNumElements;
	CW65Operation*		pcOperation;

	uiNumElements = NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcOperation = __CW65OperationArray::GetPtr(i);
		SafeKill(pcOperation);
	}

	__CW65OperationArray::Kill();
}

