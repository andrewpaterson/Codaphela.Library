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
#ifndef __MEMORY_STACK_EXTENDEDED_H__
#define __MEMORY_STACK_EXTENDEDED_H__
#include "ArrayTemplate.h"
#include "MemoryStack.h"
#include "StackMark.h"


typedef CArrayTemplate<CMemoryStack> CArrayMemoryStacks;


class CMemoryStackExtended
{
protected:
	CArrayMemoryStacks	mcStacks;
	int					miChunkSize;
	int					miElements;

public:
	void			Init(int iChunkSize);
	void			Clear(void);
	void			Kill(void);

	void*			Add(int iSize);
	void			Remove(void);
	void			Remove(int iNumToRemove);  //This is the number of elements to remove, not the amount of memory to remove.
	void			Mark(CStackMarkExtended* psMark);
	void			Rollback(CStackMarkExtended* psMark);

	int				GetTotalMemory(void);
	int				GetUsedMemory(void);
	CMemoryStack*	GetStack(int iIndex);
	int				NumStacks(void);
	int				NumElements(void);

};


#endif // !__MEMORY_STACK_EXTENDEDED_H__

