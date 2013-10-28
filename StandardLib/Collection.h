/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __COLLECTION_H__
#define __COLLECTION_H__
#include "BaseObject.h"
#include "StackPointers.h"


//Note: This class has no fields.
class CObjects;
class CCollection : public CBaseObject
{
BASE_FUNCTIONS(CCollection);
public:
			void			Kill(void);
			BOOL			IsCollection(void);
			BOOL			IsObject(void);
	virtual void			RemoveAll(void) =0;
			CBaseObject*	Dehollow(void);
};


#endif // __COLLECTION_H__

