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
	void	Kill(void) override;

	int64	GetSnapshot(void) const;

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

	OIndex	StartIndexIteration(SIndexTreeFileIterator* psIterator, void* pvData, lsize* piDataSize, lsize iMaxDataSize) override;
	OIndex	IndexIterate(SIndexTreeFileIterator* psIterator, void* pvData, lsize* piDataSize, lsize iMaxDataSize) override;
	bool	StartNameIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi) override;
	bool	NameIterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi) override;
};


#endif // __READ_TRANSACTION_H__

