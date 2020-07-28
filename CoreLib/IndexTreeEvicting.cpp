#include "BaseLib/Logger.h"
#include "BaseLib/StackMemory.h"
#include "BaseLib/DebugOutput.h"
#include "IndexTreeFileDefaultDataCallback.h"
#include "IndexTreeEvicting.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CIndexTreeEvictionStrategy* pcEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, EIndexKeyReverse eKeyReverse)
{
	return Init(pcDurableFileControl, szSubDirectory, uiCutoff, pcIndexTreeEvictionCallback, pcEvictionStrategy, pcWriterCallback, &gcSystemAllocator, IWT_Yes, eKeyReverse);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CIndexTreeEvictionStrategy* pcEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	return Init(pcDurableFileControl, szSubDirectory, uiCutoff, pcIndexTreeEvictionCallback, pcEvictionStrategy, pcWriterCallback, &gcSystemAllocator, eWriteThrough, eKeyReverse);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CIndexTreeEvictionStrategy* pcEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, CMallocator* pcMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	BOOL	bResult;

	mpcEvictionStrategy = pcEvictionStrategy;
	mpcIndexTreeEvictionCallback = pcIndexTreeEvictionCallback;

	if (pcWriterCallback == NULL)
	{
		pcWriterCallback = &gcIndexTreeFileDefaultCallback;
	}

	muiCutoff = uiCutoff;

	bResult = mcIndexTree.Init(pcDurableFileControl, szSubDirectory, pcWriterCallback, pcMalloc, eWriteThrough, eKeyReverse);

	if (mpcEvictionStrategy)
	{
		mpcEvictionStrategy->SetIndexTree(this);
	}
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
BOOL CIndexTreeEvicting::Get(void* pvKey, int iKeySize, void* pvObject, int* piDataSize)
{
	BOOL	bFound;
	int		iEvicted;

	bFound = mcIndexTree.Get(pvKey, iKeySize, pvObject, piDataSize);
	if (bFound)
	{
		iEvicted = PotentiallyEvict(pvKey, iKeySize);
	}
	return bFound;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::GetWithoutEviction(void* pvKey, int iKeySize, void* pvObject, int* piDataSize)
{
	BOOL	bResult;

	bResult = mcIndexTree.Get(pvKey, iKeySize, pvObject, piDataSize);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Put(void* pvKey, int iKeySize, void* pvObject, int iDataSize)
{
	BOOL	bResult;
	int		iEvicted;

	bResult = mcIndexTree.Put(pvKey, iKeySize, pvObject, iDataSize);
	iEvicted = PotentiallyEvict(pvKey, iKeySize);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::PutWithoutEviction(void* pvKey, int iKeySize, void* pvObject, int iDataSize)
{
	BOOL	bResult;

	bResult = mcIndexTree.Put(pvKey, iKeySize, pvObject, iDataSize);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Remove(void* pvKey, int iKeySize)
{
	BOOL	bFound;
	int		iEvicted;

	bFound = mcIndexTree.Remove(pvKey, iKeySize);
	if (bFound)
	{
		iEvicted = PotentiallyEvict(pvKey, iKeySize);
	}
	return bFound;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::HasKey(void* pvKey, int iKeySize)
{
	BOOL	bFound;
	BOOL	bAnyEvicted;

	bFound = mcIndexTree.HasKey(pvKey, iKeySize);
	if (bFound)
	{
		bAnyEvicted = PotentiallyEvict(pvKey, iKeySize);
	}
	return bFound;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned short CIndexTreeEvicting::GetDataSize(void* pvKey, int iKeySize)
{
	unsigned short	uiSize;
	int				iEvicted;

	uiSize = mcIndexTree.GetDataSize(pvKey, iKeySize);
	iEvicted = PotentiallyEvict(pvKey, iKeySize);
	return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeEvicting::PotentiallyEvict(void* pvKey, int iKeySize)
{
	size_t				uiLastSize;
	size_t				uiSize;
	CIndexTreeNodeFile*	pcDontEvict;
	BOOL				bResult;
	CChars				sz;
	int					iEvicted;

	mcIndexTree.GetNode(pvKey, iKeySize);
	uiLastSize = 0;
	pcDontEvict = mcIndexTree.GetMemoryNode(pvKey, iKeySize);
	iEvicted = 0;

	if (mpcEvictionStrategy)
	{
		for (;;)
		{
			uiSize = mcIndexTree.GetSystemMemorySize();
			if ((uiSize <= muiCutoff) || (uiSize == uiLastSize))
			{
				return iEvicted;
			}

			bResult = mpcEvictionStrategy->Run(pcDontEvict);
			uiLastSize = uiSize;
			if (!bResult)
			{
				sz.Init();
				sz.AppendData2((const char*)pvKey, iKeySize);
				gcLogger.Error2(__METHOD__, " Could not evict key [", sz.Text(), "].  Tree size [", IntToString(uiSize), "] could not be reduced below cache size [", IntToString(muiCutoff), "].", NULL);
				sz.Kill();
				return iEvicted;
			}

			iEvicted++;
		}
	}
	else
	{
		uiSize = mcIndexTree.GetSystemMemorySize();
		if (uiSize <= muiCutoff)
		{
			return 0;
		}
		else
		{
			sz.Init();
			sz.AppendData2((const char*)pvKey, iKeySize);
			gcLogger.Error2(__METHOD__, " Could not evict key [", sz.Text(), "].  Tree size [", IntToString(uiSize), "] could not be reduced below cache size [", IntToString(muiCutoff), "].", NULL);
			sz.Kill();
			return 0;
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
		return EvictNode(pcNode);
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
BOOL CIndexTreeEvicting::EvictNodeWithObject(CIndexTreeNodeFile* pcNode)
{
	CStackMemory<>		cStack;
	char*				pvMem;
	int					iKeySize;
	BOOL				bResult;
	void*				pvData;
	unsigned short		uiDataSize;

	if (mpcIndexTreeEvictionCallback)
	{
		iKeySize = mcIndexTree.GetNodeKeySize(pcNode);
		pvMem = (char*)cStack.Init(iKeySize + 1);
		mcIndexTree.GetNodeKey(pcNode, pvMem, iKeySize + 1);
		pvMem[iKeySize] = 0;
		pvData = pcNode->GetDataPtr();
		uiDataSize = pcNode->GetDataSize();

		//xxx;
		// mpcIndexTreeEvictionCallback->IndexTreeNodeEvicted calls 
        //   CIndexedDescriptorsFile::IndexTreeNodeEvicted 
		//   CIndexedData::EvictData
		//   CIndexedFilesEvicting::Evict
		//     mpcEvictionCallback->DescriptorEvicted
		//       CIndexedData::DescriptorEvicted (which tries to evict the node again)
		//         mpcIndexedDataEvictionCallback->IndexEvicted
		//    

		bResult = mpcIndexTreeEvictionCallback->IndexTreeNodeEvicted(pvMem, iKeySize, pvData, uiDataSize);

		cStack.Kill();
		return bResult;
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
BOOL CIndexTreeEvicting::Flush(void* pvKey, int iKeySize)
{
	return mcIndexTree.Flush(pvKey, iKeySize);
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
BOOL CIndexTreeEvicting::IsFlushed(void)
{
	return mcIndexTree.IsFlushed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::ValidateIndexTree(void)
{
	return mcIndexTree.ValidateIndexTree();
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
CIndexTreeFile* CIndexTreeEvicting::GetIndexTree(void)
{
	return &mcIndexTree;
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
void CIndexTreeEvicting::DebugKey(CChars* pszDest, void* pvKey, int iKeySize, BOOL bSkipRoot, BOOL bShowFlags, BOOL bShowSize, BOOL bKeyAlreadyReversed)
{
	return mcIndexTree.DebugKey(pszDest, pvKey, iKeySize, bSkipRoot, bShowFlags, bShowSize, bKeyAlreadyReversed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvicting::Dump(void)
{
	CChars				sz;

	sz.Init();
	Print(&sz, TRUE, TRUE);
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvicting::Print(CChars* pszDest, BOOL bShowFlags, BOOL bShowSize)
{
	CChars				sz;

	sz.Init();
	sz.Append("= [IndexTreeEvicting]  ");
	if (bShowSize)
	{
		sz.Append("[");
		sz.Append((unsigned long long int)mcIndexTree.GetUserMemorySize());
		sz.Append(" / ");
		sz.Append((unsigned long long int)muiCutoff);
		sz.Append("] ");
	}

	sz.Append('=', mcIndexTree.GetRoot()->NumIndexes() * 2 - sz.Length() + 20);
	pszDest->AppendNewLine(sz);
	sz.Kill();

	mcIndexTree.PrintChildren(pszDest, bShowFlags, bShowSize);
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
BOOL CIndexTreeEvicting::EvictNode(CIndexTreeNodeFile* pcNode)
{
	BOOL bEvict;

	if (pcNode->HasData())
	{
		bEvict = EvictNodeWithObject(pcNode);
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeEvicting::GetRootFlags(void)
{
	return mcIndexTree.GetRoot()->GetFlags();
}

