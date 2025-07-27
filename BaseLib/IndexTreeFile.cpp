#include "Logger.h"
#include "LogString.h"
#include "FileBasic.h"
#include "DiskFile.h"
#include "GlobalMemory.h"
#include "StackMemory.h"
#include "FilePathBuilder.h"
#include "IndexedFile.h"
#include "IndexTreeFileDebug.h"
#include "IndexTreeFileDefaultDataCallback.h"
#include "IndexTreeFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, int iMaxDataSize, int iMaxKeySize)
{
	return Init(pcDurableFileControl, szSubDirectory, &gcIndexTreeFileDefaultCallback, LifeLocal<CMallocator>(&gcSystemAllocator), IWT_Yes, IKR_No, iMaxDataSize, iMaxKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	return Init(pcDurableFileControl, szSubDirectory, &gcIndexTreeFileDefaultCallback, LifeLocal<CMallocator>(&gcSystemAllocator), eWriteThrough, eKeyReverse, MAX_DATA_SIZE, MAX_KEY_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CLifeInit<CIndexTreeDataOrderer> cDataOrderer)
{
	return Init(pcDurableFileControl, szSubDirectory, &gcIndexTreeFileDefaultCallback, LifeLocal<CMallocator>(&gcSystemAllocator), eWriteThrough, eKeyReverse, MAX_DATA_SIZE, MAX_KEY_SIZE, cDataOrderer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CLifeInit<CMallocator> cMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	return Init(pcDurableFileControl, szSubDirectory, &gcIndexTreeFileDefaultCallback, cMalloc, eWriteThrough, eKeyReverse, MAX_DATA_SIZE, MAX_KEY_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CIndexTreeFileDataCallback* pcWriterCallback, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	return Init(pcDurableFileControl, szSubDirectory, pcWriterCallback, LifeLocal<CMallocator>(&gcSystemAllocator), eWriteThrough, eKeyReverse, MAX_DATA_SIZE, MAX_KEY_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CIndexTreeFileDataCallback* pcWriterCallback, CLifeInit<CMallocator> cMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CLifeInit<CIndexTreeDataOrderer> cDataOrderer)
{
	return Init(pcDurableFileControl, szSubDirectory, pcWriterCallback, cMalloc, eWriteThrough, eKeyReverse, MAX_DATA_SIZE, MAX_KEY_SIZE, cDataOrderer);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CIndexTreeFileDataCallback* pcWriterCallback, CLifeInit<CMallocator> cMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, int iMaxDataSize, int iMaxKeySize)
{
	return Init(pcDurableFileControl, szSubDirectory, pcWriterCallback, cMalloc, eWriteThrough, eKeyReverse, iMaxDataSize, iMaxKeySize, LifeNull<CIndexTreeDataOrderer>());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CIndexTreeFileDataCallback* pcWriterCallback, CLifeInit<CMallocator> cMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, int iMaxDataSize, int iMaxKeySize, CLifeInit<CIndexTreeDataOrderer> cDataOrderer)
{
	bool					bResult;

	if (!pcDurableFileControl->IsBegun())
	{
		return gcLogger.Error2(__METHOD__, " DurableFileController.Begin must be called before Init.", NULL);
	}

	cMalloc.ConfigureLife(&mcPassThruMallocLife, &mpcPassThruMallocLife);
	mcMalloc.Init(cMalloc.GetLife());
	CIndexTree::Init(LifeLocal<CMallocator>(&mcMalloc), eKeyReverse, sizeof(CIndexTreeNodeFile), sizeof(CIndexTreeNodeFile) + sizeof(CIndexTreeDataNode), sizeof(CIndexTreeChildNode), iMaxDataSize, iMaxKeySize, cDataOrderer);

	mpcDiagnosticCallback = NULL;
	mpcDataCallback = pcWriterCallback;
	mpcRoot = NULL;
	meWriteThrough = eWriteThrough;
	mpcDurableFileControl = pcDurableFileControl;
	mcIndexFiles.Init(mpcDurableFileControl, szSubDirectory, "IDAT", "Index.IDX", "_Index.IDX");
	bResult = mcIndexFiles.ReadIndexedFileDescriptors();
	if (!bResult)
	{
		return false;
	}

	bResult = InitRoot(szSubDirectory);
	if (!bResult)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::Kill(void)
{
	if (mpcDurableFileControl->IsBegun())
	{
		return gcLogger.Error2(__METHOD__, " DurableFileController.End must called before Kill.", NULL);
	}

	if (!IsFlushed())
	{
		return gcLogger.Error2(__METHOD__, " Flush must called before Kill.", NULL);
	}

	mcPassThruMallocLife.Kill();
	mcRootIndex.Kill();
	mcIndexFiles.Kill();
	mpcDurableFileControl = NULL;
	RecurseKill(mpcRoot);

	return CIndexTree::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::RecurseKill(CIndexTreeNodeFile* pcNode)
{
	int						i;
	CIndexTreeChildNode*	pcChild;
	CIndexTreeNodeFile*		pcChildNode;
	CIndexTreeChildNode*	apcChildren;
	int						iNumNodes;

	if (pcNode != NULL)
	{
		iNumNodes = pcNode->NumIndexes();
		apcChildren = pcNode->GetNodes();
		for (i = 0; i < iNumNodes; i++)
		{
			pcChild = &apcChildren[i];
			if (pcChild->IsMemory())
			{
				pcChildNode = pcChild->u.mpcMemory;
				RecurseKill(pcChildNode);
			}
		}

		if (mpcDiagnosticCallback)
		{
			if (HasData(pcNode))
			{
				DiagnosticEvictCallback(pcNode);
			}
		}

		FreeNode(pcNode);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::InitRoot(char* szSubDirectory)
{
	bool				bRootIndexExists;
	CFileDataIndex		cRootDataIndex;
	CIndexedFile*		pcRootIndexFile;
	CStackMemory<>		cTemp;
	int					iFileSize;
	void*				pvBuffer;
	bool				bResult;
	CFilePathBuilder	cWrite;
	CFilePathBuilder	cRewrite;

	mcRootIndex.Init(mpcDurableFileControl, cWrite.Build(NullToEmpty(szSubDirectory), "Root.IDX", NULL), cRewrite.Build(NullToEmpty(szSubDirectory), "_Root.IDX", NULL));
	cWrite.Kill(); cRewrite.Kill();
	cRootDataIndex = ReadRootFileIndex();

	bRootIndexExists = cRootDataIndex.HasFile();
	if (bRootIndexExists)
	{
		//The data size on the root is always zero.
		mpcRoot = AllocateRoot(cRootDataIndex);
		iFileSize = mpcRoot->CalculateFileSize(mpcDataCallback);

		pcRootIndexFile = mcIndexFiles.GetFile(cRootDataIndex.miFile);
		if (pcRootIndexFile == NULL)
		{
			return gcLogger.Error2(__METHOD__, " Could not get root node indexed file [", IntToString(cRootDataIndex.miFile), "]", NULL);
		}

		if (iFileSize != pcRootIndexFile->GetDataSize())
		{
			return gcLogger.Error2(__METHOD__, " Root node size [", IntToString(iFileSize), "] does not match root node indexed file size[", SizeToString(pcRootIndexFile->GetDataSize()), "]", NULL);
		}

		pvBuffer = cTemp.Init(iFileSize);
		bResult = pcRootIndexFile->Read(cRootDataIndex.muiIndex, pvBuffer, 1);
		if (!bResult)
		{
			return gcLogger.Error2(__METHOD__, " Could not read root node indexed file.", NULL);
		}

		mpcRoot->InitFromBuffer(pvBuffer, iFileSize, mpcDataCallback);
		cTemp.Kill();

		return true;
	}
	else
	{
		mpcRoot = AllocateRoot();
		bResult = Write(mpcRoot);
		bResult = WriteRootFileIndex(mpcRoot->GetFileIndex());
		return bResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileDataIndex CIndexTreeFile::ReadRootFileIndex(void)
{
	CFileDataIndex	cIndex;
	filePos			iSize;

	iSize = mcRootIndex.Read(&cIndex, sizeof(CFileDataIndex), 1);
	if (iSize == 1)
	{
		return cIndex;
	}
	else if (iSize > 1)
	{
		gcLogger.Error2(__METHOD__, " Index Root file size [", LongToString(iSize * sizeof(CFileDataIndex)), "] is incorrect.  Should be SizeOf(CFileDataIndex).", NULL);
		cIndex.Init();
		return cIndex;
	}
	else
	{
		cIndex.Init();
		return cIndex;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::WriteRootFileIndex(CFileDataIndex* pcRootIndex)
{
	filePos	ulliWritten;

	ulliWritten = mcRootIndex.Write(0, pcRootIndex, sizeof(CFileDataIndex), 1);
	return ulliWritten == 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::AllocateRoot(void)
{
	CIndexTreeNodeFile*		pcNode;
	int						iRootNodeSize;

	iRootNodeSize = CalculateRootNodeSize();
	pcNode = (CIndexTreeNodeFile*)Malloc(iRootNodeSize);
	pcNode->Init(this, NULL, 0, MAX_UCHAR, 0);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::AllocateRoot(CFileDataIndex cFileIndex)
{
	CIndexTreeNodeFile*		pcNode;
	int						iAdditionalSize;

	iAdditionalSize = CalculateRootNodeSize();
	pcNode = (CIndexTreeNodeFile*)Malloc(SizeofNode() + iAdditionalSize);
	pcNode->Init(this, NULL, 0, MAX_UCHAR, cFileIndex, 0);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::AllocateNodeSingle(CIndexTreeNodeFile* pcParent, size uiIndexInParent, size uiDataSize)
{
	CIndexTreeNodeFile*		pcNode;
	size					uiSize;

	pcNode = NULL;
	uiSize = CalculateNodeSize(0, uiDataSize);
	pcNode = (CIndexTreeNodeFile*)Malloc(uiSize);
	pcNode->Init(this, pcParent, uiIndexInParent);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::AllocateNodeRange(CIndexTreeNodeFile* pcParent, size uiIndexInParent, size uiFirstIndex, size uiLastIndex, size uiDataSize)
{
	CIndexTreeNodeFile*		pcNode;
	size					iRequiredIndices;
	size					uiSize;

	pcNode = NULL;
	iRequiredIndices = pcNode->NumIndexes(uiFirstIndex, uiLastIndex);
	uiSize = CalculateNodeSize(iRequiredIndices, uiDataSize);
	pcNode = (CIndexTreeNodeFile*)Malloc(uiSize);
	pcNode->Init(this, pcParent, uiFirstIndex, uiLastIndex, uiDataSize, uiIndexInParent);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::AllocateNodeFromBuffer(CIndexTreeNodeFile* pcParent, size uiIndexInParent, void* pvBuffer, int iMaxBufferSize)
{
	uint8*					pucMemory;
	int						iPos;
	int						iNumNodes;
	uint16					uiDataSize;
	uint8					uiFirstIndex;
	uint8					uiLastIndex;
	CIndexTreeNodeFile*		pcNode;
	size					uiSize;
	int						iBufferRead;
	int						iFileSize;
	int						iFileDataSize;

	pucMemory = (uint8*)pvBuffer;
	iPos = 0;
	iFileSize = *((int*)&pucMemory[iPos]);  iPos += sizeof(int);
	iFileDataSize = *((int*)&pucMemory[iPos]);  iPos += sizeof(int);
	uiDataSize = *((uint16*)&pucMemory[iPos]);  iPos += sizeof(uint16);

	uiFirstIndex = pucMemory[iPos];  iPos++;
	uiLastIndex = pucMemory[iPos];  iPos++;
	iNumNodes = (uiLastIndex - uiFirstIndex) + 1;

	uiSize = CalculateNodeSize(iNumNodes, uiDataSize);

	pcNode = (CIndexTreeNodeFile*)Malloc(uiSize);
	if (uiDataSize == 0)
	{
		pcNode->Init(this, pcParent, uiFirstIndex, uiLastIndex, uiIndexInParent);
	}
	else
	{
		pcNode->Init(this, pcParent, uiFirstIndex, uiLastIndex, uiDataSize, uiIndexInParent);
	}
	
	iBufferRead = pcNode->InitFromBuffer(pvBuffer, iMaxBufferSize, mpcDataCallback);
	if (iBufferRead > 0)
	{
		return pcNode;
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
CIndexTreeNodeFile* CIndexTreeFile::GetNodeForData(void* pvData)
{
	CIndexTreeNodeFile*		psNode;

	psNode = DataGetHeader<CIndexTreeNodeFile, void>(pvData);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexedFile* CIndexTreeFile::GetFile(int iFile)
{
	return mcIndexFiles.GetFile(iFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::GetRoot(void)
{
	return mpcRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::GetNode(uint8* pvKey, size iKeySize)
{
	CIndexTreeNodeFile*		pcCurrent;
	uint8					c;
	size					i;
	bool					bExecute;

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return NULL;
	}

	pcCurrent = mpcRoot;
	bExecute = StartKey(&i, iKeySize);
	while (bExecute)
	{
		c = ((uint8*)pvKey)[i];
		pcCurrent = ReadNode(pcCurrent, c);
		if (pcCurrent == NULL)
		{
			return NULL;
		}
		bExecute = LoopKey(&i, iKeySize);
	}

	return pcCurrent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::GetMemoryNode(uint8* pvKey, size iKeySize)
{
	CIndexTreeNodeFile*		pcCurrent;
	uint8					c;
	size						i;
	bool					bExecute;

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return NULL;
	}

	pcCurrent = mpcRoot;
	bExecute = StartKey(&i, iKeySize);
	while (bExecute)
	{
		c = ((uint8*)pvKey)[i];
		pcCurrent = ReadMemoryNode(pcCurrent, c);
		if (pcCurrent == NULL)
		{
			return NULL;
		}
		bExecute = LoopKey(&i, iKeySize);
	}
	return pcCurrent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::ReadNode(CIndexTreeNodeFile* pcParent, size c, bool bReadNode)
{
	if (bReadNode)
	{
		return ReadNode(pcParent, c);
	}
	else
	{
		return ReadMemoryNode(pcParent, c);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::ReadNode(CIndexTreeNodeFile* pcParent, size uiIndexInParent)
{
	CIndexTreeNodeFile*		pcCurrent;
	CIndexTreeChildNode*	pcChild;

	pcChild = pcParent->Get(uiIndexInParent);
	if (pcChild)
	{
		if (pcChild->IsMemory())
		{
			pcCurrent = pcChild->u.mpcMemory;
			return pcCurrent;
		}
		else if (pcChild->IsFile())
		{
			if (Read(pcChild, pcParent, uiIndexInParent))
			{
				pcCurrent = pcChild->u.mpcMemory;
				return pcCurrent;
			}
			else
			{
				gcLogger.Error2(__METHOD__, " Could not load child node [", SizeToString(uiIndexInParent), "].", NULL);
				return NULL;
			}
		}
		else if (pcChild->IsUnallocated())
		{
			//Data for key does not exist.
			return NULL;
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Child node [", SizeToString(uiIndexInParent), "] is corrupt.  Type is [", pcChild->iType, "].", NULL);
			return NULL;
		}
	}
	else
	{
		//Data for key does not exist.
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::ReadMemoryNode(CIndexTreeNodeFile* pcParent, size uiIndexInParent)
{
	CIndexTreeNodeFile*		pcCurrent;
	CIndexTreeChildNode*	pcChild;

	pcChild = pcParent->Get(uiIndexInParent);
	if (pcChild)
	{
		if (pcChild->IsMemory())
		{
			pcCurrent = pcChild->u.mpcMemory;
			return pcCurrent;
		}
		else if (pcChild->IsFile() || pcChild->IsUnallocated())
		{
			return NULL;
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Child node [", SizeToString(uiIndexInParent), "] is corrupt.  Type is [", pcChild->iType, "].", NULL);
			return NULL;
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
bool CIndexTreeFile::HasMemoryNodes(CIndexTreeNodeFile* pcNode)
{
	size					i;
	CIndexTreeNodeFile*		pcChild;
	size					uiFirstIndex;
	size					uiLastIndex;

	uiFirstIndex = pcNode->GetFirstIndex();
	uiLastIndex = pcNode->GetLastIndex();
	for (i = uiFirstIndex; i <= uiLastIndex; i++)
	{
		pcChild = ReadMemoryNode(pcNode, i);
		if (pcChild != NULL)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::Get(uint8* pvKey, size iKeySize, void* pvData, size* piDataSize, size uiMaxDataSize)
{
	CIndexTreeNodeFile*		pcNode;
	size					uiDataSize;

	pcNode = GetNode(pvKey, iKeySize);
	if (pcNode == NULL)
	{
		return false;
	}
	else
	{
		uiDataSize = GetNodeData(pcNode, pvData, uiMaxDataSize);
		if (uiDataSize != 0)
		{
			GetReorderData(pcNode);

			SafeAssign(piDataSize, uiDataSize);

			if (mpcDiagnosticCallback)
			{
				mpcDiagnosticCallback->Get(pvKey, iKeySize, pvData, uiDataSize);
			}
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
bool CIndexTreeFile::Put(uint8* pvKey, size iKeySize, void* pvData, size iDataSize)
{
	CIndexTreeNodeFile*		pcCurrent;
	bool					bResult;
	size					uiDataSize;
	bool					bNewNode;

	ReturnNullOnFalse(ValidatePut(iKeySize, iDataSize));

	uiDataSize = (uint16)iDataSize;

	pcCurrent = AllocateKey(pvKey, iKeySize);

	bNewNode = false;
	if (!HasData(pcCurrent))
	{
		bNewNode = true;
	}

	pcCurrent = SetNodeData(pcCurrent, pvData, uiDataSize);
	if (pcCurrent == NULL)
	{
		return false;
	}

	if (meWriteThrough == IWT_Yes)
	{
		bResult = WriteBackPathWriteThrough(pcCurrent);
	}
	else if (meWriteThrough == IWT_No)
	{
		bResult = DirtySetPaths(pcCurrent);
		if (!bResult)
		{
			return false;
		}
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Don't know how to put data with Write Through [IWT_Unknown].", NULL);
		return false;
	}

	if (bNewNode)
	{
		InsertReorderData(pcCurrent);
	}
	PutReorderData(pcCurrent);

	if (mpcDiagnosticCallback)
	{
		mpcDiagnosticCallback->Put(pvKey, iKeySize, pvData, uiDataSize);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::AllocateKey(uint8* pvKey, size iKeySize)
{
	CIndexTreeNodeFile*		pcCurrent;
	uint8					c;
	bool					bExecute;
	size					i;

	pcCurrent = mpcRoot;
	bExecute = StartKey(&i, iKeySize);
	while (bExecute)
	{
		c = ((char*)pvKey)[i];
		pcCurrent = AllocateChildNode(pcCurrent, c);
		bExecute = LoopKey(&i, iKeySize);
	}
	return pcCurrent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::SetNodeData(CIndexTreeNodeFile* pcCurrent, void* pvData, size uiDataSize)
{
	CIndexTreeNodeFile*		pcReallocatedCurrent;
	size					uiOldDataSize;

	uiOldDataSize = pcCurrent->GetDataSize();
	if (uiDataSize > uiOldDataSize)
	{
		pcReallocatedCurrent = ReallocateNodeForLargerData(pcCurrent, pvData, uiDataSize);
	}
	else if (uiDataSize < uiOldDataSize)
	{
		pcReallocatedCurrent = ReallocateNodeForSmallerData(pcCurrent, pvData, uiDataSize);
	}
	else
	{
		pcReallocatedCurrent = pcCurrent;
		pcReallocatedCurrent->SetData(pvData, uiDataSize);
	}

	if (pcReallocatedCurrent == NULL)
	{
		return NULL;
	}

	pcReallocatedCurrent->SetDirtyNode(true);

	return pcReallocatedCurrent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::SetDirtyPath(CIndexTreeNodeFile* pcCurrent)
{
	if (meWriteThrough == IWT_Yes)
	{
		return gcLogger.Error2(__METHOD__, " Cannot SetDirtyPath on an index tree that is write through.", NULL);
	}
	else if (meWriteThrough == IWT_No)
	{
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Don't know how to set dirty path with Write Through [IWT_Unknown].", NULL);
		return false;
	}

	if (pcCurrent->IsDirty())
	{
		while ((pcCurrent) && (!pcCurrent->IsPathDirty()))
		{
			pcCurrent->SetDirtyPath(true);
			pcCurrent = (CIndexTreeNodeFile*)pcCurrent->GetParent();
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::SetDeletedPath(CIndexTreeNodeFile* pcCurrent)
{
	if (meWriteThrough == IWT_Yes)
	{
		return gcLogger.Error2(__METHOD__, " Cannot SetDeletedPath on an index tree that is write through.", NULL);
	}
	else if (meWriteThrough == IWT_No)
	{
	}
	else 
	{
		gcLogger.Error2(__METHOD__, " Don't know how to flush data with Write Through [IWT_Unknown].", NULL);
		return false;
	}

	if (pcCurrent->IsDeleted())
	{
		return gcLogger.Error2(__METHOD__, " Cannot SetDeletedPath on an index tree file node that is already deleted.", NULL);
	}

	while ((pcCurrent) && (!pcCurrent->IsPathDeleted()))
	{
		pcCurrent->SetDeletedPath(true);
		pcCurrent = (CIndexTreeNodeFile*)pcCurrent->GetParent();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::ReallocateNodeForContainIndex(CIndexTreeNodeFile* pcNode, size uiIndex)
{
	CIndexTreeNodeFile*		pcOldNode;
	size					tNewNodeSize;

	if (pcNode->ContainsIndex(uiIndex))
	{
		return pcNode;
	}

	tNewNodeSize = pcNode->CalculateRequiredNodeSizeForIndex(uiIndex);

	pcOldNode = pcNode;
	pcNode = (CIndexTreeNodeFile*)Realloc(pcNode, tNewNodeSize);

	pcNode->Contain(uiIndex, INDEX_TREE_FILE_NODE_UNALLOCATED);
	//pcNode->SetDirtyNode(true);

	RemapNodePointers(pcOldNode, pcNode);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::ReallocateNodeForLargerData(CIndexTreeNodeFile* pcNode, void* pvData, size uiDataSize)
{
	size					tNewNodeSize;
	CIndexTreeNodeFile*		pcOldNode;

	tNewNodeSize = pcNode->CalculateRequiredNodeSizeForData(uiDataSize);

	pcOldNode = pcNode;
	pcNode = (CIndexTreeNodeFile*)Realloc(pcNode, tNewNodeSize);
	if (pcNode)
	{
		pcNode->SetData(pvData, uiDataSize);

		RemapNodePointers(pcOldNode, pcNode);
		return pcNode;
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
CIndexTreeNodeFile* CIndexTreeFile::ReallocateNodeForSmallerData(CIndexTreeNodeFile* pcNode, void* pvData, size uiDataSize)
{
	size					tNewNodeSize;
	size					uiOriginalSize;
	CIndexTreeNodeFile*		pcOldNode;

	uiOriginalSize = pcNode->GetDataSize();
	pcNode->SetData(pvData, uiDataSize);

	tNewNodeSize = pcNode->CalculateRequiredNodeSizeForCurrent();

	pcOldNode = pcNode;
	pcNode = (CIndexTreeNodeFile*)Realloc(pcNode, tNewNodeSize);
	if (pcNode)
	{
		RemapNodePointers(pcOldNode, pcNode);
		return pcNode;
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
void CIndexTreeFile::RemapNodePointers(CIndexTreeNodeFile* pcOldNode, CIndexTreeNodeFile* pcNode)
{
	CIndexTreeNodeFile*		pcParent;

	if (pcOldNode != pcNode)
	{
		pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();
		if (pcParent)
		{
			pcParent->RemapChildNodes(pcOldNode, pcNode);
		}
		pcNode->SetChildrensParent();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::AllocateChildNode(CIndexTreeNodeFile* pcParent, size uiIndexInParent)
{
	CIndexTreeNodeFile*		pcNewFileNode;
	CIndexTreeChildNode*	pcChildNodeOnParent;
	CIndexTreeNodeFile*		pcReallocedParent;
	bool					bDirty;

	pcChildNodeOnParent = pcParent->Get(uiIndexInParent);
	if (pcChildNodeOnParent == NULL)  //Uncontained, off the left or the right of the parent node's indexes.
	{
		//This causes the parent node to be re-allocated as its indexes must be grown.
		pcReallocedParent = ReallocateNodeForContainIndex(pcParent, uiIndexInParent);

		//The new node must also still be allocated.  Two nodes have been altered (the parent and the new node).
		pcNewFileNode = AllocateNodeSingle(pcReallocedParent, uiIndexInParent, 0);

		bDirty = pcReallocedParent->SetMemory(uiIndexInParent, pcNewFileNode);

		//If the parent moved in memory then all its children must be corrected.
		if (pcParent != pcReallocedParent)
		{
			pcReallocedParent->SetChildrensParent();
		}

		return pcNewFileNode;
	}
	else
	{
		if (pcChildNodeOnParent->IsMemory())
		{
			return pcChildNodeOnParent->u.mpcMemory;
		}
		else if (pcChildNodeOnParent->IsFile())
		{
			if (Read(pcChildNodeOnParent, pcParent, uiIndexInParent))
			{
				return pcChildNodeOnParent->u.mpcMemory;
			}
			else
			{
				gcLogger.Error2(__METHOD__, " Could not load child node [", SizeToString(uiIndexInParent), "].", NULL);
				return NULL;
			}
		}
		else if (pcChildNodeOnParent->IsUnallocated())
		{
			pcNewFileNode = AllocateNodeSingle(pcParent, uiIndexInParent, 0);

			pcChildNodeOnParent->Init(pcNewFileNode);

			return pcChildNodeOnParent->u.mpcMemory;
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Child node [", SizeToString(uiIndexInParent), "] is corrupt.", NULL);
			return NULL;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::Flush(uint8* pvKey, size iKeySize)
{
	CIndexTreeNodeFile*	pcNode;
	bool				bResult;

	pcNode = GetMemoryNode(pvKey, iKeySize);
	if (!pcNode)
	{
		return false;
	}
	else
	{
		bResult = Flush(&pcNode);
		return bResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::ValidateKey(char* pszKey)
{
	size iKeySize;

	if (StrEmpty(pszKey))
	{
		return false;
	}

	iKeySize = strlen(pszKey);
	return ValidateKey((uint8*)pszKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::Remove(uint8* pvKey, size iKeySize)
{
	CIndexTreeNodeFile*		pcCurrent;
	CIndexTreeNodeFile*		pcDirty;
	size					uiOldDataSize;
	void*					pvData;
	void*					pv;
	CStackMemory<>			cStack;

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return false;
	}

	pcCurrent = GetNode(pvKey, iKeySize);
	if (pcCurrent == NULL)
	{
		return false;
	}

	uiOldDataSize = pcCurrent->GetDataSize();
	if (uiOldDataSize == 0)
	{
		return false;
	}

	RemoveReorderData(pcCurrent);

	if (mpcDiagnosticCallback)
	{
		pvData = cStack.Init(uiOldDataSize);
		pv = pcCurrent->GetDataPtr();
		memcpy(pvData, pv, uiOldDataSize);
		mpcDiagnosticCallback->Remove(pvKey, iKeySize, pvData, uiOldDataSize);
		cStack.Kill();
	}

	if (meWriteThrough == IWT_Yes)
	{
		pcDirty = RemoveWriteThrough(pcCurrent);
		if (pcDirty != NULL)
		{
			pcDirty->SetDirtyNode(true);
			return WriteBackPathWriteThrough(pcDirty);
		}
		else
		{
			return false;
		}
	}
	else if (meWriteThrough == IWT_No)
	{
		return RemoveWaitForFlush(pcCurrent);
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Don't know how to remove data with Write Through [IWT_Unknown].", NULL);
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::RemoveWriteThrough(CIndexTreeNodeFile* pcCurrent)
{
	size					c;
	CIndexTreeNodeFile*		pcParent;
	CIndexTreeNodeFile*		pcNode;
	bool					bResult;

	if (pcCurrent->GetDataSize() == 0)
	{
		return false;
	}

	bResult = true;

	pcNode = ReallocateNodeForSmallerData(pcCurrent, NULL, 0);
	while (pcNode)
	{
		pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();
		c = pcNode->GetIndexInParent();

		if (pcNode->IsEmpty())
		{
			if (pcParent != mpcRoot)
			{
				pcParent = ReallocateNodeForUncontainIndex(pcParent, c);
			}
			else
			{
				pcParent->ClearIndex(c);
			}

			bResult = Delete(pcNode);
			Free(pcNode);

			if (!bResult)
			{
				return NULL;
			}
		}
		else
		{
			return pcNode;
		}

		pcNode = pcParent;
	}

	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::ReallocateNodeForUncontainIndex(CIndexTreeNodeFile* pcNode, size c)
{
	CIndexTreeNodeFile*		pcOldNode;
	bool					bResizeNode;
	size					tNewNodeSize;

	bResizeNode = pcNode->ClearIndexAndUncontain(c);
	if (bResizeNode)
	{
		tNewNodeSize = pcNode->CalculateRequiredNodeSizeForCurrent();
		pcOldNode = pcNode;

		pcNode = (CIndexTreeNodeFile*)Realloc(pcNode, tNewNodeSize);
		RemapNodePointers(pcOldNode, pcNode);
	}

	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::RemoveWaitForFlush(CIndexTreeNodeFile* pcCurrent)
{
	if (pcCurrent->GetDataSize() == 0)
	{
		return false;
	}

	if (pcCurrent->IsDeleted())
	{
		return false;
	}

	return RemoveSetPaths(pcCurrent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::RemoveSetPaths(CIndexTreeNodeFile* pcCurrent)
{
	pcCurrent->SetDirtyNode(false);
	if (!ClearDirtyPath(pcCurrent))
	{
		return false;
	}

	if (!SetDeletedPath(pcCurrent))
	{
		return false;
	}
	pcCurrent->SetDeletedNode(true);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::DirtySetPaths(CIndexTreeNodeFile* pcCurrent)
{
	pcCurrent->SetDeletedNode(false);
	if (!ClearDeletedPath(pcCurrent))
	{
		return false;
	}

	if (!SetDirtyPath(pcCurrent))
	{
		return false;
	}
	pcCurrent->SetDirtyNode(true);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::Evict(uint8* pvKey, size iKeySize)
{
	CIndexTreeNodeFile*		pcCurrent;

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return false;
	}

	pcCurrent = GetMemoryNode(pvKey, iKeySize);
	if (pcCurrent == NULL)
	{
		return false;
	}

	return Evict(pcCurrent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::Evict(CIndexTreeNodeFile* pcNode)
{
	bool	bResult;

	pcNode->ValidateParent();
	bResult = CanEvict(pcNode);
	if (!bResult)
	{
		return false;
	}

	if (meWriteThrough == IWT_No)
	{
		bResult = Flush(&pcNode);
		if (!bResult)
		{
			return false;
		}
	}
	else if (meWriteThrough == IWT_Yes)
	{
	}
	else 
	{
		gcLogger.Error2(__METHOD__, " Don't know how to evict data with Write Through [IWT_Unknown].", NULL);
		return false;
	}

	if (pcNode)
	{
		if (mpcDiagnosticCallback)
		{
			DiagnosticEvictCallback(pcNode);
		}
		return EvictNode(pcNode);
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
bool CIndexTreeFile::EvictNode(CIndexTreeNodeFile* pcCurrent)
{
	CIndexTreeNodeFile*		pcParent;
	CIndexTreeNodeFile*		pcNode;
	bool					bOnlyFileNodes;
	int						iCount;
	bool					bResult;

	pcNode = pcCurrent;

	iCount = 0;
	while (pcNode != mpcRoot)
	{
		pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();

		bOnlyFileNodes = pcNode->HasOnlyFileNodes();
		if (!bOnlyFileNodes)
		{
			return true;
		}

		iCount++;

		if (HasData(pcNode))
		{
			RemoveReorderData(pcNode);
		}

		bResult = pcParent->ConvertToFileNode(pcNode);
		Free(pcNode);

		if (!bResult)
		{
			return false;
		}

		pcNode = pcParent;

		if (pcNode->HasData())  //Not HasData(pcNode).
		{
			break;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::Flush(CIndexTreeNodeFile** ppcCurrent)
{
	bool				bResult;
	CIndexTreeNodeFile* pcCurrent;
	CIndexTreeNodeFile* pcDirty;
	bool				bDeleted;
	bool				bDirty;

	pcCurrent = *ppcCurrent;
	if (meWriteThrough == IWT_Yes)
	{
		return gcLogger.Error2(__METHOD__, " Cannot flush an index tree that is write through.", NULL);
	}
	else if (meWriteThrough == IWT_No)
	{
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Don't know how to flush data with Write Through [IWT_Unknown].", NULL);
		return false;
	}

	bDeleted = pcCurrent->IsDeleted();
	bDirty = pcCurrent->IsDirty();
	if (bDeleted && bDirty)
	{
		return gcLogger.Error2(__METHOD__, " Cannot flush a node that is both deleted and dirty.", NULL);
	}
	else if (bDeleted)
	{
		if (mpcDiagnosticCallback)
		{
			DiagnosticFlushCallback(pcCurrent);
		}

		*ppcCurrent = NULL;
		pcCurrent->SetDeletedNode(false);
		bResult = ClearDeletedPath(pcCurrent);
		pcDirty = RemoveWriteThrough(pcCurrent);
		if (pcDirty != NULL)
		{
			pcDirty->SetDirtyNode(true);
			bResult &= SetDirtyPath(pcDirty);
			bResult &= WriteBackPathCaching(pcDirty);
			return bResult;
		}
		else
		{
			return false;
		}
	}
	else if (bDirty)
	{
		if (mpcDiagnosticCallback)
		{
			DiagnosticFlushCallback(pcCurrent);
		}

		bResult = WriteBackPathCaching(pcCurrent);
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
bool CIndexTreeFile::CanEvict(CIndexTreeNodeFile* pcNode)
{
	if (!pcNode->HasOnlyFileNodes())
	{
		gcLogger.Error2(__METHOD__, " Cannot evict node with children.", NULL);
		return false;
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
void CIndexTreeFile::FindWithFlags(CArrayVoidPtr* papNodes, uint8 uiFollowFlags, uint8 uiAddFlags)
{
	CIndexTreeRecursor	cCursor;

	cCursor.Init(mpcRoot);
	RecurseFindWithFlags(&cCursor, uiFollowFlags, uiAddFlags, papNodes);
	cCursor.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::RecurseFindWithFlags(CIndexTreeRecursor* pcCursor, uint8 uiFollowFlags, uint8 uiAddFlags, CArrayVoidPtr* papNodes)
{
	size					i;
	CIndexTreeNodeFile*		pcChild;
	size					iFirst;
	size					iLast;
	CIndexTreeNodeFile*		pcNode;

	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		if (pcNode->HasFlags(uiAddFlags))
		{
			papNodes->Add(pcNode);
		}

		if (pcNode->HasFlags(uiFollowFlags))
		{
			if (pcNode->HasNodes())
			{
				iFirst = pcNode->GetFirstIndex();
				iLast = pcNode->GetLastIndex();

				for (i = iFirst; i <= iLast; i++)
				{
					pcChild = ReadNode(pcNode, i);
					pcCursor->Push(pcChild, (uint8)i);
					RecurseFindWithFlags(pcCursor, uiFollowFlags, uiAddFlags, papNodes);
				}
			}
		}
	}
	pcCursor->Pop();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::ClearNodesFlags(CArrayVoidPtr* papNodes, uint8 uiFlags)
{
	int						i;
	CIndexTreeNodeFile*		pcNode;
	int						iNumNodes;

	iNumNodes = papNodes->NumElements();
	for (i = 0; i < iNumNodes; i++)
	{
		pcNode = (CIndexTreeNodeFile*)papNodes->GetPtr(i);
		while (pcNode != NULL && pcNode->HasFlags(uiFlags))
		{
			pcNode->ClearFlags(uiFlags);
			pcNode = (CIndexTreeNodeFile*)pcNode->GetParent();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeFile::GetUserMemorySize(void)
{
	return mcMalloc.AllocatedUserSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeFile::GetSystemMemorySize(void)
{
	return mcMalloc.AllocatedSystemSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::IsWriteThrough(void)
{
	if (meWriteThrough == IWT_Yes)
	{
		return true;
	}
	else if (meWriteThrough == IWT_No)
	{
		return false;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Don't know how to flush data with Write Through [IWT_Unknown].", NULL);
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::HasDiagnosticCallback(void)
{
	return mpcDiagnosticCallback != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDurableFileController* CIndexTreeFile::GetController(void)
{
	return mpcDurableFileControl;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CIndexTreeFile::NumElements(void)
{
	int		iNumElements;

	iNumElements = RecurseNumElements(mpcRoot);
	return iNumElements;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeFile::NumMemoryElements(void)
{
	return RecurseNumMemoryElements(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeFile::NumMemoryElements(size iSize)
{
	return RecurseNumMemoryElements(mpcRoot, iSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::Flush(void)
{
	bool	bResult;
	
	if (meWriteThrough == IWT_Yes)
	{
		return gcLogger.Error2(__METHOD__, " Cannot flush an index tree that is write through.", NULL);
	}
	else if (meWriteThrough == IWT_No)
	{
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Don't know how to flush data with Write Through [IWT_Unknown].", NULL);
		return false;
	}

	bResult = FlushDeleted();
	bResult &= FlushDirty();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::FlushDirty(void)
{
	CIndexTreeRecursor	cCursor;
	bool				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseFlushDirty(&cCursor);
	cCursor.Kill();

	return bResult;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::RecurseFlushDirty(CIndexTreeRecursor* pcCursor)
{
	CIndexTreeNodeFile*		pcNode;
	size					i;
	CIndexTreeNodeFile*		pcChild;
	bool					bResult;
	size					uiFirstIndex;
	size					uiLastIndex;

	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if ((pcNode != NULL) && pcNode->IsPathDirty())
	{
		if (pcNode->HasNodes())
		{
			uiFirstIndex = pcNode->GetFirstIndex();
			uiLastIndex = pcNode->GetLastIndex();
			for (i = uiFirstIndex; i <= uiLastIndex; i++)
			{
				pcChild = ReadNode(pcNode, i);
				pcCursor->Push(pcChild, (uint8)i);
				bResult = RecurseFlushDirty(pcCursor);
				if (!bResult)
				{
					pcCursor->Pop();
					return false;
				}
			}
		}

		if (mpcDiagnosticCallback)
		{
			if (HasData(pcNode))
			{
				DiagnosticFlushCallback(pcNode);
			}
		}

		bResult = WriteBackPathCaching(pcNode);
		if (!bResult)
		{
			pcCursor->Pop();
			return false;
		}

		pcNode->ClearFlags(INDEX_TREE_NODE_FLAG_DIRTY_PATH | INDEX_TREE_NODE_FLAG_DIRTY_NODE);
	}
	pcCursor->Pop();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::FlushDeleted(void)
{
	CArrayVoidPtr					apvDeletedNodes;
	int								i;
	int								iNumNodes;
	CIndexTreeNodeFile*				pcNode;
	CIndexTreeNodeFile*				pcDirty;
	bool							bResult;
	CListTemplateMinimal<char>*		paszKeys;
	int								iKeySize;
	uint8*							pszKey;

	apvDeletedNodes.Init();
	FindWithFlags(&apvDeletedNodes, INDEX_TREE_NODE_FLAG_DELETED_PATH, INDEX_TREE_NODE_FLAG_DELETED_NODE);

	ClearNodesFlags(&apvDeletedNodes, INDEX_TREE_NODE_FLAG_DELETED_PATH | INDEX_TREE_NODE_FLAG_DELETED_NODE);

	bResult = true;
	paszKeys = GetNodesKeys(&apvDeletedNodes);
	iNumNodes = paszKeys->NumElements();
	for (i = 0; i < iNumNodes; i++)
	{
		pszKey = (uint8*)paszKeys->Get(i, (size*)&iKeySize);
		pcNode = GetNode(pszKey, iKeySize);
		if (!pcNode->IsDirty())
		{
			if (mpcDiagnosticCallback)
			{
				mpcDiagnosticCallback->Flush(pszKey, iKeySize, NULL, 0);
			}

			pcDirty = RemoveWriteThrough(pcNode);
			if (pcDirty != NULL)
			{
				pcDirty->SetDirtyNode(true);
				SetDirtyPath(pcDirty);
			}
			else
			{
				bResult = false;
			}
		}
		else
		{
			gcLogger.Debug2("Cannot flush a deleted node that is also dirty.", NULL);
		}
	}

	SafeFree(paszKeys);
	apvDeletedNodes.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::SetDiagnosticCallback(CIndexTreeFileDiagnosticCallback* pcCallback)
{
	mpcDiagnosticCallback = pcCallback;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::IsFlushed(void)
{
	CIndexTreeRecursor	cCursor;
	bool				bResult;

	if (meWriteThrough == IWT_No)
	{
		cCursor.Init(mpcRoot);
		bResult = RecurseIsFlushed(&cCursor);
		cCursor.Kill();

		return bResult;
	}
	else if (meWriteThrough == IWT_Yes)
	{
		return true;
	}
	else 
	{
		gcLogger.Error2(__METHOD__, " Don't know how to flush data with Write Through [IWT_Unknown].", NULL);
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::RecurseIsFlushed(CIndexTreeRecursor* pcCursor)
{
	CIndexTreeNodeFile*		pcNode;
	size					i;
	CIndexTreeNodeFile*		pcChild;
	bool					bResult;
	size					uiTransientFlags;
	size					uiFirstIndex;
	size					uiLastIndex;

	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		uiTransientFlags = pcNode->GetTransientFlags();
		if (pcNode->HasFlags(INDEX_TREE_NODE_TRANSIENT_FLAGS))
		{
			return false;
		}

		if (pcNode->HasNodes())
		{
			uiFirstIndex = pcNode->GetFirstIndex();
			uiLastIndex = pcNode->GetLastIndex();
			for (i = uiFirstIndex; i <= uiLastIndex; i++)
			{
				pcChild = ReadMemoryNode(pcNode, i);
				pcCursor->Push(pcChild, (uint8)i);
				bResult = RecurseIsFlushed(pcCursor);
				if (!bResult)
				{
					pcCursor->Pop();
					return false;
				}
			}
		}
	}
	pcCursor->Pop();
	return true;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::SetWriteThrough(EIndexWriteThrough eWriteThrough)
{
	meWriteThrough = eWriteThrough;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::StartIteration(SIndexTreeFileIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, void* pvData, size* piDataSize, size iMaxDataSize)
{
	SIndexTreeFileUnsafeIterator	sIter;
	size							iDataSize;
	bool							bResult;

	memset(psIterator->pvKey, 0, MAX_KEY_SIZE);
	psIterator->iIndex = mpcRoot->GetFirstIndex();
	psIterator->iKeyLength = 0;
	
	bResult = StartUnsafeIteration(&sIter, psIterator->pvKey, &psIterator->iKeyLength, MAX_KEY_SIZE, NULL, NULL);
	psIterator->iIndex = sIter.iIndex;

	if (bResult)
	{
		CopyData((uint8*)pvKey, psIterator->pvKey, psIterator->iKeyLength, iMaxKeySize);
		SafeAssign(piKeySize, psIterator->iKeyLength);
		if (piDataSize)
		{
			iDataSize = GetNodeDataSize(sIter.pcNode);
			*piDataSize = iDataSize;
		}
		if (pvData)
		{
			GetNodeData(sIter.pcNode, pvData, iMaxDataSize);
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::Iterate(SIndexTreeFileIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, void* pvData, size* piDataSize, size iMaxDataSize)
{
	SIndexTreeFileUnsafeIterator	sIter;
	size							iDataSize;
	bool							bResult;

	sIter.iIndex = psIterator->iIndex;
	sIter.pcNode = GetNode(psIterator->pvKey, psIterator->iKeyLength);

	if (!sIter.pcNode)
	{
		return false;
	}

	bResult = UnsafeIterate(&sIter, psIterator->pvKey, &psIterator->iKeyLength, MAX_KEY_SIZE, NULL, NULL);
	psIterator->iIndex = sIter.iIndex;

	if (bResult)
	{
		CopyData((char*)pvKey, psIterator->pvKey, psIterator->iKeyLength, iMaxKeySize);
		SafeAssign(piKeySize, psIterator->iKeyLength);
		if (piDataSize)
		{
			iDataSize = GetNodeDataSize(sIter.pcNode);
			*piDataSize = iDataSize;
		}
		if (pvData)
		{
			GetNodeData(sIter.pcNode, pvData, iMaxDataSize);
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::StartUnsafeIteration(SIndexTreeFileUnsafeIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, void** ppvData, size* piDataSize)
{
	psIterator->pcNode = mpcRoot;
	psIterator->iIndex = mpcRoot->GetFirstIndex();

	return UnsafeIterate(psIterator, pvKey, piKeySize, iMaxKeySize, ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::UnsafeIterate(SIndexTreeFileUnsafeIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, void** pvData, size* piDataSize)
{
	void*	pvDataTemp;
	int		iKeySize;

	if (StepNext(psIterator))
	{
		pvDataTemp = psIterator->pcNode->GetDataPtr();
		if (pvData)
		{
			*pvData = pvDataTemp;
		}
		if (piDataSize)
		{
			*piDataSize = psIterator->pcNode->GetDataSize();
		}
		if (pvKey) 
		{
			iKeySize = GetNodeKey(psIterator->pcNode, pvKey, iMaxKeySize);
			if (piKeySize)
			{
				*piKeySize = iKeySize;
			}
		}
		else if (piKeySize)
		{
			*piKeySize = GetNodeKeySize(psIterator->pcNode);
		}

		GetReorderData(psIterator->pcNode);

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
bool CIndexTreeFile::StartUnsafeIteration(SIndexTreeFileUnsafeIterator* psIterator, void** ppvData, size* piDataSize)
{
	return StartUnsafeIteration(psIterator, NULL, NULL, miMaxDataSize, ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::UnsafeIterate(SIndexTreeFileUnsafeIterator* psIterator, void** ppvData, size* piDataSize)
{
	return UnsafeIterate(psIterator, NULL, NULL, miMaxDataSize, ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeFile::GetNodeKeysSize(CArrayVoidPtr* apvNodes)
{
	int								i;
	CIndexTreeNodeFile*				pcNode;
	int								iKeySize;
	int								iNumKeys;
	int								iKeysSize;

	iNumKeys = apvNodes->NumElements();
	iKeysSize = 0;
	for (i = 0; i < iNumKeys; i++)
	{
		pcNode = (CIndexTreeNodeFile*)apvNodes->GetPtr(i);
		iKeySize = GetNodeKeySize(pcNode);
		iKeysSize += iKeySize;
	}

	return iKeysSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CListTemplateMinimal<char>* CIndexTreeFile::GetNodesKeys(CArrayVoidPtr* apvNodes)
{
	int								i;
	CIndexTreeNodeFile*				pcNode;
	char							szKey[MAX_KEY_SIZE];
	int								iKeySize;
	int								iNumKeys;
	int								iTotalSize;
	int								iKeysSize;
	CListTemplateMinimal<char>*		paszKeyNames;

	iNumKeys = apvNodes->NumElements();
	iKeysSize = GetNodeKeysSize(apvNodes);

	paszKeyNames = NULL;
	iTotalSize = paszKeyNames->TotalSize(iNumKeys, iKeysSize);
	paszKeyNames = (CListTemplateMinimal<char>*)malloc(iTotalSize);
	paszKeyNames->Init(iNumKeys, iKeysSize);

	for (i = 0; i < iNumKeys; i++)
	{
		pcNode = (CIndexTreeNodeFile*)apvNodes->GetPtr(i);
		iKeySize = GetNodeKey(pcNode, (uint8*)szKey, MAX_KEY_SIZE);

		paszKeyNames->Add((char*)&szKey[0], iKeySize);
	}

	return paszKeyNames;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CListCharsMinimal* CIndexTreeFile::GetNodesStringKeys(CArrayVoidPtr* apvNodes)
{
	int						i;
	CIndexTreeNodeFile*		pcNode;
	char					szKey[MAX_KEY_SIZE];
	int						iKeySize;
	int						iNumKeys;
	int						iTotalSize;
	int						iKeysSize;
	CListCharsMinimal*		paszKeyNames;

	iNumKeys = apvNodes->NumElements();
	iKeysSize = GetNodeKeysSize(apvNodes);

	paszKeyNames = NULL;
	iTotalSize = paszKeyNames->TotalSize(iNumKeys, iKeysSize);
	paszKeyNames = (CListCharsMinimal*)malloc(iTotalSize);
	paszKeyNames->Init(iNumKeys, iKeysSize);

	for (i = 0; i < iNumKeys; i++)
	{
		pcNode = (CIndexTreeNodeFile*)apvNodes->GetPtr(i);
		iKeySize = GetNodeKey(pcNode, (uint8*)szKey, MAX_KEY_SIZE);

		paszKeyNames->Add((char*)&szKey[0], iKeySize);
	}

	return paszKeyNames;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::StepNext(SIndexTreeFileIterator* psIterator)
{
	CIndexTreeNodeFile*		pcChildNode;
	CIndexTreeNodeFile*		pcParent;
	size					iCount;
	CIndexTreeNodeFile*		pcNode;
	size					iIndex;

	iCount = 0;
	iIndex = psIterator->iIndex;
	pcNode = GetNode(psIterator->pvKey, psIterator->iKeyLength);

	if (pcNode)
	{
		for (;;)
		{
			pcChildNode = ReadNode(pcNode, iIndex);

			if (pcChildNode != NULL)
			{
				pcNode = pcChildNode;
				iIndex = pcChildNode->GetFirstIndex();

				if (HasData(pcChildNode))
				{
					psIterator->iKeyLength = GetNodeKey(pcChildNode, psIterator->pvKey, MAX_KEY_SIZE);
					psIterator->iIndex = iIndex;
					return pcChildNode;
				}
				iCount++;
			}
			else
			{
				for (;;)
				{
					iIndex++;
					if (iIndex > pcNode->GetLastIndex())
					{
						pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();
						if (pcParent == NULL)
						{
							return NULL;
						}
						iIndex = pcNode->GetIndexInParent();
						pcNode = pcParent;
					}
					else
					{
						break;
					}
				}
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::StepNext(SIndexTreeFileUnsafeIterator* psIterator)
{
	CIndexTreeNodeFile* pcChildNode;
	CIndexTreeNodeFile* pcParent;
	size				iCount;

	iCount = 0;
	for (;;)
	{
		pcChildNode = ReadNode(psIterator->pcNode, psIterator->iIndex);

		if (pcChildNode != NULL)
		{
			psIterator->pcNode = pcChildNode;
			psIterator->iIndex = pcChildNode->GetFirstIndex();

			if (HasData(pcChildNode))
			{
				return true;
			}
			iCount++;
		}
		else
		{
			for (;;)
			{
				psIterator->iIndex++;
				if (psIterator->iIndex > psIterator->pcNode->GetLastIndex())
				{
					pcParent = (CIndexTreeNodeFile*)psIterator->pcNode->GetParent();
					if (pcParent == NULL)
					{
						return false;
					}
					psIterator->iIndex = psIterator->pcNode->GetIndexInParent();
					psIterator->pcNode = pcParent;
				}
				else
				{
					break;
				}
			}
		}
	}
	//Should probably return something.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::ValidateIndexTree(void)
{
	return ValidateIndexTree(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::ValidateIndexTree(bool bReadNodes)
{
	bool bResult;

	bResult = ValidateMagic(bReadNodes);
	bResult &= ValidateLimits(bReadNodes);
	bResult &= ValidateParentIndex(bReadNodes);
	bResult &= ValidateTransientFlags(bReadNodes);
	bResult &= ValidateFileIndexes(bReadNodes);
	bResult &= ValidateKeys(bReadNodes);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::ValidateKeys(bool bReadNodes)
{
	CIndexTreeRecursor	cCursor;
	bool				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateKeys(&cCursor, bReadNodes);
	cCursor.Kill();

	return bResult;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::RecurseValidateKeys(CIndexTreeRecursor* pcCursor, bool bReadNodes)
{
	CIndexTreeNodeFile*		pcNode;
	size					i;
	CIndexTreeNodeFile*		pcChild;
	bool					bResult;
	CStackMemory<>			cStack;
	uint8*					pcKey;
	size					iKeySize;
	size					uiFirstIndex;
	size					uiLastIndex;

	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		if (pcNode->HasData())
		{
			pcKey = (uint8*)cStack.Init();
			pcCursor->GetKey(pcKey, &iKeySize);

			if (meReverseKey == IKR_Yes)
			{
				StrRev((char*)pcKey, iKeySize);
			}
			else if (meReverseKey == IKR_Unknown)
			{
				gcLogger.Error2(__METHOD__, " Don't know how to order key bytes for direction [IKR_Unknown].", NULL);
				return false;
			}


			bResult = Get(pcKey, iKeySize, NULL, NULL, NULL);
			if (!bResult)
			{
				pcCursor->GenerateBad();
				return gcLogger.Error2(__METHOD__, " Key [", pcCursor->GetBadKey(), "] did not reference a Node.", NULL);
			}
		}

		if (pcNode->HasNodes())
		{
			uiFirstIndex = pcNode->GetFirstIndex();
			uiLastIndex= pcNode->GetLastIndex();
			for (i = uiFirstIndex; i <= uiLastIndex; i++)
			{
				pcChild = ReadNode(pcNode, i, bReadNodes);
				pcCursor->Push(pcChild, (uint8)i);
				bResult = RecurseValidateKeys(pcCursor, bReadNodes);
				if (!bResult)
				{
					pcCursor->Pop();
					return false;
				}
			}
		}
	}
	pcCursor->Pop();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CIndexTreeFile::RecurseNumElements(CIndexTreeNodeFile* pcNode)
{
	size					i;
	CIndexTreeNodeFile*		pcChild;
	size					iLastIndex;
	uint32					count;

	count = 0;
	if (pcNode != NULL)
	{
		if (HasData(pcNode))
		{
			count++;
		}

		iLastIndex = pcNode->GetLastIndex();
		for (i = pcNode->GetFirstIndex(); i <= iLastIndex; i++)
		{
			pcChild = ReadNode(pcNode, i);
			count += RecurseNumElements(pcChild);
		}
	}
	return count;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeFile::RecurseNumMemoryElements(CIndexTreeNodeFile* pcNode)
{
	size					i;
	CIndexTreeNodeFile*		pcChild;
	size					iLastIndex;
	size					count;

	count = 0;
	if (pcNode != NULL)
	{
		if (HasData(pcNode))
		{
			count++;
		}

		iLastIndex = pcNode->GetLastIndex();
		for (i = pcNode->GetFirstIndex(); i <= iLastIndex; i++)
		{
			pcChild = ReadMemoryNode(pcNode, i);
			count += RecurseNumMemoryElements(pcChild);
		}
	}
	return count;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeFile::RecurseNumMemoryElements(CIndexTreeNodeFile* pcNode, size iSize)
{
	size					i;
	CIndexTreeNodeFile*		pcChild;
	size					iLastIndex;
	size					count;

	count = 0;
	if (pcNode != NULL)
	{
		if (HasData(pcNode))
		{
			if (pcNode->GetDataSize() == iSize)
			{
				count++;
			}
		}

		iLastIndex = pcNode->GetLastIndex();
		for (i = pcNode->GetFirstIndex(); i <= iLastIndex; i++)
		{
			pcChild = ReadMemoryNode(pcNode, i);
			count += RecurseNumMemoryElements(pcChild, iSize);
		}
	}
	return count;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeFile::ByteSize(void)
{
	return RecurseByteSize(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CIndexTreeFile::RecurseByteSize(CIndexTreeNodeFile* pcNode)
{
	size					i;
	CIndexTreeNodeFile*		pcChild;
	size					uiSize;
	uint32					uiCount;
	size					uiLastIndex;

	uiCount = 0;
	if (pcNode != NULL)
	{
		uiCount += pcNode->CalculateRequiredNodeSizeForCurrent();
		uiSize = pcNode->GetDataSize();

		uiLastIndex = pcNode->GetLastIndex();
		for (i = pcNode->GetFirstIndex(); i <= uiLastIndex; i++)
		{
			pcChild = ReadNode(pcNode, i);
			uiCount += RecurseByteSize(pcChild);
		}
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::ValidateLimits(bool bReadNodes)
{
	CIndexTreeRecursor	cCursor;
	bool				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateLimits(&cCursor, bReadNodes);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::RecurseValidateLimits(CIndexTreeRecursor* pcCursor, bool bReadNodes)
{
	size					i;
	CIndexTreeNodeFile*		pcChild;
	bool					bResult;
	CIndexTreeChildNode*	pcFirst;
	CIndexTreeChildNode*	pcLast;
	CIndexTreeNodeFile*		pcNode;
	size					uiFirstIndex;
	size					uiLastIndex;

	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		if (pcNode->HasNodes())
		{
			uiFirstIndex = pcNode->GetFirstIndex();
			uiLastIndex = pcNode->GetLastIndex();

			if (pcNode != mpcRoot)
			{
				if (!pcNode->ContainsIndex(uiFirstIndex))
				{
					pcCursor->GenerateBad();
					return gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] did not contain first index [", SizeToString(uiFirstIndex), "].", NULL);
				}
				if (!pcNode->ContainsIndex(uiLastIndex))
				{
					pcCursor->GenerateBad();
					return gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] did not contain last index [", SizeToString(uiLastIndex), "].", NULL);
				}

				pcFirst = pcNode->Get(uiFirstIndex);
				if (pcFirst == NULL)
				{
					pcCursor->GenerateBad();
					return gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] first child [", SizeToString(uiFirstIndex), "] was not a file or memory node.", NULL);
				}

				pcLast = pcNode->Get(uiLastIndex);
				if (pcLast == NULL)
				{
					pcCursor->GenerateBad();
					return gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] last child [", SizeToString(uiFirstIndex), "] was not a file or memory node.", NULL);
				}
			}

			for (i = uiFirstIndex; i <= uiLastIndex; i++)
			{
				pcChild = ReadNode(pcNode, i, bReadNodes);
				pcCursor->Push(pcChild, (uint8)i);
				bResult = RecurseValidateLimits(pcCursor, bReadNodes);
				if (!bResult)
				{
					pcCursor->Pop();
					return false;
				}
			}
		}
	}
	pcCursor->Pop();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::ValidateTransientFlags(bool bReadNodes)
{
	CIndexTreeRecursor	cCursor;
	bool				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateTransientFlags(&cCursor, bReadNodes);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::RecurseValidateTransientFlags(CIndexTreeRecursor* pcCursor, bool bReadNodes)
{
	CIndexTreeNodeFile*		pcNode;
	size					i;
	CIndexTreeNodeFile*		pcChild;
	bool					bResult;
	size					uiTransientFlags;
	CChars					sz;
	size					uiFirstIndex;
	size					uiLastIndex;

	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		uiTransientFlags = pcNode->GetTransientFlags();
		if (pcNode->HasFlags(INDEX_TREE_NODE_TRANSIENT_FLAGS))
		{
			sz.Init();
			pcCursor->GenerateBad();
			gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] has unexpected TRANSIENT_FLAGS [", pcNode->GetFlagsString(&sz),"].", NULL);
			sz.Kill();
			return false;
		}

		if (pcNode->HasNodes())
		{
			uiFirstIndex = pcNode->GetFirstIndex();
			uiLastIndex = pcNode->GetLastIndex();
			for (i = uiFirstIndex; i <= uiLastIndex; i++)
			{
				pcChild = ReadNode(pcNode, i, bReadNodes);
				pcCursor->Push(pcChild, (uint8)i);
				bResult = RecurseValidateTransientFlags(pcCursor, bReadNodes);
				if (!bResult)
				{
					pcCursor->Pop();
					return false;
				}
			}
		}
	}
	pcCursor->Pop();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::ValidateFileIndexes(bool bReadNodes)
{
	CIndexTreeRecursor	cCursor;
	bool				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateFileIndexes(&cCursor, bReadNodes);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::RecurseValidateFileIndexes(CIndexTreeRecursor* pcCursor, bool bReadNodes)
{
	CIndexTreeNodeFile*		pcNode;
	size					i;
	CIndexTreeNodeFile*		pcChild;
	bool					bResult;
	size					uiFirstIndex;
	size					uiLastIndex;

	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		if (!pcNode->GetFileIndex()->HasFile())
		{
			if (pcNode != mpcRoot)  //This is not the right solution.  The root file should be written along with all the configuration for how the tree was created.
			{
				pcCursor->GenerateBad();
				return gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] has no file associated.", NULL);
			}
		}

		if (pcNode->HasNodes())
		{
			uiFirstIndex = pcNode->GetFirstIndex();
			uiLastIndex = pcNode->GetLastIndex();
			for (i = uiFirstIndex; i <= uiLastIndex; i++)
			{
				pcChild = ReadNode(pcNode, i, bReadNodes);
				pcCursor->Push(pcChild, (uint8)i);
				bResult = RecurseValidateFileIndexes(pcCursor, bReadNodes);
				if (!bResult)
				{
					pcCursor->Pop();
					return false;
				}
			}
		}
	}
	pcCursor->Pop();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::ValidateMagic(bool bReadNodes)
{
	CIndexTreeRecursor	cCursor;
	bool				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateMagic(&cCursor, bReadNodes);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::RecurseValidateMagic(CIndexTreeRecursor* pcCursor, bool bReadNodes)
{
	CIndexTreeNodeFile*		pcNode;
	size					i;
	CIndexTreeNodeFile*		pcChild;
	bool					bResult;
	size					uiFirstIndex;
	size					uiLastIndex;

	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		if (!pcNode->IsMagic())
		{
			pcCursor->GenerateBad();
			return gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] is not magic :(", NULL);
		}

		if (pcNode->HasNodes())
		{
			uiFirstIndex = pcNode->GetFirstIndex();
			uiLastIndex = pcNode->GetLastIndex();
			for (i = uiFirstIndex; i <= uiLastIndex; i++)
			{
				pcChild = ReadNode(pcNode, i, bReadNodes);
				pcCursor->Push(pcChild, (uint8)i);
				bResult = RecurseValidateMagic(pcCursor, bReadNodes);
				if (!bResult)
				{
					pcCursor->Pop();
					return false;
				}
			}
		}
	}
	pcCursor->Pop();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::ValidateParentIndex(void)
{
	return ValidateParentIndex(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::ValidateParentIndex(bool bReadNodes)
{
	CIndexTreeRecursor	cCursor;
	bool				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateParentIndex(&cCursor, bReadNodes);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::RecurseValidateParentIndex(CIndexTreeRecursor* pcCursor, bool bReadNodes)
{
	CIndexTreeNodeFile*		pcNode;
	size					i;
	CIndexTreeNodeFile*		pcChild;
	bool					bResult;
	CIndexTreeNodeFile*		pcChildsParent;
	size					uiIndexInParent;
	CIndexTreeNodeFile*		pcShouldBeThis;
	CIndexTreeChildNode*	pcShouldBeChild;
	size					uiFirstIndex;
	size					uiLastIndex;

	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		if (pcNode->HasNodes())
		{
			uiFirstIndex = pcNode->GetFirstIndex();
			uiLastIndex = pcNode->GetLastIndex();
			for (i = uiFirstIndex; i <= uiLastIndex; i++)
			{
				pcChild = ReadNode(pcNode, i, bReadNodes);
				if (pcChild != NULL)
				{
					pcChildsParent = (CIndexTreeNodeFile*)pcChild->GetParent();
					if (pcChildsParent == NULL)
					{
						pcCursor->GenerateBad();
						return gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] parent should not be NULL.", NULL);
					}

					if (pcChildsParent != pcNode)
					{
						pcCursor->GenerateBad();
						return gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] points to the wrong parent node.", NULL);
					}

					uiIndexInParent = pcChild->GetIndexInParent();
					if (i != uiIndexInParent)
					{
						pcCursor->GenerateBad();
						return gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] parent index [", SizeToString(uiIndexInParent), "] is different points to the index in the parent [", SizeToString(i), "].", NULL);
					}
					
					pcShouldBeChild = (((CIndexTreeNodeFile*)pcNode)->GetNode(uiIndexInParent));
					if (pcShouldBeChild->IsMemory())
					{
						pcShouldBeThis = pcShouldBeChild->u.mpcMemory;
						if (pcShouldBeThis != pcChild)
						{
							pcCursor->GenerateBad();
							return gcLogger.Error2(__METHOD__, " This node is not the same as the parents child node for key [", pcCursor->GetBadNode(), "].", NULL);
						}
					}
					else if (pcShouldBeChild->IsFile())
					{
						pcCursor->GenerateBad();
						return gcLogger.Error2(__METHOD__, " This node is a file node on the parents child node for key [", pcCursor->GetBadNode(), "].", NULL);
					}
					else
					{
						pcCursor->GenerateBad();
						return gcLogger.Error2(__METHOD__, " This node is corrupt on the parents child node for key [", pcCursor->GetBadNode(), "].", NULL);
					}

					pcCursor->Push(pcChild, (uint8)i);
					bResult = RecurseValidateParentIndex(pcCursor, bReadNodes);
					if (!bResult)
					{
						pcCursor->Pop();
						return false;
					}
				}
			}

			uiFirstIndex = pcNode->GetFirstIndex();
			uiLastIndex = pcNode->GetLastIndex();
			for (i = uiFirstIndex; i <= uiLastIndex; i++)
			{
				pcChild = ReadNode(pcNode, i, bReadNodes);
				pcCursor->Push(pcChild, (uint8)i);
				bResult = RecurseValidateParentIndex(pcCursor, bReadNodes);
				if (!bResult)
				{
					pcCursor->Pop();
					return false;
				}
			}
		}
	}
	pcCursor->Pop();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::HasKey(uint8* pvKey, size iKeySize)
{
	CIndexTreeNodeFile* pcNode;

	pcNode = GetNode(pvKey, iKeySize);
	if (pcNode == NULL)
	{
		return false;
	}

	if (HasData(pcNode))
	{
		HasKeyReorderData(pcNode);
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
bool CIndexTreeFile::HasData(CIndexTreeNodeFile* pcNode)
{
	if (!pcNode->HasData() ||
		(pcNode->IsDeleted() && !pcNode->IsDirty()))
	{
		return false;
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
size CIndexTreeFile::GetDataSize(uint8* pvKey, size iKeySize)
{
	CIndexTreeNodeFile* pcNode;

	pcNode = GetNode(pvKey, iKeySize);
	if (pcNode == NULL)
	{
		return 0;
	}
	else if (!pcNode->HasData())
	{
		return 0;
	}

	GetReorderData(pcNode);

	return pcNode->GetDataSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CIndexTreeFile::CountListSize(void)
{
	return RecurseCountListSize(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CIndexTreeFile::RecurseCountListSize(CIndexTreeNodeFile* pcNode)
{
	size					i;
	CIndexTreeNodeFile*		pcChild;
	size					uiLastIndex;
	size					count;

	if (pcNode != NULL)
	{
		count = pcNode->NumIndexes();
		uiLastIndex = pcNode->GetLastIndex();
		for (i = pcNode->GetFirstIndex(); i <= uiLastIndex; i++)
		{
			pcChild = ReadNode(pcNode, i);
			count += RecurseCountListSize(pcChild);
		}
		return count;
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
bool CIndexTreeFile::Write(CIndexTreeNodeFile* pcNode)
{
	size					iWrittenPos;
	CStackMemory<>			cTemp;
	void*					pvBuffer;
	size					iFileSize;
	CIndexedFile*			pcNewIndexFile;
	CIndexedFile*			pcOldIndexFile;
	filePos					uiDataIndex;
	CFileDataIndex*			pcIndex;

	iFileSize = pcNode->CalculateFileSize(mpcDataCallback);

	pvBuffer = cTemp.Init(iFileSize);
	iWrittenPos = pcNode->WriteToBuffer(pvBuffer, iFileSize, mpcDataCallback);
	if (iWrittenPos <= 0)
	{
		return false;
	}

	if (iWrittenPos != iFileSize)
	{
		cTemp.Kill();
		return gcLogger.Error2(__METHOD__, " Could not write IndexTreeNodeFile.  Expected size [", SizeToString(iFileSize), "] is not equal to written buffer size [", SizeToString(iWrittenPos), "].", NULL);
	}

	pcIndex = pcNode->GetFileIndex();
	if (pcIndex->HasFile())
	{
		pcOldIndexFile = mcIndexFiles.GetFile(pcIndex->miFile);
	}
	else
	{
		pcOldIndexFile = NULL;
	}

	pcNewIndexFile = mcIndexFiles.GetOrCreateFile(iFileSize);
	if (!pcNewIndexFile)
	{
		cTemp.Kill();
		return false;
	}

	if (pcNewIndexFile != pcOldIndexFile)
	{
		uiDataIndex = pcNewIndexFile->Write(pvBuffer);
		if (uiDataIndex == INDEXED_FILE_WRITE_ERROR)
		{
			cTemp.Kill();
			return false;
		}
		if (pcOldIndexFile != NULL)
		{
			mcIndexFiles.Delete(pcNode->GetFileIndex());
		}
		pcNode->SetFileIndex(pcNewIndexFile->GetFileIndex(), (uint32)uiDataIndex);
	}
	else
	{
		uiDataIndex = pcNewIndexFile->Write(pcNode->GetFileIndex()->muiIndex, pvBuffer);
	}
	cTemp.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::WriteBackPathWriteThrough(CIndexTreeNodeFile* pcNode)
{
	bool				bWrite;
	CIndexTreeNodeFile* pcParent;
	CFileDataIndex		cOldIndex;
	CFileDataIndex*		pcNewIndex;

	while (pcNode)
	{
		pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();
		if (pcNode->IsDirty())
		{
			pcNode->SetDirtyNode(false);

			cOldIndex = *pcNode->GetFileIndex();

			bWrite = Write(pcNode);
			if (!bWrite)
			{
				return false;
			}

			pcNewIndex = pcNode->GetFileIndex();
			if (!cOldIndex.Equals(pcNewIndex))
			{
				if (pcParent)
				{
					pcParent->SetDirtyNode(true);
				}
			}
		}
		else
		{
			break;
		}
		pcNode = pcParent;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::WriteBackPathCaching(CIndexTreeNodeFile* pcNode)
{
	bool				bWrite;
	CIndexTreeNodeFile* pcParent;
	CFileDataIndex		cOldIndex;
	CFileDataIndex*		pcNewIndex;

	while (pcNode)
	{
		pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();
		if (pcNode->IsDirty())
		{
			pcNode->SetDirtyNode(false);

			if (!pcNode->HasChildWithFlags(INDEX_TREE_NODE_FLAG_DIRTY_PATH))
			{
				pcNode->SetDirtyPath(false);
			}

			cOldIndex = *pcNode->GetFileIndex();

			bWrite = Write(pcNode);
			if (!bWrite)
			{
				return false;
			}

			pcNewIndex = pcNode->GetFileIndex();
			if (!cOldIndex.Equals(pcNewIndex))
			{
				if (pcParent)
				{
					pcParent->SetDirtyNode(true);
				}
			}
		}
		else
		{
			return ClearDirtyPath(pcNode);
		}
		pcNode = pcParent;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::ClearDirtyPath(CIndexTreeNodeFile* pcNode)
{
	CIndexTreeNodeFile* pcParent;
	bool				bChildWithDirtyPath;
	bool				bDirtyNode;
	bool				bDirtyPath;

	bDirtyPath = pcNode->IsPathDirty();
	if (!bDirtyPath)
	{
		return true;
	}

	while (pcNode)
	{
		pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();

		bChildWithDirtyPath = pcNode->HasChildWithFlags(INDEX_TREE_NODE_FLAG_DIRTY_PATH);
		bDirtyNode = pcNode->IsDirty();
		if (!bChildWithDirtyPath && !bDirtyNode)
		{
			pcNode->SetDirtyPath(false);
		}
		else
		{
			break;
		}

		pcNode = pcParent;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::ClearDeletedPath(CIndexTreeNodeFile* pcNode)
{
	CIndexTreeNodeFile* pcParent;
	bool				bChildWithDeletedPath;
	bool				bDeletedNode;
	bool				bDeletedPath;

	bDeletedPath = pcNode->IsPathDeleted();
	if (!bDeletedPath)
	{
		return true;
	}

	while (pcNode)
	{
		pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();

		bChildWithDeletedPath = pcNode->HasChildWithFlags(INDEX_TREE_NODE_FLAG_DELETED_PATH);
		bDeletedNode = pcNode->IsDeleted();
		if (!bChildWithDeletedPath && !bDeletedNode)
		{
			pcNode->SetDeletedPath(false);
		}
		else
		{
			break;
		}

		pcNode = pcParent;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::Delete(CIndexTreeNodeFile* pcNode)
{
	CFileDataIndex*		pcIndex;

	pcIndex = pcNode->GetFileIndex();
	if (!pcIndex->HasFile())
	{
		return true;
	}
	return mcIndexFiles.Delete(pcIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::GetFiles(CArrayIndexedFilePtr* papc)
{
	mcIndexFiles.GetFiles(papc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CIndexTreeFile::NumNodes(void)
{
	CIndexTreeRecursor	cCursor;
	uint32				iResult;

	cCursor.Init(mpcRoot);
	iResult = RecurseNumNodes(&cCursor);
	cCursor.Kill();

	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CIndexTreeFile::RecurseNumNodes(CIndexTreeRecursor* pcCursor)
{
	CIndexTreeNodeFile*		pcNode;
	size					i;
	CIndexTreeNodeFile*		pcChild;
	int						iCount;
	size					uiFirstIndex;
	size					uiLastIndex;

	iCount = 0;
	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		iCount++;

		if (pcNode->HasNodes())
		{
			uiFirstIndex = pcNode->GetFirstIndex();
			uiLastIndex = pcNode->GetLastIndex();
			for (i = uiFirstIndex; i <= uiLastIndex; i++)
			{
				pcChild = ReadNode(pcNode, i);
				pcCursor->Push(pcChild, (uint8)i);
				iCount += RecurseNumNodes(pcCursor);
			}
		}
	}
	pcCursor->Pop();
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeFile::NumMemoryNodes(void)
{
	CIndexTreeRecursor	cCursor;
	int					iResult;

	cCursor.Init(mpcRoot);
	iResult = RecurseNumMemoryNodes(&cCursor);
	cCursor.Kill();

	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeFile::RecurseNumMemoryNodes(CIndexTreeRecursor* pcCursor)
{
	CIndexTreeNodeFile*		pcNode;
	size					i;
	CIndexTreeNodeFile*		pcChild;
	size					iCount;
	size					uiFirstIndex;
	size					uiLastIndex;

	iCount = 0;
	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		iCount++;

		if (pcNode->HasNodes())
		{
			uiFirstIndex = pcNode->GetFirstIndex();
			uiLastIndex = pcNode->GetLastIndex();
			for (i = uiFirstIndex; i <= uiLastIndex; i++)
			{
				pcChild = ReadMemoryNode(pcNode, i);
				pcCursor->Push(pcChild, (uint8)i);
				iCount += RecurseNumMemoryNodes(pcCursor);
			}
		}
	}
	pcCursor->Pop();
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexedFiles* CIndexTreeFile::GetIndexFiles(void)
{
	return &mcIndexFiles;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::Read(CIndexTreeChildNode* pcChildNode, CIndexTreeNodeFile* pcFileNodeParent, size uiIndexInParent)
{
	CIndexedFile*			pcFile;
	int						iMaxBufferSize;
	CStackMemory<>			cTemp;
	void*					pvBuffer;
	bool					bResult;
	CIndexTreeNodeFile*		pcFileNode;
	uint32					iFile;
	uint32					uiIndex;
	
	iFile = pcChildNode->u.mcFile.miFile;
	uiIndex = pcChildNode->u.mcFile.muiIndex;

	pcFile = mcIndexFiles.GetFile(iFile);
	iMaxBufferSize = pcFile->GetDataSize();

	pvBuffer = cTemp.Init(iMaxBufferSize);
	bResult = pcFile->Read(uiIndex, pvBuffer);
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not read tree node in file number [", IntToString(iFile), "] name [", StringToString(pcFile->GetFilename()), "] at index [", IntToString(uiIndex), "].", NULL);
		return false;
	}
		
	pcFileNode = AllocateNodeFromBuffer(pcFileNodeParent, uiIndexInParent, pvBuffer, iMaxBufferSize);
	cTemp.Kill();

	if (pcFileNode)
	{
		pcFileNode->SetFileIndex(iFile, uiIndex);
		pcChildNode->Init(pcFileNode);

		if (pcFileNode->GetDataSize() > 0)
		{
			InsertReorderData(pcFileNode);
		}

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
CIndexTreeNodeFile* CIndexTreeFile::ParentPut(CIndexTreeNodeFile* pcParent, size uiIndexInParent, uint8 uiDataSize, uint8 uiFirstIndex, uint8 uiLastIndex)
{
	//Used by the IndexTreeWriter.
	CIndexTreeChildNode*	pcCurrent;
	CIndexTreeNodeFile*		pcNewFileNode;
	bool					bDirty;

	pcCurrent = pcParent->Get(uiIndexInParent);
	if ((pcCurrent == NULL) || pcCurrent->IsUnallocated())
	{
		pcNewFileNode = AllocateNodeRange(pcParent, uiIndexInParent, uiFirstIndex, uiLastIndex, uiDataSize);
		pcNewFileNode->SetDirtyNode(true);

		bDirty = pcParent->SetMemory(uiIndexInParent, pcNewFileNode);
		if (bDirty)
		{
			pcParent->SetDirtyNode(true);
		}
		return pcNewFileNode;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Set [", SizeToString(uiIndexInParent), "].  Should only ever be called with NULL or Unallocated nodes.", NULL);
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::ParentPut(CIndexTreeNodeFile* pcParent, size uiIndexInParent, uint8 uiDataSize)
{
	//Used by the IndexTreeWriter.
	CIndexTreeChildNode*	pcCurrent;
	CIndexTreeNodeFile*		pcNewFileNode;
	bool					bDirty;

	pcCurrent = pcParent->Get(uiIndexInParent);
	if ((pcCurrent == NULL) || pcCurrent->IsUnallocated())
	{
		pcNewFileNode = AllocateNodeSingle(pcParent, uiIndexInParent, uiDataSize);
		pcNewFileNode->SetDirtyNode(true);

		bDirty = pcParent->SetMemory(uiIndexInParent, pcNewFileNode);
		if (bDirty)
		{
			pcParent->SetDirtyNode(true);
		}
		return pcNewFileNode;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Set [", SizeToString(uiIndexInParent), "].  Should only ever be called with NULL or Unallocated nodes.", NULL);
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFile::ValidateKey(uint8* pvKey, size iKeySize)
{
	CIndexTreeNodeFile*		pcCurrent;
	uint8					c;
	size					i;
	CChars					szMemory;
	CChars					szFile;
	CChars					szKey;
	bool					bExecute;

	pcCurrent = mpcRoot;

	szMemory.Init();
	szFile.Init();
	if (!szMemory.Equals(szFile))
	{
		szKey.Init();
		szKey.AppendData((char*)pvKey, iKeySize, 80);
		gcLogger.Error2(__METHOD__, " Key [", szKey.Text(), "] memory indexes do not match file indexes at depth [root]:\n", szMemory.Text(), "\n", szFile.Text(), NULL);
		szKey.Kill();
		return false;
	}
	szMemory.Kill();
	szFile.Kill();

	bExecute = StartKey(&i, iKeySize);
	while (bExecute)
	{
		c = ((uint8*)pvKey)[i];
		if (pcCurrent != NULL)
		{
			szMemory.Init();
			szFile.Init();
			if (!szMemory.Equals(szFile))
			{
				szKey.Init();
				szKey.AppendData((char*)pvKey, iKeySize, 80);
				gcLogger.Error2(__METHOD__, " Key [", szKey.Text(), "] memory indexes do not match file indexes at depth [", SizeToString(i + 1) ,"]:\n", szMemory.Text(), "\n", szFile.Text(), NULL);
				szKey.Kill();
				return false;
			}
			szMemory.Kill();
			szFile.Kill();

			pcCurrent = ReadNode(pcCurrent, c);
		}
		bExecute = LoopKey(&i, iKeySize);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::DebugKey(CChars* pszDest, uint8* pvKey, size iKeySize, bool bSkipRoot, bool bShowFlags, bool bShowSize, bool bKeyAlreadyReversed)
{
	CIndexTreeNodeFile*		pcCurrent;
	uint8					c;
	size					i;
	SIndexTreeDebugNode		sDebugNode;
	bool					bExecute;

	pcCurrent = mpcRoot;

	if (!bSkipRoot)
	{
		DebugNodeChildren(pszDest, pcCurrent, -1, bShowFlags, bShowSize);
	}

	if (bKeyAlreadyReversed)
	{
		i = 0;
		bExecute = i < iKeySize;
	}
	else
	{
		bExecute = StartKey(&i, iKeySize);
	}

	while (bExecute)
	{
		c = ((uint8*)pvKey)[i];
		if (pcCurrent != NULL)
		{
			pcCurrent = DebugNode(pszDest, pcCurrent, c, bShowFlags, bShowSize);
		}
		else
		{
			pszDest->Append("Broken Node: ");
			sDebugNode.InitBroken(c);
			sDebugNode.Print(pszDest);
			pszDest->AppendNewLine();
		}

		if (bKeyAlreadyReversed)
		{
			i++;
			bExecute = i < iKeySize;
		}
		else
		{
			bExecute = LoopKey(&i, iKeySize);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::DebugNode(CChars* pszDest, CIndexTreeNodeFile* pcParent, size uiIndexInParent, bool bShowFlags, bool bShowSize)
{
	CIndexTreeNodeFile*		pcCurrent;
	CIndexTreeChildNode*	pcChild;

	pcChild = pcParent->Get(uiIndexInParent);
	if (pcChild)
	{
		if (pcChild->IsMemory())
		{
			pcCurrent = pcChild->u.mpcMemory;
			DebugNodeChildren(pszDest, pcCurrent, uiIndexInParent, bShowFlags, bShowSize);
			return pcCurrent;
		}
		else if (pcChild->IsFile())
		{
			if (Read(pcChild, pcParent, uiIndexInParent))
			{
				pcCurrent = pcChild->u.mpcMemory;
				DebugNodeChildren(pszDest, pcCurrent, uiIndexInParent, bShowFlags, bShowSize);
				return pcCurrent;
			}
			else
			{
				DebugNode(pszDest, pcChild->u.mcFile.miFile, pcChild->u.mcFile.muiIndex, uiIndexInParent, bShowFlags, bShowSize);
				return NULL;
			}
		}
		else if (pcChild->IsUnallocated())
		{
			//Data for key does not exist.
			return NULL;
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Child node [", SizeToString(uiIndexInParent), "] is corrupt.  Type is [", pcChild->iType, "].", NULL);
			return NULL;
		}
	}
	else
	{
		//Data for key does not exist.
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::DebugNode(CChars* pszDest, uint32 iFile, uint32 uiIndex, size uIndexFromParent, bool bShowFlags, bool bShowSize)
{
	SIndexTreeDebugNode		sDebugNode;

	ReadDebugNode(&sDebugNode, iFile, uiIndex);

	sDebugNode.uiIndexInParent = (uint8)uIndexFromParent;
	sDebugNode.Print(pszDest);
	pszDest->AppendNewLine();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::ReadDebugNode(SIndexTreeDebugNode* psDebugNode, uint32 iFile, uint32 uiIndex)
{
	CIndexedFile*			pcFile;
	int						iDataSize;
	CStackMemory<>			cTemp;
	void*					pvBuffer;
	bool					bResult;

	pcFile = mcIndexFiles.GetFile(iFile);
	iDataSize = pcFile->GetDataSize();

	pvBuffer = cTemp.Init(iDataSize);
	bResult = pcFile->Read(uiIndex, pvBuffer);

	psDebugNode->InitFromBuffer(pvBuffer, iDataSize, iFile, uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AppendIndexTreeFileNodeDescrition(CChars* psz, CIndexTreeNodeFile* pcCurrent, int uIndexFromParent)
{
	char	szArrow[] = " -> ";
	bool	bHasChildren;
	bool	bHasObject;

	if (uIndexFromParent == -1)
	{
		psz->Append("  root -> ");
	}
	else
	{
		if ((uIndexFromParent >= 32) && (uIndexFromParent <= 126))
		{
			psz->Append("  ");
			psz->Append((char)uIndexFromParent);
		}
		else
		{
			if (uIndexFromParent < 10)
			{
				psz->Append("  ");
			}
			else if (uIndexFromParent < 100)
			{
				psz->Append(" ");
			}
			psz->Append(uIndexFromParent);
		}

		bHasObject = pcCurrent->HasData();
		if (bHasObject)
		{
			psz->Append("(X)");
		}
		else
		{
			psz->Append("   ");
		}

		bHasChildren = pcCurrent->HasNodes();
		if (bHasChildren)
		{
			psz->Append(szArrow);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::DebugNodeChildren(CChars* pszDest, CIndexTreeNodeFile* pcCurrent, int uIndexFromParent, bool bShowFlags, bool bShowSize)
{
	CChars	szFile;
	CChars	szMemory;
	CChars	sz;
	CChars	szFlags;

	szMemory.Init();
	AppendIndexTreeFileNodeDescrition(&szMemory, pcCurrent, uIndexFromParent);
	PrintChildFileIndexes(pcCurrent, &szMemory);

	szFlags.Init();
	if (bShowFlags)
	{
		szFlags.Append(" (");
		pcCurrent->GetFlagsString(&szFlags);
		szFlags.Append(")");
	}
	if (bShowSize)
	{
		szFlags.Append(" [");
		szFlags.Append(pcCurrent->CalculateRequiredNodeSizeForCurrent());
		szFlags.Append("]");
	}

	if (pcCurrent->GetFileIndex()->HasFile())
	{
		szFile.Init();
		AppendIndexTreeFileNodeDescrition(&szFile, pcCurrent, uIndexFromParent);
		PrintNodeFileIndexes(pcCurrent, &szFile);

		if (szMemory.Equals(szFile))
		{
			sz.Init();
			sz.Append("   Both: ");
			sz.Append(szMemory);
			pszDest->Append(sz);
			pszDest->Append(&szFlags);
			pszDest->AppendNewLine();

			sz.Kill();
		}
		else
		{
			sz.Init();
			sz.Append("+Memory: ");
			sz.Append(szMemory);
			sz.Append(&szFlags);
			sz.AppendNewLine();

			sz.Append("-  File: ");
			sz.Append(szFile);
			sz.Append(&szFlags);
			sz.AppendNewLine();

			pszDest->Append(sz);
			sz.Kill();
		}

		szFile.Kill();
	}
	else
	{
		sz.Init();
		sz.Append(" Memory: ");
		sz.Append(szMemory);
		sz.Append(&szFlags);
		pszDest->AppendNewLine(sz);
		sz.Kill();
	}

	szFlags.Kill();
	szMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::PrintChildFileIndexes(CIndexTreeNodeFile* pcCurrent, CChars* psz)
{
	size					i;
	CIndexTreeChildNode*	pcChild;
	CIndexTreeChildNode*	acChildren;
	CIndexTreeNodeFile*		pcMemoryChild;
	size					uiFirstIndex;
	size					uiLastIndex;

	acChildren = pcCurrent->GetNodes();

	if (pcCurrent->HasNodes())
	{
		uiFirstIndex = pcCurrent->GetFirstIndex();
		uiLastIndex = pcCurrent->GetLastIndex();
		for (i = 0; i <= uiLastIndex - uiFirstIndex; i++)
		{
			pcChild = &acChildren[i];
			if (pcChild->IsValid())
			{
				if (pcChild->IsMemory())
				{
					pcMemoryChild = pcChild->u.mpcMemory;
					if (pcMemoryChild->GetFileIndex()->HasFile())
					{
						psz->Append(pcMemoryChild->GetFileIndex()->miFile);
						psz->Append(":");
						psz->Append(pcMemoryChild->GetFileIndex()->muiIndex);
					}
					else
					{
						psz->Append("o");
					}
				}
				else
				{
					psz->Append(pcChild->u.mcFile.miFile);
					psz->Append(":");
					psz->Append(pcChild->u.mcFile.muiIndex);
				}
			}
			else
			{
				psz->Append(".");
			}
			psz->Append(" ");
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::PrintNodeFileIndexes(CIndexTreeNodeFile* pcCurrent, CChars* psz)
{
	CIndexedFile*			pcFile;
	int						iDataSize;
	CStackMemory<>			cTemp;
	char*					pucMemory;
	bool					bResult;
	int						iFile;
	uint32					uiIndex;
	int						i;
	SIndexTreeDebugNode		sDebugNode;
	int						iPos;
	uint32					ulliFilePos;

	iFile = pcCurrent->GetFileIndex()->miFile;

	pcFile = mcIndexFiles.GetFile(iFile);
	iDataSize = pcFile->GetDataSize();
	uiIndex = pcCurrent->GetFileIndex()->muiIndex;

	pucMemory = (char*)cTemp.Init(iDataSize);
	bResult = pcFile->Read(uiIndex, pucMemory);

	iPos = sDebugNode.InitFromBuffer(pucMemory, iDataSize, iFile, uiIndex);
	if (!(sDebugNode.sFlags & INDEX_TREE_NODE_FLAG_NODES_EMPTY))
	{
		for (i = 0; i <= sDebugNode.uiLastIndex - sDebugNode.uiFirstIndex; i++)
		{
			iFile = *((int*)&pucMemory[iPos]);  iPos += sizeof(int);
			ulliFilePos = *((uint32*)&pucMemory[iPos]);  iPos += sizeof(uint32);

			if (iFile != -1)
			{
				psz->Append(iFile);
				psz->Append(":");
				psz->Append(ulliFilePos);
			}
			else
			{
				psz->Append(".");
			}
			psz->Append(" ");
		}
	}

	cTemp.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::Dump(void)
{
	CChars				sz;

	sz.Init();
	Print(&sz, true, true);
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::Print(CChars* pszDest, bool bShowFlags, bool bShowSize)
{
	CChars				sz;

	sz.Init();
	sz.Append("= [IndexTreeFile]  ");
	if (bShowSize)
	{
		sz.Append("[");
		sz.Append((uint64)this->GetUserMemorySize());
		sz.Append("] ");
	}

	sz.Append('=', mpcRoot->NumIndexes() * 2 - sz.Length() + 18);
	pszDest->AppendNewLine(sz);
	sz.Kill();

	PrintChildren(pszDest, bShowFlags, bShowSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::PrintChildren(CChars* pszDest, bool bShowFlags, bool bShowSize)
{
	CIndexTreeRecursor	cCursor;

	cCursor.Init(mpcRoot);

	DebugNodeChildren(pszDest, mpcRoot, -1, bShowFlags, bShowSize);
	RecurseDump(pszDest, &cCursor, bShowFlags, bShowSize);

	cCursor.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::RecurseDump(CChars* pszDest, CIndexTreeRecursor* pcCursor, bool bShowFlags, bool bShowSize)
{
	CIndexTreeNodeFile*		pcNode;
	size					i;
	size					iKeySize;
	CIndexTreeNodeFile*		pcChild;
	CStackMemory<32>		cStack;
	uint8*					pvKey;
	CChars					szKey;
	bool					bHasNodes;
	bool					bHasData;
	bool					bHasMemoryNodes;
	size					uiFirstIndex;
	size					uiLastIndex;

	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		bHasMemoryNodes = HasMemoryNodes(pcNode);
		bHasNodes = pcNode->HasNodes();
		bHasData = pcNode->HasData();
		if (bHasData || !bHasMemoryNodes)
		{
			iKeySize = GetNodeKeySize(pcNode);
			pvKey = (uint8*)cStack.Init(iKeySize);
			GetNodeKey(pcNode, (uint8*)pvKey, iKeySize);

			szKey.Init("Key: ------------- [");
			szKey.AppendData2((const char*)pvKey, iKeySize);
			szKey.Append("] -------------");
			pszDest->AppendNewLine(szKey);
			szKey.Kill();

			DebugKey(pszDest, pvKey, iKeySize, true, bShowFlags, bShowSize, false);
			cStack.Kill();
		}

		if (bHasNodes)
		{
			uiFirstIndex = pcNode->GetFirstIndex();
			uiLastIndex = pcNode->GetLastIndex();
			for (i = uiFirstIndex; i <= uiLastIndex; i++)
			{
				pcChild = ReadMemoryNode(pcNode, i);
				pcCursor->Push(pcChild, (uint8)i);
				RecurseDump(pszDest, pcCursor, bShowFlags, bShowSize);
			}
		}
	}
	pcCursor->Pop();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeFile::GetNodeKeySize(CIndexTreeNode* pcNode)
{
	int				iKeySize;

	iKeySize = 0;
	while (pcNode != mpcRoot)
	{
		iKeySize++;
		pcNode = pcNode->GetParent();
	}
	return iKeySize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeFile::GetNodeKey(CIndexTreeNode* pcNode, uint8* pvDestKey, size iDestKeySize)
{
	size	iKeySize;
	size	iLength;

	if (pcNode == NULL)
	{
		return 0;
	}

	iKeySize = 0;
	iLength = 0;
	while (pcNode != mpcRoot)
	{
		if (iKeySize < iDestKeySize)
		{
			((uint8*)pvDestKey)[iLength] = (uint8)pcNode->GetIndexInParent();
			iLength++;
		}
		iKeySize++;
		pcNode = (CIndexTreeNodeFile*)pcNode->GetParent();
	}

	if (meReverseKey == IKR_No)
	{
		//The key is already reversed by revese node traversal.
		ReverseBytes(pvDestKey, iLength);
	}
	else if (meReverseKey == IKR_Unknown)
	{
		gcLogger.Error2(__METHOD__, " Don't know how to order key bytes for direction [IKR_Unknown].", NULL);
		return false;
	}

	if (iKeySize < iDestKeySize)
	{
		((uint8*)pvDestKey)[iKeySize] = 0;
	}

	return iKeySize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeFile::GetNodeDataSize(CIndexTreeNode* pcNode)
{
	if (HasData((CIndexTreeNodeFile*)pcNode))
	{
		return pcNode->GetDataSize();
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
size CIndexTreeFile::GetNodeData(CIndexTreeNode* pcNode, void* pvDestData, size iDestDataSize)
{
	int				iDataSize;
	void*			pvData;

	if (HasData((CIndexTreeNodeFile*)pcNode))
	{
		iDataSize = pcNode->GetDataSize();
		if (pvDestData)
		{
			pvData = pcNode->GetDataPtr();
			memcpy(pvDestData, pvData, MinDataSize(iDataSize, iDestDataSize));
		}
		return iDataSize;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::DiagnosticFlushCallback(CIndexTreeNodeFile* pcNode)
{
	CStackMemory<>	cStackData;
	CStackMemory<>	cStackKey;
	void*			pvData;
	size			uiDataSize;
	size			iKeySize;
	char*			pvKey;

	uiDataSize = GetNodeDataSize(pcNode);
	pvData = cStackData.Init(uiDataSize);
	GetNodeData(pcNode, pvData, uiDataSize);

	iKeySize = GetNodeKeySize(pcNode);
	pvKey = (char*)cStackKey.Init(iKeySize+1);
	GetNodeKey(pcNode, (uint8*)pvKey, iKeySize + 1);

	mpcDiagnosticCallback->Flush(pvKey, iKeySize, pvData, uiDataSize);

	cStackKey.Kill();
	cStackData.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::DiagnosticEvictCallback(CIndexTreeNodeFile* pcNode)
{
	CStackMemory<>	cStackData;
	CStackMemory<>	cStackKey;
	void*			pvData;
	size			uiDataSize;
	size			iKeySize;
	uint8*			pvKey;

	uiDataSize = GetNodeDataSize(pcNode);
	pvData = cStackData.Init(uiDataSize);
	GetNodeData(pcNode, pvData, uiDataSize);

	iKeySize = GetNodeKeySize(pcNode);
	pvKey = (uint8*)cStackKey.Init(iKeySize + 1);
	GetNodeKey(pcNode, pvKey, iKeySize + 1);

	mpcDiagnosticCallback->Evict(pvKey, iKeySize, pvData, uiDataSize);

	cStackKey.Kill();
	cStackData.Kill();
}

