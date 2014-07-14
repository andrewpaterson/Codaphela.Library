#ifndef __ARRAY_BLOCK_SORTED_H__
#define __ARRAY_BLOCK_SORTED_H__
#include "SystemAllocator.h"
#include "ArrayBlock.h"
#include "ArrayTemplate.h"


typedef CArrayTemplate<CArrayBlock> CArrayArrayBlock;


class CArrayBlockSorted 
{
protected:
	CArrayBlock			maSortedArray;
	CArrayArrayBlock	maaHoldingArrays;
	int					miHoldingBufferSize;
	int					miChunkSize;
	int					miElementSize;
	CMallocator*		mpcMallocator;
	int(*Func)(const void*, const void*);
	int*				mapiInsertionIndices;

public:
	void	Init(int iElementSize, int(*Func)(const void*, const void*));
	void	Init(int iElementSize, int iChunkSize, int iHoldingBufferSize, int iHoldingBuffers, int(*Func)(const void*, const void*));
	void	Init(CMallocator* pcMallocator, int iElementSize, int iChunkSize, int iHoldingBufferSize, int iHoldingBuffers, int(*Func)(const void*, const void*));
	void	Kill(void);
	BOOL	Add(void* pv);
	BOOL	AddIntoHoldingArrays(void* pv);
	void	InsertHoldingIntoSorted(void);
	void	ClearHoldingArrays(void);
	void	MergeHoldingArrays(CArrayBlock* paMergedArray);
	void	SortMerge(CArrayBlock* paMergedArray);
	void	InsertHoldingIntoSorted(int* paiInsertionIndices, int oldLength, CArrayBlock* paSourceArray);
	int*	CalculateInsertionIndices(CArrayBlock* paMergedHoldingArrays);
	BOOL	Contains(void* pv);
	BOOL	ContainedInHoldingArrays(void* pv);
	BOOL	ContainedInSortedArray(void* pv);
	int		NumElements(void);
	int		GetSortedSize(void);
	int		GetHoldingSize(void);
	void*	GetInSorted(int iIndex);

	//  void DribbleMerge(ArrayList<T> mergedArray)
	//  {
	//    List<MergeIndex> mergeIndices = createMergeIndices();
	//
	//    for (; ; )
	//    {
	//      void* pv = getLowestObject(mergeIndices);
	//      if (pv == null)
	//      {
	//        break;
	//      }
	//
	//      mergedArray.add(pv);
	//    }
	//  }
	//
	//  class MergeIndex
	//  {
	//     int iIndex;
	//     ArrayList<T> paHoldingArray;
	//
	//    MergeIndex(ArrayList<T> paHoldingArray)
	//    {
	//      this.paHoldingArray = paHoldingArray;
	//      iIndex = 0;
	//    }
	//
	//     T peek()
	//    {
	//      return paHoldingArray->get(iIndex);
	//    }
	//
	//     BOOL pop()
	//    {
	//      iIndex++;
	//
	//      if (iIndex == paHoldingArray->NumElements())
	//      {
	//        return FALSE;
	//      }
	//      else
	//      {
	//        return TRUE;
	//      }
	//    }
	//  }
	//
	//  T getLowestObject(List<MergeIndex> mergeIndices)
	//  {
	//    if (mergeIndices.NumElements() == 0)
	//    {
	//      return null;
	//    }
	//    else if (mergeIndices.NumElements() == 1)
	//    {
	//      MergeIndex mergeIndex = mergeIndices.get(0);
	//      T lowestObject = mergeIndex.peek();
	//      if (!mergeIndex.pop())
	//      {
	//        mergeIndices.clear();
	//      }
	//
	//      return lowestObject;
	//    }
	//    else
	//    {
	//      int i = 0;
	//      int lowest = i;
	//      MergeIndex mergeIndex = mergeIndices.get(i);
	//      T lowestObject = mergeIndex.peek();
	//
	//      for (i = 1; i < mergeIndices.NumElements(); i++)
	//      {
	//        mergeIndex = mergeIndices.get(i);
	//        T testObject = mergeIndex.peek();
	//        int compare = Func.compare(testObject, lowestObject);
	//        if (compare < 0)
	//        {
	//          lowestObject = testObject;
	//          lowest = i;
	//        }
	//      }
	//
	//      mergeIndex = mergeIndices.get(lowest);
	//      if (!mergeIndex.pop())
	//      {
	//        mergeIndices.remove(lowest);
	//      }
	//
	//      return lowestObject;
	//    }
	//  }
	//
	//  List<MergeIndex> createMergeIndices()
	//  {
	//    List<MergeIndex> mergeIndices = new ArrayList<MergeIndex>();
	//    for (ArrayList<T> paHoldingArray : maaHoldingArrays)
	//    {
	//      if (paHoldingArray->NumElements() > 0)
	//      {
	//        mergeIndices.add(new MergeIndex(paHoldingArray));
	//      }
	//    }
	//
	//    return mergeIndices;
	//  }
};


#endif // __ARRAY_BLOCK_SORTED_H__

