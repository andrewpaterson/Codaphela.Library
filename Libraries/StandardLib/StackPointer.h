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
#ifndef __STACK_POINTER_H__
#define __STACK_POINTER_H__
#include "BaseLib/Define.h"
#include "BaseLib/FreeList.h"


enum EStackPointerType
{
	SPT_Unknown,
	SPT_Pointer,
	SPT_Collection
};


class CPointer;
class CCollection;
class CEmbeddedObject;
struct SStackPointer
{
	EStackPointerType	eType;
	union
	{
		CPointer*		pcPointer;  //Pointer on the stack
		CCollection*	pcCollection;
		void*			pvPointer;  //Either of the above.  Used to compare them.
	} u;
};


class CStackPointer
{
protected:
	CStackPointer*	mpcNext;  //The object pointer to by the stack pointer points to 'this' stack pointer and additional stack pointers are linked through next.
	SStackPointer	msPointer;

public:
	void				Init(void);
	void				Init(CPointer* pcPointer);
	void				Init(CCollection* pcCollection);
	void				Kill(void);

	void				SetNext(CStackPointer* pcNext);
	int					NumPointers(void);
	CStackPointer*		FindLast(void);
	CStackPointer*		GetNext(void);
	
	CStackPointer*		Remove(CPointer* pcPointer, CFreeList* pcFreeList, bool* pbRemoved);
	CStackPointer*		Remove(CCollection* pcCollection, CFreeList* pcFreeList, bool* pbRemoved);
	void				RemoveAll(CFreeList* pcFreeList);
	void				ClearPointer(void);
	CStackPointer*		ClearPointerGetNext(CEmbeddedObject* pcPointedTo);

	EStackPointerType	GetType(void);
	SStackPointer*		Get(void);

protected:
	CStackPointer*		Remove(EStackPointerType eType, void* pvPointer, CFreeList* pcFreeList, bool* pbRemoved);
};


#endif // __STACK_POINTER_H__

