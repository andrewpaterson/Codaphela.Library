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
//No include guard: ArrayBlockClass.inl is included once per array size (ArraySizeNatural.inl and
//ArraySizeLarge.inl define the names and types it is written with).

struct ARRAY_HEADER
{
	ASIZE	miElementSize;
	ASIZE	miUsedElements;
	ASIZE	miChunkSize;

	bool	Write(CFileWriter* pcFileWriter);
	bool	Read(CFileReader* pcFileReader);
};

//Elements in an array are always of the same size.  Array elements are indexed from the start of the array.
//A pointer to an array element may become invalid if another element has been since added to or removed from the array.
//Elements in an array should be looked up by index unless it is known that the array is (temporarily) immutable.

//For constant element pointers take a look at CListBlock.

//For variable sized elements take a look at CListVariable.
class CRandom; 
class ARRAY_BLOCK : public CMalloc, protected ARRAY_HEADER
{
protected:
	ASIZE	miNumElements;
	void*	mpvArray;

public:
	void 	_Init(void);
	void 	Init(ASIZE iElementSize);
	void 	Init(CMallocator* pcMalloc, ASIZE iElementSize);
	void	Init(CMallocator* pcMalloc, ASIZE iElementSize, ASIZE iChunkSize);
	void	Init(CMallocator* pcMalloc, ASIZE iElementSize, void* pvData, ASIZE uiNumData);
	void 	ReInit(void);

	void 	Finalise(void);
	void	Fake(ASIZE iElementSize, void* pvData, ASIZE iNum, ASIZE iChunkSize = 1);

	void 	Kill(void);

	void 	SetAllocateSize(ASIZE iSize);
	void	FakeSetUsedElements(ASIZE iUsedElements);

	ASIZE	NumElements(void);
	bool	IsEmpty(void);
	bool	IsNotEmpty(void);
	ASIZE	AllocatedElements(void);
	ASIZE 	ElementSize(void);

	void*	Add(void);
	void*	Add(void* pvData);
	void* 	AddGetIndex(ASIZE* piIndex);
	ASIZE	AddExistingGetIndex(void* pvData);
	ASIZE 	AddIfUnique(void* pvData);
	ASIZE 	AddIfUniqueKey(void* pvData, ASIZE iKeyOffset, ASIZE iKeySize);

	void 	Copy(ARRAY_BLOCK* pcOtherArray);

	void*	Get(ASIZE iIndex);
	void*	SafeGet(ASIZE iIndex);
	ASIZE	GetAdjustedIndex(ASIZE iIndex);
	void*	GetData(void);
	void	GetHeader(ARRAY_HEADER* psHeader);
	ASIZE	GetIndex(void* pvData);
	void*	Tail(void);

	void*	InsertArrayAfterEnd(ARRAY_BLOCK* pcOtherArray);
	void*	InsertArrayAt(ARRAY_BLOCK* pcOtherArray, ASIZE iIndex);
	void*	InsertArrayBeforeStart(ARRAY_BLOCK* pcOtherArray);
	void* 	InsertAt(ASIZE iIndex);
	void* 	InsertAt(void* pvData, ASIZE iIndex);
	void*	InsertBlockAfterEnd(void* paElements, ASIZE iLength);
	void*	InsertBlockAt(void* paElements, ASIZE iIndex, ASIZE iLength);
	void*	InsertBlockBeforeStart(void* paElements, ASIZE iLength);
	ASIZE	InsertIntoSorted(DataCompare fCompare, void* pvData, bool bOverwriteExisting);
	void*	InsertNumAt(ASIZE iNumElements, ASIZE iIndex);
	void	InsertBatch(ASIZE iFirstIndex, ASIZE iNumInBatch, ASIZE iNumBatches, ASIZE iStrideToNextBatch);

	bool	Pop(void* pvDest);
	bool	Pop(void);
	void 	Push(void* pvData);
	void*	Push(void);
	void* 	PushCopy(void);
	bool	PopFirst(void* pvData);
	bool	PopFirst(void);

	ASIZE	AddNum(ASIZE iNumElements);
	ASIZE	RemoveNum(ASIZE iNumElements);
	void*	GrowToAtLeastNumElements(ASIZE iNumElements, bool bClear = false, uint8  iClear = 0);
	ASIZE	Resize(ASIZE iNumElements);

	void	Sort(DataCompare fCompare);
	void	BubbleSort(DataCompare fCompare);
	void	QuickSort(DataCompare fCompare);
	void	TimSort(DataCompare fCompare);
	bool	IsSorted(DataCompare fCompare);

	void	Reverse(void);
	void	Shuffle(CRandom* pcRandom = NULL);

	bool	Contains(void* pvData);
	bool	Equals(ARRAY_BLOCK* pcOtherArray);
	ASIZE 	Find(void* pvData);
	bool	FindInSorted(void* pvData, DataCompare fCompare, ASIZE* piIndex);
	ASIZE	FindWithIntKey(int iKey);
	ASIZE	FindWithIntKey(int iKey, ASIZE iKeyOffset);
	ASIZE	FindWithSizeKey(size iKey, ASIZE iKeyOffset);
	ASIZE 	FindWithKey(void* pvData, ASIZE iKeyOffset, ASIZE iKeySize);

	void 	RemoveAt(ASIZE iIndex, bool bPreserveOrder = true);
	void	RemoveAt(ASIZE* paiIndex, ASIZE iNumElements, bool bPreserveOrder = true);
	void	RemoveRange(ASIZE iStartIndex, ASIZE iEndIndexExclusive, bool bPreserveOrder = true);
	bool 	RemoveTail(void);
	bool	RemoveFirst(void);
	void	RemoveBatch(ASIZE iFirstIndex, ASIZE iNumInBatch, ASIZE iNumBatches, ASIZE iStrideToNextBatch);
	void	RemoveAll(void);
	bool	Remove(void* pvData, bool bPreserveOrder = true);
	ASIZE	RemoveArray(ARRAY_BLOCK* pcOtherArray, bool bPreserveOrder = true);
	ASIZE	RemoveArrayFromSorted(ARRAY_BLOCK* pcOtherArray, DataCompare fCompare);
	

	void	Set(ASIZE iIndex, void* pvData);
	bool	SafeSet(ASIZE iIndex, void* pvData);
	void	Swap(ASIZE iIndex1, ASIZE iIndex2);
	void 	Zero(void);
	void 	One(void);

	ASIZE 	ByteSize(void);
	ASIZE	ChunkSize(void);
	ASIZE	SetUsedElements(ASIZE iNumElements);

	ASIZE	RemoveAtNoDeallocate(ASIZE iIndex);
	bool	SetChunkSize(ASIZE iChunkSize);

	bool	Write(CFileWriter* pcFileWriter);
	bool	Read(CFileReader* pcFileReader);

	bool	WriteAllocatorAndHeader(CFileWriter* pcFileWriter);
	bool	WriteHeader(CFileWriter* pcFileWriter);
	bool	WriteHeader(CFileWriter* pcFileWriter, ASIZE uiFileElementSize);
	bool	ReadAllocatorAndHeader(CFileReader* pcFileReader);
	bool	ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc);
	bool	ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc, ASIZE uiFileElementSize, ASIZE uiElementSize);

protected:
	bool	BinarySearch(void* pvData, ASIZE iLeft, ASIZE iRight, DataCompare fCompare, ASIZE* piIndex);
	void*	CopyArrayInto(ARRAY_BLOCK* pcOtherArray, ASIZE iIndex);
	void*	CopyBlockInto(void* paElements, ASIZE iLength, ASIZE iIndex);
	void	PrivateRemoveAt(ASIZE iIndex, bool bPreserveOrder, ASIZE iDataSize);
	void	PrivateRemoveRange(ASIZE iStartIndex, ASIZE iEndIndexExclusive, bool bPreserveOrder, ASIZE iDataSize);
	ASIZE	RemoveAtNoDeallocate(ASIZE iIndex, bool bPreserveOrder, ASIZE iDataSize);
	void 	SetArraySize(ASIZE iNumElements);
#ifdef ARRAY_CHECK_SIZE
	void	Overflowed(void);
#endif // ARRAY_CHECK_SIZE
};
