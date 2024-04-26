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
#ifndef __INDEX_OBJECTS_H__
#define __INDEX_OBJECTS_H__
#include "BaseLib/IndexTreeMemory.h"
#include "BaseObject.h"


struct SIndexesIterator : public SIndexTreeMemoryIterator
{
};


// CIndexedObjects maps an OIndex to a CBaseObject*.
class CIndexedObjects
{
protected:
	CIndexTreeMemory	mcIndexes;

public:
	void			Init(void);
	void			Kill(void);
	CBaseObject*	Get(OIndex oi);
	bool			Has(OIndex oi);
	bool			Add(OIndex oi, CBaseObject* pvMemory);
	bool			Remove(OIndex oi);
	int				NumIndexed(void);

	OIndex			StartIteration(SIndexesIterator* psIterator);
	OIndex			Iterate(SIndexesIterator* psIterator);
};


#endif // __INDEX_OBJECTS_H__

