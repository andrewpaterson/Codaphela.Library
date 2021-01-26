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


class CArrayBlockSorted : public CMalloc
{
protected:
	CArrayBlock			maSortedArray;
	CArrayArrayBlock	maaHoldingArrays;
	int					(*mfCompare)(const void*, const void*);
	int*				mapiInsertionIndices;

	int					miElementSize;
	int					miHoldingBufferSize;
	BOOL				mbOverwrite;

public:
	void			Init(int iElementSize, int(*fCompare)(const void*, const void*));
	void			Init(int iElementSize, int iHoldingBufferSize, int iHoldingBuffers, int(*fCompare)(const void*, const void*));
	void			Init(CMallocator* pcMalloc, int iElementSize, int iHoldingBufferSize, int iHoldingBuffers, int(*fCompare)(const void*, const void*));
	void			Kill(void);

	BOOL			Add(void* pv);

	void*			Get(void* pv);

	BOOL			Contains(void* pv);

	BOOL			Remove(void* pv);

	void*			StartIteration(SArraySortedIterator* psIter);
	void*			Iterate(SArraySortedIterator* psIter);
	void*			GetIterated(SArraySortedIterator* psIter);

	BOOL			WriteHeader(CFileWriter* pcFileWriter);
	BOOL			Write(CFileWriter* pcFileWriter);
	BOOL			ReadHeader(CMallocator* pcMalloc, CFileReader* pcFileReader, int(*Func)(const void*, const void*));
	BOOL			Read(CFileReader* pcFileReader, int(*Func)(const void*, const void*));

	void			SetOverwrite(BOOL bOverwrite);

	int				NumElements(void);
	int				GetSortedSize(void);
	int				GetHoldingSize(void);
	void*			GetInSorted(int iIndex);
	void*			GetInHolding(int iArray, int iIndex);

	void			InsertHoldingIntoSorted(void);
	CArrayBlock*	GetSortedArray(void);

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

