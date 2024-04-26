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
#ifndef __LINK_LIST_ALIGNED_H__
#define __LINK_LIST_ALIGNED_H__
#include "LinkedListBlock.h"
#include "LinkedListTemplate.h"


struct SAlignedData
{
	uint32	uiSize;
	void*	pvAlloc;  //This is the original allocation pointer.
	int		iAlignment;
	int		iOffset;
};


struct SAlignedDataDesc
{
	uint32	uiSize;
	int		iAlignment;
	int		iOffset;

	void	Init(uint32 uiSize, int iAlignment, int iOffset);
};


//Remember there is still space above this node which pvAlloc points to.
struct SLLAlignedNode
{
	SAlignedData	sAligned;
	SLLNode			sNode;
};


class CLinkedListBlockAligned : public CBaseLinkedListBlock
{
public:
	void			Init(void);
	void			Init(CMallocator* pcMalloc);
	void			Kill(void);

	void*			InsertAfterTail(uint32 iSize, int iAlignment, int iOffset);
	void*			InsertBeforeHead(uint32 uiSize, int iAlignment, int iOffset);
	void*			InsertBeforeNode(void* psPos, uint32 uiSize, int iAlignment, int iOffset);
	void*			InsertAfterNode(void* psPos, uint32 uiSize, int iAlignment, int iOffset); 
	void*			Add(uint32 uiSize, int iAlignment);

	SLLAlignedNode*	AllocateDetached(uint32 uiSize, int iAlignment, int iOffset);
	void			Remove(void* pvData);
	bool			SafeRemove(void* pvData);
	void			FreeDetached(void* psNodeData);
	void			FreeNode(SLLAlignedNode* psNode);
	void*			Grow(void* pvData, uint32 uiNewSize);

	bool			Write(CFileWriter* pcFileWriter);
	bool			Read(CFileReader* pcFileReader);

	SLLAlignedNode*	CalculateActualStart(void* pvMem, int iAlignment, int iOffset);
	SLLAlignedNode*	GetNode(void* pvMem);

	int				ByteSize(void);

protected:	
	int				GetNodeSize(void* pvMem);

	bool			WriteHeader(CFileWriter* pcFileWriter);
	bool			WriteData(CFileWriter* pcFileWriter);
	bool			ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc, int* piNumElements);
	bool			ReadData(CFileReader* pcFileReader, int iNumElements);
};


#endif // !__LINK_LIST_ALIGNED_H__

