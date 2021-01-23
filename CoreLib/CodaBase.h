#ifndef __CODABASE_H__
#define __CODABASE_H__
#include "BaseLib/Killable.h"
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

	BOOL	Open(void);
	BOOL	Close(void);

	BOOL	ValidateConfigInitialised(void);
	BOOL	ValidateConfigKilled(void);

	BOOL	Add(OIndex oi, void* pvData, unsigned int uiDataSize);
	BOOL	Add(OIndex oi, char* szName, void* pvData, unsigned int uiDataSize);
	BOOL	Add(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize);

	BOOL	Set(OIndex oi, void* pvData);
	BOOL	Set(OIndex oi, void* pvData, unsigned int uiDataSize);
	BOOL	Set(char* szName, void* pvData);
	BOOL	Set(char* szName, void* pvData, unsigned int uiDataSize);
	BOOL	Set(CChars* szName, void* pvData);
	BOOL	Set(CChars* szName, void* pvData, unsigned int uiDataSize);

	BOOL	Put(OIndex oi, void* pvData, unsigned int uiDataSize);
	BOOL	Put(OIndex oi, char* szName, void* pvData, unsigned int uiDataSize);
	BOOL	Put(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize);

	BOOL	Get(OIndex oi, void* pvData);
	BOOL	Get(OIndex oi, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize);
	BOOL	Get(char* szName, void* pvData);
	BOOL	Get(char* szName, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize);
	BOOL	Get(CChars* szName, void* pvData);
	BOOL	Get(CChars* szName, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize);

	BOOL	GetName(OIndex oi, CChars* szName);
	BOOL	GetName(OIndex oi, char* szName, unsigned int* puiNameLength, unsigned int uiMaxNameLength);
	OIndex	GetIndex(char* szName);
	OIndex	GetIndex(CChars* szName);

	BOOL	Contains(OIndex oi);
	BOOL	Contains(char* szName);
	BOOL	Contains(CChars* szName);

	BOOL	Remove(OIndex oi);
	BOOL	Remove(char* szName);
	BOOL	Remove(CChars* szName);

	BOOL	Flush(void);
	BOOL	Flush(BOOL bClearCache);

	int64	NumIndices(void);
	int64	NumIndicesCached(void);
	int64	NumNames(void);
	int64	NumNamesCached(void);
};


#endif // __CODABASE_H__

