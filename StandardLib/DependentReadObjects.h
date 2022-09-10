#ifndef __DEPENDENT_READ_OBJECTS_H__
#define __DEPENDENT_READ_OBJECTS_H__
#include "BaseLib/Logger.h"
#include "ObjectHeader.h"
#include "BaseObject.h"
#include "Pointer.h"
#include "Objects.h"
#include "IndexGenerator.h"
#include "IndexNewOld.h"
#include "DependentReadObject.h"
#include "DependentReadPointer.h"


class CDependentReadObjects
{
public:
			void					Init(void);
			void					Kill(void);

	virtual bool					AddDependent(CObjectIdentifier* pcHeader, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated, uint16 iNumEmbedded, uint16 iEmbeddedIndex) =0;
	virtual bool					AddReverseDependent(CObjectIdentifier* pcHeader, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingHeapFrom, uint16 iNumEmbedded, uint16 iEmbeddedIndex, int iDistToRoot) =0;

	virtual CBaseObject*			AllocateForDeserialisation(CObjectHeader* pcHeader) =0;

protected:
			void					AddHeapFrom(CEmbeddedObject* pcBaseObject, CBaseObject* pcContaining);
};


#endif // !__DEPENDENT_READ_OBJECTS_H__

