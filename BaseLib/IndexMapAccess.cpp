#include "IndexMapIterator.h"
#include "IndexBlock.h"
#include "IndexMapAccess.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexMapAccess::Init(CIndexBlock* pcIndex)
{
	mpcIndex = pcIndex;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexMapAccess::Kill(void)
{
	mpcIndex = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int64 CIndexMapAccess::NumElements(void)
{
	return mpcIndex->NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexMapAccess::Dump(void)
{
	mpcIndex->Dump();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CMapIterator* CIndexMapAccess::CreateIterator(void)
{
	CIndexMapIterator* pcIter;

	pcIter = NewMalloc<CIndexMapIterator>();
	pcIter->Init(mpcIndex);

	return pcIter;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CIndexMapAccess::FreeIterator(CMapIterator* pcIter)
{
	SafeKill(pcIter);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CIndexMapAccess::Put(void* pvKey, int iKeySize, void* pvData, size_t uiDataSize)
{
	void* pv;

	pv = mpcIndex->Put(pvKey, iKeySize, uiDataSize);
	if (pv)
	{
		memcpy(pv, pvData, uiDataSize);
		return pv;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CIndexMapAccess::Get(void* pvKey, int iKeySize, size_t* puiDataSize)
{
	bool	bResult;
	void* pvData;

	bResult = mpcIndex->Get(pvKey, iKeySize, &pvData, (int*)puiDataSize);
	if (bResult)
	{
		return pvData;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size_t CIndexMapAccess::DataSize(void* pvKey, int iKeySize)
{
	return mpcIndex->DataSize(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexMapAccess::Remove(void* pvKey, int iKeySize)
{
	return mpcIndex->Remove(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CIndexMapAccess::Has(void* pvKey, int iKeySize)
{
	return mpcIndex->HasKey(pvKey, iKeySize);
}

