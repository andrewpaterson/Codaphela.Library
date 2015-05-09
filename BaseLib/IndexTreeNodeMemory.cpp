#include "PointerRemapper.h"
#include "IndexTreeNodeMemory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeMemory::Init(CIndexTreeNodeMemory* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex)
{
	size_t tSize;

	muiFirstIndex = uiFirstIndex;
	muiLastIndex = uiLastIndex;
	muiDataSize = 0;
	mbNodesEmpty = FALSE;
	mpcParent = pcParent;

	tSize = (uiLastIndex - uiFirstIndex + 1) * SizeofNodePtr();
	memset(GetNodes(), 0, tSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeMemory::Init(CIndexTreeNodeMemory* pcParent)
{
	mpcParent = pcParent;
	muiFirstIndex = 0;
	muiLastIndex = 0;
	muiDataSize = 0;
	mbNodesEmpty = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeMemory::Contain(unsigned char uiIndex)
{
	//Contain assumes that the memory this node resides in has already been sized large enough.

	unsigned char		uiOldNumIndexes;
	CIndexTreeNodeMemory**	pvDest;
	size_t				tSize;
	CIndexTreeNodeMemory**	apcChildren;

	apcChildren = GetNodes();

	if (mbNodesEmpty == TRUE)
	{
		mbNodesEmpty = FALSE;
		muiFirstIndex = uiIndex;
		muiLastIndex = uiIndex;
		apcChildren[0] = NULL;
		return;
	}

	uiOldNumIndexes = GetNumIndexes();
	apcChildren = GetNodes();

	if (uiIndex < muiFirstIndex)
	{
		tSize = (muiFirstIndex - uiIndex) * SizeofNodePtr();
		pvDest = (CIndexTreeNodeMemory**)RemapSinglePointer(apcChildren, tSize);
		memmove(pvDest, apcChildren, uiOldNumIndexes * SizeofNodePtr());
		memset(apcChildren, 0, tSize);
		muiFirstIndex = uiIndex;
	}
	else if (uiIndex > muiLastIndex)
	{
		tSize = (uiIndex - muiLastIndex) * SizeofNodePtr();
		pvDest = (CIndexTreeNodeMemory**)RemapSinglePointer(apcChildren, uiOldNumIndexes * SizeofNodePtr());
		memset(pvDest, 0, tSize);
		muiLastIndex = uiIndex;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNodeMemory::Uncontain(unsigned char uiIndex)
{
	unsigned char		uiNextFirstIndex;
	unsigned char		uiPrevLastIndex;
	int					iNewNumIndexes;
	size_t				tSize;
	void*				pvSource;
	CIndexTreeNodeMemory**	apcChildren;

	if ((uiIndex != muiFirstIndex) && (uiIndex != muiLastIndex))
	{
		return FALSE;
	}
	else if (muiFirstIndex == muiLastIndex)
	{
		mbNodesEmpty = TRUE;
		muiFirstIndex = 0;
		muiLastIndex = 0;
		return TRUE;
	}

	apcChildren = GetNodes();

	if (uiIndex == muiFirstIndex)
	{
		uiNextFirstIndex = FindNextFirstIndex();
		iNewNumIndexes = GetNumIndexes(uiNextFirstIndex, muiLastIndex);
		tSize = (uiNextFirstIndex - muiFirstIndex) * SizeofNodePtr();
		pvSource = (CIndexTreeNodeMemory**)RemapSinglePointer(apcChildren, tSize);
		memmove(apcChildren, pvSource, iNewNumIndexes * SizeofNodePtr());
		muiFirstIndex = uiNextFirstIndex;
	}
	else if (uiIndex == muiLastIndex)
	{
		uiPrevLastIndex = FindPrevLastIndex();
		muiLastIndex = uiPrevLastIndex;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeNodeMemory::Get(unsigned char uiIndex)
{
	if (ContainsIndex(uiIndex))
	{
		return GetNode(uiIndex - muiFirstIndex);
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
void* CIndexTreeNodeMemory::GetObjectPtr(void)
{
	if (muiDataSize == 0)
	{
		return NULL;
	}
	else
	{
		return RemapSinglePointer(this, SizeofNode());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeNodeMemory::GetObjectSize(void)
{
	return muiDataSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeMemory::Set(unsigned char uiIndex, CIndexTreeNodeMemory* pcNode)
{
	CIndexTreeNodeMemory**	apcChildren;

	if (ContainsIndex(uiIndex))
	{
		apcChildren = GetNodes();
		apcChildren[uiIndex - muiFirstIndex] = pcNode;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNodeMemory::Clear(unsigned char uiIndex)
{
	CIndexTreeNodeMemory**	apcChildren;

	if (ContainsIndex(uiIndex))
	{
		apcChildren = GetNodes();
		apcChildren[uiIndex - muiFirstIndex] = NULL;

		return Uncontain(uiIndex);
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
unsigned char CIndexTreeNodeMemory::FindNextFirstIndex(void)
{
	int					i;
	CIndexTreeNodeMemory*		pcChild;
	CIndexTreeNodeMemory**	apcChildren;

	apcChildren = GetNodes();
	for (i = 1; i <= (int)(muiLastIndex - muiFirstIndex); i++)
	{
		pcChild = apcChildren[i];
		if (pcChild != NULL)
		{
			return muiFirstIndex + i;
		}
	}
	return muiLastIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeNodeMemory::FindPrevLastIndex(void)
{
	int					i;
	CIndexTreeNodeMemory*		pcChild;
	CIndexTreeNodeMemory**	apcChildren;

	apcChildren = GetNodes();
	for (i = (int)(muiLastIndex - muiFirstIndex)-1; i >= 0; i--)
	{
		pcChild = apcChildren[i];
		if (pcChild != NULL)
		{
			return muiFirstIndex + i;
		}
	}
	return muiFirstIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeNodeMemory::FindIndex(CIndexTreeNodeMemory* pcChild)
{
	int					i;
	CIndexTreeNodeMemory*		pcChildTest;
	CIndexTreeNodeMemory**	apcChildren;

	apcChildren = GetNodes();
	for (i = 0; i <= (int)(muiLastIndex - muiFirstIndex); i++)
	{
		pcChildTest = apcChildren[i];
		if (pcChild == pcChildTest)
		{
			return i + muiFirstIndex;
		}
	}

	//This should never be hit.
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNodeMemory::SetObject(void* pvObject, unsigned char uiSize)
{
	if (muiDataSize == 0)
	{
		SizeObject(uiSize);
		if (pvObject)
		{
			memcpy(GetObjectPtr(), pvObject, uiSize);
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
void CIndexTreeNodeMemory::ClearObject(void)
{
	SizeObject(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNodeMemory::IsEmpty(void)
{
	int					i;
	CIndexTreeNodeMemory*		pcChild;
	CIndexTreeNodeMemory**	apcChildren;

	if (muiDataSize != 0)
	{
		return FALSE;
	}

	if (mbNodesEmpty == TRUE)
	{
		return TRUE;
	}

	apcChildren = GetNodes();
	for (i = 0; i <= muiLastIndex - muiFirstIndex; i++)
	{
		pcChild = apcChildren[i];
		if (pcChild != NULL)
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeNodeMemory::GetFirstIndex(void)
{
	return muiFirstIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexTreeNodeMemory::GetLastIndex(void)
{
	return muiLastIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNodeMemory::HasNodes(void)
{
	return mbNodesEmpty == FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeNodeMemory::ContainsIndex(unsigned char uiIndex)
{
	if (mbNodesEmpty)
	{
		return FALSE;
	}

	if ((uiIndex >= muiFirstIndex) && (uiIndex <= muiLastIndex))
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
int CIndexTreeNodeMemory::GetAdditionalIndexes(unsigned char uiIndex)
{
	unsigned char uiFirstIndex;
	unsigned char uiLastIndex;

	if (uiIndex < muiFirstIndex)
	{
		uiFirstIndex = uiIndex;
		uiLastIndex = muiLastIndex;
	}
	else if (uiIndex > muiLastIndex)
	{
		uiFirstIndex = muiFirstIndex;
		uiLastIndex = uiIndex;
	}
	else
	{
		uiFirstIndex = muiFirstIndex;
		uiLastIndex = muiLastIndex;
	}

	return (uiLastIndex - uiFirstIndex + 1);   //+1 because uiLast is inclusive.  -1 because CIndexTreeNodeMemory already includes 1.	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNodeMemory::CalculateRequiredNodeSizeForIndex(unsigned char uiIndex)
{
	size_t	tSize;
	int		iNewIndices;

	iNewIndices = GetAdditionalIndexes(uiIndex);
	tSize = SizeofNode() + muiDataSize + iNewIndices * SizeofNodePtr();

	return tSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNodeMemory::CalculateRequiredNodeSizeForEmpty(void)
{
	return SizeofNode() + muiDataSize + SizeofNodePtr();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNodeMemory::CalculateRequiredNodeSizeForData(unsigned char uiDataSize)
{
	size_t	tSize;
	int		iExistingIndices;

	iExistingIndices = GetNumIndexes();
	tSize = SizeofNode() + uiDataSize + iExistingIndices * SizeofNodePtr();

	return tSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNodeMemory::CalculateRequiredNodeSizeForCurrent(void)
{
	size_t	tSize;
	int		iExistingIndices;

	iExistingIndices = GetNumIndexes();
	tSize = SizeofNode() + muiDataSize + iExistingIndices * SizeofNodePtr();

	return tSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeNodeMemory::GetNumIndexes(void)
{
	if (mbNodesEmpty)
	{
		return 0;
	}
	return GetNumIndexes(muiFirstIndex, muiLastIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeNodeMemory::GetNumIndexes(unsigned char uiFirstIndex, unsigned char uiLastIndex)
{
	return uiLastIndex - uiFirstIndex + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeNodeMemory::GetNode(int i)
{
	return GetNodes()[i];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory** CIndexTreeNodeMemory::GetNodes(void)
{
	return (CIndexTreeNodeMemory**)RemapSinglePointer(this, SizeofNode() + muiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeMemory* CIndexTreeNodeMemory::GetParent(void)
{
	return mpcParent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeMemory::RemapChildNodes(CIndexTreeNodeMemory* pcOldNode, CIndexTreeNodeMemory* pcNewNode)
{
	int					i;
	int					iNumNodes;
	CIndexTreeNodeMemory**	apcChildren;

	iNumNodes = GetNumIndexes();
	apcChildren = GetNodes();

	for (i = 0; i < iNumNodes; i++)
	{
		if (apcChildren[i] == pcOldNode)
		{
			apcChildren[i] = pcNewNode;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeMemory::SetChildsParent(void)
{
	int					i;
	int					iNumNodes;
	CIndexTreeNodeMemory*		pcChild;
	CIndexTreeNodeMemory**	apcChildren;

	iNumNodes = GetNumIndexes();
	apcChildren = GetNodes();

	for (i = 0; i < iNumNodes; i++)
	{
		pcChild = apcChildren[i];
		if (pcChild)
		{
			pcChild->mpcParent = this;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeNodeMemory::SizeObject(unsigned char uiSize)
{
	size_t				tSize;
	CIndexTreeNodeMemory**	apcChildren;
	int					iDiff;
	CIndexTreeNodeMemory**	apcMovedChildren;

	if (!mbNodesEmpty)
	{
		apcChildren = GetNodes();
		tSize = (muiLastIndex - muiFirstIndex + 1) * SizeofNodePtr();

		iDiff = (int)uiSize - (int)muiDataSize;

		apcMovedChildren = (CIndexTreeNodeMemory**)RemapSinglePointer(apcChildren, iDiff);
		memmove(apcMovedChildren, apcChildren, tSize);
	}

	muiDataSize = uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNodeMemory::SizeofNode(void)
{
	return sizeof(CIndexTreeNodeMemory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeNodeMemory::SizeofNodePtr(void)
{
	return sizeof(CIndexTreeNodeMemory*);
}

