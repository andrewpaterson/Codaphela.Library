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
#ifndef __FREE_OBJECTS_H__
#define __FREE_OBJECTS_H__
#include "Object.h"
#include "SetObject.h"


//Objects that must be freed explicitly: held from the root until they are removed.
class CFreeObjects : public CObject
{
CONSTRUCTABLE(CFreeObjects)
DESTRUCTABLE(CFreeObjects)
protected:
	Ptr<CSetObject>		mpFreeObjects;

public:
						void			Init(void);
						void			Free(void) override;
						void			Class(void) override;

	template<class M>	M*				Add(Ptr<M>& p);
						CBaseObject*	Add(CPointer &p);
						bool			Remove(CBaseObject* pcObject);

						size			NumObjects(void);

						CBaseObject*	UnsafeGetObject(size iIndex);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CFreeObjects::Add(Ptr<M>& p)
{
	return (M*)Add((CPointer&)p);
}


#endif // __FREE_OBJECTS_H__

