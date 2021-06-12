#ifndef __DEPENDENT_READ_POINTER_H__
#define __DEPENDENT_READ_POINTER_H__
#include "BaseLib/ArrayTemplate.h"
#include "CoreLib/IndexedGeneral.h"


class CBaseObject;
class CEmbeddedObject;
class CDependentReadPointer
{
public:
	CEmbeddedObject**	mppcPointedFrom;
	CBaseObject*		mpcContaining;
	OIndex				moiPointedTo;  //This pointer refers to the OLD index (ie: the one in the file not in the database).
	uint16				miEmbeddedIndex;
	uint16				miNumEmbedded;

	void Init(CEmbeddedObject**	ppcPointedFrom, CBaseObject* pcContaining, OIndex oiPointedTo, uint16 iNumEmbedded, uint16 iEmbeddedIndex);
};


typedef CArrayTemplate<CDependentReadPointer>	CArrayDependentReadPointer;


#endif // __DEPENDENT_READ_POINTER_H__

