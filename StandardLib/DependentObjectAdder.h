#ifndef __DEPENDENT_OBJECT_ADDER_H__
#define __DEPENDENT_OBJECT_ADDER_H__
#include "ObjectHeader.h"
#include "BaseObject.h"
#include "DependentReadObjects.h"


class CDependentObjectAdder
{
protected:
	CDependentReadObjects*	mpcDependentObjects;

public:
			void			Init(CDependentReadObjects*	pcDependentObjects);
			void			Kill(void);

			BOOL			AddDependent(CObjectIdentifier* pcHeader, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated, uint16 iNumEmbedded, uint16 iEmbeddedIndex);

	virtual CBaseObject*	AllocateForDeserialisation(CObjectHeader* pcHeader) =0;

protected:
			void			AddHeapFrom(CEmbeddedObject* pcBaseObject, CEmbeddedObject** ppcPointedFrom, CBaseObject* pcContaining);
};


#endif // __DEPENDENT_OBJECT_ADDER_H__

