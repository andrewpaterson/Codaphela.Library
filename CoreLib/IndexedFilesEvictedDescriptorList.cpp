#include "IndexedCache.h"
#include "IndexedFilesEvictedDescriptorList.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFilesEvictedDescriptorList::Init(void)
{
	mcDatas.Init();
	mcDescriptors.Init(CompareLong);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFilesEvictedDescriptorList::Kill(void)
{
	mcDescriptors.Kill();
	mcDatas.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexedDataDescriptor*	CIndexedFilesEvictedDescriptorList::AddDescriptor(OIndex oi, unsigned int uiDataSize)
{
	CIndexedDataDescriptor*	pcResult;

	pcResult = mcDescriptors.Put(&oi);
	pcResult->Init(uiDataSize);
	return pcResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvictedDescriptorList::DescriptorsEvicted(CArrayVoidPtr* papsEvictedIndexedCacheDescriptors)
{
	int							i;
	SIndexedCacheDescriptor*	psDesc;
	int							iNumElements;
	void*						pvData;

	iNumElements = papsEvictedIndexedCacheDescriptors->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		psDesc = (SIndexedCacheDescriptor*)papsEvictedIndexedCacheDescriptors->GetPtr(i);
		if (psDesc != NULL)
		{
			pvData = RemapSinglePointer(psDesc, sizeof(SIndexedCacheDescriptor));
			mcDatas.Add(pvData, psDesc->iDataSize);
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFilesEvictedDescriptorList::GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
{
	CIndexedDataDescriptor* pcResult;

	pcResult = mcDescriptors.Get(&oi);
	if (pcResult)
	{
		if (pcDescriptor)
		{
			memcpy_fast(pcDescriptor, pcResult, sizeof(CIndexedDataDescriptor));
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
BOOL CIndexedFilesEvictedDescriptorList::SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
{
	return mcDescriptors.Put(&oi, pcDescriptor);
}

