#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/StackMemory.h"
#include "IndexedFile.h"
#include "IndexTreeFileDebug.h"
#include "IndexTreeFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl)
{
	return Init(pcDurableFileControl, &gcSystemAllocator, TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, BOOL bWriteThrough)
{
	return Init(pcDurableFileControl, &gcSystemAllocator, bWriteThrough);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, CMallocator* pcMalloc, BOOL bWriteThrough)
{
	//The DurableFileControl must be begun before .Init is called and should be ended afterwards.

	BOOL	bResult;

	if (!pcDurableFileControl->IsBegun())
	{
		return gcLogger.Error2(__METHOD__, " DurableFileController must be begun before calling Init.", NULL);
	}

	CIndexTree::Init(pcMalloc, sizeof(CIndexTreeNodeFile), sizeof(CIndexTreeChildNode));

	mbWriteThrough = bWriteThrough;
	mpcDurableFileControl = pcDurableFileControl;
	mcIndexFiles.Init(mpcDurableFileControl, "IDAT", "Index.IDX", mpcDurableFileControl->IsDurable() ? "_Index.IDX" : NULL);
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
void CIndexTreeFile::FakeInit(void)
{
	CIndexTree::Init(&gcSystemAllocator, sizeof(CIndexTreeNodeFile), sizeof(CIndexTreeChildNode));
	mpcRoot = NULL;
	mpcDurableFileControl = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::Kill(void)
{
	mcRootIndex.Kill();
	mcIndexFiles.Kill();
	mpcDurableFileControl = NULL;
	RecurseKill(mpcRoot);
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
		for (i = 0; i < pcNode->GetNumIndexes(); i++)
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
	CFileDataIndex		cRootFileIndex;
	CIndexedFile*		pcRootIndexFile;
	CStackMemory<>		cTemp;
	int					iNodeSize;
	void*				pvBuffer;
	BOOL				bResult;

	InitRootIndexFile();
	cRootFileIndex = ReadRootFileIndex();

	bRootIndexExists = cRootFileIndex.HasFile();
	if (bRootIndexExists)
	{
		//The data size on the root is always zero.
		mpcRoot = AllocateRoot(cRootFileIndex);
		iNodeSize = mpcRoot->CalculateBufferSize();

		pcRootIndexFile = mcIndexFiles.GetFile(cRootFileIndex.miFile);
		if (pcRootIndexFile == NULL)
		{
			return gcLogger.Error2(__METHOD__, " Could not get root node indexed file [", IntToString(cRootFileIndex.miFile), "]", NULL);
		}

		if (iNodeSize != pcRootIndexFile->GetDataSize())
		{
			return gcLogger.Error2(__METHOD__, " Root node size [", IntToString(iNodeSize), "] does not match root node indexed file size[", IntToString(pcRootIndexFile->GetDataSize()), "]", NULL);
		}

		pvBuffer = cTemp.Init(iNodeSize);
		bResult = pcRootIndexFile->Read(cRootFileIndex.muiIndex, pvBuffer, 1);
		if (!bResult)
		{
			return gcLogger.Error2(__METHOD__, " Could not read root node indexed file.", NULL);
		}

		mpcRoot->InitFromBuffer(pvBuffer, iNodeSize);
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
CIndexTreeNodeFile* CIndexTreeFile::AllocateNode(CIndexTreeNodeFile* pcParent, unsigned short uiDataSize, unsigned char uiIndexInParent)
{
	CIndexTreeNodeFile*	pcNode;

	pcNode = (CIndexTreeNodeFile*)Malloc(SizeofNode() + uiDataSize);
	pcNode->Init(this, pcParent, uiIndexInParent);
	pcNode->SetDirtyNode(TRUE);

	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::AllocateNode(CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned short uiDataSize, unsigned char uiIndexInParent)
{
	CIndexTreeNodeFile*		pcNode;
	int						iRequiredIndices;
	size_t					tSize;

	pcNode = NULL;
	iRequiredIndices = pcNode->GetNumIndexes(uiFirstIndex, uiLastIndex);
	tSize = CalculateNodeSize(iRequiredIndices, uiDataSize);
	pcNode = (CIndexTreeNodeFile*)Malloc(tSize);
	pcNode->Init(this, pcParent, uiFirstIndex, uiLastIndex, uiDataSize, uiIndexInParent);
	pcNode->SetDirtyNode(TRUE);

	return pcNode;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::AllocateNode(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent, void* pvBuffer, int iBufferSize)
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

	pucMemory = (unsigned char*)pvBuffer;
	iPos = sizeof(int);;

	uiDataSize = *((unsigned short*)&pucMemory[iPos]);  iPos += sizeof(unsigned short);

	uiFirstIndex = pucMemory[iPos];  iPos++;
	uiLastIndex = pucMemory[iPos];  iPos++;
	iNumNodes = (uiLastIndex - uiFirstIndex) + 1;

	tSize = CalculateNodeSize(iNumNodes, uiDataSize);

	pcNode = (CIndexTreeNodeFile*)Malloc(tSize);
	pcNode->Init(this, pcParent, uiFirstIndex, uiLastIndex, uiIndexInParent);
	iBufferRead = pcNode->InitFromBuffer(pvBuffer, iBufferSize);
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

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return NULL;
	}

	pcCurrent = mpcRoot;
	for (i = 0; i < iKeySize; i++)
	{
		c = ((unsigned char*)pvKey)[i];
		pcCurrent = ReadNode(pcCurrent, c);
		if (pcCurrent == NULL)
		{
			return NULL;
		}
	}
	return pcCurrent;
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
			gcLogger.Error2(__METHOD__, " Child node [", IntToString((int)uiIndexInParent), "] is corrupt.  Type is [", pcChild->u.msType.iType, "].", NULL);
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
BOOL CIndexTreeFile::Get(void* pvKey, int iKeySize, void* pvObject, unsigned short* puiDataSize)
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
		uiDataSize = pcNode->GetObjectSize();
		if ((uiDataSize == 0) || (pcNode->IsDelted() && !pcNode->IsDirty()))
		{
			return FALSE;
		}

		if (puiDataSize)
		{
			*puiDataSize = uiDataSize;
		}

		pv = pcNode->GetObjectPtr();
		if (pvObject)
		{
			memcpy(pvObject, pv, uiDataSize);
		}
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Put(char* pszKey, void* pvObject, unsigned short uiDataSize)
{
	int iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	return Put(pszKey, iKeySize, pvObject, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Put(void* pvKey, int iKeySize, void* pvObject, unsigned short uiDataSize)
{
	CIndexTreeNodeFile*		pcCurrent;
	CIndexTreeNodeFile*		pcReallocatedCurrent;
	unsigned char			c;
	BOOL					bResult;
	BOOL					bWrite;
	unsigned short			uiOriginalSize;
	BOOL					bRootHasIndex;

	if (iKeySize == 0)
	{
		return FALSE;
	}

	if ((pvObject == NULL) || (uiDataSize == 0))
	{
		return FALSE;
	}

	bRootHasIndex = mpcRoot->GetFileIndex()->HasFile();
	pcCurrent = mpcRoot;
	for (int i = 0; i < iKeySize; i++)
	{
		c = ((char*)pvKey)[i];
		pcCurrent = GetChildNodeOrAllocate(pcCurrent, c);
	}

	if (pcCurrent->GetObjectSize() <= uiDataSize)
	{
		pcReallocatedCurrent = ReallocateNodeForLargerData(pcCurrent, uiDataSize);
		bResult = pcReallocatedCurrent->SetObject(pvObject, uiDataSize);
	}
	else
	{
		uiOriginalSize = pcCurrent->GetObjectSize();
		bResult = pcCurrent->SetObject(pvObject, uiDataSize);
		pcReallocatedCurrent = ReallocateNodeForSmallerData(pcCurrent, uiOriginalSize);
	}

	if (pcCurrent != pcReallocatedCurrent)
	{
		pcReallocatedCurrent->SetChildrensParent();
		pcCurrent = pcReallocatedCurrent;
	}

	if (mbWriteThrough)
	{
		while (pcCurrent != NULL)
		{
			if (pcCurrent->IsDirty())
			{
				pcCurrent->SetDirtyNode(FALSE);
				bWrite = Write(pcCurrent);
				if (!bWrite)
				{
					bResult = FALSE;
					break;
				}
			}
			pcCurrent = (CIndexTreeNodeFile*)pcCurrent->GetParent();
		}
	}
	else
	{
		while (pcCurrent != NULL)
		{
			pcCurrent->SetDirtyPath(TRUE);
			pcCurrent = (CIndexTreeNodeFile*)pcCurrent->GetParent();
		}
	}

	bResult &= WriteRootFileIndex(bRootHasIndex, mpcRoot->GetFileIndex());
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Put(void* pvKey, int iKeySize, unsigned short uiDataSize)
{
	return Put(pvKey, iKeySize, NULL, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::ReallocateNodeForIndex(CIndexTreeNodeFile* pcNode, unsigned char uiIndex)
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
	pcNode->SetDirtyNode(TRUE); 
	pcNode->Contain(uiIndex);

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

	tNewNodeSize = pcNode->CalculateRequiredNodeSizeForData(uiDataSize);
	tOldNodeSize = pcNode->CalculateRequiredNodeSizeForCurrent();

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
	pcNode->SetDirtyNode(TRUE);

	RemapChildParents(pcOldNode, pcNode);
	return pcNode;
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
		pcParent->SetDirtyNode(TRUE);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::GetChildNodeOrAllocate(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent)
{
	CIndexTreeNodeFile*		pcNewFileNode;
	CIndexTreeChildNode*	pcChildNodeOnParent;
	CIndexTreeNodeFile*		pcReallocedParent;

	pcChildNodeOnParent = pcParent->Get(uiIndexInParent);
	if (pcChildNodeOnParent == NULL)  //Uncontained, off the left or the right of the parent node's indexes.
	{
		//This causes the parent node to be re-allocated as its indexes must be grown.
		pcReallocedParent = ReallocateNodeForIndex(pcParent, uiIndexInParent);

		//The new node must also still be allocated.  Two nodes have been altered (the parent and the new node).
		pcNewFileNode = AllocateNode(pcReallocedParent, 0, uiIndexInParent);

		pcReallocedParent->Set(uiIndexInParent, pcNewFileNode);

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
			pcNewFileNode = AllocateNode(pcParent, 0, uiIndexInParent);

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
BOOL CIndexTreeFile::Remove(char* pszKey)
{
	int iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	return Remove(pszKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Remove(void* pvKey, int iKeySize)
{
	CIndexTreeNodeFile*		pcCurrent;

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return FALSE;
	}

	pcCurrent = GetNode(pvKey, iKeySize);
	if (pcCurrent == NULL)
	{
		return FALSE;
	}

	if (mbWriteThrough)
	{
		return RemoveWriteThrough(pcCurrent);
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
BOOL CIndexTreeFile::RemoveWriteThrough(CIndexTreeNodeFile* pcCurrent)
{
	unsigned char			c;
	CIndexTreeNodeFile*		pcParent;
	CIndexTreeNodeFile*		pcOldParent;
	CIndexTreeNodeFile*		pcNode;
	void*					pvObject;
	BOOL					bResizeNode;
	size_t					tNewNodeSize;
	size_t					tOldNodeSize;
	BOOL					bResult;

	if (pcCurrent->GetObjectSize() == 0)
	{
		return FALSE;
	}

	bResult = TRUE;

	pvObject = ((void**)pcCurrent->GetObjectPtr());
	pcNode = pcCurrent;

	pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();
	pcNode->ClearObject();
	for (;;)
	{
		c = pcNode->GetIndexInParent();

		if (pcNode->IsEmpty())
		{
			tOldNodeSize = pcParent->CalculateRequiredNodeSizeForCurrent();
			if (pcParent == mpcRoot)
			{
				pcParent->Clear(c);
			}
			else
			{
				bResizeNode = pcParent->ClearAndUncontain(c);
				if (bResizeNode)
				{
					tNewNodeSize = pcParent->CalculateRequiredNodeSizeForCurrent();
					pcOldParent = pcParent;

					pcParent = (CIndexTreeNodeFile*)Realloc(pcParent, tNewNodeSize, tOldNodeSize);
					if (pcOldParent != pcParent)
					{
						pcParent->SetChildrensParent();
					}
					RemapChildParents(pcOldParent, pcParent);
				}
			}

			bResult = Delete(pcNode);
			Free(pcNode);

			if (!bResult)
			{
				break;
			}
		}
		else
		{
			pcNode->SetDirtyNode(FALSE);
			bResult = Write(pcNode);
			break;
		}

		pcNode = pcParent;
		pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();
		if (!pcParent)
		{
			if (pcNode->IsDirty())
			{
				pcNode->SetDirtyNode(FALSE);
				bResult = Write(pcNode);
			}
			break;
		}
	}

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::RemoveWaitForFlush(CIndexTreeNodeFile* pcCurrent)
{
	if (pcCurrent->GetObjectSize() == 0)
	{
		return FALSE;
	}

	pcCurrent->SetDeletedNode(TRUE);
	pcCurrent->SetDirtyNode(FALSE);
	while (pcCurrent != NULL)
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
int CIndexTreeFile::NumElements(void)
{
	return RecurseSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Flush(void)
{
	BOOL	bResult;
	BOOL	bRootHasIndex;
	
	if (mbWriteThrough)
	{
		return gcLogger.Error2(__METHOD__, " Cannot flush an index tree that is write through.", NULL);
	}

	bRootHasIndex = mpcRoot->GetFileIndex()->HasFile();

	bResult = FlushRemoved();
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

		if (pcNode->IsDirty())
		{
			bResult = Write(pcNode);
			if (!bResult)
			{
				pcCursor->Pop();
				return FALSE;
			}
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
BOOL CIndexTreeFile::FlushRemoved(void)
{
	CArrayVoidPtr					apvDeletedNodes;
	int								i;
	int								iNumNodes;
	CIndexTreeNodeFile*				pcNode;
	BOOL							bResult;
	CListTemplateMinimal<char>*		paszKeys;
	int								iKeySize;
	unsigned char*					pszKey;

	apvDeletedNodes.Init(1024);
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
			bResult |= RemoveWriteThrough(pcNode);
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
void CIndexTreeFile::SetWriteThrough(BOOL bWriteThrough)
{
	mbWriteThrough = bWriteThrough;
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
			*pvData = psIterator->pcNode->GetObjectPtr();
		}
		if (piDataSize)
		{
			*piDataSize = psIterator->pcNode->GetObjectSize();
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
			*pvData = psIterator->pcNode->GetObjectPtr();
		}
		if (piDataSize)
		{
			*piDataSize = psIterator->pcNode->GetObjectSize();
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
	void*					pvObject;
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

			pvObject = pcChildNode->GetObjectPtr();
			if (pvObject != NULL)
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
	BOOL bResult;

	bResult = ValidateMagic();
	bResult &= ValidateLimits();
	bResult &= ValidateParentIndex();
	bResult &= ValidateNoFlushFlags();
	bResult &= ValidateFileIndexes();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeFile::RecurseSize(void)
{
	return RecurseSize(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeFile::RecurseSize(CIndexTreeNodeFile* pcNode)
{
	int						i;
	CIndexTreeNodeFile*		pcChild;
	int						iLastIndex;

	int count = 0;

	if (pcNode != NULL)
	{
		if (pcNode->HasObject() && !pcNode->IsDelted())
		{
			count++;
		}

		iLastIndex = pcNode->GetLastIndex();
		for (i = pcNode->GetFirstIndex(); i <= iLastIndex; i++)
		{
			pcChild = ReadNode(pcNode, i);
			count += RecurseSize(pcChild);
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
		uiSize = pcNode->GetObjectSize();

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
BOOL CIndexTreeFile::ValidateLimits(void)
{
	CIndexTreeRecursor	cCursor;
	BOOL				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateLimits(&cCursor);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::RecurseValidateLimits(CIndexTreeRecursor* pcCursor)
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
				pcChild = ReadNode(pcNode, i);
				pcCursor->Push(pcChild, i);
				bResult = RecurseValidateLimits(pcCursor);
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
BOOL CIndexTreeFile::ValidateNoFlushFlags(void)
{
	CIndexTreeRecursor	cCursor;
	BOOL				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateNoFlushFlags(&cCursor);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::RecurseValidateNoFlushFlags(CIndexTreeRecursor* pcCursor)
{
	CIndexTreeNodeFile*		pcNode;
	int						i;
	CIndexTreeNodeFile*		pcChild;
	BOOL					bResult;

	pcNode = (CIndexTreeNodeFile*)pcCursor->GetNode();
	if (pcNode != NULL)
	{
		if (pcNode->HasFlags(INDEX_TREE_NODE_FLAG_DELETED_PATH | INDEX_TREE_NODE_FLAG_DELETED_NODE | 
							 INDEX_TREE_NODE_FLAG_DIRTY_PATH | INDEX_TREE_NODE_FLAG_DIRTY_NODE))
		{
			pcCursor->GenerateBad();
			return gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] has unexpected flags.", NULL);
		}

		if (pcNode->HasNodes())
		{
			for (i = pcNode->GetFirstIndex(); i <= pcNode->GetLastIndex(); i++)
			{
				pcChild = ReadNode(pcNode, i);
				pcCursor->Push(pcChild, i);
				bResult = RecurseValidateParentIndex(pcCursor);
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
BOOL CIndexTreeFile::ValidateFileIndexes(void)
{
	CIndexTreeRecursor	cCursor;
	BOOL				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateFileIndexes(&cCursor);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::RecurseValidateFileIndexes(CIndexTreeRecursor* pcCursor)
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
				pcChild = ReadNode(pcNode, i);
				pcCursor->Push(pcChild, i);
				bResult = RecurseValidateFileIndexes(pcCursor);
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
BOOL CIndexTreeFile::ValidateMagic(void)
{
	CIndexTreeRecursor	cCursor;
	BOOL				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateMagic(&cCursor);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::RecurseValidateMagic(CIndexTreeRecursor* pcCursor)
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
				pcChild = ReadNode(pcNode, i);
				pcCursor->Push(pcChild, i);
				bResult = RecurseValidateMagic(pcCursor);
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
	CIndexTreeRecursor	cCursor;
	BOOL				bResult;

	cCursor.Init(mpcRoot);
	bResult = RecurseValidateParentIndex(&cCursor);
	cCursor.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::RecurseValidateParentIndex(CIndexTreeRecursor* pcCursor)
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
				pcChild = ReadNode(pcNode, i);
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
					bResult = RecurseValidateParentIndex(pcCursor);
					if (!bResult)
					{
						pcCursor->Pop();
						return FALSE;
					}
				}
			}

			for (i = pcNode->GetFirstIndex(); i <= pcNode->GetLastIndex(); i++)
			{
				pcChild = ReadNode(pcNode, i);
				pcCursor->Push(pcChild, i);
				bResult = RecurseValidateParentIndex(pcCursor);
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

	return pcNode->GetObjectSize() != 0;
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
int CIndexTreeFile::CountAllocatedNodes(void)
{
	return RecurseCountAllocatedNodes(mpcRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeFile::RecurseCountAllocatedNodes(CIndexTreeNodeFile* pcNode)
{
	int						i;
	CIndexTreeNodeFile*		pcChild;
	int						count;
	int						iLastIndex;

	if (pcNode != NULL)
	{
		count = 1;
		iLastIndex = pcNode->GetLastIndex();
		for (i = pcNode->GetFirstIndex(); i <= iLastIndex; i++)
		{
			pcChild = ReadNode(pcNode, i);
			count += RecurseCountAllocatedNodes(pcChild);
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
		count = pcNode->GetNumIndexes();
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
	int					iNodeSize;
	CIndexedFile*		pcNewIndexFile;
	CIndexedFile*		pcOldIndexFile;
	unsigned int		uiDataIndex;
	CFileDataIndex*		pcIndex;

	iNodeSize = pcNode->CalculateBufferSize();
	pvBuffer = cTemp.Init(iNodeSize);
	iWrittenPos = pcNode->WriteToBuffer(pvBuffer, iNodeSize);
	if (iWrittenPos <= 0)
	{
		return FALSE;
	}

	if (iWrittenPos != iNodeSize)
	{
		cTemp.Kill();
		return gcLogger.Error2(__METHOD__, " Could not write IndexTreeNodeFile.  Expected size [", IntToString(iNodeSize), "] is not equal to written buffer size [", IntToString(iWrittenPos), "].", NULL);
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

	pcNewIndexFile = mcIndexFiles.GetOrCreateFile(iNodeSize);
	if (!pcNewIndexFile)
	{
		cTemp.Kill();
		return FALSE;
	}

	if (pcNewIndexFile != pcOldIndexFile)
	{
		uiDataIndex = pcNewIndexFile->Write(pvBuffer);
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
	int						iDataSize;
	CStackMemory<>			cTemp;
	void*					pvBuffer;
	BOOL					bResult;
	CIndexTreeNodeFile*		pcFileNode;
	int						iFile;
	unsigned int			uiIndex;
	
	iFile = pcChildNode->u.mcFile.miFile;
	uiIndex = pcChildNode->u.mcFile.muiIndex;

	pcFile = mcIndexFiles.GetFile(iFile);
	iDataSize = pcFile->GetDataSize();

	pvBuffer = cTemp.Init(iDataSize);
	bResult = pcFile->Read(uiIndex, pvBuffer);
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not read tree node in file number [", IntToString(iFile), "] name [", StringToString(pcFile->GetFileName()), "] at index [", IntToString(uiIndex), "].", NULL);
		return FALSE;
	}
		
	pcFileNode = AllocateNode(pcFileNodeParent, uiIndexInParent, pvBuffer, iDataSize);
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
CIndexTreeNodeFile* CIndexTreeFile::SetParentWithExisting(CIndexTreeNodeFile* pcParent, unsigned char c, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned char uiDataSize)
{
	//Used by the IndexTreeWriter.
	CIndexTreeChildNode*	pcCurrent;
	CIndexTreeNodeFile*		pcNew;

	pcCurrent = pcParent->Get(c);
	if (pcCurrent == NULL)
	{
		pcNew = AllocateNode(pcParent, uiFirstIndex, uiLastIndex, uiDataSize, c);
		pcParent->Set(c, pcNew);
		return pcNew;
	}
	else
	{
		if (pcCurrent->IsUnallocated())
		{
			pcNew = AllocateNode(pcParent, uiFirstIndex, uiLastIndex, uiDataSize, c);
			pcParent->Set(c, pcNew);
			return pcNew;

		}
		else
		{
			gcLogger.Error2(__METHOD__, " Set [", CharToString(c), "].  Should only ever be called with NULL or Unallocated nodes.", NULL);
			return NULL;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::SetParentWithExisting(CIndexTreeNodeFile* pcParent, unsigned char c, unsigned char uiDataSize)
{
	//Used by the IndexTreeWriter.
	CIndexTreeChildNode*	pcCurrent;
	CIndexTreeNodeFile*		pcNew;

	pcCurrent = pcParent->Get(c);
	if (pcCurrent == NULL)
	{
		pcNew = AllocateNode(pcParent, uiDataSize, c);
		pcParent->Set(c, pcNew);
		return pcNew;
	}
	else
	{
		if (pcCurrent->IsUnallocated())
		{
			pcNew = AllocateNode(pcParent, uiDataSize, c);
			pcParent->Set(c, pcNew);
			return pcNew;
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Set [", CharToString(c), "].  Should only ever be called with NULL or Unallocated nodes.", NULL);
			return NULL;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::Debug(void* pvKey, int iKeySize)
{
	CIndexTreeNodeFile*		pcCurrent;
	unsigned char			c;
	int						i;
	SIndexTreeDebugNode		sDebugNode;

	pcCurrent = mpcRoot;
	for (i = 0; i < iKeySize; i++)
	{
		c = ((unsigned char*)pvKey)[i];
		if (pcCurrent != NULL)
		{
			pcCurrent = DebugNode(pcCurrent, c);
		}
		else
		{
			sDebugNode.InitBroken(c);
			sDebugNode.Dump();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::DebugNode(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent)
{
	CIndexTreeNodeFile*		pcCurrent;
	CIndexTreeChildNode*	pcChild;

	pcChild = pcParent->Get(uiIndexInParent);
	if (pcChild)
	{
		if (pcChild->IsMemory())
		{
			pcCurrent = pcChild->u.mpcMemory;
			DebugNode(pcCurrent);
			return pcCurrent;
		}
		else if (pcChild->IsFile())
		{
			if (Read(pcChild, pcParent, uiIndexInParent))
			{
				pcCurrent = pcChild->u.mpcMemory;
				DebugNode(pcCurrent);
				return pcCurrent;
			}
			else
			{
				DebugNode(pcChild->u.mcFile.miFile, pcChild->u.mcFile.muiIndex, uiIndexInParent);
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
			gcLogger.Error2(__METHOD__, " Child node [", IntToString((int)uiIndexInParent), "] is corrupt.  Type is [", pcChild->u.msType.iType, "].", NULL);
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
void CIndexTreeFile::DebugNode(CIndexTreeNodeFile* pcCurrent)
{
	CFileDataIndex*			pcIndex;
	int						iFile;
	unsigned int			uiIndex;

	pcIndex = pcCurrent->GetFileIndex();

	iFile = pcIndex->miFile;
	uiIndex = pcIndex->muiIndex;

	DebugNode(iFile, uiIndex, pcCurrent->GetIndexInParent());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::DebugNode(int iFile, unsigned int uiIndex, unsigned int uIndexFromParent)
{
	CIndexedFile*			pcFile;
	int						iDataSize;
	CStackMemory<>			cTemp;
	void*					pvBuffer;
	BOOL					bResult;
	SIndexTreeDebugNode		sDebugNode;

	pcFile = mcIndexFiles.GetFile(iFile);
	iDataSize = pcFile->GetDataSize();

	pvBuffer = cTemp.Init(iDataSize);
	bResult = pcFile->Read(uiIndex, pvBuffer);

	sDebugNode.InitFromBuffer(pvBuffer, iDataSize, iFile, uiIndex);
	sDebugNode.uiIndexInParent = uIndexFromParent;
	sDebugNode.Dump();
}

