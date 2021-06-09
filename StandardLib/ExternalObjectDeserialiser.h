#ifndef __EXTERNAL_OBJECT_DESERIALISER_H__
#define __EXTERNAL_OBJECT_DESERIALISER_H__
#include "BaseObject.h"
#include "ObjectReader.h"
#include "Pointer.h"
#include "DependentObjectAdder.h"
#include "NamedIndexedObjects.h"


//The External Object Deserialiser works with chunk files and simple files.  
//  It does not deserialise objects out of the database.
//  It does not create hollow objects.
//  It does not load objects by id.


class CIndexGenerator;
class CExternalObjectDeserialiser : public CDependentObjectAdder
{
protected:
	CObjectReader*			mpcReader;  //The ObjectReader knows how to load the serialised form of the object by name.
	CObjects*				mpcObjects;

	CNamedIndexedObjects*	mpcMemory;
	BOOL					mbNamedHollows;
	
public:
	void			Init(CObjectReader* pcReader, BOOL bNamedHollows, CObjects* pcObjects, CNamedIndexedObjects* pcMemory);
	void			Kill(void);

	CBaseObject*	Read(char* szObjectName);
			 
	//Used by CObjectDeserialiser
	CBaseObject*	AllocateForDeserialisation(CObjectHeader* pcHeader);

protected:
	CBaseObject*	ReadSerialsed(CSerialisedObject* pcSerialised);
	BOOL			ReadDependentObjects(void);
	BOOL			ReadUnread(CDependentReadObject* pcDependent);
	void			MarkRead(OIndex oi);
	BOOL			AddContainingPointersAndCreateHollowObjects(void);
	BOOL			AddContainingPointersAndCreateHollowObject(CDependentReadPointer* pcDependentReadPointer);
};


#endif // __EXTERNAL_OBJECT_DESERIALISER_H__

