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
	size	uiSize;
	void*	pvAlloc;  //This is the original allocation pointer.
	uint8	iAlignment;
	int8	iOffset;
};


struct SAlignedDataDesc
{
	size	uiSize;
	uint8	iAlignment;
	int8	iOffset;

	void	Init(size uiSize, uint8 iAlignment, int8 iOffset);
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
	void				Init(void);
	void				Init(CMallocator* pcMalloc);
	void				Kill(void);

	void*				InsertAfterTail(size uiSize, uint16 iAlignment, int16 iOffset);
	void*				InsertBeforeHead(size uiSize, uint16 iAlignment, int16 iOffset);
	void*				InsertBeforeNode(void* psPos, size uiSize, uint16 iAlignment, int16 iOffset);
	void*				InsertAfterNode(void* psPos, size uiSize, uint16 iAlignment, int16 iOffset); 
	void*				Add(size uiSize, uint16 iAlignment);

	SLLAlignedNode*		AllocateDetached(size uiSize, uint16 iAlignment, int16 iOffset);
	void				Remove(void* pvData);
	bool				SafeRemove(void* pvData);
	void				FreeDetached(void* psNodeData);
	void				FreeNode(SLLAlignedNode* psNode);
	void*				Grow(void* pvData, size uiNewSize);

	bool				Write(CFileWriter* pcFileWriter);
	bool				Read(CFileReader* pcFileReader);

	SLLAlignedNode*		CalculateActualStart(void* pvMem, uint16 iAlignment, int16 iOffset);
	SLLAlignedNode*		GetNode(void* pvMem);

	size				ByteSize(void);

protected:	
	size				GetNodeSize(void* pvMem);

	bool				WriteHeader(CFileWriter* pcFileWriter);
	bool				WriteData(CFileWriter* pcFileWriter);
	bool				ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc, size* piNumElements);
	bool				ReadData(CFileReader* pcFileReader, size iNumElements);
};


#endif // __LINK_LIST_ALIGNED_H__

