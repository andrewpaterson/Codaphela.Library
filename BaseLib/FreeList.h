/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#ifndef __FREE_LIST_BASE_H__
#define __FREE_LIST_BASE_H__
#include "IntegerHelper.h"
#include "LinkedListBlock.h"
#include "LinkedListTemplate.h"
#include "DataTypes.h"
#include "IntegerHelper.h"


#define FREE_LIST_MAGIC	0x7b


class CFreeList;
struct SFNode
{
	unsigned char	bFull;
	char			iOffset;
	uint16			uiChunkSize;
	CFreeList*		pcList;
};


struct SFreeListParams
{
	int		iAlignment;
	char	iOffset;
	int		iElementSize;
};


struct SFreeListIterator
{
	SFNode*		psCurrent;  //Current FreeList node.
	int			iPosition;  //Element within this node.
};


class CFreeList
{
protected:
	CLinkedListBlock	mcList;
	SFNode*				mpsUnused;
	SFNode*				mpsNotFull;

	char				miOffset;
	char				miSuppliedOffset;
	char				miAlignment;
	unsigned char		muiMagic;

	uint16		miStride;
	uint16		miElementSize;

public:
	void		Init(uint16 iElementSize);
	void		Init(uint16 iElementSize, char iAlignment);
	void		Init(uint16 iElementSize, char iAlignment, char iOffset);
	void		Kill(void);

	void*		Add(SFNode** ppsNode = NULL);
	void*		Add(void* pvData);
	bool		Remove(void* pvData);
	bool		Remove(SFNode* psNode, void* pvData);
	void*		Get(int iElement);
	void*		StartIteration(SFreeListIterator* psIterator);
	void*		Iterate(SFreeListIterator* psIterator);
	void		RemoveDuringIteration(SFreeListIterator* psIterator);
	bool		Contains(void* pvData, bool bIsAllocated = true);
	int			NumElements(void);
	bool		HasElements(void);
	int			NumNodeElements(SFNode* psNode);
	void		SetAdditionalSize(int iSize);
	int			GetElementSize(void);
	int			GetElementStride(void);
	int			GetNumAllocatedChunks(void);
	SFNode*		GetNode(int iNode);
	void*		GetElement(SFNode* psNode, int iPosition);
	char		GetOffset(void);
	int			GetAlignment(void);
	void*		GetBitArray(SFNode* psNode);
	SFNode* 	FindNode(void* pvData, bool bIsAllocated = true);
	int			ByteSize(void);
	void*		GetFirstNodeElement(SFNode* psNode);
	void*		GetLastNodeElement(SFNode* psNode);
	void		RemoveNode(SFNode* psNode);
	void		GetParams(SFreeListParams* psParams);

protected:
	SFNode*		AllocateNew(void);
	void		Deallocate(SFNode* psNode);
	void*		AllocateExisting(SFNode* psNode, int iPosition);
	void*		AllocateNewSetFirst(void);
	void		FindNextAllocatedElement(SFreeListIterator* psIterator);
	void		FindPrevAllocatedElement(SFreeListIterator* psIterator);
	bool		RemoveExisiting(SFNode* psNode, int iPosition);
	int			CalculateStride(void);
	char		CalculateOffset(int iOffset);
	int			CalculateBitArraySize(uint16 uiChunkSize);
	int			CalculateOffset(SFNode* psNode);
	void*		GetElementInNode(SFNode* psNode, int iPosition);
	bool		IsElementInNodeAllocated(SFNode* psNode, int iPosition);
	void		RemoveExisiting(SFNode* psNode, void* pvData);
};


typedef CLinkedListTemplate<CFreeList> CLinkListFreeList;


#endif // !__FREE_LIST_BASE_H__

