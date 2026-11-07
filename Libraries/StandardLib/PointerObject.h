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


//A pointer to an object that the object knows about: assigning one adds it to the object's froms and moving it away removes it.
//CPointer (and friends) are a field of a CObject (declared in its Class()) or, briefly, a local variable on the stack; nowhere else.
//The root (ORoot()) is the source of memory management: objects are held by objects reachable from it, through a CObject's pointer fields
//and collections (CArrayObject, CSetObject, CMapObject, CIndexObject).  An object with no more natural holder (a dialog, say) is OMalloc'd
//and added to the root, and removed from the root when it is time to destroy it.  A class that holds a CPointer, an object or a collection
//must itself be a CObject, even when it is on the stack.  Stack pointers are for using objects, not keeping them: they shouldn't live long
//(nor should CObjects on the stack, which are only brief helpers or factories).
//The only time the embedding is NULL is when this pointer is on the stack.  A stack pointer is known to the object it points to by its
//address; ValidateFroms reports one that no longer points to it (a pointer kept somewhere it doesn't belong, such as an element of a
//CArrayTemplate that grew, and so was moved).

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

