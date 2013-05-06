#ifndef __DEPENDENT_READ_POINTER_H__
#define __DEPENDENT_READ_POINTER_H__
#include "BaseLib/ArrayTemplate.h"
#include "CoreLib/IndexedGeneral.h"


class CBaseObject;
class CDependentReadPointer
{
public:
	CBaseObject**	mppcPointedFrom;
	CBaseObject*	mpcContaining;
	OIndex			moiPointedTo;

	void Init(CBaseObject**	ppcPointedFrom, CBaseObject* pcContaining, OIndex oiPointedTo);
};


typedef CArrayTemplate<CDependentReadPointer>	CArrayDependentReadPointer;


#endif // __DEPENDENT_READ_POINTER_H__

