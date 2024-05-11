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
#ifndef __FREE_LIST_H__
#define __FREE_LIST_H__
#include "IntegerHelper.h"
#include "LinkedListBlock.h"
#include "LinkedListTemplate.h"
#include "DataTypes.h"
#include "IntegerHelper.h"


#define FREE_LIST_MAGIC	0x7b


class CFreeList;
struct SFNode
{
	uint8		bFull;
	int8		iOffset;
	size		uiChunkSize;
	CFreeList*	pcList;
};


struct SFreeListParams
{
	uint8		iAlignment;
	int8		iOffset;
	size		iElementSize;
};


struct SFreeListIterator
{
	SFNode*		psCurrent;  //Current FreeList node.
	size		iPosition;  //Element within this node.
};


class CFreeList
{
protected:
	CLinkedListBlock	mcList;
	SFNode*				mpsUnused;
	SFNode*				mpsNotFull;

	uint8				miOffset;
	int8				miSuppliedOffset;
	uint8				miAlignment;
	uint8				muiMagic;

	size				miStride;
	size				miElementSize;

public:
	void		Init(size iElementSize);
	void		Init(size iElementSize, uint16 iAlignment);
	void		Init(size iElementSize, uint16 iAlignment, int16 iOffset);
	void		Kill(void);

	void*		Add(SFNode** ppsNode = NULL);
	void*		Add(void* pvData);
	bool		Remove(void* pvData);
	bool		Remove(SFNode* psNode, void* pvData);
	void*		Get(size iElement);
	void*		StartIteration(SFreeListIterator* psIterator);
	void*		Iterate(SFreeListIterator* psIterator);
	void		RemoveDuringIteration(SFreeListIterator* psIterator);
	bool		Contains(void* pvData, bool bIsAllocated = true);
	size		NumElements(void);
	bool		HasElements(void);
	size		NumNodeElements(SFNode* psNode);
	void		SetAdditionalSize(size iSize);
	size		GetElementSize(void);
	size		GetElementStride(void);
	size		GetNumAllocatedChunks(void);
	SFNode*		GetNode(size iNode);
	void*		GetElement(SFNode* psNode, size iPosition);
	uint8		GetOffset(void);
	uint8		GetAlignment(void);
	void*		GetBitArray(SFNode* psNode);
	SFNode* 	FindNode(void* pvData, bool bIsAllocated = true);
	size		ByteSize(void);
	void*		GetFirstNodeElement(SFNode* psNode);
	void*		GetLastNodeElement(SFNode* psNode);
	void		RemoveNode(SFNode* psNode);
	void		GetParams(SFreeListParams* psParams);

protected:
	SFNode*		AllocateNew(void);
	void		Deallocate(SFNode* psNode);
	void*		AllocateExisting(SFNode* psNode, size iPosition);
	void*		AllocateNewSetFirst(void);
	void		FindNextAllocatedElement(SFreeListIterator* psIterator);
	void		FindPrevAllocatedElement(SFreeListIterator* psIterator);
	bool		RemoveExisiting(SFNode* psNode, size iPosition);
	size		CalculateStride(void);
	uint8		CalculateOffset(int16 iOffset);
	size		CalculateBitArraySize(size uiChunkSize);
	uint8		CalculateOffset(SFNode* psNode);
	void*		GetElementInNode(SFNode* psNode, size iPosition);
	bool		IsElementInNodeAllocated(SFNode* psNode, size iPosition);
	void		RemoveExisiting(SFNode* psNode, void* pvData);
};


typedef CLinkedListTemplate<CFreeList> CLinkListFreeList;


#endif // __FREE_LIST_H__

