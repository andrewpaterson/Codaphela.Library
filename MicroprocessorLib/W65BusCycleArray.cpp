#include "W65Address.h"
#include "W65BusCycleArray.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65BusCycleArray::Kill(void)
{
	size			i;
	size			uiNumElements;
	CW65BusCycle*		pcBusCycle;

	uiNumElements = NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcBusCycle = __CW65BusCycleArray::GetPtr(i);
		SafeKill(pcBusCycle);
	}

	__CW65BusCycleArray::Kill();
}

