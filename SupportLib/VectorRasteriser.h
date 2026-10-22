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
#ifndef __VECTOR_RASTERISER_H__
#define __VECTOR_RASTERISER_H__
#include "CoverageRasteriser.h"
#include "VectorPath.h"


#define VECTOR_ARC_MAX_STEPS	65536


//Fills a vector path into a grid of coverage from 0 (empty) to 255 (covered).  The path is placed in pixels by an
//origin, a scale and which way y goes; x pixels are (x - origin) * scale and y pixels are (origin - y) * scale when y
//is up or (y - origin) * scale when it is down.  The parts of the path are combined in order: exposure on adds and
//exposure off erases.
//
//Arcs are flattened to lines about the tolerance (pixels) from the arc, either side of it so areas are kept.  The
//vertices are on a grid of angles shared by every arc of a circle and symmetric about the axes and the diagonals, so
//shapes that are mirror images or pieces of each other cover the same pixels the same way.  Quarter turns can differ
//by one in 255 because the coverage rasteriser cuts edges at rows and columns differently.  All the coverage
//arithmetic is integer.
class CVectorRasteriser
{
protected:
	CCoverageRasteriser		mcCoverage;
	uint8*					mpuiPath;
	size					muiPathCapacity;
	uint8*					mpuiResult;
	float64					mfTolerance;

	SFloat64Vec2			msOrigin;
	float64					mfScale;
	bool					mbYUp;

	int						miLeft;
	int						miTop;
	int						miWidth;
	int						miHeight;

public:
	void		Init(float64 fTolerance = 0.05);
	void		Kill(void);

	void		SetTransform(SFloat64Vec2* psOrigin, float64 fPixelsPerUnit, bool bYUp);
	bool		GetPixelBounds(CVectorPath* pcPath, int* piLeft, int* piTop, int* piRight, int* piBottom);

	bool		Rasterise(CVectorPath* pcPath, int iLeft, int iTop, int iWidth, int iHeight);
	uint8*		GetCoverage(void);
	int			GetLeft(void);
	int			GetTop(void);
	int			GetWidth(void);
	int			GetHeight(void);

protected:
	bool		RasterisePart(CVectorPath* pcPath, size uiStart, size uiEnd);
	void		AddArc(SFloat64Vec2* psStart, SFloat64Vec2* psEnd, SFloat64Vec2* psCenter, bool bClockwise);
	void		AddLine(SFloat64Vec2* psStart, SFloat64Vec2* psEnd);
	float64		ToPixelX(float64 x);
	float64		ToPixelY(float64 y);
	int32		ToSubpixel(float64 fPixel, int iCorner);
	int			ArcSteps(float64 fPixelRadius);
	void		GridCosSin(int64 iIndex, int iSteps, float64* pfCos, float64* pfSin);
};


#endif // __VECTOR_RASTERISER_H__

