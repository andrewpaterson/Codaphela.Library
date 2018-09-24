#ifndef INDEX_TREE_TEMPLATE_EVICTING
#define INDEX_TREE_TEMPLATE_EVICTING
#include "IndexTreeEvicting.h"


template<class M>
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
template<class M>
BOOL CIndexTreeTemplateEvicting<M>::Get(void* pvKey, int iKeySize, M* pvObject)
{
	return CIndexTreeEvicting::Get(pvKey, iKeySize, pvObject, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CIndexTreeTemplateEvicting<M>::Put(void* pvKey, int iKeySize, M* pvObject)
{
	return CIndexTreeEvicting::Put(pvKey, iKeySize, pvObject, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CIndexTreeTemplateEvicting<M>::Remove(void* pvKey, int iKeySize)
{
	return CIndexTreeEvicting::Remove(pvKey, iKeySize);
}


#endif // INDEX_TREE_TEMPLATE_EVICTING

