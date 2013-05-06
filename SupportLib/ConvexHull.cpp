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
#include "ConvexHull.h"
#include "ShapeHelper.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/Numbers.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExtremeTriangle::Init(SFloat3* psPoint1, SFloat3* psPoint2, SFloat3* psPoint3, SFloat3* psNormal)
{
	CTriangle::Init(psPoint1, psPoint2, psPoint3, psNormal);
	maiVisible.Init(512);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExtremeTriangle::Kill(void)
{
	maiVisible.Kill();
	mpsNormal = NULL;
	mpsPosition = NULL;
	mpsPosition1 = NULL;
	mpsPosition2 = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CExtremeTriangle::FindFurthestPoint(SFloat3* psPoints, int iStride)
{
	int					iFarIndex;
	int					i;
	SFloat3*			psOther;
	float				fDist;
	float				fMaxDist;
	int					iIndex;

	fMaxDist = 0.0f;
	iFarIndex = -1;
	for (i = 0; i < maiVisible.NumElements(); i++)
	{
		iIndex = maiVisible.GetValue(i);
		psOther = GetPosition(psPoints, iStride, iIndex);

		fDist = Float3Dot(mpsNormal, psOther)-d;
		if ((fDist > fMaxDist) || ((iFarIndex == -1) && fDist == 0.0f))
		{
			fMaxDist = fDist;
			iFarIndex = iIndex;
		}
	}
	return iFarIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CExtremeTriangle::NotContains(SFloat3* psPosition)
{
	return Float3Dot(mpsNormal, psPosition) > d;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConvexHullGenerator::Init(SFloat3* psPoints, int iStride, int iNumPoints, char* szHullName)
{
	mcNormals.Init(512);
	mcTriangles.Init(512);
	mpsPoints = psPoints;
	this->iStride = iStride;
	this->iNumPoints = iNumPoints;
	mszHullName = szHullName;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConvexHullGenerator::Kill(void)
{
	SFreeListIterator	sIter;
	CExtremeTriangle*	pcTriangle;
	
	pcTriangle = mcTriangles.StartIteration(&sIter);
	while (pcTriangle)
	{
		pcTriangle->Kill();
		pcTriangle = mcTriangles.Iterate(&sIter);
	}
	
	mcTriangles.Kill();
	mcNormals.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CConvexHullGenerator::FindFirstPairTriangles(CArrayExtremeTrianglePtr* papcTriangles, int iMaxXIndex, int iMinXIndex, int iFarIndex)
{
	CHalfSpaceHelper			cHalfSpace;
	int							i;
	CExtremeTriangle*			pcTriangleUp;
	CExtremeTriangle*			pcTriangleDown;
	SFloat3*					psPosition;

	pcTriangleUp = AddTriangle(GetPosition(mpsPoints, iStride, iMaxXIndex), 
		GetPosition(mpsPoints, iStride, iMinXIndex), 
		GetPosition(mpsPoints, iStride, iFarIndex));

	pcTriangleDown = AddTriangle(GetPosition(mpsPoints, iStride, iMaxXIndex), 
		GetPosition(mpsPoints, iStride, iFarIndex), 
		GetPosition(mpsPoints, iStride, iMinXIndex));

	for (i = 0; i < iNumPoints; i++)
	{
		psPosition = GetPosition(mpsPoints, iStride, i);
		if ((i != iMaxXIndex) && (i != iMinXIndex) && (i != iFarIndex))
		{
			if (!pcTriangleUp->Contains(psPosition))
			{
				pcTriangleUp->maiVisible.Add(i);
			}
			else if (!pcTriangleDown->Contains(psPosition))
			{
				pcTriangleDown->maiVisible.Add(i);
			}
			else
			{
				gcUserError.Set("No triangle contained point");
				return FALSE;
			}
		}
	}

	//These triangles ARE sorted at this point.
	papcTriangles->Add(&pcTriangleUp);
	papcTriangles->Add(&pcTriangleDown);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConvexHullGenerator::AddPointsFromTriangles(CExtremeTriangle* pcTriangle, CArrayExtremeTrianglePtr* papcTriangles, int iDontAdd)
{
	int					i;
	CExtremeTriangle*	pcOther;
	int					j;
	int					iIndex;
	SFloat3*			psPosition;

	for (i = 0; i < papcTriangles->NumElements(); i++)
	{
		pcOther = *papcTriangles->Get(i);
		for (j = 0; j < pcOther->maiVisible.NumElements(); j++)
		{
			iIndex = pcOther->maiVisible.GetValue(j);
			if (iIndex != iDontAdd)
			{
				psPosition = GetPosition(mpsPoints, iStride, iIndex);
				if (pcTriangle->NotContains(psPosition))
				{
					pcTriangle->maiVisible.InsertIntoSorted(iIndex, TRUE);
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CConvexHullGenerator::Generate(void)
{
	int							iMaxXIndex;
	int							iMinXIndex;
	CHalfSpaceHelper			cHalfSpace;
	int							iFarIndex;
	SFloat3*					psPosition;
	CArrayExtremeTrianglePtr	apcTriangles;
	CExtremeTriangle*			pcTriangle;
	CExtremeTriangle*			pcDeleted;
	SFreeListIterator			sIter;
	CArrayExtremeTrianglePtr	cDeleted;
	CArrayExtremeTrianglePtr	cFixedDeleted;
	int							j;
	SConvexHullHoleEdge			sEdges;
	int							k;
	int							aiIndex[3];
	int							iIndex;
	CExtremeTriangle*			pcSelected;
	CTextFile					cTextFile;
	int							iTriangle;
	float						fMinX;
	float						fMaxX;

	iMaxXIndex = FindMaxX(&fMinX);
	iMinXIndex = FindMinX(&fMaxX);
	iFarIndex = FindFurthestPoint(iMaxXIndex, iMinXIndex);

	if (iFarIndex == -1)
	{
		gcUserError.Set("Could not find a third point generating Convex Hull.");
		return FALSE;
	}

	apcTriangles.Init(2048);

	if (!FindFirstPairTriangles(&apcTriangles, iMaxXIndex, iMinXIndex, iFarIndex))
	{
		gcUserError.Set("Could not find the first triangle pair generating Convex Hull.");
		return FALSE;
	}

	cDeleted.Init(512);
	cFixedDeleted.Init(512);

	for (iTriangle = 0; iTriangle < apcTriangles.NumElements(); iTriangle++)
	{
		pcSelected = *apcTriangles.Get(iTriangle);

		if ((pcSelected == NULL) || (pcSelected->maiVisible.NumElements() == 0))
		{
			continue;
		}

		aiIndex[0] = GetIndex(mpsPoints, iStride, pcSelected->mpsPosition);
		aiIndex[1] = GetIndex(mpsPoints, iStride, pcSelected->mpsPosition1);
		aiIndex[2] = GetIndex(mpsPoints, iStride, pcSelected->mpsPosition2);

		iFarIndex = pcSelected->FindFurthestPoint(mpsPoints, iStride);
		if (iFarIndex == -1)
		{
			gcUserError.Set("Could not find furthest point!");
			return FALSE;
		}

		cDeleted.FakeSetUsedElements(0);  //It's sort of safe to do this.

		psPosition = GetPosition(mpsPoints, iStride, iFarIndex);
		pcTriangle = mcTriangles.StartIteration(&sIter);
		while (pcTriangle)
		{
			if (pcTriangle->maiVisible.NumElements() > 0)
			{
				if (pcTriangle->NotContains(psPosition))
				{
					cDeleted.Add(&pcTriangle);
				}
			}
			pcTriangle = mcTriangles.Iterate(&sIter);
		}
		
		RemoveDiscontiguousTriangles(pcSelected, &cDeleted, &cFixedDeleted);

		for (j = 0; j < cDeleted.NumElements(); j++)
		{
			pcDeleted = *cDeleted.Get(j);
			FindEdges(&sEdges, pcDeleted, &cDeleted);

			for (k = 0; k < sEdges.iNumEdges; k++)
			{
				pcTriangle = AddTriangle(GetPosition(mpsPoints, iStride, sEdges.aaiEdgeIndices[k][0]),
					GetPosition(mpsPoints, iStride, sEdges.aaiEdgeIndices[k][1]),
					GetPosition(mpsPoints, iStride, iFarIndex));

				apcTriangles.Add(&pcTriangle);
				AddPointsFromTriangles(pcTriangle, &cDeleted, iFarIndex);
			}
		}

		for (j = 0; j < cDeleted.NumElements(); j++)
		{
			pcDeleted = *cDeleted.Get(j);
			pcDeleted->Kill();
			mcTriangles.Remove(pcDeleted);

			iIndex = apcTriangles.FindWithIntKey((int)(size_t)pcDeleted, 0);
			*(apcTriangles.Get(iIndex)) = NULL;
		}
	}
	cDeleted.Kill();
	cFixedDeleted.Kill();

	apcTriangles.Kill();

	RemoveSlivers();

	if (mszHullName)
	{
		CChars szTemp;
		szTemp.Init("Writing Hull file [");
		szTemp.Append(mszHullName);
		szTemp.Append("]\n");
		szTemp.Kill();
		cTextFile.Init();
		DumpTriangleObj(&cTextFile.mcText, iTriangle);
		cTextFile.Write(mszHullName);
		cTextFile.Kill();
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CExtremeTriangle* CConvexHullGenerator::FindAdjacentTriangle(CExtremeTriangle* pcTriangle, int iPositionIndex0, int iPositionIndex1)
{
	SFreeListIterator			sIter;
	CExtremeTriangle*			pcTriangleOther;

	pcTriangleOther = mcTriangles.StartIteration(&sIter);
	while (pcTriangleOther)
	{
		if (pcTriangleOther != pcTriangle)
		{
			if (TriangleHasEdge(iPositionIndex0, iPositionIndex1, pcTriangleOther))
			{
				return pcTriangleOther;
			}
		}
		pcTriangleOther = mcTriangles.Iterate(&sIter);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConvexHullGenerator::RemoveSlivers(void)
{
	SFreeListIterator			sIter;
	CExtremeTriangle*			pcTriangle;
	SFloat3*					ps0;
	SFloat3*					ps1;
	SFloat3*					ps2;
	int							iNumSlivers;
	int							iIndex0;
	int							iIndex1;
	int							iIndex2;
	CExtremeTriangle*			pcTriangleToRemove;

	pcTriangle = mcTriangles.StartIteration(&sIter);
	while (pcTriangle)
	{
		iNumSlivers = pcTriangle->IsSliver(Deg2Rad(1.0f), &ps0, &ps1, &ps2);
		if (iNumSlivers == 2)
		{
			iIndex0 = GetIndex(mpsPoints, iStride, ps0);
			iIndex1 = GetIndex(mpsPoints, iStride, ps1);
			iIndex2 = GetIndex(mpsPoints, iStride, ps2);
			
			pcTriangleToRemove = FindAdjacentTriangle(pcTriangle, iIndex0, iIndex2);

			if (pcTriangleToRemove)
			{
				mcTriangles.Remove(pcTriangleToRemove);
				mcTriangles.RemoveDuringIteration(&sIter);
				MoveTrianglesPointFrom(iIndex0, iIndex2);
			}
		}
		else if (iNumSlivers == 1)
		{
			//iIndex0 = GetIndex(mpsPoints, iStride, ps0);
			//iIndex1 = GetIndex(mpsPoints, iStride, ps1);
			//iIndex2 = GetIndex(mpsPoints, iStride, ps2);

			//pcTriangleToRemove = FindAdjacentTriangle(pcTriangle, iIndex0, iIndex1);

			//if (pcTriangleToRemove)
			//{
			//	mcTriangles.Remove(pcTriangleToRemove);
			//	mcTriangles.RemoveDuringIteration(&sIter);
			//	MoveTrianglesPointFrom(iIndex1, iIndex0);
			//}			
		}
		pcTriangle = mcTriangles.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConvexHullGenerator::MoveTrianglesPointFrom(int iOldIndex, int iNewIndex)
{
	SFreeListIterator			sIter;
	CExtremeTriangle*			pcTriangle;
	SFloat3*					psNewPosition;

	psNewPosition = GetPosition(mpsPoints, iStride, iNewIndex);
	pcTriangle = mcTriangles.StartIteration(&sIter);
	while (pcTriangle)
	{
		if (GetIndex(mpsPoints, iStride, pcTriangle->mpsPosition) == iOldIndex)
		{
			pcTriangle->mpsPosition = psNewPosition;
		}
		if (GetIndex(mpsPoints, iStride, pcTriangle->mpsPosition1) == iOldIndex)
		{
			pcTriangle->mpsPosition1 = psNewPosition;
		}
		if (GetIndex(mpsPoints, iStride, pcTriangle->mpsPosition2) == iOldIndex)
		{
			pcTriangle->mpsPosition2 = psNewPosition;
		}
		pcTriangle->Set();  //For a sliver triangle the removed triangle and the filling triangle must have been co-planar.  No need to recalculate the normal.
		
		pcTriangle = mcTriangles.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFreeListNormals* CConvexHullGenerator::GetNormals(void)
{
	return &mcNormals;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CConvexHullGenerator::RemoveDiscontiguousTriangles(CExtremeTriangle* pcSelected, CArrayExtremeTrianglePtr* papcTriangles, CArrayExtremeTrianglePtr* papcTemp)
{
	int							iIndex;
	int							i;
	int							j;
	CExtremeTriangle*			pcTriangle1;
	CExtremeTriangle*			pcTriangle2;
	BOOL						bResult;
	CArrayExtremeTrianglePtr	cTemp;

	papcTemp->FakeSetUsedElements(0);
	papcTemp->Add(&pcSelected);

	iIndex = papcTriangles->FindWithIntKey((int)(size_t)pcSelected, 0);
	papcTriangles->RemoveAtNoDeallocate(iIndex, FALSE);

	for (i = 0; i < papcTemp->NumElements(); i++)
	{
		pcTriangle1 = *papcTemp->Get(i);
		for (j = 0; j < papcTriangles->NumElements(); j++)
		{
			pcTriangle2 = *papcTriangles->Get(j);
			if (TriangleAdjacent(pcTriangle1, pcTriangle2))
			{
				papcTriangles->RemoveAtNoDeallocate(j, FALSE);
				j--;

				papcTemp->Add(&pcTriangle2);
			}
		}
	}

	bResult = FALSE;
	if (papcTriangles->NumElements() > 0)
	{
		bResult = TRUE;
	}

	memcpy(&cTemp, papcTemp, sizeof(CArrayExtremeTrianglePtr));
	memcpy(papcTemp, papcTriangles, sizeof(CArrayExtremeTrianglePtr));
	memcpy(papcTriangles, &cTemp, sizeof(CArrayExtremeTrianglePtr));
	return bResult;  //True means there were discontiguous triangles.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CConvexHullGenerator::Contained(SFloat3* psPosition)
{
	SFreeListIterator	sIter;
	CTriangle*			pcTriangle;

	pcTriangle = mcTriangles.StartIteration(&sIter);
	while (pcTriangle)
	{
		if (!pcTriangle->Contains(psPosition))
		{
			return FALSE;
		}
		pcTriangle = mcTriangles.Iterate(&sIter);
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CConvexHullGenerator::NotContained(SFloat3* psPosition)
{
	SFreeListIterator	sIter;
	CTriangle*			pcTriangle;

	pcTriangle = mcTriangles.StartIteration(&sIter);
	while (pcTriangle)
	{
		if (pcTriangle->NotContains(psPosition))
		{
			return TRUE;
		}
		pcTriangle = mcTriangles.Iterate(&sIter);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConvexHullGenerator::FindEdges(SConvexHullHoleEdge* psEdges, CExtremeTriangle* pcTriangleWithEdges, CArrayExtremeTrianglePtr* papcDeletedTriangles)
{
	int							iIndex[3];
	BOOL						bAnyHasEdge;

	iIndex[0] = GetIndex(mpsPoints, iStride, pcTriangleWithEdges->mpsPosition);
	iIndex[1] = GetIndex(mpsPoints, iStride, pcTriangleWithEdges->mpsPosition1);
	iIndex[2] = GetIndex(mpsPoints, iStride, pcTriangleWithEdges->mpsPosition2);

	psEdges->iNumEdges = 0;

	bAnyHasEdge = TrianglesHaveEdge(iIndex[0], iIndex[1], pcTriangleWithEdges, papcDeletedTriangles);
	if (!bAnyHasEdge)
	{
		psEdges->aaiEdgeIndices[psEdges->iNumEdges][0] = iIndex[0];
		psEdges->aaiEdgeIndices[psEdges->iNumEdges][1] = iIndex[1];
		psEdges->iNumEdges++;
	}

	bAnyHasEdge = TrianglesHaveEdge(iIndex[1], iIndex[2], pcTriangleWithEdges, papcDeletedTriangles);
	if (!bAnyHasEdge)
	{
		psEdges->aaiEdgeIndices[psEdges->iNumEdges][0] = iIndex[1];
		psEdges->aaiEdgeIndices[psEdges->iNumEdges][1] = iIndex[2];
		psEdges->iNumEdges++;
	}

	bAnyHasEdge = TrianglesHaveEdge(iIndex[2], iIndex[0], pcTriangleWithEdges, papcDeletedTriangles);
	if (!bAnyHasEdge)
	{
		psEdges->aaiEdgeIndices[psEdges->iNumEdges][0] = iIndex[2];
		psEdges->aaiEdgeIndices[psEdges->iNumEdges][1] = iIndex[0];
		psEdges->iNumEdges++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CConvexHullGenerator::TrianglesHaveEdge(int iEdge1, int iEdge2, CExtremeTriangle* pcTriangleWithEdges, CArrayExtremeTrianglePtr* papcDeletedTriangles)
{
	int							i;
	CExtremeTriangle*			pcTriangle;

	for (i = 0; i < papcDeletedTriangles->NumElements(); i++)
	{
		pcTriangle = *papcDeletedTriangles->Get(i);
		if (pcTriangle != pcTriangleWithEdges)
		{
			if (TriangleHasEdge(iEdge1, iEdge2, pcTriangle))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CConvexHullGenerator::TriangleHasEdge(int iEdge1, int iEdge2, CExtremeTriangle* pcTriangle)
{
	int		iOther[3];

	iOther[0] = GetIndex(mpsPoints, iStride, pcTriangle->mpsPosition);
	iOther[1] = GetIndex(mpsPoints, iStride, pcTriangle->mpsPosition1);
	iOther[2] = GetIndex(mpsPoints, iStride, pcTriangle->mpsPosition2);

	if (((iEdge1 == iOther[0]) && (iEdge2 == iOther[1]) || ((iEdge1 == iOther[1]) && (iEdge2 == iOther[0]))) ||
		((iEdge1 == iOther[1]) && (iEdge2 == iOther[2]) || ((iEdge1 == iOther[2]) && (iEdge2 == iOther[1]))) ||
		((iEdge1 == iOther[0]) && (iEdge2 == iOther[2]) || ((iEdge1 == iOther[2]) && (iEdge2 == iOther[0]))))
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CConvexHullGenerator::TriangleAdjacent(CExtremeTriangle* pcTriangle1, CExtremeTriangle* pcTriangle2)
{
	int							iIndex[3];

	iIndex[0] = GetIndex(mpsPoints, iStride, pcTriangle1->mpsPosition);
	iIndex[1] = GetIndex(mpsPoints, iStride, pcTriangle1->mpsPosition1);
	iIndex[2] = GetIndex(mpsPoints, iStride, pcTriangle1->mpsPosition2);

	if (TriangleHasEdge(iIndex[0], iIndex[1], pcTriangle2))
	{
		return TRUE;
	}
	if (TriangleHasEdge(iIndex[1], iIndex[2], pcTriangle2))
	{
		return TRUE;
	}
	if (TriangleHasEdge(iIndex[2], iIndex[0], pcTriangle2))
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CExtremeTriangle* CConvexHullGenerator::AddTriangle(SFloat3* psPoint1, SFloat3* psPoint2, SFloat3* psPoint3)
{
	CExtremeTriangle*	pcTriangle;
	SFloat3*			psNormal;
	SFloat3				sSide1;
	SFloat3				sSide2;

	psNormal = mcNormals.Add();

	pcTriangle = mcTriangles.Add();
	pcTriangle->Init(psPoint1, psPoint2, psPoint3, psNormal);
	pcTriangle->Set(psPoint1, psPoint2, psPoint3);
	return pcTriangle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CConvexHullGenerator::FindFurthestPoint(int iMaxXIndex, int iMinXIndex)
{
	CHalfSpaceHelper	cHalfSpace;

	cHalfSpace.Init();
	cHalfSpace.SetZPlane(GetPosition(mpsPoints, iStride, iMaxXIndex), GetPosition(mpsPoints, iStride, iMinXIndex));
	return cHalfSpace.FindUnsignedFurthestPoint(mpsPoints, iStride, iNumPoints);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CConvexHullGenerator::FindMaxX(float* px)
{
	int			i;
	float		x;
	int			iIndex;
	SFloat3*	psPosition;

	iIndex = -1;
	x = 0;
	for (i = 0; i < iNumPoints; i++)
	{
		psPosition = GetPosition(mpsPoints, iStride, i);
		if ((i == 0) || (psPosition->x > x))
		{
			x = psPosition->x;
			iIndex = i;
		}
	}
	SafeAssign(px, x);
	return iIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CConvexHullGenerator::FindMinX(float* px)
{
	int			i;
	float		x;
	int			iIndex;
	SFloat3*	psPosition;


	iIndex = -1;
	x = 0;
	for (i = 0; i < iNumPoints; i++)
	{
		psPosition = GetPosition(mpsPoints, iStride, i);
		if ((i == 0) || (psPosition->x < x))
		{
			x = psPosition->x;
			iIndex = i;
		}
	}
	SafeAssign(px, x);
	return iIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConvexHull::Init(void)
{
	mcPolygons.Init(12);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConvexHull::Kill(void)
{
	int			i;
	CPolygon*	pcPolygon;

	for (i = 0; i < mcPolygons.NumElements(); i++)
	{
		pcPolygon = mcPolygons.Get(i);
		pcPolygon->Kill();
	}
	mcPolygons.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConvexHull::BeginSetFromPoints(CConvexHullGenerator* psConvexHullGenerator, SFloat3* psPoints, int iStride, int iNumPoints, char* szHullName)
{
	CArrayExtremeTrianglePtr	apcTriangles;
	SFreeListIterator			sIter;
	CExtremeTriangle*			pcTriangle;

	psConvexHullGenerator->Init(psPoints, iStride, iNumPoints, szHullName);
	psConvexHullGenerator->Generate();

	apcTriangles.Init(psConvexHullGenerator->mcTriangles.NumElements());

	pcTriangle = psConvexHullGenerator->mcTriangles.StartIteration(&sIter);
	while (pcTriangle)
	{
		apcTriangles.Add(&pcTriangle);
		pcTriangle = psConvexHullGenerator->mcTriangles.Iterate(&sIter);
	}

	ConvertTrianglesToPolygons((CArrayTrianglePtr*)&apcTriangles, &mcPolygons);

	apcTriangles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConvexHullGenerator::DumpTriangleObj(CChars* psz, int iLoop)
{
	int					i;
	SFloat3*			psPosition;
	CExtremeTriangle*	pcTriangle;
	int					iIndex1;
	int					iIndex2;
	SFreeListIterator	sIter;
	int					iIndex3;

	for (i = 0; i < iNumPoints; i++)
	{
		psz->Append("v  ");
		psPosition = GetPosition(mpsPoints, iStride, i);
		psz->Append(psPosition->x, 4);
		psz->Append(" ");
		psz->Append(psPosition->y, 4);
		psz->Append(" ");
		psz->Append(psPosition->z, 4);
		psz->AppendNewLine();
	}

	psz->Append("\ng Hull_");
	psz->Append(iLoop);
	psz->AppendNewLine();

	pcTriangle = mcTriangles.StartIteration(&sIter);
	while (pcTriangle)
	{
		psz->Append("f ");
		iIndex1 = GetIndex(mpsPoints, iStride, pcTriangle->mpsPosition)+1;
		iIndex2 = GetIndex(mpsPoints, iStride, pcTriangle->mpsPosition1)+1;
		iIndex3 = GetIndex(mpsPoints, iStride, pcTriangle->mpsPosition2)+1;

		psz->Append(iIndex1);
		psz->Append(" ");
		psz->Append(iIndex2);
		psz->Append(" ");
		psz->Append(iIndex3);
		psz->Append("\n");
		pcTriangle = mcTriangles.Iterate(&sIter);
	}
	psz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConvexHull::EndSetFromPoints(SFloat3* psNormals, int iStride, CConvexHullGenerator* psConvexHullGenerator)
{
	SFreeListIterator		sIter;
	int						i;
	SFloat3*				psNormal;

	if (psNormals)
	{
		i = 0;
		psNormal = psConvexHullGenerator->GetNormals()->StartIteration(&sIter);
		while (psNormal)
		{
			*GetNormal(psNormals, iStride, i) = *psNormal;

			i++;
			psNormal = psConvexHullGenerator->GetNormals()->Iterate(&sIter);
		}
	}

	psConvexHullGenerator->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ConvertTrianglesToPolygons(CArrayTrianglePtr* papcTriangles, CArrayPolygons* pacPolygons)
{
	int				i;
	CTriangle*		pcTriangle;
	int				j;
	CTriangle*		pcTriangleOther;
	CPolygon*		pcPolygon;

	for (i = 0; i < papcTriangles->NumElements(); i++)
	{
		pcTriangle = *papcTriangles->Get(i);
		pcPolygon = pacPolygons->Add();
		pcPolygon->Init();
		pcPolygon->AddTriangle(pcTriangle);

		for (j = i+1; j < papcTriangles->NumElements(); j++)
		{
			pcTriangleOther = *papcTriangles->Get(j);

			if (pcTriangle->Coplanar(pcTriangleOther))
			{
				pcPolygon->AddTriangle(pcTriangleOther);
				papcTriangles->RemoveAt(j);
				j--;
			}
		}
	}

	//You still need to check that the coplanar polygons are contiguous.

	//for (i = 0; i < pacPolygons->NumElements(); i++)
	//{
	//	pcPolygon = pacPolygons->Get(i);
	//	pcPolygon->Dump();
	//}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConvexHull::GetIndices(CArrayInt* paiIndices, SFloat3* psPoints, int iStride)
{
	int				i;
	CPolygon*		pcPolygon;
	int				j;
	SFloat3*		psPosition;
	int				iIndex;


	for (i = 0; i < mcPolygons.NumElements(); i++)
	{
		pcPolygon = mcPolygons.Get(i);
		for (j = 0; j < pcPolygon->mapsPositions.NumElements(); j++)
		{
			psPosition = *((SFloat3**)(pcPolygon->mapsPositions.Get(j)));
			iIndex = GetIndex(psPoints, iStride, psPosition);
			if (iIndex != -1)
			{
				paiIndices->InsertIntoSorted(iIndex, TRUE);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConvexHull::GetVertices(CArrayBlock* pasPositions, SFloat3* psPoints, int iStride)
{
	CArrayInt	aiIndices;
	int			i;
	int			iIndex;
	SFloat3*	psPosition;

	aiIndices.Init(mcPolygons.NumElements()+1);
	GetIndices(&aiIndices, psPoints, iStride);

	for (i = 0; i < aiIndices.NumElements(); i++)
	{
		iIndex = aiIndices.GetValue(i);
		psPosition = GetPosition(psPoints, iStride, iIndex);
		pasPositions->Add(psPosition);
	}

	aiIndices.Kill();
}

