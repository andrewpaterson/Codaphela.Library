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
#include <string.h>
#include "PixelDifference.h"


//////////////////////////////////////////////////////////////////////////
//
// Rows that match are passed over whole; only a row that differs is
// looked at pixel by pixel.
//
//////////////////////////////////////////////////////////////////////////
lsize FindPixelDifference(const uint8* puiFirst, lsize uiFirstRowBytes, const uint8* puiSecond, lsize uiSecondRowBytes, size uiPixelBytes, int32 iWidth, int32 iHeight, CInt32Rect* pcDiffers)
{
	const uint8*	puiFirstRow;
	const uint8*	puiSecondRow;
	lsize			uiRowBytes;
	lsize			uiPixels;
	int32			x;
	int32			y;

	pcDiffers->Init(iWidth, iHeight, 0, 0);
	uiPixels = 0;
	uiRowBytes = (lsize)iWidth * uiPixelBytes;
	for (y = 0; y < iHeight; y++)
	{
		puiFirstRow = &puiFirst[(lsize)y * uiFirstRowBytes];
		puiSecondRow = &puiSecond[(lsize)y * uiSecondRowBytes];
		if (memcmp(puiFirstRow, puiSecondRow, uiRowBytes) == 0)
		{
			continue;
		}
		for (x = 0; x < iWidth; x++)
		{
			if (memcmp(&puiFirstRow[(lsize)x * uiPixelBytes], &puiSecondRow[(lsize)x * uiPixelBytes], uiPixelBytes) != 0)
			{
				uiPixels++;
				pcDiffers->GrowToContain(x, y);
			}
		}
	}
	return uiPixels;
}

