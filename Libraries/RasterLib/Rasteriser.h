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
#include "BaseLib/Jobs.h"
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


class CRasteriser;


//A triangle that may draw, and the rows of the target it may draw on.
struct SRasterDrawable
{
	SRasterTriangle*	psTriangle;
	int32				iTop;
	int32				iBottom;
};


//Some of an instance's vertices to project.
struct SRasterProjectJob
{
	CRasteriser*			pcRasteriser;
	SRasterInstance*		psInstance;
	CRasterCamera*			pcCamera;
	SRasterUnitVec3*		psLight;
	SRasterScreenVertex*	psScreen;  //The instance's first vertex.
	size					uiFirst;
	size					uiNumVertices;
};


//Some of an instance's triangles to find the drawables in.  They are the order a band draws in.
struct SRasterListJob
{
	CRasteriser*			pcRasteriser;
	CRasterTarget*			pcTarget;
	SRasterTriangle*		psTriangles;
	size					uiNumTriangles;
	SRasterScreenVertex*	psScreen;  //The instance's first vertex.
	SRasterDrawable*		psDrawables;  //Room for all its triangles.
	size					uiNumDrawables;
	int32					iTop;  //Of all its drawables.
	int32					iBottom;
	bool					bTransparent;
};


//Some rows of the target to draw.
struct SRasterBandJob
{
	CRasteriser*	pcRasteriser;
	CRasterTarget*	pcTarget;
	int32			iTop;
	int32			iBottom;
};


typedef CArrayTemplate<SRasterProjectJob>	CArrayRasterProjectJob;
typedef CArrayTemplate<SRasterListJob>		CArrayRasterListJob;
typedef CArrayTemplate<SRasterBandJob>		CArrayRasterBandJob;
typedef CArrayTemplate<SRasterDrawable>		CArrayRasterDrawable;


//Draws a CRasterMesh with integer arithmetic only, so that every build draws the same pixels (given the same camera axes).
//Triangles are filled by half-space edge functions with the top-left rule so that triangles sharing an edge never leave
//a gap or draw a pixel twice.  Opaque triangles write depth.  Transparent ones are drawn after them, unsorted, testing
//depth without writing it.  With instances that goes for all of them: every opaque triangle is drawn before any
//transparent one.
//
//Given jobs that can run at the same time (SetJobs), instances are drawn by them: vertices are projected, then the triangles
//that may draw are found, then the target is drawn a band of rows at a time.  A band draws its triangles in the order they'd
//be drawn in without jobs, and no two bands touch the same pixel or depth, so every byte drawn is the same however many
//threads there are and whichever order the jobs run in.
class CRasteriser
{
protected:
	CArrayRasterScreenVertex	masScreen;  //Only ever grows, as a frame's vertices are about the same as the last's.
	SRasterUnitVec3				msLight;  //Towards the light, on the screen.
	bool						mbCullBackFaces;
	CJobs*						mpcJobs;  //What draws instances (SetJobs), NULL to draw them on the caller's thread.
	CArrayRasterProjectJob		masProjectJobs;
	CArrayRasterListJob			masListJobs;
	CArrayRasterBandJob			masBandJobs;
	CArrayRasterDrawable		masDrawables;

public:
	void	Init(void);
	void	Kill(void);

	void	SetCullBackFaces(bool bCullBackFaces);
	void	SetJobs(CJobs* pcJobs);

	void	Draw(CRasterMesh* pcMesh, CRasterCamera* pcCamera, CRasterTarget* pcTarget);
	void	Draw(SRasterInstance* asInstances, size uiNumInstances, CRasterCamera* pcCamera, CRasterTarget* pcTarget);
	void	DrawTriangle(CRasterTarget* pcTarget, SRasterScreenVertex* psA, SRasterScreenVertex* psB, SRasterScreenVertex* psC, uint32 uiColour, bool bTransparent);

protected:
	void	GrowScreen(size uiNumVertices);
	void	DrawWithJobs(SRasterInstance* asInstances, size uiNumInstances, CRasterCamera* pcCamera, SRasterUnitVec3* psLight, CRasterTarget* pcTarget);
	void	AddListJobs(CArrayRasterTriangle* pasTriangles, SRasterScreenVertex* psScreen, CRasterTarget* pcTarget, bool bTransparent, SRasterDrawable** ppsDrawables);
	void	DrawTriangleRows(CRasterTarget* pcTarget, SRasterScreenVertex* psA, SRasterScreenVertex* psB, SRasterScreenVertex* psC, uint32 uiColour, bool bTransparent, int32 iTop, int32 iBottom);
	void	ProjectInstance(SRasterInstance* psInstance, CRasterCamera* pcCamera, SRasterUnitVec3* psLight, SRasterScreenVertex* psScreen, size uiFirst, size uiNumVertices);
	void	DrawTriangles(CArrayRasterTriangle* pasTriangles, SRasterScreenVertex* psScreen, CRasterTarget* pcTarget, bool bTransparent);

	static void	ProjectJob(void* pvJob);
	static void	ListJob(void* pvJob);
	static void	BandJob(void* pvJob);
};


#endif // __RASTERISER_H__

