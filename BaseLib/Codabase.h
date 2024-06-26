#ifndef __CODABASE_H__
#define __CODABASE_H__
#include "Killable.h"
#include "NamedIndexedData.h"
#include "DataConnection.h"


class CCodabase : public CDataConnection, public CKillable
{
protected:
	CNamedIndexedData			mcNamedIndexedData;
	CDurableFileController		mcFileController;

public:
	void	Init(char* szDirectory, CLifeInit<CIndexedDataConfig> cIndexConfig, CLifeInit<CNamedIndexesConfig> cNamedConfig);
	void	Kill(void);

	bool	Open(void);
	bool	Close(void);

	bool	ValidateConfigInitialised(void);
	bool	ValidateConfigKilled(void);
	bool	ValidateIdentifiers(void);

	bool	Add(OIndex oi, void* pvData, uint32 uiDataSize);
	bool	Add(OIndex oi, char* szName, void* pvData, uint32 uiDataSize);
	bool	Add(OIndex oi, CChars* szName, void* pvData, uint32 uiDataSize);

	bool	Set(OIndex oi, void* pvData);
	bool	Set(OIndex oi, void* pvData, uint32 uiDataSize);
	bool	Set(char* szName, void* pvData);
	bool	Set(char* szName, void* pvData, uint32 uiDataSize);
	bool	Set(CChars* szName, void* pvData);
	bool	Set(CChars* szName, void* pvData, uint32 uiDataSize);

	bool	Put(OIndex oi, void* pvData, uint32 uiDataSize);
	bool	Put(OIndex oi, char* szName, void* pvData, uint32 uiDataSize);
	bool	Put(OIndex oi, CChars* szName, void* pvData, uint32 uiDataSize);

	bool	Get(OIndex oi, void* pvData);
	bool	Get(OIndex oi, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize);
	bool	Get(char* szName, void* pvData);
	bool	Get(char* szName, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize);
	bool	Get(CChars* szName, void* pvData);
	bool	Get(CChars* szName, uint32* puiDataSize, void* pvData, uint32 uiMaxDataSize);

	bool	GetName(OIndex oi, CChars* szName);
	bool	GetName(OIndex oi, char* szName, uint32* puiNameLength, uint32 uiMaxNameLength);
	OIndex	GetIndex(char* szName);
	OIndex	GetIndex(CChars* szName);

	bool	Contains(OIndex oi);
	bool	Contains(char* szName);
	bool	Contains(CChars* szName);

	bool	Remove(OIndex oi);
	bool	Remove(char* szName);
	bool	Remove(CChars* szName);

	bool	Flush(void);
	bool	Flush(bool bClearCache);

	int64	NumIndices(void);
	int64	NumIndicesCached(void);
	int64	NumIndicesCached(size iSize);
	int64	NumDataCached(void);
	int64	NumDataCached(size iSize);
	int64	NumNames(void);
	int64	NumNamesCached(void);

	void	Dump(void);

	OIndex 	StartIndexIteration(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize);
	OIndex	IndexIterate(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize);
	bool	StartNameIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi);
	bool	NameIterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi);
};


#endif // __CODABASE_H__

