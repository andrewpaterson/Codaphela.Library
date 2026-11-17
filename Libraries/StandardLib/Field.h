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
#ifndef __FIELD_H__
#define __FIELD_H__
#include "BaseLib/DataTypes.h"
#include "BaseLib/Constructable.h"
#include "BaseLib/Killable.h"
#include "BaseLib/DatasIO.h"
#include "Unknown.h"


class CClass;
class CClasses;
class CField : public CUnknown
{
CONSTRUCTABLE(CField)
protected:
	size		muiOffset;
	CClass*		mpcFieldClass;
	CClass*		mpcContainingClass;

public:
			void		Init(CClass* pcFieldClass, size uiOffset, CClass* pcContainingClass, const char* szName);
			void		Kill(void) override;

			CClasses*	GetClasses(void) const;
			char*		GetName(void);
			CClass*		GetClass(void) const;
	virtual	size		GetSizeOf(void) const =0;

	virtual	bool		IsArray(void) const;
			size		GetOffset(void) const;
	virtual	bool		IsPointer(void) const;
	virtual	bool		IsPrimitive(void) const;
	virtual	bool		IsUnmanaged(void) const;
	virtual	bool		IsEmbeddedObject(void) const;
	virtual	SDataIO*	GetDataIO(void) const;

protected:
	virtual	size		GetNameOffset(void) const =0;
};


#endif // __FIELD_H__

