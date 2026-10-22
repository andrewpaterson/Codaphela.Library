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
#include "TextStyle.h"


//////////////////////////////////////////////////////////////////////////
//
// Regular, the font family's default size, opaque white.
//
//////////////////////////////////////////////////////////////////////////
void STextStyle::Init(void)
{
	Init(false, false, 0.0f, 0xFFFFFFFF);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STextStyle::Init(bool bBold, bool bItalic, float32 fPixelSize, ARGB32 sColour)
{
	this->bBold = bBold;
	this->bItalic = bItalic;
	this->fPixelSize = fPixelSize;
	this->sColour = sColour;
	ePosition = TP_Normal;
	bUnderline = false;
	bOverline = false;
	bStrikethrough = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFontStyle STextStyle::GetFontStyle(void)
{
	return (EFontStyle)((bBold ? FS_Bold : FS_Regular) | (bItalic ? FS_Italic : FS_Regular));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool STextStyle::Equals(STextStyle* psOther)
{
	return EqualsFont(psOther) && (sColour == psOther->sColour) && (bUnderline == psOther->bUnderline) && (bOverline == psOther->bOverline) && (bStrikethrough == psOther->bStrikethrough);
}


//////////////////////////////////////////////////////////////////////////
//
// TD_ flags.
//
//////////////////////////////////////////////////////////////////////////
uint8 STextStyle::GetDecorations(void)
{
	return (bUnderline ? TD_Underline : 0) | (bOverline ? TD_Overline : 0) | (bStrikethrough ? TD_Strikethrough : 0);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the styles use the same face, size and position, whatever their
// colours and decorations.
//
//////////////////////////////////////////////////////////////////////////
bool STextStyle::EqualsFont(STextStyle* psOther)
{
	return (bBold == psOther->bBold) && (bItalic == psOther->bItalic) && (fPixelSize == psOther->fPixelSize) && (ePosition == psOther->ePosition);
}


//////////////////////////////////////////////////////////////////////////
//
// Changes nothing.
//
//////////////////////////////////////////////////////////////////////////
void STextStyleOverride::Init(void)
{
	uiSet = 0;
	sStyle.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STextStyleOverride::SetColour(ARGB32 sColour)
{
	sStyle.sColour = sColour;
	uiSet |= TSO_Colour;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STextStyleOverride::SetBold(bool bBold)
{
	sStyle.bBold = bBold;
	uiSet |= TSO_Bold;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STextStyleOverride::SetItalic(bool bItalic)
{
	sStyle.bItalic = bItalic;
	uiSet |= TSO_Italic;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STextStyleOverride::SetPixelSize(float32 fPixelSize)
{
	sStyle.fPixelSize = fPixelSize;
	uiSet |= TSO_PixelSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STextStyleOverride::SetPosition(ETextPosition ePosition)
{
	sStyle.ePosition = ePosition;
	uiSet |= TSO_Position;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STextStyleOverride::SetUnderline(bool bUnderline)
{
	sStyle.bUnderline = bUnderline;
	uiSet |= TSO_Underline;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STextStyleOverride::SetOverline(bool bOverline)
{
	sStyle.bOverline = bOverline;
	uiSet |= TSO_Overline;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STextStyleOverride::SetStrikethrough(bool bStrikethrough)
{
	sStyle.bStrikethrough = bStrikethrough;
	uiSet |= TSO_Strikethrough;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STextStyleOverride::ApplyTo(STextStyle* psStyle)
{
	if (uiSet & TSO_Colour)
	{
		psStyle->sColour = sStyle.sColour;
	}
	if (uiSet & TSO_Bold)
	{
		psStyle->bBold = sStyle.bBold;
	}
	if (uiSet & TSO_Italic)
	{
		psStyle->bItalic = sStyle.bItalic;
	}
	if (uiSet & TSO_PixelSize)
	{
		psStyle->fPixelSize = sStyle.fPixelSize;
	}
	if (uiSet & TSO_Position)
	{
		psStyle->ePosition = sStyle.ePosition;
	}
	if (uiSet & TSO_Underline)
	{
		psStyle->bUnderline = sStyle.bUnderline;
	}
	if (uiSet & TSO_Overline)
	{
		psStyle->bOverline = sStyle.bOverline;
	}
	if (uiSet & TSO_Strikethrough)
	{
		psStyle->bStrikethrough = sStyle.bStrikethrough;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if the override can change which font is used, and so the size of
// the text.  Colours and decorations don't.
//
//////////////////////////////////////////////////////////////////////////
bool STextStyleOverride::ChangesFont(void)
{
	return (uiSet & (TSO_Bold | TSO_Italic | TSO_PixelSize | TSO_Position)) != 0;
}

