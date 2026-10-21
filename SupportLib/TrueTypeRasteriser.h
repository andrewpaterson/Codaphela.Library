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
#ifndef __TRUE_TYPE_RASTERISER_H__
#define __TRUE_TYPE_RASTERISER_H__
#include "CoverageRasteriser.h"
#include "TrueTypeFont.h"
#include "ImageColour.h"
#include "Image.h"


//Positions are fixed point with 8 fraction bits: 256 subpixels to a pixel.
#define TRUE_TYPE_SUBPIXEL_BITS		COVERAGE_SUBPIXEL_BITS
#define TRUE_TYPE_SUBPIXELS			COVERAGE_SUBPIXELS


//Pixel bounds of something rasterised, relative to the pen origin on the baseline, y down.  Right and bottom are exclusive.
struct STrueTypePixelBounds
{
	int		iLeft;
	int		iTop;
	int		iRight;
	int		iBottom;
};


//Draws TrueType glyphs anti-aliased (by exact area coverage) at a pixel size of one em.  Glyphs are drawn either
//as opacity images (one PT_uint8 IMAGE_OPACITY channel) or blended onto an image in a colour.  Text is UTF-8,
//pens are positioned to 1/256 of a pixel and there is no kerning or hinting.  Overlapping contours (as in variable
//fonts) are filled once.  All the arithmetic is integer (fixed point) so every build draws the same pixels.
class CTrueTypeRasteriser
{
protected:
	CTrueTypeFont*		mpcFont;
	int32				miPixelSize;  //Subpixels per em.
	int32				miUnitsPerEm;
	CTrueTypeOutline	mcOutline;

	CCoverageRasteriser	mcCoverage;

public:
	void			Init(CTrueTypeFont* pcFont, float32 fPixelSize);
	void			Kill(void);

	float32			GetScale(void);
	float32			GetAscent(void);
	float32			GetDescent(void);
	float32			GetLineHeight(void);
	float32			GetAdvance(int iGlyph);
	float32			GetTextWidth(char* szUTF8);

	int32			Scale(int32 iFontUnits);
	int32			GetAscentSubpixels(void);
	int32			GetDescentSubpixels(void);
	int32			GetAdvanceSubpixels(int iGlyph);
	int32			GetTextWidthSubpixels(char* szUTF8);

	bool			GetGlyphBounds(int iGlyph, int32 iPenX, STrueTypePixelBounds* psBounds);
	bool			GetTextBounds(char* szUTF8, STrueTypePixelBounds* psBounds);

	Ptr<CImage>		RasteriseGlyph(int iGlyph, int* piLeft, int* piTop);
	Ptr<CImage>		RasteriseText(char* szUTF8, int* piOriginX, int* piOriginY);
	bool			DrawText(Ptr<CImage> pcDest, float32 fX, float32 fY, char* szUTF8, CImageColour* pcColour);

protected:
	bool			BeginCoverage(int iWidth, int iHeight);
	void			EndCoverage(void);
	bool			AddGlyph(int iGlyph, int32 iX, int32 iY);
	bool			AddText(char* szUTF8, int32 iX, int32 iY);
	Ptr<CImage>		CoverageToImage(void);
	bool			BlendCoverage(Ptr<CImage> pcDest, int iDestX, int iDestY, CImageColour* pcColour);
};


#endif // __TRUE_TYPE_RASTERISER_H__
