#ifndef __MEMORY_FREE_LIST_PARAMS__
#define __MEMORY_FREE_LIST_PARAMS__
#include "ArrayTemplate.h"


#define MEMORY_FREE_LIST_SIZE_LIMIT	2048


struct SMemoryFreeListParams
{
	unsigned int	iMaxListSize;  //This is the size of the actual element in the free list including SGeneralMemoryAllocation

								   //These exist for some binary search Compare function stuffs.
	unsigned int	iMaxElementSize;
	unsigned int	iMinElementSize;

	int				iChunkSize;

	SMemoryFreeListParams*	Init(unsigned int iFreeListSize, int iPrevSize, int iChunkSize, int iHeaderSize);
};


typedef CArrayTemplate<SMemoryFreeListParams>		CArrayFreeListParams;
int CompareFreeListParam(const void* arg1, const void* arg2);



class CMemoryFreeListParams
{
private:
	CArrayFreeListParams		mcParams;
	int							miHeaderSize;
	unsigned int				muiFreeListSizeLimit;

public:
	void Init(int iHeaderSize, bool bDefaultFreeListParams = true);
	void Kill(void);

	void					AddParamBlock(unsigned int iFreeListSize, int iPrevSize, int iChunkSize);
	void					AddParamBlock(SMemoryFreeListParams* psParam);

	SMemoryFreeListParams*	GetFreeListParams(int iIndex);
	bool					Read(CFileReader* pcFileReader);
	bool					Write(CFileWriter* pcFileWriter);
	void					SetFreeListSizeLimit(unsigned int uiFreeListSizeLimit);
	SMemoryFreeListParams*	GetFreeListParamsForSize(size_t iElementSize);
	unsigned int			GetFreeListSizeLimit(void);
	unsigned int			GetMaxFreeListElementSize(void);
	int						NumParams(void);

private:
	void					InitFreeListParams(void);
};


#endif // !__MEMORY_FREE_LIST_PARAMS__

