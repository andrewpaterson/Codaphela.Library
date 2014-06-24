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
#ifndef __LINK_LIST_ALIGNED_H__
#define __LINK_LIST_ALIGNED_H__
#include "LinkListBlock.h"


struct SAlignedData
{
	unsigned int	iSize;
	void*			pvAlloc;  //This is the original allocation pointer.
	int				iAlignment;
	int				iOffset;
};


//Remember there is still space above this node which pvAlloc points to.
struct SDANode
{
	SAlignedData	sAligned;
	SDNode			sDNode;
};


class CLinkListAligned : public __CLinkListTemplate<void>
{
public:
	void		Init(void);
	void		Kill(void);
	void*		InsertAfterTail(unsigned int iSize, int iAlignment, int iOffset);
	void*		InsertBeforeHead(int iDataSize, int iAlignment, int iOffset);
	void*		InsertBeforeNode(void* psPos, int iDataSize, int iAlignment, int iOffset);
	void*		InsertAfterNode(void* psPos, int iDataSize, int iAlignment, int iOffset); 
	void*		AllocateDetached(int iDataSize, int iAlignment, int iOffset);
	void		Remove(void* pvData);
	BOOL		SafeRemove(void* pvData);
	void		FreeDetached(void* psNodeData);
	void		FreeNode(SDANode* psNode);
	void*		Grow(void* pvData, unsigned int uiNewSize);

	SDANode*	CalculateActualStart(void* pvMem, int iAlignment, int iOffset);
	SDANode*	GetNode(void* pvMem);

	int			ByteSize(void);

protected:
	int			GetNodeSize(void* pvMem);
};


#define CLinkListAlignedDataGetHeader(pvData)		DataGetHeaderMacro<SDANode, void>(pvData)
#define CLinkListAlignedHeaderGetData(pvHeader)		HeaderGetDataMacro<SDANode, void>(pvHeader)

#endif // __LINK_LIST_ALIGNED_H__

