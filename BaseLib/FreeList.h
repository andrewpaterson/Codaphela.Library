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
	uint16		uiChunkSize;
	CFreeList*	pcList;
};


struct SFreeListParams
{
	int32	iAlignment;
	char	iOffset;
	int32	iElementSize;
};


struct SFreeListIterator
{
	SFNode*		psCurrent;  //Current FreeList node.
	int32		iPosition;  //Element within this node.
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
	uint8				muiMagic;

	uint16				miStride;
	uint16				miElementSize;

public:
	void		Init(uint16 iElementSize);
	void		Init(uint16 iElementSize, char iAlignment);
	void		Init(uint16 iElementSize, char iAlignment, char iOffset);
	void		Kill(void);

	void*		Add(SFNode** ppsNode = NULL);
	void*		Add(void* pvData);
	bool		Remove(void* pvData);
	bool		Remove(SFNode* psNode, void* pvData);
	void*		Get(int32 iElement);
	void*		StartIteration(SFreeListIterator* psIterator);
	void*		Iterate(SFreeListIterator* psIterator);
	void		RemoveDuringIteration(SFreeListIterator* psIterator);
	bool		Contains(void* pvData, bool bIsAllocated = true);
	int32		NumElements(void);
	bool		HasElements(void);
	int32		NumNodeElements(SFNode* psNode);
	void		SetAdditionalSize(int32 iSize);
	int32		GetElementSize(void);
	int32		GetElementStride(void);
	int32		GetNumAllocatedChunks(void);
	SFNode*		GetNode(int32 iNode);
	void*		GetElement(SFNode* psNode, int32 iPosition);
	char		GetOffset(void);
	int32		GetAlignment(void);
	void*		GetBitArray(SFNode* psNode);
	SFNode* 	FindNode(void* pvData, bool bIsAllocated = true);
	int32		ByteSize(void);
	void*		GetFirstNodeElement(SFNode* psNode);
	void*		GetLastNodeElement(SFNode* psNode);
	void		RemoveNode(SFNode* psNode);
	void		GetParams(SFreeListParams* psParams);

protected:
	SFNode*		AllocateNew(void);
	void		Deallocate(SFNode* psNode);
	void*		AllocateExisting(SFNode* psNode, int32 iPosition);
	void*		AllocateNewSetFirst(void);
	void		FindNextAllocatedElement(SFreeListIterator* psIterator);
	void		FindPrevAllocatedElement(SFreeListIterator* psIterator);
	bool		RemoveExisiting(SFNode* psNode, int32 iPosition);
	int32		CalculateStride(void);
	char		CalculateOffset(int32 iOffset);
	int32		CalculateBitArraySize(uint16 uiChunkSize);
	int32		CalculateOffset(SFNode* psNode);
	void*		GetElementInNode(SFNode* psNode, int32 iPosition);
	bool		IsElementInNodeAllocated(SFNode* psNode, int32 iPosition);
	void		RemoveExisiting(SFNode* psNode, void* pvData);
};


typedef CLinkedListTemplate<CFreeList> CLinkListFreeList;


#endif // __FREE_LIST_H__

