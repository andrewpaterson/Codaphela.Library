#include "DependentReadPointer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadPointer::Init(CEmbeddedObject** ppcPointedFrom, CBaseObject* pcContaining, OIndex oiPointedTo, uint16 iNumEmbedded, uint16 iEmbeddedIndex)
{
	mppcPointedFrom = ppcPointedFrom;
	mpcContaining = pcContaining;
	moiPointedTo = oiPointedTo;
	miNumEmbedded = iNumEmbedded;
	miEmbeddedIndex = iEmbeddedIndex;
}

