#ifndef __READ_TRANSACTION_H__
#define __READ_TRANSACTION_H__
#include "DataReader.h"
#include "TransactionIteration.h"


class CCodabase;


//Reads a Codabase as it was at the last commit before the transaction began (its snapshot).  It never waits for a
//writer and never sees changes committed after its snapshot, including while they are being written.  Kill ends it.
//Only one index iteration and one name iteration can be in progress at a time; the iterator argument is not used.
class CReadTransaction : public CDataReader
{
protected:
	CCodabase*				mpcCodabase;
	int64					miSnapshot;
	bool					mbActive;
	CTransactionIteration	mcIteration;

public:
	void	Init(CCodabase* pcCodabase, int64 iSnapshot);
	void	Kill(void);

	int64	GetSnapshot(void);

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

	OIndex 	StartIndexIteration(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize);
	OIndex	IndexIterate(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize);
	bool	StartNameIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi);
	bool	NameIterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi);
};


#endif // __READ_TRANSACTION_H__

