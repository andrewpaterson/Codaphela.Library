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
#ifndef __TREE_BLOCK_H__
#define __TREE_BLOCK_H__
#include "TreeTemplate.h"


struct STUNode
{
	size	iSize;
	STNode	sTNode;
};


typedef CTreeTemplate<void> __CTreeBlock;
class CTreeBlock : public __CTreeBlock
{
public:
	void	Kill(void);

	size		GetNodeSize(void* psData);

	void*	InsertRoot(size uiDataSize);
	void*	InsertOnRightOfChildren(void* psParent, size uiDataSize);
	void*	InsertOnLeftOfChildren(void* psParent, size uiDataSize);
	void*	InsertOnPath(size* auiPos, size uiLevel, size uiDataSize);
	void*	InsertOnPath(size* auiPos, size uiLevel, size uiDataSize, size* auiOldPos, size uiOldLevel, void* psOldNode);
	void*	InsertAtChildNum(void* psParent, size iChildNum, size uiDataSize);
	void*	InsertOnUp(void* psPos, size uiDataSize);
	void*	InsertOnAcross(void* psPos, size uiDataSize);

	void	Detach(void* psNodeData);
	bool	RemoveLeaf(void* psNodeData);
	size	RemoveBranch(void* psNodeData);
	void	Remove(void* psNodeData);

	size	ByteSize(void);
	
	bool	WriteTreeUnknown(CFileWriter* pcFileWriter);
	bool	ReadTreeUnknown(CFileReader* pcFileReader);

protected:
	void	FreeDetached(void* psNodeData);
	void*	AllocateDetached(size uiDataSize);
	void	RecursiveFreeNodes(STNode *psNode);
};


#endif // __TREE_BLOCK_H__

