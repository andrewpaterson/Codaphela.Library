#ifndef __INDEX_PRIMITIVE_TEMPLATE_H__
#define __INDEX_PRIMITIVE_TEMPLATE_H__
#include "IndexPrimitiveBlock.h"


template<class M, class D>
class CIndexPrimitiveTemplate : public CIndexPrimitiveBlock<M>
{
public:
	BOOL	Get(M sData, D** ppsData);
	D*		Get(M sData);

	D*		Put(M sData);
	BOOL	Put(M sData, D* psData);

	BOOL	StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, D** ppsData, M* psDestKey);
	BOOL	Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, D** ppsData, M* psDestKey);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
BOOL CIndexPrimitiveTemplate<M, D>::Get(M sData, D** ppsData)
{
	return CIndexPrimitiveBlock<M>::Get(sData, ppsData, NULL);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
D* CIndexPrimitiveTemplate<M, D>::Get(M sData)
{
	return CIndexPrimitiveBlock<M>::Get(sData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
D* CIndexPrimitiveTemplate<M, D>::Put(M sData)
{
	return CIndexPrimitiveBlock<M>::Put(sData, sizof(D));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
BOOL CIndexPrimitiveTemplate<M, D>::Put(M sData, D* psData)
{
	return CIndexPrimitiveBlock<M>::Put(sData, psData, sizef(D));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
BOOL CIndexPrimitiveTemplate<M, D>::StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, D** ppsData, M* psDestKey)
{
	return CIndexPrimitiveBlock<M>::StartIteration(psIterator, ppsData, NULL, psDestKey);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
BOOL CIndexPrimitiveTemplate<M, D>::Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, D** ppsData, M* psDestKey)
{
	return CIndexPrimitiveBlock<M>::Iterate(psIterator, ppsData, NULL, psDestKey);
}


#endif // !__INDEX_PRIMITIVE_TEMPLATE_H__

