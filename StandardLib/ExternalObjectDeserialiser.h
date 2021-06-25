#ifndef __EXTERNAL_OBJECT_DESERIALISER_H__
#define __EXTERNAL_OBJECT_DESERIALISER_H__
#include "BaseObject.h"
#include "ExternalObjectReader.h"
#include "Pointer.h"
#include "DependentReadObjects.h"
#include "NamedIndexedObjects.h"


//The External Object Deserialiser works with chunk files and simple files.  
//  It does not deserialise objects out of the database.
//  It does not create hollow objects.
//  It does not load objects by id.


class CIndexGenerator;
class CExternalObjectDeserialiser : public CDependentReadObjects
{
protected:
	CArrayDependentReadObject	mcReadObjects;
	CArrayDependentReadPointer	mcPointers;
	CArrayIndexNewOld			mcIndexRemap;

	int							miGetIndex;  //The index of the next object to 'gotten' for reading.

	CExternalObjectReader*		mpcReader;  //The ObjectReader knows how to load the serialised form of the object by name.
	CObjects*					mpcObjects;

	CNamedIndexedObjects*		mpcMemory;
	BOOL						mbNamedHollows;
	
public:
	void					Init(CExternalObjectReader* pcReader, BOOL bNamedHollows, CObjects* pcObjects, CNamedIndexedObjects* pcMemory);
	void					Kill(void);

	CBaseObject*			Read(char* szObjectName);
			 
	//Used by CObjectReader
	CBaseObject*			AllocateForDeserialisation(CObjectHeader* pcHeader);

	//I don't know what these are
	OIndex					GetNewIndexFromOld(OIndex oiOld);  //Is this used?
	CArrayIndexNewOld*		GetArrayIndexNewOld(void);

protected:
	CBaseObject*			ReadSerialsed(CSerialisedObject* pcSerialised);
	BOOL					ReadDependentObjects(void);
	BOOL					ReadUnread(CDependentReadObject* pcDependent);
	void					MarkRead(OIndex oi);
	BOOL					AddHeapFromPointersAndCreateHollowObjects(void);
	BOOL					AddHeapFromPointersAndCreateHollowObject(CDependentReadPointer* pcDependentReadPointer);
	BOOL					AddDependent(CObjectIdentifier* pcHeader, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated, uint16 iNumEmbedded, uint16 iEmbeddedIndex);
	BOOL					AddReverseDependent(CObjectIdentifier* pcHeader, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingHeapFrom, uint16 iNumEmbedded, uint16 iEmbeddedIndex, int iDistToRoot);

	CDependentReadObject*	GetUnread(void);
	BOOL					Mark(OIndex oi);

	CDependentReadObject*	GetObject(OIndex oi);
		
	int						NumPointers(void);
	CDependentReadPointer*	GetPointer(int iIndex);
		
	void					AddIndexRemap(OIndex oiNew, OIndex oiOld);
	int						NumObjects(void);
};


#endif // !__EXTERNAL_OBJECT_DESERIALISER_H__

