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
#include "ScrollBarMarks.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScrollBarMarksDraw::Init(bool bHorizontal)
{
	PreInit();
	CCanvasDraw::Init();
	masMarks.Init();
	miContentLength = 0;
	mbHorizontal = bHorizontal;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScrollBarMarksDraw::Class(void)
{
	CCanvasDraw::Class();
	U_Unknown(CArrayScrollBarMarks, masMarks);
	U_Int32(miContentLength);
	U_Bool(mbHorizontal);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScrollBarMarksDraw::Free(void)
{
	masMarks.Kill();
	CCanvasDraw::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// How long what is scrolled is: the marks are along it.  False if it was
// that already.
//
//////////////////////////////////////////////////////////////////////////
bool CScrollBarMarksDraw::SetContentLength(int32 iContentLength)
{
	if (miContentLength == iContentLength)
	{
		return false;
	}
	miContentLength = iContentLength;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A source's marks, in place of those it had: each a start (x) and a
// length (y) along the content, all in one colour.
//
//////////////////////////////////////////////////////////////////////////
void CScrollBarMarksDraw::SetMarks(uint16 uiSource, SInt32Vec2* asMarks, size uiNumMarks, ARGB32 sColour)
{
	SScrollBarMark*	psMark;
	size			i;

	ClearMarks(uiSource);
	for (i = 0; i < uiNumMarks; i++)
	{
		psMark = masMarks.Add();
		psMark->iStart = asMarks[i].x;
		psMark->iLength = asMarks[i].y;
		psMark->sColour = sColour;
		psMark->uiSource = uiSource;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// False if the source had none.
//
//////////////////////////////////////////////////////////////////////////
bool CScrollBarMarksDraw::ClearMarks(uint16 uiSource)
{
	size	i;
	size	uiKept;

	uiKept = 0;
	for (i = 0; i < masMarks.NumElements(); i++)
	{
		if (masMarks.Get(i)->uiSource != uiSource)
		{
			if (uiKept != i)
			{
				*masMarks.Get(uiKept) = *masMarks.Get(i);
			}
			uiKept++;
		}
	}
	if (uiKept == masMarks.NumElements())
	{
		return false;
	}
	masMarks.RemoveRange(uiKept, masMarks.NumElements());
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Where a mark is along a track of iTrackLength: as far along it as its
// place is along the content, and as much of it, but no shorter than
// SCROLL_BAR_MARK_MINIMUM and inside the track.
//
//////////////////////////////////////////////////////////////////////////
void CScrollBarMarksDraw::GetMarkAlong(size uiIndex, int32 iTrackLength, int32* piStart, int32* piLength)
{
	SScrollBarMark*	psMark;
	int32			iStart;
	int32			iLength;

	psMark = masMarks.Get(uiIndex);
	if ((miContentLength <= 0) || (iTrackLength <= 0))
	{
		*piStart = 0;
		*piLength = 0;
		return;
	}
	iStart = (int32)(((int64)psMark->iStart * iTrackLength) / miContentLength);
	iLength = (int32)(((int64)psMark->iLength * iTrackLength) / miContentLength);
	if (iLength < SCROLL_BAR_MARK_MINIMUM)
	{
		iLength = SCROLL_BAR_MARK_MINIMUM;
	}
	if (iLength > iTrackLength)
	{
		iLength = iTrackLength;
	}
	if (iStart > iTrackLength - iLength)
	{
		iStart = iTrackLength - iLength;
	}
	if (iStart < 0)
	{
		iStart = 0;
	}
	*piStart = iStart;
	*piLength = iLength;
}


//////////////////////////////////////////////////////////////////////////
//
// Each mark, across the canvas.
//
//////////////////////////////////////////////////////////////////////////
bool CScrollBarMarksDraw::Draw(Ptr<CCanvas> pCanvas)
{
	CInt32Rect	cRect;
	SInt32Vec2	sSize;
	size		i;
	int32		iStart;
	int32		iLength;

	if (pCanvas->IsValid())
	{
		sSize = pCanvas->GetActualSize();
		for (i = 0; i < masMarks.NumElements(); i++)
		{
			//DrawBox includes the right and bottom edges of its rectangle.
			GetMarkAlong(i, mbHorizontal ? sSize.x : sSize.y, &iStart, &iLength);
			if (iLength > 0)
			{
				if (mbHorizontal)
				{
					cRect.Init(iStart, 0, iStart + iLength - 1, sSize.y - 1);
				}
				else
				{
					cRect.Init(0, iStart, sSize.x - 1, iStart + iLength - 1);
				}
				pCanvas->DrawBox(&cRect, true, masMarks.Get(i)->sColour);
			}
		}
	}
	return true;
}


size CScrollBarMarksDraw::NumMarks(void) const { return masMarks.NumElements(); }
SScrollBarMark* CScrollBarMarksDraw::GetMark(size uiIndex) { return masMarks.Get(uiIndex); }

