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
#ifndef __BASE_LINKED_LIST_BLOCK_H__
#define __BASE_LINKED_LIST_BLOCK_H__
#include "DataMacro.h"
#include "FileIO.h"
#include "Alloc.h"
#include "LinkedList.h"


struct SLinkedListBlockDesc
{
	int				iNumElements;
	unsigned int	uiNodeSize;

	void Init(int iNumElements, unsigned int uiNodeSize);
};


struct SLinkedListTemplateDesc
{
	int				iNumElements;
	unsigned int	uiNodeSize;
	unsigned int	uiDataSize;

	void Init(int iNumElements, unsigned int uiNodeSize, unsigned int uiDataSize);
};


class CBaseLinkedListBlock : public CAlloc
{
protected:
	CLinkedList		mcList;
	size_t			muiNodeSize;

public:
	void		Init(size_t uiNodeSize);
	void		Init(CMallocator* pcMalloc, size_t uiNodeSize);
	void		Kill(void);

	void*		InsertAfterTail(unsigned int uiDataSize);
	void*		InsertBeforeHead(unsigned int uiDataSize);
	void*		InsertBeforeNode(unsigned int uiDataSize, void* psPos);
	void*		InsertAfterNode(unsigned int uiDataSize, void* psPos); 

	void* 		GetHead(void);
	void* 		GetTail(void);
	void* 		GetNext(void* pvData);
	void* 		GetPrev(void* pvData);

	void		RemoveTail(void);
	void		Remove(void* pvData);
	BOOL		SafeRemove(void* pvData);

	void		MoveToHead(void* pvData);
	void		MoveToTTail(void* pvData);

	void*		InsertDetachedAfterTail(SLLNode* psNode);
	void*		InsertDetachedBeforeHead(SLLNode* psNode);
	void*		InsertDetachedBeforeNode(SLLNode* psData, SLLNode* psPos);
	void*		InsertDetachedAfterNode(SLLNode* psData, SLLNode* psPos);

	void		Detach(SLLNode* psNodeHeader);
	void		Detach(void* pvData);

	void		FreeDetached(void* pvData);
	int			NumElements(void);

	void*		Get(int iNum);
	int			IndexOf(void* pvData);
	BOOL		IsInList(void* pvData);

	void		BubbleSort(int(*fCompare)(const void*, const void*));
	void		InsertDetachedIntoSorted(int(*fCompare)(const void*, const void*), void* pvData);

protected:	
	SLLNode*	AllocateDetached(size_t uiDataSize);

	void		Swap(void* psData1, void* psData2);
	void*		NodeGetData(SLLNode* psNode);
	SLLNode*	DataGetNode(void* pvData);
};


#endif // __BASE_LINKED_LIST_BLOCK_H__

