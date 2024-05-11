#include "GlobalMemory.h"
#include "Chars.h"
#include "ArrayBlockSorted.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockSorted::_Init(void)
{
	maSortedArray._Init();
	maaHoldingArrays._Init();
	mfCompare = NULL;
	mapiInsertionIndices = NULL;
	miElementSize = 0;
	miHoldingBufferSize = 0;
	mbOverwrite = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockSorted::Init(size iElementSize, DataCompare fCompare)
{
	Init(&gcSystemAllocator, iElementSize, 256, 4, fCompare);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockSorted::Init(size iElementSize, size iHoldingBufferSize, size iHoldingBuffers, DataCompare fCompare)
{
	Init(&gcSystemAllocator, iElementSize, iHoldingBufferSize, iHoldingBuffers, fCompare);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockSorted::Init(CMallocator* pcMalloc, size iElementSize, size iHoldingBufferSize, size iHoldingBuffers, DataCompare fCompare)
{
	CArrayBlock*	paHoldingArray;
	size			i;

	CMalloc::Init(pcMalloc);
	miHoldingBufferSize = iHoldingBufferSize;
	miElementSize = iElementSize;
	mfCompare = fCompare;
	mbOverwrite = false;

	maSortedArray.Init(pcMalloc, miElementSize);
	maaHoldingArrays.Init(pcMalloc);
	if (iHoldingBuffers > 0)
	{
		maaHoldingArrays.Resize(iHoldingBuffers);
		for (i = 0; i < iHoldingBuffers; i++)
		{
			paHoldingArray = maaHoldingArrays.Get(i);
			paHoldingArray->Init(pcMalloc, miElementSize);
		}
		mapiInsertionIndices = (size*)mpcMalloc->Malloc(miHoldingBufferSize * iHoldingBuffers * sizeof(size));
	}
	else
	{
		mapiInsertionIndices = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockSorted::Kill(void)
{
	CArrayBlock*	paHoldingArray;
	size			i;
	size			iNumElements;

	iNumElements = maaHoldingArrays.NumElements();
	mpcMalloc->Free(mapiInsertionIndices);
	for (i = 0; i < iNumElements; i++)
	{
		paHoldingArray = maaHoldingArrays.Get(i);
		paHoldingArray->Kill();
	}
	maaHoldingArrays.Kill();
	maSortedArray.Kill();
	CMalloc::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayBlockSorted::Add(void* pv)
{
	bool bUpdateSortedArray;
	bool bAdded;

	if (maaHoldingArrays.NumElements() > 0)
	{
		bAdded = AddIntoHoldingArrays(pv, &bUpdateSortedArray);
		if (bUpdateSortedArray)
		{
			InsertHoldingIntoSorted();
		}
		return bAdded;
	}
	else
	{
		return InsertIntoArrayBlock(&maSortedArray, pv);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayBlockSorted::AddIntoHoldingArrays(void* pv, bool* pbUpdateSortedArray)
{
	bool			bAdded;
	CArrayBlock*	paHoldingArray;
	size			i;
	size			iNumElements;

	iNumElements = maaHoldingArrays.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		paHoldingArray = maaHoldingArrays.Get(i);
		if (paHoldingArray->NumElements() < miHoldingBufferSize)
		{
			bAdded = InsertIntoArrayBlock(paHoldingArray, pv);
			if (bAdded)
			{
				if ((paHoldingArray->NumElements() == miHoldingBufferSize) && (i == maaHoldingArrays.NumElements() - 1))
				{
					*pbUpdateSortedArray = true;
				}
				else
				{
					*pbUpdateSortedArray = false;
				}
				return true;
			}
			else
			{
				*pbUpdateSortedArray = false;
				return false;
			}
		}
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayBlockSorted::InsertIntoArrayBlock(CArrayBlock* paBlock, void* pv)
{
	size	iIndex;
	bool	bFound;

	bFound = paBlock->FindInSorted(pv, mfCompare, &iIndex);
	if (!bFound)
	{
		paBlock->InsertAt(pv, iIndex);
		return true;
	}
	else
	{
		if (mbOverwrite)
		{
			paBlock->Set(iIndex, pv);
			return true;
		}
		else
		{
			return false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockSorted::InsertHoldingIntoSorted(void)
{
	CArrayBlock		aMergedHoldingArrays;
	size*			paiInsertionIndices;
	size			oldLength;

	aMergedHoldingArrays.Init(mpcMalloc, miElementSize);
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
	size			i;
	size			iNumElements;

	iNumElements = maaHoldingArrays.NumElements();
	for (i = 0; i < iNumElements; i++)
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
	size			i;
	size			iNumElements;

	iNumElements = maaHoldingArrays.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		paHoldingArray = maaHoldingArrays.Get(i);
		paMergedArray->InsertArrayAfterEnd(paHoldingArray);
	}

	paMergedArray->QuickSort(mfCompare);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockSorted::InsertHoldingIntoSorted(size* paiInsertionIndices, size oldLength, CArrayBlock* paSourceArray)
{
	void*	pvSortedArrayElementData;

	size	offset;
	size	indexIndex;
	size	sortedIndex;
	size	destIndex;
	size	startIndex;
	size	nextSortedIndex;
	size	length;
	void*	pvOther;

	pvSortedArrayElementData = maSortedArray.GetData();

	offset = 0;
	indexIndex = ARRAY_ELEMENT_NOT_FOUND;
	sortedIndex = 0;
	destIndex = 0;
	startIndex = paSourceArray->NumElements() - 1;
	nextSortedIndex = oldLength;

	for (;;)
	{
		for (indexIndex = startIndex; indexIndex != ARRAY_ELEMENT_NOT_FOUND; indexIndex--)
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

		if (indexIndex != ARRAY_ELEMENT_NOT_FOUND)
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
size* CArrayBlockSorted::CalculateInsertionIndices(CArrayBlock* paMergedHoldingArrays)
{
	bool	bFound;
	size	iIndex;
	void*	pv;
	size	i;
	size	iNumElements;

	iNumElements = paMergedHoldingArrays->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pv = paMergedHoldingArrays->Get(i);
		bFound = maSortedArray.FindInSorted(pv, mfCompare, &iIndex);
		mapiInsertionIndices[i] = iIndex;
	}

	return mapiInsertionIndices;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayBlockSorted::Contains(void* pv)
{
	void*	pvFound;

	pvFound = Get(pv);
	if (pvFound)
	{
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayBlockSorted::Remove(void* pv)
{
	bool	bRemoved;

	bRemoved = RemoveFromSortedArray(pv);
	if (bRemoved)
	{
		return true;
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
	bool			bFound;
	size			iIndex;
	CArrayBlock*	paHoldingArray;
	size			i;
	void*			pvData;
	size			iNumElements;

	iNumElements = maaHoldingArrays.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		paHoldingArray = maaHoldingArrays.Get(i);
		bFound = paHoldingArray->FindInSorted(pv, mfCompare, &iIndex);
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
	size	iIndex;
	bool	bFound;
	void*	pvData;

	bFound = maSortedArray.FindInSorted(pv, mfCompare, &iIndex);
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
bool CArrayBlockSorted::RemoveFromHoldingArrays(void* pv)
{
	bool			bFound;
	size			iIndex;
	CArrayBlock*	paHoldingArray;
	size			i;

	for (i = 0; i < maaHoldingArrays.NumElements(); i++)
	{
		paHoldingArray = maaHoldingArrays.Get(i);
		bFound = paHoldingArray->FindInSorted(pv, mfCompare, &iIndex);
		if (bFound)
		{
			paHoldingArray->RemoveAt(iIndex);
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayBlockSorted::RemoveFromSortedArray(void* pv)
{
	size	iIndex;
	bool	bFound;

	bFound = maSortedArray.FindInSorted(pv, mfCompare, &iIndex);
	if (bFound)
	{
		maSortedArray.RemoveAt(iIndex);
		return true;
	}
	return false;
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
bool CArrayBlockSorted::WriteHeader(CFileWriter* pcFileWriter)
{
	ReturnOnFalse(pcFileWriter->WriteSize(miHoldingBufferSize));
	ReturnOnFalse(pcFileWriter->WriteSize(miElementSize));
	ReturnOnFalse(pcFileWriter->WriteSize(maaHoldingArrays.NumElements()));
	ReturnOnFalse(pcFileWriter->WriteBool(mbOverwrite));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayBlockSorted::Write(CFileWriter* pcFileWriter)
{
	bool	bResult;

	bResult = gcMallocators.Write(pcFileWriter, mpcMalloc);
	if (!bResult)
	{
		return false;
	}

	bResult = WriteHeader(pcFileWriter);
	if (!bResult)
	{
		return false;
	}

	InsertHoldingIntoSorted();
	return maSortedArray.Write(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayBlockSorted::ReadHeader(CMallocator* pcMalloc, CFileReader* pcFileReader, DataCompare fCompare)
{
	CArrayBlock*	paHoldingArray;
	size			i;
	size			iHoldingBufferSize;
	size			iElementSize;
	size			iHoldingBuffers;
	bool			bOverwrite;

	CMalloc::Init(pcMalloc);
	ReturnOnFalse(pcFileReader->ReadSize(&iHoldingBufferSize));
	ReturnOnFalse(pcFileReader->ReadSize(&iElementSize));
	ReturnOnFalse(pcFileReader->ReadSize(&iHoldingBuffers));
	ReturnOnFalse(pcFileReader->ReadBool(&bOverwrite));

	miHoldingBufferSize = iHoldingBufferSize;
	miElementSize = iElementSize;
	mfCompare = fCompare;
	mbOverwrite = bOverwrite;

	maaHoldingArrays.Init(mpcMalloc);
	maaHoldingArrays.Resize(iHoldingBuffers);
	for (i = 0; i < iHoldingBuffers; i++)
	{
		paHoldingArray = maaHoldingArrays.Get(i);
		paHoldingArray->Init(mpcMalloc, miElementSize);
	}

	mapiInsertionIndices = (size*)mpcMalloc->Malloc(miHoldingBufferSize * iHoldingBuffers * sizeof(size));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayBlockSorted::Read(CFileReader* pcFileReader, DataCompare fCompare)
{
	CMallocator*	pcMalloc;

	pcMalloc = gcMallocators.Read(pcFileReader);
	if (pcMalloc == NULL)
	{
		return false;
	}

	if (!ReadHeader(pcMalloc, pcFileReader, fCompare))
	{
		return false;
	}

	return maSortedArray.Read(pcFileReader);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockSorted::SetOverwrite(bool bOverwrite)
{
	mbOverwrite = bOverwrite;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayBlock* CArrayBlockSorted::GetArrayBlock(size iIndex)
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
size CArrayBlockSorted::NumElements(void)
{
	return GetSortedSize() + GetHoldingSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayBlockSorted::GetSortedSize(void)
{
	return maSortedArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayBlockSorted::GetHoldingSize(void)
{
	size			iSize;
	CArrayBlock*	paHoldingArray;
	size			i;

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
void* CArrayBlockSorted::GetInSorted(size iIndex)
{
	return maSortedArray.SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlockSorted::GetInHolding(size iArray, size iIndex)
{
	CArrayBlock*	paHoldingArray;

	paHoldingArray = maaHoldingArrays.SafeGet(iArray);
	if (!paHoldingArray)
	{
		return NULL;
	}
	
	return paHoldingArray->SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayBlockSorted::ByteSize(void)
{
	size			uiSize;
	CArrayBlock*	paHoldingArray;
	size				i;

	uiSize = 0;
	for (i = 0; i < maaHoldingArrays.NumElements(); i++)
	{
		paHoldingArray = maaHoldingArrays.Get(i);

		uiSize += paHoldingArray->ByteSize();
	}
	uiSize += maaHoldingArrays.ByteSize();
	uiSize += maSortedArray.ByteSize();
	uiSize += miHoldingBufferSize * maaHoldingArrays.NumElements() * sizeof(size);
	return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockSorted::Print(CChars* psz)
{
	size			uiSize;
	CArrayBlock*	paHoldingArray;
	size			i;

	uiSize = 0;
	for (i = 0; i < maaHoldingArrays.NumElements(); i++)
	{
		paHoldingArray = maaHoldingArrays.Get(i);

		psz->Append("Holding Array [")->Append(i)->Append("] Elements [")->Append(paHoldingArray->NumElements())->Append("]")->AppendNewLine();
	}
	psz->Append("Main Array Elements [")->Append(maSortedArray.NumElements())->Append("]")->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayBlockSorted::Dump(void)
{
	CChars sz;

	sz.Init();
	Print(&sz);
	sz.DumpKill();
}

