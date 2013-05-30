#ifndef __DEPENDENT_OBJECT_ADDER_H__
#define __DEPENDENT_OBJECT_ADDER_H__
#include "ObjectHeader.h"
#include "BaseObject.h"


class CDependentObjectAdder
{
public:
	virtual BOOL			AddDependent(CPointerHeader* pcHeader, CBaseObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated) =0;
	virtual CPointerObject	AllocateObject(CObjectHeader* pcHeader) =0;

protected:
			void			FixPointer(CBaseObject* pcBaseObject, CBaseObject** ppcPointedFrom, CBaseObject* pcContaining);
};


#endif // __DEPENDENT_OBJECT_ADDER_H__

