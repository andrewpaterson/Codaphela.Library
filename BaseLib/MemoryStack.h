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
#ifndef __MEMORY_STACK_H__
#define __MEMORY_STACK_H__
#include "StackMark.h"


class CMemoryStack
{
protected:
	void*	mpvMemory;
	size	miMemorySize;
	size	miUsedMemory;
	size	miElements;

public:
	void		Init(size iSize);
	void*		Allocate(size iSize);  //Same as Init, but immediately allocates memory.
	void		Clear(void);
	void		Kill(void);

	void*		Add(size iSize);
	void		Remove(void);
	void		Remove(size iNumToRemove);  //This is the number of elements to remove, not the amount of memory to remove.

	SStackMark	Mark(void);
	void		Rollback(SStackMark* psMark);

	size		GetTotalMemory(void);
	size		GetUsedMemory(void);
	size		GetRemainingMemory(void);
	void*		GetData(void);
	size		NumElements(void);
};


#endif // __MEMORY_STACK_H__

