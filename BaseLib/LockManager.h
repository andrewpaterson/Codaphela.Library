#ifndef __LOCK_MANAGER_H__
#define __LOCK_MANAGER_H__
#include "Mutex.h"
#include "ConditionVariable.h"
#include "ArrayTemplate.h"
#include "MapLongTemplate.h"
#include "IndexedGeneral.h"


enum ELockMode
{
	LM_None,
	LM_Shared,		//I read it.
	LM_Update,		//I read it and will probably write it.  Readers may continue; only one transaction may hold it.
	LM_Exclusive	//I am writing it.
};


enum ELockResult
{
	LR_Granted,
	LR_Died		//The requester is younger than a transaction it conflicts with, so it must abort (wait-die).
};


struct SLockRequest
{
	int64		iTransaction;	//The transaction's start number.  Lower is older.
	ELockMode	eMode;
};


typedef CArrayTemplate<SLockRequest>	CArrayLockRequest;


struct SLockEntry
{
	CArrayLockRequest	asHolders;
	CArrayLockRequest	asWaiters;	//In the order they will be granted.
};


//Locks objects by OIndex for write transactions until they commit or abort.  Deadlocks are prevented with wait-die: a
//request that conflicts waits only if it is older than every transaction it conflicts with, otherwise it dies.
//Waiting requests ahead of a new one count as conflicts, so a waiting writer cannot be passed for ever by new readers.
class CLockManager
{
protected:
	CMutex							mcMutex;
	CConditionVariable				mcReleased;
	CMapLongTemplate<SLockEntry*>	mcEntries;

public:
	void			Init(void);
	void			Kill(void);

	ELockResult		Lock(int64 iTransaction, OIndex oi, ELockMode eMode);
	void			ReleaseAll(int64 iTransaction);

	ELockMode		GetMode(int64 iTransaction, OIndex oi);
	size			NumLocked(void);

	static bool		IsCompatible(ELockMode eHeld, ELockMode eRequested);

protected:
	SLockEntry*		GetEntry(OIndex oi, bool bCreate);
	void			RemoveEntryIfUnused(OIndex oi, SLockEntry* psEntry);
	size			FindRequest(CArrayLockRequest* pasRequests, int64 iTransaction);
	bool			FindConflicts(SLockEntry* psEntry, int64 iTransaction, ELockMode eMode, size iWaitPosition, bool* pbOlderThanAll);
	void			Grant(SLockEntry* psEntry, int64 iTransaction, ELockMode eMode);
};


#endif // __LOCK_MANAGER_H__

