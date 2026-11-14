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
#ifndef __RASTER_POSED_H__
#define __RASTER_POSED_H__
#include "RasterMesh.h"


//Where one of a skinned mesh's matrices puts a vertex this frame, as an SRasterInstance's axes do: a position is dotted with
//asAxes for the new x, y and z, with RASTER_AXIS_BITS, then moved by sOffset, and a normal is turned by asLightAxes, the
//rotation of asAxes alone.  A bone mustn't mirror.
struct SRasterBone
{
	SInt32Vec3	asAxes[3];
	SInt32Vec3	asLightAxes[3];
	SInt32Vec3	sOffset;
};


//An instance's own vertices: a skinned mesh posed by its bones, in the space the instances are put in, one for each of the
//mesh's vertices, and their bounds.  Pose makes them, with integers only, so every build poses the same.  A vertex is where
//each of its matrices puts it, each by its weight, and its normal is turned by each, by its weight, and made a unit again.  A
//vertex with no weights stays where the mesh has it.
//
//It's kept from one frame to the next, as its vertices are about as many each time.
class CRasterPosed
{
public:
	CArrayRasterVertex	masVertices;
	SInt32Vec3			msMin;
	SInt32Vec3			msMax;

	void	Init(void);
	void	Kill(void);

	bool	Pose(CRasterMesh* pcMesh, SRasterBone* asBones, size uiNumBones);
};


#endif // __RASTER_POSED_H__

