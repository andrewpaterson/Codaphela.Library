#include "DependentObjectAdder.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentObjectAdder::FixPointer(CBaseObject* pcBaseObject, CBaseObject** ppcPointedFrom, CBaseObject* pcContaining)
{
	*ppcPointedFrom = pcBaseObject;

	if (pcContaining)
	{
		pcBaseObject->AddFrom(pcContaining);
	}
}

