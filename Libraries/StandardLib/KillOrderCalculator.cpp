#include "BaseObject.h"
#include "KillOrderCalculator.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CKillOrderCalculator::Init(CArrayBaseObjectPtr* papcKilled)
{
	mpapcKilled = papcKilled;
	mapcKilledNesObjects.Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CKillOrderCalculator::Kill(void)
{
	mapcKilledNesObjects.Kill();
	mpapcKilled = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CArrayBaseObjectPtr* CKillOrderCalculator::Calculate(void)
{
	size			i;
	size			uiNumElements;
	size			uiKept;
	CBaseObject*	pcObject;

	//Both keep the order they were found in.
	uiKept = 0;
	uiNumElements = mpapcKilled->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcObject = mpapcKilled->GetPtr(i);
		if (pcObject->IsNesPointedTo())
		{
			mapcKilledNesObjects.Add(pcObject);
		}
		else
		{
			*mpapcKilled->Get(uiKept) = pcObject;
			uiKept++;
		}
	}
	mpapcKilled->FakeSetUsedElements(uiKept);

	return &mapcKilledNesObjects;
}

