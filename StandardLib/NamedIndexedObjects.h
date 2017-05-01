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
#ifndef __NAMED_INDEX_OBJECTS_H__
#define __NAMED_INDEX_OBJECTS_H__
#include "BaseLib/IndexTreeMemory.h"
#include "IndexedObjects.h"


// CIndexedObjects maps a string name to an OIndex and then an OIndex to a CBaseObject*.


class CNamedIndexedObjects
{
protected:
	CIndexTreeMemory 	mcNames;   //xxx - here, this should be CIndexTreeBlockDisk or something.
	CIndexedObjects		mcIndexedObjects;

public:
	void				Init(void);
	void				Kill(void);
	void				ReInit(void);
	CBaseObject*		Get(OIndex oi);
	CBaseObject*		Get(char* szName);
	BOOL				AddWithID(CBaseObject* pvObject, OIndex oi);
	BOOL				AddWithIDAndName(CBaseObject* pvObject, OIndex oi, char* szName);
	BOOL				RemoveIndex(OIndex oi);
	BOOL				RemoveName(char* szName);
	OIndex				NumIndexed(void);
	int					NumNames(void);

	CIndexedObjects*	GetObjects(void);
	CBaseObject*		StartIteration(SIndexesIterator* psIter);
	CBaseObject*		Iterate(SIndexesIterator* psIter);
};



#endif // __NAMED_INDEX_OBJECTS_H__

