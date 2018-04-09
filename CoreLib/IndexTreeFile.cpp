#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/GlobalMemory.h"
#include "IndexedFile.h"
#include "IndexTreeFile.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, char* szRootFileName)
{
	return Init(pcDurableFileControl, szRootFileName, &gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, char* szRootFileName, CMallocator* pcMalloc)
{
	//The DurableFileControl must be begun before .Init is called and should be ended afterwards.

	BOOL	bResult;

	if (!pcDurableFileControl->IsBegun())
	{
		gcLogger.Error2(__METHOD__, " DurableFileController must be begun before calling Init.", NULL);
		return FALSE;
	}

	CIndexTree::Init(pcMalloc, sizeof(CIndexTreeNodeFile), sizeof(CIndexTreeChildNode));

	mpcDurableFileControl = pcDurableFileControl;
	mcIndexFiles.Init(mpcDurableFileControl, "IDAT", "Index.IDX", mpcDurableFileControl->IsDurable() ? "_Index.IDX" : NULL);

	bResult = InitRoot(szRootFileName);
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
	mszRootFileName.Kill();
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
BOOL CIndexTreeFile::InitRoot(char* szRootFileName)
{
	BOOL				bRootIndexExists;
	CFileIndex			cRootFileIndex;
	CIndexedFile*		pcRootIndexFile;
	char				pvBuffer[8 KB];
	int					iNodeSize;

	mszRootFileName.Init(szRootFileName);
	cRootFileIndex = LoadRootFileIndex(szRootFileName);

	bRootIndexExists = cRootFileIndex.HasFile();
	if (bRootIndexExists)
	{
		//The data size on the root is always zero.
		mpcRoot = AllocateRoot(cRootFileIndex);

		iNodeSize = mpcRoot->CalculateBufferSize();
		if (iNodeSize > 8 KB)
		{
			return FALSE;
		}

		pcRootIndexFile = mcIndexFiles.GetOrCreateFile(iNodeSize);  //Dangerous.  Should be a Get.
		pcRootIndexFile->Read(cRootFileIndex.mulliFilePos, pvBuffer, iNodeSize);

		mpcRoot->InitFromBuffer(this, NULL, pvBuffer, 8 KB, cRootFileIndex);

		return TRUE;
	}
	else
	{
		mpcRoot = AllocateRoot();
		return Write(mpcRoot);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileIndex CIndexTreeFile::LoadRootFileIndex(char* szRootFileName)
{
	CFileBasic	cFileBasic;
	CDiskFile	cDiskFile;
	CFileUtil	cFileUtil;
	CFileIndex	cIndex;
	filePos		iSize;

	iSize = cFileUtil.Size(szRootFileName);
	if (iSize != -1)
	{
		if (iSize == sizeof(CFileIndex))
		{
			cDiskFile.Init(szRootFileName);
			cFileBasic.Init(&cDiskFile);

			cFileBasic.Open(EFM_Read);
			cFileBasic.Read(&cIndex, 0, sizeof(CFileIndex));
			cFileBasic.Close();

			cFileBasic.Kill();
			cDiskFile.Kill();

			return cIndex;
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Index Root file size [", LongLongToString(iSize), "] is incorrect.  Should be SizeOf(CFileIndex).", NULL);
			cIndex.Init();
			return cIndex;
		}
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
CIndexTreeNodeFile* CIndexTreeFile::AllocateRoot(CFileIndex cFileIndex)
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
	return pcNode;
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
CIndexTreeNodeFile* CIndexTreeFile::ReadNode(CIndexTreeNodeFile* pcParent, unsigned char c)
{
	CIndexTreeNodeFile*		pcCurrent;
	CIndexTreeChildNode*	pcChild;

	pcChild = pcParent->Get(c);
	if (pcChild)
	{
		if (pcChild->IsMemory())
		{
			pcCurrent = pcChild->u.mpcMemory;
			return pcCurrent;
		}
		else if (pcChild->IsFile())
		{
			if (Read(pcChild))
			{
				pcCurrent = pcChild->u.mpcMemory;
				return pcCurrent;
			}
			else
			{
				gcLogger.Error2(__METHOD__, " Could not load child node [", IntToString((int)c), "].", NULL);
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
			gcLogger.Error2(__METHOD__, " Child node [", IntToString((int)c), "] is corrupt.  Type is [", pcChild->u.msType.iType, "].", NULL);
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
CIndexTreeNodeFile* CIndexTreeFile::ReadNode(CIndexTreeChildNode* pcChild)
{
	CIndexTreeNodeFile*		pcCurrent;

	if (pcChild)
	{
		if (pcChild->IsMemory())
		{
			pcCurrent = pcChild->u.mpcMemory;
			return pcCurrent;
		}
		else if (pcChild->IsFile())
		{
			if (Read(pcChild))
			{
				pcCurrent = pcChild->u.mpcMemory;
				return pcCurrent;
			}
			else
			{
				gcLogger.Error2(__METHOD__, " Could not load child node.", NULL);
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
			gcLogger.Error2(__METHOD__, " Child node is corrupt.", NULL);
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
		if ((uiDataSize == 0) || (pcNode->IsDelted()))
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
	unsigned short			uiOriginalSize;

	if (iKeySize == 0)
	{
		return FALSE;
	}

	if ((pvObject == NULL) || (uiDataSize == 0))
	{
		return FALSE;
	}

	pcCurrent = mpcRoot;

	for (int i = 0; i < iKeySize; i++)
	{
		c = ((char*)pvKey)[i];
		pcCurrent = AllocateNodeIfUnallocated(pcCurrent, c);
		pcCurrent->SetDirtyPath(TRUE);
	}
	pcCurrent->SetDirtyNode(TRUE);

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
	}

	if (bResult)
	{
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
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::AllocateNodeIfUnallocated(CIndexTreeNodeFile* pcParent, unsigned char c)
{
	CIndexTreeNodeFile*		pcNew;
	CIndexTreeChildNode*	pcCurrent;
	CIndexTreeNodeFile*		pcReallocedParent;

	pcCurrent = pcParent->Get(c);
	if (pcCurrent == NULL)  //Uncontained.
	{
		pcNew = AllocateNode(pcParent, 0, c);
		pcReallocedParent = ReallocateNodeForIndex(pcParent, c);
		pcReallocedParent->Set(c, pcNew);
		if (pcParent != pcReallocedParent)
		{
			pcReallocedParent->SetChildrensParent();
		}
		return pcNew;
	}
	else
	{
		if (pcCurrent->IsMemory())
		{
			return pcCurrent->u.mpcMemory;
		}
		else if (pcCurrent->IsFile())
		{
			if (Read(pcCurrent))
			{
				return pcCurrent->u.mpcMemory;
			}
			else
			{
				gcLogger.Error2(__METHOD__, " Could not load child node [", IntToString((int)c), "].", NULL);
				return NULL;
			}
		}
		else if (pcCurrent->IsUnallocated())
		{
			pcNew = AllocateNode(pcParent, 0, c);
			pcCurrent->Init(pcNew);
			return pcCurrent->u.mpcMemory;
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Child node [", IntToString((int)c), "] is corrupt.", NULL);
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

	return Remove(pcCurrent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Remove(CIndexTreeNodeFile* pcCurrent)
{
	unsigned char			c;
	CIndexTreeNodeFile*		pcParent;
	CIndexTreeNodeFile*		pcOldParent;
	CIndexTreeNodeFile*		pcNode;
	void*					pvObject;
	BOOL					bResizeNode;
	size_t					tNewNodeSize;
	size_t					tOldNodeSize;

	if (pcCurrent->GetObjectSize() == 0)
	{
		return FALSE;
	}

	pvObject = ((void**)pcCurrent->GetObjectPtr());

	pcNode = pcCurrent;
	pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();
	pcCurrent->ClearObject();
	for (;;)
	{
		c = pcNode->GetIndexInParent();

		if (pcNode->IsEmpty())
		{
			tOldNodeSize = pcParent->CalculateRequiredNodeSizeForCurrent();
			if (pcParent != mpcRoot)
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
			else
			{
				pcParent->Clear(c);
			}

			Free(pcNode);
		}
		else
		{
			break;
		}

		pcNode = pcParent;
		pcParent = (CIndexTreeNodeFile*)pcNode->GetParent();
		if (!pcParent)
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
BOOL CIndexTreeFile::Delete(void* pvKey, int iKeySize)
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

	if (pcCurrent->GetObjectSize() == 0)
	{
		return FALSE;
	}

	pcCurrent->SetDeletedNode(TRUE);
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
int CIndexTreeFile::NumElements(void)
{
	return RecurseSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::Flush(void)
{
	CArrayVoidPtr		apv;
	int					i;
	int					iNumNodes;
	CIndexTreeNodeFile*	pcNode;
	BOOL				bResult;

	apv.Init(1024);
	FindWithFlags(&apv, INDEX_TREE_NODE_FLAG_DELETED_PATH, INDEX_TREE_NODE_FLAG_DELETED_NODE);

	bResult = TRUE;
	iNumNodes = apv.NumElements();
	for (i = iNumNodes-1; i >= 0; i--)
	{
		pcNode = (CIndexTreeNodeFile*)apv.GetPtr(i);
		if (pcNode->IsDelted())
		{
			bResult |= Remove(pcNode);
		}
	}

	apv.Kill();
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
CListCharsMinimal* CIndexTreeFile::FindKeys(CArrayVoidPtr* apvNodes)
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
	iKeysSize = 0;
	for (i = 0; i < iNumKeys; i++)
	{
		pcNode = (CIndexTreeNodeFile*)apvNodes->GetPtr(i);
		iKeySize = FindKeySize(pcNode);
		iKeysSize += iKeySize;
	}

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

	bResult = ValidateLimits();
	bResult &= ValidateParentIndex();
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
					gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] did not contain first index [", IntToString(iFirst), "].", NULL);
					return FALSE;
				}
				if (!pcNode->ContainsIndex(iLast))
				{
					pcCursor->GenerateBad();
					gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] did not contain last index [", IntToString(iLast), "].", NULL);
					return FALSE;
				}

				pcFirst = pcNode->Get(iFirst);
				if (pcFirst == NULL)
				{
					pcCursor->GenerateBad();
					gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] first child [", IntToString(iFirst), "] was not a file or memory node.", NULL);
					return FALSE;
				}

				pcLast = pcNode->Get(iLast);
				if (pcLast == NULL)
				{
					pcCursor->GenerateBad();
					gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] for key [", pcCursor->GetBadKey(), "] last child [", IntToString(iFirst), "] was not a file or memory node.", NULL);
					return FALSE;
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
						gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] points to the wrong parent node.", NULL);
						return FALSE;
					}

					uiIndexInParent = pcChild->GetIndexInParent();
					if (i != uiIndexInParent)
					{
						pcCursor->GenerateBad();
						gcLogger.Error2(__METHOD__, " Node [", pcCursor->GetBadNode(), "] points to the wrong parent node.", NULL);
						return FALSE;
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
	char				pvBuffer[8 KB];
	int					iNodeSize;
	CIndexedFile*		pcIndexFile;
	filePos				iFilePos;

	iWrittenPos = pcNode->WriteToBuffer(pvBuffer, 8 KB);
	if (iWrittenPos <= 0)
	{
		return FALSE;
	}

	iNodeSize = pcNode->CalculateBufferSize();
	if (iWrittenPos != iNodeSize)
	{
		gcLogger.Error2(__METHOD__, " Could not write IndexTreeNodeFile.  Expected size [", IntToString(iNodeSize), "] is not equal to written buffer size [", IntToString(iWrittenPos), "].", NULL);
		return FALSE;
	}

	pcIndexFile = mcIndexFiles.GetOrCreateFile(iNodeSize);
	if (!pcIndexFile)
	{
		return TRUE;
	}
	iFilePos = pcIndexFile->Write(pvBuffer);
	pcNode->SetFileIndex(pcIndexFile->GetFileIndex(), iFilePos);
	return TRUE;
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
BOOL CIndexTreeFile::Read(CIndexTreeChildNode* pcChildNode)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::SetParentWithExisting(CIndexTreeNodeFile* pcParent, unsigned char c, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned char uiDataSize)
{
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

