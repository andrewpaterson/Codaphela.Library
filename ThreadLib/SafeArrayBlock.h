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
	BOOL	IsEmpty(void);
	BOOL	IsNotEmpty(void);
	int		AllocatedElements(void);
	int 	ElementSize(void);

	void	Add(void* pvData);
	int		AddGetIndex(void* pvData);
	int 	AddIfUnique(void* pData);
	int 	AddIfUniqueKey(void* pData, int iKeyOffset, int iKeySize);

	void 	Copy(CArrayBlock* pcTemplateArray);
	void 	Copy(CSafeArrayBlock* pcTemplateArray);
	int		Copy(CStackMemory<>* pcTemp);

	BOOL	Get(int iIndex, void* pvDest);
	BOOL	Tail(void* pvDest);

	void	InsertAt(void* pvData, int iIndex);
	int		InsertIntoSorted(int(*fCompare)(const void*, const void*), void* pvData, BOOL bOverwriteExisting);

	BOOL	Pop(void* pvDest);
	BOOL	Pop(void);
	void 	Push(void* pvData);

	int		Resize(int iNumElements);

	void	BubbleSort(int(*fCompare)(const void*, const void*));
	void	QuickSort(int(*fCompare)(const void*, const void*));
	void	Reverse(void);

	BOOL	Contains(void* pData);
	int 	Find(void* pData);
	BOOL	FindInSorted(void* pData, int(*fCompare)(const void*, const void*), int* piIndex);

	void 	RemoveAt(int iIndex, int bPreserveOrder = TRUE);
	void	RemoveRange(int iStartIndex, int iEndIndexExclusive, BOOL bPreserveOrder = TRUE);
	BOOL 	RemoveTail(void);

	void	Set(int iIndex, void* pvData);
	void	Swap(int iIndex1, int iIndex2);
	void 	Zero(void);

	int 	ByteSize(void);
	int		ChunkSize(void);
	int		SetUsedElements(int iNumElements);

	BOOL	SetChunkSize(int iChunkSize);

	BOOL	Write(CFileWriter* pcFileWriter);
	BOOL	Read(CFileReader* pcFileReader);

private:
	BOOL	MemcpyDestAndUnlock(void* pvSource, void* pvDest);
};


#endif // _SAFE_ARRAY_BLOCK_H__

