/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __TEXT_STYLE_H__
#define __TEXT_STYLE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "ColourARGB32.h"


//Bit flags: bold and italic combine.
enum EFontStyle
{
	FS_Regular = 0,
	FS_Bold = 1,
	FS_Italic = 2,
	FS_BoldItalic = 3,
};


#define NUM_FONT_STYLES		4


//Where text sits: on the baseline, or smaller and raised or lowered.
enum ETextPosition
{
	TP_Normal,
	TP_Superscript,
	TP_Subscript,
};


//How a run of text is drawn: the face of its font family, the size and the colour.
struct STextStyle
{
	bool		bBold;
	bool		bItalic;
	float32		fPixelSize;	//The height of an em in pixels.  0 for the font family's default size.
	ARGB32		sColour;
	ETextPosition	ePosition;	//Superscript and subscript are made smaller than fPixelSize by the font.

	void		Init(void);
	void		Init(bool bBold, bool bItalic, float32 fPixelSize, ARGB32 sColour);
	EFontStyle	GetFontStyle(void);
	bool		Equals(STextStyle* psOther);
	bool		EqualsFont(STextStyle* psOther);
};


#define TSO_Colour		0x01
#define TSO_Bold		0x02
#define TSO_Italic		0x04
#define TSO_PixelSize	0x08
#define TSO_Position	0x10


//Changes to a text style: only the properties whose TSO_ bit is in uiSet are changed.  Overrides applied one after
//another combine, each property taking the value of the last override that sets it.
struct STextStyleOverride
{
	uint16		uiSet;
	STextStyle	sStyle;

	void		Init(void);
	void		SetColour(ARGB32 sColour);
	void		SetBold(bool bBold);
	void		SetItalic(bool bItalic);
	void		SetPixelSize(float32 fPixelSize);
	void		SetPosition(ETextPosition ePosition);
	void		ApplyTo(STextStyle* psStyle);
	bool		ChangesFont(void);
};


#endif // __TEXT_STYLE_H__

