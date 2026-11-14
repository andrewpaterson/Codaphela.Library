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
#ifndef __RASTER_TEXTURE_H__
#define __RASTER_TEXTURE_H__
#include "BaseLib/ArrayTemplatePrimitive.h"
#include "StandardLib/Object.h"


//A UV's fraction bits: RASTER_UV_ONE is once across a texture.
#define RASTER_UV_BITS	16
#define RASTER_UV_ONE	(1 << RASTER_UV_BITS)


typedef CArrayTemplatePrimitive<uint32>	CArrayRasterTexel;


//An image a triangle is coloured by, its texels 0xAARRGGBB, a row at a time from the top, as CRasterTarget's pixels are.  A UV
//of (0, 0) is the corner of the first texel and RASTER_UV_ONE is once across, which is what the Vulkan renderer samples.  A UV
//outside 0 to RASTER_UV_ONE repeats the texture.  Sample is the texel a UV is in: there is no filtering.
//
//uiSource is whatever says what it was made from, so that whatever makes them can find one it made before.  RasterLib knows
//nothing of images: RasterRenderLib makes these from them.
class CRasterTexture : public CObject
{
CONSTRUCTABLE(CRasterTexture);
DESTRUCTABLE(CRasterTexture);
protected:
	CArrayRasterTexel	mauiTexels;
	int32				miWidth;
	int32				miHeight;
	int32				miMaskX;  //The width less one if it's a power of two, which repeats by masking, and -1 if it isn't.
	int32				miMaskY;
	uint64				muiSource;

public:
	void		Init(int32 iWidth, int32 iHeight, uint64 uiSource);
	void		Free(void) override;
	void		Class(void) override;

	uint32*		GetTexels(void);
	int32		GetWidth(void);
	int32		GetHeight(void);
	uint64		GetSource(void);

	uint32		Sample(int64 iU, int64 iV);
};


//////////////////////////////////////////////////////////////////////////
//
// Floor modulo, as a UV repeats.
//
//////////////////////////////////////////////////////////////////////////
static FORCE_INLINE int32 RasterTextureWrap(int64 iTexel, int32 iSize, int32 iMask)
{
	int64	iWrapped;

	if (iMask >= 0)
	{
		return (int32)(iTexel & iMask);
	}
	iWrapped = iTexel % iSize;
	return (int32)((iWrapped < 0) ? iWrapped + iSize : iWrapped);
}


//////////////////////////////////////////////////////////////////////////
//
// The texel that UV (iU, iV) is in, RASTER_UV_BITS of fraction each.
//
//////////////////////////////////////////////////////////////////////////
FORCE_INLINE uint32 CRasterTexture::Sample(int64 iU, int64 iV)
{
	int32	x;
	int32	y;

	x = RasterTextureWrap((iU * miWidth) >> RASTER_UV_BITS, miWidth, miMaskX);
	y = RasterTextureWrap((iV * miHeight) >> RASTER_UV_BITS, miHeight, miMaskY);
	return mauiTexels.GetData()[(size)y * (size)miWidth + (size)x];
}


#endif // __RASTER_TEXTURE_H__

