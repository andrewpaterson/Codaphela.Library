/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "StandardLib/ClassDefines.h"
#include "Canvas.h"
#include "ProgressBarDraw.h"


//////////////////////////////////////////////////////////////////////////
//
// Nothing done yet.
//
//////////////////////////////////////////////////////////////////////////
void CProgressBarDraw::Init(ARGB32 sTrack, ARGB32 sBar)
{
	PreInit();
	CCanvasDraw::Init();
	msTrack = sTrack;
	msBar = sBar;
	mfFraction = 0.0f;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProgressBarDraw::Class(void)
{
	CCanvasDraw::Class();
	U_UInt32(msTrack);
	U_UInt32(msBar);
	U_Float32(mfFraction);
}


//////////////////////////////////////////////////////////////////////////
//
// The track, then the bar over its left part.  DrawBox includes the
// right and bottom edges of its rectangle.
//
//////////////////////////////////////////////////////////////////////////
bool CProgressBarDraw::Draw(Ptr<CCanvas> pCanvas)
{
	CInt32Rect	cRect;
	SInt32Vec2	sSize;
	int32		iBar;

	if (!pCanvas->IsValid())
	{
		return true;
	}
	sSize = pCanvas->GetActualSize();
	if ((sSize.x <= 0) || (sSize.y <= 0))
	{
		return true;
	}
	cRect.Init(0, 0, sSize.x - 1, sSize.y - 1);
	pCanvas->DrawBox(&cRect, true, msTrack);

	iBar = GetBarWidth(sSize.x);
	if (iBar > 0)
	{
		cRect.Init(0, 0, iBar - 1, sSize.y - 1);
		pCanvas->DrawBox(&cRect, true, msBar);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// How many of iWidth pixels the bar covers: the fraction done, rounded,
// and all of them only when it is all done.
//
//////////////////////////////////////////////////////////////////////////
int32 CProgressBarDraw::GetBarWidth(int32 iWidth)
{
	int32	iBar;

	if (mfFraction >= 1.0f)
	{
		return iWidth;
	}
	iBar = (int32)(mfFraction * (float)iWidth + 0.5f);
	if (iBar >= iWidth)
	{
		iBar = iWidth - 1;
	}
	return (iBar > 0) ? iBar : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Kept between 0 and 1.
//
//////////////////////////////////////////////////////////////////////////
void CProgressBarDraw::SetFraction(float fFraction)
{
	if (fFraction < 0.0f)
	{
		fFraction = 0.0f;
	}
	else if (fFraction > 1.0f)
	{
		fFraction = 1.0f;
	}
	mfFraction = fFraction;
}


float CProgressBarDraw::GetFraction(void) { return mfFraction; }
void CProgressBarDraw::SetBarColour(ARGB32 sBar) { msBar = sBar; }
ARGB32 CProgressBarDraw::GetBarColour(void) { return msBar; }
void CProgressBarDraw::SetTrackColour(ARGB32 sTrack) { msTrack = sTrack; }
ARGB32 CProgressBarDraw::GetTrackColour(void) { return msTrack; }

