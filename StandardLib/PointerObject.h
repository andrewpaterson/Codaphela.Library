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
#include "EmbeddedObject.h"
#include "CoreLib/IndexedGeneral.h"


//This class is roughly a void* (or possibly an Object*).  It should probably have been called CObjectPointer but it's easier to start typing CPo...
//CPointer (and friends) are either declared on the stack or as a field on a CObject; nowhere else.  The only time embedding is NULL is when
//this pointer is declared on the stack.

class CEmbeddedObject;
class CObject;
class CObjectDeserialiser;
class CPointer
{
template<class M> friend class Ptr;
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
	BOOL				operator ! ();

	void 				SetEmbedding(CObject* pcEmbedding);

	BOOL				IsNotNull(void);
	BOOL				IsNull(void);

	CPointer*			This(void);
	CObject*			Embedding(void);
	CEmbeddedObject*	Object(void);
	CEmbeddedObject**	ObjectPtr(void);
	void				UnsafePointTo(CEmbeddedObject* pcNewObject);
	CBaseObject*		BaseObject(void);
	int					RemapFrom(CEmbeddedObject* pcOld);
	int					RemapEmbeddedFrom(CEmbeddedObject* pcNew, CEmbeddedObject* pcOld);

	BOOL				IsHollow(void);
	BOOL				Load(CObjectDeserialiser* pcFile);
	int					DistToRoot(void);
	OIndex				GetIndex(void);
	char*				GetName(void);
	BOOL				IsNamed(void);
	char*				ClassName(void);
	BOOL				IsDirty(void);
	void				Kill(void);
	void				ClearIndex(void);
	void				AssignObject(CEmbeddedObject* pcObject);
	CEmbeddedObject*	Dereference(void);

	void				PointTo(CEmbeddedObject* pcObject);
	void				Construct(CPointer& cPointer);
	void				AddFrom(CBaseObject* pcFrom);

	void				ClearObject(void);
};


#endif // __POINTER_OBJECT_H__

