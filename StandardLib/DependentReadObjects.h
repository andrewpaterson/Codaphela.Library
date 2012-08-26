#ifndef __DEPENDENT_READ_OBJECTS_H__
#define __DEPENDENT_READ_OBJECTS_H__
#include "DependentReadObject.h"


class CDependentReadPointer
{
public:
	CBaseObject**	mppcPointedFrom;
	OIndex			moiPointedTo;

	void Init(CBaseObject**	ppcPointedFrom, OIndex oiPointedTo);
};


typedef CArrayTemplate<CDependentReadPointer>	CArrayDependentReadPointer;


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
	void					Add(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr);
	CDependentReadObject*	GetUnread(void);
	void					SetInitialIndex(OIndex oi);
	BOOL					Mark(OIndex oi);

	int						NumPointers(void);
	CDependentReadPointer*	GetPointer(int iIndex);
};


#endif // __DEPENDENT_READ_OBJECTS_H__

