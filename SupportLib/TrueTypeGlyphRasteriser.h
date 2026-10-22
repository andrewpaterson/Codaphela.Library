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
#ifndef __TRUE_TYPE_GLYPH_RASTERISER_H__
#define __TRUE_TYPE_GLYPH_RASTERISER_H__
#include "GlyphRasteriser.h"
#include "TrueTypeFace.h"


//Rasterises the glyphs of a description made by CTrueTypeFontFactory::Describe from the same face and pixel size, the
//same way CTrueTypeFontFactory::Rasterise does but one glyph at a time.
class CTrueTypeGlyphRasteriser : public CGlyphRasteriser
{
CONSTRUCTABLE(CTrueTypeGlyphRasteriser);
DESTRUCTABLE(CTrueTypeGlyphRasteriser);
protected:
	Ptr<CTrueTypeFace>	mpFace;
	float32				mfPixelSize;

public:
	void	Init(Ptr<CTrueTypeFace> pFace, float32 fPixelSize);
	void	Class(void) override;
	void 	Free(void) override;

	bool	RasteriseGlyph(SGlyphDescription* psDescription, Ptr<CImage> pImage) override;
};


#endif // __TRUE_TYPE_GLYPH_RASTERISER_H__

