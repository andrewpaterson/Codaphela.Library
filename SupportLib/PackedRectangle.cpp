/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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

	if (pcRect1->mcSourceRectangle.miHeight < pcRect2->mcSourceRectangle.miHeight)
	{
		return -1;
	}
	if (pcRect1->mcSourceRectangle.miHeight > pcRect2->mcSourceRectangle.miHeight)
	{
		return 1;
	}
	if (pcRect1->mcSourceRectangle.miWidth < pcRect2->mcSourceRectangle.miWidth)
	{
		return -1;
	}
	if (pcRect1->mcSourceRectangle.miWidth > pcRect2->mcSourceRectangle.miWidth)
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
void CPackRectangleAttempt::Init(CPackSourceRectangle* pcSourceRectangle)
{
	mcSourceRectangle = *pcSourceRectangle;
	mix = -1;
	miy = -1;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackRectangleAttempt::SetPos(int x, int y)
{
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CPackedRectangle::GetUserData(void)
{
	return mpvUserData;
}

