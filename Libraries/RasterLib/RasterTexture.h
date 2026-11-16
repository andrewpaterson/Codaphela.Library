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


//A UV level of detail's fraction bits: a mip level is RASTER_LOD_ONE.
#define RASTER_LOD_BITS	8
#define RASTER_LOD_ONE	(1 << RASTER_LOD_BITS)


//How a texture's texels make a pixel's colour (CRasteriser::SetFilter).  Every filter picks its mip level from the triangle's
//texels against its pixels, made finer or coarser by each pixel's distance.
enum ERasterFilter
{
	RF_Nearest,		//The texel the UV is in, at the nearest level.
	RF_Bilinear,	//The four texels round the UV blended, at the nearest level.
	RF_Trilinear,	//Bilinear at the two levels either side, blended.
};


//A mip level: iWidth by iHeight texels from uiStart.  The masks are the size less one if it's a power of two, which repeats by
//masking, and -1 if it isn't.
struct SRasterTextureLevel
{
	size	uiStart;
	int32	iWidth;
	int32	iHeight;
	int32	iMaskX;
	int32	iMaskY;
};


typedef CArrayTemplate<SRasterTextureLevel>	CArrayRasterTextureLevel;


//An image a triangle is coloured by, its texels 0xAARRGGBB, a row at a time from the top, as CRasterTarget's pixels are.  A UV
//of (0, 0) is the corner of the first texel and RASTER_UV_ONE is once across, which is what the Vulkan renderer samples.  A UV
//outside 0 to RASTER_UV_ONE repeats the texture.
//
//It has mip levels: level 0 is made by Init, and each smaller one by AddLevel (half the one before, rounded up, as the
//renderers make them).  Sample is the texel a UV is in at level 0, SampleNearest at any level, SampleBilinear the four round
//it blended (texel centres are half a texel in), SampleTrilinear bilinear at the levels either side of a level of detail
//(RASTER_LOD_BITS of fraction), blended.  A level past the last is the last.
//
//uiSource is whatever says what it was made from, so that whatever makes them can find one it made before.  RasterLib knows
//nothing of images: RasterRenderLib makes these from them.
class CRasterTexture : public CObject
{
CONSTRUCTABLE(CRasterTexture);
DESTRUCTABLE(CRasterTexture);
protected:
	CArrayRasterTexel			mauiTexels;  //Every level's, finest first.
	CArrayRasterTextureLevel	masLevels;
	uint64						muiSource;

public:
	void		Init(int32 iWidth, int32 iHeight, uint64 uiSource);
	void		Free(void) override;
	void		Class(void) override;

	uint32*		AddLevel(int32 iWidth, int32 iHeight);  //Its texels, white, valid until the next is added.

	int32		NumLevels(void);
	uint32*		GetTexels(int32 iLevel = 0);
	int32		GetWidth(int32 iLevel = 0);
	int32		GetHeight(int32 iLevel = 0);
	uint64		GetSource(void);

	uint32		Sample(int64 iU, int64 iV);
	uint32		SampleNearest(int64 iU, int64 iV, int32 iLevel);
	uint32		SampleBilinear(int64 iU, int64 iV, int32 iLevel);
	uint32		SampleTrilinear(int64 iU, int64 iV, int32 iLod);

protected:
	SRasterTextureLevel*	Level(int32 iLevel);
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
// a and b blended, iFraction (0 to 256) of the way to b, every channel at
// once: red and blue in one multiply, alpha and green in another.
//
//////////////////////////////////////////////////////////////////////////
static FORCE_INLINE uint32 RasterTextureBlend(uint32 a, uint32 b, uint32 uiFraction)
{
	uint32	uiKeep;
	uint32	uiRedBlue;
	uint32	uiAlphaGreen;

	uiKeep = 256 - uiFraction;
	uiRedBlue = ((((a & 0x00FF00FF) * uiKeep) + ((b & 0x00FF00FF) * uiFraction)) >> 8) & 0x00FF00FF;
	uiAlphaGreen = (((((a >> 8) & 0x00FF00FF) * uiKeep) + (((b >> 8) & 0x00FF00FF) * uiFraction))) & 0xFF00FF00;
	return uiRedBlue | uiAlphaGreen;
}


//////////////////////////////////////////////////////////////////////////
//
// A level, the last for one past it.
//
//////////////////////////////////////////////////////////////////////////
FORCE_INLINE SRasterTextureLevel* CRasterTexture::Level(int32 iLevel)
{
	if (iLevel >= (int32)masLevels.NumElements())
	{
		iLevel = (int32)masLevels.NumElements() - 1;
	}
	return masLevels.Get((size)((iLevel > 0) ? iLevel : 0));
}


//////////////////////////////////////////////////////////////////////////
//
// The texel that UV (iU, iV) is in, RASTER_UV_BITS of fraction each.
//
//////////////////////////////////////////////////////////////////////////
FORCE_INLINE uint32 CRasterTexture::SampleNearest(int64 iU, int64 iV, int32 iLevel)
{
	SRasterTextureLevel*	psLevel;
	int32					x;
	int32					y;

	psLevel = Level(iLevel);
	x = RasterTextureWrap((iU * psLevel->iWidth) >> RASTER_UV_BITS, psLevel->iWidth, psLevel->iMaskX);
	y = RasterTextureWrap((iV * psLevel->iHeight) >> RASTER_UV_BITS, psLevel->iHeight, psLevel->iMaskY);
	return mauiTexels.GetData()[psLevel->uiStart + ((size)y * (size)psLevel->iWidth) + (size)x];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
FORCE_INLINE uint32 CRasterTexture::Sample(int64 iU, int64 iV)
{
	return SampleNearest(iU, iV, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The UV in texels with 8 bits of fraction, less half a texel so whole
// texels are at their centres: the four texels round it, by how far it
// is past the first across and down.
//
//////////////////////////////////////////////////////////////////////////
FORCE_INLINE uint32 CRasterTexture::SampleBilinear(int64 iU, int64 iV, int32 iLevel)
{
	SRasterTextureLevel*	psLevel;
	uint32*					puiTexels;
	int64					iX;
	int64					iY;
	int32					x0;
	int32					x1;
	int32					y0;
	int32					y1;
	uint32					uiTop;
	uint32					uiBottom;

	psLevel = Level(iLevel);
	puiTexels = mauiTexels.GetData() + psLevel->uiStart;
	iX = ((iU * psLevel->iWidth) >> (RASTER_UV_BITS - 8)) - 128;
	iY = ((iV * psLevel->iHeight) >> (RASTER_UV_BITS - 8)) - 128;
	x0 = RasterTextureWrap(iX >> 8, psLevel->iWidth, psLevel->iMaskX);
	x1 = RasterTextureWrap((iX >> 8) + 1, psLevel->iWidth, psLevel->iMaskX);
	y0 = RasterTextureWrap(iY >> 8, psLevel->iHeight, psLevel->iMaskY);
	y1 = RasterTextureWrap((iY >> 8) + 1, psLevel->iHeight, psLevel->iMaskY);
	uiTop = RasterTextureBlend(puiTexels[((size)y0 * (size)psLevel->iWidth) + (size)x0], puiTexels[((size)y0 * (size)psLevel->iWidth) + (size)x1], (uint32)(iX & 255));
	uiBottom = RasterTextureBlend(puiTexels[((size)y1 * (size)psLevel->iWidth) + (size)x0], puiTexels[((size)y1 * (size)psLevel->iWidth) + (size)x1], (uint32)(iX & 255));
	return RasterTextureBlend(uiTop, uiBottom, (uint32)(iY & 255));
}


//////////////////////////////////////////////////////////////////////////
//
// Below level 0 is level 0, past the last the last.
//
//////////////////////////////////////////////////////////////////////////
FORCE_INLINE uint32 CRasterTexture::SampleTrilinear(int64 iU, int64 iV, int32 iLod)
{
	int32	iLevel;

	if (iLod <= 0)
	{
		return SampleBilinear(iU, iV, 0);
	}
	iLevel = iLod >> RASTER_LOD_BITS;
	if (iLevel >= (int32)masLevels.NumElements() - 1)
	{
		return SampleBilinear(iU, iV, iLevel);
	}
	return RasterTextureBlend(SampleBilinear(iU, iV, iLevel), SampleBilinear(iU, iV, iLevel + 1), (uint32)(iLod & (RASTER_LOD_ONE - 1)) << (8 - RASTER_LOD_BITS));
}


#endif // __RASTER_TEXTURE_H__

