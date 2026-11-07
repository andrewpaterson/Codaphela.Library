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
#ifndef __DEPENDENT_READ_OBJECTS_H__
#define __DEPENDENT_READ_OBJECTS_H__
#include "ObjectHeader.h"
#include "BaseObject.h"
#include "Objects.h"		//Not needed here, but the libraries and tests that read objects find CObjects through this.


//What an object reader asks while it reads: the objects a read object points to (and those that point to it), and
//where to read it into.
class CDependentReadObjects
{
public:
	virtual bool					AddDependent(CObjectIdentifier* pcHeader, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated, uint16 iNumEmbedded, uint16 iEmbeddedIndex) =0;
	virtual bool					AddReverseDependent(CObjectIdentifier* pcHeader, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingHeapFrom, uint16 iNumEmbedded, uint16 iEmbeddedIndex) =0;

	virtual CBaseObject*			AllocateForDeserialisation(CObjectHeader* pcHeader) =0;

protected:
			bool					PointTo(CBaseObject* pcObject, uint16 iEmbeddedIndex, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcContaining);
};


#endif // __DEPENDENT_READ_OBJECTS_H__

