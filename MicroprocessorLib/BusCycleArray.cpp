#include "Address.h"
#include "BusCycleArray.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBusCycleArray::Kill(void)
{
	size			i;
	size			uiNumElements;
	CBusCycle*		pcBusCycle;

	uiNumElements = NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcBusCycle = __CBusCycleArray::GetPtr(i);
		pcBusCycle->Kill();
	}

	__CBusCycleArray::Kill();
}

