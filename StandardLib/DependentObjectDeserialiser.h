#ifndef __DEPENDENT_OBJECT_DESERIALISER_H__
#define __DEPENDENT_OBJECT_DESERIALISER_H__
#include "ObjectDeserialiser.h"


class CObjectGraphDeserialiser;
class CDependentObjectDeserialiser : public CObjectDeserialiser
{
protected:
	CObjectGraphDeserialiser*	mpcGraphDeserialiser;

public:
	void Init(CObjectGraphDeserialiser* pcGraphDeserialiser, CSerialisedObject* pcSerialised);
	void Kill(void);

	BOOL ReadPointer(CPointerObject* pObject);
	BOOL ReadDependent(CBaseObject** ppcObjectPtr);
};


#endif // __DEPENDENT_OBJECT_DESERIALISER_H__

