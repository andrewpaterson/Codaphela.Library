#ifndef __INDEX_TREE_TEMPLATE_H__
#define __INDEX_TREE_TEMPLATE_H__
#include "IndexTreeMemory.h"


template<class M>
class CIndexTreeTemplateMemory : public CIndexTreeMemory
{
public:
	M*		Get(char* pszKey);
	M*		Get(void* pvKey, int iKeySize);

	M*		Put(char* pszKey, M* pvObject);
	M*		Put(void* pvKey, int iKeySize, M* pvObject);

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
	return (M*)CIndexTreeMemory::Get(pszKey);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CIndexTreeTemplateMemory<M>::Get(void* pvKey, int iKeySize)
{
	return (M*)CIndexTreeMemory::Get(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CIndexTreeTemplateMemory<M>::Put(char* pszKey, M* pvObject)
{
	return (M*)CIndexTreeMemory::Put(pszKey, pvObject, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CIndexTreeTemplateMemory<M>::Put(void* pvKey, int iKeySize, M* pvObject)
{
	return (M*)CIndexTreeMemory::Put(pvKey, iKeySize, pvObject, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CIndexTreeTemplateMemory<M>::Remove(char* pszKey)
{
	return CIndexTreeMemory::Remove(pszKey);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CIndexTreeTemplateMemory<M>::Remove(void* pvKey, int iKeySize)
{
	return CIndexTreeMemory::Remove(pvKey, iKeySize);
}


#endif // __INDEX_TREE_TEMPLATE_H__

