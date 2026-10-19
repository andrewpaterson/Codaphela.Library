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
	mcReleased.Kill();
	mcMutex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ELockResult CLockManager::Lock(int64 iTransaction, OIndex oi, ELockMode eMode)
{
	CLockEntry*		pcEntry;
	SLockRequest	sRequest;
	size			iHeld;
	size			iWaiting;
	bool			bConflict;
	bool			bOlderThanAll;

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
	sRequest.iTransaction = iTransaction;
	sRequest.eMode = eMode;
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
			mcMutex.Unlock();
			return LR_Granted;
		}

		if (!bOlderThanAll)
		{
			//Wait-die: a younger transaction never waits for an older one, so no cycle of waits can form.
			pcEntry->masWaiters.RemoveAt(iWaiting);
			RemoveEntryIfUnused(oi, pcEntry);
			mcReleased.WakeAll();
			mcMutex.Unlock();
			return LR_Died;
		}

		mcReleased.Wait(&mcMutex);
		pcEntry = GetEntry(oi, false);
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
	CLockEntry*		pcEntry;
	size			iIndex;
	CArrayOIndex	aoiUnused;
	size			i;
	OIndex			oi;

	mcMutex.Lock();
	aoiUnused.Init();
	bExists = mcEntries.StartIteration(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	while (bExists)
	{
		pcEntry = *((CLockEntry**)pvData);
		iIndex = FindRequest(&pcEntry->masHolders, iTransaction);
		if (iIndex != ARRAY_ELEMENT_NOT_FOUND)
		{
			pcEntry->masHolders.RemoveAt(iIndex);
		}
		iIndex = FindRequest(&pcEntry->masWaiters, iTransaction);
		if (iIndex != ARRAY_ELEMENT_NOT_FOUND)
		{
			pcEntry->masWaiters.RemoveAt(iIndex);
		}
		if (pcEntry->IsUnused())
		{
			aoiUnused.Add(*((OIndex*)pvKey));
		}
		bExists = mcEntries.Iterate(&sIter, &pvKey, &iKeySize, &pvData, &iDataSize);
	}

	for (i = 0; i < aoiUnused.NumElements(); i++)
	{
		oi = aoiUnused.GetValue(i);
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
bool CLockManager::FindConflicts(CLockEntry* pcEntry, int64 iTransaction, ELockMode eMode, size iWaitPosition, bool* pbOlderThanAll)
{
	size			i;
	SLockRequest*	psRequest;
	bool			bConflict;

	//The conflicts are the other holders and the waiters ahead of this request that cannot share with it.
	bConflict = false;
	*pbOlderThanAll = true;
	for (i = 0; i < pcEntry->masHolders.NumElements(); i++)
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
		pcEntry->masHolders.Add(&sRequest);
	}
}

