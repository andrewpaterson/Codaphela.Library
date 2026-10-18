#ifndef __READ_TRANSACTION_H__
#define __READ_TRANSACTION_H__
#include "Chars.h"
#include "ArrayChars.h"
#include "IndexTreeFile.h"
#include "CodabaseVersions.h"


class CCodabase;


//Reads a Codabase as it was at the last commit before the transaction began (its snapshot).  It never waits for a
//writer and never sees changes committed after its snapshot, including while they are being written.  Kill ends it.
//Only one index iteration and one name iteration can be in progress at a time; the iterator argument is not used.
class CReadTransaction
{
protected:
	CCodabase*		mpcCodabase;
	int64			miSnapshot;
	bool			mbActive;
	CArrayOIndex	maoiIteration;
	size			miIndexPosition;
	CArrayChars		maszIteration;
	size			miNamePosition;

public:
	void	Init(CCodabase* pcCodabase, int64 iSnapshot);
	void	Kill(void);

	int64	GetSnapshot(void);

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

	OIndex 	StartIndexIteration(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize);
	OIndex	IndexIterate(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize);
	bool	StartNameIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi);
	bool	NameIterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi);

protected:
	bool	GetLocked(OIndex oi, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize, bool bAllData);
	OIndex	GetIndexLocked(const char* szName);
	bool	GetNameLocked(OIndex oi, CChars* pszName);
	bool	ContainsLocked(OIndex oi);
};


#endif // __READ_TRANSACTION_H__

