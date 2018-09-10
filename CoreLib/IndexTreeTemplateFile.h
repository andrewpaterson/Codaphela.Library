#ifndef __INDEX_TREE_TEMPLATE_FILE_H__
#define __INDEX_TREE_TEMPLATE_FILE_H__
#include "IndexTreeFile.h"


template<class M, class D>
class CIndexTreeTemplateFile : public CIndexTreeFile
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
BOOL CIndexTreeTemplateFile<M, D>::Get(void* pvKey, int iKeySize, M* pvObject)
{
	return CIndexTreeFile::Get(pvKey, iKeySize, pvObject, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
BOOL CIndexTreeTemplateFile<M, D>::Put(void* pvKey, int iKeySize, M* pvObject)
{
	return CIndexTreeFile::Put(pvKey, iKeySize, pvObject, sizeof(M), sizeof(D));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
BOOL CIndexTreeTemplateFile<M, D>::Remove(void* pvKey, int iKeySize)
{
	return CIndexTreeFile::Remove(pvKey, iKeySize);
}


#endif // __INDEX_TREE_TEMPLATE_FILE_H__

