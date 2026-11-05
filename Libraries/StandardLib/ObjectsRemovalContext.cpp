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
	mapcKilledObjects.Kill();
	mapcRemainingObjects.Kill();
	if (IsBusy() || (maapcRemovedObjects.NumElements() != 0))
	{
		gcLogger.Error2(__METHOD__, " Killing ObjectRemovalContext whist still performing a removal.", NULL);
	}
	maapcRemovedObjects.Kill();
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
	CArrayBaseObjectPtr*		apcPreviouslyKilled;
	CRemovalPointerContainer*	pcContainer;

	//Insert the objects that have just been killed into the list of all killed pointers.
	//This may leave duplicates but that's probably fine.
	pcContainer = maapcRemovedObjects.Get(0);
	apcPreviouslyKilled = pcContainer->GetKilled();
	mapcKilledObjects.InsertArrayAfterEnd(apcPreviouslyKilled);

	pcContainer->Kill();
	maapcRemovedObjects.RemoveAt(0);
	if (IsBusy())
	{
		//Copy the next list of objects to kill into remaining to be killed.
		//Remove all already killed object pointers from remaining.
		pcContainer = maapcRemovedObjects.Get(0);
		CopyNotKilled(pcContainer->GetKilled());
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
void CObjectRemovalContext::CopyNotKilled(CArrayBaseObjectPtr* papcNext)
{
	size			i;
	size			uiNumNext;
	size			uiIndex;
	CBaseObject*	pcObject;

	mapcKilledObjects.Sort();
	mapcRemainingObjects.ReInit();
	uiNumNext = papcNext->NumElements();
	for (i = 0; i < uiNumNext; i++)
	{
		pcObject = papcNext->GetPtr(i);
		if (!mapcKilledObjects.FindInSorted(&pcObject, &ComparePtrPtr, &uiIndex))
		{
			mapcRemainingObjects.Add(pcObject);
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
	maapcRemovedObjects.ReInit();
	mapcRemainingObjects.ReInit();
	mapcKilledObjects.ReInit();
}

