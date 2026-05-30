#include "ImageRowBlitter.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitter::Init(CBaseImageRowBlitter* pcBlitter, size uiXStart, size uiXEnd, size uiY)
{
	this->mpcBlitter = pcBlitter;
	this->sOffset.Init(uiXStart, uiY);
	this->uiXEnd = uiXEnd;
}

