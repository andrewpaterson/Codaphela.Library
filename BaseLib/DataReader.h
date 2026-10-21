#ifndef __DATA_READER_H__
#define __DATA_READER_H__
#include "Chars.h"
#include "IndexTreeFile.h"
#include "IndexedGeneral.h"


//Reading objects by index or name.  CDataConnection adds writing.
class CDataReader
{
public:
	virtual void	Kill(void) =0;

	virtual bool	Get(OIndex oi, void* pvData) =0;
	virtual bool	Get(OIndex oi, size* puiDataSize, void* pvData, size uiMaxDataSize) =0;
	virtual bool	Get(const char* szName, void* pvData) =0;
	virtual bool	Get(const char* szName, size* puiDataSize, void* pvData, size uiMaxDataSize) =0;
	virtual bool	Get(CChars* szName, void* pvData) =0;
	virtual bool	Get(CChars* szName, size* puiDataSize, void* pvData, size uiMaxDataSize) =0;

	virtual bool	GetName(OIndex oi, CChars* szName) =0;
	virtual bool	GetName(OIndex oi, char* szName, size* puiNameLength, size uiMaxNameLength) =0;
	virtual OIndex	GetIndex(const char* szName) =0;
	virtual OIndex	GetIndex(CChars* szName) =0;

	virtual bool	Contains(OIndex oi) =0;
	virtual bool	Contains(const char* szName) =0;
	virtual bool	Contains(CChars* szName) =0;

	virtual OIndex 	StartIndexIteration(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize) =0;
	virtual OIndex	IndexIterate(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize) =0;
	virtual bool	StartNameIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi) =0;
	virtual bool	NameIterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi) =0;
};


#endif // __DATA_READER_H__

