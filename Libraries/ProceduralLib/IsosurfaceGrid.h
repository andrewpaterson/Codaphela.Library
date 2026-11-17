/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ProceduralLib

Codaphela ProceduralLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ProceduralLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ProceduralLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __ISOSURFACE_GRID_H__
#define __ISOSURFACE_GRID_H__
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayTemplatePrimitive.h"
#include "BaseLib/Float32Bounds.h"
#include "SupportLib/Shape.h"


//The steps the search for a crossing takes along its edge at most.
#define ISOSURFACE_CROSSING_STEPS		16


//Where the surface crosses an edge of a grid, and the shape's gradient there made a unit: the plane the vertex fit wants.
struct SIsosurfaceCrossing
{
	SFloat32Vec3	sPosition;
	SFloat32Vec3	sNormal;
};


typedef CArrayLargeTemplate<SIsosurfaceCrossing>	CArrayIsosurfaceCrossing;
typedef CArrayLargeTemplatePrimitive<float32>		CArrayLargeFloat32;


//The one sign test of the isosurface: negative is inside, and exactly zero is outside, as a CShape's is.  A face whose cells'
//corners lie on a surface would otherwise come out twice or not at all.
bool	IsosurfaceInside(float32 fDistance);

//Where the surface crosses the edge from psStart to psEnd, whose distances differ by IsosurfaceInside, found to within fTolerance,
//and the shape's gradient there as a unit normal.
void	IsosurfaceFindCrossing(SIsosurfaceCrossing* psDest, CShape* pcShape, SFloat32Vec3* psStart, float32 fStartDistance, SFloat32Vec3* psEnd, float32 fEndDistance, float32 fTolerance);


//A uniform grid of cubic cells over a box (Isosurface.txt, step 8.2).  Sample works out the shape's distance at each corner once
//and, along each edge whose ends differ by IsosurfaceInside, where the surface crosses it and the shape's normal there.
//Marching cubes and dual contouring both start from it.  Corners and edges are counted from msOrigin along x, then y, then z.
class CIsosurfaceGrid
{
public:
	SFloat32Vec3				msOrigin;
	float32						mfCellSize;
	int							miCellsX;
	int							miCellsY;
	int							miCellsZ;
	CArrayLargeFloat32			mafDistances;  //For each corner.
	CArrayLargeInt				maiCrossings;  //For each corner, three: the crossing on the edge from it along x, y and z, or -1.
	CArrayIsosurfaceCrossing	masCrossings;

	void					Init(SFloat32Bounds* psBounds, float32 fCellSize);  //Cells of fCellSize, as few as cover psBounds, centred on it.
	void					Kill(void);

	void					Sample(CShape* pcShape);

	lsize					CornerIndex(int x, int y, int z);
	void					GetCornerPosition(SFloat32Vec3* psDest, int x, int y, int z);
	float32					GetCornerDistance(int x, int y, int z);
	SIsosurfaceCrossing*	GetCrossing(int x, int y, int z, int iAxis);  //On the edge from corner x, y, z along axis iAxis (0 is x), or NULL.
	int						NumCrossings(void) const;
};


#endif // __ISOSURFACE_GRID_H__

