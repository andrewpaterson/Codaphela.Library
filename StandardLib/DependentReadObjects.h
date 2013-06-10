#ifndef __DEPENDENT_READ_OBJECTS_H__
#define __DEPENDENT_READ_OBJECTS_H__
#include "DependentReadPointer.h"
#include "DependentReadObject.h"
#include "IndexNewOld.h"


class CIndexGenerator;
class CDependentReadObjects
{
protected:
	CArrayDependentReadObject	mcObjects;
	CArrayDependentReadPointer	mcPointers;
	CArrayIndexNewOld			mcIndexRemap;

	int							miGetIndex;  //The index of the next object to 'gotten' for reading.

public:
	void					Init(void);
	void					Kill(void);
	void					Add(CPointerHeader* pcHeader, CBaseObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated);
	CDependentReadObject*	GetUnread(void);
	CDependentReadObject*	GetObject(OIndex oi);
	BOOL					Mark(OIndex oi);

	int						NumPointers(void);
	CDependentReadPointer*	GetPointer(int iIndex);

	int						NumObjects(void);
	OIndex					GetNewIndexFromOld(OIndex oiOld);
	void					AddIndexRemap(OIndex oiNew, OIndex oiOld);
};


#endif // __DEPENDENT_READ_OBJECTS_H__

