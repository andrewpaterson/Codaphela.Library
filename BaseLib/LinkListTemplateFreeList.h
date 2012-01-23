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
#ifndef __FREE_LINK_LIST_TEMPLATE__
#define __FREE_LINK_LIST_TEMPLATE__
#include "LinkListTemplate.h"
#include "FreeListBlock.h"


template<class M>
class CLinkListTemplateFreeList : public CLinkListTemplate<M>
{
protected:	
	CFreeListBlock	mcFreeList;  //Only Unknown types of freelists can be managed.

	void*	MemoryAllocate(int iMemSize);
	void	Free(void* pvMem);

public:
	void	Init(void);
	void	Init(int iChunkSize);
	void	Kill(void);


};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template <class M>
void* CLinkListTemplateFreeList<M>::MemoryAllocate(int iMemSize)
{
	return mcFreeList.Add();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CLinkListTemplateFreeList<M>::Free(void* pvMem)
{
	mcFreeList.Remove(pvMem);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CLinkListTemplateFreeList<M>::Init(void)
{
	mcFreeList.Init(8, sizeof(M));
	mcFreeList.SetAdditionalSize(sizeof(SDNode));
	CLinkListTemplate<M>::Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CLinkListTemplateFreeList<M>::Init(int iChunkSize)
{
	mcFreeList.Init(iChunkSize, sizeof(M));
	mcFreeList.SetAdditionalSize(sizeof(SDNode));
	CLinkListTemplate<M>::Init();
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CLinkListTemplateFreeList<M>::Kill(void)
{
	mcFreeList.Kill();
	mpsHead = NULL;
	mpsTail = NULL;
	miNumElements = 0;
}


#endif //__FREE_LINK_LIST_TEMPLATE__

