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
#ifndef __INDEXED_DATA_OBJECT_DESERIALISER__
#define __INDEXED_DATA_OBJECT_DESERIALISER__
#include "DependentObjectAdder.h"
#include "ObjectAllocator.h"
#include "DependentReadObjects.h"


class CIndexedDataObjectDeserialiser : public CDependentObjectAdder
{
protected:
	CObjectAllocator*				mpcAllocator;
	CNamedIndexedData*				mpcDatabase;
	CNamedIndexedObjects*			mpcMemory;

	CDependentReadObjects			mcDependentObjects;  //CDependentObjectAdder points to this CDependentReadObjects.

public:
	void			Init(CObjectAllocator* pcAllocator, CNamedIndexedData* pcDatabase, CNamedIndexedObjects* pcMemory);
	void			Kill(void);

	CPointerObject	Read(OIndex oi);
	CPointerObject	Read(char* szObjectName);

	CPointerObject	AllocateObject(CObjectHeader* pcHeader);

protected:
	CPointerObject	Read(CSerialisedObject* pcSerialised);
	void			UpdateDependentPointersAndCreateHollowObjects(CBaseObject* pcReadObject);
	void			FixPointerOrCreateHollowObject(CBaseObject* pcReadObject, CDependentReadPointer* pcDependentReadPointer);
};


#endif // __INDEXED_DATA_OBJECT_DESERIALISER__

