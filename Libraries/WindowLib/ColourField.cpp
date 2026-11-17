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
#include <stdio.h>
#include <string.h>
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ColourARGB32.h"
#include "Popup.h"
#include "Window.h"
#include "ColourField.h"


//////////////////////////////////////////////////////////////////////////
//
// fChannel, 0 to 1, as one of 256 levels.
//
//////////////////////////////////////////////////////////////////////////
static uint8 ColourFieldByte(float32 fChannel)
{
	if (fChannel <= 0.0f)
	{
		return 0;
	}
	return (fChannel >= 1.0f) ? 255 : (uint8)(fChannel * 255.0f + 0.5f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CColourPicker::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, float32 fRed, float32 fGreen, float32 fBlue)
{
	PreInit();

	CGridContainer::Init(pWindow);
	mafColour[0] = fRed;
	mafColour[1] = fGreen;
	mafColour[2] = fBlue;
	mfHue = 0.0f;
	mbShowing = false;
	mcChanged.Init();

	mpRed = AddChannel(pTheme, "Red", 0, true);
	mpGreen = AddChannel(pTheme, "Green", 1, true);
	mpBlue = AddChannel(pTheme, "Blue", 2, true);
	mpHue = AddChannel(pTheme, "Hue", 3, false);
	mpSaturation = AddChannel(pTheme, "Saturation", 4, false);
	mpValue = AddChannel(pTheme, "Value", 5, false);
	mpHex = OMalloc<CTextField>(pWindow, pTheme, "");
	mpHex->SetAlignment(CSH_Stretch, CSV_Center);
	mpHex->GetChangeEvent()->Add(this, &CColourPicker::HexChanged);
	AddComponent(mpHex, 0, 6, 1, 1, 1, 0);
	SetSizedByContent(true);  //A popup is the size its content wants.

	Show(true);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
// A number field of a channel, 0 to 1, in its row.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CNumberField> CColourPicker::AddChannel(Ptr<CWindowTheme> pTheme, const char* szName, int32 iRow, bool bRGB)
{
	Ptr<CNumberField>	pNumber;

	pNumber = OMalloc<CNumberField>(mpWindow, pTheme, szName, 0.0);
	pNumber->SetRange(0.0, 1.0);
	pNumber->SetDesiredSize(COLOUR_PICKER_WIDTH, pNumber->GetDesiredSize().y);
	if (bRGB)
	{
		pNumber->GetChangeEvent()->Add(this, &CColourPicker::RGBChanged);
	}
	else
	{
		pNumber->GetChangeEvent()->Add(this, &CColourPicker::HSVChanged);
	}
	AddComponent(pNumber, 0, iRow, 1, 1, 1, 0);
	return pNumber;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CColourPicker::Class(void)
{
	CGridContainer::Class();

	M_Pointer(mpRed);
	M_Pointer(mpGreen);
	M_Pointer(mpBlue);
	M_Pointer(mpHue);
	M_Pointer(mpSaturation);
	M_Pointer(mpValue);
	M_Pointer(mpHex);
	U_Float32Array(mafColour, 3);
	U_Float32(mfHue);
	U_Bool(mbShowing);
	U_Unknown(CComponentEvent<CColourPicker>, mcChanged);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CColourPicker::Free(void)
{
	mcChanged.Kill();
	CGridContainer::Free();
}


float32* CColourPicker::GetColour(void) { return mafColour; }
CComponentEvent<CColourPicker>* CColourPicker::GetChangeEvent(void) { return &mcChanged; }
Ptr<CTextField> CColourPicker::GetHex(void) { return mpHex; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CNumberField> CColourPicker::GetChannel(size uiChannel)
{
	switch (uiChannel)
	{
	case 0:		return mpRed;
	case 1:		return mpGreen;
	case 2:		return mpBlue;
	case 3:		return mpHue;
	case 4:		return mpSaturation;
	default:	return mpValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Without calling the change event.
//
//////////////////////////////////////////////////////////////////////////
void CColourPicker::SetColour(float32 fRed, float32 fGreen, float32 fBlue)
{
	mafColour[0] = fRed;
	mafColour[1] = fGreen;
	mafColour[2] = fBlue;
	Show(true);
}


//////////////////////////////////////////////////////////////////////////
//
// The fields are what the colour is.  The hue shown is the one kept
// while the colour is a grey.  The hex isn't set while it's being typed
// in (bHex false), so the caret stays where it is.
//
//////////////////////////////////////////////////////////////////////////
void CColourPicker::Show(bool bHex)
{
	float32	fHue;
	float32	fSaturation;
	float32	fValue;
	char	szHex[16];

	ColourRGBToHSV(mafColour[0], mafColour[1], mafColour[2], &fHue, &fSaturation, &fValue);
	if (fSaturation > 0.0f)
	{
		mfHue = fHue;
	}

	mbShowing = true;
	mpRed->SetValue(mafColour[0]);
	mpGreen->SetValue(mafColour[1]);
	mpBlue->SetValue(mafColour[2]);
	mpHue->SetValue(mfHue);
	mpSaturation->SetValue(fSaturation);
	mpValue->SetValue(fValue);
	if (bHex)
	{
		snprintf(szHex, sizeof(szHex), "%02X%02X%02X", ColourFieldByte(mafColour[0]), ColourFieldByte(mafColour[1]), ColourFieldByte(mafColour[2]));
		mpHex->SetText(szHex);
	}
	mbShowing = false;
}


//////////////////////////////////////////////////////////////////////////
//
// The colour chosen with a field: the others show it and the change
// event is called.
//
//////////////////////////////////////////////////////////////////////////
void CColourPicker::Change(float32 fRed, float32 fGreen, float32 fBlue, bool bHex)
{
	mafColour[0] = fRed;
	mafColour[1] = fGreen;
	mafColour[2] = fBlue;
	Show(bHex);
	mcChanged.Call(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CColourPicker::RGBChanged(CNumberField* pcNumber)
{
	if (!mbShowing)
	{
		Change((float32)mpRed->GetValue(), (float32)mpGreen->GetValue(), (float32)mpBlue->GetValue(), true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The hue chosen is kept, whatever the saturation and value make of it.
//
//////////////////////////////////////////////////////////////////////////
void CColourPicker::HSVChanged(CNumberField* pcNumber)
{
	float32	fRed;
	float32	fGreen;
	float32	fBlue;

	if (!mbShowing)
	{
		mfHue = (float32)mpHue->GetValue();
		ColourHSVToRGB(mfHue, (float32)mpSaturation->GetValue(), (float32)mpValue->GetValue(), &fRed, &fGreen, &fBlue);
		Change(fRed, fGreen, fBlue, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Six hex digits, with or without a # before them, are a colour; anything
// else is still being typed.
//
//////////////////////////////////////////////////////////////////////////
void CColourPicker::HexChanged(CTextField* pcHex)
{
	const char*		szHex;
	unsigned int	uiRed;
	unsigned int	uiGreen;
	unsigned int	uiBlue;
	int				iUsed;

	if (mbShowing)
	{
		return;
	}
	szHex = pcHex->GetText();
	if (*szHex == '#')
	{
		szHex++;
	}
	iUsed = 0;
	if ((strlen(szHex) == 6) && (sscanf(szHex, "%2x%2x%2x%n", &uiRed, &uiGreen, &uiBlue, &iUsed) == 3) && (iUsed == 6))
	{
		Change(uiRed / 255.0f, uiGreen / 255.0f, uiBlue / 255.0f, false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CColourField::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szName, float32 fRed, float32 fGreen, float32 fBlue)
{
	Ptr<CFont>	pFont;
	int32		iPaddingX;
	int32		iPaddingY;
	int32		iWidth;

	PreInit();

	CGridContainer::Init(pWindow);
	mpTheme = pTheme;
	mafColour[0] = fRed;
	mafColour[1] = fGreen;
	mafColour[2] = fBlue;
	mcChanged.Init();

	pFont = pTheme->GetFont();
	iPaddingX = pTheme->GetMetric(TM_FieldPaddingX);
	iPaddingY = pTheme->GetMetric(TM_FieldPaddingY);
	iWidth = NUMBER_FIELD_COLUMNS * (int32)pFont->GetAverageWidth() + 2 * iPaddingX;

	mpName = OMalloc<CLabel>(pWindow, pFont, szName);
	mpName->SetAlignment(CSH_Left, CSV_Center);
	mpSwatchDraw = OMalloc<CClearCanvasDraw>(0);
	mpSwatch = OMalloc<CCanvas>(pWindow, mpSwatchDraw);
	mpSwatch->SetDesiredSize((int32)(iWidth * COLOUR_FIELD_SWATCH), pFont->GetLineHeight() + 2 * iPaddingY);
	mpSwatch->SetAlignment(CSH_Right, CSV_Center);
	AddComponent(mpName, 0, 0, 1, 1, 1, 0);
	AddComponent(mpSwatch, 1, 0, 1, 1, 0, 0);

	SetAlignment(CSH_Stretch, CSV_Center);
	SetHitAsWhole(true);
	SetCanGetFocus(true);
	AddInputAction(CIC_ActivateComponent, this, &CColourField::ActivateComponent);

	Show();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CColourField::Class(void)
{
	CGridContainer::Class();

	M_Pointer(mpTheme);
	M_Pointer(mpName);
	M_Pointer(mpSwatchDraw);
	M_Pointer(mpSwatch);
	M_Pointer(mpPicker);
	U_Float32Array(mafColour, 3);
	U_Unknown(CComponentEvent<CColourField>, mcChanged);
}


//////////////////////////////////////////////////////////////////////////
//
// The picker may outlive the field (its popup holds it): it's no longer
// listened to.
//
//////////////////////////////////////////////////////////////////////////
void CColourField::Free(void)
{
	if (mpPicker.IsNotNull())
	{
		mpPicker->GetChangeEvent()->Remove(this);
	}
	mcChanged.Kill();
	CGridContainer::Free();
}


float32* CColourField::GetColour(void) { return mafColour; }
CComponentEvent<CColourField>* CColourField::GetChangeEvent(void) { return &mcChanged; }
Ptr<CColourPicker> CColourField::GetPicker(void) { return mpPicker; }


//////////////////////////////////////////////////////////////////////////
//
// Without calling the change event.  An open picker shows it.
//
//////////////////////////////////////////////////////////////////////////
void CColourField::SetColour(float32 fRed, float32 fGreen, float32 fBlue)
{
	mafColour[0] = fRed;
	mafColour[1] = fGreen;
	mafColour[2] = fBlue;
	Show();
	if (mpPicker.IsNotNull())
	{
		mpPicker->SetColour(fRed, fGreen, fBlue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The swatch is the colour.
//
//////////////////////////////////////////////////////////////////////////
void CColourField::Show(void)
{
	mpSwatchDraw->SetColour(Set32BitColour(ColourFieldByte(mafColour[0]), ColourFieldByte(mafColour[1]), ColourFieldByte(mafColour[2])));
	mpSwatch->Invalidate();
}


//////////////////////////////////////////////////////////////////////////
//
// Pressed: the picker opens, or closes if it's open.
//
//////////////////////////////////////////////////////////////////////////
void CColourField::ActivateComponent(int x, int y)
{
	if (mpPicker.IsNotNull())
	{
		ClosePicker();
	}
	else
	{
		OpenPicker();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The picker in a popup under the field, with a menu's look.
//
//////////////////////////////////////////////////////////////////////////
void CColourField::OpenPicker(void)
{
	Ptr<CPopup>		pPopup;
	SInt32Vec2		sPosition;
	SInsets			sInsets;
	int32			iInset;

	if (mpPicker.IsNotNull())
	{
		return;
	}
	mpPicker = OMalloc<CColourPicker>(mpWindow, mpTheme, mafColour[0], mafColour[1], mafColour[2]);
	mpPicker->GetChangeEvent()->Add(this, &CColourField::PickerChanged);

	sPosition = GetPosition();
	sPosition.y += GetActualSize().y;
	pPopup = mpWindow->OpenPopup(mpPicker, sPosition, this);
	iInset = mpTheme->GetMetric(TM_MenuInsetY);
	sInsets.Init(iInset, iInset, iInset, iInset);
	pPopup->SetLook(mpTheme->GetColour(TC_MenuBackground), mpTheme->GetColour(TC_MenuBorder), &sInsets);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CColourField::ClosePicker(void)
{
	if (mpPicker.IsNotNull())
	{
		mpWindow->ClosePopup(mpPicker);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The picker's popup has closed, by ClosePicker or a press elsewhere.
//
//////////////////////////////////////////////////////////////////////////
void CColourField::PopupClosed(Ptr<CComponent> pContent)
{
	CGridContainer::PopupClosed(pContent);
	if (pContent == mpPicker)
	{
		mpPicker->GetChangeEvent()->Remove(this);
		mpPicker = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CColourField::PickerChanged(CColourPicker* pcPicker)
{
	float32*	pafColour;

	pafColour = pcPicker->GetColour();
	mafColour[0] = pafColour[0];
	mafColour[1] = pafColour[1];
	mafColour[2] = pafColour[2];
	Show();
	mcChanged.Call(this);
}

