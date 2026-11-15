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
#include <math.h>
#include <string.h>
#include "MarchingCubes.h"
#include "IsosurfaceOctree.h"


//////////////////////////////////////////////////////////////////////////
//
// The root is the fewest smallest cells, a power of two, that cover the
// box along its longest side, centred on it.
//
//////////////////////////////////////////////////////////////////////////
void CIsosurfaceOctree::Init(SFloat32Bounds* psBounds, float32 fSmallest, float32 fLargest, float32 fSplitAngle, float32 fCentreTolerance)
{
	SFloat32Vec3		sSize;
	SFloat32Vec3		sCenter;
	float32				fLongest;
	int					iRootSize;
	SIsosurfaceNode*	psRoot;

	mfSmallest = fSmallest;
	mfLargest = fLargest;
	mfSplitCosine = cosf(fSplitAngle);
	mfCentreTolerance = fCentreTolerance;

	psBounds->GetSize(&sSize);
	psBounds->GetCenter(&sCenter);
	fLongest = fmaxf(sSize.x, fmaxf(sSize.y, sSize.z));
	iRootSize = 1;
	while (((float32)iRootSize * fSmallest) < fLongest)
	{
		iRootSize *= 2;
	}
	msOrigin.Init(sCenter.x - ((float32)iRootSize * fSmallest * 0.5f), sCenter.y - ((float32)iRootSize * fSmallest * 0.5f), sCenter.z - ((float32)iRootSize * fSmallest * 0.5f));

	masNodes.Init();
	psRoot = masNodes.Add();
	psRoot->aiCorner[0] = 0;
	psRoot->aiCorner[1] = 0;
	psRoot->aiCorner[2] = 0;
	psRoot->iSize = iRootSize;
	psRoot->iFirstChild = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIsosurfaceOctree::Kill(void)
{
	masNodes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIsosurfaceOctree::Build(CShape* pcShape)
{
	BuildNode(pcShape, 0);
	while (Balance())
	{
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Splits each leaf with a cube of its size beside it (across a face, an
// edge or a corner) that is split twice, so leaves beside each other
// differ by one size at most.  True if any was split.
//
//////////////////////////////////////////////////////////////////////////
bool CIsosurfaceOctree::Balance(void)
{
	SIsosurfaceNode		sLeaf;
	SIsosurfaceNode*	psNeighbour;
	int					iNode;
	int					iNumNodes;
	int					iNeighbour;
	int					dx;
	int					dy;
	int					dz;
	int					iChild;
	bool				bSplit;
	bool				bAny;

	bAny = false;
	iNumNodes = (int)masNodes.NumElements();
	for (iNode = 0; iNode < iNumNodes; iNode++)
	{
		sLeaf = *masNodes.Get((lsize)iNode);
		if ((sLeaf.iFirstChild != -1) || (sLeaf.iSize <= 1))
		{
			continue;
		}

		bSplit = false;
		for (dz = -1; (dz <= 1) && !bSplit; dz++)
		{
			for (dy = -1; (dy <= 1) && !bSplit; dy++)
			{
				for (dx = -1; (dx <= 1) && !bSplit; dx++)
				{
					iNeighbour = FindNode(sLeaf.aiCorner[0] + (dx * sLeaf.iSize), sLeaf.aiCorner[1] + (dy * sLeaf.iSize), sLeaf.aiCorner[2] + (dz * sLeaf.iSize), sLeaf.iSize);
					if (iNeighbour == -1)
					{
						continue;
					}
					psNeighbour = masNodes.Get((lsize)iNeighbour);
					if (psNeighbour->iFirstChild == -1)
					{
						continue;
					}
					for (iChild = 0; iChild < 8; iChild++)
					{
						if (masNodes.Get((lsize)(psNeighbour->iFirstChild + iChild))->iFirstChild != -1)
						{
							bSplit = true;
							break;
						}
					}
				}
			}
		}
		if (bSplit)
		{
			SplitNode(iNode);
			bAny = true;
		}
	}
	return bAny;
}


//////////////////////////////////////////////////////////////////////////
//
// The node of iSize with its least corner at x, y, z, or -1 if there is
// none (it's outside, or in a bigger leaf).
//
//////////////////////////////////////////////////////////////////////////
int CIsosurfaceOctree::FindNode(int x, int y, int z, int iSize)
{
	SIsosurfaceNode*	psNode;
	int					iNode;
	int					iHalf;
	int					iChild;

	psNode = masNodes.Get(0);
	if ((x < 0) || (y < 0) || (z < 0) || (x >= psNode->iSize) || (y >= psNode->iSize) || (z >= psNode->iSize))
	{
		return -1;
	}

	iNode = 0;
	while (psNode->iSize > iSize)
	{
		if (psNode->iFirstChild == -1)
		{
			return -1;
		}
		iHalf = psNode->iSize / 2;
		iChild = 0;
		if (x >= (psNode->aiCorner[0] + iHalf))
		{
			iChild |= 1;
		}
		if (y >= (psNode->aiCorner[1] + iHalf))
		{
			iChild |= 2;
		}
		if (z >= (psNode->aiCorner[2] + iHalf))
		{
			iChild |= 4;
		}
		iNode = psNode->iFirstChild + iChild;
		psNode = masNodes.Get((lsize)iNode);
	}
	return iNode;
}


//////////////////////////////////////////////////////////////////////////
//
// The node array grows as children are added, so nodes are held by index.
//
//////////////////////////////////////////////////////////////////////////
void CIsosurfaceOctree::BuildNode(CShape* pcShape, int iNode)
{
	SIsosurfaceNode		sNode;
	int					iFirstChild;
	int					iChild;

	sNode = *masNodes.Get((lsize)iNode);
	if (!ShouldSplit(pcShape, &sNode))
	{
		return;
	}

	SplitNode(iNode);
	iFirstChild = masNodes.Get((lsize)iNode)->iFirstChild;
	for (iChild = 0; iChild < 8; iChild++)
	{
		BuildNode(pcShape, iFirstChild + iChild);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Eight leaves half its size.
//
//////////////////////////////////////////////////////////////////////////
void CIsosurfaceOctree::SplitNode(int iNode)
{
	SIsosurfaceNode		sNode;
	SIsosurfaceNode*	psChild;
	int					iFirstChild;
	int					iChild;
	int					iHalf;

	sNode = *masNodes.Get((lsize)iNode);
	iFirstChild = (int)masNodes.NumElements();
	iHalf = sNode.iSize / 2;
	for (iChild = 0; iChild < 8; iChild++)
	{
		psChild = masNodes.Add();
		psChild->aiCorner[0] = sNode.aiCorner[0] + ((iChild & 1) * iHalf);
		psChild->aiCorner[1] = sNode.aiCorner[1] + (((iChild >> 1) & 1) * iHalf);
		psChild->aiCorner[2] = sNode.aiCorner[2] + (((iChild >> 2) & 1) * iHalf);
		psChild->iSize = iHalf;
		psChild->iFirstChild = -1;
	}
	masNodes.Get((lsize)iNode)->iFirstChild = iFirstChild;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIsosurfaceOctree::ShouldSplit(CShape* pcShape, SIsosurfaceNode* psNode)
{
	SFloat32Vec3			asCorners[MARCHING_CUBES_CORNERS];
	float32					afDistances[MARCHING_CUBES_CORNERS];
	SFloat32Vec3			asNormals[MARCHING_CUBES_EDGES];
	SIsosurfaceCrossing		sCrossing;
	SFloat32Vec3			sCentre;
	float32					fCellSize;
	float32					fCentreDistance;
	float32					fAverage;
	int						iCorner;
	int						iEdge;
	int						iStart;
	int						iEnd;
	int						iNumNormals;
	int						i;
	int						j;

	if (psNode->iSize <= 1)
	{
		return false;
	}

	fCellSize = (float32)psNode->iSize * mfSmallest;
	GetPosition(&sCentre, psNode->aiCorner[0], psNode->aiCorner[1], psNode->aiCorner[2]);
	sCentre += SFloat32Vec3(fCellSize * 0.5f, fCellSize * 0.5f, fCellSize * 0.5f);
	fCentreDistance = pcShape->Distance(&sCentre);
	if (fabsf(fCentreDistance) > (fCellSize * 0.8660254f))
	{
		return false;
	}
	if (fCellSize > (mfLargest * 1.0001f))
	{
		return true;
	}

	fAverage = 0.0f;
	for (iCorner = 0; iCorner < MARCHING_CUBES_CORNERS; iCorner++)
	{
		GetPosition(&asCorners[iCorner], psNode->aiCorner[0] + ((iCorner & 1) * psNode->iSize), psNode->aiCorner[1] + (((iCorner >> 1) & 1) * psNode->iSize), psNode->aiCorner[2] + (((iCorner >> 2) & 1) * psNode->iSize));
		afDistances[iCorner] = pcShape->Distance(&asCorners[iCorner]);
		fAverage += afDistances[iCorner];
	}
	fAverage /= (float32)MARCHING_CUBES_CORNERS;
	if (fabsf(fCentreDistance - fAverage) > mfCentreTolerance)
	{
		return true;
	}

	iNumNormals = 0;
	for (iEdge = 0; iEdge < MARCHING_CUBES_EDGES; iEdge++)
	{
		iStart = CMarchingCubes::EdgeStartCorner(iEdge);
		iEnd = CMarchingCubes::EdgeEndCorner(iEdge);
		if (IsosurfaceInside(afDistances[iStart]) != IsosurfaceInside(afDistances[iEnd]))
		{
			IsosurfaceFindCrossing(&sCrossing, pcShape, &asCorners[iStart], afDistances[iStart], &asCorners[iEnd], afDistances[iEnd], fCellSize * 1e-6f);
			asNormals[iNumNormals] = sCrossing.sNormal;
			iNumNormals++;
		}
	}
	for (i = 0; i < iNumNormals; i++)
	{
		for (j = i + 1; j < iNumNormals; j++)
		{
			if (Float32Vec3Dot(&asNormals[i], &asNormals[j]) < mfSplitCosine)
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The minimal edges with crossings.  A leaf's edge is minimal when the
// cubes of its size in the other three places around it aren't split:
// the leaves holding them are no smaller.  Of the leaves of its size
// around it, the first in order makes it, so it's made once.  The places
// go anticlockwise seen from the edge's far end, as in CDualContouring,
// and the polygon is turned round when the edge starts outside.
//
//////////////////////////////////////////////////////////////////////////
void CIsosurfaceOctree::FindPolygons(CShape* pcShape, CArrayIsosurfacePolygon* pasPolygons, CArrayIsosurfaceFit* pasFits, CArrayDualContouringPieces* pasPieces, CArrayLargeInt* paiFirstVertices)
{
	static const int		aaiAround[4][2] = {{-1, -1}, {0, -1}, {0, 0}, {-1, 0}};
	SIsosurfaceNode*		psLeaf;
	SIsosurfaceNode*		psNode;
	SIsosurfacePolygon*		psPolygon;
	SIsosurfaceCrossing		sCrossing;
	SFloat32Vec3			sStart;
	SFloat32Vec3			sEnd;
	int						iLeaf;
	int						iNumNodes;
	int						iAxis;
	int						iU;
	int						iV;
	int						k;
	int						i;
	int						iSize;
	int						aiStart[3];
	int						aiPlace[3];
	int						aiLeaves[4];
	int						aiVertices[4];
	int						aiDistinct[4];
	int						iLocalEdge;
	int						iNumDistinct;
	int						iSwap;
	bool					bMinimal;
	float32					fStartDistance;
	float32					fEndDistance;
	float32					fWeight;

	iNumNodes = (int)masNodes.NumElements();
	for (iLeaf = 0; iLeaf < iNumNodes; iLeaf++)
	{
		psLeaf = masNodes.Get((lsize)iLeaf);
		if (psLeaf->iFirstChild != -1)
		{
			continue;
		}

		iSize = psLeaf->iSize;
		for (iAxis = 0; iAxis < 3; iAxis++)
		{
			iU = (iAxis + 1) % 3;
			iV = (iAxis + 2) % 3;
			for (k = 0; k < 4; k++)
			{
				psLeaf = masNodes.Get((lsize)iLeaf);
				aiStart[0] = psLeaf->aiCorner[0];
				aiStart[1] = psLeaf->aiCorner[1];
				aiStart[2] = psLeaf->aiCorner[2];
				aiStart[iU] += (k & 1) * iSize;
				aiStart[iV] += ((k >> 1) & 1) * iSize;

				bMinimal = true;
				for (i = 0; i < 4; i++)
				{
					aiPlace[0] = aiStart[0];
					aiPlace[1] = aiStart[1];
					aiPlace[2] = aiStart[2];
					aiPlace[iU] += aaiAround[i][0] * iSize;
					aiPlace[iV] += aaiAround[i][1] * iSize;
					aiLeaves[i] = FindLeaf((2 * aiPlace[0]) + iSize, (2 * aiPlace[1]) + iSize, (2 * aiPlace[2]) + iSize);
					if (aiLeaves[i] == -1)
					{
						bMinimal = false;
						break;
					}
					psNode = masNodes.Get((lsize)aiLeaves[i]);
					if (psNode->iSize < iSize)
					{
						bMinimal = false;
						break;
					}
				}
				if (!bMinimal)
				{
					continue;
				}
				for (i = 0; i < 4; i++)
				{
					if (masNodes.Get((lsize)aiLeaves[i])->iSize == iSize)
					{
						break;
					}
				}
				if (aiLeaves[i] != iLeaf)
				{
					continue;
				}

				GetPosition(&sStart, aiStart[0], aiStart[1], aiStart[2]);
				aiStart[iAxis] += iSize;
				GetPosition(&sEnd, aiStart[0], aiStart[1], aiStart[2]);
				fStartDistance = pcShape->Distance(&sStart);
				fEndDistance = pcShape->Distance(&sEnd);
				if (IsosurfaceInside(fStartDistance) == IsosurfaceInside(fEndDistance))
				{
					continue;
				}
				IsosurfaceFindCrossing(&sCrossing, pcShape, &sStart, fStartDistance, &sEnd, fEndDistance, (float32)iSize * mfSmallest * 1e-6f);

				//A smallest leaf's vertex is its piece's that has the edge; the edge is its own, from its corner at aiPlace.
				aiStart[iAxis] -= iSize;
				for (i = 0; i < 4; i++)
				{
					psNode = masNodes.Get((lsize)aiLeaves[i]);
					aiVertices[i] = paiFirstVertices->GetValue((lsize)aiLeaves[i]);
					if (psNode->iSize == 1)
					{
						iLocalEdge = (iAxis * 4) + (aiStart[iU] - psNode->aiCorner[iU]) + (2 * (aiStart[iV] - psNode->aiCorner[iV]));
						aiVertices[i] += pasPieces->Get((lsize)aiLeaves[i])->aiEdgePieces[iLocalEdge];
					}
				}

				iNumDistinct = 0;
				for (i = 0; i < 4; i++)
				{
					if ((iNumDistinct == 0) || (aiDistinct[iNumDistinct - 1] != aiVertices[i]))
					{
						aiDistinct[iNumDistinct] = aiVertices[i];
						aiLeaves[iNumDistinct] = aiLeaves[i];
						iNumDistinct++;
					}
				}
				if ((iNumDistinct > 1) && (aiDistinct[iNumDistinct - 1] == aiDistinct[0]))
				{
					iNumDistinct--;
				}
				if (iNumDistinct < 3)
				{
					continue;
				}

				for (i = 0; i < iNumDistinct; i++)
				{
					fWeight = (float32)iSize / (float32)masNodes.Get((lsize)aiLeaves[i])->iSize;
					pasFits->Get((lsize)aiDistinct[i])->Add(&sCrossing.sPosition, &sCrossing.sNormal, fWeight);
				}
				if (!IsosurfaceInside(fStartDistance))
				{
					iSwap = aiDistinct[1];
					aiDistinct[1] = aiDistinct[iNumDistinct - 1];
					aiDistinct[iNumDistinct - 1] = iSwap;
				}

				psPolygon = pasPolygons->Add();
				psPolygon->iNumVertices = iNumDistinct;
				psPolygon->sNormal = sCrossing.sNormal;
				for (i = 0; i < iNumDistinct; i++)
				{
					psPolygon->aiVertices[i] = aiDistinct[i];
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The vertices each leaf has: a smallest leaf one for each sheet of
// surface in it (its corners' marching cubes case, its pieces joined
// where they aren't two sheets), a bigger leaf one.  paiFirstVertices is
// each leaf's first (-1 for a smallest leaf the surface doesn't cross),
// paiVertexLeaves each vertex's leaf, and it returns how many there are.
//
//////////////////////////////////////////////////////////////////////////
int CIsosurfaceOctree::FindPieces(CShape* pcShape, CArrayDualContouringPieces* pasPieces, CArrayLargeInt* paiFirstVertices, CArrayLargeInt* paiVertexLeaves)
{
	SIsosurfaceNode*		psNode;
	SMarchingCubesCase		sCase;
	SIsosurfaceCrossing		sCrossing;
	SFloat32Vec3			asCorners[MARCHING_CUBES_CORNERS];
	float32					afDistances[MARCHING_CUBES_CORNERS];
	SFloat32Vec3			asNormals[MARCHING_CUBES_EDGES];
	int						iNumNodes;
	int						iNode;
	int						iCorner;
	int						iEdge;
	int						iStart;
	int						iEnd;
	int						iPiece;
	int						iNumVertices;

	iNumNodes = (int)masNodes.NumElements();
	pasPieces->Resize((lsize)iNumNodes);
	paiFirstVertices->Resize((lsize)iNumNodes);
	paiFirstVertices->SetArrayValues(-1);
	iNumVertices = 0;
	for (iNode = 0; iNode < iNumNodes; iNode++)
	{
		psNode = masNodes.Get((lsize)iNode);
		if (psNode->iFirstChild != -1)
		{
			continue;
		}
		if (psNode->iSize > 1)
		{
			paiFirstVertices->SetValue((lsize)iNode, iNumVertices);
			paiVertexLeaves->Add(iNode);
			iNumVertices++;
			continue;
		}

		for (iCorner = 0; iCorner < MARCHING_CUBES_CORNERS; iCorner++)
		{
			GetPosition(&asCorners[iCorner], psNode->aiCorner[0] + (iCorner & 1), psNode->aiCorner[1] + ((iCorner >> 1) & 1), psNode->aiCorner[2] + ((iCorner >> 2) & 1));
			afDistances[iCorner] = pcShape->Distance(&asCorners[iCorner]);
		}
		CMarchingCubes::DecideCase(&sCase, afDistances);
		if (sCase.iNumPieces == 0)
		{
			continue;
		}
		for (iEdge = 0; iEdge < MARCHING_CUBES_EDGES; iEdge++)
		{
			if (sCase.aiEdgePieces[iEdge] >= 0)
			{
				iStart = CMarchingCubes::EdgeStartCorner(iEdge);
				iEnd = CMarchingCubes::EdgeEndCorner(iEdge);
				IsosurfaceFindCrossing(&sCrossing, pcShape, &asCorners[iStart], afDistances[iStart], &asCorners[iEnd], afDistances[iEnd], mfSmallest * 1e-6f);
				asNormals[iEdge] = sCrossing.sNormal;
			}
		}
		IsosurfaceJoinPieces(&sCase, asNormals);

		memcpy(pasPieces->Get((lsize)iNode)->aiEdgePieces, sCase.aiEdgePieces, sizeof(sCase.aiEdgePieces));
		paiFirstVertices->SetValue((lsize)iNode, iNumVertices);
		for (iPiece = 0; iPiece < sCase.iNumPieces; iPiece++)
		{
			paiVertexLeaves->Add(iNode);
			iNumVertices++;
		}
	}
	return iNumVertices;
}


//////////////////////////////////////////////////////////////////////////
//
// The vertices crossings were added to, fitted and kept in their leaves'
// cells, then the polygons on them.
//
//////////////////////////////////////////////////////////////////////////
void CIsosurfaceOctree::Generate(CMeshEditor* pcMeshEditor, CShape* pcShape, int iFaceName)
{
	CArrayIsosurfacePolygon		asPolygons;
	CArrayIsosurfaceFit			asFits;
	CArrayDualContouringPieces	asPieces;
	CArrayLargeInt				aiFirstVertices;
	CArrayLargeInt				aiVertexLeaves;
	CArrayLargeInt				aiCorners;
	SIsosurfaceNode*			psNode;
	SIsosurfaceFit*				psFit;
	SIsosurfacePolygon*			psPolygon;
	SFloat32Bounds				sCell;
	SFloat32Vec3				sCentre;
	SFloat32Vec3				sVertex;
	int							iNumVertices;
	int							iVertex;
	int							iCorner;
	int							i;
	int							j;
	int							aiPolygon[4];
	float32						fCellSize;

	asPolygons.Init();
	asFits.Init();
	asPieces.Init();
	aiFirstVertices.Init();
	aiVertexLeaves.Init();
	aiCorners.Init();

	iNumVertices = FindPieces(pcShape, &asPieces, &aiFirstVertices, &aiVertexLeaves);
	asFits.Resize((lsize)iNumVertices);
	aiCorners.Resize((lsize)iNumVertices);
	aiCorners.SetArrayValues(-1);
	for (iVertex = 0; iVertex < iNumVertices; iVertex++)
	{
		psNode = masNodes.Get((lsize)aiVertexLeaves.GetValue((lsize)iVertex));
		fCellSize = (float32)psNode->iSize * mfSmallest;
		GetPosition(&sCentre, psNode->aiCorner[0], psNode->aiCorner[1], psNode->aiCorner[2]);
		sCentre += SFloat32Vec3(fCellSize * 0.5f, fCellSize * 0.5f, fCellSize * 0.5f);
		asFits.Get((lsize)iVertex)->Init(&sCentre);
	}

	FindPolygons(pcShape, &asPolygons, &asFits, &asPieces, &aiFirstVertices);

	iCorner = pcMeshEditor->NumCorners();
	for (iVertex = 0; iVertex < iNumVertices; iVertex++)
	{
		psFit = asFits.Get((lsize)iVertex);
		if (psFit->iNumPoints == 0)
		{
			continue;
		}
		psNode = masNodes.Get((lsize)aiVertexLeaves.GetValue((lsize)iVertex));
		GetPosition(&sCell.sMinimum, psNode->aiCorner[0], psNode->aiCorner[1], psNode->aiCorner[2]);
		GetPosition(&sCell.sMaximum, psNode->aiCorner[0] + psNode->iSize, psNode->aiCorner[1] + psNode->iSize, psNode->aiCorner[2] + psNode->iSize);
		psFit->Solve(&sVertex, &sCell);
		pcMeshEditor->AddCorner(&sVertex);
		aiCorners.SetValue((lsize)iVertex, iCorner);
		iCorner++;
	}

	for (i = 0; i < (int)asPolygons.NumElements(); i++)
	{
		psPolygon = asPolygons.Get((lsize)i);
		for (j = 0; j < psPolygon->iNumVertices; j++)
		{
			aiPolygon[j] = aiCorners.GetValue((lsize)psPolygon->aiVertices[j]);
		}
		if (psPolygon->iNumVertices == 4)
		{
			IsosurfaceAddQuad(pcMeshEditor, pcShape, aiPolygon, &psPolygon->sNormal, iFaceName);
		}
		else
		{
			pcMeshEditor->AddFace(aiPolygon[0], aiPolygon[1], aiPolygon[2], iFaceName);
		}
	}

	aiCorners.Kill();
	aiVertexLeaves.Kill();
	aiFirstVertices.Kill();
	asPieces.Kill();
	asFits.Kill();
	asPolygons.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIsosurfaceOctree::NumLeaves(void)
{
	int		i;
	int		iLeaves;

	iLeaves = 0;
	for (i = 0; i < (int)masNodes.NumElements(); i++)
	{
		if (masNodes.Get((lsize)i)->iFirstChild == -1)
		{
			iLeaves++;
		}
	}
	return iLeaves;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIsosurfaceOctree::FindLeaf(int x2, int y2, int z2)
{
	SIsosurfaceNode*	psNode;
	int					iNode;
	int					iHalf;
	int					iChild;
	int					iRoot2;

	psNode = masNodes.Get(0);
	iRoot2 = 2 * psNode->iSize;
	if ((x2 < 0) || (y2 < 0) || (z2 < 0) || (x2 >= iRoot2) || (y2 >= iRoot2) || (z2 >= iRoot2))
	{
		return -1;
	}

	iNode = 0;
	while (psNode->iFirstChild != -1)
	{
		iHalf = psNode->iSize / 2;
		iChild = 0;
		if (x2 >= (2 * (psNode->aiCorner[0] + iHalf)))
		{
			iChild |= 1;
		}
		if (y2 >= (2 * (psNode->aiCorner[1] + iHalf)))
		{
			iChild |= 2;
		}
		if (z2 >= (2 * (psNode->aiCorner[2] + iHalf)))
		{
			iChild |= 4;
		}
		iNode = psNode->iFirstChild + iChild;
		psNode = masNodes.Get((lsize)iNode);
	}
	return iNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIsosurfaceOctree::GetPosition(SFloat32Vec3* psDest, int x, int y, int z)
{
	psDest->Init(msOrigin.x + ((float32)x * mfSmallest), msOrigin.y + ((float32)y * mfSmallest), msOrigin.z + ((float32)z * mfSmallest));
}

