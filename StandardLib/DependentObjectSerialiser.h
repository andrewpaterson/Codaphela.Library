#ifndef __DEPENDENT_OBJECT_SERIALISER_H__
#define __DEPENDENT_OBJECT_SERIALISER_H__
#include "ObjectSerialiser.h"

//This class exists so that an the entire reachable graph from an object can be written.
//It makes no sense to use this if objects are being persisted by CObjects.  
//So far the only usages I have are for testing (or maybe writing out the initial state of the graph).
class CObjectGraphSerialiser;
class CDependentObjectSerialiser : public CObjectSerialiser
{
protected:
	CObjectGraphSerialiser*		mpcGraphWriter;

public:
	void		Init(CObjectGraphSerialiser* pcGraphWriter, CBaseObject* pcObject);
	void		Kill(void);

	BOOL		WritePointer(CPointerObject pObject);

protected:
	BOOL		WriteDependent(CBaseObject* pcObject);
};


#endif // __DEPENDENT_OBJECT_SERIALISER_H__

