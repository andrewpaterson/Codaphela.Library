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
#include <math.h>
#include <stdlib.h>
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ColourARGB32.h"
#include "Focus.h"
#include "Window.h"
#include "ColourRampBar.h"


//////////////////////////////////////////////////////////////////////////
//
// A colour's channels, 0 to 1, as a colour.
//
//////////////////////////////////////////////////////////////////////////
static ARGB32 ColourRampColour(float32* pafColour)
{
	return Set32BitColour(fmaxf(0.0f, fminf(1.0f, pafColour[0])), fmaxf(0.0f, fminf(1.0f, pafColour[1])), fmaxf(0.0f, fminf(1.0f, pafColour[2])));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CColourRampBarDraw::Init(CColourRampBar* pcRamp)
{
	PreInit();

	CCelCanvasDraw::Init();
	mpcRamp = pcRamp;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CColourRampBarDraw::Class(void)
{
	CCelCanvasDraw::Class();

	U_Pointer(mpcRamp);
}


//////////////////////////////////////////////////////////////////////////
//
// The bar is the canvas less a marker's half width each side, so the
// end stops' markers are within it.  Each marker is a triangle pointing
// up at its place.
//
//////////////////////////////////////////////////////////////////////////
void CColourRampBarDraw::Paint(CImageDraw* pcDraw, SInt32Vec2 sSize)
{
	Ptr<CWindowTheme>	pTheme;
	SColourRampStop*	psStop;
	SFloat64Vec2		asMarker[3];
	float32				afColour[3];
	float64				fMiddle;
	int32				iWidth;
	int32				iBarBottom;
	int32				x;
	size				i;

	pTheme = mpcRamp->GetTheme();
	iWidth = sSize.x - 2 * COLOUR_RAMP_MARKER_WIDTH;
	iBarBottom = sSize.y - COLOUR_RAMP_MARKER_HEIGHT;
	if ((iWidth <= 0) || (iBarBottom <= 0))
	{
		return;
	}

	for (x = 0; x < iWidth; x++)
	{
		mpcRamp->At(((float32)x + 0.5f) / (float32)iWidth, afColour);
		pcDraw->SetColour(ColourRampColour(afColour));
		pcDraw->DrawRectangle(COLOUR_RAMP_MARKER_WIDTH + x, 0, COLOUR_RAMP_MARKER_WIDTH + x + 1, iBarBottom);
	}

	for (i = 0; i < mpcRamp->NumStops(); i++)
	{
		psStop = mpcRamp->GetStop(i);
		fMiddle = COLOUR_RAMP_MARKER_WIDTH + psStop->fPlace * iWidth;
		asMarker[0].Init(fMiddle, iBarBottom);
		asMarker[1].Init(fMiddle + COLOUR_RAMP_MARKER_WIDTH, sSize.y);
		asMarker[2].Init(fMiddle - COLOUR_RAMP_MARKER_WIDTH, sSize.y);
		pcDraw->SetColour(pTheme->GetColour(((int32)i == mpcRamp->GetChosen()) ? TC_Accent : TC_Text));
		pcDraw->DrawPolygon(asMarker, 3);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// With no stops: whoever's ramp it is adds them (AddStop).
//
//////////////////////////////////////////////////////////////////////////
void CColourRampBar::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme)
{
	PreInit();

	CGridContainer::Init(pWindow);
	mpTheme = pTheme;
	masStops.Init();
	miChosen = -1;
	mbDragging = false;
	mbShowing = false;
	mcChanged.Init();
	mcChosen.Init();

	mpBarDraw = OMalloc<CColourRampBarDraw>(this);
	mpBar = OMalloc<CCanvas>(pWindow, mpBarDraw);
	mpBar->SetTransparent(true);
	mpBar->SetDesiredSize(-1, COLOUR_RAMP_BAR_HEIGHT + COLOUR_RAMP_MARKER_HEIGHT);
	mpBar->SetAlignment(CSH_Stretch, CSV_Center);
	mpBar->SetCanGetFocus(true);
	mpBar->AddInputAction(CIC_ActivateComponent, this, &CColourRampBar::BarPressed);
	mpBar->AddInputAction(CIC_ExecuteComponent, this, &CColourRampBar::BarLetGo);
	mpBar->AddInputAction(CIC_ContDragSelect, this, &CColourRampBar::BarDragged);
	mpBar->AddInputAction(CIC_StopDragSelect, this, &CColourRampBar::BarReleased);

	mpAdd = OMalloc<CButton>(pWindow, pTheme, "+");
	mpAdd->GetClickEvent()->Add(this, &CColourRampBar::AddClicked);
	mpRemove = OMalloc<CButton>(pWindow, pTheme, "-");
	mpRemove->GetClickEvent()->Add(this, &CColourRampBar::RemoveClicked);
	mpPlace = OMalloc<CNumberField>(pWindow, pTheme, "Pos", 0.0);
	mpPlace->SetRange(0.0, 1.0);
	mpPlace->GetChangeEvent()->Add(this, &CColourRampBar::PlaceChanged);
	mpColour = OMalloc<CColourField>(pWindow, pTheme, "Colour", 0.0f, 0.0f, 0.0f);
	mpColour->GetChangeEvent()->Add(this, &CColourRampBar::ColourChanged);

	AddComponent(mpBar, 0, 0, 3, 1, 1, 0);
	AddComponent(mpAdd, 0, 1, 1, 1, 0, 0);
	AddComponent(mpRemove, 1, 1, 1, 1, 0, 0);
	AddComponent(mpPlace, 2, 1, 1, 1, 1, 0);
	AddComponent(mpColour, 0, 2, 3, 1, 1, 0);
	SetAlignment(CSH_Stretch, CSV_Center);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CColourRampBar::Class(void)
{
	CGridContainer::Class();

	M_Pointer(mpTheme);
	M_Pointer(mpBarDraw);
	M_Pointer(mpBar);
	M_Pointer(mpAdd);
	M_Pointer(mpRemove);
	M_Pointer(mpPlace);
	M_Pointer(mpColour);
	U_Unknown(CArrayColourRampStop, masStops);
	U_Int32(miChosen);
	U_Bool(mbDragging);
	U_Bool(mbShowing);
	U_Unknown(CComponentEvent<CColourRampBar>, mcChanged);
	U_Unknown(CComponentEvent<CColourRampBar>, mcChosen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CColourRampBar::Free(void)
{
	mcChosen.Kill();
	mcChanged.Kill();
	masStops.Kill();
	CGridContainer::Free();
}


size CColourRampBar::NumStops(void) const { return masStops.NumElements(); }
SColourRampStop* CColourRampBar::GetStop(size uiIndex) { return masStops.SafeGet(uiIndex); }
int32 CColourRampBar::GetChosen(void) const { return miChosen; }
CComponentEvent<CColourRampBar>* CColourRampBar::GetChangeEvent(void) { return &mcChanged; }
CComponentEvent<CColourRampBar>* CColourRampBar::GetChoiceEvent(void) { return &mcChosen; }
Ptr<CCanvas> CColourRampBar::GetBar(void) const { return mpBar; }
Ptr<CNumberField> CColourRampBar::GetPlace(void) const { return mpPlace; }
Ptr<CColourField> CColourRampBar::GetColour(void) const { return mpColour; }
Ptr<CWindowTheme> CColourRampBar::GetTheme(void) const { return mpTheme; }


//////////////////////////////////////////////////////////////////////////
//
// A stop, put where its place is among the others (after any at the same
// place).  Returns its index.  The first added is the chosen one.  No
// event is called.
//
//////////////////////////////////////////////////////////////////////////
int32 CColourRampBar::AddStop(float32 fPlace, float32 fRed, float32 fGreen, float32 fBlue, int32 iData)
{
	SColourRampStop*	psStop;
	size				uiIndex;

	uiIndex = masStops.NumElements();
	while ((uiIndex > 0) && (masStops.Get(uiIndex - 1)->fPlace > fPlace))
	{
		uiIndex--;
	}
	psStop = masStops.InsertAt(uiIndex);
	psStop->fPlace = fPlace;
	psStop->afColour[0] = fRed;
	psStop->afColour[1] = fGreen;
	psStop->afColour[2] = fBlue;
	psStop->iData = iData;

	if (miChosen < 0)
	{
		miChosen = (int32)uiIndex;
	}
	else if ((int32)uiIndex <= miChosen)
	{
		miChosen++;
	}
	Show();
	return (int32)uiIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CColourRampBar::RemoveAllStops(void)
{
	masStops.ReInit();
	miChosen = -1;
	Show();
}


//////////////////////////////////////////////////////////////////////////
//
// The stop is the chosen one.  The choice event is called if it's
// another.
//
//////////////////////////////////////////////////////////////////////////
void CColourRampBar::Choose(int32 iStop)
{
	if ((iStop >= 0) && (iStop < (int32)masStops.NumElements()) && (iStop != miChosen))
	{
		miChosen = iStop;
		Show();
		mcChosen.Call(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The ramp's colour at a place: the stop's there, a blend of the two
// either side of it, or the end stop's beyond the ends.  Black with no
// stops.
//
//////////////////////////////////////////////////////////////////////////
void CColourRampBar::At(float32 fPlace, float32* pafColour)
{
	SColourRampStop*	psBefore;
	SColourRampStop*	psAfter;
	float32				fAlong;
	size				uiAfter;
	size				i;

	if (masStops.NumElements() == 0)
	{
		pafColour[0] = pafColour[1] = pafColour[2] = 0.0f;
		return;
	}
	uiAfter = 0;
	while ((uiAfter < masStops.NumElements()) && (masStops.Get(uiAfter)->fPlace <= fPlace))
	{
		uiAfter++;
	}
	psBefore = masStops.Get((uiAfter > 0) ? uiAfter - 1 : 0);
	psAfter = masStops.Get((uiAfter < masStops.NumElements()) ? uiAfter : masStops.NumElements() - 1);
	fAlong = (psAfter->fPlace > psBefore->fPlace) ? (fPlace - psBefore->fPlace) / (psAfter->fPlace - psBefore->fPlace) : 0.0f;
	fAlong = fmaxf(0.0f, fminf(1.0f, fAlong));
	for (i = 0; i < 3; i++)
	{
		pafColour[i] = psBefore->afColour[i] + (psAfter->afColour[i] - psBefore->afColour[i]) * fAlong;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The place and colour fields are the chosen stop's, and the bar is
// painted again.
//
//////////////////////////////////////////////////////////////////////////
void CColourRampBar::Show(void)
{
	SColourRampStop*	psStop;

	psStop = masStops.SafeGet((size)miChosen);
	if ((miChosen >= 0) && (psStop != NULL))
	{
		mbShowing = true;
		mpPlace->SetValue(psStop->fPlace);
		mpColour->SetColour(psStop->afColour[0], psStop->afColour[1], psStop->afColour[2]);
		mbShowing = false;
	}
	mpBarDraw->Repaint();
	mpBar->Invalidate();
}


//////////////////////////////////////////////////////////////////////////
//
// The ramp is another: it's shown, and the change event called.
//
//////////////////////////////////////////////////////////////////////////
void CColourRampBar::Changed(void)
{
	Show();
	mcChanged.Call(this);
}


//////////////////////////////////////////////////////////////////////////
//
// The stop is put at fPlace, kept to 0 to 1, changing places with those
// it passes.  Returns where it now is among the stops.
//
//////////////////////////////////////////////////////////////////////////
int32 CColourRampBar::MoveStop(int32 iStop, float32 fPlace)
{
	SColourRampStop		sStop;

	masStops.Get(iStop)->fPlace = fmaxf(0.0f, fminf(1.0f, fPlace));
	while ((iStop > 0) && (masStops.Get(iStop - 1)->fPlace > masStops.Get(iStop)->fPlace))
	{
		sStop = *masStops.Get(iStop);
		*masStops.Get(iStop) = *masStops.Get(iStop - 1);
		*masStops.Get(iStop - 1) = sStop;
		iStop--;
	}
	while ((iStop + 1 < (int32)masStops.NumElements()) && (masStops.Get(iStop + 1)->fPlace < masStops.Get(iStop)->fPlace))
	{
		sStop = *masStops.Get(iStop);
		*masStops.Get(iStop) = *masStops.Get(iStop + 1);
		*masStops.Get(iStop + 1) = sStop;
		iStop++;
	}
	return iStop;
}


//////////////////////////////////////////////////////////////////////////
//
// The bar's pressed at x, y in it: the stop whose marker is nearest, if
// it's near enough, is chosen and taken hold of.
//
//////////////////////////////////////////////////////////////////////////
void CColourRampBar::BarPressed(int x, int y)
{
	int32	iWidth;
	int32	iNearest;
	int32	iDistance;
	int32	iMarker;
	size	i;

	iWidth = mpBar->GetActualSize().x - 2 * COLOUR_RAMP_MARKER_WIDTH;
	iNearest = -1;
	iDistance = COLOUR_RAMP_MARKER_REACH + 1;
	for (i = 0; i < masStops.NumElements(); i++)
	{
		iMarker = COLOUR_RAMP_MARKER_WIDTH + (int32)(masStops.Get(i)->fPlace * iWidth);
		if (abs(x - iMarker) < iDistance)
		{
			iDistance = abs(x - iMarker);
			iNearest = (int32)i;
		}
	}
	if (iNearest >= 0)
	{
		Choose(iNearest);
		mpWindow->GetFocus()->StartDrag(FD_Select, x, y);  //Which stops any drag there was, so only then is it dragging.
		mbDragging = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The button's come up over the bar.
//
//////////////////////////////////////////////////////////////////////////
void CColourRampBar::BarLetGo(int x, int y)
{
	if (mbDragging)
	{
		mpWindow->GetFocus()->StopDrag(FD_Select);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The chosen stop goes where the pointer is along the bar.
//
//////////////////////////////////////////////////////////////////////////
void CColourRampBar::BarDragged(int x, int y)
{
	int32	iWidth;
	float32	fPlace;

	iWidth = mpBar->GetActualSize().x - 2 * COLOUR_RAMP_MARKER_WIDTH;
	if (!mbDragging || (miChosen < 0) || (iWidth <= 0))
	{
		return;
	}
	fPlace = fmaxf(0.0f, fminf(1.0f, (float32)(x - COLOUR_RAMP_MARKER_WIDTH) / (float32)iWidth));
	if (fPlace != masStops.Get(miChosen)->fPlace)
	{
		miChosen = MoveStop(miChosen, fPlace);
		Changed();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CColourRampBar::BarReleased(int x, int y)
{
	mbDragging = false;
}


//////////////////////////////////////////////////////////////////////////
//
// A stop half way from the chosen one to the next (to the last, from
// the last), the ramp's colour there, which is then the chosen one.
//
//////////////////////////////////////////////////////////////////////////
void CColourRampBar::AddClicked(CButton* pcButton)
{
	float32		afColour[3];
	float32		fPlace;
	float32		fOther;

	if (miChosen < 0)
	{
		miChosen = AddStop(0.5f, 0.5f, 0.5f, 0.5f, -1);
	}
	else
	{
		fPlace = masStops.Get(miChosen)->fPlace;
		if (miChosen + 1 < (int32)masStops.NumElements())
		{
			fOther = masStops.Get(miChosen + 1)->fPlace;
		}
		else
		{
			fOther = (miChosen > 0) ? masStops.Get(miChosen - 1)->fPlace : 1.0f - fPlace;
		}
		fPlace = (fPlace + fOther) * 0.5f;
		At(fPlace, afColour);
		miChosen = -1;
		miChosen = AddStop(fPlace, afColour[0], afColour[1], afColour[2], -1);
	}
	mcChosen.Call(this);
	Changed();
}


//////////////////////////////////////////////////////////////////////////
//
// The chosen stop goes, unless it's the only one, and the one before it
// (or the first) is chosen.
//
//////////////////////////////////////////////////////////////////////////
void CColourRampBar::RemoveClicked(CButton* pcButton)
{
	if ((miChosen < 0) || (masStops.NumElements() <= 1))
	{
		return;
	}
	masStops.RemoveAt((size)miChosen);
	miChosen = (miChosen > 0) ? miChosen - 1 : 0;
	mcChosen.Call(this);
	Changed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CColourRampBar::PlaceChanged(CNumberField* pcPlace)
{
	if (!mbShowing && (miChosen >= 0))
	{
		miChosen = MoveStop(miChosen, (float32)pcPlace->GetValue());
		Changed();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CColourRampBar::ColourChanged(CColourField* pcColour)
{
	SColourRampStop*	psStop;
	float32*			pafColour;

	if (!mbShowing && (miChosen >= 0))
	{
		psStop = masStops.Get(miChosen);
		pafColour = pcColour->GetColour();
		psStop->afColour[0] = pafColour[0];
		psStop->afColour[1] = pafColour[1];
		psStop->afColour[2] = pafColour[2];
		Changed();
	}
}

