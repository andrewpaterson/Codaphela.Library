#ifndef __CODABASE_H__
#define __CODABASE_H__
#include "Killable.h"
#include "NamedIndexedData.h"
#include "DataConnection.h"
#include "Mutex.h"
#include "ConditionVariable.h"
#include "CodabaseVersions.h"
#include "LockManager.h"
#include "MapStringTemplate.h"
#include "ArrayInt64.h"
#include "ArrayTemplatePtr.h"


class CWriteTransaction;
class CReadTransaction;


typedef CArrayTemplatePtr<CWriteTransaction> CArrayWriteTransaction;


//A durable bracket is always open while the Codabase is started, so the store can be read at any time.  Writes are
//made by write transactions, which run at the same time and lock the objects they use (wait-die), or by a direct
//session.  A direct session (Open ... Close) is an implicit write transaction that is always the oldest, so it waits
//but never dies.  Direct calls between Open and Close go through it, from any thread; Close commits it.  Without a
//session direct reads see the committed store and direct writes fail.
class CCodabase : public CDataConnection, public CKillable
{
friend class CWriteTransaction;
friend class CReadTransaction;
protected:
	CNamedIndexedData				mcNamedIndexedData;
	CDurableFileController			mcFileController;
	CLifeInit<CIndexedDataConfig>	mcIndexConfig;
	CLifeInit<CNamedIndexesConfig>	mcNamedConfig;
	bool							mbStarted;
	bool							mbControllerInitialised;
	CMutex							mcMutex;			//Held for each operation.  Even reads change the caches.
	bool							mbFailed;			//A commit failed part way.  No more writes until restarted.
	CCodabaseVersions				mcVersions;			//Old versions for read transactions.
	int64							miCommitNumber;		//The last commit.  The commit in progress is miCommitNumber + 1.
	CArrayInt64						maiSnapshots;		//The snapshot of every open read transaction.
	CLockManager					mcLocks;
	CMapStringTemplate<int64>		mcReservedNames;	//Names being added by uncommitted write transactions, and which one.
	int64							miLastStartNumber;  //Write transactions are numbered from 1.  The session is 0, the oldest.
	CArrayWriteTransaction			mapcWriters;	//Every write transaction that has not been killed.
	CMutex							mcSessionMutex;		//Held for each direct call while a session is open.
	CConditionVariable				mcSessionFinished;
	CWriteTransaction*				mpcSession;
	int								miSessionThread;

public:
	void	Init(const char* szDirectory, EDurableSync eSync, CLifeInit<CIndexedDataConfig> cIndexConfig, CLifeInit<CNamedIndexesConfig> cNamedConfig);
	bool	Start(void);
	void	Kill(void);

	bool	Open(void);
	bool	Close(void);

	CWriteTransaction*	BeginWriteTransaction(void);
	CReadTransaction*	BeginReadTransaction(void);
	CLockManager*		GetLockManager(void);
	bool				IsFailed(void);
	size				NumVersions(void);

	bool	ValidateConfigInitialised(void);
	bool	ValidateConfigKilled(void);
	bool	ValidateIdentifiers(void);

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

	int64	NumIndices(void);
	//While a session is open the cached counts include the objects and names it has added (not ones it changed).
	int64	NumIndicesCached(void);
	int64	NumIndicesCached(size iSize);
	int64	NumDataCached(void);
	int64	NumDataCached(size iSize);
	int64	NumNames(void);
	int64	NumNamesCached(void);

	void	Dump(void);

	//Each iteration call is locked, but an iteration may miss or see changes other threads make between its calls.
	OIndex 	StartIndexIteration(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize);
	OIndex	IndexIterate(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize);
	bool	StartNameIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi);
	bool	NameIterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi);

protected:
	//Used by the transactions.  Each takes the locks it needs.
	bool		StoredGet(OIndex oi, size* puiDataSize, void* pvData, size uiMaxDataSize);
	bool		StoredGet(OIndex oi, void* pvData);
	bool		StoredContains(OIndex oi);
	bool		StoredGetName(OIndex oi, CChars* pszName);
	OIndex		StoredGetIndex(const char* szName);
	void		StoredIndices(CArrayOIndex* paoi);
	void		StoredNames(CArrayChars* paszNames);

	bool		SnapshotGet(int64 iSnapshot, OIndex oi, size* puiDataSize, void* pvData, size uiMaxDataSize, bool bAllData);
	bool		SnapshotContains(int64 iSnapshot, OIndex oi);
	bool		SnapshotGetName(int64 iSnapshot, OIndex oi, CChars* pszName);
	OIndex		SnapshotGetIndex(int64 iSnapshot, const char* szName);
	void		SnapshotIndices(int64 iSnapshot, CArrayOIndex* paoi);
	void		SnapshotNames(int64 iSnapshot, CArrayChars* paszNames);
	void		EndRead(int64 iSnapshot);

	ELockResult	LockObject(int64 iTransaction, OIndex oi, ELockMode eMode);
	void		ReleaseLocks(int64 iTransaction, CArrayOIndex* paoiLocked);
	bool		ReserveName(const char* szName, int64 iTransaction, bool bRemovedByTransaction);
	void		ReleaseNames(CArrayChars* paszNames, int64 iTransaction);
	bool		CommitWriteSet(CMapBlock* pcRecords);
	void		RemoveWriter(CWriteTransaction* pcTransaction);

	bool		CommitLocked(void);
	bool		ApplyLocked(CMapBlock* pcRecords, bool bRemovals);
	bool		NeedsSession(const char* szMethod);
	bool		HasWriterOnThisThread(void);
	void		RemoveOldVersions(void);
	bool		SnapshotGetLocked(int64 iSnapshot, OIndex oi, size* puiDataSize, void* pvData, size uiMaxDataSize, bool bAllData);
	bool		SnapshotContainsLocked(int64 iSnapshot, OIndex oi);
	OIndex		SnapshotGetIndexLocked(int64 iSnapshot, const char* szName);
};


#endif // __CODABASE_H__

