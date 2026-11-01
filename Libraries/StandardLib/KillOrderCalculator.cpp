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
	size			iNumElements;
	CBaseObject*	pcObject;

	iNumElements = mpapcKilled->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcObject = mpapcKilled->GetPtr(i);
		if (pcObject->IsNesPointedTo())
		{
			mapcKilledNesObjects.Add(pcObject);
			mpapcKilled->RemoveAt(i, false);
			i--;
			iNumElements--;
		}
	}

	return &mapcKilledNesObjects;
}

