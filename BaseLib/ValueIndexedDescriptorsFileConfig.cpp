#include "ValueIndexedDescriptorsFileConfig.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CValueIndexedDescriptorsFileConfig::Init(char* szSubdirectory, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionStrategy* pcEvictionStrategy)
{
	Init(szSubdirectory, uiIndexCacheSize, eWriteThrough, pcEvictionStrategy, NULL, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CValueIndexedDescriptorsFileConfig::Init(char* szSubdirectory, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionStrategy* pcEvictionStrategy, CIndexedDataEvictionCallback* pcEvictionUserCallback, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback)
{
	mszSubdirectory = szSubdirectory;
	muiIndexCacheSize = uiIndexCacheSize;
	meWriteThrough = eWriteThrough;
	mpcEvictionStrategy = pcEvictionStrategy;
	mpcIndexedDataEvictionUserCallback = pcEvictionUserCallback;
	mpcIndexTreeEvictionCallback = pcIndexTreeEvictionCallback;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CValueIndexedDescriptorsFileConfig::GetSubdirectory(void)
{
	return mszSubdirectory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CValueIndexedDescriptorsFileConfig::GetIndexCacheSize(void)
{
	return muiIndexCacheSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EIndexWriteThrough CValueIndexedDescriptorsFileConfig::GetWriteThrough(void)
{
	return meWriteThrough;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeEvictionStrategy* CValueIndexedDescriptorsFileConfig::GetEvictionStrategy()
{
	return mpcEvictionStrategy;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexedDataEvictionCallback* CValueIndexedDescriptorsFileConfig::GetIndexedDataEvictionUserCallback(void)
{
	return mpcIndexedDataEvictionUserCallback;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeEvictionCallback* CValueIndexedDescriptorsFileConfig::GetIndexTreeEvictionCallback(void)
{
	return mpcIndexTreeEvictionCallback;
}
