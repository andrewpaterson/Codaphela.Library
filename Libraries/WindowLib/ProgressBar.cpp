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
#include "ProgressBar.h"


//////////////////////////////////////////////////////////////////////////
//
// Empty, in sTrack, with sBar for what will be done.  As wide as it is
// laid out (no width of its own) and iHeight pixels high.
//
//////////////////////////////////////////////////////////////////////////
void CProgressBar::Init(Ptr<CWindow> pWindow, ARGB32 sTrack, ARGB32 sBar, int32 iHeight)
{
	PreInit();

	mpDraw = OMalloc<CProgressBarDraw>(sTrack, sBar);
	CCanvas::Init(pWindow, mpDraw);
	SetDesiredSize(0, iHeight);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProgressBar::Class(void)
{
	CCanvas::Class();
	M_Pointer(mpDraw);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProgressBar::Free(void)
{
	CCanvas::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CProgressBar::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CProgressBar::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// How much is done, 0 to 1.  The bar is drawn again if it changes.
//
//////////////////////////////////////////////////////////////////////////
void CProgressBar::SetFraction(float fFraction)
{
	float	fOld;

	fOld = mpDraw->GetFraction();
	mpDraw->SetFraction(fFraction);
	if (mpDraw->GetFraction() != fOld)
	{
		Invalidate();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// iDone of iTotal is done.  Nothing is done when there is nothing to do.
//
//////////////////////////////////////////////////////////////////////////
void CProgressBar::SetProgress(int64 iDone, int64 iTotal)
{
	SetFraction((iTotal > 0) ? (float)((double)iDone / (double)iTotal) : 0.0f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProgressBar::SetBarColour(ARGB32 sBar)
{
	if (mpDraw->GetBarColour() != sBar)
	{
		mpDraw->SetBarColour(sBar);
		Invalidate();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProgressBar::SetTrackColour(ARGB32 sTrack)
{
	if (mpDraw->GetTrackColour() != sTrack)
	{
		mpDraw->SetTrackColour(sTrack);
		Invalidate();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How many pixels of the bar's width are done.
//
//////////////////////////////////////////////////////////////////////////
int32 CProgressBar::GetBarWidth(void)
{
	return mpDraw->GetBarWidth(GetActualSize().x);
}


float CProgressBar::GetFraction(void) { return mpDraw->GetFraction(); }
ARGB32 CProgressBar::GetBarColour(void) { return mpDraw->GetBarColour(); }
ARGB32 CProgressBar::GetTrackColour(void) { return mpDraw->GetTrackColour(); }

