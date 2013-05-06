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
#ifndef __TREE_BLOCK_H__
#define __TREE_BLOCK_H__
#include "TreeTemplate.h"


struct STUNode
{
	SUnknownType	sType;
	STNode			sTNode;
};


typedef CTreeTemplate<void> __CTreeBlock;
class CTreeBlock : public __CTreeBlock
{
private:
	void	RecursiveFreeNodes(STNode *psNode);

public:
	void	Kill(void);
	int		GetNodeType(void* psData);
	int		GetNodeSize(void* psData);
	void	GetNodeTypeAndSize(void* pvData, SUnknownType* psType);
	void	SetNodeTypeAndSize(void* pvData, SUnknownType* psType);
	void*	InsertRoot(int iDataSize, int iDataType);
	void*	InsertOnRightOfChildren(void* psParent, int iDataSize, int iDataType);
	void*	InsertOnLeftOfChildren(void* psParent, int iDataSize, int iDataType);
	void*	InsertOnPath(int* aiPos, int iLevel, int iDataSize, int iDataType);
	void*	InsertOnPath(int* aiPos, int iLevel, int iDataSize, int iDataType, int* aiOldPos, int iOldLevel, void* psOldNode);
	void*	InsertAtChildNum(void* psParent, int iChildNum, int iDataSize, int iDataType);
	void*	InsertOnUp(void* psPos, int iDataSize, int iDataType);
	void*	InsertOnAcross(void* psPos, int iDataSize, int iDataType);
	void	FreeDetached(void* psNodeData);
	void*	AllocateDetached(int iDataSize, int iDataType);
	void	Detach(void* psNodeData);
	BOOL	RemoveLeaf(void* psNodeData);
	int		RemoveBranch(void* psNodeData);
	void	Remove(void* psNodeData);

	//Get malloc size.
	int		MallocSize(void);
};


#define CTreeUnknownDataGetHeader(pvData)	DataGetHeaderMacro<STUNode, void>(pvData)
#define CTreeUnknownHeaderGetData(pvHeader)	HeaderGetDataMacro<STUNode, void>(pvHeader)


#endif //__TREE_BLOCK_H__

