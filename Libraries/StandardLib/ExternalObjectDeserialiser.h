#ifndef __EXTERNAL_OBJECT_DESERIALISER_H__
#define __EXTERNAL_OBJECT_DESERIALISER_H__
#include "BaseObject.h"
#include "ExternalObjectReader.h"
#include "Pointer.h"
#include "DependentReadObjects.h"
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

