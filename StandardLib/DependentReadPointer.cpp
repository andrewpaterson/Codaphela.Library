#include "DependentReadPointer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadPointer::Init(CBaseObject** ppcPointedFrom, CBaseObject* pcContaining, OIndex oiPointedTo)
{
	mppcPointedFrom = ppcPointedFrom;
	mpcContaining = pcContaining;
	moiPointedTo = oiPointedTo;
}

