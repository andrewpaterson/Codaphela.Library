#include "ArrayBlockSorted.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockSorted::Init(int iElementSize, int(*Func)(const void*, const void*))
{
	Init(&gcSystemAllocator, iElementSize, 1024, 256, 4, Func);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockSorted::Init(int iElementSize, int iChunkSize, int iHoldingBufferSize, int iHoldingBuffers, int(*Func)(const void*, const void*))
{
	Init(&gcSystemAllocator, iElementSize, iChunkSize, iHoldingBufferSize, iHoldingBuffers, Func);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockSorted::Init(CMallocator* pcMallocator, int iElementSize, int iChunkSize, int iHoldingBufferSize, int iHoldingBuffers, int(*Func)(const void*, const void*))
{
	CArrayBlock*	paHoldingArray;
	int				i;

	mpcMallocator = pcMallocator;
	miHoldingBufferSize = iHoldingBufferSize;
	miChunkSize = iChunkSize;
	miElementSize = iElementSize;
	this->Func = Func;

	maSortedArray.Init(pcMallocator, iElementSize, miChunkSize);
	maaHoldingArrays.Allocate(pcMallocator, iHoldingBuffers);
	for (i = 0; i < iHoldingBuffers; i++)
	{
		paHoldingArray = maaHoldingArrays.Get(i);
		paHoldingArray->Init(pcMallocator, miElementSize, iHoldingBufferSize);
	}

	mapiInsertionIndices = (int*)mpcMallocator->Malloc(miHoldingBufferSize * iHoldingBuffers * sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockSorted::Kill(void)
{
	CArrayBlock*	paHoldingArray;
	int				i;

	mpcMallocator->Free(mapiInsertionIndices);
	for (i = 0; i < maaHoldingArrays.NumElements(); i++)
	{
		paHoldingArray = maaHoldingArrays.Get(i);
		paHoldingArray->Kill();
	}
	maaHoldingArrays.Kill();
	maSortedArray.Kill();
	mpcMallocator = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlockSorted::Add(void* pv)
{
	if (!Contains(pv))
	{
		BOOL updateSortedArray = AddIntoHoldingArrays(pv);
		if (updateSortedArray)
		{
			InsertHoldingIntoSorted();
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlockSorted::AddIntoHoldingArrays(void* pv)
{
	int				iIndex;
	BOOL			bFound;
	CArrayBlock*	paHoldingArray;
	int				i;

	for (i = 0; i < maaHoldingArrays.NumElements(); i++)
	{
		paHoldingArray = maaHoldingArrays.Get(i);
		if (paHoldingArray->NumElements() < miHoldingBufferSize)
		{
			bFound = paHoldingArray->FindInSorted(pv, Func, &iIndex);
			paHoldingArray->InsertAt(pv, iIndex);

			if ((paHoldingArray->NumElements() == miHoldingBufferSize) && (i == maaHoldingArrays.NumElements() - 1))
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockSorted::InsertHoldingIntoSorted(void)
{
	CArrayBlock		aMergedHoldingArrays;
	int*			paiInsertionIndices;
	int				oldLength;

	aMergedHoldingArrays.Init(mpcMallocator, miElementSize, miHoldingBufferSize * maaHoldingArrays.NumElements());
	MergeHoldingArrays(&aMergedHoldingArrays);

	if (aMergedHoldingArrays.IsNotEmpty())
	{
		if (maSortedArray.IsEmpty())
		{
			maSortedArray.Copy(&aMergedHoldingArrays);
		}
		else
		{
			paiInsertionIndices = CalculateInsertionIndices(&aMergedHoldingArrays);

			oldLength = maSortedArray.NumElements();
			maSortedArray.InsertArrayAfterEnd(&aMergedHoldingArrays);

			InsertHoldingIntoSorted(paiInsertionIndices, oldLength, &aMergedHoldingArrays);
		}

		ClearHoldingArrays();
	}
	aMergedHoldingArrays.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockSorted::ClearHoldingArrays(void)
{
	CArrayBlock*	paHoldingArray;
	int				i;

	for (i = 0; i < maaHoldingArrays.NumElements(); i++)
	{
		paHoldingArray = maaHoldingArrays.Get(i);
		paHoldingArray->ReInit();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockSorted::MergeHoldingArrays(CArrayBlock* paMergedArray)
{
	if (maaHoldingArrays.NumElements() == 1)
	{
		paMergedArray->Copy(maaHoldingArrays.Get(0));
	}
	else
	{
		SortMerge(paMergedArray);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockSorted::SortMerge(CArrayBlock* paMergedArray)
{
	CArrayBlock*	paHoldingArray;
	int				i;

	for (i = 0; i < maaHoldingArrays.NumElements(); i++)
	{
		paHoldingArray = maaHoldingArrays.Get(i);
		paMergedArray->InsertArrayAfterEnd(paHoldingArray);
	}

	paMergedArray->QuickSort(Func);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockSorted::InsertHoldingIntoSorted(int* paiInsertionIndices, int oldLength, CArrayBlock* paSourceArray)
{
	void* pvSortedArrayElementData = maSortedArray.GetData();

	int		offset;
	int		indexIndex;
	int		sortedIndex;
	int		destIndex;
	int		startIndex;
	int		nextSortedIndex;
	int		length;
	void*	pvOther;

	offset = 0;
	indexIndex = -1;
	sortedIndex = 0;
	destIndex = 0;
	startIndex = paSourceArray->NumElements() - 1;
	nextSortedIndex = oldLength;

	for (;;)
	{
		for (indexIndex = startIndex; indexIndex >= 0; indexIndex--)
		{
			sortedIndex = paiInsertionIndices[indexIndex];
			destIndex = sortedIndex + indexIndex;
			if (sortedIndex >= nextSortedIndex)
			{
				pvOther = paSourceArray->Get(indexIndex);
				maSortedArray.Set(destIndex, pvOther);
			}
			else
			{
				break;
			}
		}

		if (indexIndex >= 0)
		{
			length = nextSortedIndex - sortedIndex;
			startIndex = indexIndex;
			nextSortedIndex = nextSortedIndex - length;
			memcpy(RemapSinglePointer(pvSortedArrayElementData, (destIndex + 1) * miElementSize), RemapSinglePointer(pvSortedArrayElementData, sortedIndex * miElementSize), length * miElementSize);
		}
		else
		{
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int* CArrayBlockSorted::CalculateInsertionIndices(CArrayBlock* paMergedHoldingArrays)
{
	BOOL	bFound;
	int		iIndex;
	void*	pv;
	int		i;

	for (i = 0; i < paMergedHoldingArrays->NumElements(); i++)
	{
		pv = paMergedHoldingArrays->Get(i);
		bFound = maSortedArray.FindInSorted(pv, Func, &iIndex);
		mapiInsertionIndices[i] = iIndex;
	}

	return mapiInsertionIndices;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlockSorted::Contains(void* pv)
{
	void* pvFound;

	pvFound = Get(pv);
	if (pvFound)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlockSorted::Remove(void* pv)
{
	BOOL	bRemoved;

	bRemoved = RemoveFromSortedArray(pv);
	if (bRemoved)
	{
		return TRUE;
	}

	bRemoved = RemoveFromHoldingArrays(pv);
	return bRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlockSorted::Get(void* pv)
{
	void*	pvFound;

	pvFound = FindInSortedArray(pv);
	if (pvFound)
	{
		return pvFound;
	}
	else 
	{
		pvFound = FindInHoldingArrays(pv);
		if (pvFound)
		{
			return pvFound;
		}
		else
		{
			return NULL;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlockSorted::FindInHoldingArrays(void* pv)
{
	BOOL			bFound;
	int				iIndex;
	CArrayBlock*	paHoldingArray;
	int				i;
	void*			pvData;

	for (i = 0; i < maaHoldingArrays.NumElements(); i++)
	{
		paHoldingArray = maaHoldingArrays.Get(i);
		bFound = paHoldingArray->FindInSorted(pv, Func, &iIndex);
		if (bFound)
		{
			pvData = paHoldingArray->Get(iIndex);
			return pvData;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlockSorted::FindInSortedArray(void* pv)
{
	int		iIndex;
	BOOL	bFound;
	void*	pvData;

	bFound = maSortedArray.FindInSorted(pv, Func, &iIndex);
	if (bFound)
	{
		pvData = maSortedArray.Get(iIndex);
		return pvData;

	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlockSorted::RemoveFromHoldingArrays(void* pv)
{
	BOOL			bFound;
	int				iIndex;
	CArrayBlock*	paHoldingArray;
	int				i;

	for (i = 0; i < maaHoldingArrays.NumElements(); i++)
	{
		paHoldingArray = maaHoldingArrays.Get(i);
		bFound = paHoldingArray->FindInSorted(pv, Func, &iIndex);
		if (bFound)
		{
			paHoldingArray->RemoveAt(iIndex);
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlockSorted::RemoveFromSortedArray(void* pv)
{
	int		iIndex;
	BOOL	bFound;

	bFound = maSortedArray.FindInSorted(pv, Func, &iIndex);
	if (bFound)
	{
		maSortedArray.RemoveAt(iIndex);
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlockSorted::StartIteration(SArraySortedIterator* psIter)
{
	psIter->iArrayBlock = 0;
	psIter->iIndex = 0;

	if (NumElements() == 0)
	{
		return NULL;
	}

	return GetIterated(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlockSorted::Iterate(SArraySortedIterator* psIter)
{
	CArrayBlock*	paHoldingArray;

	paHoldingArray = GetArrayBlock(psIter->iArrayBlock);
	if (paHoldingArray)
	{
		psIter->iIndex++;
		for (;;)
		{
			if (psIter->iIndex >= paHoldingArray->NumElements())
			{
				psIter->iArrayBlock++;
				psIter->iIndex = 0;
			}
			else
			{
				return GetIterated(psIter);
			}

			paHoldingArray = GetArrayBlock(psIter->iArrayBlock);
			if (!paHoldingArray)
			{
				return NULL;
			}
		}
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlockSorted::GetIterated(SArraySortedIterator* psIter)
{
	CArrayBlock*	paHoldingArray;

	paHoldingArray = GetArrayBlock(psIter->iArrayBlock);
	if (paHoldingArray)
	{
		return paHoldingArray->SafeGet(psIter->iIndex);
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlockSorted::WriteHeader(CFileWriter* pcFileWriter)
{
	ReturnOnFalse(pcFileWriter->WriteInt(miHoldingBufferSize));
	ReturnOnFalse(pcFileWriter->WriteInt(miChunkSize));
	ReturnOnFalse(pcFileWriter->WriteInt(miElementSize));
	ReturnOnFalse(pcFileWriter->WriteInt(maaHoldingArrays.NumElements()));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlockSorted::Write(CFileWriter* pcFileWriter)
{
	if (!WriteHeader(pcFileWriter))
	{
		return FALSE;
	}

	InsertHoldingIntoSorted();
	return maSortedArray.Write(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlockSorted::ReadHeader(CFileReader* pcFileReader, int(*Func)(const void*, const void*))
{
	CArrayBlock*	paHoldingArray;
	int				i;
	int				iHoldingBufferSize;
	int				iChunkSize;
	int				iElementSize;
	int				iHoldingBuffers;

	mpcMallocator = &gcSystemAllocator;
	ReturnOnFalse(pcFileReader->ReadInt(&iHoldingBufferSize));
	ReturnOnFalse(pcFileReader->ReadInt(&iChunkSize));
	ReturnOnFalse(pcFileReader->ReadInt(&iElementSize));
	ReturnOnFalse(pcFileReader->ReadInt(&iHoldingBuffers));

	miHoldingBufferSize = iHoldingBufferSize;
	miChunkSize = iChunkSize;
	miElementSize = iElementSize;
	this->Func = Func;

	maaHoldingArrays.Allocate(mpcMallocator, iHoldingBuffers);
	for (i = 0; i < iHoldingBuffers; i++)
	{
		paHoldingArray = maaHoldingArrays.Get(i);
		paHoldingArray->Init(mpcMallocator, miElementSize, iHoldingBufferSize);
	}

	mapiInsertionIndices = (int*)mpcMallocator->Malloc(miHoldingBufferSize * iHoldingBuffers * sizeof(int));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlockSorted::Read(CFileReader* pcFileReader, int(*Func)(const void*, const void*))
{
	if (!ReadHeader(pcFileReader, Func))
	{
		return FALSE;
	}

	return maSortedArray.Read(pcFileReader);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayBlock* CArrayBlockSorted::GetArrayBlock(int iIndex)
{
	CArrayBlock*	paHoldingArray;

	if (maSortedArray.NumElements() > 0)
	{
		if (iIndex == 0)
		{
			return &maSortedArray;
		}
		else
		{
			paHoldingArray = maaHoldingArrays.SafeGet(iIndex - 1);
			return paHoldingArray;
		}
	}
	else
	{
		paHoldingArray = maaHoldingArrays.SafeGet(iIndex);
		return paHoldingArray;
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayBlock* CArrayBlockSorted::GetSortedArray(void)
{
	return &maSortedArray;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CArrayBlockSorted::NumElements(void)
{
	return GetSortedSize() + GetHoldingSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CArrayBlockSorted::GetSortedSize(void)
{
	return maSortedArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CArrayBlockSorted::GetHoldingSize(void)
{
	int				iSize;
	CArrayBlock*	paHoldingArray;
	int				i;

	iSize = 0;
	for (i = 0; i < maaHoldingArrays.NumElements(); i++)
	{
		paHoldingArray = maaHoldingArrays.Get(i);

		iSize += paHoldingArray->NumElements();
	}
	return iSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlockSorted::GetInSorted(int iIndex)
{
	return maSortedArray.SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlockSorted::GetInHolding(int iArray, int iIndex)
{
	CArrayBlock*	paHoldingArray;

	paHoldingArray = maaHoldingArrays.SafeGet(iArray);
	if (!paHoldingArray)
	{
		return NULL;
	}
	
	return paHoldingArray->SafeGet(iIndex);
}

