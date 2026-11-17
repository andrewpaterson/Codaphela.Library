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
#ifndef __UNMANAGED_FIELD_H__
#define __UNMANAGED_FIELD_H__
#include "Field.h"


class CBaseObject;
class CUnmanagedField : public CField
{
CONSTRUCTABLE(CUnmanagedField)
protected:
	size			muiLength;
	size			muiElementSize;
	EPrimitiveType	meType;

public:
	void		Init(EPrimitiveType eType, ptrdiff_t iOffset, CClass* pcContainingClass, const char* szName);
	void		Init(EPrimitiveType eType, ptrdiff_t iOffset, CClass* pcContainingClass, size uiLength, const char* szName);
	void		Init(EPrimitiveType eType, ptrdiff_t iOffset, CClass* pcContainingClass, size uiElementSize, size uiLength, const char* szName);
	void		Kill(void) override;

	bool		IsArray(void) const override;
	size		GetLength(void) const;
	bool		IsUnmanaged(void) const override;
	void*		GetData(CBaseObject* pcFieldContainer);
	size		GetSizeOf(void) const override;
	EPrimitiveType	GetType(void) const;
	size		GetNameOffset(void) const override;
};


#endif // __UNMANAGED_FIELD_H__

