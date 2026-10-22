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
#ifndef __FONT_DESCRIPTION_H__
#define __FONT_DESCRIPTION_H__
#include "BaseLib/IndexTemplate.h"
#include "BaseLib/UnicodeReader.h"
#include "StandardLib/Object.h"
#include "SubImage.h"


#define FONT_MAX_KEY_LENGTH		64
#define FIRST_LETTER			32


//Where a glyph's pixels are in a font's image and how far it advances.  The glyph itself (a CGlyph and its
//CImageCel) is only made by a CFont the first time the character is asked for.
struct SGlyphDescription
{
	CSubImage	cSubImage;
	int16		iStep;
	size		uiIndex;  //0 to NumGlyphs - 1 in the order the glyphs were put.  A CFont keeps its glyphs by it.
};


typedef CIndexTemplate<SGlyphDescription> CIndexGlyphDescription;


//Everything about a font that doesn't depend on its colour: its metrics and where each character's glyph is in an
//image of a known size.  Fonts of different colours made from the same TrueType font and size share one
//description, each with its own image laid out the same way.  Glyphs are keyed by their character's code point
//bytes (little endian), as many bytes as the code point's length.
class CFontDescription : public CObject
{
CONSTRUCTABLE(CFontDescription);
DESTRUCTABLE(CFontDescription);
protected:
	CCharsImmutable			mszName;
	CIndexGlyphDescription	macGlyphs;
	int32					miImageWidth;
	int32					miImageHeight;
	size					miAverageWidth;		//Only valid when mbWidthsCalculated.
	size					miAverageHeight;
	bool					mbFixedWidth;		//Only valid when mbWidthsCalculated.
	bool					mbWidthsCalculated;
	int16					miSpaceWidth;
	int16					miAscent;
	int16					miDescent;
	int16					miTabSpaceCount;  //tab width = miTabSpaceCount * miSpaceWidth.

public:
	void				Init(char* szName, int16 iSpaceWidth, int16 iAscent, int16 iDescent, int32 iImageWidth, int32 iImageHeight);
	void				Class(void) override;
	void 				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				PutGlyph(uint32 c, size uiLength, CSubImage* pcSubImage, int16 iStep);
	void				PutGlyph(uint8* puiBuffer, size uiLength, CSubImage* pcSubImage, int16 iStep);
	SGlyphDescription*	GetGlyph(uint8* puiBuffer, size uiLength);
	SGlyphDescription*	GetGlyph(CUnicodeReader* pcUTFReader);
	size				ReadKey(CUnicodeReader* pcUTFReader, uint8* puiKey, size uiMaxKeyLength);
	size				NumGlyphs(void);

	bool				StartIteration(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize);
	bool				Iterate(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize);

	bool				Is(char* szName);
	char*				GetFontName(void);  //Not GetName, that is the object's name.
	size 				CalculateWidth(char* szText);
	bool				IsWhitespace(uint16 c);
	int32				GetImageWidth(void);
	int32				GetImageHeight(void);
	int16				GetAscent(void);
	int16				GetDescent(void);
	int16				GetLineHeight(void);
	int16				GetTabSpaceCount(void);
	int16				GetSpaceWidth(void);
	size				GetAverageWidth(void);
	size				GetAverageHeight(void);
	bool				IsFixedWidth(void);

protected:
	void				CalculateWidths(void);
};


#endif // __FONT_DESCRIPTION_H__

