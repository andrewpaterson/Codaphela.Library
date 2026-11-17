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
#include <stdio.h>
#include <stdlib.h>
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "Canvas.h"
#include "Focus.h"
#include "Window.h"
#include "NumberField.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumberFieldEdit::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szText, CNumberField* pcField)
{
	PreInit();

	CTextField::Init(pWindow, pTheme, szText);
	mpcField = pcField;
	AddInputAction(CIC_Enter, this, &CNumberFieldEdit::Commit);
	AddInputAction(CIC_ClearSelection, this, &CNumberFieldEdit::Cancel);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumberFieldEdit::Class(void)
{
	CTextField::Class();

	U_Pointer(mpcField);
}


bool CNumberFieldEdit::WantsEnter(void) { return true; }
bool CNumberFieldEdit::WantsEscape(void) { return true; }
bool CNumberFieldEdit::IsTabStop(void) { return false; }  //It is reached by clicking the number, not by Tab.
void CNumberFieldEdit::Commit(void) { mpcField->EndEdit(true); }
void CNumberFieldEdit::Cancel(void) { mpcField->EndEdit(false); }


//////////////////////////////////////////////////////////////////////////
//
// Focus going elsewhere keeps what was typed.
//
//////////////////////////////////////////////////////////////////////////
void CNumberFieldEdit::FocusLost(void)
{
	CTextField::FocusLost();
	mpcField->EndEdit(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumberField::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szName, float64 fValue)
{
	Ptr<CFont>	pFont;
	int32		iPaddingX;
	int32		iPaddingY;

	PreInit();

	CFillContainer::Init(pWindow);
	mpTheme = pTheme;
	mfValue = fValue;
	mfMinimum = 0;
	mfMaximum = 0;
	mbRange = false;
	mbWhole = false;
	mbPressed = false;
	mbDragged = false;
	miPressedX = 0;
	mfPressedValue = 0;
	mcChanged.Init();

	pFont = pTheme->GetFont();
	iPaddingX = pTheme->GetMetric(TM_FieldPaddingX);
	iPaddingY = pTheme->GetMetric(TM_FieldPaddingY);

	mpBarDraw = OMalloc<CProgressBarDraw>(pTheme->GetColour(TC_Border), pTheme->GetColour(TC_Accent));
	mpBar = OMalloc<CCanvas>(pWindow, mpBarDraw);
	mpBar->SetAlignment(CSH_Stretch, CSV_Stretch);
	mpName = OMalloc<CLabel>(pWindow, pFont, szName);
	mpName->SetAlignment(CSH_Left, CSV_Center);
	mpNumber = OMalloc<CLabel>(pWindow, pFont, "");
	mpNumber->SetAlignment(CSH_Right, CSV_Center);

	SetInsets(iPaddingY, iPaddingX, iPaddingY, iPaddingX);
	SetDesiredSize(NUMBER_FIELD_COLUMNS * (int32)pFont->GetAverageWidth() + 2 * iPaddingX, pFont->GetLineHeight() + 2 * iPaddingY);
	SetAlignment(CSH_Stretch, CSV_Center);
	SetHitAsWhole(true);
	SetCanGetFocus(true);  //A drag is the focussed component's.
	SetCursor(NC_SizeWestEast);

	CFillContainer::AddComponent(mpBar);
	CFillContainer::AddComponent(mpName);
	CFillContainer::AddComponent(mpNumber);

	AddInputAction(CIC_ActivateComponent, this, &CNumberField::ActivateComponent);
	AddInputAction(CIC_ExecuteComponent, this, &CNumberField::ExecuteComponent);
	AddInputAction(CIC_ContDragSelect, this, &CNumberField::ContDragSelect);
	AddInputAction(CIC_StopDragSelect, this, &CNumberField::StopDragSelect);

	Show();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumberField::Class(void)
{
	CFillContainer::Class();

	M_Pointer(mpTheme);
	M_Pointer(mpBarDraw);
	M_Pointer(mpBar);
	M_Pointer(mpName);
	M_Pointer(mpNumber);
	M_Pointer(mpEdit);
	U_Float64(mfValue);
	U_Float64(mfMinimum);
	U_Float64(mfMaximum);
	U_Bool(mbRange);
	U_Bool(mbWhole);
	U_Bool(mbPressed);
	U_Bool(mbDragged);
	U_Int32(miPressedX);
	U_Float64(mfPressedValue);
	U_Unknown(CComponentEvent<CNumberField>, mcChanged);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumberField::Free(void)
{
	mcChanged.Kill();
	CFillContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// The value is kept between the two from now on, and the bar shows how
// far it is from the first to the second.
//
//////////////////////////////////////////////////////////////////////////
void CNumberField::SetRange(float64 fMinimum, float64 fMaximum)
{
	mfMinimum = fMinimum;
	mfMaximum = fMaximum;
	mbRange = fMaximum > fMinimum;
	SetValue(mfValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumberField::SetWhole(bool bWhole)
{
	mbWhole = bWhole;
	SetValue(mfValue);
}


//////////////////////////////////////////////////////////////////////////
//
// fValue as the field may hold it: whole, and within the range.
//
//////////////////////////////////////////////////////////////////////////
float64 CNumberField::Kept(float64 fValue)
{
	if (mbWhole)
	{
		fValue = floor(fValue + 0.5);
	}
	if (mbRange)
	{
		fValue = fmax(mfMinimum, fmin(mfMaximum, fValue));
	}
	return fValue;
}


//////////////////////////////////////////////////////////////////////////
//
// Without calling the change event.
//
//////////////////////////////////////////////////////////////////////////
void CNumberField::SetValue(float64 fValue)
{
	mfValue = Kept(fValue);
	Show();
}


//////////////////////////////////////////////////////////////////////////
//
// The number's text and the bar are what the value is.
//
//////////////////////////////////////////////////////////////////////////
void CNumberField::Show(void)
{
	char	szNumber[64];

	if (mbWhole)
	{
		snprintf(szNumber, sizeof(szNumber), "%.0f", mfValue);
	}
	else
	{
		snprintf(szNumber, sizeof(szNumber), "%.*f", NUMBER_FIELD_DECIMALS, mfValue);
	}
	mpNumber->SetText(szNumber);
	mpBarDraw->SetFraction(mbRange ? (float)((mfValue - mfMinimum) / (mfMaximum - mfMinimum)) : 0.0f);
	mpBar->Invalidate();
}


//////////////////////////////////////////////////////////////////////////
//
// The value set by dragging or typing: the change event is called if
// it's another.  True if it was.
//
//////////////////////////////////////////////////////////////////////////
bool CNumberField::Change(float64 fValue)
{
	fValue = Kept(fValue);
	if (fValue == mfValue)
	{
		return false;
	}
	mfValue = fValue;
	Show();
	mcChanged.Call(this);
	return true;
}


float64 CNumberField::GetValue(void) { return mfValue; }
char* CNumberField::GetNumberText(void) { return mpNumber->GetText(); }
CComponentEvent<CNumberField>* CNumberField::GetChangeEvent(void) { return &mcChanged; }
bool CNumberField::IsEditing(void) { return mpEdit.IsNotNull(); }
Ptr<CTextField> CNumberField::GetEdit(void) { return mpEdit; }


//////////////////////////////////////////////////////////////////////////
//
// The button's gone down on it: a click or the start of a drag.  x and y
// are relative to the field.
//
//////////////////////////////////////////////////////////////////////////
void CNumberField::ActivateComponent(int x, int y)
{
	if (IsEditing())
	{
		return;
	}
	mbPressed = true;
	mbDragged = false;
	miPressedX = x;
	mfPressedValue = mfValue;
	mpWindow->GetFocus()->StartDrag(FD_Select, x, y);
}


//////////////////////////////////////////////////////////////////////////
//
// The button's come up over it.
//
//////////////////////////////////////////////////////////////////////////
void CNumberField::ExecuteComponent(int x, int y)
{
	if (mbPressed)
	{
		mpWindow->GetFocus()->StopDrag(FD_Select);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The value is where it was pressed and what the drag from there is
// worth, so dragging back puts it back.
//
//////////////////////////////////////////////////////////////////////////
void CNumberField::ContDragSelect(int x, int y)
{
	float64		fPerPixel;
	int32		iWidth;

	if (!mbPressed)
	{
		return;
	}
	if (!mbDragged && (abs(x - miPressedX) <= NUMBER_FIELD_DRAG_PIXELS))
	{
		return;
	}
	mbDragged = true;

	iWidth = GetActualSize().x;
	if (mbRange && (iWidth > 0))
	{
		fPerPixel = (mfMaximum - mfMinimum) / iWidth;
	}
	else if (mbWhole)
	{
		fPerPixel = 1.0 / NUMBER_FIELD_INTEGER_PIXELS;
	}
	else
	{
		fPerPixel = fmax(fabs(mfPressedValue), 1.0) * NUMBER_FIELD_DRAG_SHARE;
	}
	Change(mfPressedValue + (x - miPressedX) * fPerPixel);
}


//////////////////////////////////////////////////////////////////////////
//
// Let go without having dragged, it was a click: it's typed into.
//
//////////////////////////////////////////////////////////////////////////
void CNumberField::StopDragSelect(int x, int y)
{
	bool	bClicked;

	bClicked = mbPressed && !mbDragged;
	mbPressed = false;
	mbDragged = false;
	if (bClicked)
	{
		BeginEdit();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A text field of the number, all selected and focussed, in place of the
// bar and its labels.
//
//////////////////////////////////////////////////////////////////////////
void CNumberField::BeginEdit(void)
{
	if (IsEditing())
	{
		return;
	}
	mpEdit = OMalloc<CNumberFieldEdit>(mpWindow, mpTheme, mpNumber->GetText(), this);
	mpEdit->SetAlignment(CSH_Stretch, CSV_Stretch);
	SetHitAsWhole(false);
	CFillContainer::RemoveAllComponents();
	CFillContainer::AddComponent(mpEdit);
	mpEdit->SelectAll();
	mpWindow->GetFocus()->SetFocus(mpEdit);
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// The bar and its labels again.  bCommit takes what was typed if all of
// it is a number; otherwise the number is what it was.
//
//////////////////////////////////////////////////////////////////////////
void CNumberField::EndEdit(bool bCommit)
{
	Ptr<CNumberFieldEdit>	pEdit;
	char*					szText;
	char*					szEnd;
	float64					fValue;
	bool					bNumber;

	if (!IsEditing())
	{
		return;
	}

	//The edit is let go of first: taking it out loses it the focus, which ends the edit again.
	pEdit = mpEdit;
	mpEdit = NULL;

	szText = pEdit->GetText();
	fValue = strtod(szText, &szEnd);
	while ((*szEnd == ' ') || (*szEnd == '\t'))
	{
		szEnd++;
	}
	bNumber = (szEnd != szText) && (*szEnd == '\0') && isfinite(fValue);

	CFillContainer::RemoveAllComponents();
	CFillContainer::AddComponent(mpBar);
	CFillContainer::AddComponent(mpName);
	CFillContainer::AddComponent(mpNumber);
	SetHitAsWhole(true);
	InvalidateLayout();

	if (bCommit && bNumber)
	{
		Change(fValue);
	}
}

