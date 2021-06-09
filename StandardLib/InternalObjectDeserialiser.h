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
#ifndef __INTERNAL_OBJECT_DESERIALISER__
#define __INTERNAL_OBJECT_DESERIALISER__
#include "CoreLib/DataConnection.h"
#include "DependentObjectAdder.h"
#include "DependentReadObjects.h"


//The Interal Object Deserialiser only deserialises objects out of the CObjects database.
//  The index in the header is assumed to match the database index key.
//  The name in the header is assumed to match the database name key.


class CInternalObjectDeserialiser : public CDependentObjectAdder
{
protected:
	CDataConnection*		mpcDataConnection;
	CNamedIndexedObjects*	mpcMemory;
	CObjects*				mpcObjects;

	CDependentReadObjects	mcDependentObjects;  //CDependentObjectAdder points to this CDependentReadObjects.

public:
	void			Init(CObjects* pcObjects, CDataConnection* pcDataConnection, CNamedIndexedObjects* pcMemory);
	void			Kill(void);

	CBaseObject*	Read(OIndex oi);
	CBaseObject*	Read(char* szObjectName);

	CBaseObject*	AllocateForDeserialisation(CObjectHeader* pcHeader);

protected:
	CBaseObject*	ReadSerialised(CSerialisedObject* pcSerialised);
	BOOL			AddContainingPointersAndCreateHollowObjects(void);
	BOOL			AddContainingPointersAndCreateHollowObject(CDependentReadPointer* pcDependentReadPointer);
};


#endif // __INTERNAL_OBJECT_DESERIALISER__

