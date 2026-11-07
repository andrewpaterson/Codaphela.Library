/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __STACK_POINTERS_H__
#define __STACK_POINTERS_H__
#include "BaseLib/FreeList.h"
#include "StackPointer.h"


class CStackPointers
{
protected:
	CFreeList		mcFreeList;

public:
	void			Init(void);
	void			Kill(void);

	CStackPointer*	Add(CPointer* pcPointer);
	CStackPointer*	Add(CCollection* pcCollection);
	void			SetLastNext(CStackPointer* pcStackPointer, CStackPointer* pcFirst);

	void			RemoveAll(CStackPointer* pcFirst);
	CStackPointer* 	Remove(CStackPointer* pcFirst, CPointer* pcPointer, bool* pbRemoved);
	CStackPointer*	Remove(CStackPointer* pcFirst, CCollection* pcCollection, bool* pbRemoved);
	void			ClearAllPointers(void);

	size			NumElements(void);
	CStackPointer*	Get(size uiIndex);

	CStackPointer*	StartIteration(SFreeListIterator* psIter);
	CStackPointer*	Iterate(SFreeListIterator* psIter);
};


extern CStackPointers	gcStackPointers;


#endif // __STACK_POINTERS_H__

