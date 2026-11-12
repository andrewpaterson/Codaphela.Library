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
#include "BandsCanvasDraw.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBandsCanvasDraw::Init(ARGB32 sColour)
{
	PreInit();
	CCanvasDraw::Init();
	msColour = sColour;
	masBands.Init();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBandsCanvasDraw::Class(void)
{
	CCanvasDraw::Class();
	U_UInt32(msColour);
	U_Unknown(CArrayInt32Vec2, masBands);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBandsCanvasDraw::Free(void)
{
	masBands.Kill();
	CCanvasDraw::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// Each band, across the canvas.
//
//////////////////////////////////////////////////////////////////////////
bool CBandsCanvasDraw::Draw(Ptr<CCanvas> pCanvas)
{
	CInt32Rect	cRect;
	SInt32Vec2*	psBand;
	size		i;

	if (pCanvas->IsValid())
	{
		for (i = 0; i < masBands.NumElements(); i++)
		{
			//DrawBox includes the right and bottom edges of its rectangle.
			psBand = masBands.Get(i);
			if (psBand->y > 0)
			{
				cRect.Init(0, psBand->x, pCanvas->GetActualSize().x - 1, psBand->x + psBand->y - 1);
				pCanvas->DrawBox(&cRect, true, msColour);
			}
		}
	}
	return true;
}


void CBandsCanvasDraw::SetColour(ARGB32 sColour) { msColour = sColour; }
ARGB32 CBandsCanvasDraw::GetColour(void) { return msColour; }
void CBandsCanvasDraw::ClearBands(void) { masBands.ReInit(); }
void CBandsCanvasDraw::AddBand(int32 iTop, int32 iHeight) { masBands.Add()->Init(iTop, iHeight); }
size CBandsCanvasDraw::NumBands(void) { return masBands.NumElements(); }
SInt32Vec2 CBandsCanvasDraw::GetBand(size uiIndex) { return *masBands.Get(uiIndex); }

