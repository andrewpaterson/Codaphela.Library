#include "ValueNamedIndexesConfig.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CValueNamedIndexesConfig::Init(char* szSubDirectory, size_t uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough)
{
	Init(szSubDirectory, uiIndexCacheSize, cEvictionStrategy, eWriteThrough, NULL, LifeNull<CIndexTreeDataOrderer>());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CValueNamedIndexesConfig::Init(char* szSubDirectory, size_t uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback, CLifeInit<CIndexTreeDataOrderer> cIndexTreeDataOrderer)
{
	muiIndexCacheSize = uiIndexCacheSize;
	mcEvictionStrategy = cEvictionStrategy;
	meWriteThrough = eWriteThrough;
	mpEvictionCallback = pcEvictionCallback;
	mszSubDirectory = szSubDirectory;
	mcIndexTreeDataOrderer = cIndexTreeDataOrderer;
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
CLifeInit<CIndexTreeEvictionStrategy> CValueNamedIndexesConfig::GetEvictionStrategy(void)
{
	return mcEvictionStrategy;
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
CLifeInit<CIndexTreeDataOrderer> CValueNamedIndexesConfig::GetIndexTreeDataOrderer(void)
{
	return mcIndexTreeDataOrderer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLifeInit<CNamedIndexesConfig> CValueNamedIndexesConfig::Create(char* szSubDirectory, size_t uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough)
{
	CLifeInit<CNamedIndexesConfig> cLife = LifeAlloc<CValueNamedIndexesConfig, CNamedIndexesConfig>(szSubDirectory, uiIndexCacheSize, cEvictionStrategy, eWriteThrough);
	return cLife;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLifeInit<CNamedIndexesConfig> CValueNamedIndexesConfig::Create(char* szSubDirectory, size_t uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback)
{
	CIndexTreeEvictionStrategy*			pcEvictionStrategy;
	CLifeInit<CIndexTreeDataOrderer>	cIndexTreeDataOrderer;
	CIndexTreeDataOrderer*				pcIndexTreeDataOrderer;

	pcEvictionStrategy = cEvictionStrategy.GetLife();
	pcIndexTreeDataOrderer = pcEvictionStrategy->GetDataOrderer();
	cIndexTreeDataOrderer.Init(pcIndexTreeDataOrderer, false, false);

	CLifeInit<CNamedIndexesConfig> cLife = LifeAlloc<CValueNamedIndexesConfig, CNamedIndexesConfig>(szSubDirectory, uiIndexCacheSize, cEvictionStrategy, eWriteThrough, pcEvictionCallback, cIndexTreeDataOrderer);
	return cLife;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLifeInit<CNamedIndexesConfig> CValueNamedIndexesConfig::Create(char* szSubDirectory, size_t uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback, CLifeInit<CIndexTreeDataOrderer> cIndexTreeDataOrderer)
{
	
	CLifeInit<CNamedIndexesConfig> cLife = LifeAlloc<CValueNamedIndexesConfig, CNamedIndexesConfig>(szSubDirectory, uiIndexCacheSize, cEvictionStrategy, eWriteThrough, pcEvictionCallback, cIndexTreeDataOrderer);
	return cLife;
}

