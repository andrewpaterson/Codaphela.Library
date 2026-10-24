/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __CIRCUIT_FONTS_H__
#define __CIRCUIT_FONTS_H__
#include "SupportLib/FontFamily.h"


//Pixels to a grid unit at zoom one; Logicim's Viewport.SCALE.  Font sizes are pixels at zoom one too.
#define CIRCUIT_TEXT_SCALE		10.0


//The faces Logicim draws with: Java's SansSerif (Logicim bundles Liberation Sans so it measures the same everywhere)
//plain and bold, and Monospaced (Liberation Mono here, which has Courier New's metrics as Java's Monospaced on
//Windows).
enum ECircuitFont
{
	CF_SansSerif,
	CF_SansSerifBold,
	CF_Monospaced,
};


//Which end of a line of text its position is at, as Logicim's HorizontalAlignment.
enum ETextAlignment
{
	TA_Left,
	TA_Centre,
	TA_Right,
};


//The size of a line of text in grid units, as Logicim's TextView.calculateTextDimension: its width and height in
//whole pixels at the font size.  The width is the glyphs' steps (advances rounded to whole pixels; Java also hints
//them, so a few strings come out a pixel different from Java's) and the height is the font's line height.
struct SCircuitTextSize
{
	float64		fWidth;
	float64		fHeight;

	void	Init(float64 fWidth, float64 fHeight);
};


//The font families circuit text is drawn in: a sans serif family with regular and bold faces and a monospaced family
//with a regular face.  Each size (and turn) of a face is rasterised the first time it's drawn.  The families are the
//caller's objects; root them.
class CCircuitFonts
{
protected:
	Ptr<CFontFamily>	mpSansSerif;
	Ptr<CFontFamily>	mpMonospaced;

public:
	void				Init(Ptr<CFontFamily> pSansSerif, Ptr<CFontFamily> pMonospaced);
	void				Kill(void);

	Ptr<CFontColours>	GetColours(ECircuitFont eFont, float32 fPixelSize, EFontTurn eTurn);
	SCircuitTextSize	Measure(ECircuitFont eFont, float64 fSize, const char* szText);

	Ptr<CFontFamily>	GetSansSerif(void);
	Ptr<CFontFamily>	GetMonospaced(void);
};


Ptr<CFontFamily> CreateCircuitFontFamily(const char* szName, const char* szRegularFilename, const char* szBoldFilename, EColourOrder eColourOrder);
bool LoadCircuitFonts(CCircuitFonts* pcFonts, const char* szDirectory, EColourOrder eColourOrder);
const char*	TextAlignmentToString(ETextAlignment eAlignment);
bool		StringToTextAlignment(const char* szAlignment, ETextAlignment* peAlignment);


#endif // __CIRCUIT_FONTS_H__
