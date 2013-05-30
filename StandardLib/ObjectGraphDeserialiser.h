#ifndef __OBJECT_GRAPH_DESERIALISER_H__
#define __OBJECT_GRAPH_DESERIALISER_H__
#include "IndexNewOld.h"
#include "BaseObject.h"
#include "ObjectReader.h"
#include "Pointer.h"
#include "DependentReadObjects.h"
#include "DependentObjectAdder.h"


//The graph deserialiser works with chunk files and simple files.  
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

	CDependentReadObjects	mcDependentObjects;
	CArrayIndexNewOld		mcIndexRemap;
	
public:
	void			Init(CObjectReader* pcReader, CIndexGenerator* pcIndexGenerator, CObjectAllocator* pcAllocator);
	void			Kill(void);

	CPointerObject	Read(char* szObjectName);
			 
	//Used by CObjectDeserialiser
	BOOL			AddDependent(CPointerHeader* pcHeader, CBaseObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated);
	CPointerObject	AllocateObject(CObjectHeader* pcHeader);

protected:
	//CPointerObject	Read(OIndex oi);
	BOOL			ReadAfterAddDependent(void);
	BOOL			ReadUnread(CDependentReadObject* pcDependent, BOOL bFirst);
	void			MarkRead(OIndex oi);
	BOOL			FixPointers(void);
	OIndex			GetNewIndexFromOld(OIndex oiNew);
	void			AddIndexRemap(OIndex oiNew, OIndex oiOld);
};


#endif // __OBJECT_GRAPH_DESERIALISER_H__

