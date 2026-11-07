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
#ifndef __MAP_ENTRY_H__
#define __MAP_ENTRY_H__

#include "Object.h"
#include "Pointer.h"


class CMapEntry : public CObject
{
CONSTRUCTABLE(CMapEntry)
DESTRUCTABLE(CMapEntry)
protected:
	Ptr<>	mpKey;
	Ptr<>	mpValue;

public:
					CMapEntry(void);
					CMapEntry(const CMapEntry& cOther);
	void			operator = (const CMapEntry& pcPointer);

	void			Init(void);
	void			Init(CPointer& pKey, CPointer& pValue);
	void			Init(CBaseObject* pcKey, CBaseObject* pcValue);
	void			Class(void) override;
	void			Free(void) override;

	void			Clear(void);
	bool			Exists(void);
	CPointer		Key(void);
	CPointer		Value(void);
	CBaseObject*	KeyObject(void) const;
	CBaseObject*	ValueObject(void) const;
	bool			ValidatePointersEmbedded(void);
};


#endif // __MAP_ENTRY_H__

