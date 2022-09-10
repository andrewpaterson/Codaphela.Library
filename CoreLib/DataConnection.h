#ifndef __DATA_CONNECTION_H__
#define __DATA_CONNECTION_H__
#include "BaseLib/Chars.h"
#include "CoreLib/IndexTreeFile.h"
#include "IndexedGeneral.h"


class CDataConnection
{
public:
	virtual bool	Add(OIndex oi, void* pvData, unsigned int uiDataSize) =0;
	virtual bool	Add(OIndex oi, char* szName, void* pvData, unsigned int uiDataSize) =0;
	virtual bool	Add(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize) =0;

	virtual bool	Set(OIndex oi, void* pvData) =0;
	virtual bool	Set(OIndex oi, void* pvData, unsigned int uiDataSize) =0;
	virtual bool	Set(char* szName, void* pvData) =0;
	virtual bool	Set(char* szName, void* pvData, unsigned int uiDataSize) =0;
	virtual bool	Set(CChars* szName, void* pvData) =0;
	virtual bool	Set(CChars* szName, void* pvData, unsigned int uiDataSize) =0;

	virtual bool	Put(OIndex oi, void* pvData, unsigned int uiDataSize) =0;
	virtual bool	Put(OIndex oi, char* szName, void* pvData, unsigned int uiDataSize) =0;
	virtual bool	Put(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize) =0;

	virtual bool	Get(OIndex oi, void* pvData) =0;
	virtual bool	Get(OIndex oi, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize) =0;
	virtual bool	Get(char* szName, void* pvData) =0;
	virtual bool	Get(char* szName, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize) =0;
	virtual bool	Get(CChars* szName, void* pvData) =0;
	virtual bool	Get(CChars* szName, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize) =0;

	virtual bool	GetName(OIndex oi, CChars* szName) =0;
	virtual bool	GetName(OIndex oi, char* szName, unsigned int* puiNameLength, unsigned int uiMaxNameLength) =0;
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

	virtual OIndex 	StartIndexIteration(SIndexTreeFileIterator* psIterator, void* pvData, size_t* piDataSize, size_t iMaxDataSize) =0;
	virtual OIndex	IndexIterate(SIndexTreeFileIterator* psIterator, void* pvData, size_t* piDataSize, size_t iMaxDataSize) =0;
	virtual bool	StartNameIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi) =0;
	virtual bool	NameIterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi) =0;
};


#endif // !__DATA_CONNECTION_H__

