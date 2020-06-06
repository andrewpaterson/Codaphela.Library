#include "Logger.h"
#include "IndexTree.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTree::Init(CMallocator* pcMalloc, EIndexKeyReverse eKeyReverse, size_t tSizeofNode, size_t tSizeofDataNode, size_t tSizeofNodePtr, int iMaxDataSize, int	iMaxKeySize, CIndexTreeDataOrderer* pcDataOrderer)
{
	BOOL bResult;

	mpcMalloc = pcMalloc;
	meReverseKey = eKeyReverse;
	mtSizeofNode = tSizeofNode;
	mtSizeofDataNode = tSizeofDataNode;
	mtSizeofNodePtr = tSizeofNodePtr;
	bResult = TRUE;
	if ((iMaxKeySize <= 0) || (iMaxKeySize > MAX_KEY_SIZE))
	{
		gcLogger.Error2(__METHOD__, " Max Key size [", IntToString(iMaxKeySize), "] must be positive and <= [", MAX_KEY_SIZE, "].", NULL);
		iMaxKeySize = MAX_KEY_SIZE;
		bResult = FALSE;
	}
	if ((iMaxDataSize <= 0) || (iMaxDataSize > MAX_DATA_SIZE))
	{
		gcLogger.Error2(__METHOD__, " Data size [", IntToString(iMaxDataSize), "] must be positive and <= [", IntToString(MAX_DATA_SIZE), "].", NULL);
		iMaxDataSize = MAX_DATA_SIZE;
		bResult = FALSE;
	}
	miMaxDataSize = iMaxDataSize;
	miMaxKeySize = iMaxKeySize;

	mpcDataOrderer = pcDataOrderer;
	if (mpcDataOrderer)
	{
		mpcDataOrderer->SetIndexTree(this);
	}

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTree::Kill(void)
{
	if (mpcDataOrderer)
	{
		mpcDataOrderer->Kill();
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTree::Malloc(size_t tSize)
{
	return mpcMalloc->Malloc(tSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTree::Free(void* pv)
{
	mpcMalloc->Free(pv);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTree::Realloc(void* pv, size_t tNewSize, size_t tExistingSize)
{
	if (tExistingSize != tNewSize)
	{
		return mpcMalloc->Realloc(pv, tNewSize);
	}
	else
	{
		return pv;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTree::SizeofNode(void)
{
	return mtSizeofNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTree::SizeofDataNode(void)
{
	return mtSizeofDataNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTree::SizeofNodePtr(void)
{
	return mtSizeofNodePtr;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTree::FreeNode(CIndexTreeNode* pcNode)
{
	Free(pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTree::ValidatePut(int iKeySize, int iDataSize)
{
	if ((iKeySize <= 0) || (iKeySize > miMaxKeySize))
	{
		gcLogger.Error2(__METHOD__, "Key size [", IntToString(iKeySize), "] must be positive and <= [", IntToString(miMaxKeySize), "].", NULL);
		return FALSE;
	}
	if ((iDataSize <= 0) || (iDataSize > miMaxDataSize))
	{
		gcLogger.Error2(__METHOD__, "Data size [", IntToString(iDataSize), "] must be positive and <= [", IntToString(miMaxDataSize), "].", NULL);
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTree::InsertReorderData(CIndexTreeNode* pcNode)
{
	if (mpcDataOrderer)
	{
		mpcDataOrderer->New(pcNode->GetNodeData());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTree::GetReorderData(CIndexTreeNode* pcNode)
{
	if (mpcDataOrderer)
	{
		mpcDataOrderer->Get(pcNode->GetNodeData());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTree::PutReorderData(CIndexTreeNode* pcNode)
{
	if (mpcDataOrderer)
	{
		mpcDataOrderer->Put(pcNode->GetNodeData());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTree::RemoveReorderData(CIndexTreeNode* pcNode)
{
	if (mpcDataOrderer)
	{
		mpcDataOrderer->Remove(pcNode->GetNodeData());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTree::HasKeyReorderData(CIndexTreeNode* pcNode)
{
	if (mpcDataOrderer)
	{
		mpcDataOrderer->HasKey(pcNode->GetNodeData());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTree::CalculateRootNodeSize(void)
{
	return CalculateNodeSize(MAX_UCHAR + 1, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTree::CalculateNodeSize(int iRequiredIndices, int iDataSize)
{
	if (iDataSize == 0)
	{
		return SizeofNode() + iRequiredIndices * SizeofNodePtr();
	}
	else
	{
		return SizeofDataNode() + iDataSize + iRequiredIndices * SizeofNodePtr();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EIndexKeyReverse CIndexTree::ReverseKeys(void)
{
	return meReverseKey;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTree::StartKey(int* pi, int iKeySize)
{
	if (meReverseKey == IKR_No)
	{
		*pi = 0;
		return *pi < iKeySize;
	}
	else
	{
		*pi = iKeySize - 1;
		return *pi >= 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTree::LoopKey(int* pi, int iKeySize)
{
	if (meReverseKey == IKR_No)
	{
		(*pi)++;
		return *pi < iKeySize;
	}
	else
	{
		(*pi)--;
		return *pi >= 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNode* CIndexTree::GetNodeForDataNode(CIndexTreeDataNode* pcDataNode)
{
	return (CIndexTreeNode*)RemapSinglePointer(pcDataNode, -((int)mtSizeofNode));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNode* CIndexTree::GetNodeForData(void* pvData)
{
	return (CIndexTreeNode*)RemapSinglePointer(pvData, -((int)mtSizeofDataNode));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTree::GetDataForDataNode(CIndexTreeDataNode* pcDataNode)
{
	return RemapSinglePointer(pcDataNode, (mtSizeofDataNode - mtSizeofNode));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTree::GetDataForNode(CIndexTreeNode* pcNode)
{
	return RemapSinglePointer(pcNode, mtSizeofDataNode);
}


