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
#ifndef __LINKED_LIST_BLOCK_H__
#define __LINKED_LIST_BLOCK_H__
#include "BaseLinkedListBlock.h"


struct SLLBlockNode : public SLLNode
{
	uint32	uiSize;
};


class CLinkedListBlock : public CBaseLinkedListBlock
{
public:
	void			Init(void);
	void			Init(CMallocator* pcMalloc);
	void			Kill(void);

	void*			InsertAfterTail(uint32 uiDataSize);
	void*			InsertBeforeHead(uint32 uiDataSize);
	void*			InsertBeforeNode(uint32 uiDataSize, void* psPos);
	void*			InsertAfterNode(uint32 uiDataSize, void* psPos); 
	void*			Add(uint32 uiDataSize);

	int				ByteSize(void);

	bool			Write(CFileWriter* pcFileWriter);
	bool			Read(CFileReader* pcFileReader);

	void			InsertDetachedAfterTail(void* pvData);

protected:	
	uint32			GetNodeSize(void* psData);
	SLLBlockNode*	AllocateDetached(uint32 uiDataSize);
	SLLBlockNode*	DataGetNode(void *pvData);

	bool			WriteHeader(CFileWriter* pcFileWriter);
	bool			WriteData(CFileWriter* pcFileWriter);
	bool			ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc, int* piNumElements);
	bool			ReadData(CFileReader* pcFileReader, int iNumElements);
};


#endif // !__LINKED_LIST_BLOCK_H__

