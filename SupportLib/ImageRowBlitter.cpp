#include "ImageRowBlitter.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitter::Init(CBaseImageRowBlitter* pcBlitter, size uiXStart, size uiXEnd, size uiY)
{
	mpcBlitter = pcBlitter;
	msOffset.Init(uiXStart, uiY);
	muiXEnd = uiXEnd;
}

