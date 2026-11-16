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
#ifndef __WEAK_POINTER_FIELD_H__
#define __WEAK_POINTER_FIELD_H__
#include "Field.h"


class CWeakPointer;
class CBaseObject;


//A weak pointer field (W_Pointer): the object system clears it when its holder is freed, and doesn't save it.
class CWeakPointerField : public CField
{
CONSTRUCTABLE(CWeakPointerField)
public:
	void			Init(ptrdiff_t iOffset, CClass* pcContainingClass, const char* szName);
	size			GetSizeOf(void) override;
	CWeakPointer*	GetWeakPointer(CBaseObject* pcFieldContainer);

protected:
	size			GetNameOffset(void) override;
};


#endif // __WEAK_POINTER_FIELD_H__

