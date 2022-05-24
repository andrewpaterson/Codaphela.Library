#ifndef __ARRAY_BASE_H__
#define __ARRAY_BASE_H__
#include "FastMemset.h"
#include "PointerRemapper.h"
#include "FastFunctions.h"
#include "Mallocator.h"
#include "FileIO.h"
#include "Malloc.h"
#include "DataCallback.h"


struct SArrayTemplateHeader
{
	int		miElementSize;
	int		miUsedElements;
	int		miChunkSize;
};


//Elements in an array are always of the same size.  Array elements are indexed from the start of the array.
//A pointer to an array element may become invalid if another element has been since added to or removed from the array.
//Elements in an array should be looked up by index unless it is known that the array is (temporarily) immutable.

//For constant element pointers take a look at CListBlock.

//For variable sized elements take a look at CListVariable.
class CRandom; 
class CArrayBlock : public CMalloc, protected SArrayTemplateHeader
{
protected:
	int		miNumElements;
	void*	mpvArray;

public:
	void 	_Init(void);
	void 	Init(int iElementSize);
	void 	Init(CMallocator* pcMalloc, int iElementSize);
	void	Init(CMallocator* pcMalloc, int iElementSize, int iChunkSize);
	void 	ReInit(void);

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
	int		AddGetIndex(void* pvData);
	int 	AddIfUnique(void* pData);
	int 	AddIfUniqueKey(void* pData, int iKeyOffset, int iKeySize);

	void 	Copy(CArrayBlock* pcTemplateArray);

	void*	Get(int iIndex);
	void*	SafeGet(int iIndex);
	int		GetAdjustedIndex(int iIndex);
	void*	GetData(void);
	void	GetHeader(SArrayTemplateHeader* psHeader);
	int		GetIndex(void* pvData);
	void*	Tail(void);

	void*	InsertArrayAfterEnd(CArrayBlock* pcTemplateArray);
	void*	InsertArrayAt(CArrayBlock* pcTemplateArray, int iIndex);
	void*	InsertArrayBeforeStart(CArrayBlock* pcTemplateArray);
	void* 	InsertAt(int iIndex);
	void* 	InsertAt(void* pvData, int iIndex);
	void*	InsertBlockAfterEnd(void* paElements, int iLength);
	void*	InsertBlockAt(void* paElements, int iIndex, int iLength);
	void*	InsertBlockBeforeStart(void* paElements, int iLength);
	int		InsertIntoSorted(DataCompare fCompare, void* pvData, BOOL bOverwriteExisting);
	void*	InsertNumAt(int iNumElements, int iIndex);
	void	InsertBatch(int iFirstIndex, int iNumInBatch, int iNumBatches, int iStrideToNextBatch);

	BOOL	Pop(void* pvDest);
	BOOL	Pop(void);
	void 	Push(void* pvData);
	void*	Push(void);
	void* 	PushCopy(void);
	BOOL	PopFirst(void* pvData);
	BOOL	PopFirst(void);

	int		AddNum(int iNumElements);
	void*	GrowToAtLeastNumElements(int iNumElements, BOOL bClear = FALSE, unsigned char  iClear = 0);
	int		Resize(int iNumElements);

	void	BubbleSort(DataCompare fCompare);
	void	QuickSort(DataCompare fCompare);
	void	TimSort(DataCompare fCompare);

	void	Reverse(void);
	void	Shuffle(CRandom* pcRandom = NULL);

	BOOL	Contains(void* pData);
	BOOL	Equals(CArrayBlock* pcTemplateArray);
	int 	Find(void* pData);
	BOOL	FindInSorted(void* pvData, DataCompare fCompare, int* piIndex);
	int		FindWithIntKey(int iKey);
	int		FindWithIntKey(int iKey, int iKeyOffset);
	int 	FindWithKey(void* pData, int iKeyOffset, int iKeySize);

	void 	RemoveAt(int iIndex, int bPreserveOrder = TRUE);
	void	RemoveAt(int* paiIndex, int iNumElements, BOOL bPreserveOrder = TRUE);
	void	RemoveRange(int iStartIndex, int iEndIndexExclusive, BOOL bPreserveOrder = TRUE);
	BOOL 	RemoveTail(void);
	BOOL	RemoveFirst(void);
	void	RemoveBatch(int iFirstIndex, int iNumInBatch, int iNumBatches, int iStrideToNextBatch);

	void	Set(int iIndex, void* pvData);
	BOOL	SafeSet(int iIndex, void* pvData);
	void	Swap(int iIndex1, int iIndex2);
	void	Unuse(void);  //Da fuq?
	void 	Zero(void);

	int 	ByteSize(void);
	int		ChunkSize(void);
	int		SetUsedElements(int iNumElements);

	int		RemoveAtNoDeallocate(int iIndex);
	BOOL	SetChunkSize(int iChunkSize);

	BOOL	Write(CFileWriter* pcFileWriter);
	BOOL	Read(CFileReader* pcFileReader);

	BOOL	WriteAllocatorAndHeader(CFileWriter* pcFileWriter);
	BOOL	WriteHeader(CFileWriter* pcFileWriter);
	BOOL	ReadAllocatorAndHeader(CFileReader* pcFileReader);
	BOOL	ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc);

protected:
	BOOL	BinarySearch(void* pData, int iLeft, int iRight, DataCompare fCompare, int* piIndex);
	void*	CopyArrayInto(CArrayBlock* pcTemplateArray, int iIndex);
	void*	CopyBlockInto(void* paElements, int iLength, int iIndex);
	void	PrivateRemoveAt(int iIndex, BOOL bPreserveOrder, int iDataSize);
	void	PrivateRemoveRange(int iStartIndex, int iEndIndexExclusive, int bPreserveOrder, int iDataSize);
	int		RemoveAtNoDeallocate(int iIndex, BOOL bPreserveOrder, int iDataSize);
	void 	SetArraySize(int iNumElements);
};


#endif // !__ARRAY_BASE_H__

