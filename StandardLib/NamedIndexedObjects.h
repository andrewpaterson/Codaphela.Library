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
	CIndexTreeMemory 	mcMemoryNames;
	CIndexedObjects		mcMemoryIndexedObjects;

public:
	void				Init(void);
	void				Kill(void);
	void				ReInit(void);
	CBaseObject*		Get(OIndex oi);
	CBaseObject*		Get(char* szName);
	CBaseObject*		Get(char* szObjectName, OIndex oi);
	bool				Has(OIndex oi);
	bool				Has(char* szName);
	bool				AddIntoMemoryWithIndex(CBaseObject* pvObject);
	bool				AddIntoMemoryWithNameAndIndex(CBaseObject* pvObject);
	bool				RemoveIndex(OIndex oi);
	bool				RemoveName(char* szName);
	size				NumIndexed(void);
	size				NumNames(void);

	CIndexedObjects*	GetObjects(void);
	CIndexTreeMemory*	GetNames(void);
	CBaseObject*		StartIteration(SIndexesIterator* psIter);
	CBaseObject*		Iterate(SIndexesIterator* psIter);

	bool				ValidateNoDirty(void);
	bool				FreeObjects(void);

	void				Dump(void);
};



#endif // __NAMED_INDEX_OBJECTS_H__

