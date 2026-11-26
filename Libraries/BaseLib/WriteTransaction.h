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


//The change a write transaction makes to one object.  The name (with its terminating zero) follows it.  The data is
//allocated on its own (FreeData frees it): a map value is at most 64KB with a 16 bit size, the object's data isn't.
class CWriteRecord
{
public:
	bool	bRemoved;			//The object is removed when the transaction commits.
	bool	bReplacesStored;	//The stored object was removed and this one added in its place.
	int32	iNameLength;
	lsize	uiDataSize;
	void*	mpvData;			//NULL when uiDataSize is 0.

	char*	GetName(void) const;
	void*	GetData(void) const;
	void	FreeData(void);
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
	void	Kill(void) override;

	bool	Commit(void);
	void	Abort(void);
	bool	IsActive(void) const;
	bool	HasDied(void) const;
	bool	Restart(void);
	int64	GetStartNumber(void) const;
	int		GetThread(void) const;

	bool	Lock(OIndex oi, ELockMode eMode);

	bool	Add(OIndex oi, void* pvData, lsize uiDataSize) override;
	bool	Add(OIndex oi, const char* szName, void* pvData, lsize uiDataSize) override;
	bool	Add(OIndex oi, CChars* szName, void* pvData, lsize uiDataSize) override;

	bool	Set(OIndex oi, void* pvData) override;
	bool	Set(OIndex oi, void* pvData, lsize uiDataSize) override;
	bool	Set(const char* szName, void* pvData) override;
	bool	Set(const char* szName, void* pvData, lsize uiDataSize) override;
	bool	Set(CChars* szName, void* pvData) override;
	bool	Set(CChars* szName, void* pvData, lsize uiDataSize) override;

	bool	Put(OIndex oi, void* pvData, lsize uiDataSize) override;
	bool	Put(OIndex oi, const char* szName, void* pvData, lsize uiDataSize) override;
	bool	Put(OIndex oi, CChars* szName, void* pvData, lsize uiDataSize) override;

	bool	Get(OIndex oi, void* pvData) override;
	bool	Get(OIndex oi, lsize* puiDataSize, void* pvData, lsize uiMaxDataSize) override;
	bool	Get(const char* szName, void* pvData) override;
	bool	Get(const char* szName, lsize* puiDataSize, void* pvData, lsize uiMaxDataSize) override;
	bool	Get(CChars* szName, void* pvData) override;
	bool	Get(CChars* szName, lsize* puiDataSize, void* pvData, lsize uiMaxDataSize) override;

	bool	GetName(OIndex oi, CChars* szName) override;
	bool	GetName(OIndex oi, char* szName, size* puiNameLength, size uiMaxNameLength) override;
	OIndex	GetIndex(const char* szName) override;
	OIndex	GetIndex(CChars* szName) override;

	bool	Contains(OIndex oi) override;
	bool	Contains(const char* szName) override;
	bool	Contains(CChars* szName) override;

	bool	Remove(OIndex oi) override;
	bool	Remove(const char* szName) override;
	bool	Remove(CChars* szName) override;

	bool	Flush(void) override;
	bool	Flush(bool bClearCache) override;
	bool	IsWriteThrough(void) override;

	OIndex	StartIndexIteration(SIndexTreeFileIterator* psIterator, void* pvData, lsize* piDataSize, lsize iMaxDataSize) override;
	OIndex	IndexIterate(SIndexTreeFileIterator* psIterator, void* pvData, lsize* piDataSize, lsize iMaxDataSize) override;
	bool	StartNameIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi) override;
	bool	NameIterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi) override;

	void	CountChanges(SWriteCounts* psCounts, lsize iCachedSize);

protected:
	bool			CheckActive(void);
	bool			LockOrDie(OIndex oi, ELockMode eMode);
	bool			FindAndLock(const char* szName, ELockMode eMode, OIndex* poi);
	void			Die(void);
	void			ReleaseResources(void);
	bool			ReserveNewName(const char* szName);
	CWriteRecord*	GetRecord(OIndex oi);
	bool			PutRecord(OIndex oi, bool bRemoved, bool bReplacesStored, const char* szName, void* pvData, lsize uiDataSize);
	bool			RemoveRecord(OIndex oi);
	void			FreeRecords(void);
	bool			Exists(OIndex oi);
	OIndex			FindName(const char* szName);
	bool			GetCurrentName(OIndex oi, CChars* pszName);
	bool			GetDataSize(OIndex oi, lsize* puiDataSize);
	bool			SetData(OIndex oi, void* pvData, lsize uiDataSize);
	bool			AddLocked(OIndex oi, const char* szName, void* pvData, lsize uiDataSize);
	bool			SetLocked(OIndex oi, void* pvData, lsize uiDataSize);
	bool			RemoveLocked(OIndex oi);
	void			Finish(void);
};


#endif // __WRITE_TRANSACTION_H__

