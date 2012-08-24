#ifndef __OBJECT_GRAPH_DESERIALISER_H__
#define __OBJECT_GRAPH_DESERIALISER_H__
#include "BaseObject.h"
#include "ObjectReader.h"
#include "Pointer.h"


class CObjectGraphDeserialiser
{
protected:
	CObjectReader*	mpcReader;

public:
	void			Init(CObjectReader* pcReader);
	void			Kill(void);

	CPointerObject	Read(char* szObjectName);
};


#endif // __OBJECT_GRAPH_DESERIALISER_H__

