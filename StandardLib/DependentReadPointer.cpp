#include "DependentReadPointer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadPointer::Init(CEmbeddedObject** ppcPointedFrom, CBaseObject* pcContaining, OIndex oiPointedTo, unsigned short iNumEmbedded, unsigned short iEmbeddedIndex)
{
	mppcPointedFrom = ppcPointedFrom;
	mpcContaining = pcContaining;
	moiPointedTo = oiPointedTo;
	miNumEmbedded = iNumEmbedded;
	miEmbeddedIndex = iEmbeddedIndex;
}

