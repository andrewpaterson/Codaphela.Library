#ifndef __INDEX_TREE_TEMPLATE_FILE_H__
#define __INDEX_TREE_TEMPLATE_FILE_H__
#include "IndexTreeFile.h"


template<class M>
class CIndexTreeTemplateFile : public CIndexTreeFile
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
M* CIndexTreeTemplateFile<M>::Get(char* pszKey)
{
	return (M*)CIndexTreeFile::Get(pszKey);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CIndexTreeTemplateFile<M>::Get(void* pvKey, int iKeySize)
{
	return (M*)CIndexTreeFile::Get(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CIndexTreeTemplateFile<M>::Put(char* pszKey, M* pvObject)
{
	return (M*)CIndexTreeFile::Put(pszKey, pvObject, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CIndexTreeTemplateFile<M>::Put(void* pvKey, int iKeySize, M* pvObject)
{
	return (M*)CIndexTreeFile::Put(pvKey, iKeySize, pvObject, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CIndexTreeTemplateFile<M>::Remove(char* pszKey)
{
	return CIndexTreeFile::Remove(pszKey);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CIndexTreeTemplateFile<M>::Remove(void* pvKey, int iKeySize)
{
	return CIndexTreeFile::Remove(pvKey, iKeySize);
}


#endif // __INDEX_TREE_TEMPLATE_FILE_H__

