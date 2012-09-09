#ifndef __HOLLOW_OBJECT_GRAPH_DESERIALISER_H__
#define __HOLLOW_OBJECT_GRAPH_DESERIALISER_H__
#include "ObjectGraphDeserialiser.h"


class CHollowObjectGraphDeserialiser : public CObjectGraphDeserialiser
{
public:
	void			Init(CObjectReader* pcReader, CIndexGenerator* pcIndexGenerator);
	void			Kill(void);

	void			AddDependent(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr, CBaseObject* pcContaining);
};


#endif // __HOLLOW_OBJECT_GRAPH_DESERIALISER_H__

