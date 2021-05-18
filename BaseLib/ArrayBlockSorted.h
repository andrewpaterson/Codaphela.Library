#ifndef __ARRAY_BLOCK_SORTED_H__
#define __ARRAY_BLOCK_SORTED_H__
#include "SystemAllocator.h"
#include "ArrayBlock.h"
#include "ArrayTemplate.h"
#include "Malloc.h"


typedef CArrayTemplate<CArrayBlock> CArrayArrayBlock;


struct SArraySortedIterator
{
	int iArrayBlock;
	int iIndex;
};


class CChars;
class CArrayBlockSorted : public CMalloc
{
protected:
	CArrayBlock			maSortedArray;
	CArrayArrayBlock	maaHoldingArrays;
	DataCompare			mfCompare;
	int*				mapiInsertionIndices;

	int					miElementSize;
	int					miHoldingBufferSize;
	BOOL				mbOverwrite;

public:
	void			Init(int iElementSize, DataCompare fCompare);
	void			Init(int iElementSize, int iHoldingBufferSize, int iHoldingBuffers, DataCompare fCompare);
	void			Init(CMallocator* pcMalloc, int iElementSize, int iHoldingBufferSize, int iHoldingBuffers, DataCompare fCompare);
	void			Kill(void);

	BOOL			Add(void* pv);

	void*			Get(void* pv);

	BOOL			Contains(void* pv);

	BOOL			Remove(void* pv);

	void*			StartIteration(SArraySortedIterator* psIter);
	void*			Iterate(SArraySortedIterator* psIter);
	void*			GetIterated(SArraySortedIterator* psIter);

	size_t			ByteSize(void);

	BOOL			WriteHeader(CFileWriter* pcFileWriter);
	BOOL			Write(CFileWriter* pcFileWriter);
	BOOL			ReadHeader(CMallocator* pcMalloc, CFileReader* pcFileReader, DataCompare fCompare);
	BOOL			Read(CFileReader* pcFileReader, DataCompare fCompare);

	void			SetOverwrite(BOOL bOverwrite);

	int				NumElements(void);
	int				GetSortedSize(void);
	int				GetHoldingSize(void);
	void*			GetInSorted(int iIndex);
	void*			GetInHolding(int iArray, int iIndex);

	void			InsertHoldingIntoSorted(void);
	CArrayBlock*	GetSortedArray(void);

	void			Print(CChars* psz);
	void			Dump(void);

protected:
	BOOL			AddIntoHoldingArrays(void* pv, BOOL* pbUpdateSortedArray);
	void			ClearHoldingArrays(void);
	void			MergeHoldingArrays(CArrayBlock* paMergedArray);
	void			SortMerge(CArrayBlock* paMergedArray);
	void			InsertHoldingIntoSorted(int* paiInsertionIndices, int oldLength, CArrayBlock* paSourceArray);
	int*			CalculateInsertionIndices(CArrayBlock* paMergedHoldingArrays);
	void*			FindInHoldingArrays(void* pv);
	void*			FindInSortedArray(void* pv);
	BOOL			RemoveFromHoldingArrays(void* pv);
	BOOL			RemoveFromSortedArray(void* pv);
	CArrayBlock*	GetArrayBlock(int iIndex);
	BOOL			InsertIntoArrayBlock(CArrayBlock* paBlock, void* pv);
};


#endif // __ARRAY_BLOCK_SORTED_H__

