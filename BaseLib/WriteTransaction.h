#ifndef __WRITE_TRANSACTION_H__
#define __WRITE_TRANSACTION_H__
#include "DataConnection.h"
#include "MapBlock.h"
#include "MapStringTemplate.h"
#include "ArrayChars.h"
#include "CodabaseVersions.h"
#include "LockManager.h"


class CCodabase;


//The change a write transaction makes to one object.  The name (with its terminating zero) and then the data follow it.
struct SWriteRecord
{
	bool	bRemoved;			//The object is removed when the transaction commits.
	bool	bReplacesStored;	//The stored object was removed and this one added in its place.
	int32	iNameLength;
	uint32	uiDataSize;

	char*	GetName(void);
	void*	GetData(void);
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
	CMapBlock					mcRecords;		//OIndex to SWriteRecord.
	CMapStringTemplate<OIndex>	mcNames;		//Names added or removed by the transaction.  INVALID_O_INDEX means removed.
	CArrayChars					maszReserved;	//Names this transaction has reserved.
	bool						mbActive;
	bool						mbDied;
	CArrayOIndex				maoiIteration;
	size						miIndexPosition;
	CArrayChars					maszIteration;
	size						miNamePosition;

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

	bool	Add(OIndex oi, void* pvData, uint32 uiDataSize);
	bool	Add(OIndex oi, const char* szName, void* pvData, uint32 uiDataSize);
	bool	Add(OIndex oi, CChars* szName, void* pvData, uint32 uiDataSize);

	bool	Set(OIndex oi, void* pvData);
	bool	Set(OIndex oi, void* pvData, uint32 uiDataSize);
	bool	Set(const char* szName, void* pvData);
	bool	Set(const char* szName, void* pvData, uint32 uiDataSize);
	bool	Set(CChars* szName, void* pvData);
	bool	Set(CChars* szName, void* pvData, uint32 uiDataSize);

	bool	Put(OIndex oi, void* pvData, uint32 uiDataSize);
	bool	Put(OIndex oi, const char* szName, void* pvData, uint32 uiDataSize);
	bool	Put(OIndex oi, CChars* szName, void* pvData, uint32 uiDataSize);

	bool	Get(OIndex oi, void* pvData);
	bool	Get(OIndex oi, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize);
	bool	Get(const char* szName, void* pvData);
	bool	Get(const char* szName, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize);
	bool	Get(CChars* szName, void* pvData);
	bool	Get(CChars* szName, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize);

	bool	GetName(OIndex oi, CChars* szName);
	bool	GetName(OIndex oi, char* szName, uint32* puiNameLength, uint32 uiMaxNameLength);
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

	void	GetCountChanges(int64* piIndices, int64* piNames);

protected:
	bool			CheckActive(void);
	bool			LockOrDie(OIndex oi, ELockMode eMode);
	bool			FindAndLock(const char* szName, ELockMode eMode, OIndex* poi);
	void			Die(void);
	void			ReleaseResources(void);
	bool			ReserveNewName(const char* szName);
	SWriteRecord*	GetRecord(OIndex oi);
	bool			PutRecord(OIndex oi, bool bRemoved, bool bReplacesStored, const char* szName, void* pvData, uint32 uiDataSize);
	bool			Exists(OIndex oi);
	OIndex			FindName(const char* szName);
	bool			GetCurrentName(OIndex oi, CChars* pszName);
	bool			GetDataSize(OIndex oi, uint32* puiDataSize);
	bool			SetData(OIndex oi, void* pvData, uint32 uiDataSize);
	bool			AddLocked(OIndex oi, const char* szName, void* pvData, uint32 uiDataSize);
	bool			SetLocked(OIndex oi, void* pvData, uint32 uiDataSize);
	bool			RemoveLocked(OIndex oi);
	bool			Apply(bool bRemovals);
	void			Finish(void);
};


#endif // __WRITE_TRANSACTION_H__

