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
#ifndef __TREE_TEMPLATE_MEMORY_STACK_H__
#define __TREE_TEMPLATE_MEMORY_STACK_H__
#include "TreeBlockMemoryStack.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlockMemoryStack::MemoryAllocate(int iMemSize)
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
void CTreeBlockMemoryStack::Free(void* pvMem)
{
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTreeBlockMemoryStack::Init(void)
{
	mpcMemoryStack = NULL;
	__CTreeBlock::Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTreeBlockMemoryStack::Init(CMemoryStack* pcMemoryStack)
{
	mpcMemoryStack = pcMemoryStack;
	__CTreeBlock::Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTreeBlockMemoryStack::Kill(void)
{
	mpsRoot = NULL;
	miNumElements = 0;
	miLevel = 0;
}


#endif //__TREE_TEMPLATE_MEMORY_STACK_H__

