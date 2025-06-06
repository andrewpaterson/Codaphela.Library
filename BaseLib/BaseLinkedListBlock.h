/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "Malloc.h"
#include "LinkedList.h"


struct SLinkedListBlockDesc
{
	size	iNumElements;
	size	uiNodeSize;

	void Init(size iNumElements, size uiNodeSize);
};


struct SLinkedListTemplateDesc
{
	size	iNumElements;
	size	uiNodeSize;
	size	uiDataSize;

	void Init(size iNumElements, size uiNodeSize, size uiDataSize);
};


class CBaseLinkedListBlock : public CMalloc
{
protected:
	CLinkedList		mcList;
	size			muiNodeSize;

public:
	void		Init(size uiNodeSize);
	void		Init(CMallocator* pcMalloc, size uiNodeSize);
	void		Kill(void);

	void*		InsertAfterTail(size uiDataSize);
	void*		InsertBeforeHead(size uiDataSize);
	void*		InsertBeforeNode(size uiDataSize, void* psPos);
	void*		InsertAfterNode(size uiDataSize, void* psPos); 

	void* 		GetHead(void);
	void* 		GetTail(void);
	void* 		GetNext(void* pvData);
	void* 		GetPrev(void* pvData);

	void		RemoveTail(void);
	void		Remove(void* pvData);
	bool		SafeRemove(void* pvData);

	void		MoveToHead(void* pvData);
	void		MoveToTTail(void* pvData);

	void*		InsertDetachedAfterTail(SLLNode* psNode);
	void*		InsertDetachedBeforeHead(SLLNode* psNode);
	void*		InsertDetachedBeforeNode(SLLNode* psData, SLLNode* psPos);
	void*		InsertDetachedAfterNode(SLLNode* psData, SLLNode* psPos);

	void		Detach(SLLNode* psNodeHeader);
	void		Detach(void* pvData);

	void		FreeDetached(void* pvData);
	size		NumElements(void);

	void*		Get(size iNum);
	size		IndexOf(void* pvData);
	bool		IsInList(void* pvData);

	void		BubbleSort(DataCompare fCompare);
	void		InsertDetachedIntoSorted(DataCompare fCompare, void* pvData);

protected:	
	SLLNode*	AllocateDetached(size uiDataSize);

	void		Swap(void* psData1, void* psData2);
	void*		NodeGetData(SLLNode* psNode);
	SLLNode*	DataGetNode(void* pvData);
};


#endif // __BASE_LINKED_LIST_BLOCK_H__

