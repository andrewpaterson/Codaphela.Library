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

			BOOL			AddDependent(CPointerHeader* pcHeader, CBaseObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated);

	virtual CPointerObject	AllocateObject(CObjectHeader* pcHeader) =0;

protected:
	void	AddContainingPointer(CBaseObject* pcBaseObject, CBaseObject** ppcPointedFrom, CBaseObject* pcContaining);
};


#endif // __DEPENDENT_OBJECT_ADDER_H__

