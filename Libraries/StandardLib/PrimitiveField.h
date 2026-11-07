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
#ifndef __PRIMITIVE_FIELD_H__
#define __PRIMITIVE_FIELD_H__
#include "Field.h"


class CPrimitiveObject;
class CBaseObject;
class CPrimitiveField : public CField
{
CONSTRUCTABLE(CPrimitiveField)
protected:
	SDataIO*	mpcIO;
	size		muiValueOffset;

public:
	void				Init(CClass* pcFieldClass, ptrdiff_t iOffset, ptrdiff_t iValueOffset, CClass* pcContainingClass, SDataIO* pcIO, const char* szName);
	void				Kill(void) override;

	bool				IsPrimitive(void) override;
	size				GetSizeOf(void) override;
	size				GetNameOffset(void) override;
	SDataIO*			GetDataIO(void) override;
	CPrimitiveObject*	GetPrimitiveObject(CBaseObject* pcFieldContainer);
	void*				GetValue(CBaseObject* pcFieldContainer);
};


#endif // __PRIMITIVE_FIELD_H__

