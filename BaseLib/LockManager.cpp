#include "LockManager.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLockManager::Init(void)
{
	mcMutex.Init();
	mcReleased.Init();
	mcEntries.Init(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLockManager::Kill(void)
{
	SMapIterator	sIter;
	void*			pvKey;
	void*			pvData;
	size			iKeySize;
	size			iDataSize;
	bool			bExists;
	SLockEntry*		psEntry;

	bExists = mcEntries.StartIteration(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	while (bExists)
	{
		psEntry = *((SLockEntry**)pvData);
		psEntry->asHolders.Kill();
		psEntry->asWaiters.Kill();
		free(psEntry);
		bExists = mcEntries.Iterate(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	}
	mcEntries.Kill();
	mcReleased.Kill();
	mcMutex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ELockResult CLockManager::Lock(int64 iTransaction, OIndex oi, ELockMode eMode)
{
	SLockEntry*		psEntry;
	SLockRequest	sRequest;
	size			iHeld;
	size			iWaiting;
	bool			bConflict;
	bool			bOlderThanAll;

	mcMutex.Lock();
	psEntry = GetEntry(oi, true);

	//A lock already held at least as strongly is granted straight away.  Otherwise the stronger mode is requested.
	iHeld = FindRequest(&psEntry->asHolders, iTransaction);
	if (iHeld != ARRAY_ELEMENT_NOT_FOUND)
	{
		if (psEntry->asHolders.Get(iHeld)->eMode >= eMode)
		{
			mcMutex.Unlock();
			return LR_Granted;
		}
	}

	//An upgrade goes to the front of the queue because it already holds the lock.
	sRequest.iTransaction = iTransaction;
	sRequest.eMode = eMode;
	if (iHeld != ARRAY_ELEMENT_NOT_FOUND)
	{
		psEntry->asWaiters.InsertAt(&sRequest, 0);
	}
	else
	{
		psEntry->asWaiters.Add(&sRequest);
	}

	for (;;)
	{
		iWaiting = FindRequest(&psEntry->asWaiters, iTransaction);
		bConflict = FindConflicts(psEntry, iTransaction, eMode, iWaiting, &bOlderThanAll);
		if (!bConflict)
		{
			psEntry->asWaiters.RemoveAt(iWaiting);
			Grant(psEntry, iTransaction, eMode);
			mcMutex.Unlock();
			return LR_Granted;
		}

		if (!bOlderThanAll)
		{
			//Wait-die: a younger transaction never waits for an older one, so no cycle of waits can form.
			psEntry->asWaiters.RemoveAt(iWaiting);
			RemoveEntryIfUnused(oi, psEntry);
			mcReleased.WakeAll();
			mcMutex.Unlock();
			return LR_Died;
		}

		mcReleased.Wait(&mcMutex);
		psEntry = GetEntry(oi, false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLockManager::ReleaseAll(int64 iTransaction)
{
	SMapIterator	sIter;
	void*			pvKey;
	void*			pvData;
	size			iKeySize;
	size			iDataSize;
	bool			bExists;
	SLockEntry*		psEntry;
	size			iIndex;
	CArrayTemplate<OIndex>	aoiUnused;
	size			i;
	OIndex			oi;

	mcMutex.Lock();
	aoiUnused.Init();
	bExists = mcEntries.StartIteration(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	while (bExists)
	{
		psEntry = *((SLockEntry**)pvData);
		iIndex = FindRequest(&psEntry->asHolders, iTransaction);
		if (iIndex != ARRAY_ELEMENT_NOT_FOUND)
		{
			psEntry->asHolders.RemoveAt(iIndex);
		}
		iIndex = FindRequest(&psEntry->asWaiters, iTransaction);
		if (iIndex != ARRAY_ELEMENT_NOT_FOUND)
		{
			psEntry->asWaiters.RemoveAt(iIndex);
		}
		if ((psEntry->asHolders.NumElements() == 0) && (psEntry->asWaiters.NumElements() == 0))
		{
			aoiUnused.Add((OIndex*)pvKey);
		}
		bExists = mcEntries.Iterate(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	}

	for (i = 0; i < aoiUnused.NumElements(); i++)
	{
		oi = *aoiUnused.Get(i);
		RemoveEntryIfUnused(oi, GetEntry(oi, false));
	}
	aoiUnused.Kill();

	mcReleased.WakeAll();
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ELockMode CLockManager::GetMode(int64 iTransaction, OIndex oi)
{
	SLockEntry*	psEntry;
	size		iHeld;
	ELockMode	eMode;

	mcMutex.Lock();
	eMode = LM_None;
	psEntry = GetEntry(oi, false);
	if (psEntry != NULL)
	{
		iHeld = FindRequest(&psEntry->asHolders, iTransaction);
		if (iHeld != ARRAY_ELEMENT_NOT_FOUND)
		{
			eMode = psEntry->asHolders.Get(iHeld)->eMode;
		}
	}
	mcMutex.Unlock();
	return eMode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CLockManager::NumLocked(void)
{
	size	uiNumLocked;

	mcMutex.Lock();
	uiNumLocked = mcEntries.NumElements();
	mcMutex.Unlock();
	return uiNumLocked;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLockManager::IsCompatible(ELockMode eHeld, ELockMode eRequested)
{
	//Only shared locks may be held with each other, and one update lock with shared ones.
	if ((eHeld == LM_Shared) && ((eRequested == LM_Shared) || (eRequested == LM_Update)))
	{
		return true;
	}
	if ((eHeld == LM_Update) && (eRequested == LM_Shared))
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SLockEntry* CLockManager::GetEntry(OIndex oi, bool bCreate)
{
	SLockEntry**	ppsEntry;
	SLockEntry*		psEntry;

	ppsEntry = mcEntries.Get(oi);
	if (ppsEntry != NULL)
	{
		return *ppsEntry;
	}
	if (!bCreate)
	{
		return NULL;
	}

	psEntry = (SLockEntry*)malloc(sizeof(SLockEntry));
	psEntry->asHolders.Init();
	psEntry->asWaiters.Init();
	mcEntries.Put(oi, &psEntry);
	return psEntry;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLockManager::RemoveEntryIfUnused(OIndex oi, SLockEntry* psEntry)
{
	if ((psEntry != NULL) && (psEntry->asHolders.NumElements() == 0) && (psEntry->asWaiters.NumElements() == 0))
	{
		psEntry->asHolders.Kill();
		psEntry->asWaiters.Kill();
		free(psEntry);
		mcEntries.Remove(oi);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CLockManager::FindRequest(CArrayLockRequest* pasRequests, int64 iTransaction)
{
	size	i;

	for (i = 0; i < pasRequests->NumElements(); i++)
	{
		if (pasRequests->Get(i)->iTransaction == iTransaction)
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLockManager::FindConflicts(SLockEntry* psEntry, int64 iTransaction, ELockMode eMode, size iWaitPosition, bool* pbOlderThanAll)
{
	size			i;
	SLockRequest*	psRequest;
	bool			bConflict;

	//The conflicts are the other holders and the waiters ahead of this request that cannot share with it.
	bConflict = false;
	*pbOlderThanAll = true;
	for (i = 0; i < psEntry->asHolders.NumElements(); i++)
	{
		psRequest = psEntry->asHolders.Get(i);
		if ((psRequest->iTransaction != iTransaction) && !IsCompatible(psRequest->eMode, eMode))
		{
			bConflict = true;
			if (iTransaction > psRequest->iTransaction)
			{
				*pbOlderThanAll = false;
			}
		}
	}
	for (i = 0; i < iWaitPosition; i++)
	{
		psRequest = psEntry->asWaiters.Get(i);
		if ((psRequest->iTransaction != iTransaction) && (!IsCompatible(psRequest->eMode, eMode) || !IsCompatible(eMode, psRequest->eMode)))
		{
			bConflict = true;
			if (iTransaction > psRequest->iTransaction)
			{
				*pbOlderThanAll = false;
			}
		}
	}
	return bConflict;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLockManager::Grant(SLockEntry* psEntry, int64 iTransaction, ELockMode eMode)
{
	size			iHeld;
	SLockRequest	sRequest;

	iHeld = FindRequest(&psEntry->asHolders, iTransaction);
	if (iHeld != ARRAY_ELEMENT_NOT_FOUND)
	{
		if (psEntry->asHolders.Get(iHeld)->eMode < eMode)
		{
			psEntry->asHolders.Get(iHeld)->eMode = eMode;
		}
	}
	else
	{
		sRequest.iTransaction = iTransaction;
		sRequest.eMode = eMode;
		psEntry->asHolders.Add(&sRequest);
	}
}

