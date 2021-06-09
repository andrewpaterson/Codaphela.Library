#ifndef __DEPENDENT_OBJECT_ADDER_H__
#define __DEPENDENT_OBJECT_ADDER_H__
#include "BaseLib/Log.h"
#include "ObjectHeader.h"
#include "BaseObject.h"
#include "Pointer.h"
#include "Objects.h"
#include "IndexGenerator.h"
#include "IndexNewOld.h"
#include "DependentReadObject.h"
#include "DependentReadPointer.h"


class CDependentObjectAdder
{
protected:
	CArrayDependentReadObject	mcReadObjects;
	CArrayDependentReadPointer	mcPointers;
	CArrayIndexNewOld			mcIndexRemap;

	int							miGetIndex;  //The index of the next object to 'gotten' for reading.

public:
			void					Init(void);
			void					Kill(void);

			BOOL					AddDependent(CObjectIdentifier* pcHeader, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated, uint16 iNumEmbedded, uint16 iEmbeddedIndex);

	virtual CBaseObject*			AllocateForDeserialisation(CObjectHeader* pcHeader) =0;

			OIndex					GetNewIndexFromOld(OIndex oiOld);  //Is this used?
			CArrayIndexNewOld*		GetArrayIndexNewOld(void);

protected:
			void					AddHeapFrom(CEmbeddedObject* pcBaseObject, CEmbeddedObject** ppcPointedFrom, CBaseObject* pcContaining);
			CDependentReadObject*	GetUnread(void);
			CDependentReadObject*	GetObject(OIndex oi);
			BOOL					Mark(OIndex oi);
		
			int						NumPointers(void);
			CDependentReadPointer*	GetPointer(int iIndex);
		
			int						NumObjects(void);
			void					AddIndexRemap(OIndex oiNew, OIndex oiOld);
};


#endif // __DEPENDENT_OBJECT_ADDER_H__

