#ifndef __FONT_H__
#define __FONT_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "BaseLib/UnicodeReader.h"
#include "StandardLib/Object.h"
#include "StandardLib/ObjectWriter.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/Array.h"
#include "BaseLib/ArrayBit.h"
#include "ColourARGB32.h"
#include "Image.h"
#include "Glyph.h"
#include "FontDescription.h"


//A font in one colour: a description shared by every colour of the font and an image laid out as the description
//says.  A glyph is generated the first time its character is asked for and kept by its description's index.  A lazy
//font's image starts empty and each glyph's pixels are drawn into it when the glyph is generated: a white font's by
//the description's rasteriser, a coloured font's by recolouring the white font's.
class CFont : public CObject
{
CONSTRUCTABLE(CFont);
DESTRUCTABLE(CFont);
protected:
	Ptr<CFontDescription>	mpDescription;
	Ptr<CImage>				mpImage;
	CArray<CGlyph>			mapGlyphs;  //By SGlyphDescription::uiIndex, NULL until generated.
	bool					mbLazy;			//The image only has the pixels of the glyphs asked for so far.
	CArrayBit				mabHasPixels;	//By SGlyphDescription::uiIndex, for a lazy font.
	Ptr<CFont>				mpWhite;		//A lazy coloured font recolours its glyphs from this white font.  NULL if white.
	ARGB32					msColour;		//Of a lazy coloured font.

public:
	Ptr<CFont>				Init(Ptr<CFontDescription> pDescription, Ptr<CImage> pImage);
	void					Class(void) override;
	void 					Free(void) override;

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	Ptr<CGlyph>				GetGlyph(uint16 c, size uiLength);
	Ptr<CGlyph>				GetGlyph(uint32 c, size uiLength);
	Ptr<CGlyph>				GetGlyph(uint8* puiBuffer, size uiLength);
	Ptr<CGlyph>				GetGlyph(CUnicodeReader* pcUTFReader);
	Ptr<CGlyph>				GetGlyph(SGlyphDescription* psDescription);
	SGlyphDescription*		GetGlyphDescription(uint8* puiBuffer, size uiLength);
	Ptr<CImageCel>			CreateCel(SGlyphDescription* psDescription);

	void					SetLazy(void);
	void					SetLazyColour(Ptr<CFont> pWhite, ARGB32 sColour);
	bool					IsLazy(void);
	bool					HasPixels(SGlyphDescription* psDescription);
	void					EnsurePixels(SGlyphDescription* psDescription);
	Ptr<CFontDescription>	GetDescription(void);
	Ptr<CImage>				GetImage(void);

	bool					Is(char* szName);
	char*					GetFontName(void);  //Not GetName, that is the object's name.
	size 					CalculateWidth(char* szText);
	bool					IsWhitespace(uint16 c);
	int16					GetAscent(void);
	int16					GetDescent(void);
	int16					GetLineHeight(void);
	int16					GetTabSpaceCount(void);
	int16					GetSpaceWidth(void);
	size					GetAverageWidth(void);
	size					GetAverageHeight(void);
	bool					IsFixedWidth(void);

	size					NumGlyphs(void);
	size					NumGeneratedGlyphs(void);

	bool					StartIteration(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize);
	bool					Iterate(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize);

protected:
	void					RecolourGlyph(SGlyphDescription* psDescription);
};


#endif // __FONT_H__

