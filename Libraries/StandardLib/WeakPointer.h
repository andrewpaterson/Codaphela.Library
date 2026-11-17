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
#ifndef __WEAK_POINTER_H__
#define __WEAK_POINTER_H__
#include "Pointer.h"
#include "WeakFroms.h"


//A pointer to an object that doesn't keep it (WeakReferencesPlan.txt).  It's a weak from of its object, not a heap from,
//so no distance walk sees it: the object is freed when nothing strong reaches it, and then this is NULL (before any Free()
//of the objects freed with it runs).  Only a CObject field, declared in Class() by W_Pointer, so that when its holder is
//freed it comes off its object's weak froms.  It isn't saved: it loads as NULL.
//
//It's used through Get, a Ptr that holds the object for as long as it's used: a weak pointer has no operator ->.
class CWeakPointer : public CWeakHolder
{
protected:
	CEmbeddedObject*	mpcObject;

public:
						CWeakPointer();

	void				ClearWeak(size uiSlot) override;
	void				RemapWeak(size uiSlot, CEmbeddedObject* pcObject) override;
	CEmbeddedObject*	GetWeak(size uiSlot) override;

	void				PointTo(CEmbeddedObject* pcObject);  //Off its old object's weak froms, onto the new one's (NULL for an object being freed).
	void				PointToWeak(CWeakPointer* pcWeak);  //At what another weak pointer points at.
	void				HoldObject(CPointer& pHeld);  //pHeld, which is NULL, holds the object: NULL still if it has been freed.
	void				Release(void);  //Off its object's weak froms, and NULL: its holder is being freed.
	CEmbeddedObject*	Object(void);
	bool				IsNull(void) const;
	bool				IsNotNull(void) const;
	CWeakPointer*		This(void);

protected:
	void				PointToInside(CEmbeddedObject* pcObject);

private:
						CWeakPointer(const CWeakPointer& cPointer);  //A weak pointer is a field: it isn't copied.
};


template<class M = CEmbeddedObject>
class Weak : public CWeakPointer
{
public:
	Weak&	operator = (const CPointer& pPointer);
	Weak&	operator = (CEmbeddedObject* pcObject);
	Weak&	operator = (Weak& cWeak);

	Ptr<M>	Get(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Weak<M>& Weak<M>::operator = (const CPointer& pPointer)
{
	PointTo(pPointer.Object());
	return *this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Weak<M>& Weak<M>::operator = (CEmbeddedObject* pcObject)
{
	PointTo(pcObject);
	return *this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Weak<M>& Weak<M>::operator = (Weak& cWeak)
{
	PointToWeak(&cWeak);
	return *this;
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer that holds the object for as long as it is used, NULL if the object has been freed.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> Weak<M>::Get(void)
{
	Ptr<M>	pHeld;

	HoldObject(pHeld);
	return pHeld;
}


#endif // __WEAK_POINTER_H__

