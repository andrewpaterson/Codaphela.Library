#ifndef __INDEX_TEMPLATE_H__
#define __INDEX_TEMPLATE_H__
#include "IndexBlock.h"


template<class D>
class CIndexTemplate : public CIndexBlock
{
public:
	bool	Get(uint8* pvKey, size iKeySize, D** ppvData);
	D*		Get(uint8* pvKey, size iKeySize);

	D*		Put(uint8* pvKey, size iKeySize);
	bool	Put(uint8* pvKey, size iKeySize, void* pvData);

	bool	Remove(uint8* pvKey, size iKeySize);

	size	DataSize(uint8* pvKey, size iKeySize);

	bool	HasKey(uint8* pvKey, size iKeySize);

	bool	StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, void* pvDestKey, size* puiKeySize, size uiMaxKeySize, D** ppvData);
	bool	Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, void* pvDestKey, size* puiKeySize, size uiMaxKeySize, D** ppvData);
	bool	StartIteration(SIndexTreeMemoryIterator* psIterator, void* pvDestKey, size* puiKeySize, size uiMaxKeySize, D** ppvData);
	bool	Iterate(SIndexTreeMemoryIterator* psIterator, void* pvDestKey, size* puiKeySize, size uiMaxKeySize, D** ppvData);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
bool CIndexTemplate<D>::Get(uint8* pvKey, size iKeySize, D** ppvData)
{
	size iDataSize;

	return CIndexBlock::Get(pvKey, iKeySize, ppvData, &iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CIndexTemplate<D>::Get(uint8* pvKey, size iKeySize)
{
	return (D*)CIndexBlock::Get(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CIndexTemplate<D>::Put(uint8* pvKey, size iKeySize)
{
	return (D*)CIndexBlock::Put(pvKey, iKeySize, sizeof(D));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
bool CIndexTemplate<D>::Put(uint8* pvKey, size iKeySize, void* pvData)
{
	return CIndexBlock::Put(pvKey, iKeySize, pvData, sizeof(D));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
bool CIndexTemplate<D>::Remove(uint8* pvKey, size iKeySize)
{
	return CIndexBlock::Remove(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
size CIndexTemplate<D>::DataSize(uint8* pvKey, size iKeySize)
{
	return CIndexBlock::DataSize(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
bool CIndexTemplate<D>::HasKey(uint8* pvKey, size iKeySize)
{
	return CIndexBlock::HasKey(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
bool CIndexTemplate<D>::StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, void* pvDestKey, size* puiKeySize, size uiMaxKeySize, D** ppvData)
{
	return CIndexBlock::StartIteration(psIterator, (void**)ppvData, NULL, pvDestKey, puiKeySize, uiMaxKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
bool CIndexTemplate<D>::Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, void* pvDestKey, size* puiKeySize, size uiMaxKeySize, D** ppvData)
{
	return CIndexBlock::Iterate(psIterator, (void**)ppvData, NULL, pvDestKey, puiKeySize, uiMaxKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
bool CIndexTemplate<D>::StartIteration(SIndexTreeMemoryIterator* psIterator, void* pvDestKey, size* puiKeySize, size uiMaxKeySize, D** ppvData)
{
	return CIndexBlock::StartIteration(psIterator, (uint8*)pvDestKey, puiKeySize, uiMaxKeySize, (void**)ppvData, NULL, sizeof(D));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
bool CIndexTemplate<D>::Iterate(SIndexTreeMemoryIterator* psIterator, void* pvDestKey, size* puiKeySize, size uiMaxKeySize, D** ppvData)
{
	return CIndexBlock::Iterate(psIterator, (uint8*)pvDestKey, puiKeySize, uiMaxKeySize, (void**)ppvData, NULL, sizeof(D));
}


#endif // __INDEX_TEMPLATE_H__

