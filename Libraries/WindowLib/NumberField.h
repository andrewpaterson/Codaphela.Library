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
#ifndef __NUMBER_FIELD_H__
#define __NUMBER_FIELD_H__
#include "ComponentEvent.h"
#include "FillContainer.h"
#include "Label.h"
#include "ProgressBarDraw.h"
#include "TextField.h"
#include "WindowTheme.h"


#define NUMBER_FIELD_DRAG_PIXELS	2		//How far the pointer moves from where it was pressed before it's a drag, not a click.
#define NUMBER_FIELD_DRAG_SHARE		0.01	//Of the number's size a pixel's drag changes one with no range.
#define NUMBER_FIELD_INTEGER_PIXELS	8		//A whole number changes by one for this many pixels' drag.
#define NUMBER_FIELD_DECIMALS		3		//Shown of a number that isn't whole.
#define NUMBER_FIELD_COLUMNS		14		//Its width, in the font's average characters, unless a desired width is set.


class CNumberField;


//The text field a number is typed into: enter and escape are its own and end the typing, as does focus going elsewhere.
class CNumberFieldEdit : public CTextField
{
CONSTRUCTABLE(CNumberFieldEdit);
DESTRUCTABLE(CNumberFieldEdit);
protected:
	CNumberField*	mpcField;  //Not held: it holds this.

public:
	void	Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szText, CNumberField* pcField);
	void	Class(void) override;

	bool	WantsEnter(void) override;
	bool	WantsEscape(void) override;
	bool	IsTabStop(void) const override;
	void	FocusLost(void) override;
	void	Commit(void);
	void	Cancel(void);
};


//A number with a name, as Blender's number fields are: the name on the left and the number on the right of a bar.
//
//Dragged sideways the number changes: one with a range (SetRange) goes from one end of the range to the other across the
//field's width and the bar shows how far along it is; one with none changes by a hundredth of its size for each pixel (a
//number near nothing by a hundredth), a whole number (SetWhole) by one every few pixels.
//
//Clicked (pressed and let go without dragging) it's typed into: the bar and its labels give way to a text field holding
//the number, all of it selected.  Enter, or focus going elsewhere, takes what was typed if it's a number, kept to the
//range; Escape, or anything that isn't a number, puts back the number there was.
//
//Its change event is called whenever the number changes by dragging or typing, not when SetValue sets it.
class CNumberField : public CFillContainer
{
CONSTRUCTABLE(CNumberField);
DESTRUCTABLE(CNumberField);
protected:
	Ptr<CWindowTheme>				mpTheme;
	Ptr<CProgressBarDraw>			mpBarDraw;
	Ptr<CCanvas>					mpBar;
	Ptr<CLabel>						mpName;
	Ptr<CLabel>						mpNumber;
	Ptr<CNumberFieldEdit>			mpEdit;  //While it's typed into, NULL otherwise.
	float64							mfValue;
	float64							mfMinimum;
	float64							mfMaximum;
	bool							mbRange;  //The value is kept between mfMinimum and mfMaximum.
	bool							mbWhole;  //The value is a whole number.
	bool							mbPressed;  //The button is down on it.
	bool							mbDragged;  //And the pointer has moved far enough to be dragging.
	int32							miPressedX;
	float64							mfPressedValue;
	CComponentEvent<CNumberField>	mcChanged;

public:
	void							Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szName, float64 fValue);
	void							Class(void) override;
	void							Free(void) override;

	void							SetRange(float64 fMinimum, float64 fMaximum);
	void							SetWhole(bool bWhole);
	void							SetValue(float64 fValue);
	float64							GetValue(void) const;
	char*							GetNumberText(void);
	CComponentEvent<CNumberField>*	GetChangeEvent(void);

	void							ActivateComponent(int x, int y);
	void							ExecuteComponent(int x, int y);
	void							ContDragSelect(int x, int y);
	void							StopDragSelect(int x, int y);

	void							BeginEdit(void);
	void							EndEdit(bool bCommit);
	bool							IsEditing(void) const;
	Ptr<CTextField>					GetEdit(void) const;

protected:
	float64							Kept(float64 fValue);
	void							Show(void);
	bool							Change(float64 fValue);
};


#endif // __NUMBER_FIELD_H__

