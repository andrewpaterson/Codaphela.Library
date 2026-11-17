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
#ifndef __COLOUR_FIELD_H__
#define __COLOUR_FIELD_H__
#include "Canvas.h"
#include "ClearCanvasDraw.h"
#include "ComponentEvent.h"
#include "GridContainer.h"
#include "Label.h"
#include "NumberField.h"
#include "TextField.h"
#include "WindowTheme.h"


#define COLOUR_PICKER_WIDTH		180		//Its number fields'.
#define COLOUR_FIELD_SWATCH		0.5f	//Of a colour field's width its swatch is, the name having the rest.


//A colour chosen by its numbers, as the fields of Blender's picker are (without its wheel): red, green and blue; hue,
//saturation and value; and hex (RRGGBB), each 0 to 1 but the hex.  Changing one sets the others.  A grey has no hue, so
//the hue last chosen is kept while the colour is grey rather than snapping to red.
//
//Its change event is called whenever the colour is changed by its fields, not when SetColour sets it.
class CColourPicker : public CGridContainer
{
CONSTRUCTABLE(CColourPicker);
DESTRUCTABLE(CColourPicker);
protected:
	Ptr<CNumberField>				mpRed;
	Ptr<CNumberField>				mpGreen;
	Ptr<CNumberField>				mpBlue;
	Ptr<CNumberField>				mpHue;
	Ptr<CNumberField>				mpSaturation;
	Ptr<CNumberField>				mpValue;
	Ptr<CTextField>					mpHex;
	float32							mafColour[3];  //Red, green and blue.
	float32							mfHue;  //Kept while the colour is a grey.
	bool							mbShowing;  //The fields are being set to the colour: their changes aren't the user's.
	CComponentEvent<CColourPicker>	mcChanged;

public:
	void							Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, float32 fRed, float32 fGreen, float32 fBlue);
	void							Class(void) override;
	void							Free(void) override;

	void							SetColour(float32 fRed, float32 fGreen, float32 fBlue);
	float32*						GetColour(void);
	CComponentEvent<CColourPicker>*	GetChangeEvent(void);
	Ptr<CNumberField>				GetChannel(size uiChannel);  //0 to 5: red, green, blue, hue, saturation, value.
	Ptr<CTextField>					GetHex(void);

	void							RGBChanged(CNumberField* pcNumber);
	void							HSVChanged(CNumberField* pcNumber);
	void							HexChanged(CTextField* pcHex);

protected:
	Ptr<CNumberField>				AddChannel(Ptr<CWindowTheme> pTheme, const char* szName, int32 iRow, bool bRGB);
	void							Show(bool bHex);
	void							Change(float32 fRed, float32 fGreen, float32 fBlue, bool bHex);
};


//A named colour, shown as a swatch of it beside its name.  Pressed, it opens a picker (CColourPicker) under itself as a
//popup, and closes it if it's open; a press anywhere but in the picker closes it too.  Its change event is called
//whenever the picker changes the colour, not when SetColour sets it.
class CColourField : public CGridContainer
{
CONSTRUCTABLE(CColourField);
DESTRUCTABLE(CColourField);
protected:
	Ptr<CWindowTheme>				mpTheme;
	Ptr<CLabel>						mpName;
	Ptr<CClearCanvasDraw>			mpSwatchDraw;
	Ptr<CCanvas>					mpSwatch;
	Ptr<CColourPicker>				mpPicker;  //While it's open, NULL otherwise.
	float32							mafColour[3];  //Red, green and blue, 0 to 1.
	CComponentEvent<CColourField>	mcChanged;

public:
	void							Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szName, float32 fRed, float32 fGreen, float32 fBlue);
	void							Class(void) override;
	void							Free(void) override;

	void							SetColour(float32 fRed, float32 fGreen, float32 fBlue);
	float32*						GetColour(void);
	CComponentEvent<CColourField>*	GetChangeEvent(void);

	void							ActivateComponent(int x, int y);
	void							OpenPicker(void);
	void							ClosePicker(void);
	Ptr<CColourPicker>				GetPicker(void);
	void							PopupClosed(Ptr<CComponent> pContent) override;
	void							PickerChanged(CColourPicker* pcPicker);

protected:
	void							Show(void);
};


#endif // __COLOUR_FIELD_H__

