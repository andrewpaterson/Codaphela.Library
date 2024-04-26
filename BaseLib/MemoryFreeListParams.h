#ifndef __MEMORY_FREE_LIST_PARAMS__
#define __MEMORY_FREE_LIST_PARAMS__
#include "ArrayTemplate.h"


#define MEMORY_FREE_LIST_SIZE_LIMIT	2048


struct SMemoryFreeListParams
{
	uint32	iMaxListSize;  //This is the size of the actual element in the free list including SGeneralMemoryAllocation

								   //These exist for some binary search Compare function stuffs.
	uint32	iMaxElementSize;
	uint32	iMinElementSize;

	int				iChunkSize;

	SMemoryFreeListParams*	Init(uint32 iFreeListSize, int iPrevSize, int iChunkSize, int iHeaderSize);
};


typedef CArrayTemplate<SMemoryFreeListParams>		CArrayFreeListParams;
int CompareFreeListParam(const void* arg1, const void* arg2);



class CMemoryFreeListParams
{
private:
	CArrayFreeListParams		mcParams;
	int							miHeaderSize;
	uint32				muiFreeListSizeLimit;

public:
	void Init(int iHeaderSize, bool bDefaultFreeListParams = true);
	void Kill(void);

	void					AddParamBlock(uint32 iFreeListSize, int iPrevSize, int iChunkSize);
	void					AddParamBlock(SMemoryFreeListParams* psParam);

	SMemoryFreeListParams*	GetFreeListParams(int iIndex);
	bool					Read(CFileReader* pcFileReader);
	bool					Write(CFileWriter* pcFileWriter);
	void					SetFreeListSizeLimit(uint32 uiFreeListSizeLimit);
	SMemoryFreeListParams*	GetFreeListParamsForSize(size_t iElementSize);
	uint32			GetFreeListSizeLimit(void);
	uint32			GetMaxFreeListElementSize(void);
	int						NumParams(void);

private:
	void					InitFreeListParams(void);
};


#endif // __MEMORY_FREE_LIST_PARAMS__

