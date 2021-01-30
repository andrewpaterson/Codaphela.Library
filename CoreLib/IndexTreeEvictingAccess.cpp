#include "IndexTreeEvictingAccess.h"
#include "IndexTreeFileIterator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvictingAccess::Init(CIndexTreeEvicting* pcTree)
{
	CIndexTreeAccess::Init();
	mpcTree = pcTree;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvictingAccess::Kill(void)
{
	mpcTree = NULL;
	return CIndexTreeAccess::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvictingAccess::Dump(void)
{
	mpcTree->Dump();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvictingAccess::Flush(void)
{
	if (!mpcTree->IsWriteThrough())
	{
		return mpcTree->Flush();
	}
	else
	{
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexTreeEvictingAccess::NumElements(void)
{
	return mpcTree->NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvictingAccess::ValidateIndex(void)
{
	if (mpcTree->IsFlushed())
	{
		return mpcTree->ValidateIndexTree();
	}
	else
	{
		return TRUE;
	}

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvictingAccess::Put(void* pvKey, int iKeySize, void* pvData, size_t iDataSize)
{
	return mpcTree->Put(pvKey, iKeySize, pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvictingAccess::Get(void* pvKey, int iKeySize, void* pvData, size_t* piDataSize)
{
	size_t	iDataSize;
	BOOL	bResult;

	bResult = mpcTree->Get(pvKey, iKeySize, pvData, &iDataSize);

	SafeAssign(piDataSize, iDataSize);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvictingAccess::Remove(void* pvKey, int iKeySize)
{
	return mpcTree->Remove(pvKey, iKeySize);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeEvictingAccess::DataSize(void* pvKey, int iKeySize)
{
	return mpcTree->GetDataSize(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvictingAccess::Has(void* pvKey, int iKeySize)
{
	return mpcTree->HasKey(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvictingAccess::Evict(void* pvKey, int iKeySize)
{
	return mpcTree->Evict(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvictingAccess::Flush(void* pvKey, int iKeySize)
{
	return mpcTree->Flush(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeIterator* CIndexTreeEvictingAccess::CreateIterator(void)
{
	CIndexTreeFileIterator* pcIter;

	pcIter = NewMalloc<CIndexTreeFileIterator>();
	pcIter->Init(mpcTree->GetIndexTree());

	return pcIter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvictingAccess::FreeIterator(CIndexTreeIterator* pcIter)
{
	SafeKill(pcIter);
}

