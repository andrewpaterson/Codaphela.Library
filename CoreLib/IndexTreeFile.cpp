#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/StackMemory.h"
#include "BaseLib/FilePathBuilder.h"
#include "IndexedFile.h"
#include "IndexTreeFileDebug.h"
#include "IndexTreeFileDefaultDataCallback.h"
#include "IndexTreeFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, int iMaxDataSize, int iMaxKeySize)
{
	return Init(pcDurableFileControl, szSubDirectory, &gcIndexTreeFileDefaultCallback, &gcSystemAllocator, IWT_Yes, IKR_No, iMaxDataSize, iMaxKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	return Init(pcDurableFileControl, szSubDirectory, &gcIndexTreeFileDefaultCallback, &gcSystemAllocator, eWriteThrough, eKeyReverse, MAX_DATA_SIZE, MAX_KEY_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CIndexTreeDataOrderer* pcDataOrderer)
{
	return Init(pcDurableFileControl, szSubDirectory, &gcIndexTreeFileDefaultCallback, &gcSystemAllocator, eWriteThrough, eKeyReverse, MAX_DATA_SIZE, MAX_KEY_SIZE, pcDataOrderer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CMallocator* pcMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	return Init(pcDurableFileControl, szSubDirectory, &gcIndexTreeFileDefaultCallback, pcMalloc, eWriteThrough, eKeyReverse, MAX_DATA_SIZE, MAX_KEY_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CIndexTreeFileDataCallback* pcWriterCallback, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	return Init(pcDurableFileControl, szSubDirectory, pcWriterCallback, &gcSystemAllocator, eWriteThrough, eKeyReverse, MAX_DATA_SIZE, MAX_KEY_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CIndexTreeFileDataCallback* pcWriterCallback, CMallocator* pcMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CIndexTreeDataOrderer* pcDataOrderer)
{
	return Init(pcDurableFileControl, szSubDirectory, pcWriterCallback, pcMalloc, eWriteThrough, eKeyReverse, MAX_DATA_SIZE, MAX_KEY_SIZE, pcDataOrderer);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CIndexTreeFileDataCallback* pcWriterCallback, CMallocator* pcMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, int iMaxDataSize, int iMaxKeySize)
{
	return Init(pcDurableFileControl, szSubDirectory, pcWriterCallback, pcMalloc, eWriteThrough, eKeyReverse, iMaxDataSize, iMaxKeySize, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CIndexTreeFileDataCallback* pcWriterCallback, CMallocator* pcMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, int iMaxDataSize, int iMaxKeySize, CIndexTreeDataOrderer* pcDataOrderer)
{
	BOOL	bResult;

	if (!pcDurableFileControl->IsBegun())
	{
		return gcLogger.Error2(__METHOD__, " DurableFileController.Begin must be called before Init.", NULL);
	}

	mcMalloc.Init(pcMalloc);
	CIndexTree::Init(&mcMalloc, eKeyReverse, sizeof(CIndexTreeNodeFile), sizeof(CIndexTreeNodeFile) + sizeof(CIndexTreeDataNode), sizeof(CIndexTreeChildNode), iMaxDataSize, iMaxKeySize, pcDataOrderer);

	mpcDiagnosticCallback = NULL;
	mpcDataCallback = pcWriterCallback;
	mpcRoot = NULL;
	meWriteThrough = eWriteThrough;
	mpcDurableFileControl = pcDurableFileControl;
	mcIndexFiles.Init(mpcDurableFileControl, szSubDirectory, "IDAT", "Index.IDX", "_Index.IDX");
	bResult = mcIndexFiles.ReadIndexedFileDescriptors();
	if (!bResult)
	{
		return FALSE;
	}

	bResult = InitRoot(szSubDirectory);
	if (!bResult)
	{
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Kill(void)
{
	if (mpcDurableFileControl->IsBegun())
	{
		return gcLogger.Error2(__METHOD__, " DurableFileController.End must called before Kill.", NULL);
	}

	if (!IsFlushed())
	{
		return gcLogger.Error2(__METHOD__, " Flush must called before Kill.", NULL);
	}

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
BOOL CIndexTreeFile::InitRoot(char* szSubDirectory)
{
	BOOL				bRootIndexExists;
	CFileDataIndex		cRootDataIndex;
	CIndexedFile*		pcRootIndexFile;
	CStackMemory<>		cTemp;
	int					iFileSize;
	void*				pvBuffer;
	BOOL				bResult;
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
			return gcLogger.Error2(__METHOD__, " Root node size [", IntToString(iFileSize), "] does not match root node indexed file size[", IntToString(pcRootIndexFile->GetDataSize()), "]", NULL);
		}

		pvBuffer = cTemp.Init(iFileSize);
		bResult = pcRootIndexFile->Read(cRootDataIndex.muiIndex, pvBuffer, 1);
		if (!bResult)
		{
			return gcLogger.Error2(__METHOD__, " Could not read root node indexed file.", NULL);
		}

		mpcRoot->InitFromBuffer(pvBuffer, iFileSize, mpcDataCallback);
		cTemp.Kill();

		return TRUE;
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
		gcLogger.Error2(__METHOD__, " Index Root file size [", LongLongToString(iSize * sizeof(CFileDataIndex)), "] is incorrect.  Should be SizeOf(CFileDataIndex).", NULL);
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
BOOL CIndexTreeFile::WriteRootFileIndex(CFileDataIndex* pcRootIndex)
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
CIndexTreeNodeFile* CIndexTreeFile::AllocateNodeSingle(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent, unsigned short uiDataSize)
{
	CIndexTreeNodeFile*		pcNode;
	size_t					tSize;

	pcNode = NULL;
	tSize = CalculateNodeSize(0, uiDataSize);
	pcNode = (CIndexTreeNodeFile*)Malloc(tSize);
	pcNode->Init(this, pcParent, uiIndexInParent);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::AllocateNodeRange(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned short uiDataSize)
{
	CIndexTreeNodeFile*		pcNode;
	int						iRequiredIndices;
	size_t					tSize;

	pcNode = NULL;
	iRequiredIndices = pcNode->NumIndexes(uiFirstIndex, uiLastIndex);
	tSize = CalculateNodeSize(iRequiredIndices, uiDataSize);
	pcNode = (CIndexTreeNodeFile*)Malloc(tSize);
	pcNode->Init(this, pcParent, uiFirstIndex, uiLastIndex, uiDataSize, uiIndexInParent);
	return pcNode;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::AllocateNodeFromBuffer(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent, void* pvBuffer, int iMaxBufferSize)
{
	unsigned char*			pucMemory;
	int						iPos;
	int						iNumNodes;
	unsigned short			uiDataSize;
	unsigned char			uiFirstIndex;
	unsigned char			uiLastIndex;
	CIndexTreeNodeFile*		pcNode;
	size_t					tSize;
	int						iBufferRead;
	int						iFileSize;
	int						iFileDataSize;

	pucMemory = (unsigned char*)pvBuffer;
	iPos = 0;
	iFileSize = *((int*)&pucMemory[iPos]);  iPos += sizeof(int);
	iFileDataSize = *((int*)&pucMemory[iPos]);  iPos += sizeof(int);
	uiDataSize = *((unsigned short*)&pucMemory[iPos]);  iPos += sizeof(unsigned short);

	uiFirstIndex = pucMemory[iPos];  iPos++;
	uiLastIndex = pucMemory[iPos];  iPos++;
	iNumNodes = (uiLastIndex - uiFirstIndex) + 1;

	tSize = CalculateNodeSize(iNumNodes, uiDataSize);

	pcNode = (CIndexTreeNodeFile*)Malloc(tSize);
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
CIndexTreeNodeFile* CIndexTreeFile::GetNode(void* pvKey, int iKeySize)
{
	CIndexTreeNodeFile*		pcCurrent;
	unsigned char			c;
	int						i;
	BOOL					bExecute;

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return NULL;
	}

	pcCurrent = mpcRoot;
	bExecute = StartKey(&i, iKeySize);
	while (bExecute)
	{
		c = ((unsigned char*)pvKey)[i];
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
CIndexTreeNodeFile* CIndexTreeFile::GetMemoryNode(void* pvKey, int iKeySize)
{
	CIndexTreeNodeFile*		pcCurrent;
	unsigned char			c;
	int						i;
	BOOL					bExecute;

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return NULL;
	}

	pcCurrent = mpcRoot;
	bExecute = StartKey(&i, iKeySize);
	while (bExecute)
	{
		c = ((unsigned char*)pvKey)[i];
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
CIndexTreeNodeFile* CIndexTreeFile::ReadNode(CIndexTreeNodeFile* pcParent, unsigned char c, BOOL bReadNode)
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
CIndexTreeNodeFile* CIndexTreeFile::ReadNode(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent)
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
				gcLogger.Error2(__METHOD__, " Could not load child node [", IntToString((int)uiIndexInParent), "].", NULL);
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
			gcLogger.Error2(__METHOD__, " Child node [", IntToString((int)uiIndexInParent), "] is corrupt.  Type is [", pcChild->iType, "].", NULL);
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
CIndexTreeNodeFile* CIndexTreeFile::ReadMemoryNode(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent)
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
			gcLogger.Error2(__METHOD__, " Child node [", IntToString((int)uiIndexInParent), "] is corrupt.  Type is [", pcChild->iType, "].", NULL);
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
BOOL CIndexTreeFile::HasMemoryNodes(CIndexTreeNodeFile* pcNode)
{
	int						i;
	CIndexTreeNodeFile*		pcChild;

	for (i = pcNode->GetFirstIndex(); i <= pcNode->GetLastIndex(); i++)
	{
		pcChild = ReadMemoryNode(pcNode, i);
		if (pcChild != NULL)
		{
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Get(void* pvKey, int iKeySize, void* pvData, size_t* piDataSize)
{
	CIndexTreeNodeFile* pcNode;
	void*				pv;
	unsigned short		uiDataSize;

	pcNode = GetNode(pvKey, iKeySize);
	if (pcNode == NULL)
	{
		return FALSE;
	}
	else
	{
		if (HasData(pcNode))
		{
			uiDataSize = pcNode->GetDataSize();

			GetReorderData(pcNode);

			SafeAssign(piDataSize, uiDataSize);

			pv = pcNode->GetDataPtr();
			if (pvData)
			{
				memcpy(pvData, pv, uiDataSize);
			}

			if (mpcDiagnosticCallback)
			{
				mpcDiagnosticCallback->Get(pvKey, iKeySize, pvData, uiDataSize);
			}
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Put(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
	CIndexTreeNodeFile*		pcCurrent;
	BOOL					bResult;
	unsigned short			uiDataSize;
	BOOL					bNewNode;

	ReturnNullOnFalse(ValidatePut(iKeySize, iDataSize));

	uiDataSize = (unsigned short)iDataSize;

	pcCurrent = AllocateKey(pvKey, iKeySize);

	bNewNode = FALSE;
	if (!HasData(pcCurrent))
	{
		bNewNode = TRUE;
	}

	pcCurrent = SetNodeData(pcCurrent, pvData, uiDataSize);
	if (pcCurrent == NULL)
	{
		return FALSE;
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
			return FALSE;
		}
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Don't know how to put data with Write Through [IWT_Unknown].", NULL);
		return FALSE;
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
CIndexTreeNodeFile* CIndexTreeFile::AllocateKey(void* pvKey, int iKeySize)
{
	CIndexTreeNodeFile*		pcCurrent;
	unsigned char			c;
	BOOL					bExecute;
	int						i;

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
CIndexTreeNodeFile* CIndexTreeFile::SetNodeData(CIndexTreeNodeFile* pcCurrent, void* pvData, unsigned short uiDataSize)
{
	CIndexTreeNodeFile*		pcReallocatedCurrent;
	unsigned short			uiOldDataSize;

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

	pcReallocatedCurrent->SetDirtyNode(TRUE);

	return pcReallocatedCurrent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::SetDirtyPath(CIndexTreeNodeFile* pcCurrent)
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
		return FALSE;
	}

	if (pcCurrent->IsDirty())
	{
		while ((pcCurrent) && (!pcCurrent->IsPathDirty()))
		{
			pcCurrent->SetDirtyPath(TRUE);
			pcCurrent = (CIndexTreeNodeFile*)pcCurrent->GetParent();
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::SetDeletedPath(CIndexTreeNodeFile* pcCurrent)
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
		return FALSE;
	}

	if (pcCurrent->IsDeleted())
	{
		return gcLogger.Error2(__METHOD__, " Cannot SetDeletedPath on an index tree file node that is already deleted.", NULL);
	}

	while ((pcCurrent) && (!pcCurrent->IsPathDeleted()))
	{
		pcCurrent->SetDeletedPath(TRUE);
		pcCurrent = (CIndexTreeNodeFile*)pcCurrent->GetParent();
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::ReallocateNodeForContainIndex(CIndexTreeNodeFile* pcNode, unsigned char uiIndex)
{
	CIndexTreeNodeFile*		pcOldNode;
	size_t					tNewNodeSize;

	if (pcNode->ContainsIndex(uiIndex))
	{
		return pcNode;
	}

	tNewNodeSize = pcNode->CalculateRequiredNodeSizeForIndex(uiIndex);

	pcOldNode = pcNode;
	pcNode = (CIndexTreeNodeFile*)Realloc(pcNode, tNewNodeSize);

	pcNode->Contain(uiIndex, INDEX_TREE_FILE_NODE_UNALLOCATED);
	//pcNode->SetDirtyNode(TRUE);

	RemapNodePointers(pcOldNode, pcNode);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::ReallocateNodeForLargerData(CIndexTreeNodeFile* pcNode, void* pvData, unsigned short uiDataSize)
{
	size_t					tNewNodeSize;
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
CIndexTreeNodeFile* CIndexTreeFile::ReallocateNodeForSmallerData(CIndexTreeNodeFile* pcNode, void* pvData, unsigned short uiDataSize)
{
	size_t					tNewNodeSize;
	unsigned short			uiOriginalSize;
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
CIndexTreeNodeFile* CIndexTreeFile::AllocateChildNode(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent)
{
	CIndexTreeNodeFile*		pcNewFileNode;
	CIndexTreeChildNode*	pcChildNodeOnParent;
	CIndexTreeNodeFile*		pcReallocedParent;
	BOOL					bDirty;

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
				gcLogger.Error2(__METHOD__, " Could not load child node [", IntToString((int)uiIndexInParent), "].", NULL);
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
			gcLogger.Error2(__METHOD__, " Child node [", IntToString((int)uiIndexInParent), "] is corrupt.", NULL);
			return NULL;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Flush(void* pvKey, int iKeySize)
{
	CIndexTreeNodeFile*	pcNode;
	BOOL				bResult;

	pcNode = GetMemoryNode(pvKey, iKeySize);
	if (!pcNode)
	{
		return FALSE;
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
BOOL CIndexTreeFile::ValidateKey(char* pszKey)
{
	int iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	return ValidateKey(pszKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Remove(void* pvKey, int iKeySize)
{
	CIndexTreeNodeFile*		pcCurrent;
	CIndexTreeNodeFile*		pcDirty;
	unsigned short			uiOldDataSize;
	void*					pvData;
	void*					pv;
	CStackMemory<>			cStack;

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return FALSE;
	}

	pcCurrent = GetNode(pvKey, iKeySize);
	if (pcCurrent == NULL)
	{
		return FALSE;
	}

	uiOldDataSize = pcCurrent->GetDataSize();
	if (uiOldDataSize == 0)
	{
		return FALSE;
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
			pcDirty->SetDirtyNode(TRUE);
			return WriteBackPathWriteThrough(pcDirty);
		}
		else
		{
			return FALSE;
		}
	}
	else if (meWriteThrough == IWT_No)
	{
		return RemoveWaitForFlush(pcCurrent);
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Don't know how to remove data with Write Through [IWT_Unknown].", NULL);
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::RemoveWriteThrough(CIndexTreeNodeFile* pcCurrent)
{
	unsigned char			c;
	CIndexTreeNodeFile*		pcParent;
	CIndexTreeNodeFile*		pcNode;
	BOOL					bResult;

	if (pcCurrent->GetDataSize() == 0)
	{
		return FALSE;
	}

	bResult = TRUE;

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
CIndexTreeNodeFile* CIndexTreeFile::ReallocateNodeForUncontainIndex(CIndexTreeNodeFile* pcNode, unsigned char c)
{
	CIndexTreeNodeFile*		pcOldNode;
	BOOL					bResizeNode;
	size_t					tNewNodeSize;

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
BOOL CIndexTreeFile::RemoveWaitForFlush(CIndexTreeNodeFile* pcCurrent)
{
	if (pcCurrent->GetDataSize() == 0)
	{
		return FALSE;
	}

	if (pcCurrent->IsDeleted())
	{
		return FALSE;
	}

	return RemoveSetPaths(pcCurrent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::RemoveSetPaths(CIndexTreeNodeFile* pcCurrent)
{
	pcCurrent->SetDirtyNode(FALSE);
	if (!ClearDirtyPath(pcCurrent))
	{
		return FALSE;
	}

	if (!SetDeletedPath(pcCurrent))
	{
		return FALSE;
	}
	pcCurrent->SetDeletedNode(TRUE);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::DirtySetPaths(CIndexTreeNodeFile* pcCurrent)
{
	pcCurrent->SetDeletedNode(FALSE);
	if (!ClearDeletedPath(pcCurrent))
	{
		return FALSE;
	}

	if (!SetDirtyPath(pcCurrent))
	{
		return FALSE;
	}
	pcCurrent->SetDirtyNode(TRUE);

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Evict(void* pvKey, int iKeySize)
{
	CIndexTreeNodeFile*		pcCurrent;

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return FALSE;
	}

	pcCurrent = GetMemoryNode(pvKey, iKeySize);
	if (pcCurrent == NULL)
	{
		return FALSE;
	}

	return Evict(pcCurrent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Evict(CIndexTreeNodeFile* pcNode)
{
	BOOL	bResult;

	pcNode->ValidateParent();
	bResult = CanEvict(pcNode);
	if (!bResult)
	{
		return FALSE;
	}

	if (meWriteThrough == IWT_No)
	{
		bResult = Flush(&pcNode);
		if (!bResult)
		{
			return FALSE;
		}
	}
	else if (meWriteThrough == IWT_Yes)
	{
	}
	else 
	{
		gcLogger.Error2(__METHOD__, " Don't know how to evict data with Write Through [IWT_Unknown].", NULL);
		return FALSE;
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
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::EvictNode(CIndexTreeNodeFile* pcCurrent)
{
	CIndexTreeNodeFile*		pcParent;
	CIndexTreeNodeFile*		pcNode;
	BOOL					bOnlyFileNodes;
	int						iCount;
	BOOL					bResult;

	pcNode = pcCurrent;

	iCount = 0;
	while (pcNode != mpcRoot)
	{
		pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();

		bOnlyFileNodes = pcNode->HasOnlyFileNodes();
		if (!bOnlyFileNodes)
		{
			return TRUE;
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
			return FALSE;
		}

		pcNode = pcParent;

		if (pcNode->HasData())  //Not HasData(pcNode).
		{
			break;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Flush(CIndexTreeNodeFile** ppcCurrent)
{
	BOOL				bResult;
	CIndexTreeNodeFile* pcCurrent;
	CIndexTreeNodeFile* pcDirty;
	BOOL				bDeleted;
	BOOL				bDirty;

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
		return FALSE;
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
		pcCurrent->SetDeletedNode(FALSE);
		bResult = ClearDeletedPath(pcCurrent);
		pcDirty = RemoveWriteThrough(pcCurrent);
		if (pcDirty != NULL)
		{
			pcDirty->SetDirtyNode(TRUE);
			bResult &= SetDirtyPath(pcDirty);
			bResult &= WriteBackPathCaching(pcDirty);
			return bResult;
		}
		else
		{
			return FALSE;
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
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::CanEvict(CIndexTreeNodeFile* pcNode)
{
	if (!pcNode->HasOnlyFileNodes())
	{
		gcLogger.Error2(__METHOD__, " Cannot evict node with children.", NULL);
		return FALSE;
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
void CIndexTreeFile::FindWithFlags(CArrayVoidPtr* papNodes, unsigned char uiFollowFlags, unsigned char uiAddFlags)
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
void CIndexTreeFile::RecurseFindWithFlags(CIndexTreeRecursor* pcCursor, unsigned char uiFollowFlags, unsigned char uiAddFlags, CArrayVoidPtr* papNodes)
{
	int						i;
	CIndexTreeNodeFile*		pcChild;
	unsigned char			iFirst;
	unsigned char			iLast;
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
					pcCursor->Push(pcChild, i);
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
void CIndexTreeFile::ClearNodesFlags(CArrayVoidPtr* papNodes, unsigned char uiFlags)
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
size_t CIndexTreeFile::GetUserMemorySize(void)
{
	return mcMalloc.AllocatedUserSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeFile::GetSystemMemorySize(void)
{
	return mcMalloc.AllocatedSystemSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::IsWriteThrough(void)
{
	if (meWriteThrough == IWT_Yes)
	{
		return TRUE;
	}
	else if (meWriteThrough == IWT_No)
	{
		return FALSE;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Don't know how to flush data with Write Through [IWT_Unknown].", NULL);
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::HasDiagnosticCallback(void)
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
int CIndexTreeFile::NumElements(void)
{
	int		iNumElements;

	iNumElements = RecurseNumElements(mpcRoot);
	return iNumElements;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeFile::NumMemoryElements(void)
{
	return RecurseNumMemoryElements(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeFile::NumMemoryElements(size_t iSize)
{
	return RecurseNumMemoryElements(mpcRoot, iSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Flush(void)
{
	BOOL	bResult;
	
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
		return FALSE;
	}

	bResult = FlushDeleted();
	bResult &= FlushDirty();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::FlushDirty(void)
{
	CIndexTreeRecursor	cCursor;
	BOOL				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseFlushDirty(&cCursor);
	cCursor.Kill();

	return bResult;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::RecurseFlushDirty(CIndexTreeRecursor* pcCursor)
{
	CIndexTreeNodeFile*		pcNode;
	int						i;
	CIndexTreeNodeFile*		pcChild;
	BOOL					bResult;

	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if ((pcNode != NULL) && pcNode->IsPathDirty())
	{
		if (pcNode->HasNodes())
		{
			for (i = pcNode->GetFirstIndex(); i <= pcNode->GetLastIndex(); i++)
			{
				pcChild = ReadNode(pcNode, i);
				pcCursor->Push(pcChild, i);
				bResult = RecurseFlushDirty(pcCursor);
				if (!bResult)
				{
					pcCursor->Pop();
					return FALSE;
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
			return FALSE;
		}

		pcNode->ClearFlags(INDEX_TREE_NODE_FLAG_DIRTY_PATH | INDEX_TREE_NODE_FLAG_DIRTY_NODE);
	}
	pcCursor->Pop();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::FlushDeleted(void)
{
	CArrayVoidPtr					apvDeletedNodes;
	int								i;
	int								iNumNodes;
	CIndexTreeNodeFile*				pcNode;
	CIndexTreeNodeFile*				pcDirty;
	BOOL							bResult;
	CListTemplateMinimal<char>*		paszKeys;
	int								iKeySize;
	unsigned char*					pszKey;

	apvDeletedNodes.Init();
	FindWithFlags(&apvDeletedNodes, INDEX_TREE_NODE_FLAG_DELETED_PATH, INDEX_TREE_NODE_FLAG_DELETED_NODE);

	ClearNodesFlags(&apvDeletedNodes, INDEX_TREE_NODE_FLAG_DELETED_PATH | INDEX_TREE_NODE_FLAG_DELETED_NODE);

	bResult = TRUE;
	paszKeys = GetNodesKeys(&apvDeletedNodes);
	iNumNodes = paszKeys->NumElements();
	for (i = 0; i < iNumNodes; i++)
	{
		pszKey = (unsigned char*)paszKeys->Get(i, (size_t*)&iKeySize);
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
				pcDirty->SetDirtyNode(TRUE);
				SetDirtyPath(pcDirty);
			}
			else
			{
				bResult = FALSE;
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
BOOL CIndexTreeFile::IsFlushed(void)
{
	CIndexTreeRecursor	cCursor;
	BOOL				bResult;

	if (meWriteThrough == IWT_No)
	{
		cCursor.Init(mpcRoot);
		bResult = RecurseIsFlushed(&cCursor);
		cCursor.Kill();

		return bResult;
	}
	else if (meWriteThrough == IWT_Yes)
	{
		return TRUE;
	}
	else 
	{
		gcLogger.Error2(__METHOD__, " Don't know how to flush data with Write Through [IWT_Unknown].", NULL);
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::RecurseIsFlushed(CIndexTreeRecursor* pcCursor)
{
	CIndexTreeNodeFile*		pcNode;
	int						i;
	CIndexTreeNodeFile*		pcChild;
	BOOL					bResult;
	unsigned char			uiTransientFlags;

	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		uiTransientFlags = pcNode->GetTransientFlags();
		if (pcNode->HasFlags(INDEX_TREE_NODE_TRANSIENT_FLAGS))
		{
			return FALSE;
		}

		if (pcNode->HasNodes())
		{
			for (i = pcNode->GetFirstIndex(); i <= pcNode->GetLastIndex(); i++)
			{
				pcChild = ReadMemoryNode(pcNode, i);
				pcCursor->Push(pcChild, i);
				bResult = RecurseIsFlushed(pcCursor);
				if (!bResult)
				{
					pcCursor->Pop();
					return FALSE;
				}
			}
		}
	}
	pcCursor->Pop();
	return TRUE;
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
BOOL CIndexTreeFile::StartIteration(SIndexTreeFileIterator* psIterator, void* pvKey, int* piKeySize, void* pvData, size_t* piDataSize)
{
	psIterator->pcNode = mpcRoot;
	psIterator->iIndex = mpcRoot->GetFirstIndex();

	return Iterate(psIterator, pvKey, piKeySize, pvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Iterate(SIndexTreeFileIterator* psIterator, void* pvKey, int* piKeySize, void* pvData, size_t* piDataSize)
{
	int		iKeySize;

	if (StepNext(psIterator))
	{
		*piDataSize = psIterator->pcNode->GetDataSize();
		memcmp_fast(pvData, psIterator->pcNode->GetDataPtr(), *piDataSize);
		iKeySize = GetNodeKey(psIterator->pcNode, (char*)pvKey, MAX_KEY_SIZE);
		*piKeySize = iKeySize;

		GetReorderData(psIterator->pcNode);

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
BOOL CIndexTreeFile::StartUnsafeIteration(SIndexTreeFileIterator* psIterator, char* pvKey, int* piKeySize, void** ppvData, size_t* piDataSize)
{
	psIterator->pcNode = mpcRoot;
	psIterator->iIndex = mpcRoot->GetFirstIndex();

	return UnsafeIterate(psIterator, pvKey, piKeySize, ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::UnsafeIterate(SIndexTreeFileIterator* psIterator, char* pvKey, int* piKeySize, void** pvData, size_t* piDataSize)
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
			iKeySize = GetNodeKey(psIterator->pcNode, pvKey, MAX_KEY_SIZE);
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
BOOL CIndexTreeFile::StartUnsafeIteration(SIndexTreeFileIterator* psIterator, void** ppvData, size_t* piDataSize)
{
	return StartUnsafeIteration(psIterator, NULL, NULL, ppvData, piDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::UnsafeIterate(SIndexTreeFileIterator* psIterator, void** ppvData, size_t* piDataSize)
{
	return UnsafeIterate(psIterator, NULL, NULL, ppvData, piDataSize);
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
		iKeySize = GetNodeKey(pcNode, szKey, MAX_KEY_SIZE);

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
		iKeySize = GetNodeKey(pcNode, szKey, MAX_KEY_SIZE);

		paszKeyNames->Add((char*)&szKey[0], iKeySize);
	}

	return paszKeyNames;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::StepNext(SIndexTreeFileIterator* psIterator)
{
	CIndexTreeNodeFile*		pcChildNode;
	CIndexTreeNodeFile*		pcParent;
	int						iCount;

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
				return TRUE;
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
						return FALSE;
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
BOOL CIndexTreeFile::ValidateIndexTree(void)
{
	return ValidateIndexTree(TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::ValidateIndexTree(BOOL bReadNodes)
{
	BOOL bResult;

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
BOOL CIndexTreeFile::ValidateKeys(BOOL bReadNodes)
{
	CIndexTreeRecursor	cCursor;
	BOOL				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateKeys(&cCursor, bReadNodes);
	cCursor.Kill();

	return bResult;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::RecurseValidateKeys(CIndexTreeRecursor* pcCursor, BOOL bReadNodes)
{
	CIndexTreeNodeFile*		pcNode;
	int						i;
	CIndexTreeNodeFile*		pcChild;
	BOOL					bResult;
	CStackMemory<>			cStack;
	char*					pcKey;
	int						iKeySize;
	size_t					iDataSize;

	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		if (pcNode->HasData())
		{
			pcKey = (char*)cStack.Init();
			pcCursor->GetKey(pcKey, &iKeySize);

			if (meReverseKey == IKR_Yes)
			{
				StrRev(pcKey, iKeySize);
			}
			else if (meReverseKey == IKR_Unknown)
			{
				gcLogger.Error2(__METHOD__, " Don't know how to order key bytes for direction [IKR_Unknown].", NULL);
				return FALSE;
			}


			bResult = Get(pcKey, iKeySize, NULL, &iDataSize);
			if (!bResult)
			{
				pcCursor->GenerateBad();
				return gcLogger.Error2(__METHOD__, " Key [", pcCursor->GetBadKey(), "] did not reference a Node.", NULL);
			}
		}

		if (pcNode->HasNodes())
		{
			for (i = pcNode->GetFirstIndex(); i <= pcNode->GetLastIndex(); i++)
			{
				pcChild = ReadNode(pcNode, i, bReadNodes);
				pcCursor->Push(pcChild, i);
				bResult = RecurseValidateKeys(pcCursor, bReadNodes);
				if (!bResult)
				{
					pcCursor->Pop();
					return FALSE;
				}
			}
		}
	}
	pcCursor->Pop();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeFile::RecurseNumElements(CIndexTreeNodeFile* pcNode)
{
	int						i;
	CIndexTreeNodeFile*		pcChild;
	int						iLastIndex;

	int count = 0;

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
int CIndexTreeFile::RecurseNumMemoryElements(CIndexTreeNodeFile* pcNode)
{
	int						i;
	CIndexTreeNodeFile* pcChild;
	int						iLastIndex;

	int count = 0;

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
int CIndexTreeFile::RecurseNumMemoryElements(CIndexTreeNodeFile* pcNode, size_t iSize)
{
	int						i;
	CIndexTreeNodeFile*		pcChild;
	int						iLastIndex;

	int count = 0;

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
size_t CIndexTreeFile::ByteSize(void)
{
	return RecurseByteSize(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeFile::RecurseByteSize(CIndexTreeNodeFile* pcNode)
{
	int						i;
	CIndexTreeNodeFile*		pcChild;
	unsigned short			uiSize;
	size_t					tSize;
	int						iLastIndex;

	tSize = 0;
	if (pcNode != NULL)
	{
		tSize += pcNode->CalculateRequiredNodeSizeForCurrent();
		uiSize = pcNode->GetDataSize();

		iLastIndex = pcNode->GetLastIndex();
		for (i = pcNode->GetFirstIndex(); i <= iLastIndex; i++)
		{
			pcChild = ReadNode(pcNode, i);
			tSize += RecurseByteSize(pcChild);
		}
	}
	return tSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::ValidateLimits(BOOL bReadNodes)
{
	CIndexTreeRecursor	cCursor;
	BOOL				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateLimits(&cCursor, bReadNodes);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::RecurseValidateLimits(CIndexTreeRecursor* pcCursor, BOOL bReadNodes)
{
	int						i;
	CIndexTreeNodeFile*		pcChild;
	unsigned char			iFirst;
	unsigned char			iLast;
	BOOL					bResult;
	CIndexTreeChildNode*	pcFirst;
	CIndexTreeChildNode*	pcLast;
	CIndexTreeNodeFile*		pcNode;

	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		if (pcNode->HasNodes())
		{
			iFirst = pcNode->GetFirstIndex();
			iLast = pcNode->GetLastIndex();

			if (pcNode != mpcRoot)
			{
				if (!pcNode->ContainsIndex(iFirst))
				{
					pcCursor->GenerateBad();
					return gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] did not contain first index [", IntToString(iFirst), "].", NULL);
				}
				if (!pcNode->ContainsIndex(iLast))
				{
					pcCursor->GenerateBad();
					return gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] did not contain last index [", IntToString(iLast), "].", NULL);
				}

				pcFirst = pcNode->Get(iFirst);
				if (pcFirst == NULL)
				{
					pcCursor->GenerateBad();
					return gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] first child [", IntToString(iFirst), "] was not a file or memory node.", NULL);
				}

				pcLast = pcNode->Get(iLast);
				if (pcLast == NULL)
				{
					pcCursor->GenerateBad();
					return gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] last child [", IntToString(iFirst), "] was not a file or memory node.", NULL);
				}
			}

			for (i = iFirst; i <= iLast; i++)
			{
				pcChild = ReadNode(pcNode, i, bReadNodes);
				pcCursor->Push(pcChild, i);
				bResult = RecurseValidateLimits(pcCursor, bReadNodes);
				if (!bResult)
				{
					pcCursor->Pop();
					return FALSE;
				}
			}
		}
	}
	pcCursor->Pop();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::ValidateTransientFlags(BOOL bReadNodes)
{
	CIndexTreeRecursor	cCursor;
	BOOL				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateTransientFlags(&cCursor, bReadNodes);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::RecurseValidateTransientFlags(CIndexTreeRecursor* pcCursor, BOOL bReadNodes)
{
	CIndexTreeNodeFile*		pcNode;
	int						i;
	CIndexTreeNodeFile*		pcChild;
	BOOL					bResult;
	unsigned char			uiTransientFlags;
	CChars					sz;

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
			return FALSE;
		}

		if (pcNode->HasNodes())
		{
			for (i = pcNode->GetFirstIndex(); i <= pcNode->GetLastIndex(); i++)
			{
				pcChild = ReadNode(pcNode, i, bReadNodes);
				pcCursor->Push(pcChild, i);
				bResult = RecurseValidateTransientFlags(pcCursor, bReadNodes);
				if (!bResult)
				{
					pcCursor->Pop();
					return FALSE;
				}
			}
		}
	}
	pcCursor->Pop();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::ValidateFileIndexes(BOOL bReadNodes)
{
	CIndexTreeRecursor	cCursor;
	BOOL				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateFileIndexes(&cCursor, bReadNodes);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::RecurseValidateFileIndexes(CIndexTreeRecursor* pcCursor, BOOL bReadNodes)
{
	CIndexTreeNodeFile*		pcNode;
	int						i;
	CIndexTreeNodeFile*		pcChild;
	BOOL					bResult;

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
			for (i = pcNode->GetFirstIndex(); i <= pcNode->GetLastIndex(); i++)
			{
				pcChild = ReadNode(pcNode, i, bReadNodes);
				pcCursor->Push(pcChild, i);
				bResult = RecurseValidateFileIndexes(pcCursor, bReadNodes);
				if (!bResult)
				{
					pcCursor->Pop();
					return FALSE;
				}
			}
		}
	}
	pcCursor->Pop();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::ValidateMagic(BOOL bReadNodes)
{
	CIndexTreeRecursor	cCursor;
	BOOL				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateMagic(&cCursor, bReadNodes);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::RecurseValidateMagic(CIndexTreeRecursor* pcCursor, BOOL bReadNodes)
{
	CIndexTreeNodeFile*		pcNode;
	int						i;
	CIndexTreeNodeFile*		pcChild;
	BOOL					bResult;

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
			for (i = pcNode->GetFirstIndex(); i <= pcNode->GetLastIndex(); i++)
			{
				pcChild = ReadNode(pcNode, i, bReadNodes);
				pcCursor->Push(pcChild, i);
				bResult = RecurseValidateMagic(pcCursor, bReadNodes);
				if (!bResult)
				{
					pcCursor->Pop();
					return FALSE;
				}
			}
		}
	}
	pcCursor->Pop();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::ValidateParentIndex(void)
{
	return ValidateParentIndex(TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::ValidateParentIndex(BOOL bReadNodes)
{
	CIndexTreeRecursor	cCursor;
	BOOL				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateParentIndex(&cCursor, bReadNodes);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::RecurseValidateParentIndex(CIndexTreeRecursor* pcCursor, BOOL bReadNodes)
{
	CIndexTreeNodeFile*		pcNode;
	int						i;
	CIndexTreeNodeFile*		pcChild;
	BOOL					bResult;
	CIndexTreeNodeFile*		pcChildsParent;
	unsigned char			uiIndexInParent;
	CIndexTreeNodeFile*		pcShouldBeThis;
	CIndexTreeChildNode*	pcShouldBeChild;
	int						iFirstIndex;
	int						iLastIndex;

	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		if (pcNode->HasNodes())
		{
			iFirstIndex = pcNode->GetFirstIndex();
			iLastIndex = pcNode->GetLastIndex();
			for (i = iFirstIndex; i <= iLastIndex; i++)
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
						return gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] parent index [", IntToString(uiIndexInParent), "] is different points to the index in the parent [", IntToString(i), "].", NULL);
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

					pcCursor->Push(pcChild, i);
					bResult = RecurseValidateParentIndex(pcCursor, bReadNodes);
					if (!bResult)
					{
						pcCursor->Pop();
						return FALSE;
					}
				}
			}

			for (i = pcNode->GetFirstIndex(); i <= pcNode->GetLastIndex(); i++)
			{
				pcChild = ReadNode(pcNode, i, bReadNodes);
				pcCursor->Push(pcChild, i);
				bResult = RecurseValidateParentIndex(pcCursor, bReadNodes);
				if (!bResult)
				{
					pcCursor->Pop();
					return FALSE;
				}
			}
		}
	}
	pcCursor->Pop();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::HasKey(void* pvKey, int iKeySize)
{
	CIndexTreeNodeFile* pcNode;

	pcNode = GetNode(pvKey, iKeySize);
	if (pcNode == NULL)
	{
		return FALSE;
	}

	if (HasData(pcNode))
	{
		HasKeyReorderData(pcNode);
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
BOOL CIndexTreeFile::HasData(CIndexTreeNodeFile* pcNode)
{
	if (!pcNode->HasData() ||
		(pcNode->IsDeleted() && !pcNode->IsDirty()))
	{
		return FALSE;
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
unsigned short CIndexTreeFile::GetDataSize(void* pvKey, int iKeySize)
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
int CIndexTreeFile::CountListSize(void)
{
	return RecurseCountListSize(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeFile::RecurseCountListSize(CIndexTreeNodeFile* pcNode)
{
	int						i;
	CIndexTreeNodeFile*		pcChild;
	int						iLastIndex;
	int						count;

	if (pcNode != NULL)
	{
		count = pcNode->NumIndexes();
		iLastIndex = pcNode->GetLastIndex();
		for (i = pcNode->GetFirstIndex(); i <= iLastIndex; i++)
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
BOOL CIndexTreeFile::Write(CIndexTreeNodeFile* pcNode)
{
	int						iWrittenPos;
	CStackMemory<>			cTemp;
	void*					pvBuffer;
	int						iFileSize;
	CIndexedFile*			pcNewIndexFile;
	CIndexedFile*			pcOldIndexFile;
	unsigned int			uiDataIndex;
	CFileDataIndex*			pcIndex;

	iFileSize = pcNode->CalculateFileSize(mpcDataCallback);

	pvBuffer = cTemp.Init(iFileSize);
	iWrittenPos = pcNode->WriteToBuffer(pvBuffer, iFileSize, mpcDataCallback);
	if (iWrittenPos <= 0)
	{
		return FALSE;
	}

	if (iWrittenPos != iFileSize)
	{
		cTemp.Kill();
		return gcLogger.Error2(__METHOD__, " Could not write IndexTreeNodeFile.  Expected size [", IntToString(iFileSize), "] is not equal to written buffer size [", IntToString(iWrittenPos), "].", NULL);
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
		return FALSE;
	}

	if (pcNewIndexFile != pcOldIndexFile)
	{
		uiDataIndex = pcNewIndexFile->Write(pvBuffer);
		if (uiDataIndex == INDEXED_FILE_WRITE_ERROR)
		{
			cTemp.Kill();
			return FALSE;
		}
		if (pcOldIndexFile != NULL)
		{
			mcIndexFiles.Delete(pcNode->GetFileIndex());
		}
		pcNode->SetFileIndex(pcNewIndexFile->GetFileIndex(), uiDataIndex);
	}
	else
	{
		uiDataIndex = pcNewIndexFile->Write(pcNode->GetFileIndex()->muiIndex, pvBuffer);
	}
	cTemp.Kill();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::WriteBackPathWriteThrough(CIndexTreeNodeFile* pcNode)
{
	BOOL				bWrite;
	CIndexTreeNodeFile* pcParent;
	CFileDataIndex		cOldIndex;
	CFileDataIndex*		pcNewIndex;

	while (pcNode)
	{
		pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();
		if (pcNode->IsDirty())
		{
			pcNode->SetDirtyNode(FALSE);

			cOldIndex = *pcNode->GetFileIndex();

			bWrite = Write(pcNode);
			if (!bWrite)
			{
				return FALSE;
			}

			pcNewIndex = pcNode->GetFileIndex();
			if (!cOldIndex.Equals(pcNewIndex))
			{
				if (pcParent)
				{
					pcParent->SetDirtyNode(TRUE);
				}
			}
		}
		else
		{
			break;
		}
		pcNode = pcParent;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::WriteBackPathCaching(CIndexTreeNodeFile* pcNode)
{
	BOOL				bWrite;
	CIndexTreeNodeFile* pcParent;
	CFileDataIndex		cOldIndex;
	CFileDataIndex*		pcNewIndex;

	while (pcNode)
	{
		pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();
		if (pcNode->IsDirty())
		{
			pcNode->SetDirtyNode(FALSE);

			if (!pcNode->HasChildWithFlags(INDEX_TREE_NODE_FLAG_DIRTY_PATH))
			{
				pcNode->SetDirtyPath(FALSE);
			}

			cOldIndex = *pcNode->GetFileIndex();

			bWrite = Write(pcNode);
			if (!bWrite)
			{
				return FALSE;
			}

			pcNewIndex = pcNode->GetFileIndex();
			if (!cOldIndex.Equals(pcNewIndex))
			{
				if (pcParent)
				{
					pcParent->SetDirtyNode(TRUE);
				}
			}
		}
		else
		{
			return ClearDirtyPath(pcNode);
		}
		pcNode = pcParent;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::ClearDirtyPath(CIndexTreeNodeFile* pcNode)
{
	CIndexTreeNodeFile* pcParent;
	BOOL				bChildWithDirtyPath;
	BOOL				bDirtyNode;
	BOOL				bDirtyPath;

	bDirtyPath = pcNode->IsPathDirty();
	if (!bDirtyPath)
	{
		return TRUE;
	}

	while (pcNode)
	{
		pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();

		bChildWithDirtyPath = pcNode->HasChildWithFlags(INDEX_TREE_NODE_FLAG_DIRTY_PATH);
		bDirtyNode = pcNode->IsDirty();
		if (!bChildWithDirtyPath && !bDirtyNode)
		{
			pcNode->SetDirtyPath(FALSE);
		}
		else
		{
			break;
		}

		pcNode = pcParent;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::ClearDeletedPath(CIndexTreeNodeFile* pcNode)
{
	CIndexTreeNodeFile* pcParent;
	BOOL				bChildWithDeletedPath;
	BOOL				bDeletedNode;
	BOOL				bDeletedPath;

	bDeletedPath = pcNode->IsPathDeleted();
	if (!bDeletedPath)
	{
		return TRUE;
	}

	while (pcNode)
	{
		pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();

		bChildWithDeletedPath = pcNode->HasChildWithFlags(INDEX_TREE_NODE_FLAG_DELETED_PATH);
		bDeletedNode = pcNode->IsDeleted();
		if (!bChildWithDeletedPath && !bDeletedNode)
		{
			pcNode->SetDeletedPath(FALSE);
		}
		else
		{
			break;
		}

		pcNode = pcParent;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Delete(CIndexTreeNodeFile* pcNode)
{
	CFileDataIndex*		pcIndex;

	pcIndex = pcNode->GetFileIndex();
	if (!pcIndex->HasFile())
	{
		return TRUE;
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
int CIndexTreeFile::NumNodes(void)
{
	CIndexTreeRecursor	cCursor;
	BOOL				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseNumNodes(&cCursor);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeFile::RecurseNumNodes(CIndexTreeRecursor* pcCursor)
{
	CIndexTreeNodeFile*		pcNode;
	int						i;
	CIndexTreeNodeFile*		pcChild;
	int						iCount;

	iCount = 0;
	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		iCount++;

		if (pcNode->HasNodes())
		{
			for (i = pcNode->GetFirstIndex(); i <= pcNode->GetLastIndex(); i++)
			{
				pcChild = ReadNode(pcNode, i);
				pcCursor->Push(pcChild, i);
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
int CIndexTreeFile::NumMemoryNodes(void)
{
	CIndexTreeRecursor	cCursor;
	BOOL				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseNumMemoryNodes(&cCursor);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeFile::RecurseNumMemoryNodes(CIndexTreeRecursor* pcCursor)
{
	CIndexTreeNodeFile*		pcNode;
	int						i;
	CIndexTreeNodeFile*		pcChild;
	int						iCount;

	iCount = 0;
	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		iCount++;

		if (pcNode->HasNodes())
		{
			for (i = pcNode->GetFirstIndex(); i <= pcNode->GetLastIndex(); i++)
			{
				pcChild = ReadMemoryNode(pcNode, i);
				pcCursor->Push(pcChild, i);
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
BOOL CIndexTreeFile::Read(CIndexTreeChildNode* pcChildNode, CIndexTreeNodeFile* pcFileNodeParent, unsigned char uiIndexInParent)
{
	CIndexedFile*			pcFile;
	int						iMaxBufferSize;
	CStackMemory<>			cTemp;
	void*					pvBuffer;
	BOOL					bResult;
	CIndexTreeNodeFile*		pcFileNode;
	int						iFile;
	unsigned int			uiIndex;
	
	iFile = pcChildNode->u.mcFile.miFile;
	uiIndex = pcChildNode->u.mcFile.muiIndex;

	pcFile = mcIndexFiles.GetFile(iFile);
	iMaxBufferSize = pcFile->GetDataSize();

	pvBuffer = cTemp.Init(iMaxBufferSize);
	bResult = pcFile->Read(uiIndex, pvBuffer);
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not read tree node in file number [", IntToString(iFile), "] name [", StringToString(pcFile->GetFileName()), "] at index [", IntToString(uiIndex), "].", NULL);
		return FALSE;
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
CIndexTreeNodeFile* CIndexTreeFile::ParentPut(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent, unsigned char uiDataSize, unsigned char uiFirstIndex, unsigned char uiLastIndex)
{
	//Used by the IndexTreeWriter.
	CIndexTreeChildNode*	pcCurrent;
	CIndexTreeNodeFile*		pcNewFileNode;
	BOOL					bDirty;

	pcCurrent = pcParent->Get(uiIndexInParent);
	if ((pcCurrent == NULL) || pcCurrent->IsUnallocated())
	{
		pcNewFileNode = AllocateNodeRange(pcParent, uiIndexInParent, uiFirstIndex, uiLastIndex, uiDataSize);
		pcNewFileNode->SetDirtyNode(TRUE);

		bDirty = pcParent->SetMemory(uiIndexInParent, pcNewFileNode);
		if (bDirty)
		{
			pcParent->SetDirtyNode(TRUE);
		}
		return pcNewFileNode;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Set [", CharToString(uiIndexInParent), "].  Should only ever be called with NULL or Unallocated nodes.", NULL);
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::ParentPut(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent, unsigned char uiDataSize)
{
	//Used by the IndexTreeWriter.
	CIndexTreeChildNode*	pcCurrent;
	CIndexTreeNodeFile*		pcNewFileNode;
	BOOL					bDirty;

	pcCurrent = pcParent->Get(uiIndexInParent);
	if ((pcCurrent == NULL) || pcCurrent->IsUnallocated())
	{
		pcNewFileNode = AllocateNodeSingle(pcParent, uiIndexInParent, uiDataSize);
		pcNewFileNode->SetDirtyNode(TRUE);

		bDirty = pcParent->SetMemory(uiIndexInParent, pcNewFileNode);
		if (bDirty)
		{
			pcParent->SetDirtyNode(TRUE);
		}
		return pcNewFileNode;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Set [", CharToString(uiIndexInParent), "].  Should only ever be called with NULL or Unallocated nodes.", NULL);
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::ValidateKey(void* pvKey, int iKeySize)
{
	CIndexTreeNodeFile*		pcCurrent;
	unsigned char			c;
	int						i;
	CChars					szMemory;
	CChars					szFile;
	CChars					szKey;
	BOOL					bExecute;

	pcCurrent = mpcRoot;

	szMemory.Init();
	szFile.Init();
	if (!szMemory.Equals(szFile))
	{
		szKey.Init();
		szKey.AppendData((char*)pvKey, iKeySize, 80);
		gcLogger.Error2(__METHOD__, " Key [", szKey.Text(), "] memory indexes do not match file indexes at depth [root]:\n", szMemory.Text(), "\n", szFile.Text(), NULL);
		szKey.Kill();
		return FALSE;
	}
	szMemory.Kill();
	szFile.Kill();

	bExecute = StartKey(&i, iKeySize);
	while (bExecute)
	{
		c = ((unsigned char*)pvKey)[i];
		if (pcCurrent != NULL)
		{
			szMemory.Init();
			szFile.Init();
			if (!szMemory.Equals(szFile))
			{
				szKey.Init();
				szKey.AppendData((char*)pvKey, iKeySize, 80);
				gcLogger.Error2(__METHOD__, " Key [", szKey.Text(), "] memory indexes do not match file indexes at depth [", IntToString(i+1) ,"]:\n", szMemory.Text(), "\n", szFile.Text(), NULL);
				szKey.Kill();
				return FALSE;
			}
			szMemory.Kill();
			szFile.Kill();

			pcCurrent = ReadNode(pcCurrent, c);
		}
		bExecute = LoopKey(&i, iKeySize);
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::DebugKey(CChars* pszDest, void* pvKey, int iKeySize, BOOL bSkipRoot, BOOL bShowFlags, BOOL bShowSize, BOOL bKeyAlreadyReversed)
{
	CIndexTreeNodeFile*		pcCurrent;
	unsigned char			c;
	int						i;
	SIndexTreeDebugNode		sDebugNode;
	BOOL					bExecute;

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
		c = ((unsigned char*)pvKey)[i];
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
CIndexTreeNodeFile* CIndexTreeFile::DebugNode(CChars* pszDest, CIndexTreeNodeFile* pcParent, int uiIndexInParent, BOOL bShowFlags, BOOL bShowSize)
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
			gcLogger.Error2(__METHOD__, " Child node [", IntToString((int)uiIndexInParent), "] is corrupt.  Type is [", pcChild->iType, "].", NULL);
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
void CIndexTreeFile::DebugNode(CChars* pszDest, int iFile, unsigned int uiIndex, int uIndexFromParent, BOOL bShowFlags, BOOL bShowSize)
{
	SIndexTreeDebugNode		sDebugNode;

	ReadDebugNode(&sDebugNode, iFile, uiIndex);

	sDebugNode.uiIndexInParent = uIndexFromParent;
	sDebugNode.Print(pszDest);
	pszDest->AppendNewLine();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::ReadDebugNode(SIndexTreeDebugNode* psDebugNode, int iFile, unsigned int uiIndex)
{
	CIndexedFile*			pcFile;
	int						iDataSize;
	CStackMemory<>			cTemp;
	void*					pvBuffer;
	BOOL					bResult;

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
	BOOL	bHasChildren;
	BOOL	bHasObject;

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
void CIndexTreeFile::DebugNodeChildren(CChars* pszDest, CIndexTreeNodeFile* pcCurrent, int uIndexFromParent, BOOL bShowFlags, BOOL bShowSize)
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
	int						i;
	CIndexTreeChildNode*	pcChild;
	CIndexTreeChildNode*	acChildren;
	CIndexTreeNodeFile*		pcMemoryChild;

	acChildren = pcCurrent->GetNodes();

	if (pcCurrent->HasNodes())
	{
		for (i = 0; i <= pcCurrent->GetLastIndex() - pcCurrent->GetFirstIndex(); i++)
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
	BOOL					bResult;
	int						iFile;
	unsigned int			uiIndex;
	int						i;
	SIndexTreeDebugNode		sDebugNode;
	int						iPos;
	unsigned int			ulliFilePos;

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
			ulliFilePos = *((unsigned int*)&pucMemory[iPos]);  iPos += sizeof(unsigned int);

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
	Print(&sz, TRUE, TRUE);
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::Print(CChars* pszDest, BOOL bShowFlags, BOOL bShowSize)
{
	CChars				sz;

	sz.Init();
	sz.Append("= [IndexTreeFile]  ");
	if (bShowSize)
	{
		sz.Append("[");
		sz.Append((unsigned long long int)this->GetUserMemorySize());
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
void CIndexTreeFile::PrintChildren(CChars* pszDest, BOOL bShowFlags, BOOL bShowSize)
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
void CIndexTreeFile::RecurseDump(CChars* pszDest, CIndexTreeRecursor* pcCursor, BOOL bShowFlags, BOOL bShowSize)
{
	CIndexTreeNodeFile*		pcNode;
	int						i;
	int						iKeySize;
	CIndexTreeNodeFile*		pcChild;
	CStackMemory<32>		cStack;
	char		*			pvKey;
	CChars					szKey;
	BOOL					bHasNodes;
	BOOL					bHasData;
	BOOL					bHasMemoryNodes;

	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		bHasMemoryNodes = HasMemoryNodes(pcNode);
		bHasNodes = pcNode->HasNodes();
		bHasData = pcNode->HasData();
		if (bHasData || !bHasMemoryNodes)
		{
			iKeySize = GetNodeKeySize(pcNode);
			pvKey = (char*)cStack.Init(iKeySize);
			GetNodeKey(pcNode, pvKey, iKeySize);

			szKey.Init("Key: ------------- [");
			szKey.AppendData2((const char*)pvKey, iKeySize);
			szKey.Append("] -------------");
			pszDest->AppendNewLine(szKey);
			szKey.Kill();

			DebugKey(pszDest, pvKey, iKeySize, TRUE, bShowFlags, bShowSize, FALSE);
			cStack.Kill();
		}

		if (bHasNodes)
		{
			for (i = pcNode->GetFirstIndex(); i <= pcNode->GetLastIndex(); i++)
			{
				pcChild = ReadMemoryNode(pcNode, i);
				pcCursor->Push(pcChild, i);
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
int CIndexTreeFile::GetNodeKeySize(CIndexTreeNode* pcNode)
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
int CIndexTreeFile::GetNodeKey(CIndexTreeNode* pcNode, char* pvDestKey, int iDestKeySize)
{
	int				iKeySize;
	int				iLength;

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
			pvDestKey[iLength] = pcNode->GetIndexInParent();
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
		return FALSE;
	}

	if (iKeySize < iDestKeySize)
	{
		pvDestKey[iKeySize] = 0;
	}

	return iKeySize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeFile::GetNodeDataSize(CIndexTreeNode* pcNode)
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
int CIndexTreeFile::GetNodeData(CIndexTreeNode* pcNode, void* pvDestData, int iDestDataSize)
{
	int				iDataSize;
	void*			pvData;

	if (HasData((CIndexTreeNodeFile*)pcNode))
	{
		iDataSize = pcNode->GetDataSize();
		pvData = pcNode->GetDataPtr();
		if (iDataSize > iDestDataSize)
		{
			memcpy_fast(pvDestData, pvData, iDestDataSize);
		}
		else
		{
			memcpy_fast(pvDestData, pvData, iDataSize);
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
	unsigned short	uiDataSize;
	int				iKeySize;
	char*			pvKey;

	uiDataSize = GetNodeDataSize(pcNode);
	pvData = cStackData.Init(uiDataSize);
	GetNodeData(pcNode, pvData, uiDataSize);

	iKeySize = GetNodeKeySize(pcNode);
	pvKey = (char*)cStackKey.Init(iKeySize+1);
	GetNodeKey(pcNode, pvKey, iKeySize + 1);

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
	void* pvData;
	unsigned short	uiDataSize;
	int				iKeySize;
	char* pvKey;

	uiDataSize = GetNodeDataSize(pcNode);
	pvData = cStackData.Init(uiDataSize);
	GetNodeData(pcNode, pvData, uiDataSize);

	iKeySize = GetNodeKeySize(pcNode);
	pvKey = (char*)cStackKey.Init(iKeySize + 1);
	GetNodeKey(pcNode, pvKey, iKeySize + 1);

	mpcDiagnosticCallback->Evict(pvKey, iKeySize, pvData, uiDataSize);

	cStackKey.Kill();
	cStackData.Kill();
}

