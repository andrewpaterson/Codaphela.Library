/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ShapeLib

Codaphela ShapeLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ShapeLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ShapeLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __CONVEX_HULL_H__
#define __CONVEX_HULL_H__
#include "BaseLib/GeometricTypes.h"
#include "BaseLib/FreeList.h"
#include "Triangle.h"
#include "Polygon.h"


class CExtremeTriangle : public CTriangle
{
public:
	CArrayInt	maiVisible;

	void	Init(SFloat3* psPoint1, SFloat3* psPoint2, SFloat3* psPoint3, SFloat3* psNormal);
	void	Kill(void);
	int		FindFurthestPoint(SFloat3* psPoints, int iStride);
	bool	NotContains(SFloat3* psPosition);
};



typedef CArrayTemplate<CExtremeTriangle*>	CArrayExtremeTrianglePtr;
typedef CArrayTemplate<CTriangle*>			CArrayTrianglePtr;


struct SConvexHullHoleEdge
{
	size	iNumEdges;
	int		aaiEdgeIndices[3][2];  //1 triangle 2 edge indices, ... 3 triangles 2 edge indices.
};


class CConvexHullGenerator
{
public:
	CFreeList	mcNormals;
	CFreeList	mcTriangles;
	SFloat3*	mpsPoints;
	size		iStride;
	size		iNumPoints;
	char*		mszHullName;

	void 					Init(SFloat3* psPoints, int iStride, size iNumPoints, char* szHullName = NULL);
	void 					Kill(void);
	bool					Generate(void);
	int						FindMaxX(float* px);
	int						FindMinX(float* px);
	int						FindFurthestPoint(int iMaxXIndex, int iMinXIndex);
	CExtremeTriangle*		AddTriangle(SFloat3* psPoint1, SFloat3* psPoint2, SFloat3* psPoint3);
	void					FindEdges(SConvexHullHoleEdge* psEdges, CExtremeTriangle* pcTriangleWithEdges, CArrayExtremeTrianglePtr* papcDeletedTriangles);
	bool					TrianglesHaveEdge(int iEdge1, int iEdge2, CExtremeTriangle* pcTriangleWithEdges, CArrayExtremeTrianglePtr* papcDeletedTriangles);
	bool					TriangleHasEdge(int iEdge1, int iEdge2, CExtremeTriangle* pcTriangle);
	bool					TriangleAdjacent(CExtremeTriangle* pcTriangle1, CExtremeTriangle* pcTriangle2);
	bool					Contained(SFloat3* psPosition);
	bool					NotContained(SFloat3* psPosition);
	bool					RemoveDiscontiguousTriangles(CExtremeTriangle* pcSelected, CArrayExtremeTrianglePtr* papcTriangles, CArrayExtremeTrianglePtr* papcTemp);
	bool					FindFirstPairTriangles(CArrayExtremeTrianglePtr* apcTriangles, int iMaxXIndex, int iMinXIndex, int iFarIndex);
	void					AddPointsFromTriangles(CExtremeTriangle* pcTriangle, CArrayExtremeTrianglePtr* papcTriangles, int iDontAdd);
	CExtremeTriangle*		FindAdjacentTriangle(CExtremeTriangle* pcTriangle, int iPositionIndex0, int iPositionIndex1);
	void					RemoveSlivers(void);
	void					MoveTrianglesPointFrom(int iOldIndex, int iNewIndex);
	CFreeList*				GetNormals(void);
	void					DumpTriangleObj(CChars* psz, int iLoop);
};


void ConvertTrianglesToPolygons(CArrayTrianglePtr* papcTriangles, CArrayPolygons* pacPolygons);


class CConvexHull
{
public:
	CArrayPolygons	mcPolygons;

	void 	Init(void);
	void 	Kill(void);
	void	BeginSetFromPoints(CConvexHullGenerator* psConvexHullGenerator, SFloat3* psPoints, int iStride, int iNumPoints, char* szHullName);
	void	EndSetFromPoints(SFloat3* psNormals, int iStride, CConvexHullGenerator* psConvexHullGenerator);
	void	GetIndices(CArrayInt* paiIndices, SFloat3* psPoints, int iStride);
	void	GetVertices(CArrayBlock* pasPositions, SFloat3* psPoints, int iStride);  //The positions are COPIED into this array.  It's not an array of pointers
};


#endif // __CONVEX_HULL_H__

