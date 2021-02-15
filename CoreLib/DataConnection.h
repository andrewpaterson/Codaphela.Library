#ifndef __DATA_CONNECTION_H__
#define __DATA_CONNECTION_H__
#include "BaseLib/Chars.h"
#include "CoreLib/IndexTreeFile.h"
#include "IndexedGeneral.h"


class CDataConnection
{
public:
	virtual BOOL	Add(OIndex oi, void* pvData, unsigned int uiDataSize) =0;
	virtual BOOL	Add(OIndex oi, char* szName, void* pvData, unsigned int uiDataSize) =0;
	virtual BOOL	Add(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize) =0;

	virtual BOOL	Set(OIndex oi, void* pvData) =0;
	virtual BOOL	Set(OIndex oi, void* pvData, unsigned int uiDataSize) =0;
	virtual BOOL	Set(char* szName, void* pvData) =0;
	virtual BOOL	Set(char* szName, void* pvData, unsigned int uiDataSize) =0;
	virtual BOOL	Set(CChars* szName, void* pvData) =0;
	virtual BOOL	Set(CChars* szName, void* pvData, unsigned int uiDataSize) =0;

	virtual BOOL	Put(OIndex oi, void* pvData, unsigned int uiDataSize) =0;
	virtual BOOL	Put(OIndex oi, char* szName, void* pvData, unsigned int uiDataSize) =0;
	virtual BOOL	Put(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize) =0;

	virtual BOOL	Get(OIndex oi, void* pvData) =0;
	virtual BOOL	Get(OIndex oi, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize) =0;
	virtual BOOL	Get(char* szName, void* pvData) =0;
	virtual BOOL	Get(char* szName, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize) =0;
	virtual BOOL	Get(CChars* szName, void* pvData) =0;
	virtual BOOL	Get(CChars* szName, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize) =0;

	virtual BOOL	GetName(OIndex oi, CChars* szName) =0;
	virtual BOOL	GetName(OIndex oi, char* szName, unsigned int* puiNameLength, unsigned int uiMaxNameLength) =0;
	virtual OIndex	GetIndex(char* szName) =0;
	virtual OIndex	GetIndex(CChars* szName) =0;

	virtual BOOL	Contains(OIndex oi) =0;
	virtual BOOL	Contains(char* szName) =0;
	virtual BOOL	Contains(CChars* szName) =0;

	virtual BOOL	Remove(OIndex oi) =0;
	virtual BOOL	Remove(char* szName) =0;
	virtual BOOL	Remove(CChars* szName) =0;

	virtual BOOL	Flush(void) =0;
	virtual BOOL	Flush(BOOL bClearCache) =0;

	virtual OIndex 	StartIndexIteration(SIndexTreeFileIterator* psIterator, void* pvData, size_t* piDataSize, size_t iMaxDataSize) =0;
	virtual OIndex	IndexIterate(SIndexTreeFileIterator* psIterator, void* pvData, size_t* piDataSize, size_t iMaxDataSize) =0;
	virtual BOOL	StartNameIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi) =0;
	virtual BOOL	NameIterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi) =0;
};


#endif // __DATA_CONNECTION_H__

