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
#ifndef __TREE_BLOCK_H__
#define __TREE_BLOCK_H__
#include "TreeTemplate.h"


struct STUNode
{
	int		iSize;
	STNode	sTNode;
};


typedef CTreeTemplate<void> __CTreeBlock;
class CTreeBlock : public __CTreeBlock
{
public:
	void	Kill(void);

	int		GetNodeSize(void* psData);

	void*	InsertRoot(int iDataSize);
	void*	InsertOnRightOfChildren(void* psParent, int iDataSize);
	void*	InsertOnLeftOfChildren(void* psParent, int iDataSize);
	void*	InsertOnPath(int* aiPos, int iLevel, int iDataSize);
	void*	InsertOnPath(int* aiPos, int iLevel, int iDataSize, int* aiOldPos, int iOldLevel, void* psOldNode);
	void*	InsertAtChildNum(void* psParent, int iChildNum, int iDataSize);
	void*	InsertOnUp(void* psPos, int iDataSize);
	void*	InsertOnAcross(void* psPos, int iDataSize);

	void	Detach(void* psNodeData);
	BOOL	RemoveLeaf(void* psNodeData);
	int		RemoveBranch(void* psNodeData);
	void	Remove(void* psNodeData);

	int		ByteSize(void);
	
	BOOL	WriteTreeUnknown(CFileWriter* pcFileWriter);
	BOOL	ReadTreeUnknown(CFileReader* pcFileReader);

protected:
	void	FreeDetached(void* psNodeData);
	void*	AllocateDetached(int iDataSize);
	void	RecursiveFreeNodes(STNode *psNode);
};


#endif // !__TREE_BLOCK_H__

