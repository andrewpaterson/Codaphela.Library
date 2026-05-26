#ifndef __IMAGE_ROW_BLITTERS_H__
#define __IMAGE_ROW_BLITTERS_H__
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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "BaseLib/Int32Vec2.h"
#include "StandardLib/Pointer.h"
#include "ColourFormat.h"
#include "ImageCel.h"
#include "Rectangle.h"
#include "ImageCopyDimension.h"
#include "ImageRowBlitter.h"


struct SImageRowBlitter
{
	CImageRowBlitter*	pcBlitter;
	SInt32Vec2			sOffset;	//Offset into the cel?  Image?  Don't know.
	int32				iYRepeat;	//How many times the yOffset increments and is reused.
};


typedef	CArrayTemplate<SImageRowBlitter>	CArrayImageRowBlitter;


class CImageRowBlitters : public CUnknown
{
CONSTRUCTABLE(CImageRowBlitters);
protected:
	CArrayImageRowBlitter	macRowBlitters;

public:
	void	Init(void);
	void	Kill(void) override;
};


#endif // __IMAGE_ROW_BLITTERS_H__

