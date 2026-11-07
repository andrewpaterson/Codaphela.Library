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
#ifndef __POINTER_FIELD_H__
#define __POINTER_FIELD_H__
#include "Field.h"


class CPointer;
class CBaseObject;
class CPointerField : public CField
{
CONSTRUCTABLE(CPointerField)
protected:
	bool		mbDoNoFreePointedTo;

public:
	void		Init(ptrdiff_t iOffset, CClass* pcContainingClass, const char* szName);
	void		Kill(void) override;

	bool		IsPointer(void) override;
	size		GetSizeOf(void) override;
	size		GetNameOffset(void) override;
	CPointer*	GetPointer(CBaseObject* pcFieldContainer);
	void		DoNotFreePointedTo(bool bValue);
	bool		IsDoNotFreePointedTo(void);
};


#endif // __POINTER_FIELD_H__

