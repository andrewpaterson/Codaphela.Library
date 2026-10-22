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
#ifndef __GLYPH_RASTERISER_H__
#define __GLYPH_RASTERISER_H__
#include "StandardLib/Object.h"
#include "Image.h"


struct SGlyphDescription;


//Draws a glyph of a font description into its place in an image, in white with its coverage as the opacity.  A
//description with a rasteriser can have its fonts' glyphs drawn as they are first asked for (see CFont).
class CGlyphRasteriser : public CObject
{
CONSTRUCTABLE(CGlyphRasteriser);
DESTRUCTABLE(CGlyphRasteriser);
public:
			void	Init(void);
			void	Class(void) override;
			void 	Free(void) override;

			bool	Save(CObjectWriter* pcFile) override;
			bool	Load(CObjectReader* pcFile) override;

	virtual bool	RasteriseGlyph(SGlyphDescription* psDescription, Ptr<CImage> pImage) =0;
};


#endif // __GLYPH_RASTERISER_H__

