/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "IntegerHelper.h"
#include "SystemAllocator.h"
#include "GlobalMemory.h"
#include "LinkedListBlockAligned.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlockAligned::Init(void)
{
	CBaseLinkedListBlock::Init(sizeof(SLLNode));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlockAligned::Kill(void)
{
	SLLAlignedNode*		psNode;
	SLLAlignedNode*		psNode2;
	void*				pvData;

	pvData = HeaderGetData<SLLNode, void>((SLLNode*)mcList.GetHead());  //Yes this is the correct macro.
	psNode = DataGetHeader<SLLAlignedNode, void>(pvData);
	while (psNode)
	{
		pvData = HeaderGetData<SLLNode, void>((SLLNode*)psNode->sNode.psNext);  //Yes this is the correct macro.
		psNode2 = DataGetHeader<SLLAlignedNode, void>(pvData);
		FreeNode(psNode);
		psNode = psNode2;
	}
	mcList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlockAligned::FreeNode(SLLAlignedNode* psNode)
{
	Free(psNode->sAligned.pvAlloc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlockAligned::InsertAfterTail(unsigned int uiSize, int iAlignment, int iOffset)
{
	SLLAlignedNode*	psNode;

	psNode = AllocateDetached(uiSize, iAlignment, iOffset);
	return CBaseLinkedListBlock::InsertDetachedAfterTail(&psNode->sNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlockAligned::InsertBeforeHead(unsigned int uiSize, int iAlignment, int iOffset)
{
	SLLAlignedNode* psNode;

	psNode = AllocateDetached(uiSize, iAlignment, iOffset);
	return CBaseLinkedListBlock::InsertDetachedBeforeHead(&psNode->sNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlockAligned::InsertBeforeNode(void* psPos, unsigned int uiSize, int iAlignment, int iOffset)
{
	SLLAlignedNode* psNode;
	SLLAlignedNode* psNodePos;

	psNode = AllocateDetached(uiSize, iAlignment, iOffset);
	psNodePos = DataGetHeader<SLLAlignedNode, void>(psPos);
	return CBaseLinkedListBlock::InsertDetachedBeforeNode(&psNode->sNode, &psNodePos->sNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlockAligned::InsertAfterNode(void* psPos, unsigned int uiSize, int iAlignment, int iOffset)
{
	SLLAlignedNode* psNode;
	SLLAlignedNode* psNodePos;

	psNode = AllocateDetached(uiSize, iAlignment, iOffset);
	psNodePos = DataGetHeader<SLLAlignedNode, void>(psPos);
	return CBaseLinkedListBlock::InsertDetachedAfterNode(&psNode->sNode, &psNodePos->sNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SLLAlignedNode* CLinkedListBlockAligned::AllocateDetached(unsigned int uiSize, int iAlignment, int iOffset)
{
	void*				pvMem;
	int					iTotalSize;
	SLLAlignedNode*		psNode;
	int					iCaclulatedOffset;

	iCaclulatedOffset = ::CalculateOffset(iOffset - sizeof(SLLAlignedNode), iAlignment);
	iTotalSize = uiSize + sizeof(SLLAlignedNode) + iAlignment - 1;

	pvMem = Malloc(iTotalSize);
	if (pvMem != NULL)
	{
		psNode = CalculateActualStart(pvMem, iAlignment, iCaclulatedOffset);
		psNode->sAligned.iAlignment = iAlignment;
		psNode->sAligned.iOffset = iOffset;
		psNode->sAligned.uiSize = uiSize;
		psNode->sAligned.pvAlloc = pvMem;

		return psNode;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlockAligned::Add(unsigned int uiSize, int iAlignment)
{
	return InsertAfterTail(uiSize, iAlignment, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CLinkedListBlockAligned::ByteSize(void)
{
	int		iSize;
	void*	pvNode;

	iSize = 0;

	pvNode = GetHead();
	while(pvNode)
	{
		iSize += GetNodeSize(pvNode);
		pvNode = GetNext(pvNode);
	}

	return iSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CLinkedListBlockAligned::GetNodeSize(void* pvMem)
{
	SLLAlignedNode*		psNodeHeader;

	psNodeHeader = DataGetHeader<SLLAlignedNode, void>(pvMem);
	return psNodeHeader->sAligned.uiSize + sizeof(SLLAlignedNode) + psNodeHeader->sAligned.iAlignment-1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SLLAlignedNode* CLinkedListBlockAligned::CalculateActualStart(void* pvMem, int iAlignment, int iOffset)
{
	size_t	iStart;
	size_t	iByteDiff;
	size_t	iActualOffset;

	iStart = (size_t)pvMem;

	iByteDiff = iStart % iAlignment;
	if (iByteDiff == 0)
	{
		return (SLLAlignedNode*)(size_t)(iStart + iOffset);
	}
	else
	{
		iActualOffset = (((size_t)iAlignment - iByteDiff) + iOffset) % (size_t)iAlignment;
		return (SLLAlignedNode*)(size_t)(iStart + iActualOffset);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SLLAlignedNode* CLinkedListBlockAligned::GetNode(void* pvMem)
{
	return (SLLAlignedNode*)(size_t) ((unsigned int)(size_t) pvMem - sizeof(SLLAlignedNode));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlockAligned::Remove(void* pvData)
{
	Detach(pvData);
	FreeDetached(pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLinkedListBlockAligned::SafeRemove(void* pvData)
{
	if (IsInList(pvData))
	{
		Remove(pvData);
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlockAligned::FreeDetached(void* pvData)
{
	SLLAlignedNode*		psNodeHeader;

	psNodeHeader = DataGetHeader<SLLAlignedNode, void>(pvData);
	if (psNodeHeader)
	{
		FreeNode(psNodeHeader);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlockAligned::Grow(void* pvData, unsigned int uiNewSize)
{
	SLLAlignedNode*		psNodeHeader;
	void*				pvAllocatedEnd;
	void*				pvObjectEnd;
	void*				pvNew;
	unsigned int		uiSize;

	psNodeHeader = DataGetHeader<SLLAlignedNode, void>(pvData);

	if (uiNewSize == 0)
	{
		Remove(pvData);
		return NULL;
	}

	pvAllocatedEnd = (void*)(size_t) ((int)(size_t) psNodeHeader->sAligned.pvAlloc + sizeof(SLLAlignedNode) + psNodeHeader->sAligned.uiSize + psNodeHeader->sAligned.iAlignment-1);
	pvObjectEnd = (void*)(size_t) ((int)(size_t) pvData + uiNewSize);

	if (pvAllocatedEnd >= pvObjectEnd)
	{
		psNodeHeader->sAligned.uiSize = uiNewSize;
		return pvData;
	}
	else
	{
		pvNew = InsertAfterNode(pvData, uiNewSize, psNodeHeader->sAligned.iAlignment, psNodeHeader->sAligned.iOffset);
		uiSize = (uiNewSize < psNodeHeader->sAligned.uiSize) ? uiNewSize : psNodeHeader->sAligned.uiSize;
		memcpy(pvNew, pvData, uiSize);
		Remove(pvData);
		return pvNew;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CLinkedListBlockAligned::Write(CFileWriter* pcFileWriter)
{
	BOOL	bResult;

	bResult = gcMallocators.WriteMallocator(pcFileWriter, mpcMalloc);
	if (!bResult)
	{
		return FALSE;
	}

	bResult = WriteHeader(pcFileWriter);
	if (!bResult)
	{
		return FALSE;
	}

	bResult = WriteData(pcFileWriter);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CLinkedListBlockAligned::WriteData(CFileWriter* pcFileWriter)
{
	void*				pvData;
	unsigned int		uiSize;
	SLLAlignedNode*		psNode;
	SAlignedDataDesc	sDataDesc;

	pvData = GetHead();
	while (pvData)
	{

		psNode = DataGetHeader<SLLAlignedNode, void>(pvData);

		uiSize = psNode->sAligned.uiSize;
		sDataDesc.Init(uiSize, psNode->sAligned.iAlignment, psNode->sAligned.iOffset);

		if (!pcFileWriter->WriteData(&sDataDesc, sizeof(SAlignedDataDesc)))
		{
			return FALSE;
		}
		if (!pcFileWriter->WriteData(pvData, uiSize))
		{
			return FALSE;
		}

		pvData = GetNext(pvData);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CLinkedListBlockAligned::Read(CFileReader* pcFileReader)
{
	int				iNumElements;
	BOOL			bResult;
	CMallocator*	pcMalloc;

	pcMalloc = gcMallocators.ReadMallocator(pcFileReader);
	if (pcMalloc == NULL)
	{
		return FALSE;
	}

	bResult = ReadHeader(pcFileReader, pcMalloc, &iNumElements);
	if (!bResult)
	{
		return FALSE;
	}

	bResult = ReadData(pcFileReader, iNumElements);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CLinkedListBlockAligned::ReadData(CFileReader* pcFileReader, int iNumElements)
{
	int					i;
	void*				pvData;
	unsigned int		uiSize;
	SAlignedDataDesc	sDataDesc;

	for (i = 0; i < iNumElements; i++)
	{
		if (!pcFileReader->ReadData(&sDataDesc, sizeof(SAlignedDataDesc)))
		{
			return FALSE;
		}

		uiSize = sDataDesc.uiSize;
		pvData = InsertAfterTail(uiSize, sDataDesc.iAlignment, sDataDesc.iOffset);
		if (!pcFileReader->ReadData(pvData, uiSize))
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SAlignedDataDesc::Init(unsigned int uiSize, int iAlignment, int iOffset)
{
	this->uiSize = uiSize;
	this->iAlignment = iAlignment;
	this->iOffset = iOffset;
}

