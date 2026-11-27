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
#ifndef __ARRAY_WEAK_OBJECT_H__
#define __ARRAY_WEAK_OBJECT_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "Object.h"
#include "WeakFroms.h"


//Objects held weakly (WeakReferencesPlan.txt): an entry doesn't keep its object, and is NULL once the object is freed.
//Embedded in its holder by M_Embedded, which Inits it.  An entry is never taken out while objects are being freed (a free
//only NULLs it), so nothing iterating sees the array change size then; Remove and Compact take entries out, moving the last
//into the gap, so the order isn't kept.  Get is a Ptr, which holds the object while it's used.  Nothing is saved: it loads
//empty, as a weak pointer loads NULL.  Its entries come off their objects' weak froms when it's freed.
//Design page: Docs/libraries/standardlib/weak-references.md
class CArrayWeakObject : public CObject, public CWeakHolder
{
CONSTRUCTABLE(CArrayWeakObject)
DESTRUCTABLE(CArrayWeakObject)
protected:
	CArrayTemplatePtr<CEmbeddedObject>	mapcObjects;

public:
	void		Init(void);
	void		Class(void) override;
	void		Free(void) override;
	bool		Save(CObjectWriter* pcFile) override;
	bool		Load(CObjectReader* pcFile) override;

	void				ClearWeak(size uiSlot) override;
	void				RemapWeak(size uiSlot, CEmbeddedObject* pcObject) override;
	CEmbeddedObject*	GetWeak(size uiSlot) override;

	size		Add(CEmbeddedObject* pcObject);  //Its index.  NULL, or an object being freed, isn't added.
	size		Add(CPointer& pObject);
	bool		Remove(CEmbeddedObject* pcObject);  //Its first entry.
	void		RemoveAt(size uiIndex);
	void		Compact(void);  //The NULLs taken out.
	void		RemoveAll(void);

	size		NumElements(void);  //NULLs too.
	Ptr<>		Get(size uiIndex);  //NULL where its object has been freed.
	bool		Contains(CEmbeddedObject* pcObject);

protected:
	void		Move(size uiFrom, size uiTo);
};


#endif // __ARRAY_WEAK_OBJECT_H__

