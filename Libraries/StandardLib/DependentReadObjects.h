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
#include "BaseLib/Logger.h"
#include "ObjectHeader.h"
#include "BaseObject.h"
#include "Pointer.h"
#include "Objects.h"
#include "IndexGenerator.h"
#include "IndexNewOld.h"
#include "DependentReadObject.h"
#include "DependentReadPointer.h"


class CDependentReadObjects
{
public:
			void					Init(void);
			void					Kill(void);

	virtual bool					AddDependent(CObjectIdentifier* pcHeader, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated, uint16 iNumEmbedded, uint16 iEmbeddedIndex) =0;
	virtual bool					AddReverseDependent(CObjectIdentifier* pcHeader, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingHeapFrom, uint16 iNumEmbedded, uint16 iEmbeddedIndex) =0;

	virtual CBaseObject*			AllocateForDeserialisation(CObjectHeader* pcHeader) =0;

protected:
			bool					PointTo(CBaseObject* pcObject, uint16 iEmbeddedIndex, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcContaining);
};


#endif // __DEPENDENT_READ_OBJECTS_H__

