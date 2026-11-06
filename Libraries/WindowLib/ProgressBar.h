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
#ifndef __PROGRESS_BAR_H__
#define __PROGRESS_BAR_H__
#include "Canvas.h"
#include "ProgressBarDraw.h"


//A thin bar that fills from the left as something is done, in a colour that can change (to say how it went, say).
//It is iHeight pixels high and as wide as it is laid out: give it CSH_Stretch in its container to fill the width.
class CProgressBar : public CCanvas
{
CONSTRUCTABLE(CProgressBar);
DESTRUCTABLE(CProgressBar);
protected:
	Ptr<CProgressBarDraw>	mpDraw;

public:
	void		Init(Ptr<CWindow> pWindow, ARGB32 sTrack, ARGB32 sBar, int32 iHeight);
	void		Class(void) override;
	void 		Free(void) override;

	bool		Save(CObjectWriter* pcFile) override;
	bool		Load(CObjectReader* pcFile) override;

	void		SetFraction(float fFraction);
	void		SetProgress(int64 iDone, int64 iTotal);
	float		GetFraction(void);
	void		SetBarColour(ARGB32 sBar);
	ARGB32		GetBarColour(void);
	void		SetTrackColour(ARGB32 sTrack);
	ARGB32		GetTrackColour(void);
	int32		GetBarWidth(void);
};


#endif // __PROGRESS_BAR_H__

