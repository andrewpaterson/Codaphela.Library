#ifndef __OBJECT_GRAPH_DESERIALISER_H__
#define __OBJECT_GRAPH_DESERIALISER_H__
#include "BaseObject.h"
#include "ObjectReader.h"
#include "Pointer.h"
#include "DependentReadObjects.h"
#include "DependentObjectAdder.h"
#include "NamedIndexedObjects.h"


//The Object Graph Deserialiser works with chunk files and simple files.  
//  It does not deserialise objects out of the database.
//  It does not create hollow objects.
//  It does not load objects by id.


class CObjectAllocator;
class CIndexGenerator;
class CObjectGraphDeserialiser : public CDependentObjectAdder
{
protected:
	CObjectReader*			mpcReader;  //The ObjectReader knows how to load the serialised form of the object by name.
	CObjectAllocator*		mpcAllocator;

	CNamedIndexedObjects*	mpcMemory;
	BOOL					mbNamedHollows;
	
public:
	void			Init(CObjectReader* pcReader, BOOL bNamedHollows, CObjectAllocator* pcAllocator, CDependentReadObjects* pcDependentReadObjects, CNamedIndexedObjects* pcMemory);
	void			Kill(void);

	CBaseObject*	Read(char* szObjectName);
			 
	//Used by CObjectDeserialiser
	CBaseObject*	AllocateObject(CObjectHeader* pcHeader);

	OIndex			GetNewIndexFromOld(OIndex oiNew);

protected:
	CBaseObject*	ReadSerialsed(CSerialisedObject* pcSerialised);
	BOOL			ReadDependentObjects(void);
	BOOL			ReadUnread(CDependentReadObject* pcDependent);
	void			MarkRead(OIndex oi);
	BOOL			AddContainingPointersAndCreateHollowObjects(void);
	BOOL			AddContainingPointersAndCreateHollowObject(CDependentReadPointer* pcDependentReadPointer);
};


#endif // __OBJECT_GRAPH_DESERIALISER_H__

