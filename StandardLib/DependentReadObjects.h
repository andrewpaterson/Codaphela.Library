#ifndef __DEPENDENT_READ_OBJECTS_H__
#define __DEPENDENT_READ_OBJECTS_H__
#include "DependentReadObject.h"


class CDependentReadObjects
{
protected:
	CArrayDependentReadObject	mcObjects;
	int							miGetIndex;

public:
	void					Init(void);
	void					Kill(void);
	void					Add(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr);
	CDependentReadObject*	GetUnread(void);
	void					SetInitialIndex(OIndex oi);
};


#endif // __DEPENDENT_READ_OBJECTS_H__

