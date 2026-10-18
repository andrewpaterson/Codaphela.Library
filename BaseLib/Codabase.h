#ifndef __CODABASE_H__
#define __CODABASE_H__
#include "Killable.h"
#include "NamedIndexedData.h"
#include "DataConnection.h"
#include "Mutex.h"
#include "ConditionVariable.h"
#include "CodabaseVersions.h"


class CWriteTransaction;
class CReadTransaction;


//A durable bracket is always open while the Codabase is started, so the store can be read at any time.  Writes are
//committed either by a write transaction or by a direct session (Open ... Close).  Only one writer, transaction or
//session, is active at a time; others wait for it to finish.
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
	CMutex							mcMutex;  //Held for each operation.  Even reads change the caches.
	CConditionVariable				mcWriterFinished;
	bool							mbWriterActive;
	int								miWriterThread;
	bool							mbSessionOpen;
	bool							mbFailed;  //A commit failed part way.  No more writes until restarted.
	CCodabaseVersions				mcVersions;  //Old versions for read transactions.
	int64							miCommitNumber;  //The last commit.  The commit in progress is miCommitNumber + 1.
	CArrayTemplate<int64>			maiSnapshots;  //The snapshot of every open read transaction.

public:
	void	Init(const char* szDirectory, EDurableSync eSync, CLifeInit<CIndexedDataConfig> cIndexConfig, CLifeInit<CNamedIndexesConfig> cNamedConfig);
	bool	Start(void);
	void	Kill(void);

	bool	Open(void);
	bool	Close(void);

	CWriteTransaction*	BeginWriteTransaction(void);
	CReadTransaction*	BeginReadTransaction(void);
	bool				IsFailed(void);
	int64				GetCommitNumber(void);
	size				NumVersions(void);

	bool	ValidateConfigInitialised(void);
	bool	ValidateConfigKilled(void);
	bool	ValidateIdentifiers(void);

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

	int64	NumIndices(void);
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
	bool	AcquireWriter(void);
	void	ReleaseWriter(void);
	bool	CommitLocked(void);
	bool	CanWriteDirectly(void);
	void	SaveVersions(OIndex oi, const char* szNewName);
	void	RemoveOldVersions(void);
	void	EndRead(int64 iSnapshot);
	void	GetStoredIndices(CArrayOIndex* paoi);
	void	GetStoredNames(CArrayChars* paszNames);
};


#endif // __CODABASE_H__

