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
#ifndef __LINK_LIST_BLOCK_H__
#define __LINK_LIST_BLOCK_H__
#include "LinkListTemplate.h"
#include "DataTypes.h"


struct SDUNode
{
	SUnknownType	sType;
	SDNode			sDNode;
};


class CLinkListBlock : public __CLinkListTemplate<void>
{
public:
	void	Init(void);
	void	Kill(void);
	int		GetNodeType(void* psData);
	void	GetNodeTypeAndSize(void* pvData, SUnknownType* psType);
	void	SetNodeTypeAndSize(void* pvData, SUnknownType* psType);
	void*	InsertAfterTail(int iDataSize);
	void*	InsertBeforeHead(int iDataSize);
	void*	InsertBeforeNode(int iDataSize, void* psPos);
	void*	InsertAfterNode(int iDataSize, void* psPos); 
	void*	AllocateDetached(int iDataSize);
	void	Remove(void* pvData);
	BOOL	SafeRemove(void* pvData);
	void	FreeDetached(void* psNodeData);

	int		ByteSize(void);

	BOOL	WriteLinkListBlock(CFileWriter* pcFileWriter);
	BOOL	ReadLinkListBlock(CFileReader* pcFileReader);

protected:
	int		GetNodeSize(void* psData);
};


#endif //__LINK_LIST_BLOCK_H__

