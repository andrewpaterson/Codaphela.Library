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
#ifndef __SET_OBJECT_H__
#define __SET_OBJECT_H__
#include "SetUnknown.h"
#include "ArrayCommonObject.h"
#include "Pointer.h"


class CSetObject : public CArrayCommonObject
{
CONSTRUCTABLE(CSetObject)
DESTRUCTABLE(CSetObject)
public:
						void			Init(bool bSortPointers = true);

						CPointer		UnsafeGet(size iIndex);
						CBaseObject*	UnsafeGetObject(size iIndex);

						//Do not Add, Remove or call Contains while iterating over a sorted set.  They can remove the NULLs left by RemoveDuringIteration which moves the Objects not yet iterated.
						void			RemoveDuringIteration(SSetIterator* psIter);

						void			TouchAll(void) override;

						void			Dump(void) override;
						void			PrintSlots(CChars* psz);
};


#endif // __SET_OBJECT_H__

