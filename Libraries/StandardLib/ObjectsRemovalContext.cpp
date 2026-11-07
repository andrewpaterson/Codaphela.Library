#include "BaseLib/Logger.h"
#include "BaseLib/PointerFunctions.h"
#include "ObjectsRemovalContext.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectRemovalContext::Init(void)
{
	maapcRemovedObjects.Init();
	mapcRemainingObjects.Init();
	mapcKilledObjects.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectRemovalContext::Kill(void)
{
	if (IsBusy())
	{
		gcLogger.Error2(__METHOD__, " Killing ObjectRemovalContext whilst still performing a removal.", NULL);
	}
	Clear();
	maapcRemovedObjects.Kill();
	mapcRemainingObjects.Kill();
	mapcKilledObjects.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayBaseObjectPtr* CObjectRemovalContext::Start(CArrayBaseObjectPtr* papcKilled)
{
	CRemovalPointerContainer* pcContainer;

	pcContainer = maapcRemovedObjects.Add();
	pcContainer->Init(papcKilled, false);
	return papcKilled;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayBaseObjectPtr* CObjectRemovalContext::Done(void)
{
	CRemovalPointerContainer*	pcContainer;
	size						i;
	size						uiNumRemoved;

	//The batch just freed has been deallocated, and nothing has been allocated since.  Its objects are taken out of the
	//batches still waiting now: once anything is allocated an object could be made where one of them was, and a
	//batch it is in must not take it for the freed one.
	pcContainer = maapcRemovedObjects.Get(0);
	mapcKilledObjects.ReInit();
	mapcKilledObjects.InsertArrayAfterEnd(pcContainer->GetKilled());
	mapcKilledObjects.Sort();
	uiNumRemoved = maapcRemovedObjects.NumElements();
	for (i = 1; i < uiNumRemoved; i++)
	{
		RemoveKilled(maapcRemovedObjects.Get(i)->GetKilled());
	}
	mapcKilledObjects.ReInit();

	pcContainer->Kill();
	maapcRemovedObjects.RemoveAt(0);
	if (IsBusy())
	{
		pcContainer = maapcRemovedObjects.Get(0);
		mapcRemainingObjects.ReInit();
		mapcRemainingObjects.InsertArrayAfterEnd(pcContainer->GetKilled());
		return &mapcRemainingObjects;
	}
	else
	{
		Clear();
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The objects in papcNext not killed already, in papcNext's order.  The
// killed objects are found by address (they may be deallocated), so only
// the record of them is sorted.
//
//////////////////////////////////////////////////////////////////////////
void CObjectRemovalContext::RemoveKilled(CArrayBaseObjectPtr* papcWaiting)
{
	size			i;
	size			uiIndex;
	CBaseObject*	pcObject;

	//Backwards, so removing doesn't move the ones still to look at.
	i = papcWaiting->NumElements();
	while (i > 0)
	{
		i--;
		pcObject = papcWaiting->GetPtr(i);
		if (mapcKilledObjects.FindInSorted(&pcObject, &ComparePtrPtr, &uiIndex))
		{
			papcWaiting->RemoveAt(i, true);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectRemovalContext::Push(CArrayBaseObjectPtr* papcKilled)
{
	CRemovalPointerContainer*	pcContainer;

	pcContainer = maapcRemovedObjects.Add();
	pcContainer->Init(papcKilled, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectRemovalContext::IsBusy(void)
{
	return maapcRemovedObjects.NumElements() != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectRemovalContext::Clear(void)
{
	size	i;
	size	uiNumRemoved;

	//Pushed batches are copies, which their containers free.
	uiNumRemoved = maapcRemovedObjects.NumElements();
	for (i = 0; i < uiNumRemoved; i++)
	{
		maapcRemovedObjects.Get(i)->Kill();
	}
	maapcRemovedObjects.ReInit();
	mapcRemainingObjects.ReInit();
	mapcKilledObjects.ReInit();
}
