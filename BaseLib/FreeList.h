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
#ifndef __FREE_LIST_BASE_H__
#define __FREE_LIST_BASE_H__
#include "IntegerHelper.h"
#include "LinkedListTemplate.h"
#include "DataTypes.h"
#include "IntegerHelper.h"


class CFreeList;
struct SFNode
{
	unsigned char	bFull;
	char			iOffset;
	unsigned short  uiChunkSize;
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

	int					miStride;

	int					miElementSize;

public:
	void		Init(int iElementSize);
	void		Init(int iElementSize, char iAlignment);
	void		Init(int iElementSize, char iAlignment, char iOffset);
	void		Kill(void);

	void*		Add(SFNode** ppsNode = NULL);
	void*		Add(void* pvData);
	BOOL		Remove(void* pvData);
	BOOL		Remove(SFNode* psNode, void* pvData);
	void*		Get(int iElement);
	void*		StartIteration(SFreeListIterator* psIterator);
	void*		Iterate(SFreeListIterator* psIterator);
	void		RemoveDuringIteration(SFreeListIterator* psIterator);
	BOOL		Contains(void* pvData, BOOL bIsAllocated = TRUE);
	int			NumElements(void);
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
	SFNode* 	FindNode(void* pvData, BOOL bIsAllocated = TRUE);
	int			ByteSize(void);
	void*		GetFirstNodeElement(SFNode* psNode);
	void*		GetLastNodeElement(SFNode* psNode);
	void		RemoveNode(SFNode* psNode);
	void		GetParams(SFreeListParams* psParams);

protected:
	SFNode* AllocateNew(void);
	void	Deallocate(SFNode* psNode);
	void*	AllocateExisting(SFNode* psNode, int iPosition);
	void*	AllocateNewSetFirst(void);
	void	FindNextAllocatedElement(SFreeListIterator* psIterator);
	void	FindPrevAllocatedElement(SFreeListIterator* psIterator);
	BOOL	RemoveExisiting(SFNode* psNode, int iPosition);
	int		CalculateStride(void);
	char	CalculateOffset(int iOffset);
	int		CalculateBitArraySize(unsigned short uiChunkSize);
	int		CalculateOffset(SFNode* psNode);
	void*	GetElementInNode(SFNode* psNode, int iPosition);
	BOOL	IsElementInNodeAllocated(SFNode* psNode, int iPosition);
	void	RemoveExisiting(SFNode* psNode, void* pvData);
};


typedef CLinkedListTemplate<CFreeList> CLinkListFreeList;


#endif // __FREE_LIST_BASE_H__

