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
#ifndef __ISOSURFACE_OCTREE_H__
#define __ISOSURFACE_OCTREE_H__
#include "BaseLib/ArrayTemplate.h"
#include "SupportLib/MeshEditor.h"
#include "IsosurfaceGrid.h"
#include "IsosurfaceFit.h"
#include "DualContouring.h"


//A cell of the octree, in units of the smallest cell.
struct SIsosurfaceNode
{
	int		aiCorner[3];  //Its least corner.
	int		iSize;  //A power of two.
	int		iFirstChild;  //Its eight children follow, numbered by their corners' x, y and z as bits 0, 1 and 2.  -1 for a leaf.
};


//The faces of one crossed minimal edge: the vertices of the leaves around it (their fits), anticlockwise seen from outside.  Three
//where one leaf is two of the four places around it.
struct SIsosurfacePolygon
{
	int				aiVertices[4];
	int				iNumVertices;
	SFloat32Vec3	sNormal;  //The surface's at its edge's crossing.
};


typedef CArrayLargeTemplate<SIsosurfaceNode>	CArrayIsosurfaceNode;
typedef CArrayLargeTemplate<SIsosurfacePolygon>	CArrayIsosurfacePolygon;
typedef CArrayLargeTemplate<SIsosurfaceFit>		CArrayIsosurfaceFit;


//Dual contouring on an octree (Isosurface.txt, step 8.6), so detail goes where the shape needs it.  Build splits a cell from the
//root down, to the smallest size at most:
//  not at all where its centre is further from the surface than half its diagonal (there's no surface in it, even by a bound),
//  always while it's bigger than the largest size,
//  and where its centre's distance differs from its corners' average by more than mfCentreTolerance (a notch or bump thinner
//  than it, or a curve or crease the corners can't see) or its crossings' normals differ by more than the split angle.
//Generate makes faces on the minimal edges, those of the smallest leaf around them (Ju et al. 2002): each that crosses is one
//quad or triangle of the leaves around it, and adds its crossing to each of their fits, so a big leaf beside small ones has a
//vertex where their edges cross its faces.  A crossing counts in a leaf's fit by its edge's length over the leaf's size, so a
//big leaf's many small neighbours don't pull its vertex over theirs.  Nothing cracks between sizes, as every face is made from the smallest edges.
//Then it's balanced: a leaf beside a cube of its size split twice is split, so leaves beside each other differ by one size at
//most, as a big leaf among much smaller ones folds faces over.  A leaf of the smallest size has a vertex for each sheet of surface
//in it, as CDualContouring's cells do (manifold dual contouring, step 8.8); a bigger one has one, as its normals agree.
//Corner distances are worked out again where cells meet: Distance is the same each time, so cells agree on their signs.
class CIsosurfaceOctree
{
public:
	SFloat32Vec3			msOrigin;
	float32					mfSmallest;
	float32					mfLargest;
	float32					mfSplitCosine;
	float32					mfCentreTolerance;
	CArrayIsosurfaceNode	masNodes;  //The root first.

	void	Init(SFloat32Bounds* psBounds, float32 fSmallest, float32 fLargest, float32 fSplitAngle, float32 fCentreTolerance);  //Radians.
	void	Kill(void);

	void	Build(CShape* pcShape);
	void	Generate(CMeshEditor* pcMeshEditor, CShape* pcShape, int iFaceName = 0);

	int		NumLeaves(void);
	int		FindLeaf(int x2, int y2, int z2);  //The leaf holding the point at these halves of the smallest cell, or -1 outside.
	void	GetPosition(SFloat32Vec3* psDest, int x, int y, int z);

protected:
	void	BuildNode(CShape* pcShape, int iNode);
	void	SplitNode(int iNode);
	int		FindNode(int x, int y, int z, int iSize);
	bool	Balance(void);
	bool	ShouldSplit(CShape* pcShape, SIsosurfaceNode* psNode);
	int		FindPieces(CShape* pcShape, CArrayDualContouringPieces* pasPieces, CArrayLargeInt* paiFirstVertices, CArrayLargeInt* paiVertexLeaves);
	void	FindPolygons(CShape* pcShape, CArrayIsosurfacePolygon* pasPolygons, CArrayIsosurfaceFit* pasFits, CArrayDualContouringPieces* pasPieces, CArrayLargeInt* paiFirstVertices);
};


#endif // __ISOSURFACE_OCTREE_H__

