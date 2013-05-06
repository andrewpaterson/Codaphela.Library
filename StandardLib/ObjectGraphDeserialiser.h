#ifndef __OBJECT_GRAPH_DESERIALISER_H__
#define __OBJECT_GRAPH_DESERIALISER_H__
#include "IndexNewOld.h"
#include "BaseObject.h"
#include "ObjectReader.h"
#include "Pointer.h"
#include "DependentReadObjects.h"


class CObjectAllocator;
class CIndexGenerator;
class CObjectGraphDeserialiser
{
protected:
	CObjectReader*			mpcReader;
	CDependentReadObjects	mcDependentObjects;
	CArrayIndexNewOld		mcIndexRemap;
	CObjectAllocator*		mpcAllocator;
	
public:
			void			Init(CObjectReader* pcReader, CIndexGenerator* pcIndexGenerator, CObjectAllocator* pcAllocator);
			void			Kill(void);

			CPointerObject	Read(char* szObjectName);
	virtual BOOL			AddDependent(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr, CBaseObject* pcContaining) =0;
			void			AddIndexRemap(OIndex oiNew, OIndex oiOld);

protected:
			BOOL			ReadUnread(CDependentReadObject* pcDependent, BOOL bFirst);
			void			MarkRead(OIndex oi);
			BOOL			FixPointers(void);
			void			FixPointer(CBaseObject* pcBaseObject, CBaseObject** ppcPointedFrom, CBaseObject* pcContaining);
			BOOL			FixExisting(void);
			OIndex			GetExistingRemap(OIndex oiNew);
			void			ForceAddDependent(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr);
};


#endif // __OBJECT_GRAPH_DESERIALISER_H__

