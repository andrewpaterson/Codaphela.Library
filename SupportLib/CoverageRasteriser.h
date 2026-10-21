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
#ifndef __COVERAGE_RASTERISER_H__
#define __COVERAGE_RASTERISER_H__
#include "BaseLib/PrimitiveTypes.h"


//Positions are fixed point with 8 fraction bits: 256 subpixels to a pixel.
#define COVERAGE_SUBPIXEL_BITS		8
#define COVERAGE_SUBPIXELS			(1 << COVERAGE_SUBPIXEL_BITS)


//Fills outlines anti-aliased by exact area coverage into a grid of pixels from 0 (empty) to 255 (covered).  Outlines are
//lines and quadratic curves in subpixels, y down.  Any winding counts as covered so overlapping contours are filled once
//and contours winding the other way inside them are holes.  All the arithmetic is integer (fixed point) so every build
//makes the same coverage.
class CCoverageRasteriser
{
protected:
	//Each pixel cell accumulates the height (cover) and twice the area (area) of the edges crossing it, in subpixels.
	int32*		mpiCover;
	int32*		mpiArea;
	uint8*		mpuiCoverage;
	int			miWidth;
	int			miHeight;
	size		muiCapacity;

public:
	void		Init(void);
	void		Kill(void);

	bool		Begin(int iWidth, int iHeight);
	void		End(void);

	void		AddQuadratic(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2);
	void		AddLine(int32 x1, int32 y1, int32 x2, int32 y2);
	void		Accumulate(void);

	uint8*		GetCoverage(void);
	int			GetWidth(void);
	int			GetHeight(void);

protected:
	void		AddRowLine(int iRow, int32 x1, int32 y1, int32 x2, int32 y2);
	void		AddCellLine(int iRow, int32 x1, int32 y1, int32 x2, int32 y2);
};


#endif // __COVERAGE_RASTERISER_H__

