#include "BaseLib/Logger.h"
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

	pcContainer = maapcRemovedObjects.Get(0);
	pcContainer->Kill();
	maapcRemovedObjects.RemoveAt(0);
	if (IsBusy())
	{
		//First get the previously killed object points
		//Insert them into the list of all killed pointers
		//This may leave duplicates but that's probably fine.
		pcContainer = maapcRemovedObjects.Get(0);
		apcPreviouslyKilled = pcContainer->GetKilled();
		mapcKilledObjects.InsertArrayAfterEnd(apcPreviouslyKilled);

		//Copy the incoming list of objects to killed into remaining to be killed.
		//Remove all already killed object pointers from remaining.
		mapcRemainingObjects.ReInit();
		mapcRemainingObjects.InsertArrayAfterEnd(apcPreviouslyKilled);
		mapcRemainingObjects.Sort();
		mapcRemainingObjects.RemoveArrayFromSorted(&mapcKilledObjects);

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

