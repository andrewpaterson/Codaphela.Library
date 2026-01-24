#ifndef _SAFE_ARRAY_BLOCK_H__
#define _SAFE_ARRAY_BLOCK_H__
#include <mutex>
#include "BaseLib/ArrayBlock.h"
#include "BaseLib/StackMemory.h"


class CSafeArrayBlock
{
private:
	std::mutex	m;
	CArrayBlock	c;

public:
	void 	Init(size iElementSize);
	void 	Init(CMallocator* pcMalloc, size iElementSize);
	void	Init(CMallocator* pcMalloc, size iElementSize, size iChunkSize);
	void 	ReInit(void);

	void 	Finalise(void);
	void 	Kill(void);

	size	NumElements(void);
	bool	IsEmpty(void);
	bool	IsNotEmpty(void);
	size	AllocatedElements(void);
	size 	ElementSize(void);

	void	Add(void* pvData);
	size	AddGetIndex(void* pvData);
	size 	AddIfUnique(void* pData);
	size 	AddIfUniqueKey(void* pData, size iKeyOffset, size iKeySize);

	void 	Copy(CArrayBlock* pcTemplateArray);
	void 	Copy(CSafeArrayBlock* pcTemplateArray);
	size	Copy(CStackMemory<>* pcTemp);

	bool	Get(size iIndex, void* pvDest);
	bool	Tail(void* pvDest);

	void	InsertAt(void* pvData, size iIndex);
	size	InsertIntoSorted(int(*fCompare)(const void*, const void*), void* pvData, bool bOverwriteExisting);

	bool	Pop(void* pvDest);
	bool	Pop(void);
	void 	Push(void* pvData);
	bool	PopFirst(void* pvData);
	bool	PopFirst(void);

	size	Resize(size iNumElements);

	void	BubbleSort(int(*fCompare)(const void*, const void*));
	void	QuickSort(int(*fCompare)(const void*, const void*));
	void	Reverse(void);

	bool	Contains(void* pData);
	size 	Find(void* pData);
	bool	FindInSorted(void* pData, int(*fCompare)(const void*, const void*), size* piIndex);

	void 	RemoveAt(size iIndex, bool bPreserveOrder = true);
	void	RemoveRange(size iStartIndex, size iEndIndexExclusive, bool bPreserveOrder = true);
	bool 	RemoveTail(void);

	void	Set(size iIndex, void* pvData);
	void	Swap(size iIndex1, size iIndex2);
	void 	Zero(void);

	size 	ByteSize(void);
	size	ChunkSize(void);
	size	SetUsedElements(size iNumElements);

	bool	SetChunkSize(size iChunkSize);

	bool	Write(CFileWriter* pcFileWriter);
	bool	Read(CFileReader* pcFileReader);

private:
	bool	MemcpyDestAndUnlock(void* pvSource, void* pvDest);
};


#endif // _SAFE_ARRAY_BLOCK_H__

