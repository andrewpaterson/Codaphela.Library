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
#ifndef __RASTERISER_H__
#define __RASTERISER_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Int64Vec3.h"
#include "RasterCamera.h"
#include "RasterMesh.h"
#include "RasterTarget.h"


typedef CArrayTemplate<SRasterScreenVertex>	CArrayRasterScreenVertex;


//Draws a CRasterMesh with integer arithmetic only, so that every build draws the same pixels (given the same camera axes).
//Triangles are filled by half-space edge functions with the top-left rule so that triangles sharing an edge never leave
//a gap or draw a pixel twice.  Opaque triangles write depth.  Transparent ones are drawn after them, unsorted, testing
//depth without writing it.
class CRasteriser
{
protected:
	CArrayRasterScreenVertex	masScreen;
	SInt64Vec3					msLight;  //Towards the light, on the screen, with RASTER_NORMAL_BITS.
	bool						mbCullBackFaces;

public:
	void	Init(void);
	void	Kill(void);

	void	SetCullBackFaces(bool bCullBackFaces);

	void	Draw(CRasterMesh* pcMesh, CRasterCamera* pcCamera, CRasterTarget* pcTarget);
	void	DrawTriangle(CRasterTarget* pcTarget, SRasterScreenVertex* psA, SRasterScreenVertex* psB, SRasterScreenVertex* psC, uint32 uiColour, bool bTransparent);

protected:
	void	DrawTriangles(CArrayRasterTriangle* pasTriangles, CRasterTarget* pcTarget, bool bTransparent);
};


#endif // __RASTERISER_H__

