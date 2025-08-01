#include "Logger.h"
#include "StackMemory.h"
#include "DebugOutput.h"
#include "IndexTreeFileDefaultDataCallback.h"
#include "IndexTreeEvicting.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeEvicting::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, EIndexKeyReverse eKeyReverse)
{
	return Init(pcDurableFileControl, szSubDirectory, uiCutoff, pcIndexTreeEvictionCallback, cEvictionStrategy, pcWriterCallback, LifeLocal<CMallocator>(&gcSystemAllocator), IWT_Yes, eKeyReverse);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeEvicting::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CLifeInit<CIndexTreeDataOrderer> cDataOrderer)
{
	return Init(pcDurableFileControl, szSubDirectory, uiCutoff, pcIndexTreeEvictionCallback, cEvictionStrategy, pcWriterCallback, LifeLocal<CMallocator>(&gcSystemAllocator), eWriteThrough, eKeyReverse, cDataOrderer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeEvicting::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	return Init(pcDurableFileControl, szSubDirectory, uiCutoff, pcIndexTreeEvictionCallback, cEvictionStrategy, pcWriterCallback, LifeLocal<CMallocator>(&gcSystemAllocator), eWriteThrough, eKeyReverse);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeEvicting::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, CLifeInit<CMallocator> cMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	return Init(pcDurableFileControl, szSubDirectory, uiCutoff, pcIndexTreeEvictionCallback, cEvictionStrategy, pcWriterCallback, cMalloc, eWriteThrough, eKeyReverse, LifeNull<CIndexTreeDataOrderer>());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeEvicting::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, CLifeInit<CMallocator> cMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CLifeInit<CIndexTreeDataOrderer> cDataOrderer)
{
	bool	bResult;

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
bool CIndexTreeEvicting::Kill(void)
{
	bool bResult;

	mcEvictionStrategy.Kill();
	bResult = mcIndexTree.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeEvicting::Get(uint8* pvKey, size iKeySize, void* pvData, size* piDataSize, size uiMaxDataSize)
{
	bool	bFound;
	size	iEvicted;

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
bool CIndexTreeEvicting::GetWithoutEviction(uint8* pvKey, size iKeySize, void* pvData, size* piDataSize, size uiMaxDataSize)
{
	bool	bResult;

	bResult = mcIndexTree.Get(pvKey, iKeySize, pvData, piDataSize, uiMaxDataSize);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeEvicting::Put(uint8* pvKey, size iKeySize, void* pvData, size iDataSize)
{
	bool	bResult;
	size	iEvicted;

	bResult = mcIndexTree.Put(pvKey, iKeySize, pvData, iDataSize);
	iEvicted = PotentiallyEvict(pvKey, iKeySize);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeEvicting::PutWithoutEviction(uint8* pvKey, size iKeySize, void* pvData, size iDataSize)
{
	bool	bResult;

	bResult = mcIndexTree.Put(pvKey, iKeySize, pvData, iDataSize);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeEvicting::Remove(uint8* pvKey, size iKeySize)
{
	bool	bFound;
	size		iEvicted;

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
bool CIndexTreeEvicting::HasKey(uint8* pvKey, size iKeySize)
{
	bool	bFound;
	bool	bAnyEvicted;

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
size CIndexTreeEvicting::GetDataSize(uint8* pvKey, size iKeySize)
{
	size	uiSize;
	size	iEvicted;

	uiSize = mcIndexTree.GetDataSize(pvKey, iKeySize);
	iEvicted = PotentiallyEvict(pvKey, iKeySize);
	return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeEvicting::PotentiallyEvict(uint8* pvKey, size iKeySize)
{
	size					uiLastSize;
	size					uiSize;
	CIndexTreeNodeFile*		pcDontEvict;
	bool					bResult;
	CChars					sz;
	size					iEvicted;

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
bool CIndexTreeEvicting::Evict(uint8* pvKey, size iKeySize)
{
	CIndexTreeNodeFile* pcNode;

	pcNode = mcIndexTree.GetMemoryNode(pvKey, iKeySize);
	if (pcNode)
	{
		return EvictNode(pcNode);
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
bool CIndexTreeEvicting::EvictNodeWithObject(CIndexTreeNodeFile* pcNode)
{
	CStackMemory<>		cStack;
	uint8*				pvMem;
	size				iKeySize;
	bool				bResult;
	void*				pvData;
	size				uiDataSize;

	if (mpcIndexTreeEvictionCallback)
	{
		iKeySize = mcIndexTree.GetNodeKeySize(pcNode);
		pvMem = (uint8*)cStack.Init(iKeySize + 1);
		mcIndexTree.GetNodeKey(pcNode, pvMem, iKeySize + 1);
		pvMem[iKeySize] = 0;
		pvData = pcNode->GetDataPtr();
		uiDataSize = pcNode->GetDataSize();

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
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeEvicting::Flush(uint8* pvKey, size iKeySize)
{
	return mcIndexTree.Flush(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeEvicting::IsWriteThrough(void)
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
bool CIndexTreeEvicting::Flush(void)
{
	return mcIndexTree.Flush();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeEvicting::IsFlushed(void)
{
	return mcIndexTree.IsFlushed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeEvicting::ValidateIndexTree(void)
{
	return mcIndexTree.ValidateIndexTree();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeEvicting::ValidateIndexTree(bool bReadNodes)
{
	return mcIndexTree.ValidateIndexTree(bReadNodes);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeEvicting::HasDiagnosticCallback(void)
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
size CIndexTreeEvicting::NumNodes(void)
{
	return mcIndexTree.NumNodes();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeEvicting::NumMemoryNodes(void)
{
	return mcIndexTree.NumMemoryNodes();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexTreeEvicting::NumElements(void)
{
	SIndexTreeFileIterator	sIter;
	int64					iCount;
	bool					bExists;

	iCount = 0;
	bExists = StartIteration(&sIter, NULL, NULL, 0, NULL, NULL, 0);
	while (bExists)
	{
		iCount++;
		bExists = Iterate(&sIter, NULL, NULL, 0, NULL, NULL, 0);
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeEvicting::NumMemoryElements(void)
{
	return mcIndexTree.NumMemoryElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeEvicting::NumMemoryElements(size iSize)
{
	return mcIndexTree.NumMemoryElements(iSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvicting::DebugKey(CChars* pszDest, uint8* pvKey, size iKeySize, bool bSkipRoot, bool bShowFlags, bool bShowSize, bool bKeyAlreadyReversed)
{
	return mcIndexTree.DebugKey(pszDest, pvKey, iKeySize, bSkipRoot, bShowFlags, bShowSize, bKeyAlreadyReversed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvicting::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz, true, true);
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvicting::Print(CChars* pszDest, bool bShowFlags, bool bShowSize)
{
	CChars	sz;

	sz.Init();
	sz.Append("= [IndexTreeEvicting]  ");
	if (bShowSize)
	{
		sz.Append("[");
		sz.Append((uint64)mcIndexTree.GetUserMemorySize());
		sz.Append(" / ");
		sz.Append((uint64)muiCutoff);
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
CIndexTreeNodeFile* CIndexTreeEvicting::GetMemoryNode(uint8* pvKey, size iKeySize)
{
	return mcIndexTree.GetMemoryNode(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeEvicting::EvictNode(CIndexTreeNodeFile* pcNode)
{
	bool bEvict;

	if (pcNode->HasData())
	{
		bEvict = EvictNodeWithObject(pcNode);
	}
	else
	{
		bEvict = true;
	}

	if (bEvict)
	{
		return mcIndexTree.Evict(pcNode);
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
size CIndexTreeEvicting::GetNodeKey(CIndexTreeNodeFile* pcNode, uint8* pvDestKey, size iDestKeySize)
{
	return mcIndexTree.GetNodeKey(pcNode, pvDestKey, iDestKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeEvicting::GetNodeKeySize(CIndexTreeNodeFile* pcNode)
{
	return mcIndexTree.GetNodeKeySize(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeEvicting::GetSystemMemorySize(void)
{
	return mcIndexTree.GetSystemMemorySize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeEvicting::GetUserMemorySize(void)
{
	return mcIndexTree.GetUserMemorySize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeEvicting::GetRootFlags(void)
{
	return mcIndexTree.GetRoot()->GetFlags();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeEvicting::StartIteration(SIndexTreeFileIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, void* pvData, size* piDataSize, size iMaxDataSize)
{
	bool	bResult;
	size	iEvicted;

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
bool CIndexTreeEvicting::Iterate(SIndexTreeFileIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, void* pvData, size* piDataSize, size iMaxDataSize)
{

	bool	bResult;
	size		iEvicted;

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

