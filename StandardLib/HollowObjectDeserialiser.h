#ifndef __HOLLOW_OBJECT_DESERIALISER_H__
#define __HOLLOW_OBJECT_DESERIALISER_H__
#include "ObjectDeserialiser.h"


class CObjectGraphDeserialiser;
class CHollowObjectDeserialiser : public CObjectDeserialiser
{
protected:
public:
	void	Init(CSerialisedObject* pcSerialised);
	void	Kill(void);

	BOOL	ReadPointer(CPointerObject* pObject);
	BOOL	ReadDependent(CBaseObject** ppcObjectPtr, CBaseObject* pcContaining);

protected:
	void	AddHollowObject(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr, CBaseObject* pcContaining);
};


#endif // __HOLLOW_OBJECT_DESERIALISER_H__

