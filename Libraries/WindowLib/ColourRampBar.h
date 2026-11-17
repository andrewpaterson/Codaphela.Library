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
#ifndef __COLOUR_RAMP_BAR_H__
#define __COLOUR_RAMP_BAR_H__
#include "BaseLib/ArrayTemplate.h"
#include "Button.h"
#include "CelCanvasDraw.h"
#include "ColourField.h"
#include "ComponentEvent.h"
#include "GridContainer.h"
#include "NumberField.h"
#include "WindowTheme.h"


#define COLOUR_RAMP_BAR_HEIGHT		22		//Of the bar of colours.
#define COLOUR_RAMP_MARKER_HEIGHT	9		//Of a stop's marker under the bar,
#define COLOUR_RAMP_MARKER_WIDTH	5		//and half how wide it is.
#define COLOUR_RAMP_MARKER_REACH	7		//How far from a marker's middle a press still takes hold of it.


class CColourRampBar;


//A stop of a colour ramp: a colour at a place along it.  iData is whoever's ramp it is's to use: a stop the bar adds has
//-1.
struct SColourRampStop
{
	float32		fPlace;  //0 at the left end to 1 at the right.
	float32		afColour[3];  //Red, green and blue, 0 to 1.
	int32		iData;
};

typedef CArrayTemplate<SColourRampStop>	CArrayColourRampStop;


//Paints a colour ramp's bar: the ramp's colours from left to right, each column a blend of the stops either side of it,
//and under it each stop's marker, the chosen one's in the accent colour.
class CColourRampBarDraw : public CCelCanvasDraw
{
CONSTRUCTABLE(CColourRampBarDraw);
DESTRUCTABLE(CColourRampBarDraw);
protected:
	CColourRampBar*		mpcRamp;  //Not held: it holds this.

public:
	void	Init(CColourRampBar* pcRamp);
	void	Class(void) override;

protected:
	void	Paint(CImageDraw* pcDraw, SInt32Vec2 sSize) override;
};


//A colour ramp, as Blender's Color Ramp is: a bar of the ramp's colours with its stops as markers under it; + and - to
//add a stop and remove the chosen one; and the chosen stop's place and colour.
//
//A marker pressed is the chosen stop, and dragged along the bar it's moved; the stops are kept in the order of their
//places, a stop dragged past another changing places with it.  + adds a stop half way between the chosen one and the
//next (or the last, for the last), the ramp's colour there.  - removes the chosen stop, unless it's the only one.
//
//Its change event is called whenever a stop is moved, added, removed or recoloured with it; its choice event whenever
//another stop is chosen.
class CColourRampBar : public CGridContainer
{
CONSTRUCTABLE(CColourRampBar);
DESTRUCTABLE(CColourRampBar);
protected:
	Ptr<CWindowTheme>				mpTheme;
	Ptr<CColourRampBarDraw>			mpBarDraw;
	Ptr<CCanvas>					mpBar;
	Ptr<CButton>					mpAdd;
	Ptr<CButton>					mpRemove;
	Ptr<CNumberField>				mpPlace;
	Ptr<CColourField>				mpColour;
	CArrayColourRampStop			masStops;  //In the order of their places.
	int32							miChosen;  //-1 if there are none.
	bool							mbDragging;
	bool							mbShowing;  //The place and colour are being set to the chosen stop's: their changes aren't the user's.
	CComponentEvent<CColourRampBar>	mcChanged;
	CComponentEvent<CColourRampBar>	mcChosen;

public:
	void								Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme);
	void								Class(void) override;
	void								Free(void) override;

	int32								AddStop(float32 fPlace, float32 fRed, float32 fGreen, float32 fBlue, int32 iData);
	void								RemoveAllStops(void);
	size								NumStops(void) const;
	SColourRampStop*					GetStop(size uiIndex);
	void								Choose(int32 iStop);
	int32								GetChosen(void) const;
	void								At(float32 fPlace, float32* pafColour);
	CComponentEvent<CColourRampBar>*	GetChangeEvent(void);
	CComponentEvent<CColourRampBar>*	GetChoiceEvent(void);
	Ptr<CCanvas>						GetBar(void) const;
	Ptr<CNumberField>					GetPlace(void) const;
	Ptr<CColourField>					GetColour(void) const;
	Ptr<CWindowTheme>					GetTheme(void) const;

	void								BarPressed(int x, int y);
	void								BarLetGo(int x, int y);
	void								BarDragged(int x, int y);
	void								BarReleased(int x, int y);
	void								AddClicked(CButton* pcButton);
	void								RemoveClicked(CButton* pcButton);
	void								PlaceChanged(CNumberField* pcPlace);
	void								ColourChanged(CColourField* pcColour);

protected:
	int32								MoveStop(int32 iStop, float32 fPlace);
	void								Show(void);
	void								Changed(void);
};


#endif // __COLOUR_RAMP_BAR_H__

