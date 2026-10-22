#ifndef __DATA_CONNECTION_H__
#define __DATA_CONNECTION_H__
#include "DataReader.h"


//Reading and writing objects by index or name.
class CDataConnection : public CDataReader
{
public:
	virtual bool	Add(OIndex oi, void* pvData, size uiDataSize) =0;
	virtual bool	Add(OIndex oi, const char* szName, void* pvData, size uiDataSize) =0;
	virtual bool	Add(OIndex oi, CChars* szName, void* pvData, size uiDataSize) =0;

	virtual bool	Set(OIndex oi, void* pvData) =0;
	virtual bool	Set(OIndex oi, void* pvData, size uiDataSize) =0;
	virtual bool	Set(const char* szName, void* pvData) =0;
	virtual bool	Set(const char* szName, void* pvData, size uiDataSize) =0;
	virtual bool	Set(CChars* szName, void* pvData) =0;
	virtual bool	Set(CChars* szName, void* pvData, size uiDataSize) =0;

	virtual bool	Put(OIndex oi, void* pvData, size uiDataSize) =0;
	virtual bool	Put(OIndex oi, const char* szName, void* pvData, size uiDataSize) =0;
	virtual bool	Put(OIndex oi, CChars* szName, void* pvData, size uiDataSize) =0;

	virtual bool	Remove(OIndex oi) =0;
	virtual bool	Remove(const char* szName) =0;
	virtual bool	Remove(CChars* szName) =0;

	virtual bool	Flush(void) =0;
	virtual bool	Flush(bool bClearCache) =0;
	virtual bool	IsWriteThrough(void) =0;
};


#endif // __DATA_CONNECTION_H__

