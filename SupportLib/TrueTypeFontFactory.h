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
#ifndef __TRUE_TYPE_FONT_FACTORY_H__
#define __TRUE_TYPE_FONT_FACTORY_H__
#include "BaseLib/ArrayTemplate.h"
#include "TrueTypeRasteriser.h"
#include "Font.h"


//Where a character's glyph is in the rasterised image and how it sits in its line.  Bearing and top are relative
//to the pen origin on the baseline, y down.
struct STrueTypeFontCharacter
{
	uint32	uiCodePoint;
	size	uiCodePointLength;  //The length CFont keys the glyph with.
	int		iGlyph;
	int		iAdvance;
	int		iBearing;
	int		iTop;
	int		iImageLeft;
	int		iImageTop;
	int		iImageWidth;
	int		iImageHeight;
};


typedef CArrayTemplate<STrueTypeFontCharacter>	CArrayTrueTypeFontCharacter;


//Makes bitmap CFonts from a TrueType font.  Describe measures the characters and packs them into an image layout once,
//then Rasterise draws an image (opacity, red, green and blue, PT_uint8) in a colour for that layout, as many times
//as there are colours.  A CFont is a description with one of its images.  Each glyph's full size is its advance by
//the line height, so text laid out with CTextLayout sits on a common baseline.  Advances are rounded to whole pixels.
//The arithmetic is integer, so every build makes the same font.
class CTrueTypeFontFactory
{
protected:
	CTrueTypeFont*					mpcFont;
	CTrueTypeRasteriser				mcRasteriser;
	CArrayTrueTypeFontCharacter		macCharacters;
	int								miAscent;
	int								miDescent;
	int32							miPixelSize64;	//The size in 1/64 pixels.

public:
	void			Init(CTrueTypeFont* pcFont, float32 fPixelSize);
	void			Kill(void);

	Ptr<CFontDescription>	Describe(char* szName, char* szUTF8Characters, int iImageWidth = 512);
	Ptr<CImage>				Rasterise(Ptr<CFontDescription> pDescription, CImageColourRGB* pcColour);

	size			NumCharacters(void);
	int				GetAscent(void);
	int				GetDescent(void);

protected:
	bool			AddCharacters(char* szUTF8Characters);
	bool			HasCharacter(uint32 uiCodePoint);
	bool			Pack(int iImageWidth, int* piImageHeight);
	void			CopyGlyph(Ptr<CImage> pImage, int iGlyph, CSubImage* pcSubImage, CImageColourRGB* pcColour);
	uint8			ColourByte(float32 fChannel);
	int16			ScaleFontUnits(int iUnits);
	void			DescribeScriptAndLines(Ptr<CFontDescription> pDescription);
};


#endif // __TRUE_TYPE_FONT_FACTORY_H__
