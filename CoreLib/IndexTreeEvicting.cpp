#include "BaseLib/Logger.h"
#include "BaseLib/StackMemory.h"
#include "IndexTreeEvicting.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Init(CDurableFileController* pcDurableFileControl, size_t uiCutoff, CIndexTreeEvictionCallback* pcEvictionCallback, CIndexTreeEvictionStrategy* pcEvictionStrategy, CIndexTreeFileCallback* pcWriterCallback)
{
	return Init(pcDurableFileControl, uiCutoff, pcEvictionCallback, pcEvictionStrategy, pcWriterCallback, &gcSystemAllocator, TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Init(CDurableFileController* pcDurableFileControl, size_t uiCutoff, CIndexTreeEvictionCallback* pcEvictionCallback, CIndexTreeEvictionStrategy* pcEvictionStrategy, CIndexTreeFileCallback* pcWriterCallback, BOOL bWriteThrough)
{
	return Init(pcDurableFileControl, uiCutoff, pcEvictionCallback, pcEvictionStrategy, pcWriterCallback, &gcSystemAllocator, bWriteThrough);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Init(CDurableFileController* pcDurableFileControl, size_t uiCutoff, CIndexTreeEvictionCallback* pcEvictionCallback, CIndexTreeEvictionStrategy* pcEvictionStrategy, CIndexTreeFileCallback* pcWriterCallback, CMallocator* pcMalloc, BOOL bWriteThrough)
{
	BOOL	bResult;
	mpcEvictionStrategy = pcEvictionStrategy;
	mpcEvictionCallback = pcEvictionCallback;
	muiCutoff = uiCutoff;

	bResult = mcIndexTree.Init(pcDurableFileControl, pcWriterCallback, pcMalloc, bWriteThrough);
	mpcEvictionStrategy->SetIndexTree(this);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Kill(void)
{
	BOOL bResult;

	bResult = mcIndexTree.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Get(void* pvKey, int iKeySize, void* pvObject, unsigned short* puiDataSize)
{
	BOOL	bResult;

	bResult = mcIndexTree.Get(pvKey, iKeySize, pvObject, puiDataSize);
	PotentiallyEvict(pvKey, iKeySize);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Put(void* pvKey, int iKeySize, void* pvObject, unsigned short uiDataSize)
{
	BOOL	bResult;

	bResult = mcIndexTree.Put(pvKey, iKeySize, pvObject, uiDataSize);
	PotentiallyEvict(pvKey, iKeySize);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::PutWithoutEviction(void* pvKey, int iKeySize, void* pvObject, unsigned short uiDataSize)
{
	BOOL	bResult;

	bResult = mcIndexTree.Put(pvKey, iKeySize, pvObject, uiDataSize);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Remove(void* pvKey, int iKeySize)
{
	BOOL	bResult;

	bResult = mcIndexTree.Remove(pvKey, iKeySize);
	PotentiallyEvict(pvKey, iKeySize);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::HasKey(void* pvKey, int iKeySize)
{
	BOOL	bResult;

	bResult = mcIndexTree.HasKey(pvKey, iKeySize);
	PotentiallyEvict(pvKey, iKeySize);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned short CIndexTreeEvicting::ObjectSize(void* pvKey, int iKeySize)
{
	unsigned short	uiSize;

	uiSize = mcIndexTree.ObjectSize(pvKey, iKeySize);
	PotentiallyEvict(pvKey, iKeySize);
	return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvicting::PotentiallyEvict(void* pvKey, int iKeySize)
{
	size_t				uiLastSize;
	size_t				uiSize;
	CIndexTreeNodeFile*	pcDontEvict;
	BOOL				bResult;
	CChars				sz;

	mcIndexTree.GetNode(pvKey, iKeySize);
	uiLastSize = 0;
	pcDontEvict = mcIndexTree.GetMemoryNode(pvKey, iKeySize);

	for (;;)
	{
		uiSize = mcIndexTree.GetSystemMemorySize();
		if ((uiSize <= muiCutoff) || (uiSize == uiLastSize))
		{
			return;
		}

		bResult = mpcEvictionStrategy->Run(pcDontEvict);
		uiLastSize = uiSize;
		if (!bResult)
		{
			sz.Init();
			sz.AppendData2((const char*)pvKey, iKeySize);
			gcLogger.Error2(__METHOD__, " Could not evict key [", sz.Text(), "].  Tree size [", IntToString(uiSize), "] could not be reduced below cache size [", IntToString(muiCutoff), "].", NULL);
			sz.Kill();
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Evict(void* pvKey, int iKeySize)
{
	CIndexTreeNodeFile* pcNode;

	pcNode = mcIndexTree.GetMemoryNode(pvKey, iKeySize);
	if (pcNode)
	{
		return Evict(pcNode);
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
BOOL CIndexTreeEvicting::EvictNodeCallback(CIndexTreeNodeFile* pcNode)
{
	CStackMemory<>		cStack;
	unsigned char*		pvMem;
	int					iKeySize;
	BOOL				bResult;
	void*				pvData;
	unsigned short		uiDataSize;

	iKeySize = mcIndexTree.GetNodeKeySize(pcNode);
	pvMem = (unsigned char*)cStack.Init(iKeySize + 1);
	mcIndexTree.GetNodeKey(pcNode, pvMem, iKeySize + 1);
	pvMem[iKeySize] = 0;
	pvData = pcNode->GetObjectPtr();
	uiDataSize = pcNode->ObjectSize();

	bResult = mpcEvictionCallback->NodeEvicted(&mcIndexTree, pvMem, iKeySize, pvData, uiDataSize);
	
	cStack.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::IsWriteThrough(void)
{
	return mcIndexTree.IsWriteThrough();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Flush(void)
{
	return mcIndexTree.Flush();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexedFiles* CIndexTreeEvicting::GetIndexFiles(void)
{
	return mcIndexTree.GetIndexFiles();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeEvicting::NumNodes(void)
{
	return mcIndexTree.NumNodes();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeEvicting::NumMemoryNodes(void)
{
	return mcIndexTree.NumMemoryNodes();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeEvicting::NumElements(void)
{
	return mcIndexTree.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeEvicting::NumMemoryElements(void)
{
	return mcIndexTree.NumMemoryElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvicting::DebugKey(void* pvKey, int iKeySize, BOOL bSkipRoot)
{
	return mcIndexTree.DebugKey(pvKey, iKeySize, bSkipRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvicting::Dump(void)
{
	return mcIndexTree.Dump();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeEvicting::GetRoot(void)
{
	return mcIndexTree.GetRoot();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeEvicting::GetMemoryNode(void* pvKey, int iKeySize)
{
	return mcIndexTree.GetMemoryNode(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Evict(CIndexTreeNodeFile* pcNode)
{
	BOOL bEvict;

	if (pcNode->HasObject())
	{
		bEvict = EvictNodeCallback(pcNode);
	}
	else
	{
		bEvict = TRUE;
	}

	if (bEvict)
	{
		return mcIndexTree.Evict(pcNode);
	}
	else
	{
		return FALSE;  //If the node couldn't be evicted shouldn't this return FALSE?
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeEvicting::GetSystemMemorySize(void)
{
	return mcIndexTree.GetSystemMemorySize();
}

