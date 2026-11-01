#include "UnknownKillList.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKillList::AddKill(CUnknownTestHelper* pcKilled)
{
	Add(&pcKilled);
}

bool CKillList::Contains(CUnknownTestHelper* pcKilled)
{
	size					i;
	CUnknownTestHelper*		pcTest;

	for (i = 0; i < miUsedElements; i++)
	{
		pcTest = *Get(i);
		if (pcTest == pcKilled)
		{
			return true;
		}
	}
	return false;
}

