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
#ifndef __TREE_TEMPLATE_FREE_LIST_H__
#define __TREE_TEMPLATE_FREE_LIST_H__
#include "TreeTemplate.h"
#include "FreeList.h"


template <class M>
class CTreeTemplateFreeList : public CTreeTemplate<M>
{
protected:	
	CFreeList<M>		mcFreeList;

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
void* CTreeTemplateFreeList<M>::MemoryAllocate(int iMemSize)
{
	return mcFreeList.Add();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CTreeTemplateFreeList<M>::Free(void* pvMem)
{
	return mcFreeList.Remove(pvMem);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CTreeTemplateFreeList<M>::Init(void)
{
	mcFreeList.Init();
	mcFreeList.SetAdditionalSize(sizeof(SDNode));
	CTreeTemplate<M>::Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CTreeTemplateFreeList<M>::Init(int iChunkSize)
{
	mcFreeList.Init(iChunkSize);
	mcFreeList.SetAdditionalSize(sizeof(SDNode));
	CTreeTemplate<M>::Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CTreeTemplateFreeList<M>::Kill(void)
{
	mcFreeList.Kill();
	mpsRoot = NULL;
}


#endif //__TREE_TEMPLATE_FREE_LIST_H__

