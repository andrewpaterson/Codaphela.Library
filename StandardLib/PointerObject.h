/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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


#define LOG_POINTER_DEBUG()	LogPointerDebug(this, __ENGINE_PRETTY_FUNCTION__)


//This class is roughly a void* (or possibly an Object*).  It should probably have been called CObjectPointer but it's easier to start typing CPo...
//CPointer (and friends) are either declared on the stack or as a field on a CObject; nowhere else.  The only time embedding is NULL is when
//this pointer is declared on the stack.

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
						CPointer(CPointer& pcPointer);
						~CPointer();

	void				operator = (CEmbeddedObject* pcObject);
	void				operator = (CPointer& pcPointer);
	CEmbeddedObject*	operator -> ();
	CEmbeddedObject*	operator & ();
	bool				operator ! ();
	bool				operator == (CPointer& pcPointer);
	bool				operator != (CPointer& pcPointer);
	bool				operator == (void* pcPointer);
	bool				operator != (void* pcPointer);

	void 				SetEmbedding(CObject* pcEmbedding);

	bool				IsNotNull(void);
	bool				IsNull(void);

	CPointer*			This(void);
	CObject*			Embedding(void);
	CEmbeddedObject*	Object(void);
	CEmbeddedObject**	ObjectPtr(void);
	void				UnsafePointTo(CEmbeddedObject* pcNewObject);
	CBaseObject*		BaseObject(void);
	int					MorphInto(CEmbeddedObject* pcOld);

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
	bool				IsEmbeddingAllocatedInObjects(void);
	void				Kill(void);
	void				AssignObject(CEmbeddedObject* pcObject);
	CEmbeddedObject*	Dereference(void);
	CEmbeddedObject*	DereferenceArrow(void);

	void				PointTo(CEmbeddedObject* pcObject, bool bKillIfNoRoot);
	void				AddHeapFrom(CBaseObject* pcFrom);

	CEmbeddedObject*	Return(void);
	void				UnsafeClearObject(void);

	void				DumpFroms(void);
	void				DumpPointerTos(void);
	void				Dump(void);
};


void LogPointerDebug(CPointer* pvThis, char* szMethod);


#endif // __POINTER_OBJECT_H__

