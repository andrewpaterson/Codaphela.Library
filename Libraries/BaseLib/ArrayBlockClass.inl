/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __ARRAY_BLOCK_CLASS_H__
#define __ARRAY_BLOCK_CLASS_H__


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


#endif // __ARRAY_BLOCK_CLASS_H__

