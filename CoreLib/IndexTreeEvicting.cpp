#include "BaseLib/Logger.h"
#include "BaseLib/StackMemory.h"
#include "BaseLib/DebugOutput.h"
#include "IndexTreeFileDefaultDataCallback.h"
#include "IndexTreeEvicting.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, EIndexKeyReverse eKeyReverse)
{
	return Init(pcDurableFileControl, szSubDirectory, uiCutoff, pcIndexTreeEvictionCallback, cEvictionStrategy, pcWriterCallback, LifeLocal<CMallocator>(&gcSystemAllocator), IWT_Yes, eKeyReverse);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CLifeInit<CIndexTreeDataOrderer> cDataOrderer)
{
	return Init(pcDurableFileControl, szSubDirectory, uiCutoff, pcIndexTreeEvictionCallback, cEvictionStrategy, pcWriterCallback, LifeLocal<CMallocator>(&gcSystemAllocator), eWriteThrough, eKeyReverse, cDataOrderer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	return Init(pcDurableFileControl, szSubDirectory, uiCutoff, pcIndexTreeEvictionCallback, cEvictionStrategy, pcWriterCallback, LifeLocal<CMallocator>(&gcSystemAllocator), eWriteThrough, eKeyReverse);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, CLifeInit<CMallocator> cMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	return Init(pcDurableFileControl, szSubDirectory, uiCutoff, pcIndexTreeEvictionCallback, cEvictionStrategy, pcWriterCallback, cMalloc, eWriteThrough, eKeyReverse, LifeNull<CIndexTreeDataOrderer>());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, CLifeInit<CMallocator> cMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CLifeInit<CIndexTreeDataOrderer> cDataOrderer)
{
	BOOL	bResult;

	cEvictionStrategy.ConfigureLife(&mcEvictionStrategy, &mpcEvictionStrategy);
	mpcIndexTreeEvictionCallback = pcIndexTreeEvictionCallback;

	if (pcWriterCallback == NULL)
	{
		pcWriterCallback = &gcIndexTreeFileDefaultCallback;
	}

	muiCutoff = uiCutoff;

	bResult = mcIndexTree.Init(pcDurableFileControl, szSubDirectory, pcWriterCallback, cMalloc, eWriteThrough, eKeyReverse, cDataOrderer);

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
BOOL CIndexTreeEvicting::Get(void* pvKey, int iKeySize, void* pvData, size_t* piDataSize, size_t uiMaxDataSize)
{
	BOOL	bFound;
	int		iEvicted;

	bFound = mcIndexTree.Get(pvKey, iKeySize, pvData, piDataSize, uiMaxDataSize);
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
BOOL CIndexTreeEvicting::GetWithoutEviction(void* pvKey, int iKeySize, void* pvData, size_t* piDataSize, size_t uiMaxDataSize)
{
	BOOL	bResult;

	bResult = mcIndexTree.Get(pvKey, iKeySize, pvData, piDataSize, uiMaxDataSize);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Put(void* pvKey, int iKeySize, void* pvData, size_t iDataSize)
{
	BOOL	bResult;
	int		iEvicted;

	bResult = mcIndexTree.Put(pvKey, iKeySize, pvData, iDataSize);
	iEvicted = PotentiallyEvict(pvKey, iKeySize);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::PutWithoutEviction(void* pvKey, int iKeySize, void* pvData, size_t iDataSize)
{
	BOOL	bResult;

	bResult = mcIndexTree.Put(pvKey, iKeySize, pvData, iDataSize);
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
uint16 CIndexTreeEvicting::GetDataSize(void* pvKey, int iKeySize)
{
	uint16	uiSize;
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
				gcLogger.Error2(__METHOD__, " Could not evict anything while touching key [", sz.Text(), "].  Tree size [", IntToString(uiSize), "] could not be reduced below cache size [", IntToString(muiCutoff), "].", NULL);
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
			gcLogger.Error2(__METHOD__, " Could not anything while touching evict key [", sz.Text(), "].  Tree size [", IntToString(uiSize), "] could not be reduced below cache size [", IntToString(muiCutoff), "].", NULL);
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
	uint16		uiDataSize;

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
void CIndexTreeEvicting::SetDiagnosticCallback(CIndexTreeFileDiagnosticCallback* pcCallback)
{
	return mcIndexTree.SetDiagnosticCallback(pcCallback);
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
BOOL CIndexTreeEvicting::ValidateIndexTree(BOOL bReadNodes)
{
	return mcIndexTree.ValidateIndexTree(bReadNodes);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::HasDiagnosticCallback(void)
{
	return mcIndexTree.HasDiagnosticCallback();
}


CIndexTreeDataOrderer* CIndexTreeEvicting::GetDataOrderer(void)
{
	return mcIndexTree.GetDataOrderer();
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
int CIndexTreeEvicting::NumMemoryElements(size_t iSize)
{
	return mcIndexTree.NumMemoryElements(iSize);
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
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeEvicting::GetNodeKey(CIndexTreeNodeFile* pcNode, char* pvDestKey, int iDestKeySize)
{
	return mcIndexTree.GetNodeKey(pcNode, pvDestKey, iDestKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeEvicting::GetNodeKeySize(CIndexTreeNodeFile* pcNode)
{
	return mcIndexTree.GetNodeKeySize(pcNode);
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
size_t CIndexTreeEvicting::GetUserMemorySize(void)
{
	return mcIndexTree.GetUserMemorySize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeEvicting::GetRootFlags(void)
{
	return mcIndexTree.GetRoot()->GetFlags();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::StartIteration(SIndexTreeFileIterator* psIterator, void* pvKey, int* piKeySize, int iMaxKeySize, void* pvData, size_t* piDataSize, size_t iMaxDataSize)
{
	BOOL	bResult;
	int		iEvicted;

	bResult = mcIndexTree.StartIteration(psIterator, pvKey, piKeySize, iMaxKeySize, pvData, piDataSize, iMaxDataSize);
	if (bResult)
	{
		iEvicted = PotentiallyEvict(psIterator->pvKey, psIterator->iKeyLength);
	}
	else
	{
		iEvicted = PotentiallyEvict(NULL, 0);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvicting::Iterate(SIndexTreeFileIterator* psIterator, void* pvKey, int* piKeySize, int iMaxKeySize, void* pvData, size_t* piDataSize, size_t iMaxDataSize)
{

	BOOL	bResult;
	int		iEvicted;

	bResult = mcIndexTree.Iterate(psIterator, pvKey, piKeySize, iMaxKeySize, pvData, piDataSize, iMaxDataSize);
	if (bResult)
	{
		iEvicted = PotentiallyEvict(psIterator->pvKey, psIterator->iKeyLength);
	}
	else
	{
		iEvicted = PotentiallyEvict(NULL, 0);
	}
	return bResult;
}

