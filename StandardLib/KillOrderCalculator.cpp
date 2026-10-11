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
	size						i;
	size						iNumElements;
	CBaseObject*				pcObject;
	CArrayEmbeddedObjectPtrE8	apNesObjectsPointingToThis;
	size						iNesPointersTo;

	iNumElements = mpapcKilled->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcObject = mpapcKilled->GetPtr(i);
		apNesObjectsPointingToThis.Init();
		pcObject->GetNesPointerToThis(&apNesObjectsPointingToThis);
		iNesPointersTo = apNesObjectsPointingToThis.NumElements();
		if (iNesPointersTo > 0)
		{
			mapcKilledNesObjects.Add(pcObject);
			mpapcKilled->RemoveAt(i, false);
			i--;
			iNumElements--;
		}
		apNesObjectsPointingToThis.Kill();
	}

	return &mapcKilledNesObjects;
}

