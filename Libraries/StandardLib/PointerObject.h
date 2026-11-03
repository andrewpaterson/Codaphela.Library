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
#ifndef __POINTER_OBJECT_H__
#define __POINTER_OBJECT_H__
#include "BaseLib/Logger.h"
#include "BaseLib/IndexedGeneral.h"
#include "EmbeddedObject.h"


extern bool gbLogPointer;


#if defined(DEBUG_POINTER) && defined(_DEBUG)
#define LOG_POINTER_DEBUG()  LogPointerDebug(this, __ENGINE_PRETTY_FUNCTION__)
#else
#define LOG_POINTER_DEBUG()
#endif


//This class is roughly a void* (or possibly an Object*).  It should probably have been called CObjectPointer but it's easier to start typing CPo...
//CPointer (and friends) are either a field of a CObject (declared in its Class()) or a "stack" pointer: anything else, with a NULL embedding.
//A stack pointer is known to the object it points to by its address, so it must never move: on the stack, in a CUnknown or a NewMalloc'd
//class, or in a plain class or struct that is itself in one of those.  Never in an array that grows by copying its elements
//(CArrayTemplate) or anything else that is memcpy'd.  Objects' ValidateFroms reports a stack pointer that has moved.

class CEmbeddedObject;
class CObject;
class CObjectReader;

class CPointer
{
template<class M> 
friend class Ptr;
protected:
	CEmbeddedObject*	mpcObject;
	CObject*			mpcEmbedding;  //Collections do not embed pointer objects.  They manage their own pointers.

public:
						CPointer();
						CPointer(CEmbeddedObject* pcObject);
						CPointer(CPointer& pPointer);
						CPointer(const CPointer& pPointer);
						~CPointer();

	void				operator = (CEmbeddedObject* pcObject);
	void				operator = (CPointer& pPointer);
	void				operator = (const CPointer& pPointer);
	CEmbeddedObject*	operator -> ();
	CEmbeddedObject*	operator & ();
	bool				operator ! ();
	bool				operator == (const CPointer& pPointer);
	bool				operator != (const CPointer& pPointer);
	bool				operator == (void* pcPointer);
	bool				operator != (void* pcPointer);

	void 				SetEmbedding(CObject* pcEmbedding);

	bool				IsNotNull(void) const;
	bool				IsNull(void) const;
	bool				IsNotEqual(CPointer& pPointer);
	bool				IsEqual(CPointer& pPointer);
	bool				IsEqual(CEmbeddedObject* pcObject);

	CPointer*			This(void);
	CObject*			Embedding(void);
	CEmbeddedObject*	Object(void) const;
	CEmbeddedObject**	ObjectPtr(void);
	void				UnsafePointTo(CEmbeddedObject* pcNewObject);
	CBaseObject*		BaseObject(void);
	size				MorphInto(CEmbeddedObject* pcNew);

	bool				IsHollow(void);
	bool				Load(CObjectReader* pcFile);
	int					GetDistToRoot(void);
	int					GetDistToStack(void);
	OIndex				GetIndex(void);
	char*				GetName(void);
	bool				IsNamed(void);
	const char*			ClassName(void);
	CClass*				GetClass(void);
	bool				IsDirty(void);
	void				LogicalDirty(void);
	bool				IsEmbeddingAllocatedInObjects(void);
	void				Kill(void);
	void				AssignObject(CEmbeddedObject* pcObject);
	CEmbeddedObject*	Dereference(void);
	CEmbeddedObject*	DereferenceArrow(void);

	void				AddHeapFrom(CBaseObject* pcFrom);
	void				PointToNull(void);

	CEmbeddedObject*	Return(void);
	void				UnsafeClearObject(void);

	size				NumStackFroms(void);
	size				NumHeapFroms(void);
	size				NumEmbedded(void);
	size				NumPointerTos(void);

	void				DumpFroms(void);
	void				DumpPointerTos(void);
	void				Dump(void);

protected:
	void				PointTo(CEmbeddedObject* pcObject);
	bool				IsEmbeddingHeld(void);
};


void LogPointerDebug(CPointer* pvThis, const char* szMethod);


#endif // __POINTER_OBJECT_H__

