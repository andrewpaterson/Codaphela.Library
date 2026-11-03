/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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


typedef CArrayTemplate<CMemoryStack> CArrayMemoryStack;


//Memory added a piece at a time in chunks, and given back all at once (Clear, Recycle or Kill) or from the end (Remove,
//Rollback).  Recycle keeps the chunks for the Adds after (spares), so memory used again and again isn't allocated
//again.
class CMemoryStackExtended
{
protected:
	CArrayMemoryStack	mcStacks;
	CArrayMemoryStack	mcSpares;		//Empty chunks (of muiChunkSize) kept by Recycle.
	lsize				muiChunkSize;
	size				muiElements;

public:
	void			Init(lsize iChunkSize);
	void			Clear(void);
	void			Recycle(lsize uiMaxKept);
	void			Kill(void);

	void*			Add(lsize iSize);
	void			Remove(void);
	void			Remove(size iNumToRemove);  //This is the number of elements to remove, not the amount of memory to remove.
	void			Mark(CStackMarkExtended* psMark);
	void			Rollback(CStackMarkExtended* psMark);

	lsize			GetTotalMemory(void);
	lsize			GetUsedMemory(void);
	CMemoryStack*	GetStack(size iIndex);
	size			NumStacks(void);
	size			NumElements(void);
	size			NumSpares(void);

protected:
	CMemoryStack*	AddChunk(void);

};


#endif // __MEMORY_STACK_EXTENDEDED_H__

