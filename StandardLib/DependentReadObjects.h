#ifndef __DEPENDENT_READ_OBJECTS_H__
#define __DEPENDENT_READ_OBJECTS_H__
#include "DependentReadPointer.h"
#include "DependentReadObject.h"


class CIndexGenerator;
class CDependentReadObjects
{
protected:
	CArrayDependentReadObject	mcObjects;
	CArrayDependentReadPointer	mcPointers;
	int							miGetIndex;
	CIndexGenerator*			mpcIndexGenerator;

public:
	void					Init(CIndexGenerator* pcIndexGenerator);
	void					Kill(void);
	void					Add(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr, CBaseObject* pcContaining);
	void					AddIgnoreNamed(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr, CBaseObject* pcContaining);
	void					AddHollow(char* szName, OIndex oiOld, CBaseObject** ppcObjectPtr, CBaseObject* pcContaining);
	CDependentReadObject*	GetUnread(void);
	void					SetInitialIndex(OIndex oi);
	BOOL					Mark(OIndex oi);

	int						NumPointers(void);
	CDependentReadPointer*	GetPointer(int iIndex);

	int						NumObjects(void);
	CDependentReadObject*	GetObject(int iIndex);
};


#endif // __DEPENDENT_READ_OBJECTS_H__

