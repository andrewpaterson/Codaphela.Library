#ifndef __INDEX_TEMPLATE_H__
#define __INDEX_TEMPLATE_H__
#include "IndexBlock.h"


template<class M, class D>
class CIndexTemplate : public CIndexBlock
{
	bool	Get(M* pvKey, D** ppvData);
	D*		Get(M* pvKey);

	D*		Put(M* pvKey);
	bool	Put(M* pvKey, void* pvData);

	bool	Remove(M* pvKey);

	size	DataSize(M* pvKey);

	bool	HasKey(M* pvKey);

	bool	StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, D** ppvData, M* pvDestKey);
	bool	Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, D** ppvData, M* pvDestKey);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
bool CIndexTemplate<M, D>::Get(M* pvKey, D** ppvData)
{
	size iDataSize;

	return CIndexBlock::Get(pvKey, sizeof(M), ppvData, &iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
D* CIndexTemplate<M, D>::Get(M* pvKey)
{
	return CIndexBlock::Get(pvKey, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
D* CIndexTemplate<M, D>::Put(M* pvKey)
{
	return CIndexBlock::Put(pvKey, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
bool CIndexTemplate<M, D>::Put(M* pvKey, void* pvData)
{
	return CIndexBlock::Put(pvKey, sizeof(M), sizeof(D));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
bool CIndexTemplate<M, D>::Remove(M* pvKey)
{
	return CIndexBlock::Remove(pvKey, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
size CIndexTemplate<M, D>::DataSize(M* pvKey)
{
	return CIndexBlock::DataSize(pvKey, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
bool CIndexTemplate<M, D>::HasKey(M* pvKey)
{
	return CIndexBlock::HasKey(pvKey, sizeof(M));
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
bool CIndexTemplate<M, D>::StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, D** ppvData, M* pvDestKey)
{
	return CIndexBlock::StartIteration(psIterator, ppvData, NULL, pvDestKey);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
bool CIndexTemplate<M, D>::Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, D** ppvData, M* pvDestKey)
{
	return CIndexBlock:Iterate(psIterator, ppvData, pvDestKey);
}


#endif // __INDEX_TEMPLATE_H__

