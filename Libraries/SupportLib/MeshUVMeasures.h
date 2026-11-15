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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#ifndef __MESH_UV_MEASURES_H__
#define __MESH_UV_MEASURES_H__
#include "Mesh.h"


//How much a UV layer's faces are stretched against their 3D shapes (MeasureMeshUVStretch), in texels: a layout fLayoutWidth by
//fLayoutHeight texels across UVs 0 to 1, at fTexelsPerUnit texels to a unit of the mesh.  A face's map from 3D to texels
//stretches it by two amounts at right angles (its singular values): their ratio is how far its angles change, and their product
//against fTexelsPerUnit squared how far its texels per area are from the density.
struct SMeshUVStretch
{
	float32	fWorstAngle;  //The largest ratio, 1 if every face keeps its shape.
	float32	fMeanAngle;  //The ratio weighted by the faces' areas.
	float32	fLeastArea;  //The least product against the density squared,
	float32	fMostArea;  //and the most: 1 for both is the density everywhere.
	int		iNumFaces;  //Faces measured: those with UVs and area in 3D and in UV.
};


//What an unwrap is judged by (UnwrapPlan.txt, step 1), of UV layer iLayer.  A face anticlockwise seen from outside is to be
//anticlockwise in its UVs, with u to the right and v up; one that isn't, or has no area in UV, is turned over.

//Faces with no UVs, or with any of their corners' UVs missing.
int		CountMeshUVFacesMissing(CMesh* pcMesh, int iLayer);

//Faces turned over in UV.
int		CountMeshUVFacesTurned(CMesh* pcMesh, int iLayer);

//UVs outside 0 to 1.
int		CountMeshUVsOutside(CMesh* pcMesh, int iLayer);

//Pairs of faces whose UV triangles overlap: whose insides share some area, not just an edge or a corner.
int		CountMeshUVOverlaps(CMesh* pcMesh, int iLayer);

void	MeasureMeshUVStretch(SMeshUVStretch* psDest, CMesh* pcMesh, int iLayer, float32 fLayoutWidth, float32 fLayoutHeight, float32 fTexelsPerUnit);


//The same of any triangles (a chart's, as it's laid flat): iNumTriangles of them, their corners three at a time in paiCorners,
//indices into pasUVs (in texels) and pasPositions (each UV's position).
int		CountUVTrianglesTurned(SFloat32Vec2* pasUVs, int* paiCorners, int iNumTriangles);
int		CountUVTriangleOverlaps(SFloat32Vec2* pasUVs, int* paiCorners, int iNumTriangles);
void	MeasureUVTriangleStretch(SMeshUVStretch* psDest, SFloat32Vec2* pasUVs, SFloat32Vec3* pasPositions, int* paiCorners, int iNumTriangles, float32 fTexelsPerUnit);


#endif // __MESH_UV_MEASURES_H__

