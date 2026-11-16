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
#ifndef __QUICK_HULL_H__
#define __QUICK_HULL_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayInt.h"
#include "BaseLib/Float32Plane.h"


//How far in front of a face a point must be to be outside the hull, as a fraction of how far the points reach along x, y and z
//together.
#define QUICK_HULL_TOLERANCE	(1e-5f)


//A face of a hull: a polygon of coplanar triangles, its corners in CQuickHull's list from uiFirstCorner, going anticlockwise seen
//from outside, and its plane facing out.
struct SQuickHullFace
{
	SFloat32Plane	sPlane;
	size			uiFirstCorner;
	size			uiNumCorners;
};

typedef CArrayTemplate<SQuickHullFace>	CArrayQuickHullFace;


//A triangle of the hull while it's being made.  Its neighbour across edge i (from corner i to corner i + 1) is aiNeighbours[i],
//and aiOutside holds the points in front of it that no triangle before it took.
struct SQuickHullTriangle
{
	int				aiCorners[3];
	int				aiNeighbours[3];
	SFloat32Plane	sPlane;
	CArrayInt		aiOutside;
	int				iVisited;
	int				iGroup;
	bool			bDeleted;
};

typedef CArrayTemplate<SQuickHullTriangle>	CArrayQuickHullTriangle;


//Points to their convex hull (Barber, Dobkin and Huhdanpaa's Quickhull, 1996).  It starts from a tetrahedron, gives each point
//to a triangle it's in front of, and grows by the point furthest in front of a triangle until none is left in front of any.
//A point is only in front by more than a tolerance scaled to how far the points reach from the origin, so points on a face,
//or as good as, don't make slivers.  A face is the triangles that end up in one plane, merged.
//Generate fails, with no faces, given fewer than four points or points that are all in one plane.
class CQuickHull
{
protected:
	SFloat32Vec3*			mpsPoints;
	size					muiStride;
	size					muiNumPoints;
	float32					mfTolerance;
	CArrayQuickHullTriangle	mcTriangles;
	int						miVisit;
	CArrayQuickHullFace		mcFaces;
	CArrayInt				maiCorners;

public:
	void			Init(void);
	void			Kill(void);

	bool			Generate(SFloat32Vec3* psPoints, size uiStride, size uiNumPoints);

	size			NumFaces(void) const;
	SQuickHullFace*	GetFace(size uiFace);
	int				GetCorner(SQuickHullFace* psFace, size uiCorner);  //A point's index.
	void			GetVertices(CArrayInt* paiPoints);  //The points that are corners, each once, in order.
	float32			GetTolerance(void) const;

protected:
	SFloat32Vec3*	GetPoint(int iPoint);
	void			KillTriangles(void);
	void			KillFaces(void);
	float32			CalculateTolerance(void);
	bool			MakeTetrahedron(void);
	int				AddTriangle(int i1, int i2, int i3);
	void			LinkNeighbours(int iTriangle1, int iTriangle2);
	int				FindTriangleWithOutside(void);
	int				FindFurthestOutside(int iTriangle);
	bool			AddPoint(int iTriangle, int iEye);
	void			GiveToTriangles(int iPoint, CArrayInt* paiTriangles);
	void			MakeFaces(void);
	bool			MakeFace(int iSeed, int iGroup);
	void			AddTriangleFace(int iTriangle);
};


#endif // __QUICK_HULL_H__

