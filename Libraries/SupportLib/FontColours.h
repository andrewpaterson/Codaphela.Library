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
#ifndef __FONT_COLOURS_H__
#define __FONT_COLOURS_H__
#include "BaseLib/ArrayTemplate.h"
#include "StandardLib/Array.h"
#include "ColourARGB32.h"
#include "Font.h"


#define FONT_COLOURS_WHITE	0xFFFFFFFF


typedef CArrayTemplate<ARGB32>	CArrayARGB32;


//A font in as many colours as are asked for.  The font it is made from must be white: every pixel of its image is
//white and the glyphs' coverage is in the opacity.  A colour is made the first time it is asked for by copying the
//white image and setting its red, green and blue to the colour; the opacity is kept, scaled by the colour's alpha.
//Every colour shares the white font's description so they all have the same metrics, and each makes its own glyphs
//as they are asked for.
class CFontColours : public CObject
{
CONSTRUCTABLE(CFontColours);
DESTRUCTABLE(CFontColours);
protected:
	Ptr<CFont>		mpWhite;
	CArray<CFont>	mapFonts;	//The recoloured fonts, in the same order as masColours.
	CArrayARGB32	masColours;

public:
	void			Init(Ptr<CFont> pWhite);
	void			Class(void) override;
	void 			Free(void) override;

	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	Ptr<CFont>		GetFont(ARGB32 sColour);
	Ptr<CFont>		GetWhite(void);
	size			NumColours(void);

protected:
	Ptr<CFont>		Recolour(ARGB32 sColour);
};


#endif // __FONT_COLOURS_H__

