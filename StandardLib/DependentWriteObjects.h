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
#ifndef __DEPENDENT_WRITE_OBJECTS_H__
#define __DEPENDENT_WRITE_OBJECTS_H__
#include "DependentWriteObject.h"


class CDependentWriteObjects
{
protected:
	CArrayDependentWriteObject	mcObjects;
	int							miGetIndex;

public:
	void			Init(void);
	void			Kill(void);
	void			Add(CBaseObject* pcObject);
	CBaseObject*	GetUnwritten(void);
	void			Mark(CBaseObject* pcObject);
};


#endif // __DEPENDENT_WRITE_OBJECTS_H__

