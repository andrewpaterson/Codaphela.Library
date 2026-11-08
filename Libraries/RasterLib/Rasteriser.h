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
#include "RasterCamera.h"
#include "RasterMesh.h"
#include "RasterTarget.h"


typedef CArrayTemplate<SRasterScreenVertex>	CArrayRasterScreenVertex;


//A mesh somewhere: its positions and normals turned by asAxes, then its positions moved by sOffset.  Any number can share
//one mesh, which is only held once.  asAxes are what a position is dotted with for the new x, y and z, with
//RASTER_AXIS_BITS.  They must turn it, not mirror it, or its faces would wind the wrong way.
struct SRasterInstance
{
	CRasterMesh*	pcMesh;
	SInt32Vec3		asAxes[3];
	SInt32Vec3		sOffset;
};


//Draws a CRasterMesh with integer arithmetic only, so that every build draws the same pixels (given the same camera axes).
//Triangles are filled by half-space edge functions with the top-left rule so that triangles sharing an edge never leave
//a gap or draw a pixel twice.  Opaque triangles write depth.  Transparent ones are drawn after them, unsorted, testing
//depth without writing it.  With instances that goes for all of them: every opaque triangle is drawn before any
//transparent one.
class CRasteriser
{
protected:
	CArrayRasterScreenVertex	masScreen;  //Only ever grows, as a frame's vertices are about the same as the last's.
	SRasterUnitVec3				msLight;  //Towards the light, on the screen.
	bool						mbCullBackFaces;

public:
	void	Init(void);
	void	Kill(void);

	void	SetCullBackFaces(bool bCullBackFaces);

	void	Draw(CRasterMesh* pcMesh, CRasterCamera* pcCamera, CRasterTarget* pcTarget);
	void	Draw(SRasterInstance* asInstances, size uiNumInstances, CRasterCamera* pcCamera, CRasterTarget* pcTarget);
	void	DrawTriangle(CRasterTarget* pcTarget, SRasterScreenVertex* psA, SRasterScreenVertex* psB, SRasterScreenVertex* psC, uint32 uiColour, bool bTransparent);

protected:
	void	GrowScreen(size uiNumVertices);
	void	ProjectInstance(SRasterInstance* psInstance, CRasterCamera* pcCamera, SRasterUnitVec3* psLight, SRasterScreenVertex* psScreen);
	void	DrawTriangles(CArrayRasterTriangle* pasTriangles, SRasterScreenVertex* psScreen, CRasterTarget* pcTarget, bool bTransparent);
};


#endif // __RASTERISER_H__

