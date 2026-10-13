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
	if (!IsBusy())
	{
		Push(papcKilled);
		return papcKilled;
	}
	else
	{
		CArrayBaseObjectPtr* apcTemp;

		apcTemp = maapcRemovedObjects.TailPtr();
		if (mapcRemainingObjects.NumElements() == 0)
		{
			mapcKilledObjects.InsertArrayAt(apcTemp, 0);
			mapcKilledObjects.Sort();

			mapcRemainingObjects.InsertArrayAt(papcKilled, 0);
			mapcRemainingObjects.Sort();
			mapcRemainingObjects.RemoveArrayFromSorted(&mapcKilledObjects);
		}

		Push(papcKilled);
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectRemovalContext::Done(void)
{
	CArrayBaseObjectPtr* papcKilled;

	papcKilled = Pull();
	if (IsBusy())
	{
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectRemovalContext::Push(CArrayBaseObjectPtr* papcKilled)
{
	maapcRemovedObjects.Push(papcKilled);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayBaseObjectPtr* CObjectRemovalContext::Pull(void) 
{
	return maapcRemovedObjects.Pop();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectRemovalContext::IsBusy(void)
{
	return maapcRemovedObjects.NumElements() != 0;
}

