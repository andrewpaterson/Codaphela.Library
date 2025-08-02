#ifndef __DATA_CONNECTION_H__
#define __DATA_CONNECTION_H__
#include "Chars.h"
#include "IndexTreeFile.h"
#include "IndexedGeneral.h"


class CDataConnection
{
public:
	virtual bool	Add(OIndex oi, void* pvData, uint32 uiDataSize) =0;
	virtual bool	Add(OIndex oi, char* szName, void* pvData, uint32 uiDataSize) =0;
	virtual bool	Add(OIndex oi, CChars* szName, void* pvData, uint32 uiDataSize) =0;

	virtual bool	Set(OIndex oi, void* pvData) =0;
	virtual bool	Set(OIndex oi, void* pvData, uint32 uiDataSize) =0;
	virtual bool	Set(char* szName, void* pvData) =0;
	virtual bool	Set(char* szName, void* pvData, uint32 uiDataSize) =0;
	virtual bool	Set(CChars* szName, void* pvData) =0;
	virtual bool	Set(CChars* szName, void* pvData, uint32 uiDataSize) =0;

	virtual bool	Put(OIndex oi, void* pvData, uint32 uiDataSize) =0;
	virtual bool	Put(OIndex oi, char* szName, void* pvData, uint32 uiDataSize) =0;
	virtual bool	Put(OIndex oi, CChars* szName, void* pvData, uint32 uiDataSize) =0;

	virtual bool	Get(OIndex oi, void* pvData) =0;
	virtual bool	Get(OIndex oi, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize) =0;
	virtual bool	Get(char* szName, void* pvData) =0;
	virtual bool	Get(char* szName, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize) =0;
	virtual bool	Get(CChars* szName, void* pvData) =0;
	virtual bool	Get(CChars* szName, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize) =0;

	virtual bool	GetName(OIndex oi, CChars* szName) =0;
	virtual bool	GetName(OIndex oi, char* szName, uint32* puiNameLength, uint32 uiMaxNameLength) =0;
	virtual OIndex	GetIndex(char* szName) =0;
	virtual OIndex	GetIndex(CChars* szName) =0;

	virtual bool	Contains(OIndex oi) =0;
	virtual bool	Contains(char* szName) =0;
	virtual bool	Contains(CChars* szName) =0;

	virtual bool	Remove(OIndex oi) =0;
	virtual bool	Remove(char* szName) =0;
	virtual bool	Remove(CChars* szName) =0;

	virtual bool	Flush(void) =0;
	virtual bool	Flush(bool bClearCache) =0;
	virtual bool	IsWriteThrough(void) =0;

	virtual OIndex 	StartIndexIteration(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize) =0;
	virtual OIndex	IndexIterate(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize) =0;
	virtual bool	StartNameIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi) =0;
	virtual bool	NameIterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi) =0;
};


#endif // __DATA_CONNECTION_H__

