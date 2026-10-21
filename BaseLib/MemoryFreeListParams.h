#ifndef __MEMORY_FREE_LIST_PARAMS__
#define __MEMORY_FREE_LIST_PARAMS__
#include "ArrayTemplate.h"


#define MEMORY_FREE_LIST_SIZE_LIMIT	2048


struct SMemoryFreeListParams
{
	size	iMaxListSize;	//This is the size of the actual element in the free list including SGeneralMemoryAllocation

	//These exist for some binary search Compare function stuffs.
	size	iMaxElementSize;
	size	iMinElementSize;

	size	iChunkSize;

	SMemoryFreeListParams*	Init(size iFreeListSize, size iPrevSize, size iChunkSize, size iHeaderSize);
	bool					Write(CFileWriter* pcFileWriter);
	bool					Read(CFileReader* pcFileReader);
};


typedef CArrayTemplate<SMemoryFreeListParams>		CArrayFreeListParams;
typedef CArrayTemplate<uint16>						CArrayFreeListParamIndices;
int CompareFreeListParam(const void* arg1, const void* arg2);


class CMemoryFreeListParams
{
private:
	CArrayFreeListParams	mcParams;
	size					miHeaderSize;
	size					muiFreeListSizeLimit;

	//The index of the params for every element size up to GetMaxFreeListElementSize.  Built when first needed.
	CArrayFreeListParamIndices	mcSizeIndices;
	bool						mbSizeIndicesValid;

public:
	void Init(size iHeaderSize, bool bDefaultFreeListParams = true);
	void Kill(void);

	void					AddParamBlock(size iFreeListSize, size iPrevSize, size iChunkSize);
	void					AddParamBlock(SMemoryFreeListParams* psParam);

	SMemoryFreeListParams*	GetFreeListParams(size iIndex);
	bool					Read(CFileReader* pcFileReader);
	bool					Write(CFileWriter* pcFileWriter);
	void					SetFreeListSizeLimit(size uiFreeListSizeLimit);
	SMemoryFreeListParams*	GetFreeListParamsForSize(size iElementSize);
	size					GetFreeListParamsIndexForSize(size iElementSize);
	size					GetFreeListSizeLimit(void);
	size					GetMaxFreeListElementSize(void);
	size					NumParams(void);

private:
	void					InitFreeListParams(void);
	size					FindFreeListParamsIndex(size uiElementSize);
	void					BuildSizeIndices(void);
};


#endif // __MEMORY_FREE_LIST_PARAMS__

