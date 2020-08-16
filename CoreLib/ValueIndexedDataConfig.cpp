#include "ValueIndexedDataConfig.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CValueIndexedDataConfig::Init(char* szSubDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionStrategy* pcEvictionStrategy)
{
	Init(szSubDirectory, uiDataCacheSize, uiIndexCacheSize, eWriteThrough, pcEvictionStrategy, NULL, NULL, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CValueIndexedDataConfig::Init(char* szSubDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionStrategy* pcEvictionStrategy, CIndexTreeEvictionCallback* pcIndexTreeEvictionUserCallback, CIndexedDataEvictionCallback* pcIndexedDataEvictionUserCallback, CIndexTreeDataOrderer* pcIndexTreeDataOrderer)
{
	mszSubDirectory = szSubDirectory;

	muiDataCacheSize = uiDataCacheSize;
	muiIndexCacheSize = uiIndexCacheSize;
	meWriteThrough = eWriteThrough;
	mpcIndexTreeEvictionUserCallback = pcIndexTreeEvictionUserCallback;
	mpcIndexedDataEvictionUserCallback = pcIndexedDataEvictionUserCallback;
	mpcEvictionStrategy = pcEvictionStrategy;
	mpcIndexTreeDataOrderer = pcIndexTreeDataOrderer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CValueIndexedDataConfig::Kill(void)
{
	CIndexedDataConfig::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CValueIndexedDataConfig::GetSubdirectory(void)
{
	return mszSubDirectory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CValueIndexedDataConfig::GetDataCacheSize(void)
{
	return muiDataCacheSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CValueIndexedDataConfig::GetIndexCacheSize(void)
{
	return muiIndexCacheSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EIndexWriteThrough CValueIndexedDataConfig::GetWriteThrough(void)
{
	return meWriteThrough;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeEvictionCallback* CValueIndexedDataConfig::GetIndexTreeEvictionUserCallback(void)
{
	return mpcIndexTreeEvictionUserCallback;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexedDataEvictionCallback* CValueIndexedDataConfig::GetIndexedDataEvictionUserCallback(void)
{
	return mpcIndexedDataEvictionUserCallback;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeEvictionStrategy* CValueIndexedDataConfig::GetEvictionStrategy(void)
{
	return mpcEvictionStrategy;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeDataOrderer* CValueIndexedDataConfig::GetIndexTreeDataOrderer(void)
{
	return mpcIndexTreeDataOrderer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLifeInit<CIndexedDataConfig> CValueIndexedDataConfig::Create(char* szSubDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionStrategy* pcEvictionStrategy)
{
	LIFE_ALLOC(CValueIndexedDataConfig, CIndexedDataConfig)->Init(szSubDirectory, uiDataCacheSize, uiIndexCacheSize, eWriteThrough, pcEvictionStrategy);
	return cLife;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLifeInit<CIndexedDataConfig> CValueIndexedDataConfig::Create(char* szSubDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionStrategy* pcEvictionStrategy, CIndexTreeEvictionCallback* pcIndexTreeEvictionUserCallback, CIndexedDataEvictionCallback* pcIndexedDataEvictionUserCallback, CIndexTreeDataOrderer* pcIndexTreeDataOrderer)
{
	LIFE_ALLOC(CValueIndexedDataConfig, CIndexedDataConfig)->Init(szSubDirectory, uiDataCacheSize, uiIndexCacheSize, eWriteThrough, pcEvictionStrategy, pcIndexTreeEvictionUserCallback, pcIndexedDataEvictionUserCallback, pcIndexTreeDataOrderer);
	return cLife;
}

