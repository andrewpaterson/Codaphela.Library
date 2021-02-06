#include "ValueIndexedDataConfig.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CValueIndexedDataConfig::Init(char* szSubDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy)
{
	Init(szSubDirectory, uiDataCacheSize, uiIndexCacheSize, eWriteThrough, cEvictionStrategy, NULL, NULL, LifeNull<CIndexTreeDataOrderer>());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CValueIndexedDataConfig::Init(char* szSubDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeEvictionCallback* pcIndexTreeEvictionUserCallback, CIndexedDataEvictionCallback* pcIndexedDataEvictionUserCallback, CLifeInit<CIndexTreeDataOrderer> cIndexTreeDataOrderer)
{
	mszSubDirectory = szSubDirectory;

	muiDataCacheSize = uiDataCacheSize;
	muiIndexCacheSize = uiIndexCacheSize;
	meWriteThrough = eWriteThrough;
	mpcIndexTreeEvictionUserCallback = pcIndexTreeEvictionUserCallback;
	mpcIndexedDataEvictionUserCallback = pcIndexedDataEvictionUserCallback;
	mcEvictionStrategy = cEvictionStrategy;
	mcIndexTreeDataOrderer = cIndexTreeDataOrderer;
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
CLifeInit<CIndexTreeEvictionStrategy> CValueIndexedDataConfig::GetEvictionStrategy(void)
{
	return mcEvictionStrategy;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLifeInit<CIndexTreeDataOrderer> CValueIndexedDataConfig::GetIndexTreeDataOrderer(void)
{
	return mcIndexTreeDataOrderer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLifeInit<CIndexedDataConfig> CValueIndexedDataConfig::Create(char* szSubDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy)
{
	CLifeInit<CIndexedDataConfig> cLife = LifeAlloc<CValueIndexedDataConfig, CIndexedDataConfig>(szSubDirectory, uiDataCacheSize, uiIndexCacheSize, eWriteThrough, cEvictionStrategy);
	return cLife;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLifeInit<CIndexedDataConfig> CValueIndexedDataConfig::Create(char* szSubDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeEvictionCallback* pcIndexTreeEvictionUserCallback, CIndexedDataEvictionCallback* pcIndexedDataEvictionUserCallback)
{
	CIndexTreeEvictionStrategy*			pcEvictionStrategy;
	CLifeInit<CIndexTreeDataOrderer>	cIndexTreeDataOrderer;
	CIndexTreeDataOrderer*				pcIndexTreeDataOrderer;

	pcEvictionStrategy = cEvictionStrategy.GetLife();
	pcIndexTreeDataOrderer = pcEvictionStrategy->GetDataOrderer();
	cIndexTreeDataOrderer.Init(pcIndexTreeDataOrderer, FALSE, FALSE);

	CLifeInit<CIndexedDataConfig> cLife = LifeAlloc<CValueIndexedDataConfig, CIndexedDataConfig>(szSubDirectory, uiDataCacheSize, uiIndexCacheSize, eWriteThrough, cEvictionStrategy, pcIndexTreeEvictionUserCallback, pcIndexedDataEvictionUserCallback, cIndexTreeDataOrderer);
	return cLife;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLifeInit<CIndexedDataConfig> CValueIndexedDataConfig::Create(char* szSubDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeEvictionCallback* pcIndexTreeEvictionUserCallback, CIndexedDataEvictionCallback* pcIndexedDataEvictionUserCallback, CLifeInit<CIndexTreeDataOrderer> cIndexTreeDataOrderer)
{
	CLifeInit<CIndexedDataConfig> cLife = LifeAlloc<CValueIndexedDataConfig, CIndexedDataConfig>(szSubDirectory, uiDataCacheSize, uiIndexCacheSize, eWriteThrough, cEvictionStrategy, pcIndexTreeEvictionUserCallback, pcIndexedDataEvictionUserCallback, cIndexTreeDataOrderer);
	return cLife;
}

