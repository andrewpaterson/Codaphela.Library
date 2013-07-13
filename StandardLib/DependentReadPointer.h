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
	OIndex			moiPointedTo;  //This pointer refers to the OLD index (ie: the one in the file not in the database).
	unsigned short	miEmbeddedIndex;
	unsigned short	miNumEmbedded;

	void Init(CBaseObject**	ppcPointedFrom, CBaseObject* pcContaining, OIndex oiPointedTo, unsigned short iNumEmbedded, unsigned short iEmbeddedIndex);
};


typedef CArrayTemplate<CDependentReadPointer>	CArrayDependentReadPointer;


#endif // __DEPENDENT_READ_POINTER_H__

