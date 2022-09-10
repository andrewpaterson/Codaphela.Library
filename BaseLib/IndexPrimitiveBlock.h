#ifndef __INDEX_PRIMITIVE_BLOCK_H__
#define __INDEX_PRIMITIVE_BLOCK_H__
#include "IndexBlock.h"


template<class M>
class CIndexPrimitiveBlock : public CIndexBlock
{
public:
	bool	Get(M sData, void** ppvData, int* piDataSize);
	void*	Get(M sData);

	void*	Put(M sData, int iDataSize);
	bool	Put(M sData, void* pvData, int iDataSize);

	bool	Remove(M sData);

	size_t	DataSize(M sData);

	bool	HasKey(M sData);

	bool	StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size_t* puiDataSize, M* psDestKey);
	bool	Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size_t* puiDataSize, M* psDestKey);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CIndexPrimitiveBlock<M>::Get(M sData, void** ppvData, int* piDataSize)
{
	return CIndexBlock::Get(&sData, sizeof(M), ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void* CIndexPrimitiveBlock<M>::Get(M sData)
{
	return CIndexBlock::Get(&sData, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void* CIndexPrimitiveBlock<M>::Put(M sData, int iDataSize)
{
	return CIndexBlock::Put(&sData, sizeof(M), iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CIndexPrimitiveBlock<M>::Put(M sData, void* pvData, int iDataSize)
{
	return CIndexBlock::Put(&sData, sizeof(M), pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CIndexPrimitiveBlock<M>::Remove(M sData)
{
	return CIndexBlock::Remove(&sData, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size_t CIndexPrimitiveBlock<M>::DataSize(M sData)
{
	return CIndexBlock::DataSize(&sData, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CIndexPrimitiveBlock<M>::HasKey(M sData)
{
	return CIndexBlock::HasKey(&sData, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CIndexPrimitiveBlock<M>::StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size_t* puiDataSize, M* psDestKey)
{
	return CIndexBlock::StartIteration(psIterator, ppvData, puiDataSize, psDestKey, NULL, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CIndexPrimitiveBlock<M>::Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, void** ppvData, size_t* puiDataSize, M* psDestKey)
{
	return CIndexBlock::Iterate(psIterator, ppvData, puiDataSize, psDestKey, NULL, 0);
}


#endif // !__INDEX_PRIMITIVE_BLOCK_H__

