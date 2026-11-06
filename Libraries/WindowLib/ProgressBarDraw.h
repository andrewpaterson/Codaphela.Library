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
#ifndef __PROGRESS_BAR_DRAW_H__
#define __PROGRESS_BAR_DRAW_H__
#include "SupportLib/ColourARGB32.h"
#include "CanvasDraw.h"


//Draws a progress bar: the whole canvas in the track colour, then the part done, from the left, in the bar colour.
class CProgressBarDraw : public CCanvasDraw
{
CONSTRUCTABLE(CProgressBarDraw);
DESTRUCTABLE(CProgressBarDraw);
protected:
	ARGB32	msTrack;
	ARGB32	msBar;
	float	mfFraction;  //How much is done, 0 to 1.

public:
	void	Init(ARGB32 sTrack, ARGB32 sBar);
	void	Class(void) override;

	bool	Draw(Ptr<CCanvas> pCanvas) override;

	void	SetFraction(float fFraction);
	float	GetFraction(void);
	void	SetBarColour(ARGB32 sBar);
	ARGB32	GetBarColour(void);
	void	SetTrackColour(ARGB32 sTrack);
	ARGB32	GetTrackColour(void);
	int32	GetBarWidth(int32 iWidth);
};


#endif // __PROGRESS_BAR_DRAW_H__

