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
	bool				mbOverwrite;

public:
	void			_Init(void);
	void			Init(int iElementSize, DataCompare fCompare);
	void			Init(int iElementSize, int iHoldingBufferSize, int iHoldingBuffers, DataCompare fCompare);
	void			Init(CMallocator* pcMalloc, int iElementSize, int iHoldingBufferSize, int iHoldingBuffers, DataCompare fCompare);
	void			Kill(void);

	bool			Add(void* pv);

	void*			Get(void* pv);

	bool			Contains(void* pv);

	bool			Remove(void* pv);

	void*			StartIteration(SArraySortedIterator* psIter);
	void*			Iterate(SArraySortedIterator* psIter);
	void*			GetIterated(SArraySortedIterator* psIter);

	size_t			ByteSize(void);

	bool			WriteHeader(CFileWriter* pcFileWriter);
	bool			Write(CFileWriter* pcFileWriter);
	bool			ReadHeader(CMallocator* pcMalloc, CFileReader* pcFileReader, DataCompare fCompare);
	bool			Read(CFileReader* pcFileReader, DataCompare fCompare);

	void			SetOverwrite(bool bOverwrite);

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
	bool			AddIntoHoldingArrays(void* pv, bool* pbUpdateSortedArray);
	void			ClearHoldingArrays(void);
	void			MergeHoldingArrays(CArrayBlock* paMergedArray);
	void			SortMerge(CArrayBlock* paMergedArray);
	void			InsertHoldingIntoSorted(int* paiInsertionIndices, int oldLength, CArrayBlock* paSourceArray);
	int*			CalculateInsertionIndices(CArrayBlock* paMergedHoldingArrays);
	void*			FindInHoldingArrays(void* pv);
	void*			FindInSortedArray(void* pv);
	bool			RemoveFromHoldingArrays(void* pv);
	bool			RemoveFromSortedArray(void* pv);
	CArrayBlock*	GetArrayBlock(int iIndex);
	bool			InsertIntoArrayBlock(CArrayBlock* paBlock, void* pv);
};


#endif // __ARRAY_BLOCK_SORTED_H__

