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
#ifndef __WEAK_FROMS_H__
#define __WEAK_FROMS_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/HashMapTemplate.h"
#include "ObjectPointerArrays.h"


class CEmbeddedObject;


//What holds weak references (WeakReferencesPlan.txt): told to let go of the one at uiSlot when its object is freed, and to
//point it at another when its object is replaced (by a hollow when it's evicted, by the loaded object when that's loaded).
//A weak pointer has one slot (0), a weak collection one for each entry.
class CWeakHolder
{
public:
	virtual void				ClearWeak(size uiSlot) =0;  //Its object is being freed: NULL it, and do nothing else.
	virtual void				RemapWeak(size uiSlot, CEmbeddedObject* pcObject) =0;  //Its object is replaced by pcObject: point at it, and do nothing else.
	virtual CEmbeddedObject*	GetWeak(size uiSlot) =0;  //What it points at, for validation.
};


//A weak reference to an object: its holder and the slot in it.
struct SWeakFrom
{
	CWeakHolder*	pcHolder;
	size			uiSlot;
};


typedef CArrayTemplate<SWeakFrom>	CArrayWeakFrom;


//The weak froms of every weakly held object in a CObjects, kept apart from its heap and stack froms so no distance walk
//or from scan can see them: a weak reference can't keep anything.  Keyed by the object itself (an embedded object by
//itself, not by its container).  An object with none isn't in the map.
class CWeakFroms
{
protected:
	CHashMapTemplate<CEmbeddedObject*, CArrayWeakFrom*>	mcObjects;  //Each weakly held object to its weak froms.

public:
	void			Init(void);
	void			Kill(void);

	void			Add(CEmbeddedObject* pcObject, CWeakHolder* pcHolder, size uiSlot);
	bool			Remove(CEmbeddedObject* pcObject, CWeakHolder* pcHolder, size uiSlot);
	void			Clear(CEmbeddedObject* pcObject);  //Each holder's ClearWeak, then the object has none.
	void			Move(CEmbeddedObject* pcFrom, CEmbeddedObject* pcTo);  //Every weak from of pcFrom becomes pcTo's, its holder pointed at pcTo.

	size			NumWeakFroms(CEmbeddedObject* pcObject);
	SWeakFrom*		GetWeakFrom(CEmbeddedObject* pcObject, size uiIndex);
	size			NumObjects(void);
	void			GetObjects(CArrayEmbeddedObjectPtr* papcObjects);  //Every weakly held object.
	bool			Contains(CEmbeddedObject* pcObject, CWeakHolder* pcHolder, size uiSlot);
	void			ValidateConsistency(void);  //Every holder points back at its object, whose container has OBJECT_FLAGS_WEAKLY_HELD and isn't freed.

protected:
	CArrayWeakFrom*	Find(CEmbeddedObject* pcObject);
};


#endif // __WEAK_FROMS_H__

