#ifndef __MAP_PTR_PRIMITIVE_TEMPLATE_H__
#define __MAP_PTR_PRIMITIVE_TEMPLATE_H__
#include "PointerFunctions.h"
#include "MapTemplate.h"


template<class D>
class CMapPtrPrimitiveTemplate : public CMapTemplate<void*, D>
{
public:
	void	Init(void);
	void	Init(CMallocator* pcMalloc, int iChunkSize);

	D		Get(void* pvKey);

	D*		Put(void* pvKey);
	BOOL	Put(void* pvKey, D sData);

	BOOL	Remove(void* pvKey);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapPtrPrimitiveTemplate<D>::Init(void)
{
	CMapBlock::Init(&ComparePtrPtr);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapPtrPrimitiveTemplate<D>::Init(CMallocator* pcMalloc, int iChunkSize)
{
	CMapBlock::Init(pcMalloc, iChunkSize, &ComparePtrPtr, TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class D>
D CMapPtrPrimitiveTemplate<D>::Get(void* pvKey)
{
	D* psResult;

	psResult = CMapTemplate<void*, D>::Get(&pvKey);
	if (psResult)
	{
		return *psResult;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CMapPtrPrimitiveTemplate<D>::Put(void* pvKey)
{
	return CMapTemplate<void*, D>::Put(&pvKey);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class D>
BOOL CMapPtrPrimitiveTemplate<D>::Put(void* pvKey, D sData)
{
	return CMapTemplate<void*, D>::Put(&pvKey, &sData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class D>
BOOL CMapPtrPrimitiveTemplate<D>::Remove(void* pvKey)
{
	return CMapTemplate<void*, D>::Remove(&pvKey);
}


#endif // __MAP_PTR_PRIMITIVE_TEMPLATE_H__

