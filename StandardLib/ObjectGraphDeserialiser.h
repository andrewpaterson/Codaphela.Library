#ifndef __OBJECT_GRAPH_DESERIALISER_H__
#define __OBJECT_GRAPH_DESERIALISER_H__
#include "BaseObject.h"
#include "ObjectReader.h"
#include "Pointer.h"
#include "DependentReadObjects.h"


class CObjectGraphDeserialiser
{
protected:
	CObjectReader*			mpcReader;
	CDependentReadObjects	mcDependentObjects;

public:
	void			Init(CObjectReader* pcReader);
	void			Kill(void);

	CPointerObject	Read(char* szObjectName);
	void			AddDependent(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr);

protected:
	BOOL			ReadUnread(CDependentReadObject* pcDependent, BOOL bFirst);
	void			MarkRead(CDependentReadObject* pcDependent);
};


#endif // __OBJECT_GRAPH_DESERIALISER_H__

