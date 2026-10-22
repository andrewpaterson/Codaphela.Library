#ifndef __WRITE_TRANSACTION_H__
#define __WRITE_TRANSACTION_H__
#include "DataConnection.h"
#include "MapBlock.h"
#include "MapStringTemplate.h"
#include "MapLongTemplate.h"
#include "ArrayChars.h"
#include "LockManager.h"
#include "TransactionIteration.h"


class CCodabase;


//The change a write transaction makes to one object.  The name (with its terminating zero) and then the data follow it.
class CWriteRecord
{
public:
	bool	bRemoved;			//The object is removed when the transaction commits.
	bool	bReplacesStored;	//The stored object was removed and this one added in its place.
	int32	iNameLength;
	size	uiDataSize;

	char*	GetName(void);
	void*	GetData(void);
};


//How the numbers of objects and names change when a write transaction commits, and how many it adds.
struct SWriteCounts
{
	int64	iIndices;
	int64	iNames;
	int64	iNewIndices;
	int64	iNewIndicesOfSize;
	int64	iNewNames;
};


//Changes made to a Codabase that are kept private until Commit, which applies them all in one durable commit.
//Abort, or Kill before Commit, discards them.
//
//Write transactions run at the same time.  Each object is locked before it is used: shared to read it, exclusive to
//change it, held until the transaction commits or aborts.  A transaction that would wait for an older one dies instead
//(wait-die): its changes are discarded, its locks released, and the call that found the conflict returns false.
//HasDied tells it apart from other failures and Restart begins it again with its original start number, so it gets
//older and will eventually succeed.
//
//A name being added is reserved until commit, so another transaction adding the same name fails straight away.  Name
//lookups and iteration do not lock what they do not find, so they are not protected against other transactions adding
//objects (phantoms).  Only one index iteration and one name iteration can be in progress at a time; the iterator
//argument is not used.
class CWriteTransaction : public CDataConnection
{
protected:
	CCodabase*					mpcCodabase;
	int64						miStartNumber;	//Lower is older.  0 is the direct session.
	int							miThread;
	CMapBlock					mcRecords;		//OIndex to CWriteRecord.
	CMapStringTemplate<OIndex>	mcNames;		//Names added or removed by the transaction.  INVALID_O_INDEX means removed.
	CArrayChars					maszReserved;	//Names this transaction has reserved.
	CMapLongTemplate<ELockMode>	mcLocked;		//The objects this transaction has locked, and how strongly.
	bool						mbActive;
	bool						mbDied;
	CTransactionIteration		mcIteration;

public:
	void	Init(CCodabase* pcCodabase, int64 iStartNumber);
	void	Kill(void);

	bool	Commit(void);
	void	Abort(void);
	bool	IsActive(void);
	bool	HasDied(void);
	bool	Restart(void);
	int64	GetStartNumber(void);
	int		GetThread(void);

	bool	Lock(OIndex oi, ELockMode eMode);

	bool	Add(OIndex oi, void* pvData, size uiDataSize);
	bool	Add(OIndex oi, const char* szName, void* pvData, size uiDataSize);
	bool	Add(OIndex oi, CChars* szName, void* pvData, size uiDataSize);

	bool	Set(OIndex oi, void* pvData);
	bool	Set(OIndex oi, void* pvData, size uiDataSize);
	bool	Set(const char* szName, void* pvData);
	bool	Set(const char* szName, void* pvData, size uiDataSize);
	bool	Set(CChars* szName, void* pvData);
	bool	Set(CChars* szName, void* pvData, size uiDataSize);

	bool	Put(OIndex oi, void* pvData, size uiDataSize);
	bool	Put(OIndex oi, const char* szName, void* pvData, size uiDataSize);
	bool	Put(OIndex oi, CChars* szName, void* pvData, size uiDataSize);

	bool	Get(OIndex oi, void* pvData);
	bool	Get(OIndex oi, size* puiDataSize, void* pvData, size uiMaxDataSize);
	bool	Get(const char* szName, void* pvData);
	bool	Get(const char* szName, size* puiDataSize, void* pvData, size uiMaxDataSize);
	bool	Get(CChars* szName, void* pvData);
	bool	Get(CChars* szName, size* puiDataSize, void* pvData, size uiMaxDataSize);

	bool	GetName(OIndex oi, CChars* szName);
	bool	GetName(OIndex oi, char* szName, size* puiNameLength, size uiMaxNameLength);
	OIndex	GetIndex(const char* szName);
	OIndex	GetIndex(CChars* szName);

	bool	Contains(OIndex oi);
	bool	Contains(const char* szName);
	bool	Contains(CChars* szName);

	bool	Remove(OIndex oi);
	bool	Remove(const char* szName);
	bool	Remove(CChars* szName);

	bool	Flush(void);
	bool	Flush(bool bClearCache);
	bool	IsWriteThrough(void);

	OIndex 	StartIndexIteration(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize);
	OIndex	IndexIterate(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize);
	bool	StartNameIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi);
	bool	NameIterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi);

	void	CountChanges(SWriteCounts* psCounts, size iCachedSize);

protected:
	bool			CheckActive(void);
	bool			LockOrDie(OIndex oi, ELockMode eMode);
	bool			FindAndLock(const char* szName, ELockMode eMode, OIndex* poi);
	void			Die(void);
	void			ReleaseResources(void);
	bool			ReserveNewName(const char* szName);
	CWriteRecord*	GetRecord(OIndex oi);
	bool			PutRecord(OIndex oi, bool bRemoved, bool bReplacesStored, const char* szName, void* pvData, size uiDataSize);
	bool			Exists(OIndex oi);
	OIndex			FindName(const char* szName);
	bool			GetCurrentName(OIndex oi, CChars* pszName);
	bool			GetDataSize(OIndex oi, size* puiDataSize);
	bool			SetData(OIndex oi, void* pvData, size uiDataSize);
	bool			AddLocked(OIndex oi, const char* szName, void* pvData, size uiDataSize);
	bool			SetLocked(OIndex oi, void* pvData, size uiDataSize);
	bool			RemoveLocked(OIndex oi);
	void			Finish(void);
};


#endif // __WRITE_TRANSACTION_H__

