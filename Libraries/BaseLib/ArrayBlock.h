#ifndef __ARRAY_BASE_H__
#define __ARRAY_BASE_H__
#include "PointerRemapper.h"
#include "Mallocator.h"
#include "FileIO.h"
#include "Malloc.h"
#include "ArrayElementNotFound.h"
#include "DataCompare.h"
#include "StringHelper.h"
#include "Memory32.h"


//Arrays of pointers write this, the size of a pointer in a 32 bit build, as their element size so the file is the same
//in 32 and 64 bit builds.
#define ARRAY_FILE_POINTER_SIZE	4


//What an array counted in size (natural) and one counted in lsize (large) differ in, given to the array templates as
//their SIZE.  A natural array on the W65C816 holds at most MAX_SIZE bytes, so it moves them with memcpy and the
//others (size is size_t there); a large one with the 32 suffixed ones.  Where size is 16 bits (SIZE_16) a natural array that would need more than MAX_SIZE bytes fails as if out of
//memory (CheckSize).
struct SArrayNaturalSize
{
	typedef size		Count;

	static const size	Max = MAX_SIZE;
	static const size	NotFound = ARRAY_ELEMENT_NOT_FOUND;
#if defined(SIZE_16)
	static const bool	CheckSize = true;
#else
	static const bool	CheckSize = false;
#endif // SIZE_16

	static void*	Copy(void* pvDest, const void* pvSource, size uiBytes)	{ return memcpy(pvDest, pvSource, uiBytes); }
	static void*	Move(void* pvDest, const void* pvSource, size uiBytes)	{ return memmove(pvDest, pvSource, uiBytes); }
	static void*	Set(void* pvDest, int iValue, size uiBytes)				{ return memset(pvDest, iValue, uiBytes); }
	static int		Compare(const void* pv1, const void* pv2, size uiBytes)	{ return memcmp(pv1, pv2, uiBytes); }
	static void		Swap(const void* pv1, const void* pv2, size uiBytes)	{ MemSwp(pv1, pv2, uiBytes); }
};


struct SArrayLargeSize
{
	typedef lsize		Count;

	static const lsize	Max = MAX_LSIZE;
	static const lsize	NotFound = ARRAY_LARGE_ELEMENT_NOT_FOUND;
	static const bool	CheckSize = false;

	//A large array can pass 64KB: the 32 suffixed functions (Memory32.h).
	static void*	Copy(void* pvDest, const void* pvSource, lsize uiBytes)	{ return memcpy32(pvDest, pvSource, uiBytes); }
	static void*	Move(void* pvDest, const void* pvSource, lsize uiBytes)	{ return memmove32(pvDest, pvSource, uiBytes); }
	static void*	Set(void* pvDest, int iValue, lsize uiBytes)			{ return memset32(pvDest, iValue, uiBytes); }
	static int		Compare(const void* pv1, const void* pv2, lsize uiBytes)	{ return memcmp32(pv1, pv2, uiBytes); }
	static void		Swap(const void* pv1, const void* pv2, lsize uiBytes)	{ MemSwp32(pv1, pv2, uiBytes); }
};


//A natural array's header is SArrayTemplateHeader, a large array's SArrayLargeTemplateHeader.
template<class SIZE>
struct SArrayHeaderBase
{
	typedef typename SIZE::Count	Count;

	Count	miElementSize;
	Count	miUsedElements;
	Count	miChunkSize;

	bool	Write(CFileWriter* pcFileWriter);
	bool	Read(CFileReader* pcFileReader);
};

//Elements in an array are always of the same size.  Array elements are indexed from the start of the array.
//A pointer to an array element may become invalid if another element has been since added to or removed from the array.
//Elements in an array should be looked up by index unless it is known that the array is (temporarily) immutable.

//For constant element pointers take a look at CListBlock.

//For variable sized elements take a look at CListVariable.

//CArrayBlock (counted in size) and CArrayLargeBlock (counted in lsize) are this, written once for either size policy
//(SArrayNaturalSize or SArrayLargeSize).
class CRandom;
template<class SIZE>
class CArrayBlockBase : public CMalloc, protected SArrayHeaderBase<SIZE>
{
public:
	typedef SIZE					SizePolicy;		//The arrays deriving from it are written with these.
	typedef typename SIZE::Count	Count;

protected:
	using SArrayHeaderBase<SIZE>::miElementSize;
	using SArrayHeaderBase<SIZE>::miUsedElements;
	using SArrayHeaderBase<SIZE>::miChunkSize;

	Count	miNumElements;
	void*	mpvArray;

public:
	void	_Init(void);
	void	Init(Count iElementSize);
	void	Init(CMallocator* pcMalloc, Count iElementSize);
	void	Init(CMallocator* pcMalloc, Count iElementSize, Count iChunkSize);
	void	Init(CMallocator* pcMalloc, Count iElementSize, void* pvData, Count uiNumData);
	void	ReInit(void);

	void	Finalise(void);
	void	Fake(Count iElementSize, void* pvData, Count iNum, Count iChunkSize = 1);
	void	TakeFrom(CArrayBlockBase<SIZE>* pcSource);

	void	Kill(void);

	void	SetAllocateSize(Count iSize);
	void	FakeSetUsedElements(Count iUsedElements);

	Count	NumElements(void) const;
	bool	IsEmpty(void) const;
	bool	IsNotEmpty(void) const;
	Count	AllocatedElements(void) const;
	Count	ElementSize(void) const;

	void*	Add(void);
	void*	Add(void* pvData);
	void*	AddGetIndex(Count* piIndex);
	Count	AddExistingGetIndex(void* pvData);
	Count	AddIfUnique(void* pvData);
	Count	AddIfUniqueKey(void* pvData, Count iKeyOffset, Count iKeySize);

	void	Copy(CArrayBlockBase<SIZE>* pcOtherArray);

	void*	Get(Count iIndex);
	void*	SafeGet(Count iIndex);
	Count	GetAdjustedIndex(Count iIndex);
	void*	GetData(void) const;
	void	GetHeader(SArrayHeaderBase<SIZE>* psHeader);
	Count	GetIndex(void* pvData);
	void*	Tail(void);

	void*	InsertArrayAfterEnd(CArrayBlockBase<SIZE>* pcOtherArray);
	void*	InsertArrayAt(CArrayBlockBase<SIZE>* pcOtherArray, Count iIndex);
	void*	InsertArrayBeforeStart(CArrayBlockBase<SIZE>* pcOtherArray);
	void*	InsertAt(Count iIndex);
	void*	InsertAt(void* pvData, Count iIndex);
	void*	InsertBlockAfterEnd(void* paElements, Count iLength);
	void*	InsertBlockAt(void* paElements, Count iIndex, Count iLength);
	void*	InsertBlockBeforeStart(void* paElements, Count iLength);
	Count	InsertIntoSorted(DataCompare fCompare, void* pvData, bool bOverwriteExisting);
	void*	InsertNumAt(Count iNumElements, Count iIndex);
	void	InsertBatch(Count iFirstIndex, Count iNumInBatch, Count iNumBatches, Count iStrideToNextBatch);

	bool	Pop(void* pvDest);
	bool	Pop(void);
	void	Push(void* pvData);
	void*	Push(void);
	void*	PushCopy(void);
	bool	PopFirst(void* pvData);
	bool	PopFirst(void);

	Count	AddNum(Count iNumElements);
	Count	RemoveNum(Count iNumElements);
	void*	GrowToAtLeastNumElements(Count iNumElements, bool bClear = false, uint8  iClear = 0);
	Count	Resize(Count iNumElements);

	void	Sort(DataCompare fCompare);
	void	BubbleSort(DataCompare fCompare);
	void	QuickSort(DataCompare fCompare);
	void	TimSort(DataCompare fCompare);
	bool	IsSorted(DataCompare fCompare);

	void	Reverse(void);
	void	Shuffle(CRandom* pcRandom = NULL);

	bool	Contains(void* pvData);
	bool	Equals(CArrayBlockBase<SIZE>* pcOtherArray);
	Count	Find(void* pvData);
	bool	FindInSorted(void* pvData, DataCompare fCompare, Count* piIndex);
	Count	FindWithIntKey(int iKey);
	Count	FindWithIntKey(int iKey, Count iKeyOffset);
	Count	FindWithSizeKey(size iKey, Count iKeyOffset);
	Count	FindWithKey(void* pvData, Count iKeyOffset, Count iKeySize);

	void	RemoveAt(Count iIndex, bool bPreserveOrder = true);
	void	RemoveAt(Count* paiIndex, Count iNumElements, bool bPreserveOrder = true);
	void	RemoveRange(Count iStartIndex, Count iEndIndexExclusive, bool bPreserveOrder = true);
	bool	RemoveTail(void);
	bool	RemoveFirst(void);
	void	RemoveBatch(Count iFirstIndex, Count iNumInBatch, Count iNumBatches, Count iStrideToNextBatch);
	void	RemoveAll(void);
	bool	Remove(void* pvData, bool bPreserveOrder = true);
	Count	RemoveArray(CArrayBlockBase<SIZE>* pcOtherArray, bool bPreserveOrder = true);
	Count	RemoveArrayFromSorted(CArrayBlockBase<SIZE>* pcOtherArray, DataCompare fCompare);


	void	Set(Count iIndex, void* pvData);
	bool	SafeSet(Count iIndex, void* pvData);
	void	Swap(Count iIndex1, Count iIndex2);
	void	Zero(void);
	void	One(void);

	Count	ByteSize(void);
	Count	ChunkSize(void);
	Count	SetUsedElements(Count iNumElements);

	bool	SetChunkSize(Count iChunkSize);

	bool	Write(CFileWriter* pcFileWriter);
	bool	Read(CFileReader* pcFileReader);

	bool	WriteAllocatorAndHeader(CFileWriter* pcFileWriter);
	bool	WriteHeader(CFileWriter* pcFileWriter);
	bool	WriteHeader(CFileWriter* pcFileWriter, Count uiFileElementSize);
	bool	ReadAllocatorAndHeader(CFileReader* pcFileReader);
	bool	ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc);
	bool	ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc, Count uiFileElementSize, Count uiElementSize);

protected:
	bool	BinarySearch(void* pvData, Count iLeft, Count iRight, DataCompare fCompare, Count* piIndex);
	void*	CopyArrayInto(CArrayBlockBase<SIZE>* pcOtherArray, Count iIndex);
	void*	CopyBlockInto(void* paElements, Count iLength, Count iIndex);
	void	PrivateRemoveAt(Count iIndex, bool bPreserveOrder, Count iDataSize);
	void	PrivateRemoveRange(Count iStartIndex, Count iEndIndexExclusive, bool bPreserveOrder, Count iDataSize);
	Count	RemoveAtNoDeallocate(Count iIndex, bool bPreserveOrder, Count iDataSize);
	bool	SetArraySize(Count iNumElements);
	void	Overflowed(void);
};


//ArrayBlock.cpp instantiates them; no other unit compiles their functions.
extern template struct SArrayHeaderBase<SArrayNaturalSize>;
extern template struct SArrayHeaderBase<SArrayLargeSize>;
extern template class CArrayBlockBase<SArrayNaturalSize>;
extern template class CArrayBlockBase<SArrayLargeSize>;


//The names the arrays are known by.  They are classes, not typedefs, so they can be declared ahead.
struct SArrayTemplateHeader : public SArrayHeaderBase<SArrayNaturalSize> {};
struct SArrayLargeTemplateHeader : public SArrayHeaderBase<SArrayLargeSize> {};
class CArrayBlock : public CArrayBlockBase<SArrayNaturalSize> {};
class CArrayLargeBlock : public CArrayBlockBase<SArrayLargeSize> {};


#endif // __ARRAY_BASE_H__

