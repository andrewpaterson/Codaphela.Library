#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/StackMemory.h"
#include "IndexedFile.h"
#include "IndexTreeFileDebug.h"
#include "IndexTreeFileDefaultCallback.h"
#include "IndexTreeFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, EIndexKeyReverse eKeyReverse)
{
	return Init(pcDurableFileControl, &gcIndexTreeFileDefaultCallback, &gcSystemAllocator, IWT_Yes, eKeyReverse);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	return Init(pcDurableFileControl, &gcIndexTreeFileDefaultCallback, &gcSystemAllocator, eWriteThrough, eKeyReverse);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, CMallocator* pcMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	return Init(pcDurableFileControl, &gcIndexTreeFileDefaultCallback, pcMalloc, eWriteThrough, eKeyReverse);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, CIndexTreeFileCallback* pcWriterCallback, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	return Init(pcDurableFileControl, pcWriterCallback, &gcSystemAllocator, eWriteThrough, eKeyReverse);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, CIndexTreeFileCallback* pcWriterCallback, CMallocator* pcMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	//The DurableFileControl must be begun before .Init is called and should be ended afterwards.

	BOOL	bResult;

	if (!pcDurableFileControl->IsBegun())
	{
		return gcLogger.Error2(__METHOD__, " DurableFileController.Begin must be called before Init.", NULL);
	}

	mcMalloc.Init(pcMalloc);
	CIndexTree::Init(&mcMalloc, eKeyReverse, sizeof(CIndexTreeNodeFile), sizeof(CIndexTreeChildNode));

	mpcDataCallback = pcWriterCallback;
	mpcRoot = NULL;
	meWriteThrough = eWriteThrough;
	mpcDurableFileControl = pcDurableFileControl;
	mcIndexFiles.Init(mpcDurableFileControl, "IDAT", "Index.IDX", "_Index.IDX");
	bResult = mcIndexFiles.ReadIndexedFileDescriptors();
	if (!bResult)
	{
		return FALSE;
	}

	bResult = InitRoot();
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
void CIndexTreeFile::FakeInit(EIndexKeyReverse eKeyReverse)
{
	CIndexTree::Init(&gcSystemAllocator, eKeyReverse, sizeof(CIndexTreeNodeFile), sizeof(CIndexTreeChildNode));
	mpcRoot = NULL;
	mpcDurableFileControl = NULL;
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

	return TRUE;
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

	if (pcNode != NULL)
	{
		for (i = 0; i < pcNode->NumIndexes(); i++)
		{
			pcChild = pcNode->GetNode(i);
			if (pcChild->IsMemory())
			{
				//Save if dirty.
				pcChildNode = pcChild->u.mpcMemory;
				RecurseKill(pcChildNode);
			}
		}
		FreeNode(pcNode);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::InitRoot(void)
{
	BOOL				bRootIndexExists;
	CFileDataIndex		cRootDataIndex;
	CIndexedFile*		pcRootIndexFile;
	CStackMemory<>		cTemp;
	int					iFileSize;
	void*				pvBuffer;
	BOOL				bResult;

	InitRootIndexFile();
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
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::InitRootIndexFile(void)
{

	CChars	szFileName;
	CChars	szRewriteName;

	szFileName.Init(mpcDurableFileControl->GetDirectory());
	szFileName.Append(FILE_SEPARATOR);
	szFileName.Append("Root.IDX");

	szRewriteName.Init(mpcDurableFileControl->GetRewriteDirectory());
	szRewriteName.Append(FILE_SEPARATOR);
	szRewriteName.Append("_Root.IDX");

	mcRootIndex.Init(mpcDurableFileControl, szFileName.Text(), szRewriteName.Text());

	szFileName.Kill();
	szRewriteName.Kill();
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
BOOL CIndexTreeFile::WriteRootFileIndex(BOOL bRootHasIndex, CFileDataIndex* pcRootIndex)
{
	if (!bRootHasIndex)
	{
		if (pcRootIndex->HasFile())
		{
			return WriteRootFileIndex(pcRootIndex);
		}
		return TRUE;
	}
	return TRUE;
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
	pcNode->Init(this, pcParent, uiFirstIndex, uiLastIndex, uiIndexInParent);
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
BOOL CIndexTreeFile::Get(void* pvKey, int iKeySize, void* pvData, unsigned short* puiDataSize)
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
		uiDataSize = pcNode->GetDataSize();
		if ((uiDataSize == 0) || (pcNode->IsDeleted() && !pcNode->IsDirty()))
		{
			return FALSE;
		}

		if (puiDataSize)
		{
			*puiDataSize = uiDataSize;
		}

		pv = pcNode->GetDataPtr();
		if (pvData)
		{
			memcpy(pvData, pv, uiDataSize);
		}
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Put(void* pvKey, int iKeySize, void* pvData, unsigned short uiDataSize)
{
	CIndexTreeNodeFile*		pcCurrent;
	BOOL					bResult;
	BOOL					bRootHasIndex;

	if (iKeySize == 0)
	{
		return FALSE;
	}

	if ((pvData == NULL) || (uiDataSize == 0))
	{
		return FALSE;
	}

	bRootHasIndex = mpcRoot->GetFileIndex()->HasFile();
	pcCurrent = GetOrAllocateKey(pvKey, iKeySize);

	pcCurrent = SetNodeData(pcCurrent, pvData, uiDataSize);
	if (pcCurrent == NULL)
	{
		return FALSE;
	}


	if (meWriteThrough == IWT_Yes)
	{
		bResult = WriteBackPathWriteThrough(pcCurrent);
	}
	else
	{
		bResult = DirtySetPaths(pcCurrent);
		if (!bResult)
		{
			return FALSE;
		}
	}

	bResult = WriteRootFileIndex(bRootHasIndex, mpcRoot->GetFileIndex());
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::SetNodeData(CIndexTreeNodeFile* pcCurrent, void* pvData, unsigned short uiDataSize)
{
	CIndexTreeNodeFile*		pcReallocatedCurrent;
	unsigned short			uiOriginalSize;

	if (uiDataSize > pcCurrent->GetDataSize())
	{
		pcReallocatedCurrent = ReallocateNodeForLargerData(pcCurrent, uiDataSize);
		pcReallocatedCurrent->SetData(pvData, uiDataSize);
	}
	else if (uiDataSize < pcCurrent->GetDataSize())
	{
		uiOriginalSize = pcCurrent->GetDataSize();
		pcCurrent->SetData(pvData, uiDataSize);
		pcReallocatedCurrent = ReallocateNodeForSmallerData(pcCurrent, uiOriginalSize);
	}
	else
	{
		pcReallocatedCurrent = pcCurrent;
		pcReallocatedCurrent->SetData(pvData, uiDataSize);
	}
	pcReallocatedCurrent->SetDirtyNode(TRUE);

	if (pcReallocatedCurrent == NULL)
	{
		return NULL;
	}
	//The new / old data sizes being different does not imply that the node will have been reallocated.  Always best to check.
	if (pcCurrent != pcReallocatedCurrent)
	{
		pcReallocatedCurrent->SetChildrensParent();
	}

	return pcReallocatedCurrent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::GetOrAllocateKey(void* pvKey, int iKeySize)
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
		pcCurrent = GetOrAllocateChildNode(pcCurrent, c);
		bExecute = LoopKey(&i, iKeySize);
	}
	return pcCurrent;
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
	size_t					tOldNodeSize;

	if (pcNode->ContainsIndex(uiIndex))
	{
		return pcNode;
	}

	tOldNodeSize = pcNode->CalculateRequiredNodeSizeForCurrent();
	tNewNodeSize = pcNode->CalculateRequiredNodeSizeForIndex(uiIndex);

	pcOldNode = pcNode;
	pcNode = (CIndexTreeNodeFile*)Realloc(pcNode, tNewNodeSize, tOldNodeSize);

	pcNode->Contain(uiIndex);
	pcNode->SetDirtyNode(TRUE);

	RemapChildParents(pcOldNode, pcNode);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::ReallocateNodeForLargerData(CIndexTreeNodeFile* pcNode, unsigned short uiDataSize)
{
	size_t					tNewNodeSize;
	size_t					tOldNodeSize;

	tOldNodeSize = pcNode->CalculateRequiredNodeSizeForCurrent();
	tNewNodeSize = pcNode->CalculateRequiredNodeSizeForData(uiDataSize);

	return ReallocateNodeForData(pcNode, tNewNodeSize, tOldNodeSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::ReallocateNodeForSmallerData(CIndexTreeNodeFile* pcNode, unsigned short uiOriginalSize)
{
	size_t					tNewNodeSize;
	size_t					tOldNodeSize;

	tOldNodeSize = pcNode->CalculateRequiredNodeSizeForData(uiOriginalSize);
	tNewNodeSize = pcNode->CalculateRequiredNodeSizeForCurrent();

	return ReallocateNodeForData(pcNode, tNewNodeSize, tOldNodeSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::ReallocateNodeForData(CIndexTreeNodeFile* pcNode, size_t tNewNodeSize, size_t tOldNodeSize)
{
	CIndexTreeNodeFile*	pcOldNode;

	pcOldNode = pcNode;
	pcNode = (CIndexTreeNodeFile*)Realloc(pcNode, tNewNodeSize, tOldNodeSize);
	if (pcNode)
	{
		RemapChildParents(pcOldNode, pcNode);
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
void CIndexTreeFile::RemapChildParents(CIndexTreeNodeFile* pcOldNode, CIndexTreeNodeFile* pcNode)
{
	CIndexTreeNodeFile*		pcParent;

	if (pcOldNode != pcNode)
	{
		pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();
		if (pcParent)
		{
			pcParent->RemapChildNodes(pcOldNode, pcNode);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::GetOrAllocateChildNode(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent)
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
		pcNewFileNode->SetDirtyNode(TRUE);

		bDirty = pcReallocedParent->SetMemory(uiIndexInParent, pcNewFileNode);
		if (bDirty)
		{
			pcReallocedParent->SetDirtyNode(TRUE);
		}

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
			pcNewFileNode->SetDirtyNode(TRUE);

			pcChildNodeOnParent->Init(pcNewFileNode);
			pcParent->SetDirtyNode(TRUE);

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

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return FALSE;
	}

	pcCurrent = GetNode(pvKey, iKeySize);
	if (pcCurrent == NULL)
	{
		return FALSE;
	}

	if (meWriteThrough == IWT_Yes)
	{
		pcDirty = RemoveWriteThrough(pcCurrent);
		if (pcDirty != NULL)
		{
			return WriteBackPathWriteThrough(pcDirty);
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return RemoveWaitForFlush(pcCurrent);
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
	void*					pvData;
	size_t					tOldNodeSize;
	BOOL					bResult;
	BOOL					bDirty;

	if (pcCurrent->GetDataSize() == 0)
	{
		return FALSE;
	}

	bResult = TRUE;

	pvData = ((void**)pcCurrent->GetDataPtr());
	pcNode = pcCurrent;

	pcNode->ClearData();
	pcNode->SetDirtyNode(TRUE);
	while (pcNode)
	{
		pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();
		c = pcNode->GetIndexInParent();

		if (pcNode->IsEmpty())
		{
			tOldNodeSize = pcParent->CalculateRequiredNodeSizeForCurrent();
			if (pcParent != mpcRoot)
			{
				pcParent = ReallocateNodeForUncontainIndex(pcParent, c, tOldNodeSize);  //Sets parent dirty.
			}
			else
			{
				bDirty = pcParent->ClearIndex(c);
				if (bDirty)
				{
					pcParent->SetDirtyNode(TRUE);
				}
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
CIndexTreeNodeFile* CIndexTreeFile::ReallocateNodeForUncontainIndex(CIndexTreeNodeFile* pcNode, unsigned char c, size_t tOldNodeSize)
{
	CIndexTreeNodeFile*		pcOldNode;
	BOOL					bResizeNode;
	size_t					tNewNodeSize;

	bResizeNode = pcNode->ClearIndexAndUncontain(c);
	if (bResizeNode)
	{
		tNewNodeSize = pcNode->CalculateRequiredNodeSizeForCurrent();
		pcOldNode = pcNode;

		pcNode = (CIndexTreeNodeFile*)Realloc(pcNode, tNewNodeSize, tOldNodeSize);
		if (pcOldNode != pcNode)
		{
			pcNode->SetChildrensParent();
			RemapChildParents(pcOldNode, pcNode);
		}
	}
	pcNode->SetDirtyNode(TRUE);

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
	if (!ClearDirtyPath(pcCurrent))
	{
		return FALSE;
	}
	pcCurrent->SetDirtyNode(FALSE);

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
	if (!ClearDeletedPath(pcCurrent))
	{
		return FALSE;
	}
	pcCurrent->SetDeletedNode(FALSE);

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
	BOOL					bResult;

	bResult = CanEvict(pcNode);
	if (!bResult)
	{
		return FALSE;
	}

	if (!meWriteThrough)
	{
		bResult = Flush(&pcNode);
		if (!bResult)
		{
			return FALSE;
		}
	}

	if (pcNode)
	{
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

		bResult = pcParent->ConvertToFileNode(pcNode);
		Free(pcNode);

		if (!bResult)
		{
			return FALSE;
		}

		pcNode = pcParent;

		if (pcNode->HasObject())
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
	BOOL				bRootHasIndex;
	CIndexTreeNodeFile* pcCurrent;
	CIndexTreeNodeFile* pcDirty;
	BOOL				bDeleted;
	BOOL				bDirty;

	pcCurrent = *ppcCurrent;
	if (meWriteThrough == IWT_Yes)
	{
		return gcLogger.Error2(__METHOD__, " Cannot flush an index tree that is write through.", NULL);
	}

	bDeleted = pcCurrent->IsDeleted();
	bDirty = pcCurrent->IsDirty();
	if (bDeleted && bDirty)
	{
		return gcLogger.Error2(__METHOD__, " Cannot flush a node that is both deleted and dirty.", NULL);
	}
	else if (bDeleted)
	{
		*ppcCurrent = NULL;
		pcDirty = RemoveWriteThrough(pcCurrent);
		if (pcDirty)
		{
			bResult = SetDirtyPath(pcDirty);
			WriteBackPathCaching(pcDirty);
			ClearDeletedPath(pcDirty);
			return bResult;
		}
		else
		{
			return FALSE;
		}
	}
	else if (bDirty)
	{
		bRootHasIndex = mpcRoot->GetFileIndex()->HasFile();
		bResult = WriteBackPathCaching(pcCurrent);
		bResult &= WriteRootFileIndex(bRootHasIndex, mpcRoot->GetFileIndex());
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
	return meWriteThrough;
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
BOOL CIndexTreeFile::Flush(void)
{
	BOOL	bResult;
	BOOL	bRootHasIndex;
	
	if (meWriteThrough == IWT_Yes)
	{
		return gcLogger.Error2(__METHOD__, " Cannot flush an index tree that is write through.", NULL);
	}

	bRootHasIndex = mpcRoot->GetFileIndex()->HasFile();

	bResult = FlushDeleted();
	bResult &= FlushDirty();

	bResult &= WriteRootFileIndex(bRootHasIndex, mpcRoot->GetFileIndex());
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
	paszKeys = FindKeys(&apvDeletedNodes);
	iNumNodes = paszKeys->NumElements();
	for (i = 0; i < iNumNodes; i++)
	{
		pszKey = (unsigned char*)paszKeys->Get(i, &iKeySize);
		pcNode = GetNode(pszKey, iKeySize);
		if (!pcNode->IsDirty())
		{
			pcDirty = RemoveWriteThrough(pcNode);
			if (pcDirty)
			{
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
BOOL CIndexTreeFile::IsFlushed(void)
{
	CIndexTreeRecursor	cCursor;
	BOOL				bResult;

	if (!meWriteThrough)
	{
		cCursor.Init(mpcRoot);
		bResult = RecurseIsFlushed(&cCursor);
		cCursor.Kill();

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
BOOL CIndexTreeFile::StartIteration(SIndexTreeFileIterator* psIterator, void** pvData, int* piDataSize)
{
	psIterator->pcNode = mpcRoot;
	psIterator->iIndex = mpcRoot->GetFirstIndex();

	if (StepNext(psIterator))
	{
		if (pvData)
		{
			*pvData = psIterator->pcNode->GetDataPtr();
		}
		if (piDataSize)
		{
			*piDataSize = psIterator->pcNode->GetDataSize();
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
BOOL CIndexTreeFile::Iterate(SIndexTreeFileIterator* psIterator, void** pvData, int* piDataSize)
{
	if (StepNext(psIterator))
	{
		if (pvData)
		{
			*pvData = psIterator->pcNode->GetDataPtr();
		}
		if (piDataSize)
		{
			*piDataSize = psIterator->pcNode->GetDataSize();
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
int CIndexTreeFile::FindKeySize(CIndexTreeNodeFile* pcNode)
{
	//What is the difference between this and GetNodeKeySize
	int				iKeySize;
	CIndexTreeNode*	pcCurrent;

	iKeySize = 0;
	pcCurrent = pcNode;
	while (pcCurrent != mpcRoot)
	{
		iKeySize++;
		pcCurrent = pcCurrent->GetParent();
	}
	return iKeySize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::FindKey(CIndexTreeNodeFile* pcNode, CArrayChar* pacKey)
{
	unsigned char	uiKeyReversed[MAX_KEY_SIZE];
	int				iKeySize;
	int				i;

	if (pcNode != NULL)
	{
		FindKeyReversed(pcNode, &uiKeyReversed[0], &iKeySize);

		for (i = iKeySize - 1; i >= 0; i--)
		{
			pacKey->Add(uiKeyReversed[i]);
		}
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::FindKey(CIndexTreeNodeFile* pcNode, unsigned char* uiKey, int* piKeySize)
{
	if (pcNode != NULL)
	{
		FindKeyReversed(pcNode, uiKey, piKeySize);
		ReverseBytes(uiKey, *piKeySize);
	}
	else
	{
		piKeySize = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::FindKeyReversed(CIndexTreeNodeFile* pcNode, unsigned char* uiKeyReversed, int* piKeySize)
{
	int				iKeySize;
	CIndexTreeNode*	pcCurrent;

	iKeySize = 0;
	pcCurrent = pcNode;
	while (pcCurrent != mpcRoot)
	{
		uiKeyReversed[iKeySize] = pcCurrent->GetIndexInParent();
		iKeySize++;
		pcCurrent = pcCurrent->GetParent();
	}
	*piKeySize = iKeySize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeFile::FindKeysSize(CArrayVoidPtr* apvNodes)
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
		iKeySize = FindKeySize(pcNode);
		iKeysSize += iKeySize;
	}

	return iKeysSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CListTemplateMinimal<char>* CIndexTreeFile::FindKeys(CArrayVoidPtr* apvNodes)
{
	int								i;
	CIndexTreeNodeFile*				pcNode;
	unsigned char					szKey[MAX_KEY_SIZE];
	int								iKeySize;
	int								iNumKeys;
	int								iTotalSize;
	int								iKeysSize;
	CListTemplateMinimal<char>*		paszKeyNames;

	iNumKeys = apvNodes->NumElements();
	iKeysSize = FindKeysSize(apvNodes);

	paszKeyNames = NULL;
	iTotalSize = paszKeyNames->TotalSize(iNumKeys, iKeysSize);
	paszKeyNames = (CListTemplateMinimal<char>*)malloc(iTotalSize);
	paszKeyNames->Init(iNumKeys, iKeysSize);

	for (i = 0; i < iNumKeys; i++)
	{
		pcNode = (CIndexTreeNodeFile*)apvNodes->GetPtr(i);
		FindKey(pcNode, szKey, &iKeySize);

		paszKeyNames->Add((char*)&szKey[0], iKeySize);
	}

	return paszKeyNames;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CListCharsMinimal* CIndexTreeFile::FindStringKeys(CArrayVoidPtr* apvNodes)
{
	int						i;
	CIndexTreeNodeFile*		pcNode;
	unsigned char			szKey[MAX_KEY_SIZE];
	int						iKeySize;
	int						iNumKeys;
	int						iTotalSize;
	int						iKeysSize;
	CListCharsMinimal*		paszKeyNames;

	iNumKeys = apvNodes->NumElements();
	iKeysSize = FindKeysSize(apvNodes);

	paszKeyNames = NULL;
	iTotalSize = paszKeyNames->TotalSize(iNumKeys, iKeysSize);
	paszKeyNames = (CListCharsMinimal*)malloc(iTotalSize);
	paszKeyNames->Init(iNumKeys, iKeysSize);

	for (i = 0; i < iNumKeys; i++)
	{
		pcNode = (CIndexTreeNodeFile*)apvNodes->GetPtr(i);
		FindKey(pcNode, szKey, &iKeySize);

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
	void*					pvData;
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

			pvData = pcChildNode->GetDataPtr();
			if (pvData != NULL)
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
	return bResult;
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
		if (pcNode->HasObject() && !pcNode->IsDeleted())
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
	CIndexTreeNodeFile*		pcChild;
	int						iLastIndex;

	int count = 0;

	if (pcNode != NULL)
	{
		if (pcNode->HasObject() && !pcNode->IsDeleted())
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
			pcCursor->GenerateBad();
			return gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] has no file associated.", NULL);
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

	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		if (pcNode->HasNodes())
		{
			for (i = pcNode->GetFirstIndex(); i <= pcNode->GetLastIndex(); i++)
			{
				pcChild = ReadNode(pcNode, i, bReadNodes);
				if (pcChild != NULL)
				{
					pcChildsParent = (CIndexTreeNodeFile*)pcChild->GetParent();
					if (pcChildsParent != pcNode)
					{
						pcCursor->GenerateBad();
						return gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] points to the wrong parent node.", NULL);
					}

					uiIndexInParent = pcChild->GetIndexInParent();
					if (i != uiIndexInParent)
					{
						pcCursor->GenerateBad();
						return gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] points to the wrong parent node.", NULL);
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

	if (pcNode->GetDataSize() != 0)
	{
		return !pcNode->IsDeleted();
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
BOOL CIndexTreeFile::HasKey(char* pszKey)
{
	int iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	return HasKey(pszKey, iKeySize);
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
	int					iWrittenPos;
	CStackMemory<>		cTemp;
	void*				pvBuffer;
	int					iFileSize;
	CIndexedFile*		pcNewIndexFile;
	CIndexedFile*		pcOldIndexFile;
	unsigned int		uiDataIndex;
	CFileDataIndex*		pcIndex;

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
			break;
		}
		pcNode = pcParent;
	}

	return ClearDirtyPath(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::ClearDirtyPath(CIndexTreeNodeFile* pcNode)
{
	CIndexTreeNodeFile* pcParent;

	while (pcNode)
	{
		pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();
		if (!pcNode->HasChildWithFlags(INDEX_TREE_NODE_FLAG_DIRTY_PATH))
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

	while (pcNode)
	{
		pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();
		if (!pcNode->HasChildWithFlags(INDEX_TREE_NODE_FLAG_DELETED_PATH))
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
void CIndexTreeFile::DebugKey(CChars* pszDest, void* pvKey, int iKeySize, BOOL bSkipRoot, BOOL bShowFlags)
{
	CIndexTreeNodeFile*		pcCurrent;
	unsigned char			c;
	int						i;
	SIndexTreeDebugNode		sDebugNode;
	BOOL					bExecute;

	pcCurrent = mpcRoot;

	if (!bSkipRoot)
	{
		DebugNodeChildren(pszDest, pcCurrent, -1, bShowFlags);
	}

	bExecute = StartKey(&i, iKeySize);
	while (bExecute)
	{
		c = ((unsigned char*)pvKey)[i];
		if (pcCurrent != NULL)
		{
			pcCurrent = DebugNode(pszDest, pcCurrent, c, bShowFlags);
		}
		else
		{
			sDebugNode.InitBroken(c);
			sDebugNode.Dump();
		}
		bExecute = LoopKey(&i, iKeySize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::DebugNode(CChars* pszDest, CIndexTreeNodeFile* pcParent, int uiIndexInParent, BOOL bShowFlags)
{
	CIndexTreeNodeFile*		pcCurrent;
	CIndexTreeChildNode*	pcChild;

	pcChild = pcParent->Get(uiIndexInParent);
	if (pcChild)
	{
		if (pcChild->IsMemory())
		{
			pcCurrent = pcChild->u.mpcMemory;
			DebugNodeChildren(pszDest, pcCurrent, uiIndexInParent, bShowFlags);
			return pcCurrent;
		}
		else if (pcChild->IsFile())
		{
			if (Read(pcChild, pcParent, uiIndexInParent))
			{
				pcCurrent = pcChild->u.mpcMemory;
				DebugNodeChildren(pszDest, pcCurrent, uiIndexInParent, bShowFlags);
				return pcCurrent;
			}
			else
			{
				DebugNode(pszDest, pcChild->u.mcFile.miFile, pcChild->u.mcFile.muiIndex, uiIndexInParent, bShowFlags);
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
void CIndexTreeFile::DebugNode(CChars* pszDest, int iFile, unsigned int uiIndex, int uIndexFromParent, BOOL bShowFlags)
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

		bHasObject = pcCurrent->HasObject();
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
void CIndexTreeFile::DebugNodeChildren(CChars* pszDest, CIndexTreeNodeFile* pcCurrent, int uIndexFromParent, BOOL bShowFlags)
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
	if (!(sDebugNode.sFlags & INDEX_TREE_NODE_FLAG_EMPTY))
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
	Print(&sz, TRUE);
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::Print(CChars* pszDest, BOOL bShowFlags)
{
	CIndexTreeRecursor	cCursor;
	CChars				sz;

	cCursor.Init(mpcRoot);

	sz.Init();
	sz.Append("= [IndexTreeFile]  ");
	sz.Append('=', mpcRoot->NumIndexes() * 2 - 1);
	pszDest->AppendNewLine(sz);
	sz.Kill();

	DebugNodeChildren(pszDest, mpcRoot, -1, bShowFlags);
	RecurseDump(pszDest, &cCursor, bShowFlags);

	cCursor.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::RecurseDump(CChars* pszDest, CIndexTreeRecursor* pcCursor, BOOL bShowFlags)
{
	CIndexTreeNodeFile*		pcNode;
	int						i;
	int						iKeySize;
	CIndexTreeNodeFile*		pcChild;
	CStackMemory<32>		cStack;
	unsigned char*			pvKey;
	CChars					szKey;

	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		if (pcNode->HasObject())
		{
			iKeySize = GetNodeKeySize(pcNode);
			pvKey = (unsigned char*)cStack.Init(iKeySize);
			GetNodeKey(pcNode, pvKey, iKeySize);

			szKey.Init("Key: ------------- [");
			szKey.AppendData2((const char*)pvKey, iKeySize);
			szKey.Append("] -------------");
			pszDest->AppendNewLine(szKey);
			szKey.Kill();

			DebugKey(pszDest, pvKey, iKeySize, TRUE, bShowFlags);
			cStack.Kill();
		}

		if (pcNode->HasNodes())
		{
			for (i = pcNode->GetFirstIndex(); i <= pcNode->GetLastIndex(); i++)
			{
				pcChild = ReadMemoryNode(pcNode, i);
				pcCursor->Push(pcChild, i);
				RecurseDump(pszDest, pcCursor, bShowFlags);
			}
		}
	}
	pcCursor->Pop();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeFile::GetNodeKeySize(CIndexTreeNodeFile* pcNode)
{
	//What is the difference between this and FindKeySize
	int		iKeySize;

	iKeySize = 0;
	while (pcNode)
	{
		iKeySize++;
		pcNode = (CIndexTreeNodeFile*)pcNode->GetParent();
	}
	return iKeySize-1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::GetNodeKey(CIndexTreeNodeFile* pcNode, unsigned char* pvDestKey, int iDestSize)
{
	int				iKeySize;

	iKeySize = 0;
	while (pcNode && pcNode != mpcRoot)
	{
		if (iKeySize >= iDestSize)
		{
			return FALSE;
		}

		pvDestKey[iKeySize] = pcNode->GetIndexInParent();
		iKeySize++;

		pcNode = (CIndexTreeNodeFile*)pcNode->GetParent();
	}

	ReverseBytes(pvDestKey, iKeySize);
	if (iKeySize < iDestSize)
	{
		pvDestKey[iKeySize] = 0;
	}

	return TRUE;
}

