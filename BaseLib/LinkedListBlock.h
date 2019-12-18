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
	unsigned int	uiSize;
	SLLNode*		psNext;
	SLLNode*		psPrev;
};


struct SLinkedListBlockDesc
{
	int		iNumElements;

	void Init(int iNumElements);
};


class CLinkedListBlock
{
protected:
	SLLNode*		mpsHead;
	SLLNode*		mpsTail;
	CMallocator*	mpcMalloc;

public:
	void	Init(void);
	void	Init(CMallocator* pcMalloc);
	void	Kill(void);

	void*	InsertAfterTail(unsigned int uiDataSize);
	void*	InsertBeforeHead(unsigned int uiDataSize);
	void*	InsertBeforeNode(unsigned int uiDataSize, void* psPos);
	void*	InsertAfterNode(unsigned int uiDataSize, void* psPos); 
	void*	AllocateDetached(unsigned int uiDataSize);

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

	BOOL	Write(CFileWriter* pcFileWriter);
	BOOL	Read(CFileReader* pcFileReader);

	void	BubbleSort(int(*)(const void*, const void*));

protected:	
	void*			Malloc(size_t tSize);
	void*			Realloc(void* pv, size_t iMemSize);
	void			Free(void* pv);

	void			Swap(void* psData1, void* psData2);
	unsigned int	GetNodeSize(void* psData);

	BOOL			WriteAllocatorAndHeader(CFileWriter* pcFileWriter);
	BOOL			ReadAllocatorAndHeader(CFileReader* pcFileReader, SLinkedListBlockDesc* psDesc);
	BOOL			WriteHeader(CFileWriter* pcFileWriter);
	BOOL			ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc, SLinkedListBlockDesc* psDesc);
};


#endif // __TEMPLATE_BASE_H__

