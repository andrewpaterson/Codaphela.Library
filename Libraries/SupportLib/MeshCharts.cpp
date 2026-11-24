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
#include <math.h>
#include "MeshCharts.h"
#include "BaseLib/UnionFind.h"


//How far, at most, a face may end up from its grown chart's normal: short of a right angle, so it can't fold when laid flat.
#define MESH_CHARTS_FOLD_COSINE		(0.0871557f)		//cos 85 degrees.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void MeshFaceAreaNormal(SFloat32Vec3* psDest, CMesh* pcMesh, int iFace)
{
	int*			piCorners;
	SFloat32Vec3	sSide1;
	SFloat32Vec3	sSide2;

	piCorners = pcMesh->GetFaceCorners(iFace);
	sSide1 = *pcMesh->GetPosition(piCorners[1]) - *pcMesh->GetPosition(piCorners[0]);
	sSide2 = *pcMesh->GetPosition(piCorners[2]) - *pcMesh->GetPosition(piCorners[0]);
	Float32Vec3Cross(psDest, &sSide1, &sSide2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshCharts::Init(void)
{
	maiFaceCharts.Init();
	maiChartStarts.Init();
	maiChartFaces.Init();
	masNormals.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshCharts::Kill(void)
{
	masNormals.Kill();
	maiChartFaces.Kill();
	maiChartStarts.Kill();
	maiFaceCharts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshCharts::NumCharts(void) const
{
	return (int)masNormals.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshCharts::NumFaces(int iChart)
{
	return maiChartStarts.GetValue((lsize)iChart + 1) - maiChartStarts.GetValue((lsize)iChart);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshCharts::GetFace(int iChart, int i)
{
	return maiChartFaces.GetValue((lsize)(maiChartStarts.GetValue((lsize)iChart) + i));
}


//////////////////////////////////////////////////////////////////////////
//
// Each chart's faces, in face order, from which chart each face is in.
//
//////////////////////////////////////////////////////////////////////////
void CMeshCharts::ListFaces(int iNumFaces, int iNumCharts)
{
	CArrayLargeInt	aiFilled;
	int				iFace;
	int				iChart;

	maiChartStarts.Resize((lsize)iNumCharts + 1);
	maiChartStarts.SetArrayValues(0);
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		iChart = maiFaceCharts.GetValue((lsize)iFace);
		maiChartStarts.SetValue((lsize)iChart + 1, maiChartStarts.GetValue((lsize)iChart + 1) + 1);
	}
	for (iChart = 0; iChart < iNumCharts; iChart++)
	{
		maiChartStarts.SetValue((lsize)iChart + 1, maiChartStarts.GetValue((lsize)iChart + 1) + maiChartStarts.GetValue((lsize)iChart));
	}

	aiFilled.Init();
	aiFilled.Resize((lsize)iNumCharts);
	aiFilled.SetArrayValues(0);
	maiChartFaces.Resize((lsize)iNumFaces);
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		iChart = maiFaceCharts.GetValue((lsize)iFace);
		maiChartFaces.SetValue((lsize)(maiChartStarts.GetValue((lsize)iChart) + aiFilled.GetValue((lsize)iChart)), iFace);
		aiFilled.SetValue((lsize)iChart, aiFilled.GetValue((lsize)iChart) + 1);
	}
	aiFilled.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Faces of one direction joined by an edge, found by union and find.
//
//////////////////////////////////////////////////////////////////////////
void CMeshCharts::MakeBox(CMesh* pcMesh)
{
	CArrayLargeInt	aiDirections;
	CUnionFindLarge	cParents;
	CArrayLargeInt	aiRootCharts;
	CMeshEdge*		pcEdge;
	SFloat32Vec3	sNormal;
	float32			afNormal[3];
	int				iNumFaces;
	int				iNumCharts;
	int				iFace;
	int				iAxis;
	int				iRoot0;
	int				iDirection;
	int				i;

	iNumFaces = pcMesh->NumFaces();
	aiDirections.Init();
	cParents.Init();
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		MeshFaceAreaNormal(&sNormal, pcMesh, iFace);
		afNormal[0] = sNormal.x;
		afNormal[1] = sNormal.y;
		afNormal[2] = sNormal.z;
		iAxis = 0;
		for (i = 1; i < 3; i++)
		{
			if (fabsf(afNormal[i]) > fabsf(afNormal[iAxis]))
			{
				iAxis = i;
			}
		}
		aiDirections.Add((iAxis * 2) + ((afNormal[iAxis] > 0.0f) ? 1 : 0));
		cParents.Add();
	}
	for (i = 0; i < pcMesh->NumEdges(); i++)
	{
		pcEdge = pcMesh->GetEdge(i);
		if ((pcEdge->maiFace[0] < 0) || (pcEdge->maiFace[1] < 0) || (aiDirections.GetValue((lsize)pcEdge->maiFace[0]) != aiDirections.GetValue((lsize)pcEdge->maiFace[1])))
		{
			continue;
		}
		cParents.Join((lsize)pcEdge->maiFace[0], (lsize)pcEdge->maiFace[1]);
	}

	maiFaceCharts.Resize((lsize)iNumFaces);
	masNormals.ReInit();
	aiRootCharts.Init();
	aiRootCharts.Resize((lsize)iNumFaces);
	aiRootCharts.SetArrayValues(-1);
	iNumCharts = 0;
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		iRoot0 = (int)cParents.Find((lsize)iFace);
		if (aiRootCharts.GetValue((lsize)iRoot0) == -1)
		{
			aiRootCharts.SetValue((lsize)iRoot0, iNumCharts);
			iDirection = aiDirections.GetValue((lsize)iRoot0);
			afNormal[0] = 0.0f;
			afNormal[1] = 0.0f;
			afNormal[2] = 0.0f;
			afNormal[iDirection / 2] = ((iDirection & 1) != 0) ? 1.0f : -1.0f;
			masNormals.Add()->Init(afNormal[0], afNormal[1], afNormal[2]);
			iNumCharts++;
		}
		maiFaceCharts.SetValue((lsize)iFace, aiRootCharts.GetValue((lsize)iRoot0));
	}
	ListFaces(iNumFaces, iNumCharts);

	aiRootCharts.Kill();
	cParents.Kill();
	aiDirections.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each face's neighbours across its edges, as a list for each face.
//
//////////////////////////////////////////////////////////////////////////
static void MeshChartsNeighbours(CMesh* pcMesh, CArrayLargeInt* paiStarts, CArrayLargeInt* paiNeighbours)
{
	CArrayLargeInt	aiFilled;
	CMeshEdge*		pcEdge;
	int				iNumFaces;
	int				iFace;
	int				i;
	int				k;

	iNumFaces = pcMesh->NumFaces();
	paiStarts->Resize((lsize)iNumFaces + 1);
	paiStarts->SetArrayValues(0);
	for (i = 0; i < pcMesh->NumEdges(); i++)
	{
		pcEdge = pcMesh->GetEdge(i);
		if ((pcEdge->maiFace[0] >= 0) && (pcEdge->maiFace[1] >= 0))
		{
			for (k = 0; k < 2; k++)
			{
				iFace = pcEdge->maiFace[k];
				paiStarts->SetValue((lsize)iFace + 1, paiStarts->GetValue((lsize)iFace + 1) + 1);
			}
		}
	}
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		paiStarts->SetValue((lsize)iFace + 1, paiStarts->GetValue((lsize)iFace + 1) + paiStarts->GetValue((lsize)iFace));
	}

	paiNeighbours->Resize((lsize)paiStarts->GetValue((lsize)iNumFaces));
	aiFilled.Init();
	aiFilled.Resize((lsize)iNumFaces);
	aiFilled.SetArrayValues(0);
	for (i = 0; i < pcMesh->NumEdges(); i++)
	{
		pcEdge = pcMesh->GetEdge(i);
		if ((pcEdge->maiFace[0] >= 0) && (pcEdge->maiFace[1] >= 0))
		{
			for (k = 0; k < 2; k++)
			{
				iFace = pcEdge->maiFace[k];
				paiNeighbours->SetValue((lsize)(paiStarts->GetValue((lsize)iFace) + aiFilled.GetValue((lsize)iFace)), pcEdge->maiFace[1 - k]);
				aiFilled.SetValue((lsize)iFace, aiFilled.GetValue((lsize)iFace) + 1);
			}
		}
	}
	aiFilled.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshCharts::Grow(CMesh* pcMesh, float32 fMaxAngle)
{
	maiFaceCharts.Resize((lsize)pcMesh->NumFaces());
	maiFaceCharts.SetArrayValues(-1);
	masNormals.ReInit();
	GrowFree(pcMesh, fMaxAngle);
}


//////////////////////////////////////////////////////////////////////////
//
// The faces of the charts in paiCharts let go and grown into charts
// again; the other charts are kept, numbered again from 0 in order.
//
//////////////////////////////////////////////////////////////////////////
void CMeshCharts::Regrow(CMesh* pcMesh, CArrayInt* paiCharts, float32 fMaxAngle)
{
	CArrayLargeInt			aiNumbers;
	CArrayLargeFloat32Vec3	asKept;
	int						iNumFaces;
	int						iFace;
	int						iChart;
	int						iKept;
	size					i;

	aiNumbers.Init();
	aiNumbers.Resize((lsize)NumCharts());
	aiNumbers.SetArrayValues(0);
	for (i = 0; i < paiCharts->NumElements(); i++)
	{
		aiNumbers.SetValue((lsize)paiCharts->GetValue(i), -1);
	}
	asKept.Init();
	iKept = 0;
	for (iChart = 0; iChart < NumCharts(); iChart++)
	{
		if (aiNumbers.GetValue((lsize)iChart) != -1)
		{
			aiNumbers.SetValue((lsize)iChart, iKept);
			asKept.Add(masNormals.Get((lsize)iChart));
			iKept++;
		}
	}
	iNumFaces = pcMesh->NumFaces();
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		maiFaceCharts.SetValue((lsize)iFace, aiNumbers.GetValue((lsize)maiFaceCharts.GetValue((lsize)iFace)));
	}
	masNormals.ReInit();
	for (iChart = 0; iChart < iKept; iChart++)
	{
		masNormals.Add(asKept.Get((lsize)iChart));
	}
	asKept.Kill();
	aiNumbers.Kill();
	GrowFree(pcMesh, fMaxAngle);
}


//////////////////////////////////////////////////////////////////////////
//
// A chart grows from a seed (a face in no chart) by taking the face beside
// it (its frontier) nearest its average normal, while that's within the
// angle.  Then any face too far from its final average for it to be laid
// flat is let go, and is a seed again; seeds go round until every face is
// in a chart.  The new charts follow those there are.
//
//////////////////////////////////////////////////////////////////////////
void CMeshCharts::GrowFree(CMesh* pcMesh, float32 fMaxAngle)
{
	CArrayLargeInt			aiStarts;
	CArrayLargeInt			aiNeighbours;
	CArrayLargeFloat32Vec3	asNormals;  //Each face's, a unit.
	CArrayLargeFloat32Vec3	asAreaNormals;  //Each face's, twice its area long.
	CArrayLargeInt			aiFrontier;
	CArrayLargeInt			aiSeen;  //The chart whose frontier a face was last put on.
	CArrayLargeInt			aiMembers;
	SFloat32Vec3			sSum;
	SFloat32Vec3			sAverage;
	SFloat32Vec3*			psNormal;
	float32					fMinCosine;
	float32					fBest;
	float32					fDot;
	float32					fLength;
	int						iNumFaces;
	int						iNumCharts;
	int						iSeed;
	int						iFace;
	int						iBest;
	int						iNeighbour;
	int						i;
	int						j;
	bool					bAny;

	iNumFaces = pcMesh->NumFaces();
	fMinCosine = cosf(fMaxAngle);
	aiStarts.Init();
	aiNeighbours.Init();
	MeshChartsNeighbours(pcMesh, &aiStarts, &aiNeighbours);

	asNormals.Init();
	asAreaNormals.Init();
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		psNormal = asAreaNormals.Add();
		MeshFaceAreaNormal(psNormal, pcMesh, iFace);
		fLength = (float32)psNormal->Magnitude();
		psNormal = asNormals.Add();
		*psNormal = *asAreaNormals.Get((lsize)iFace);
		if (fLength > 0.0f)
		{
			*psNormal /= fLength;
		}
	}

	aiFrontier.Init();
	aiMembers.Init();
	aiSeen.Init();
	aiSeen.Resize((lsize)iNumFaces);
	aiSeen.SetArrayValues(-1);
	iNumCharts = NumCharts();
	do
	{
		bAny = false;
		for (iSeed = 0; iSeed < iNumFaces; iSeed++)
		{
			if (maiFaceCharts.GetValue((lsize)iSeed) != -1)
			{
				continue;
			}
			bAny = true;

			aiMembers.ReInit();
			aiFrontier.ReInit();
			aiFrontier.Add(iSeed);
			aiSeen.SetValue((lsize)iSeed, iNumCharts);
			sSum.Init(0, 0, 0);
			sAverage = *asNormals.Get((lsize)iSeed);
			while (aiFrontier.NumElements() > 0)
			{
				iBest = -1;
				fBest = -2.0f;
				for (i = 0; i < (int)aiFrontier.NumElements(); i++)
				{
					fDot = Float32Vec3Dot(asNormals.Get((lsize)aiFrontier.GetValue((lsize)i)), &sAverage);
					if (fDot > fBest)
					{
						fBest = fDot;
						iBest = i;
					}
				}
				if ((fBest < fMinCosine) && (aiMembers.NumElements() > 0))
				{
					break;
				}

				iFace = aiFrontier.GetValue((lsize)iBest);
				aiFrontier.SetValue((lsize)iBest, aiFrontier.GetValue(aiFrontier.NumElements() - 1));
				aiFrontier.RemoveNum(1);
				aiMembers.Add(iFace);
				maiFaceCharts.SetValue((lsize)iFace, iNumCharts);
				sSum += *asAreaNormals.Get((lsize)iFace);
				fLength = (float32)sSum.Magnitude();
				if (fLength > 0.0f)
				{
					sAverage = sSum / fLength;
				}

				for (j = aiStarts.GetValue((lsize)iFace); j < aiStarts.GetValue((lsize)iFace + 1); j++)
				{
					iNeighbour = aiNeighbours.GetValue((lsize)j);
					if ((maiFaceCharts.GetValue((lsize)iNeighbour) == -1) && (aiSeen.GetValue((lsize)iNeighbour) != iNumCharts))
					{
						aiSeen.SetValue((lsize)iNeighbour, iNumCharts);
						aiFrontier.Add(iNeighbour);
					}
				}
			}

			//The seed always stays, so every pass makes a chart and they end.  The normal is then the faces kept's.
			sSum = *asAreaNormals.Get((lsize)iSeed);
			for (i = 1; i < (int)aiMembers.NumElements(); i++)
			{
				iFace = aiMembers.GetValue((lsize)i);
				if (Float32Vec3Dot(asNormals.Get((lsize)iFace), &sAverage) < MESH_CHARTS_FOLD_COSINE)
				{
					maiFaceCharts.SetValue((lsize)iFace, -1);
				}
				else
				{
					sSum += *asAreaNormals.Get((lsize)iFace);
				}
			}
			fLength = (float32)sSum.Magnitude();
			if (fLength > 0.0f)
			{
				sAverage = sSum / fLength;
			}
			masNormals.Add(&sAverage);
			iNumCharts++;
		}
	}
	while (bAny);

	ListFaces(iNumFaces, iNumCharts);

	aiSeen.Kill();
	aiMembers.Kill();
	aiFrontier.Kill();
	asAreaNormals.Kill();
	asNormals.Kill();
	aiNeighbours.Kill();
	aiStarts.Kill();
}

