#ifndef __WRITE_TRANSACTION_H__
#define __WRITE_TRANSACTION_H__
#include "DataConnection.h"
#include "MapBlock.h"
#include "MapStringTemplate.h"
#include "ArrayChars.h"
#include "CodabaseVersions.h"


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
//Abort, or Kill before Commit, discards them.  Only one writer (a write transaction or an Open ... Close session)
//is active at a time, so the stored objects a write transaction reads cannot be changed by anyone else.
//Iteration includes the transaction's own changes.  Only one index iteration and one name iteration can be in progress
//at a time; the iterator argument is not used.
class CWriteTransaction : public CDataConnection
{
protected:
	CCodabase*					mpcCodabase;
	CMapBlock					mcRecords;	//OIndex to SWriteRecord.
	CMapStringTemplate<OIndex>	mcNames;	//Names added or removed by the transaction.  INVALID_O_INDEX means removed.
	bool						mbActive;
	CArrayOIndex				maoiIteration;
	size						miIndexPosition;
	CArrayChars					maszIteration;
	size						miNamePosition;

public:
	void	Init(CCodabase* pcCodabase);
	void	Kill(void);

	bool	Commit(void);
	void	Abort(void);
	bool	IsActive(void);

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

protected:
	bool			CheckActive(const char* szMethod);
	SWriteRecord*	GetRecord(OIndex oi);
	bool			PutRecord(OIndex oi, bool bRemoved, bool bReplacesStored, const char* szName, void* pvData, uint32 uiDataSize);
	bool			StoredContains(OIndex oi);
	bool			Exists(OIndex oi);
	OIndex			FindName(const char* szName);
	bool			GetCurrentName(OIndex oi, CChars* pszName);
	bool			GetDataSize(OIndex oi, uint32* puiDataSize);
	bool			SetData(OIndex oi, void* pvData, uint32 uiDataSize);
	bool			Apply(bool bRemovals);
	void			Finish(void);
};


#endif // __WRITE_TRANSACTION_H__

