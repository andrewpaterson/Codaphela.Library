#include "OffsetImageRowBlitter.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COffsetImageRowBlitter::Init(CBaseImageRowBlitter* pcBlitter, CBaseImageRowSizingBlitter* pcSizingBlitter, size uiXStart, size uiXEnd, size uiY)
{
	mpcBlitter = pcBlitter;
	mpcSizingBlitter = pcSizingBlitter;
	msOffset.Init(uiXStart, uiY);
	muiXEnd = uiXEnd;
}

