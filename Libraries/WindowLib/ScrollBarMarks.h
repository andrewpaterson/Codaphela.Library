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
#ifndef __SCROLL_BAR_MARKS_H__
#define __SCROLL_BAR_MARKS_H__
#include "BaseLib/ArrayTemplate.h"
#include "SupportLib/ColourARGB32.h"
#include "CanvasDraw.h"


#define SCROLL_BAR_MARK_MINIMUM		3	//The shortest a mark is drawn along the track, so a line of a long text can be seen.


//A place in what a scroll pane scrolls that its scroll bar marks (CScrollBar::SetMarks): a search's match, a changed
//line, a bookmark.  Its start and length are along the content, in the content's pixels.
struct SScrollBarMark
{
	int32	iStart;
	int32	iLength;
	ARGB32	sColour;
	uint16	uiSource;  //Whose it is: each source's marks are set and cleared together.
};


typedef CArrayTemplate<SScrollBarMark>	CArrayScrollBarMarks;


//Draws a scroll bar's marks on a canvas laid over its track, as IntelliJ IDEA's stripe beside the scroll bar: each
//mark a band across the track where its place is along the content, at least SCROLL_BAR_MARK_MINIMUM long.  The marks
//of the sources set later are drawn over the earlier ones'.  The canvas should be transparent.
class CScrollBarMarksDraw : public CCanvasDraw
{
CONSTRUCTABLE(CScrollBarMarksDraw);
DESTRUCTABLE(CScrollBarMarksDraw);
protected:
	CArrayScrollBarMarks	masMarks;
	int32					miContentLength;
	bool					mbHorizontal;

public:
	void	Init(bool bHorizontal);
	void	Class(void) override;
	void	Free(void) override;

	bool	Draw(Ptr<CCanvas> pCanvas) override;

	bool			SetContentLength(int32 iContentLength);
	void			SetMarks(uint16 uiSource, SInt32Vec2* asMarks, size uiNumMarks, ARGB32 sColour);
	bool			ClearMarks(uint16 uiSource);
	size			NumMarks(void) const;
	SScrollBarMark*	GetMark(size uiIndex);
	void			GetMarkAlong(size uiIndex, int32 iTrackLength, int32* piStart, int32* piLength);
};


#endif // __SCROLL_BAR_MARKS_H__

