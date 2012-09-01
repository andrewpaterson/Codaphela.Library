#ifndef __OBJECT_GRAPH_DESERIALISER_H__
#define __OBJECT_GRAPH_DESERIALISER_H__
#include "IndexNewOld.h"
#include "BaseObject.h"
#include "ObjectReader.h"
#include "Pointer.h"
#include "DependentReadObjects.h"


class CIndexGenerator;
class CObjectGraphDeserialiser
{
protected:
	CObjectReader*			mpcReader;
	CDependentReadObjects	mcDependentObjects;
	CArrayIndexNewOld		mcIndexRemap;
	
public:
	void			Init(CObjectReader* pcReader, CIndexGenerator* pcIndexGenerator);
	void			Kill(void);

	CPointerObject	Read(char* szObjectName);
	void			AddDependent(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr, CBaseObject* pcContaining);
	void			AddIndexRemap(OIndex oiNew, OIndex oiOld);

protected:
	BOOL			ReadUnread(CDependentReadObject* pcDependent, BOOL bFirst);
	void			MarkRead(OIndex oi);
	BOOL			FixPointers(void);
	BOOL			FixExisting(void);
	OIndex			GetExistingRemap(OIndex oiNew);
};


#endif // __OBJECT_GRAPH_DESERIALISER_H__

