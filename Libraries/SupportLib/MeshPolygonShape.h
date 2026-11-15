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
#ifndef __MESH_POLYGON_SHAPE_H__
#define __MESH_POLYGON_SHAPE_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTemplatePrimitive.h"
#include "BaseLib/PrimitiveTypes.h"


class CMesh;
class CMeshPolygon;


//What's the same about any file that holds polygons: cutting one into a mesh's faces when reading it, and finding its way around
//them when writing it.

//An edge of a face, going the way the face goes around.
struct SMeshPolygonEdge
{
	int		iFrom;
	int		iTo;
	int		iFace;
	int		iFaceCorner;
};


typedef CArrayTemplate<SMeshPolygonEdge>	CArrayMeshPolygonEdge;
typedef CArrayTemplatePrimitive<int32>		CArrayMeshPolygonCorners;


//Whether two of a polygon's corners are next to each other around it, so that the edge between them is on its outside.
bool	IsMeshPolygonEdge(int iCorner1, int iCorner2, int iNumCorners);

//How much flatter, as the cosine of a fold, one way of cutting a polygon must be than another to be taken instead.  Below it they're
//the same, as a flat polygon's cuts are once rounded.
#define MESH_POLYGON_FOLD_TOLERANCE		1e-9

//Whether a quad is cut between its first and third corners (faces 1 2 3 and 1 3 4) instead of its second and fourth (faces 1 2 4 and
//2 3 4, as a flat convex quad is).  A concave quad is cut through its inward corner, so neither face is outside it, and a bent quad
//the way its faces fold least.
bool	IsMeshQuadCutFirstToThird(CMesh* pcMesh, int iCorner1, int iCorner2, int iCorner3, int iCorner4);

//paiPositions has a position for each of its corners.  paiFaceCorners gets three of those corners for each face to make: each time
//the ear that folds least against what's left, else the first, so a concave polygon's faces are inside it and a bent one's creases
//are as small as they can be.
void	CutMeshPolygon(CMesh* pcMesh, const int32* paiPositions, int32 iNumCorners, CArrayMeshPolygonCorners* paiFaceCorners);

//pasEdges gets the edges of all a polygon's faces and pasLoop the ones around its outside, in order.  False unless they are one loop
//with every corner of the polygon on it.
bool	GetMeshPolygonLoop(CMesh* pcMesh, CMeshPolygon* pcPolygon, CArrayMeshPolygonEdge* pasEdges, CArrayMeshPolygonEdge* pasLoop);


#endif // __MESH_POLYGON_SHAPE_H__

