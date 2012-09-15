#ifndef __HOLLOW_OBJECT_GRAPH_DESERIALISER_H__
#define __HOLLOW_OBJECT_GRAPH_DESERIALISER_H__
#include "ObjectGraphDeserialiser.h"


class CHollowObjectGraphDeserialiser : public CObjectGraphDeserialiser
{
protected:
	CArrayIndexNewOld	mcExistingHollowRemap;

public:
	void			Init(CObjectReader* pcReader, CIndexGenerator* pcIndexGenerator);
	void			Kill(void);

	CPointerObject	Read(char* szObjectName);
	void			AddDependent(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr, CBaseObject* pcContaining);
	BOOL			AddHollow(CDependentReadObject* pcDependent);
	OIndex			GetExistingHollowRemap(OIndex oiNew);
	BOOL			FixPointers(void);
};


#endif // __HOLLOW_OBJECT_GRAPH_DESERIALISER_H__

