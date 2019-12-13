#ifndef __INDEX_TREE_TEMPLATE_MEMORY_H__
#define __INDEX_TREE_TEMPLATE_MEMORY_H__
#include "IndexTreeMemory.h"


template<class M>
class CIndexTreeTemplateMemory : public CIndexTreeMemory
{
public:
	M*		Put(void* pvKey, int iKeySize, M* pvObject);

	BOOL	Remove(void* pvKey, int iKeySize);
};


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
BOOL CIndexTreeTemplateMemory<M>::Remove(void* pvKey, int iKeySize)
{
	return CIndexTreeMemory::Remove(pvKey, iKeySize);
}


#endif // __INDEX_TREE_TEMPLATE_MEMORY_H__

