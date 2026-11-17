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
#include "EmbeddedObject.h"
#include "BaseObject.h"
#include "Objects.h"
#include "WeakPointer.h"
#include "ObjectsLock.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CWeakPointer::CWeakPointer()
{
	mpcObject = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWeakPointer::ClearWeak(size uiSlot)
{
	mpcObject = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWeakPointer::RemapWeak(size uiSlot, CEmbeddedObject* pcObject)
{
	mpcObject = pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWeakPointer::PointTo(CEmbeddedObject* pcObject)
{
	OBJECTS_WRITE();

	//An object being freed has had its weak references NULLed already (a Free() reaching it for one), so one to it is NULL.
	if (pcObject && pcObject->IsBeingFreed())
	{
		pcObject = NULL;
	}
	if (mpcObject == pcObject)
	{
		return;
	}
	if (mpcObject)
	{
		mpcObject->GetObjectsOrDefault()->RemoveWeakFrom(mpcObject, this, 0);
	}
	mpcObject = pcObject;
	if (mpcObject)
	{
		mpcObject->GetObjectsOrDefault()->AddWeakFrom(mpcObject, this, 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWeakPointer::Release(void)
{
	PointTo(NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CWeakPointer::Object(void) { return mpcObject; }
CEmbeddedObject* CWeakPointer::GetWeak(size uiSlot) { return mpcObject; }
bool CWeakPointer::IsNull(void) const { return mpcObject == NULL; }
bool CWeakPointer::IsNotNull(void) const { return mpcObject != NULL; }
CWeakPointer* CWeakPointer::This(void) { return this; }

