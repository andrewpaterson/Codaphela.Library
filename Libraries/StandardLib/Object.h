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
#ifndef __OBJECT_H__
#define __OBJECT_H__
#include "Integer.h"
#include "Float.h"
#include "Character.h"
#include "PrimitiveVoid.h"
#include "ClassDefines.h"
#include "BaseObject.h"


/* Implement these on anything extending CObject
	void	Init(void);
	void	Class(void) override;
	void 	Free(void) override;

	bool	Save(CObjectWriter* pcFile) override;
	bool	Load(CObjectReader* pcFile) override;
*/


//Object is abstract (due to Class() and Free) and should remaain that way.
class CObject : public CBaseObject
{
template<class M>
friend class Ptr;
friend class CPointer;
friend class CObjects;
friend class CExternalObjectDeserialiser;

CONSTRUCTABLE(CObject)
protected:
	//The pointers are the class's pointer fields (NumPointerFields, GetPointerField), so an object on the stack must be
	//killed (or go out of scope) before ObjectsKill frees the classes.  The embedded objects are kept
	//flat: GetEmbeddedObject's index (the one objects are saved by) reaches any depth in one step.
	CArrayTemplateEmbeddedPtr<CBaseObject, 3>		mapEmbedded;  //Every object embedded in this, at any depth, in GetEmbeddedObject's order (index + 1): each followed by those embedded in it.
#if defined(LINUX_GNU) && !defined(BUILD_64)
	//CObject's fields end 4 bytes short of its 8 byte alignment.  g++ -m32 puts a subclass's first field in those 4
	//bytes; MSVC never puts one in a base's tail padding.  Filled, so both lay subclasses out alike (TestClass checks).
	uint32											muiTailPadding;
#endif

public:
						CObject();
						~CObject();

	void				Allocate(CObjects* pcObjects) override;
	void				Kill(void) final override;
	bool				IsCollection(void) override;
	bool				IsObject(void) override;
	void				GetPointerFieldsTo(CEmbeddedObject* pcTo, CArrayHeapFromPointer* pacPointers) override;

	size				GetEmbeddedIndex(CEmbeddedObject* pcEmbedded) override;
	size				CountNumEmbedded(void);
	CEmbeddedObject*	GetEmbeddedObject(size iIndex) override;
	CBaseObject*		Dehollow(void) override;
	size				NumPointerTos(void) override;
	size				BaseNumPointerTos(void) override;
	void				GetPointerTos(CArrayEmbeddedObjectPtr* papcTos) override;
	void				BaseGetPointerTos(CArrayEmbeddedObjectPtr* papcTos) override;

	size				GetFieldPointerToIndex(CPointer* pcFieldPointer);
	size				GetNumFieldPointerTos(void);
	CPointer*			GetFieldPointerTo(size iIndex);


	void				ValidateConsistency(void) override;

protected:
	void				EmbedFields(void) override;
	void				FreeInternal(void) override;
	void				DoneKill(bool bAllocatedInObjects) override;
	void				FreePointers(void) override;
	void				RemovePointerTo(CEmbeddedObject* pcTo) override;
	bool				ClearPointerTos(bool bFree) override;
	void				RemoveAllHeapFroms(void) override;
	void				RemoveAllStackFroms(void) override;
	size				RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew) override;
	CBaseObject*		GetContained(size uiIndex);
	size				NumPointerFields(void);
	CPointer*			GetPointerField(size uiIndex);
	CBaseObject*		IterateDirectlyEmbedded(size* puiIndex);
	void				ValidateEmbeddedConsistency(void) override;
	void				EmbedPointerFields(void);
	void				EmbedEmbeddedObjectFields(void);
	void				ClassNotImplemented(void);
};


#endif // __OBJECT_H__

