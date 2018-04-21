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
#ifndef __TEMPLATE_BASE_H__
#define __TEMPLATE_BASE_H__
#include "Define.h"
#include "DataMacro.h"
#include "FileIO.h"
#include "Mallocator.h"


//For the LinkedList.
struct SLLNode
{
	int			iSize;
	SLLNode*	psNext;
	SLLNode*	psPrev;
};


class CLinkedListBlock
{
protected:
	SLLNode*		mpsHead; 
	SLLNode*		mpsTail;
	int				miNumElements;  //Remove this.
	CMallocator*	mpcMalloc;

public:
	void	Init(void);
	void	Init(CMallocator* pcMalloc);
	void	Kill(void);

	void*	InsertAfterTail(int iDataSize);
	void*	InsertBeforeHead(int iDataSize);
	void*	InsertBeforeNode(int iDataSize, void* psPos);
	void*	InsertAfterNode(int iDataSize, void* psPos); 
	void*	AllocateDetached(int iDataSize);

	void* 	GetHead(void);
	void* 	GetTail(void);
	void* 	GetNext(void* pvData);
	void* 	GetPrev(void* pvData);

	void	RemoveTail(void);
	void	Remove(void* pvData);
	BOOL	SafeRemove(void* pvData);

	void	InsertDetachedAfterTail(void* psNode);
	void	InsertDetachedBeforeHead(void* psNode);
	void	InsertDetachedBeforeNode(void* psData, void* psPos);
	void	InsertDetachedAfterNode(void* psData, void* psPos);
	void	InsertDetachedIntoSorted(int(*)(const void*, const void*), void* psNode);

	void	Detach(SLLNode* psNodeHeader);
	void	Detach(void* pvData);

	void	FreeDetached(void* pvData);
	int		NumElements(void);
	int		ByteSize(void);

	void*	Get(int iNum);
	int		IndexOf(void* pvData);
	BOOL	IsInList(void* pvData);

	BOOL	WriteLinkListBlock(CFileWriter* pcFileWriter);
	BOOL	ReadLinkListBlock(CFileReader* pcFileReader);

	void	BubbleSort(int(*)(const void*, const void*));

protected:	
	void*	Malloc(size_t tSize);
	void*	Realloc(void* pv, size_t iMemSize);
	void	Free(void* pv);

	void	Swap(void* psData1, void* psData2);
	int		GetNodeSize(void* psData);
};


#endif // __TEMPLATE_BASE_H__

