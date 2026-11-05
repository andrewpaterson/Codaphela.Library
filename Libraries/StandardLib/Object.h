#ifndef __OBJECT_H__
#define __OBJECT_H__
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
	//Both pointers and embedded objects are now explicitly setup in the class.  Remove these fields.
	CArrayTemplateEmbeddedPtr<CPointer, 5>		mapPointers;  //Pointers in this object.  
	CArrayTemplateEmbeddedPtr<CBaseObject, 3>		mapEmbedded;  //Objects embedded in this object

public:
						CObject();
						~CObject();

	void				Allocate(CObjects* pcObjects);
	void				Kill(void) final override;
	bool				IsCollection(void) override;
	bool				IsObject(void) override;
	void				GetPointerFieldsTo(CEmbeddedObject* pcTo, CArrayHeapFromPointer* pacPointers) override;

	bool				IsDirty(void);
	size				GetEmbeddedIndex(CEmbeddedObject* pcEmbedded);
	size				NumEmbedded(void);
	size				CountNumEmbedded(void);
	CEmbeddedObject*	GetEmbeddedObject(size iIndex);
	CBaseObject*		Dehollow(void);
	size				NumHeapFroms(void);
	size				NumStackFroms(void);
	void				SetFlag(uint16 iFlag, uint16 iFlagValue);
	void				GetStackFroms(CArrayStackPointerE32* papcFroms);
	size				NumPointerTos(void);
	size				BaseNumPointerTos(void);
	void				GetPointerTos(CArrayEmbeddedObjectPtr* papcTos);
	void				BaseGetPointerTos(CArrayEmbeddedObjectPtr* papcTos);

	size				GetFieldPointerToIndex(CPointer* pcFieldPointer);
	size				GetNumFieldPointerTos(void);
	CPointer*			GetFieldPointerTo(size iIndex);

	void				CopyFields(CEmbeddedObject* pcOther) override;

	void				ValidateConsistency(void);

protected:
	void				EmbedFields(void) override;
	void				FreeInternal(void) override;
	void				DoneKill(bool bAllocatedInObjects) override;
	void				FreePointers(void) override;
	void				RemovePointerTo(CEmbeddedObject* pcTo);
	bool				ClearPointerTos(bool bFree) override;
	void				RemoveAllHeapFroms(void) override;
	void				RemoveAllStackFroms(void) override;
	size				RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew) override;
	bool				SetDistToRoot(int iDistToRoot) override;
	bool				RecurseGetEmbeddedIndex(CEmbeddedObject* pcTest, size* piIndex);
	CEmbeddedObject*	RecurseGetEmbeddedObject(size iIndex, size* iCount);
	bool				RecurseGetFieldPointerToIndex(CPointer* pcTest, size* piIndex);
	CPointer*			RecurseGetFieldPointerTo(size iIndex, size* piCount);
	void				ValidateEmbeddedConsistency(void);
	void				EmbedPointerFields(void);
	void				EmbedEmbeddedObjectFields(void);
	void				ClassNotImplemented(void);
};


#endif // __OBJECT_H__

