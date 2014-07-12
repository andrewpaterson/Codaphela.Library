#ifndef __ARRAY_BASE_H__
#define __ARRAY_BASE_H__
#include "FastMemset.h"
#include "PointerRemapper.h"
#include "FastFunctions.h"
#include "Mallocator.h"
#include "FileIO.h"


struct SArrayTemplateHeader
{
	int			miElementSize;
	int			miUsedElements;
	int			miChunkSize;
};


class CArrayBlock : protected SArrayTemplateHeader
{
protected:
	int				miNumElements;
	void*			mpvArray;
	CMallocator*	mpcMalloc;

public:
	void 	Init(int iElementSize, int iChunkSize);
	void 	InitFromHeader(void);
	void 	ReInit(int iChunkSize = 0);
	void 	Allocate(int iElementSize, int iNumElements);
	void 	Allocate(int iElementSize, int iChunkSize, int iNumElements);

	void 	Finalise(void);
	void	Fake(int iElementSize, void* pvData, int iNum, int iChunkSize = 1);

	void 	Kill(void);

	void 	SetAllocateSize(int iSize);
	void	FakeSetUsedElements(int iUsedElements);

	int		NumElements(void);
	BOOL	IsEmpty(void);
	BOOL	IsNotEmpty(void);
	int		AllocatedElements(void);
	int 	ElementSize(void);

	void*	Add(void);
	void*	Add(void* pvData);
	void* 	AddGetIndex(int* piIndex);
	int 	AddIfUnique(void* pData);
	int 	AddIfUniqueKey(void* pData, int iKeyOffset, int iKeySize);

	void	BatchInsertElements(int iFirstIndex, int iNumInBatch, int iNumBatches, int iStrideToNextBatch);
	void	BatchRemoveElements(int iFirstIndex, int iNumInBatch, int iNumBatches, int iStrideToNextBatch);

	BOOL 	Copy(CArrayBlock* pcTemplateArray);

	void*	Get(int iIndex);
	void*	SafeGet(int iIndex);
	int		GetAdjustedIndex(int iIndex);
	void*	GetData(void);
	void	GetHeader(SArrayTemplateHeader* psHeader);
	int		GetIndex(void* pvElement);
	void*	Tail(void);

	void	InsertArrayAfterEnd(CArrayBlock* pcTemplateArray);
	void	InsertArrayAt(CArrayBlock* pcTemplateArray, int iIndex);
	void	InsertArrayBeforeStart(CArrayBlock* pcTemplateArray);
	void* 	InsertAt(int iIndex);
	void* 	InsertAt(void* pvData, int iIndex);
	void	InsertBlockAfterEnd(void* paElements, int iLength);
	void	InsertBlockAt(void* paElements, int iIndex, int iLength);
	void	InsertBlockBeforeStart(void* paElements, int iLength);
	int		InsertIntoSorted(int(*)(const void*, const void*), void* pvElement, BOOL bOverwriteExisting);
	void*	InsertNumElementsAt(int iNumElements, int iIndex);

	void	Pop(void* pvData);
	void	Pop(void);
	void 	Push(void* pvElement);
	void*	Push(void);
	void 	PushCopy(void);

	void	GrowByChunk(void);
	int		GrowByNumElements(int iNumElements);
	void*	GrowToAtLeastNumElements(int iNumElements, BOOL bClear = FALSE, int iClear = 0);  //ie:  Don't shrink the array.
	int		GrowToNumElements(int iNumElements);  //Can shrink the array.  Should probably call this resize.

	void	BubbleSort(int(*)(const void*, const void*));
	void	QuickSort(int(*)(const void*, const void*));
	void	Reverse(void);

	BOOL	Contains(void* pData);
	BOOL	Equals(CArrayBlock* pcTemplateArray);
	int 	Find(void* pData);
	BOOL	FindInSorted(void* pData, int(*)(const void*, const void*), int* piIndex);
	int		FindWithIntKey(int iKey);
	int		FindWithIntKey(int iKey, int iKeyOffset);
	int 	FindWithKey(void* pData, int iKeyOffset, int iKeySize);

	void 	RemoveAt(int iIndex, int bPreserveOrder = TRUE);
	void	RemoveAt(int* paiIndex, int iNumElements, BOOL bPreserveOrder = TRUE);
	void	RemoveAtNoDeallocate(int iIndex, int bPreserveOrder);
	void	RemoveRange(int iStartIndex, int iEndIndexExclusive, BOOL bPreserveOrder = TRUE);
	void 	RemoveTail(void);

	void	Set(int iIndex, void* pvData);
	BOOL	SafeSet(int iIndex, void* pvData);
	void	Swap(int iIndex1, int iIndex2);
	void	Unuse(void);
	void 	Zero(void);

	int 	ByteSize(void);
	int		ChunkSize(void);
	void	SetUsedElements(int iNumElements);

	BOOL	WriteArrayTemplateHeader(CFileWriter* pcFileWriter);
	BOOL	WriteArrayTemplate(CFileWriter* pcFileWriter);
	BOOL	ReadArrayTemplateHeader(CFileReader* pcFileReader);
	BOOL	ReadArrayTemplate(CFileReader* pcFileReader);
	BOOL	WriteArrayUnknown(CFileWriter* pcFileWriter);
	BOOL	ReadArrayUnknown(CFileReader* pcFileReader);

protected:
	void*	Malloc(size_t tSize);
	void*	Realloc(void* pv, size_t iMemSize);
	void	Free(void* pv);

	void 	Init(SArrayTemplateHeader* psHeader);

	BOOL	BinarySearch(void* pData, int iLeft, int iRight, int(*)(const void*, const void*), int* piIndex);
	void	CopyArrayInto(CArrayBlock* pcTemplateArray, int iIndex);
	void	CopyBlockInto(void* paElements, int iLength, int iIndex);
	void	PrivateRemoveAt(int iIndex, BOOL bPreserveOrder, int iDataSize);
	void	PrivateRemoveRange(int iStartIndex, int iEndIndexExclusive, int bPreserveOrder, int iDataSize);
	void	RemoveAtNoDeallocate(int iIndex, BOOL bPreserveOrder, int iDataSize);
	void 	SetArraySize(int iNumElements);
};


#endif // __ARRAY_BASE_H__

