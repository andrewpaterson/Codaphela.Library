#include "LockManager.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLockEntry::Init(void)
{
	masHolders.Init();
	masWaiters.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLockEntry::Kill(void)
{
	masHolders.Kill();
	masWaiters.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLockEntry::IsUnused(void)
{
	return (masHolders.NumElements() == 0) && (masWaiters.NumElements() == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLockManager::Init(void)
{
	mcMutex.Init();
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
	CLockEntry*		pcEntry;

	bExists = mcEntries.StartIteration(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	while (bExists)
	{
		pcEntry = *((CLockEntry**)pvData);
		pcEntry->Kill();
		free(pcEntry);
		bExists = mcEntries.Iterate(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	}
	mcEntries.Kill();
	mcMutex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ELockResult CLockManager::Lock(int64 iTransaction, OIndex oi, ELockMode eMode)
{
	CLockEntry*			pcEntry;
	SLockRequest		sRequest;
	size				iHeld;
	size				iWaiting;
	bool				bConflict;
	bool				bOlderThanAll;
	CConditionVariable	cWake;
	ELockResult			eResult;

	mcMutex.Lock();
	pcEntry = GetEntry(oi, true);

	//A lock already held at least as strongly is granted straight away.  Otherwise the stronger mode is requested.
	iHeld = FindRequest(&pcEntry->masHolders, iTransaction);
	if (iHeld != ARRAY_ELEMENT_NOT_FOUND)
	{
		if (pcEntry->masHolders.Get(iHeld)->eMode >= eMode)
		{
			mcMutex.Unlock();
			return LR_Granted;
		}
	}

	//An upgrade goes to the front of the queue because it already holds the lock.
	cWake.Init();
	sRequest.iTransaction = iTransaction;
	sRequest.eMode = eMode;
	sRequest.pcWake = &cWake;
	if (iHeld != ARRAY_ELEMENT_NOT_FOUND)
	{
		pcEntry->masWaiters.InsertAt(&sRequest, 0);
	}
	else
	{
		pcEntry->masWaiters.Add(&sRequest);
	}

	for (;;)
	{
		iWaiting = FindRequest(&pcEntry->masWaiters, iTransaction);
		bConflict = FindConflicts(pcEntry, iTransaction, eMode, iWaiting, &bOlderThanAll);
		if (!bConflict)
		{
			pcEntry->masWaiters.RemoveAt(iWaiting);
			Grant(pcEntry, iTransaction, eMode);
			eResult = LR_Granted;
			break;
		}

		if (!bOlderThanAll)
		{
			//Wait-die: a younger transaction never waits for an older one, so no cycle of waits can form.
			pcEntry->masWaiters.RemoveAt(iWaiting);
			eResult = LR_Died;
			break;
		}

		cWake.Wait(&mcMutex);
	}

	//The queue changed, so the requests behind this one may be able to go on.
	WakeWaiters(pcEntry);
	RemoveEntryIfUnused(oi, pcEntry);
	mcMutex.Unlock();
	cWake.Kill();
	return eResult;
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
	CArrayOIndex	aoiLocked;

	//Every entry is looked at.  A transaction that knows what it locked uses Release instead.
	aoiLocked.Init();
	mcMutex.Lock();
	bExists = mcEntries.StartIteration(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	while (bExists)
	{
		aoiLocked.Add(*((OIndex*)pvKey));
		bExists = mcEntries.Iterate(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	}
	mcMutex.Unlock();

	Release(iTransaction, &aoiLocked);
	aoiLocked.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLockManager::Release(int64 iTransaction, CArrayOIndex* paoiLocked)
{
	lsize		i;
	OIndex		oi;
	CLockEntry*	pcEntry;
	lsize		uiNumElements;

	mcMutex.Lock();
	uiNumElements = paoiLocked->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		oi = paoiLocked->GetValue(i);
		pcEntry = GetEntry(oi, false);
		if (pcEntry != NULL)
		{
			ReleaseFromEntry(oi, pcEntry, iTransaction);
		}
	}
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ELockMode CLockManager::GetMode(int64 iTransaction, OIndex oi)
{
	CLockEntry*	pcEntry;
	size		iHeld;
	ELockMode	eMode;

	mcMutex.Lock();
	eMode = LM_None;
	pcEntry = GetEntry(oi, false);
	if (pcEntry != NULL)
	{
		iHeld = FindRequest(&pcEntry->masHolders, iTransaction);
		if (iHeld != ARRAY_ELEMENT_NOT_FOUND)
		{
			eMode = pcEntry->masHolders.Get(iHeld)->eMode;
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
CLockEntry* CLockManager::GetEntry(OIndex oi, bool bCreate)
{
	CLockEntry**	ppcEntry;
	CLockEntry*		pcEntry;

	ppcEntry = mcEntries.Get(oi);
	if (ppcEntry != NULL)
	{
		return *ppcEntry;
	}
	if (!bCreate)
	{
		return NULL;
	}

	pcEntry = (CLockEntry*)malloc(sizeof(CLockEntry));
	pcEntry->Init();
	mcEntries.Put(oi, &pcEntry);
	return pcEntry;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLockManager::RemoveEntryIfUnused(OIndex oi, CLockEntry* pcEntry)
{
	if ((pcEntry != NULL) && pcEntry->IsUnused())
	{
		pcEntry->Kill();
		free(pcEntry);
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
	size	uiNumElements;

	uiNumElements = pasRequests->NumElements();
	for (i = 0; i < uiNumElements; i++)
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
bool CLockManager::FindConflicts(CLockEntry* pcEntry, int64 iTransaction, ELockMode eMode, size iWaitPosition, bool* pbOlderThanAll)
{
	size			i;
	SLockRequest*	psRequest;
	bool			bConflict;
	size			uiNumHolders;

	//The conflicts are the other holders and the waiters ahead of this request that cannot share with it.
	bConflict = false;
	*pbOlderThanAll = true;
	uiNumHolders = pcEntry->masHolders.NumElements();
	for (i = 0; i < uiNumHolders; i++)
	{
		psRequest = pcEntry->masHolders.Get(i);
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
		psRequest = pcEntry->masWaiters.Get(i);
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
void CLockManager::ReleaseFromEntry(OIndex oi, CLockEntry* pcEntry, int64 iTransaction)
{
	size	iIndex;

	//mcMutex must be locked.
	iIndex = FindRequest(&pcEntry->masHolders, iTransaction);
	if (iIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		pcEntry->masHolders.RemoveAt(iIndex);
		WakeWaiters(pcEntry);
	}
	RemoveEntryIfUnused(oi, pcEntry);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLockManager::WakeWaiters(CLockEntry* pcEntry)
{
	size	i;
	size	uiNumWaiters;

	//mcMutex must be locked.  Each waiter checks again whether it can have the lock.
	uiNumWaiters = pcEntry->masWaiters.NumElements();
	for (i = 0; i < uiNumWaiters; i++)
	{
		pcEntry->masWaiters.Get(i)->pcWake->WakeOne();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLockManager::Grant(CLockEntry* pcEntry, int64 iTransaction, ELockMode eMode)
{
	size			iHeld;
	SLockRequest	sRequest;

	iHeld = FindRequest(&pcEntry->masHolders, iTransaction);
	if (iHeld != ARRAY_ELEMENT_NOT_FOUND)
	{
		if (pcEntry->masHolders.Get(iHeld)->eMode < eMode)
		{
			pcEntry->masHolders.Get(iHeld)->eMode = eMode;
		}
	}
	else
	{
		sRequest.iTransaction = iTransaction;
		sRequest.eMode = eMode;
		sRequest.pcWake = NULL;
		pcEntry->masHolders.Add(&sRequest);
	}
}

