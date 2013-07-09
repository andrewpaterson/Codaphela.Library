#include "DependentReadPointer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadPointer::Init(CBaseObject** ppcPointedFrom, CBaseObject* pcContaining, OIndex oiPointedTo, int iEmbeddedIndex)
{
	mppcPointedFrom = ppcPointedFrom;
	mpcContaining = pcContaining;
	moiPointedTo = oiPointedTo;
	miEmbeddedIndex = iEmbeddedIndex;
}

