/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RenderLib

Codaphela RenderLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RenderLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RenderLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __TEXTURE_LEVELS_H__
#define __TEXTURE_LEVELS_H__
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayUint32.h"


class CMaterialImage;


//A material image's texels at every mip level, as the renderers texture with them (GetImageTexels): level 0 the image's, then
//each half the one before (rounded up) to 1 by 1.  A level is the image's own (CMaterialImage's mapLevels, as a bake makes
//them) where it has one that size, or else made from the level before by HalveTexels.  The levels are end to end, finest first.
//
//A GPU halves rounding down (a level 3 across is followed by one 1 across, not 2), and takes no other sizes: bRoundDown gives
//its levels, the ones whose sizes differ made from the level before by ShrinkTexels.  The two are the same for sizes that are
//powers of two.
class CTextureLevels
{
public:
	CArrayLargeUint32	mauiTexels;
	CArrayLargeInt		maiStarts;  //Each level's first texel.
	CArrayLargeInt		maiWidths;
	CArrayLargeInt		maiHeights;

	bool	Init(CMaterialImage* pcImage, bool bRoundDown = false);  //False, and no levels, for no image or one that isn't bytes.
	void	Kill(void);

	int32	NumLevels(void) const;
	uint32*	GetTexels(int32 iLevel);
	int32	GetWidth(int32 iLevel);
	int32	GetHeight(int32 iLevel);
};


//The size of the level after one iSize texels across.
int32	HalfTextureSize(int32 iSize);

//The level after puiSource, iWidth by iHeight: each of its texels the mean of the two by two it covers (each channel, rounded),
//or of the one or two at an odd size's last row and column.
void	HalveTexels(uint32* puiDest, uint32* puiSource, int32 iWidth, int32 iHeight);

//puiSource, iWidth by iHeight, made iDestWidth by iDestHeight, neither more than the source's: each of its texels the mean of
//the source's it covers (each channel, rounded).
void	ShrinkTexels(uint32* puiDest, int32 iDestWidth, int32 iDestHeight, uint32* puiSource, int32 iWidth, int32 iHeight);


#endif // __TEXTURE_LEVELS_H__

