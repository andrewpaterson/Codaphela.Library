#ifndef INDEX_TREE_TEMPLATE_EVICTING
#define INDEX_TREE_TEMPLATE_EVICTING
#include "IndexTreeEvicting.h"


template<class M, class D>
class CIndexTreeTemplateEvicting : public CIndexTreeEvicting
{
public:
	BOOL	Get(void* pvKey, int iKeySize, M* pvObject);

	BOOL	Put(void* pvKey, int iKeySize, M* pvObject);

	BOOL	Remove(void* pvKey, int iKeySize);
};



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
BOOL CIndexTreeTemplateEvicting<M, D>::Get(void* pvKey, int iKeySize, M* pvObject)
{
	return CIndexTreeEvicting::Get(pvKey, iKeySize, pvObject, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
BOOL CIndexTreeTemplateEvicting<M, D>::Put(void* pvKey, int iKeySize, M* pvObject)
{
	return CIndexTreeEvicting::Put(pvKey, iKeySize, pvObject, sizeof(M), sizeof(D));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
BOOL CIndexTreeTemplateEvicting<M, D>::Remove(void* pvKey, int iKeySize)
{
	return CIndexTreeEvicting::Remove(pvKey, iKeySize);
}


#endif // INDEX_TREE_TEMPLATE_EVICTING

