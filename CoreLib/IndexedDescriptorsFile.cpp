#include "BaseLib/Logger.h"
#include "IndexedDataCommon.h"
#include "IndexedDescriptorsFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDescriptorsFile::Init(CIndexedDataCommon* pcIndexedData, CDurableFileController* pcDurableFileController, char* szSubDirectory, size_t uiCutoff, EIndexWriteThrough eWriteThrough)
{
	Init(mpcIndexedData, pcDurableFileController, szSubDirectory, uiCutoff, eWriteThrough, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDescriptorsFile::Init(CIndexedDataCommon* pcIndexedData, CDurableFileController* pcDurableFileController, char* szSubDirectory, size_t uiCutoff, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback)
{
	CIndexTreeEvictionCallback*	pcCallback;

	mpcIndexedData = pcIndexedData;
	if (pcEvictionCallback)
	{
		mcEvictionCallbackWrapper.Init(pcEvictionCallback, this);
		pcCallback = &mcEvictionCallbackWrapper;
	}
	else
	{
		mcEvictionCallbackWrapper.Init(NULL, NULL);
		pcCallback = this;
	}
	mcIndexTree.Init(pcDurableFileController, szSubDirectory, uiCutoff, pcCallback, &mcEvictionStrategy, this, eWriteThrough, IKR_Yes);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::Kill(void)
{
	BOOL bResult;

	bResult = mcIndexTree.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::Remove(OIndex oi)
{
	return mcIndexTree.Remove(&oi, sizeof(OIndex));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi)
{
	return mcIndexTree.Get(&oi, sizeof(OIndex), pcDescriptor, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi, BOOL bNoEviction)
{
	if (!bNoEviction)
	{
		return mcIndexTree.Get(&oi, sizeof(OIndex), pcDescriptor, NULL);
	}
	else
	{
		return mcIndexTree.GetWithoutEviction(&oi, sizeof(OIndex), pcDescriptor, NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::Set(CIndexedDataDescriptor* pcDescriptor, OIndex oi)
{
	return mcIndexTree.Put(&oi, sizeof(OIndex), pcDescriptor, sizeof(CIndexedDataDescriptor));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::Set(CIndexedDataDescriptor* pcDescriptor, OIndex oi, BOOL bNoEviction)
{
	if (!bNoEviction)
	{
		return mcIndexTree.Put(&oi, sizeof(OIndex), pcDescriptor, sizeof(CIndexedDataDescriptor));
	}
	else
	{
		return mcIndexTree.PutWithoutEviction(&oi, sizeof(OIndex), pcDescriptor, sizeof(CIndexedDataDescriptor));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::SetCache(void* pvCache, unsigned int uiDataSize, OIndex oi)
{
	CIndexTreeNode*			pcNode;
	CIndexedDataDescriptor*	pcDescriptor;

	pcNode = mcIndexTree.GetMemoryNode(&oi, sizeof(OIndex));
	if (pcNode)
	{
		pcDescriptor = (CIndexedDataDescriptor*)pcNode->GetDataPtr();
		pcDescriptor->Cache(pvCache, uiDataSize);
		return TRUE;
	}
	return FALSE;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedDescriptorsFile::NumElements(void)
{
	return NumIndices();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedDescriptorsFile::NumIndices(void)
{
	return mcIndexTree.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedDescriptorsFile::NumIndicesCached(void)
{
	return mcIndexTree.NumMemoryElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedDescriptorsFile::NumDatas(void)
{
	return mpcIndexedData->NumDatas();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::Flush(void)
{
	return mcIndexTree.Flush();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::IsFlushed(void)
{
	return mcIndexTree.IsFlushed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::GetIfInMemory(CIndexedDataDescriptor* pcDescriptor, OIndex oi)
{
	CIndexTreeNodeFile*		pcNode;
	CIndexedDataDescriptor*	pcReturn;

	pcNode = mcIndexTree.GetMemoryNode(&oi, sizeof(OIndex));
	if (pcNode)
	{
		pcReturn = (CIndexedDataDescriptor*)pcNode->GetDataPtr();
		if (pcDescriptor)
		{
			memcpy(pcDescriptor, pcReturn, sizeof(CIndexedDataDescriptor));
		}
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::Evict(OIndex oi)
{
	return mcIndexTree.Evict(&oi, sizeof(OIndex));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::Flush(OIndex oi)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::IndexTreeNodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
	OIndex	oi;

	if (iKeySize != sizeof(OIndex))
	{
		return gcLogger.Error2(__METHOD__, " Key evicted was not an OIndex.", NULL);
	}

	oi = *((OIndex*)pvKey);
	return mpcIndexedData->EvictData(oi, (CIndexedDataDescriptor*)pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexedDescriptorsFile::GetSystemMemorySize(void)
{
	return mcIndexTree.GetSystemMemorySize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexedDescriptorsFile::GetRootFlags(void)
{
	return mcIndexTree.GetRootFlags();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDescriptorsFile::Dump(void)
{
	mcIndexTree.Dump();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned short CIndexedDescriptorsFile::IndexTreeDataSize(unsigned short uiSourceSize)
{
	if (uiSourceSize != 0)
	{
		return sizeof(SIndexedFileDataDescriptor);
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::IndexTreeWriteData(void* pvDataBuffer, void* pvSource, int iFileDataSize, unsigned short uiSourceDataSize)
{
	memcpy_fast(pvDataBuffer, pvSource, iFileDataSize);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::IndexTreeReadData(void* pvDest, void* pvDataBuffer, unsigned short uiDestDataSize, int iFileDataSize)
{
	memset_fast(pvDest, 0, uiDestDataSize);
	memcpy_fast(pvDest, pvDataBuffer, iFileDataSize);
	return TRUE;
}

