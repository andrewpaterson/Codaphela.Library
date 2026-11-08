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
#include "BaseLib/FixedPoint.h"
#include "BaseLib/PrimitiveTypes.h"


//A position: 256 subpixels to a pixel.
typedef SFixed<int32, int64, 8>		SCoverageSubpixel;
typedef SFixed<int64, int64, 8>		SCoverageWideSubpixel;

//A subpixel times a subpixel.
typedef SFixed<int32, int64, 16>	SCoverageArea;

//A number of something to multiply or divide by.
typedef SFixed<int32, int64, 0>		SCoverageWhole;


//Fills outlines anti-aliased by exact area coverage into a grid of pixels from 0 (empty) to 255 (covered).  Outlines are
//lines and quadratic curves in subpixels, y down.  Any winding counts as covered so overlapping contours are filled once
//and contours winding the other way inside them are holes.  All the arithmetic is integer (fixed point) so every build
//makes the same coverage.
class CCoverageRasteriser
{
protected:
	//Each pixel cell accumulates the height (cover) and twice the area (area) of the edges crossing it, in subpixels.
	SCoverageSubpixel*	mpsCover;
	SCoverageArea*		mpsArea;
	uint8*		mpuiCoverage;
	int			miWidth;
	int			miHeight;
	lsize		muiCapacity;

public:
	void		Init(void);
	void		Kill(void);

	bool		Begin(int iWidth, int iHeight);
	void		End(void);

	void		AddQuadratic(SCoverageSubpixel x0, SCoverageSubpixel y0, SCoverageSubpixel x1, SCoverageSubpixel y1, SCoverageSubpixel x2, SCoverageSubpixel y2);
	void		AddLine(SCoverageSubpixel x1, SCoverageSubpixel y1, SCoverageSubpixel x2, SCoverageSubpixel y2);
	void		Accumulate(void);

	uint8*		GetCoverage(void);
	int			GetWidth(void);
	int			GetHeight(void);

protected:
	void		AddRowLine(int iRow, SCoverageSubpixel x1, SCoverageSubpixel y1, SCoverageSubpixel x2, SCoverageSubpixel y2);
	void		AddCellLine(int iRow, SCoverageSubpixel x1, SCoverageSubpixel y1, SCoverageSubpixel x2, SCoverageSubpixel y2);
};


#endif // __COVERAGE_RASTERISER_H__

