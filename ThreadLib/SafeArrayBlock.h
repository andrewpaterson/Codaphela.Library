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
	void 	Init(int iElementSize);
	void 	Init(CMallocator* pcMalloc, int iElementSize);
	void	Init(CMallocator* pcMalloc, int iElementSize, int iChunkSize);
	void 	ReInit(void);

	void 	Finalise(void);
	void 	Kill(void);

	int		NumElements(void);
	bool	IsEmpty(void);
	bool	IsNotEmpty(void);
	int		AllocatedElements(void);
	int 	ElementSize(void);

	void	Add(void* pvData);
	int		AddGetIndex(void* pvData);
	int 	AddIfUnique(void* pData);
	int 	AddIfUniqueKey(void* pData, int iKeyOffset, int iKeySize);

	void 	Copy(CArrayBlock* pcTemplateArray);
	void 	Copy(CSafeArrayBlock* pcTemplateArray);
	int		Copy(CStackMemory<>* pcTemp);

	bool	Get(int iIndex, void* pvDest);
	bool	Tail(void* pvDest);

	void	InsertAt(void* pvData, int iIndex);
	int		InsertIntoSorted(int(*fCompare)(const void*, const void*), void* pvData, bool bOverwriteExisting);

	bool	Pop(void* pvDest);
	bool	Pop(void);
	void 	Push(void* pvData);
	bool	PopFirst(void* pvData);
	bool	PopFirst(void);

	int		Resize(int iNumElements);

	void	BubbleSort(int(*fCompare)(const void*, const void*));
	void	QuickSort(int(*fCompare)(const void*, const void*));
	void	Reverse(void);

	bool	Contains(void* pData);
	int 	Find(void* pData);
	bool	FindInSorted(void* pData, int(*fCompare)(const void*, const void*), int* piIndex);

	void 	RemoveAt(int iIndex, int bPreserveOrder = true);
	void	RemoveRange(int iStartIndex, int iEndIndexExclusive, bool bPreserveOrder = true);
	bool 	RemoveTail(void);

	void	Set(int iIndex, void* pvData);
	void	Swap(int iIndex1, int iIndex2);
	void 	Zero(void);

	int 	ByteSize(void);
	int		ChunkSize(void);
	int		SetUsedElements(int iNumElements);

	bool	SetChunkSize(int iChunkSize);

	bool	Write(CFileWriter* pcFileWriter);
	bool	Read(CFileReader* pcFileReader);

private:
	bool	MemcpyDestAndUnlock(void* pvSource, void* pvDest);
};


#endif // _SAFE_ARRAY_BLOCK_H__

