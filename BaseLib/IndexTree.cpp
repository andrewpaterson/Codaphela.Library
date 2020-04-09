#include "IndexTree.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTree::Init(CMallocator* pcMalloc, EIndexKeyReverse eKeyReverse, size_t tSizeofNode, size_t tSizeofNodePtr, int iMaxDataSize, int	iMaxKeySize)
{
	mpcMalloc = pcMalloc;
	meReverseKey = eKeyReverse;
	mtSizeofNode = tSizeofNode;
	mtSizeofNodePtr = tSizeofNodePtr;
	miMaxDataSize = iMaxDataSize;
	miMaxKeySize = iMaxKeySize;
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
	return SizeofNode() + iDataSize + iRequiredIndices * SizeofNodePtr();
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
BOOL CIndexTree::StartKey(int* pi, int iKeysSize)
{
	if (meReverseKey == IKR_No)
	{
		*pi = 0;
		return *pi < iKeysSize;
	}
	else
	{
		*pi = iKeysSize - 1;
		return *pi >= 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTree::LoopKey(int* pi, int iKeysSize)
{
	if (meReverseKey == IKR_No)
	{
		(*pi)++;
		return *pi < iKeysSize;
	}
	else
	{
		(*pi)--;
		return *pi >= 0;
	}
}

