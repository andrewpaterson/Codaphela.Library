/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RasterLib

Codaphela RasterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RasterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RasterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __RASTER_TARGET_H__
#define __RASTER_TARGET_H__
#include "BaseLib/PrimitiveTypes.h"


//The pixels CRasteriser draws on and their depths.  The pixels are not owned so they can be a canvas's.  They are 32 bits,
//0xAARRGGBB: bytes B, G, R, X in memory as CX11Canvas holds them.
class CRasterTarget
{
public:
	uint32*		mpuiPixels;
	int32*		mpiDepths;
	int32		miWidth;
	int32		miHeight;
	int32		miStride;  //Pixels from the start of one row to the next.

	bool		Init(uint32* puiPixels, int32 iWidth, int32 iHeight, int32 iStride);
	void		Kill(void);

	void		Clear(uint32 uiColour);
};


#endif // __RASTER_TARGET_H__

