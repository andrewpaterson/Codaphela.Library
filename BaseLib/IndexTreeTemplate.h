#ifndef __INDEX_TREE_TEMPLATE_H__
#define __INDEX_TREE_TEMPLATE_H__
#include "IndexTreeBlock.h"


template<class M>
class CIndexTreeTemplateMemory : public CIndexTreeBlockMemory
{
public:
	M*		Get(char* pszKey);
	M*		Get(void* pvKey, int iKeySize);

	BOOL	Put(char* pszKey, M* pvObject);
	BOOL	Put(void* pvKey, int iKeySize, M* pvObject);

	BOOL	Remove(char* pszKey);
	BOOL	Remove(void* pvKey, int iKeySize);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CIndexTreeTemplateMemory<M>::Get(char* pszKey)
{
	return (M*)CIndexTreeBlockMemory::Get(pszKey);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CIndexTreeTemplateMemory<M>::Get(void* pvKey, int iKeySize)
{
	return (M*)CIndexTreeBlockMemory::Get(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CIndexTreeTemplateMemory<M>::Put(char* pszKey, M* pvObject)
{
	return CIndexTreeBlockMemory::Put(pszKey, pvObject, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CIndexTreeTemplateMemory<M>::Put(void* pvKey, int iKeySize, M* pvObject)
{
	return CIndexTreeBlockMemory::Put(pvKey, iKeySize, pvObject, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CIndexTreeTemplateMemory<M>::Remove(char* pszKey)
{
	return CIndexTreeBlockMemory::Remove(pszKey);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CIndexTreeTemplateMemory<M>::Remove(void* pvKey, int iKeySize)
{
	return CIndexTreeBlockMemory::Remove(pvKey, iKeySize);
}


#endif // __INDEX_TREE_TEMPLATE_H__

