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
#ifndef __EXTERNAL_OBJECT_DESERIALISER_H__
#define __EXTERNAL_OBJECT_DESERIALISER_H__
#include "BaseObject.h"
#include "ExternalObjectReader.h"
#include "Pointer.h"
#include "DependentReadObjects.h"
#include "DependentReadObject.h"
#include "DependentReadPointer.h"
#include "IndexNewOld.h"
#include "BaseLib/MapLongLong.h"
#include "BaseLib/ArrayTemplate.h"
#include "NamedIndexedObjects.h"


//Reads an object and everything it points to out of an external format (chunk files, simple files, a converter's
//source) into memory, giving each read object a new index.  Named objects replace the ones in memory or the database
//with their names (and keep their indices).  With named hollows the named objects pointed to aren't read but left
//hollow, to be loaded from the database.  Read objects are dirty: they're saved into the database once rooted.


class CExternalObjectDeserialiser : public CDependentReadObjects
{
protected:
	CArrayDependentReadObject	mcReadObjects;		//In the order they were found.
	CMapLongLong				mcReadObjectPositions;	//Old index -> position in mcReadObjects.
	CArrayTemplate<OIndex>		maoiUnread;			//A min-heap of the old indices found: read smallest first.  Read ones are skipped when they come up.
	CArrayDependentReadPointer	mcPointers;
	CArrayIndexNewOld			mcIndexRemap;
	CMapLongLong				mcNewIndices;		//Old index -> new index, as mcIndexRemap.

	CExternalObjectReader*		mpcReader;  //The ObjectReader knows how to load the serialised form of the object by name.
	CObjects*					mpcObjects;

	CNamedIndexedObjects*		mpcMemory;
	bool						mbNamedHollows;
	
public:
	void					Init(CExternalObjectReader* pcReader, bool bNamedHollows, CObjects* pcObjects);
	void					Kill(void);

	CBaseObject*			Read(const char* szObjectName);
			 
	//Used by CObjectReader
	CBaseObject*			AllocateForDeserialisation(CObjectHeader* pcHeader) override;

	//The new index each read object was given, by the index it had in the external format.
	OIndex					GetNewIndexFromOld(OIndex oiOld);
	CArrayIndexNewOld*		GetArrayIndexNewOld(void);

protected:
	CBaseObject*			ReadObjectAndDependents(const char* szObjectName);
	CBaseObject*			ReadSerialised(CSerialisedObject* pcSerialised);
	bool					ReadDependentObjects(void);
	bool					ReadUnread(CDependentReadObject* pcDependent);
	bool					AddHeapFromPointersAndCreateHollowObjects(void);
	bool					AddHeapFromPointersAndCreateHollowObject(CDependentReadPointer* pcDependentReadPointer);
	bool					AddDependent(CObjectIdentifier* pcObjectIdentifier, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated, uint16 iNumEmbedded, uint16 iEmbeddedIndex) override;
	bool					AddReverseDependent(CObjectIdentifier* pcObjectIdentifier, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingHeapFrom, uint16 iNumEmbedded, uint16 iEmbeddedIndex) override;

	CDependentReadObject*	GetUnread(void);
	void					PushUnread(OIndex oi);
	OIndex					PopUnread(void);
	bool					Mark(OIndex oi);

	CDependentReadObject*	GetObject(OIndex oi);
		
	size					NumPointers(void);
	CDependentReadPointer*	GetPointer(size iIndex);
		
	void					AddIndexRemap(OIndex oiNew, OIndex oiOld);
	size					NumObjects(void);
};


#endif // __EXTERNAL_OBJECT_DESERIALISER_H__

