/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __IMAGE_DRAW_BOX_H__
#define __IMAGE_DRAW_BOX_H__
#include "Image.h"
#include "ImageModifier.h"


class CImageDrawBox : public CImageModifier
{
BASE_FUNCTIONS(CImageDrawBox);
public:
	CRectangle		mcRectangle;
	BOOL			mbWholeImage;
	CImageColour*	mpcColour;
	BOOL			mbFilled;

	void Init(CRectangle* pcRectangle, CImageColour* pcColour, BOOL bFilled = TRUE);
	void Kill(void);
	void Modify(CImage* pcImage);
};


#endif // __IMAGE_IMAGE_RENAME_H__

