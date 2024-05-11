#include "IndexTreeFileAccess.h"
#include "IndexTreeFileIterator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFileAccess::Init(CIndexTreeFile* pcTree)
{
	CIndexAccess::Init();
	mpcTree = pcTree;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFileAccess::Kill(void)
{
	mpcTree = NULL;
	return CIndexAccess::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileAccess::Dump(void)
{
	mpcTree->Dump();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFileAccess::Flush(void)
{
	if (!mpcTree->IsWriteThrough())
	{
		return mpcTree->Flush();
	}
	else
	{
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexTreeFileAccess::NumElements(void)
{
	return mpcTree->NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFileAccess::ValidateIndex(void)
{
	if (mpcTree->IsFlushed())
	{
		return mpcTree->ValidateIndexTree();
	}
	else
	{
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFileAccess::Put(void* pvKey, size iKeySize, void* pvData, size iDataSize)
{
	return mpcTree->Put(pvKey, iKeySize, pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFileAccess::Get(void* pvKey, size iKeySize, void* pvData, size* piDataSize, size uiMaxDataSize)
{
	size	iDataSize;
	bool	bResult;

	bResult = mpcTree->Get(pvKey, iKeySize, pvData, &iDataSize, uiMaxDataSize);

	SafeAssign(piDataSize, iDataSize);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFileAccess::Remove(void* pvKey, size iKeySize)
{
	return mpcTree->Remove(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeFileAccess::DataSize(void* pvKey, size iKeySize)
{
	return mpcTree->GetDataSize(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFileAccess::Has(void* pvKey, size iKeySize)
{
	return mpcTree->HasKey(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFileAccess::Flush(void* pvKey, size iKeySize)
{
	return mpcTree->Flush(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFileAccess::Evict(void* pvKey, size iKeySize)
{
	return mpcTree->Evict(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeIterator* CIndexTreeFileAccess::CreateIterator(void)
{
	CIndexTreeFileIterator* pcIter;

	pcIter = NewMalloc<CIndexTreeFileIterator>();
	pcIter->Init(mpcTree);

	return pcIter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFileAccess::FreeIterator(CIndexTreeIterator* pcIter)
{
	SafeKill(pcIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeFile* CIndexTreeFileAccess::GetTree(void)
{
	return mpcTree;
}

