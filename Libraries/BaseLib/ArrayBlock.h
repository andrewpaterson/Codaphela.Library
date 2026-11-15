#ifndef __ARRAY_BASE_H__
#define __ARRAY_BASE_H__
#include "PointerRemapper.h"
#include "Mallocator.h"
#include "FileIO.h"
#include "Malloc.h"
#include "ArrayElementNotFound.h"
#include "DataCompare.h"
#include "StringHelper.h"


//Arrays of pointers write this, the size of a pointer in a 32 bit build, as their element size so the file is the same
//in 32 and 64 bit builds.
#define ARRAY_FILE_POINTER_SIZE	4


//What an array counted in size (natural) and one counted in lsize (large) differ in, given to the array templates as
//their SIZE.  A natural array on the W65C816 holds at most MAX_SIZE bytes, so it moves them with Libc65's 16 bit
//counts.  Where size is 16 bits (SIZE_16) a natural array that would need more than MAX_SIZE bytes fails as if out of
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

#if defined(__W65C816__)
	static void*	Copy(void* pvDest, const void* pvSource, size uiBytes)	{ return memcpy16(pvDest, pvSource, uiBytes); }
	static void*	Move(void* pvDest, const void* pvSource, size uiBytes)	{ return memmove16(pvDest, pvSource, uiBytes); }
	static void*	Set(void* pvDest, int iValue, size uiBytes)				{ return memset16(pvDest, iValue, uiBytes); }
	static int		Compare(const void* pv1, const void* pv2, size uiBytes)	{ return memcmp16(pv1, pv2, uiBytes); }
	static void		Swap(const void* pv1, const void* pv2, size uiBytes)	{ MemSwp16(pv1, pv2, uiBytes); }
#else
	static void*	Copy(void* pvDest, const void* pvSource, size uiBytes)	{ return memcpy(pvDest, pvSource, uiBytes); }
	static void*	Move(void* pvDest, const void* pvSource, size uiBytes)	{ return memmove(pvDest, pvSource, uiBytes); }
	static void*	Set(void* pvDest, int iValue, size uiBytes)				{ return memset(pvDest, iValue, uiBytes); }
	static int		Compare(const void* pv1, const void* pv2, size uiBytes)	{ return memcmp(pv1, pv2, uiBytes); }
	static void		Swap(const void* pv1, const void* pv2, size uiBytes)	{ MemSwp(pv1, pv2, uiBytes); }
#endif // __W65C816__
};


struct SArrayLargeSize
{
	typedef lsize		Count;

	static const lsize	Max = MAX_LSIZE;
	static const lsize	NotFound = ARRAY_LARGE_ELEMENT_NOT_FOUND;
	static const bool	CheckSize = false;

	static void*	Copy(void* pvDest, const void* pvSource, lsize uiBytes)	{ return memcpy(pvDest, pvSource, uiBytes); }
	static void*	Move(void* pvDest, const void* pvSource, lsize uiBytes)	{ return memmove(pvDest, pvSource, uiBytes); }
	static void*	Set(void* pvDest, int iValue, lsize uiBytes)			{ return memset(pvDest, iValue, uiBytes); }
	static int		Compare(const void* pv1, const void* pv2, lsize uiBytes)	{ return memcmp(pv1, pv2, uiBytes); }
	static void		Swap(const void* pv1, const void* pv2, lsize uiBytes)	{ MemSwp(pv1, pv2, uiBytes); }
};


//A natural array's header is SArrayTemplateHeader, a large array's SArrayLargeTemplateHeader.
template<class SIZE>
struct SArrayHeaderBase
{
	typename SIZE::Count	miElementSize;
	typename SIZE::Count	miUsedElements;
	typename SIZE::Count	miChunkSize;

	bool					Write(CFileWriter* pcFileWriter);
	bool					Read(CFileReader* pcFileReader);
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

	typename SIZE::Count	miNumElements;
	void*					mpvArray;

public:
	void					_Init(void);
	void					Init(typename SIZE::Count iElementSize);
	void					Init(CMallocator* pcMalloc, typename SIZE::Count iElementSize);
	void					Init(CMallocator* pcMalloc, typename SIZE::Count iElementSize, typename SIZE::Count iChunkSize);
	void					Init(CMallocator* pcMalloc, typename SIZE::Count iElementSize, void* pvData, typename SIZE::Count uiNumData);
	void					ReInit(void);

	void					Finalise(void);
	void					Fake(typename SIZE::Count iElementSize, void* pvData, typename SIZE::Count iNum, typename SIZE::Count iChunkSize = 1);
	void					TakeFrom(CArrayBlockBase<SIZE>* pcSource);

	void					Kill(void);

	void					SetAllocateSize(typename SIZE::Count iSize);
	void					FakeSetUsedElements(typename SIZE::Count iUsedElements);

	typename SIZE::Count	NumElements(void);
	bool					IsEmpty(void);
	bool					IsNotEmpty(void);
	typename SIZE::Count	AllocatedElements(void);
	typename SIZE::Count	ElementSize(void);

	void*					Add(void);
	void*					Add(void* pvData);
	void*					AddGetIndex(typename SIZE::Count* piIndex);
	typename SIZE::Count	AddExistingGetIndex(void* pvData);
	typename SIZE::Count	AddIfUnique(void* pvData);
	typename SIZE::Count	AddIfUniqueKey(void* pvData, typename SIZE::Count iKeyOffset, typename SIZE::Count iKeySize);

	void					Copy(CArrayBlockBase<SIZE>* pcOtherArray);

	void*					Get(typename SIZE::Count iIndex);
	void*					SafeGet(typename SIZE::Count iIndex);
	typename SIZE::Count	GetAdjustedIndex(typename SIZE::Count iIndex);
	void*					GetData(void);
	void					GetHeader(SArrayHeaderBase<SIZE>* psHeader);
	typename SIZE::Count	GetIndex(void* pvData);
	void*					Tail(void);

	void*					InsertArrayAfterEnd(CArrayBlockBase<SIZE>* pcOtherArray);
	void*					InsertArrayAt(CArrayBlockBase<SIZE>* pcOtherArray, typename SIZE::Count iIndex);
	void*					InsertArrayBeforeStart(CArrayBlockBase<SIZE>* pcOtherArray);
	void*					InsertAt(typename SIZE::Count iIndex);
	void*					InsertAt(void* pvData, typename SIZE::Count iIndex);
	void*					InsertBlockAfterEnd(void* paElements, typename SIZE::Count iLength);
	void*					InsertBlockAt(void* paElements, typename SIZE::Count iIndex, typename SIZE::Count iLength);
	void*					InsertBlockBeforeStart(void* paElements, typename SIZE::Count iLength);
	typename SIZE::Count	InsertIntoSorted(DataCompare fCompare, void* pvData, bool bOverwriteExisting);
	void*					InsertNumAt(typename SIZE::Count iNumElements, typename SIZE::Count iIndex);
	void					InsertBatch(typename SIZE::Count iFirstIndex, typename SIZE::Count iNumInBatch, typename SIZE::Count iNumBatches, typename SIZE::Count iStrideToNextBatch);

	bool					Pop(void* pvDest);
	bool					Pop(void);
	void					Push(void* pvData);
	void*					Push(void);
	void*					PushCopy(void);
	bool					PopFirst(void* pvData);
	bool					PopFirst(void);

	typename SIZE::Count	AddNum(typename SIZE::Count iNumElements);
	typename SIZE::Count	RemoveNum(typename SIZE::Count iNumElements);
	void*					GrowToAtLeastNumElements(typename SIZE::Count iNumElements, bool bClear = false, uint8  iClear = 0);
	typename SIZE::Count	Resize(typename SIZE::Count iNumElements);

	void					Sort(DataCompare fCompare);
	void					BubbleSort(DataCompare fCompare);
	void					QuickSort(DataCompare fCompare);
	void					TimSort(DataCompare fCompare);
	bool					IsSorted(DataCompare fCompare);

	void					Reverse(void);
	void					Shuffle(CRandom* pcRandom = NULL);

	bool					Contains(void* pvData);
	bool					Equals(CArrayBlockBase<SIZE>* pcOtherArray);
	typename SIZE::Count	Find(void* pvData);
	bool					FindInSorted(void* pvData, DataCompare fCompare, typename SIZE::Count* piIndex);
	typename SIZE::Count	FindWithIntKey(int iKey);
	typename SIZE::Count	FindWithIntKey(int iKey, typename SIZE::Count iKeyOffset);
	typename SIZE::Count	FindWithSizeKey(size iKey, typename SIZE::Count iKeyOffset);
	typename SIZE::Count	FindWithKey(void* pvData, typename SIZE::Count iKeyOffset, typename SIZE::Count iKeySize);

	void					RemoveAt(typename SIZE::Count iIndex, bool bPreserveOrder = true);
	void					RemoveAt(typename SIZE::Count* paiIndex, typename SIZE::Count iNumElements, bool bPreserveOrder = true);
	void					RemoveRange(typename SIZE::Count iStartIndex, typename SIZE::Count iEndIndexExclusive, bool bPreserveOrder = true);
	bool					RemoveTail(void);
	bool					RemoveFirst(void);
	void					RemoveBatch(typename SIZE::Count iFirstIndex, typename SIZE::Count iNumInBatch, typename SIZE::Count iNumBatches, typename SIZE::Count iStrideToNextBatch);
	void					RemoveAll(void);
	bool					Remove(void* pvData, bool bPreserveOrder = true);
	typename SIZE::Count	RemoveArray(CArrayBlockBase<SIZE>* pcOtherArray, bool bPreserveOrder = true);
	typename SIZE::Count	RemoveArrayFromSorted(CArrayBlockBase<SIZE>* pcOtherArray, DataCompare fCompare);


	void					Set(typename SIZE::Count iIndex, void* pvData);
	bool					SafeSet(typename SIZE::Count iIndex, void* pvData);
	void					Swap(typename SIZE::Count iIndex1, typename SIZE::Count iIndex2);
	void					Zero(void);
	void					One(void);

	typename SIZE::Count	ByteSize(void);
	typename SIZE::Count	ChunkSize(void);
	typename SIZE::Count	SetUsedElements(typename SIZE::Count iNumElements);

	typename SIZE::Count	RemoveAtNoDeallocate(typename SIZE::Count iIndex);
	bool					SetChunkSize(typename SIZE::Count iChunkSize);

	bool					Write(CFileWriter* pcFileWriter);
	bool					Read(CFileReader* pcFileReader);

	bool					WriteAllocatorAndHeader(CFileWriter* pcFileWriter);
	bool					WriteHeader(CFileWriter* pcFileWriter);
	bool					WriteHeader(CFileWriter* pcFileWriter, typename SIZE::Count uiFileElementSize);
	bool					ReadAllocatorAndHeader(CFileReader* pcFileReader);
	bool					ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc);
	bool					ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc, typename SIZE::Count uiFileElementSize, typename SIZE::Count uiElementSize);

protected:
	bool					BinarySearch(void* pvData, typename SIZE::Count iLeft, typename SIZE::Count iRight, DataCompare fCompare, typename SIZE::Count* piIndex);
	void*					CopyArrayInto(CArrayBlockBase<SIZE>* pcOtherArray, typename SIZE::Count iIndex);
	void*					CopyBlockInto(void* paElements, typename SIZE::Count iLength, typename SIZE::Count iIndex);
	void					PrivateRemoveAt(typename SIZE::Count iIndex, bool bPreserveOrder, typename SIZE::Count iDataSize);
	void					PrivateRemoveRange(typename SIZE::Count iStartIndex, typename SIZE::Count iEndIndexExclusive, bool bPreserveOrder, typename SIZE::Count iDataSize);
	typename SIZE::Count	RemoveAtNoDeallocate(typename SIZE::Count iIndex, bool bPreserveOrder, typename SIZE::Count iDataSize);
	void					SetArraySize(typename SIZE::Count iNumElements);
	void					Overflowed(void);
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

