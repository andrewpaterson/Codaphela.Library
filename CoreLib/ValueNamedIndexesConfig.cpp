#include "ValueNamedIndexesConfig.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CValueNamedIndexesConfig::Init(char* szSubDirectory, size_t uiIndexCacheSize, CIndexTreeEvictionStrategy* pcNamedEvictionStrategy, EIndexWriteThrough eWriteThrough)
{
	Init(szSubDirectory, uiIndexCacheSize, pcNamedEvictionStrategy, eWriteThrough, NULL, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CValueNamedIndexesConfig::Init(char* szSubDirectory, size_t uiIndexCacheSize, CIndexTreeEvictionStrategy* pcNamedEvictionStrategy, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback, CIndexTreeDataOrderer* pcIndexTreeDataOrderer)
{
	muiIndexCacheSize = uiIndexCacheSize;
	mpcNamedEvictionStrategy = pcNamedEvictionStrategy;
	meWriteThrough = eWriteThrough;
	mpEvictionCallback = pcEvictionCallback;
	mszSubDirectory = szSubDirectory;
	mpcIndexTreeDataOrderer = pcIndexTreeDataOrderer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CValueNamedIndexesConfig::Kill(void)
{
	CNamedIndexesConfig::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CValueNamedIndexesConfig::GetIndexCacheSize(void)
{
	return muiIndexCacheSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeEvictionStrategy* CValueNamedIndexesConfig::GetEvictionStrategy(void)
{
	return mpcNamedEvictionStrategy;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EIndexWriteThrough CValueNamedIndexesConfig::GetWriteThrough(void)
{
	return meWriteThrough;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeEvictionCallback* CValueNamedIndexesConfig::GetIndexTreeEvictionCallback(void)
{
	return mpEvictionCallback;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CValueNamedIndexesConfig::GetSubDirectory(void)
{
	return mszSubDirectory;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeDataOrderer* CValueNamedIndexesConfig::GetIndexTreeDataOrderer(void)
{
	return mpcIndexTreeDataOrderer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLifeInit<CNamedIndexesConfig> CValueNamedIndexesConfig::Create(char* szSubDirectory, size_t uiIndexCacheSize, CIndexTreeEvictionStrategy* pcNamedEvictionStrategy, EIndexWriteThrough eWriteThrough)
{
	LIFE_ALLOC(CValueNamedIndexesConfig, CNamedIndexesConfig)->Init(szSubDirectory, uiIndexCacheSize, pcNamedEvictionStrategy, eWriteThrough);
	return cLife;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLifeInit<CNamedIndexesConfig> CValueNamedIndexesConfig::Create(char* szSubDirectory, size_t uiIndexCacheSize, CIndexTreeEvictionStrategy* pcNamedEvictionStrategy, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback, CIndexTreeDataOrderer* pcIndexTreeDataOrderer)
{
	LIFE_ALLOC(CValueNamedIndexesConfig, CNamedIndexesConfig)->Init(szSubDirectory, uiIndexCacheSize, pcNamedEvictionStrategy, eWriteThrough, pcEvictionCallback, pcIndexTreeDataOrderer);
	return cLife;
}

