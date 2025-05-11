#include "Address.h"
#include "OperationArray.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperationArray::Kill(void)
{
	size			i;
	size			uiNumElements;
	COperation*		pcOperation;

	uiNumElements = NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcOperation = __COperationArray::GetPtr(i);
		SafeKill(pcOperation);
	}

	__COperationArray::Kill();
}

