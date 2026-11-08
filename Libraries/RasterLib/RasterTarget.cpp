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
#include <stdlib.h>
#include <string.h>
#include "RasterTarget.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRasterTarget::Init(uint32* puiPixels, int32 iWidth, int32 iHeight, int32 iStride)
{
	mpuiPixels = puiPixels;
	miWidth = iWidth;
	miHeight = iHeight;
	miStride = iStride;
	mpiDepths = (int32*)malloc((lsize)iWidth * (lsize)iHeight * sizeof(int32));
	return mpiDepths != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterTarget::Kill(void)
{
	free(mpiDepths);
	mpiDepths = NULL;
	mpuiPixels = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A depth of 0 is further away than anything that can be drawn.
//
//////////////////////////////////////////////////////////////////////////
void CRasterTarget::Clear(uint32 uiColour)
{
	int32	x;
	int32	y;
	uint32*	puiRow;

	memset(mpiDepths, 0, (lsize)miWidth * (lsize)miHeight * sizeof(int32));

	for (y = 0; y < miHeight; y++)
	{
		puiRow = &mpuiPixels[(lsize)y * (lsize)miStride];
		for (x = 0; x < miWidth; x++)
		{
			puiRow[x] = uiColour;
		}
	}
}

