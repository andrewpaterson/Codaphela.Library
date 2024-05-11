#ifndef __ARRAY_BASE_H__
#define __ARRAY_BASE_H__
#include "FastMemset.h"
#include "PointerRemapper.h"
#include "FastFunctions.h"
#include "Mallocator.h"
#include "FileIO.h"
#include "Malloc.h"
#include "DataTypes.h"
#include "ArrayElementNotFound.h"
#include "DataCompare.h"


struct SArrayTemplateHeader
{
	size		miElementSize;
	size		miUsedElements;
	size		miChunkSize;
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
	size	miNumElements;
	void*	mpvArray;

public:
	void 	_Init(void);
	void 	Init(size iElementSize);
	void 	Init(CMallocator* pcMalloc, size iElementSize);
	void	Init(CMallocator* pcMalloc, size iElementSize, size iChunkSize);
	void 	ReInit(void);

	void 	Finalise(void);
	void	Fake(size iElementSize, void* pvData, size iNum, size iChunkSize = 1);

	void 	Kill(void);

	void 	SetAllocateSize(size iSize);
	void	FakeSetUsedElements(size iUsedElements);

	size	NumElements(void);
	bool	IsEmpty(void);
	bool	IsNotEmpty(void);
	size	AllocatedElements(void);
	size 	ElementSize(void);

	void*	Add(void);
	void*	Add(void* pvData);
	void* 	AddGetIndex(size* piIndex);
	size	AddExistingGetIndex(void* pvData);
	size 	AddIfUnique(void* pvData);
	size 	AddIfUniqueKey(void* pvData, size iKeyOffset, size iKeySize);

	void 	Copy(CArrayBlock* pcTemplateArray);

	void*	Get(size iIndex);
	void*	SafeGet(size iIndex);
	size	GetAdjustedIndex(size iIndex);
	void*	GetData(void);
	void	GetHeader(SArrayTemplateHeader* psHeader);
	size	GetIndex(void* pvData);
	void*	Tail(void);

	void*	InsertArrayAfterEnd(CArrayBlock* pcTemplateArray);
	void*	InsertArrayAt(CArrayBlock* pcTemplateArray, size iIndex);
	void*	InsertArrayBeforeStart(CArrayBlock* pcTemplateArray);
	void* 	InsertAt(size iIndex);
	void* 	InsertAt(void* pvData, size iIndex);
	void*	InsertBlockAfterEnd(void* paElements, size iLength);
	void*	InsertBlockAt(void* paElements, size iIndex, size iLength);
	void*	InsertBlockBeforeStart(void* paElements, size iLength);
	size	InsertIntoSorted(DataCompare fCompare, void* pvData, bool bOverwriteExisting);
	void*	InsertNumAt(size iNumElements, size iIndex);
	void	InsertBatch(size iFirstIndex, size iNumInBatch, int32 iNumBatches, size iStrideToNextBatch);

	bool	Pop(void* pvDest);
	bool	Pop(void);
	void 	Push(void* pvData);
	void*	Push(void);
	void* 	PushCopy(void);
	bool	PopFirst(void* pvData);
	bool	PopFirst(void);

	size	AddNum(size iNumElements);
	size	RemoveNum(size iNumElements);
	void*	GrowToAtLeastNumElements(size iNumElements, bool bClear = false, uint8  iClear = 0);
	size	Resize(size iNumElements);

	void	BubbleSort(DataCompare fCompare);
	void	QuickSort(DataCompare fCompare);
	void	TimSort(DataCompare fCompare);

	void	Reverse(void);
	void	Shuffle(CRandom* pcRandom = NULL);

	bool	Contains(void* pvData);
	bool	Equals(CArrayBlock* pcTemplateArray);
	size 	Find(void* pvData);
	bool	FindInSorted(void* pvData, DataCompare fCompare, size* piIndex);
	size	FindWithIntKey(size iKey);
	size	FindWithIntKey(size iKey, size iKeyOffset);
	size 	FindWithKey(void* pvData, size iKeyOffset, size iKeySize);

	void 	RemoveAt(size iIndex, bool bPreserveOrder = true);
	void	RemoveAt(size* paiIndex, size iNumElements, bool bPreserveOrder = true);
	void	RemoveRange(size iStartIndex, size iEndIndexExclusive, bool bPreserveOrder = true);
	bool 	RemoveTail(void);
	bool	RemoveFirst(void);
	void	RemoveBatch(size iFirstIndex, size iNumInBatch, size iNumBatches, size iStrideToNextBatch);

	void	Set(size iIndex, void* pvData);
	bool	SafeSet(size iIndex, void* pvData);
	void	Swap(size iIndex1, size iIndex2);
	void 	Zero(void);

	size 	ByteSize(void);
	size	ChunkSize(void);
	size	SetUsedElements(size iNumElements);

	size	RemoveAtNoDeallocate(size iIndex);
	bool	SetChunkSize(size iChunkSize);

	bool	Write(CFileWriter* pcFileWriter);
	bool	Read(CFileReader* pcFileReader);

	bool	WriteAllocatorAndHeader(CFileWriter* pcFileWriter);
	bool	WriteHeader(CFileWriter* pcFileWriter);
	bool	ReadAllocatorAndHeader(CFileReader* pcFileReader);
	bool	ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc);

protected:
	bool	BinarySearch(void* pvData, size iLeft, size iRight, DataCompare fCompare, size* piIndex);
	void*	CopyArrayInto(CArrayBlock* pcTemplateArray, size iIndex);
	void*	CopyBlockInto(void* paElements, size iLength, size iIndex);
	void	PrivateRemoveAt(size iIndex, bool bPreserveOrder, size iDataSize);
	void	PrivateRemoveRange(size iStartIndex, size iEndIndexExclusive, bool bPreserveOrder, size iDataSize);
	size	RemoveAtNoDeallocate(size iIndex, bool bPreserveOrder, size iDataSize);
	void 	SetArraySize(size iNumElements);
};


#endif // __ARRAY_BASE_H__

