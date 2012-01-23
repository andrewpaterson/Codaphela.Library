/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "PackedRectangle.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int ComparePackRectangleAttempt(const void* pv1, const void* pv2)
{
	CPackRectangleAttempt*	pcRect1 = (CPackRectangleAttempt*)pv1;
	CPackRectangleAttempt*	pcRect2 = (CPackRectangleAttempt*)pv2;

	if (pcRect1->mpcPackedRectangle->miHeight < pcRect2->mpcPackedRectangle->miHeight)
	{
		return -1;
	}
	if (pcRect1->mpcPackedRectangle->miHeight > pcRect2->mpcPackedRectangle->miHeight)
	{
		return 1;
	}
	if (pcRect1->mpcPackedRectangle->miWidth < pcRect2->mpcPackedRectangle->miWidth)
	{
		return -1;
	}
	if (pcRect1->mpcPackedRectangle->miWidth > pcRect2->mpcPackedRectangle->miWidth)
	{
		return 1;
	}
	return 0;
}

	
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackSourceRectangle::Init(CRectangle* pcRect, void* pvUserData)
{
	miWidth = pcRect->GetWidth();
	miHeight = pcRect->GetHeight();
	mpvUserData = pvUserData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackRectangleAttempt::Init(CPackSourceRectangle* pcPackedRectangle)
{
	mpcPackedRectangle = pcPackedRectangle;
	mpcLine = NULL;
	mix = -1;
	miy = -1;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackRectangleAttempt::Set(CPackedLine* pcLine, int x, int y)
{
	mpcLine = pcLine;
	mix = x;
	miy = y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackedRectangle::Init(int iLeft, int iTop, int iRight, int iBottom, void* pvUserData)
{
	CRectangle::Init(iLeft, iTop, iRight, iBottom);
	mpvUserData = pvUserData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackedRectangle::Init(CRectangle* pcRect, void* pvUserData)
{
	CRectangle::Init(pcRect);
	mpvUserData = pvUserData;
}

