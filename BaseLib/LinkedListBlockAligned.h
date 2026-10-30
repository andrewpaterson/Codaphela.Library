/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
	lsize	uiSize;
	void*	pvAlloc;  //This is the original allocation pointer.
	uint8	iAlignment;
	int8	iOffset;
};


struct SAlignedDataDesc
{
	lsize	uiSize;
	uint8	iAlignment;
	int8	iOffset;

	void	Init(lsize uiSize, uint8 iAlignment, int8 iOffset);
	bool	Write(CFileWriter* pcFileWriter);
	bool	Read(CFileReader* pcFileReader);
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

	void*				InsertAfterTail(lsize uiSize, uint16 iAlignment, int16 iOffset);
	void*				InsertBeforeHead(lsize uiSize, uint16 iAlignment, int16 iOffset);
	void*				InsertBeforeNode(void* psPos, lsize uiSize, uint16 iAlignment, int16 iOffset);
	void*				InsertAfterNode(void* psPos, lsize uiSize, uint16 iAlignment, int16 iOffset); 
	void*				Add(lsize uiSize, uint16 iAlignment);

	SLLAlignedNode*		AllocateDetached(lsize uiSize, uint16 iAlignment, int16 iOffset);
	void				Remove(void* pvData);
	bool				SafeRemove(void* pvData);
	void				FreeDetached(void* psNodeData);
	void				FreeNode(SLLAlignedNode* psNode);
	void*				Grow(void* pvData, lsize uiNewSize);

	bool				Write(CFileWriter* pcFileWriter);
	bool				Read(CFileReader* pcFileReader);

	SLLAlignedNode*		CalculateActualStart(void* pvMem, uint16 iAlignment, int16 iOffset);
	SLLAlignedNode*		GetNode(void* pvMem);

	lsize				ByteSize(void);

protected:	
	lsize				GetNodeSize(void* pvMem);

	bool				WriteHeader(CFileWriter* pcFileWriter);
	bool				WriteData(CFileWriter* pcFileWriter);
	bool				ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc, size* piNumElements);
	bool				ReadData(CFileReader* pcFileReader, size iNumElements);
};


#endif // __LINK_LIST_ALIGNED_H__

