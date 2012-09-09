#ifndef __DEPENDENT_OBJECT_GRAPH_DESERIALISER_H__
#define __DEPENDENT_OBJECT_GRAPH_DESERIALISER_H__
#include "ObjectGraphDeserialiser.h"


class CDependentObjectGraphDeserialiser : public CObjectGraphDeserialiser
{
public:
	void	Init(CObjectReader* pcReader, CIndexGenerator* pcIndexGenerator);
	void	Kill(void);

	void	AddDependent(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr, CBaseObject* pcContaining);
};


#endif // __DEPENDENT_OBJECT_GRAPH_DESERIALISER_H__

