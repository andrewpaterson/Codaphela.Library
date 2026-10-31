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
#include <stdlib.h>
#include <string.h>
#include "BaseLib/FixedPoint.h"
#include "BaseLib/IntegerHelper.h"
#include "CoverageRasteriser.h"


//A pixel completely covered accumulates cover * 2 * COVERAGE_SUBPIXELS - area of this.
#define COVERAGE_FULL				((int32)2 * COVERAGE_SUBPIXELS * COVERAGE_SUBPIXELS)

//Curves whose control point is less than this far (squared, in subpixels) from straight are drawn as one line;
//one third of a pixel squared.
#define COVERAGE_FLAT_DEVIATION		((int32)COVERAGE_SUBPIXELS * COVERAGE_SUBPIXELS / 3)


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCoverageRasteriser::Init(void)
{
	mpiCover = NULL;
	mpiArea = NULL;
	mpuiCoverage = NULL;
	miWidth = 0;
	miHeight = 0;
	muiCapacity = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCoverageRasteriser::Kill(void)
{
	End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// Starts an empty grid.  The buffers are kept between grids and only
// grow, so many small grids do not allocate.
//
//////////////////////////////////////////////////////////////////////////
bool CCoverageRasteriser::Begin(int iWidth, int iHeight)
{
	lsize	uiCells;

	//An empty grid (nothing to draw) is allowed.
	if ((iWidth < 0) || (iHeight < 0))
	{
		return false;
	}
	uiCells = (lsize)iWidth * iHeight;
	if (uiCells == 0)
	{
		miWidth = iWidth;
		miHeight = iHeight;
		return true;
	}
	if (uiCells > muiCapacity)
	{
		End();
		mpiCover = (int32*)malloc(uiCells * sizeof(int32));
		mpiArea = (int32*)malloc(uiCells * sizeof(int32));
		mpuiCoverage = (uint8*)malloc(uiCells);
		if ((mpiCover == NULL) || (mpiArea == NULL) || (mpuiCoverage == NULL))
		{
			End();
			return false;
		}
		muiCapacity = uiCells;
	}

	miWidth = iWidth;
	miHeight = iHeight;
	memset(mpiCover, 0, uiCells * sizeof(int32));
	memset(mpiArea, 0, uiCells * sizeof(int32));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCoverageRasteriser::End(void)
{
	free(mpiCover);
	free(mpiArea);
	free(mpuiCoverage);
	mpiCover = NULL;
	mpiArea = NULL;
	mpuiCoverage = NULL;
	miWidth = 0;
	miHeight = 0;
	muiCapacity = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Splits a quadratic Bezier curve into lines; more lines the further the
// control point is from the chord.
//
//////////////////////////////////////////////////////////////////////////
void CCoverageRasteriser::AddQuadratic(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2)
{
	int64	iDeviationX;
	int64	iDeviationY;
	int64	iDeviation;
	uint64	iNumLines;
	uint64	i;
	int64	iDivisor;
	int64	iWeight0;
	int64	iWeight1;
	int64	iWeight2;
	int32	iLastX;
	int32	iLastY;
	int32	x;
	int32	y;

	iDeviationX = (int64)x0 - 2 * (int64)x1 + x2;
	iDeviationY = (int64)y0 - 2 * (int64)y1 + y2;
	iDeviation = iDeviationX * iDeviationX + iDeviationY * iDeviationY;
	if (iDeviation < COVERAGE_FLAT_DEVIATION)
	{
		AddLine(x0, y0, x2, y2);
		return;
	}

	//1 + the fourth root of three times the deviation in pixels squared.  A subpixel is 1/16th of a pixel's fourth root.
	iNumLines = 1 + SquareRoot(SquareRoot(3 * iDeviation)) / 16;

	//Points at t = i / n are ((n - i)^2 * p0 + 2 * i * (n - i) * p1 + i^2 * p2) / n^2.
	iDivisor = iNumLines * iNumLines;
	iLastX = x0;
	iLastY = y0;
	for (i = 1; i < iNumLines; i++)
	{
		iWeight0 = (iNumLines - i) * (iNumLines - i);
		iWeight1 = 2 * i * (iNumLines - i);
		iWeight2 = i * i;
		x = (int32)DivideRound(iWeight0 * x0 + iWeight1 * x1 + iWeight2 * x2, iDivisor);
		y = (int32)DivideRound(iWeight0 * y0 + iWeight1 * y1 + iWeight2 * y2, iDivisor);
		AddLine(iLastX, iLastY, x, y);
		iLastX = x;
		iLastY = y;
	}
	AddLine(iLastX, iLastY, x2, y2);
}

//////////////////////////////////////////////////////////////////////////
//
// Adds a line (in subpixels) one pixel row at a time.  Lines going down
// add coverage, lines going up subtract it.  Where a line crosses a row
// boundary is calculated the same way for both rows, so the pieces join
// exactly.
//
//////////////////////////////////////////////////////////////////////////
void CCoverageRasteriser::AddLine(int32 x1, int32 y1, int32 x2, int32 y2)
{
	int32	iYMin;
	int32	iYMax;
	int		iRow;
	int		iRowStart;
	int		iRowEnd;
	int32	iTop;
	int32	iBottom;
	int32	iTopX;
	int32	iBottomX;

	if (y1 == y2)
	{
		return;
	}

	iYMin = (y1 < y2) ? y1 : y2;
	iYMax = (y1 < y2) ? y2 : y1;
	iRowStart = FixedFloor(iYMin, COVERAGE_SUBPIXEL_BITS);
	if (iRowStart < 0)
	{
		iRowStart = 0;
	}
	iRowEnd = FixedCeiling(iYMax, COVERAGE_SUBPIXEL_BITS);
	if (iRowEnd > miHeight)
	{
		iRowEnd = miHeight;
	}

	for (iRow = iRowStart; iRow < iRowEnd; iRow++)
	{
		iTop = FixedFromInt(iRow, COVERAGE_SUBPIXEL_BITS);
		iBottom = FixedFromInt(iRow + 1, COVERAGE_SUBPIXEL_BITS);
		iTop = (iTop > iYMin) ? iTop : iYMin;
		iBottom = (iBottom < iYMax) ? iBottom : iYMax;
		if (iTop >= iBottom)
		{
			continue;
		}

		iTopX = x1 + (int32)DivideRound((int64)(x2 - x1) * (iTop - y1), y2 - y1);
		iBottomX = x1 + (int32)DivideRound((int64)(x2 - x1) * (iBottom - y1), y2 - y1);
		if (y1 < y2)
		{
			AddRowLine(iRow, iTopX, iTop, iBottomX, iBottom);
		}
		else
		{
			AddRowLine(iRow, iBottomX, iBottom, iTopX, iTop);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
// Splits a line inside one pixel row where it crosses pixel columns.
//
//////////////////////////////////////////////////////////////////////////
void CCoverageRasteriser::AddRowLine(int iRow, int32 x1, int32 y1, int32 x2, int32 y2)
{
	int32	iBoundary;
	int32	iStartX;
	int32	iStartY;
	int32	y;

	iStartX = x1;
	iStartY = y1;
	if (x1 < x2)
	{
		for (iBoundary = FixedFromInt(FixedFloor(x1, COVERAGE_SUBPIXEL_BITS) + 1, COVERAGE_SUBPIXEL_BITS); iBoundary < x2; iBoundary += COVERAGE_SUBPIXELS)
		{
			y = y1 + (int32)DivideRound((int64)(y2 - y1) * (iBoundary - x1), x2 - x1);
			AddCellLine(iRow, iStartX, iStartY, iBoundary, y);
			iStartX = iBoundary;
			iStartY = y;
		}
	}
	else if (x1 > x2)
	{
		for (iBoundary = FixedFromInt(FixedCeiling(x1, COVERAGE_SUBPIXEL_BITS) - 1, COVERAGE_SUBPIXEL_BITS); iBoundary > x2; iBoundary -= COVERAGE_SUBPIXELS)
		{
			y = y1 + (int32)DivideRound((int64)(y2 - y1) * (iBoundary - x1), x2 - x1);
			AddCellLine(iRow, iStartX, iStartY, iBoundary, y);
			iStartX = iBoundary;
			iStartY = y;
		}
	}
	AddCellLine(iRow, iStartX, iStartY, x2, y2);
}

//////////////////////////////////////////////////////////////////////////
//
// Adds a line inside one pixel.  The cover is its height and the area is
// twice the area between it and the pixel's left edge (times its
// height's sign).  Lines left of the buffer cover the first column
// completely; lines right of it are dropped.
//
//////////////////////////////////////////////////////////////////////////
void CCoverageRasteriser::AddCellLine(int iRow, int32 x1, int32 y1, int32 x2, int32 y2)
{
	int32	iHeight;
	int		iColumn;
	int32	iLeft;
	int32	iX1;
	int32	iX2;
	lsize	uiCell;

	iHeight = y2 - y1;
	if (iHeight == 0)
	{
		return;
	}

	iColumn = FixedFloor((x1 < x2) ? x1 : x2, COVERAGE_SUBPIXEL_BITS);
	if (iColumn >= miWidth)
	{
		return;
	}
	if (iColumn < 0)
	{
		iColumn = 0;
		iX1 = 0;
		iX2 = 0;
	}
	else
	{
		iLeft = FixedFromInt(iColumn, COVERAGE_SUBPIXEL_BITS);
		iX1 = x1 - iLeft;
		iX2 = x2 - iLeft;
	}

	uiCell = (lsize)iRow * miWidth + iColumn;
	mpiCover[uiCell] += iHeight;
	mpiArea[uiCell] += (iX1 + iX2) * iHeight;
}

//////////////////////////////////////////////////////////////////////////
//
// Sums each row from the left into coverage from 0 to 255.  A pixel's
// coverage is the cover of the cells left of it (which cross it
// completely) less the part of its own cell's edges left of it.  Any
// winding counts as covered.
//
//////////////////////////////////////////////////////////////////////////
void CCoverageRasteriser::Accumulate(void)
{
	uint	x;
	uint	y;
	lsize	uiCell;
	int64	iCover;
	int64	iCoverage;

	for (y = 0; y < (uint)miHeight; y++)
	{
		iCover = 0;
		for (x = 0; x < (uint)miWidth; x++)
		{
			uiCell = (lsize)y * miWidth + x;
			iCover += mpiCover[uiCell];
			iCoverage = iCover * 2 * COVERAGE_SUBPIXELS - mpiArea[uiCell];
			if (iCoverage < 0)
			{
				iCoverage = -iCoverage;
			}
			iCoverage = DivideRound(iCoverage * 255, COVERAGE_FULL);
			mpuiCoverage[uiCell] = (uint8)((iCoverage > 255) ? 255 : iCoverage);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8* CCoverageRasteriser::GetCoverage(void)
{
	return mpuiCoverage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCoverageRasteriser::GetWidth(void)
{
	return miWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCoverageRasteriser::GetHeight(void)
{
	return miHeight;
}

