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


//A pixel completely covered accumulates cover * 2 - area of this many pixels' area.
#define COVERAGE_FULL_PIXELS		2

//Curves whose control point is less than this far (squared) from straight are drawn as one line; one third of a pixel
//squared.
#define COVERAGE_FLAT_DEVIATION		(SFixed<int64, int64, 16>::FromRaw(65536 / 3))


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCoverageRasteriser::Init(void)
{
	mpsCover = NULL;
	mpsArea = NULL;
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
		mpsCover = (SCoverageSubpixel*)malloc(uiCells * sizeof(SCoverageSubpixel));
		mpsArea = (SCoverageArea*)malloc(uiCells * sizeof(SCoverageArea));
		mpuiCoverage = (uint8*)malloc(uiCells);
		if ((mpsCover == NULL) || (mpsArea == NULL) || (mpuiCoverage == NULL))
		{
			End();
			return false;
		}
		muiCapacity = uiCells;
	}

	miWidth = iWidth;
	miHeight = iHeight;
	memset(mpsCover, 0, uiCells * sizeof(SCoverageSubpixel));
	memset(mpsArea, 0, uiCells * sizeof(SCoverageArea));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCoverageRasteriser::End(void)
{
	free(mpsCover);
	free(mpsArea);
	free(mpuiCoverage);
	mpsCover = NULL;
	mpsArea = NULL;
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
void CCoverageRasteriser::AddQuadratic(SCoverageSubpixel x0, SCoverageSubpixel y0, SCoverageSubpixel x1, SCoverageSubpixel y1, SCoverageSubpixel x2, SCoverageSubpixel y2)
{
	typedef SFixed<int64, int64, 0>		SWeight;

	SCoverageWideSubpixel		sWideX0;
	SCoverageWideSubpixel		sWideY0;
	SCoverageWideSubpixel		sWideX1;
	SCoverageWideSubpixel		sWideY1;
	SCoverageWideSubpixel		sWideX2;
	SCoverageWideSubpixel		sWideY2;
	SCoverageWideSubpixel		sDeviationX;
	SCoverageWideSubpixel		sDeviationY;
	SFixed<int64, int64, 16>	sDeviation;
	uint64						iNumLines;
	uint64						i;
	SWeight						sDivisor;
	SWeight						sWeight0;
	SWeight						sWeight1;
	SWeight						sWeight2;
	SCoverageSubpixel			sLastX;
	SCoverageSubpixel			sLastY;
	SCoverageSubpixel			x;
	SCoverageSubpixel			y;

	sWideX0 = x0.To<SCoverageWideSubpixel>();
	sWideY0 = y0.To<SCoverageWideSubpixel>();
	sWideX1 = x1.To<SCoverageWideSubpixel>();
	sWideY1 = y1.To<SCoverageWideSubpixel>();
	sWideX2 = x2.To<SCoverageWideSubpixel>();
	sWideY2 = y2.To<SCoverageWideSubpixel>();

	sDeviationX = sWideX0 - sWideX1 - sWideX1 + sWideX2;
	sDeviationY = sWideY0 - sWideY1 - sWideY1 + sWideY2;
	sDeviation = sDeviationX * sDeviationX + sDeviationY * sDeviationY;
	if (sDeviation < COVERAGE_FLAT_DEVIATION)
	{
		AddLine(x0, y0, x2, y2);
		return;
	}

	//1 + the fourth root of three times the deviation in pixels squared.  A subpixel is 1/16th of a pixel's fourth root.
	iNumLines = 1 + SquareRoot(SquareRoot(3 * sDeviation.miRaw)) / 16;

	//Points at t = i / n are ((n - i)^2 * p0 + 2 * i * (n - i) * p1 + i^2 * p2) / n^2.
	sDivisor = SWeight::FromRaw(iNumLines * iNumLines);
	sLastX = x0;
	sLastY = y0;
	for (i = 1; i < iNumLines; i++)
	{
		sWeight0 = SWeight::FromRaw((iNumLines - i) * (iNumLines - i));
		sWeight1 = SWeight::FromRaw(2 * i * (iNumLines - i));
		sWeight2 = SWeight::FromRaw(i * i);
		x = (sWeight0 * sWideX0 + sWeight1 * sWideX1 + sWeight2 * sWideX2).DivideRound(sDivisor).To<SCoverageSubpixel>();
		y = (sWeight0 * sWideY0 + sWeight1 * sWideY1 + sWeight2 * sWideY2).DivideRound(sDivisor).To<SCoverageSubpixel>();
		AddLine(sLastX, sLastY, x, y);
		sLastX = x;
		sLastY = y;
	}
	AddLine(sLastX, sLastY, x2, y2);
}

//////////////////////////////////////////////////////////////////////////
//
// Adds a line (in subpixels) one pixel row at a time.  Lines going down
// add coverage, lines going up subtract it.  Where a line crosses a row
// boundary is calculated the same way for both rows, so the pieces join
// exactly.
//
//////////////////////////////////////////////////////////////////////////
void CCoverageRasteriser::AddLine(SCoverageSubpixel x1, SCoverageSubpixel y1, SCoverageSubpixel x2, SCoverageSubpixel y2)
{
	SCoverageSubpixel	sYMin;
	SCoverageSubpixel	sYMax;
	int					iRow;
	int					iRowStart;
	int					iRowEnd;
	SCoverageSubpixel	sTop;
	SCoverageSubpixel	sBottom;
	SCoverageSubpixel	sTopX;
	SCoverageSubpixel	sBottomX;

	if (y1 == y2)
	{
		return;
	}

	sYMin = (y1 < y2) ? y1 : y2;
	sYMax = (y1 < y2) ? y2 : y1;
	iRowStart = sYMin.Floor();
	if (iRowStart < 0)
	{
		iRowStart = 0;
	}
	iRowEnd = sYMax.Ceiling();
	if (iRowEnd > miHeight)
	{
		iRowEnd = miHeight;
	}

	for (iRow = iRowStart; iRow < iRowEnd; iRow++)
	{
		sTop = SCoverageSubpixel::FromInt(iRow);
		sBottom = SCoverageSubpixel::FromInt(iRow + 1);
		sTop = (sTop > sYMin) ? sTop : sYMin;
		sBottom = (sBottom < sYMax) ? sBottom : sYMax;
		if (sTop >= sBottom)
		{
			continue;
		}

		sTopX = x1 + ((x2 - x1) * (sTop - y1)).DivideRound(y2 - y1).To<SCoverageSubpixel>();
		sBottomX = x1 + ((x2 - x1) * (sBottom - y1)).DivideRound(y2 - y1).To<SCoverageSubpixel>();
		if (y1 < y2)
		{
			AddRowLine(iRow, sTopX, sTop, sBottomX, sBottom);
		}
		else
		{
			AddRowLine(iRow, sBottomX, sBottom, sTopX, sTop);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
// Splits a line inside one pixel row where it crosses pixel columns.
//
//////////////////////////////////////////////////////////////////////////
void CCoverageRasteriser::AddRowLine(int iRow, SCoverageSubpixel x1, SCoverageSubpixel y1, SCoverageSubpixel x2, SCoverageSubpixel y2)
{
	SCoverageSubpixel	sBoundary;
	SCoverageSubpixel	sStartX;
	SCoverageSubpixel	sStartY;
	SCoverageSubpixel	sPixel;
	SCoverageSubpixel	y;

	sStartX = x1;
	sStartY = y1;
	sPixel = SCoverageSubpixel::FromInt(1);
	if (x1 < x2)
	{
		for (sBoundary = SCoverageSubpixel::FromInt(x1.Floor() + 1); sBoundary < x2; sBoundary += sPixel)
		{
			y = y1 + ((y2 - y1) * (sBoundary - x1)).DivideRound(x2 - x1).To<SCoverageSubpixel>();
			AddCellLine(iRow, sStartX, sStartY, sBoundary, y);
			sStartX = sBoundary;
			sStartY = y;
		}
	}
	else if (x1 > x2)
	{
		for (sBoundary = SCoverageSubpixel::FromInt(x1.Ceiling() - 1); sBoundary > x2; sBoundary -= sPixel)
		{
			y = y1 + ((y2 - y1) * (sBoundary - x1)).DivideRound(x2 - x1).To<SCoverageSubpixel>();
			AddCellLine(iRow, sStartX, sStartY, sBoundary, y);
			sStartX = sBoundary;
			sStartY = y;
		}
	}
	AddCellLine(iRow, sStartX, sStartY, x2, y2);
}

//////////////////////////////////////////////////////////////////////////
//
// Adds a line inside one pixel.  The cover is its height and the area is
// twice the area between it and the pixel's left edge (times its
// height's sign).  Lines left of the buffer cover the first column
// completely; lines right of it are dropped.
//
//////////////////////////////////////////////////////////////////////////
void CCoverageRasteriser::AddCellLine(int iRow, SCoverageSubpixel x1, SCoverageSubpixel y1, SCoverageSubpixel x2, SCoverageSubpixel y2)
{
	SCoverageSubpixel	sHeight;
	int					iColumn;
	SCoverageSubpixel	sLeft;
	SCoverageSubpixel	sX1;
	SCoverageSubpixel	sX2;
	lsize				uiCell;

	sHeight = y2 - y1;
	if (sHeight.miRaw == 0)
	{
		return;
	}

	iColumn = ((x1 < x2) ? x1 : x2).Floor();
	if (iColumn >= miWidth)
	{
		return;
	}
	if (iColumn < 0)
	{
		iColumn = 0;
		sX1 = SCoverageSubpixel::FromRaw(0);
		sX2 = SCoverageSubpixel::FromRaw(0);
	}
	else
	{
		sLeft = SCoverageSubpixel::FromInt(iColumn);
		sX1 = x1 - sLeft;
		sX2 = x2 - sLeft;
	}

	uiCell = (lsize)iRow * miWidth + iColumn;
	mpsCover[uiCell] += sHeight;
	mpsArea[uiCell] += ((sX1 + sX2) * sHeight).To<SCoverageArea>();
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
	SCoverageSubpixel	sCover;
	SCoverageArea		sCoverage;
	SCoverageArea		sFull;

	sFull = SCoverageArea::FromInt(COVERAGE_FULL_PIXELS);
	for (y = 0; y < (uint)miHeight; y++)
	{
		sCover = SCoverageSubpixel::FromRaw(0);
		for (x = 0; x < (uint)miWidth; x++)
		{
			uiCell = (lsize)y * miWidth + x;
			sCover += mpsCover[uiCell];
			sCoverage = sCover.To<SCoverageArea>() + sCover.To<SCoverageArea>() - mpsArea[uiCell];
			if (sCoverage.miRaw < 0)
			{
				sCoverage = -sCoverage;
			}
			if (sCoverage > sFull)
			{
				sCoverage = sFull;
			}
			//Rounded 255ths; the full area is a power of two and clamped first, so this is a shift in 32 bits.
			mpuiCoverage[uiCell] = (uint8)(((sCoverage.miRaw * 255 + sFull.miRaw / 2)) >> (SCoverageArea::Bits + 1));
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

