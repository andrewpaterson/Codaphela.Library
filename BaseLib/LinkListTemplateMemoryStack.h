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
#ifndef __LINK_LIST_TEMPLATE_MEMORY_STACK__
#define __LINK_LIST_TEMPLATE_MEMORY_STACK__
#include "LinkListTemplate.h"
#include "MemoryStack.h"


template<class M>
class CLinkListTemplateMemoryStack : public CLinkListTemplate<M>
{
protected:	
	CMemoryStack*	mpcMemoryStack;

	void*	MemoryAllocate(int iMemSize);
	void	Free(void* pvMem);

public:
	void	Init(void);
	void	Init(CMemoryStack* pcMemoryStack);
	void	Kill(void);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template <class M>
void* CLinkListTemplateMemoryStack<M>::MalMemoryAllocateloc(int iMemSize)
{
	if (mpcMemoryStack)
	{
		return mpcMemoryStack->Add(iMemSize);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CLinkListTemplateMemoryStack<M>::Free(void* pvMem)
{
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CLinkListTemplateMemoryStack<M>::Init(void)
{
	mpcMemoryStack = NULL;
	CLinkListTemplate<M>::Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CLinkListTemplateMemoryStack<M>::Init(CMemoryStack* pcMemoryStack)
{
	mpcMemoryStack = pcMemoryStack;
	CLinkListTemplate<M>::Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CLinkListTemplateMemoryStack<M>::Kill(void)
{
	mpsHead = NULL;
	mpsTail = NULL;
	miNumElements = 0;
}


#endif //__LINK_LIST_TEMPLATE_MEMORY_STACK__

