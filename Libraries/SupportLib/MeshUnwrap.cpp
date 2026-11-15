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
#include "BaseLib/ArrayInt.h"
#include "BaseLib/Float32Vec2.h"
#include "StandardLib/Objects.h"
#include "MeshUnwrap.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshUnwrap::Init(float32 fTexelsPerUnit)
{
	PreInit();
	CMeshModifier::Init();
	mfTexelsPerUnit = fTexelsPerUnit;
	miPageSize = MESH_UNWRAP_PAGE;
	miGutter = MESH_UNWRAP_GUTTER;
	miLayoutWidth = 0;
	miLayoutHeight = 0;
	miNumCharts = 0;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshUnwrap::Class(void)
{
	CMeshModifier::Class();
	U_Float32(mfTexelsPerUnit);
	U_SInt(miPageSize);
	U_SInt(miGutter);
	U_SInt(miLayoutWidth);
	U_SInt(miLayoutHeight);
	U_SInt(miNumCharts);
}


//////////////////////////////////////////////////////////////////////////
//
// The direction a face's normal is nearest: (axis * 2) + 1 along it, axis
// * 2 against it.
//
//////////////////////////////////////////////////////////////////////////
static int MeshUnwrapDirection(CMesh* pcMesh, int iFace)
{
	int*			piCorners;
	SFloat32Vec3	sSide1;
	SFloat32Vec3	sSide2;
	SFloat32Vec3	sNormal;
	float32			afNormal[3];
	int				iAxis;
	int				i;

	piCorners = pcMesh->GetFaceCorners(iFace);
	sSide1 = *pcMesh->GetPosition(piCorners[1]) - *pcMesh->GetPosition(piCorners[0]);
	sSide2 = *pcMesh->GetPosition(piCorners[2]) - *pcMesh->GetPosition(piCorners[0]);
	Float32Vec3Cross(&sNormal, &sSide1, &sSide2);
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
	return (iAxis * 2) + ((afNormal[iAxis] > 0.0f) ? 1 : 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int MeshUnwrapRoot(CArrayLargeInt* paiParents, int i)
{
	while (paiParents->GetValue((lsize)i) != i)
	{
		paiParents->SetValue((lsize)i, paiParents->GetValue((lsize)paiParents->GetValue((lsize)i)));
		i = paiParents->GetValue((lsize)i);
	}
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
// Where a rectangle iWidth by iHeight goes on the skyline (each step its
// x, its height and its width, left to right, across iLayoutWidth): the
// step it starts at whose highest under it is lowest, the leftmost of
// those.  It's then put there and the skyline raised under it.
//
//////////////////////////////////////////////////////////////////////////
static void MeshUnwrapPlace(CArrayInt* paiSkyline, int iLayoutWidth, int iWidth, int iHeight, int* piX, int* piY)
{
	int		iNumSteps;
	int		i;
	int		j;
	int		x;
	int		y;
	int		iBestX;
	int		iBestY;
	int		iRight;
	int		iStepX;
	int		iStepY;
	int		iStepWidth;
	CArrayInt	aiNew;

	iNumSteps = (int)paiSkyline->NumElements() / 3;
	iBestX = 0;
	iBestY = -1;
	for (i = 0; i < iNumSteps; i++)
	{
		x = paiSkyline->GetValue((size)(i * 3));
		if (x + iWidth > iLayoutWidth)
		{
			break;
		}
		y = 0;
		for (j = i; j < iNumSteps; j++)
		{
			if (paiSkyline->GetValue((size)(j * 3)) >= x + iWidth)
			{
				break;
			}
			if (paiSkyline->GetValue((size)((j * 3) + 1)) > y)
			{
				y = paiSkyline->GetValue((size)((j * 3) + 1));
			}
		}
		if ((iBestY == -1) || (y < iBestY))
		{
			iBestX = x;
			iBestY = y;
		}
	}
	*piX = iBestX;
	*piY = iBestY;

	//The steps left of it, it, what's left of a step it covers part of, and the steps right of it.
	aiNew.Init();
	iRight = iBestX + iWidth;
	for (i = 0; i < iNumSteps; i++)
	{
		iStepX = paiSkyline->GetValue((size)(i * 3));
		iStepY = paiSkyline->GetValue((size)((i * 3) + 1));
		iStepWidth = paiSkyline->GetValue((size)((i * 3) + 2));
		if (iStepX + iStepWidth <= iBestX)
		{
			aiNew.Add(iStepX);
			aiNew.Add(iStepY);
			aiNew.Add(iStepWidth);
		}
		if (iStepX == iBestX)
		{
			aiNew.Add(iBestX);
			aiNew.Add(iBestY + iHeight);
			aiNew.Add(iWidth);
		}
		if ((iStepX + iStepWidth > iRight) && (iStepX < iRight))
		{
			aiNew.Add(iRight);
			aiNew.Add(iStepY);
			aiNew.Add(iStepX + iStepWidth - iRight);
		}
		else if (iStepX >= iRight)
		{
			aiNew.Add(iStepX);
			aiNew.Add(iStepY);
			aiNew.Add(iStepWidth);
		}
	}
	paiSkyline->ReInit();
	for (i = 0; i < (int)aiNew.NumElements(); i++)
	{
		paiSkyline->Add(aiNew.GetValue((size)i));
	}
	aiNew.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A copy of the mesh with UV layer 0 made again.  Faces are put in charts
// by their directions and the edges they share; each chart's corners are
// laid flat along its direction in texels (u along the next axis round
// and v the one after, u turned round looking against the axis, so faces
// facing that way stay anticlockwise); then the charts' rectangles are
// packed and the UVs set from where they went.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshUnwrap::Modify(Ptr<CMesh> pMesh)
{
	Ptr<CMesh>				pOutput;
	CMeshUVLayer*			pcLayer;
	SMeshUVFace*			psUVFace;
	CMeshEdge*				pcEdge;
	SFloat32Vec3*			psPosition;
	SFloat32Vec2*			psUV;
	CArrayLargeInt			aiDirections;
	CArrayLargeInt			aiParents;
	CArrayLargeInt			aiCharts;  //Each face's chart.
	CArrayLargeInt			aiChartStarts;  //Where each chart's faces start in aiChartFaces.
	CArrayLargeInt			aiChartFaces;
	CArrayLargeInt			aiCornerChart;  //The chart a corner last had a UV in,
	CArrayLargeInt			aiCornerUV;  //and that UV.
	CArrayLargeInt			aiUVCharts;  //Each UV's chart.
	CArrayLargeFloat32Vec2	asTexels;  //Each UV in texels along its chart's direction.
	CArrayLargeFloat32Vec2	asChartMinimums;
	CArrayLargeInt			aiChartSizes;  //Each chart's width and height in texels.
	CArrayLargeInt			aiChartPlaces;  //Where its rectangle went, gutters and all.
	CArrayLargeInt			aiOrder;
	CArrayInt				aiSkyline;
	int						iNumFaces;
	int						iNumCorners;
	int						iNumCharts;
	int						iFace;
	int						iChart;
	int						iCorner;
	int						iUV;
	int						iDirection;
	int						iAxis;
	int						iRoot0;
	int						iRoot1;
	int						i;
	int						j;
	int						k;
	int						x;
	int						y;
	int						iWidest;
	int						iLayoutWidth;
	int						iLayoutHeight;
	int						iGutters;
	int*					piCorners;
	float32					afPosition[3];
	float32					fU;
	float32					fV;
	float64					fArea;
	SFloat32Vec2*			psMinimum;

	iNumFaces = pMesh->NumFaces();
	if ((iNumFaces == 0) || (mfTexelsPerUnit <= 0.0f))
	{
		return pMesh;
	}
	iNumCorners = pMesh->NumCorners();

	//Charts: faces of one direction joined by edges.
	aiDirections.Init();
	aiParents.Init();
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		aiDirections.Add(MeshUnwrapDirection(&pMesh, iFace));
		aiParents.Add(iFace);
	}
	for (i = 0; i < pMesh->NumEdges(); i++)
	{
		pcEdge = pMesh->GetEdge(i);
		if ((pcEdge->maiFace[0] < 0) || (pcEdge->maiFace[1] < 0))
		{
			continue;
		}
		if (aiDirections.GetValue((lsize)pcEdge->maiFace[0]) != aiDirections.GetValue((lsize)pcEdge->maiFace[1]))
		{
			continue;
		}
		iRoot0 = MeshUnwrapRoot(&aiParents, pcEdge->maiFace[0]);
		iRoot1 = MeshUnwrapRoot(&aiParents, pcEdge->maiFace[1]);
		if (iRoot0 != iRoot1)
		{
			aiParents.SetValue((lsize)((iRoot0 > iRoot1) ? iRoot0 : iRoot1), (iRoot0 < iRoot1) ? iRoot0 : iRoot1);
		}
	}

	aiCharts.Init();
	aiCharts.Resize((lsize)iNumFaces);
	aiCharts.SetArrayValues(-1);
	iNumCharts = 0;
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		iRoot0 = MeshUnwrapRoot(&aiParents, iFace);
		if (aiCharts.GetValue((lsize)iRoot0) == -1)
		{
			aiCharts.SetValue((lsize)iRoot0, iNumCharts);
			iNumCharts++;
		}
		aiCharts.SetValue((lsize)iFace, aiCharts.GetValue((lsize)iRoot0));
	}

	aiChartStarts.Init();
	aiChartStarts.Resize((lsize)iNumCharts + 1);
	aiChartStarts.SetArrayValues(0);
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		iChart = aiCharts.GetValue((lsize)iFace);
		aiChartStarts.SetValue((lsize)iChart + 1, aiChartStarts.GetValue((lsize)iChart + 1) + 1);
	}
	for (iChart = 0; iChart < iNumCharts; iChart++)
	{
		aiChartStarts.SetValue((lsize)iChart + 1, aiChartStarts.GetValue((lsize)iChart + 1) + aiChartStarts.GetValue((lsize)iChart));
	}
	aiChartFaces.Init();
	aiChartFaces.Resize((lsize)iNumFaces);
	aiOrder.Init();
	aiOrder.Resize((lsize)iNumCharts);
	aiOrder.SetArrayValues(0);
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		iChart = aiCharts.GetValue((lsize)iFace);
		aiChartFaces.SetValue((lsize)(aiChartStarts.GetValue((lsize)iChart) + aiOrder.GetValue((lsize)iChart)), iFace);
		aiOrder.SetValue((lsize)iChart, aiOrder.GetValue((lsize)iChart) + 1);
	}

	//Each chart's corners laid flat in texels, a UV for each corner in each chart it's in.
	aiCornerChart.Init();
	aiCornerChart.Resize((lsize)iNumCorners);
	aiCornerChart.SetArrayValues(-1);
	aiCornerUV.Init();
	aiCornerUV.Resize((lsize)iNumCorners);
	aiUVCharts.Init();
	asTexels.Init();
	asChartMinimums.Init();
	asChartMinimums.Resize((lsize)iNumCharts);
	aiChartSizes.Init();
	aiChartSizes.Resize((lsize)iNumCharts * 2);

	pOutput = OMalloc<CMesh>();
	pOutput->CopyFrom(&pMesh);
	pOutput->mcUVs.mbInUse = true;
	pcLayer = pOutput->mcUVs.EnsureLayerExists(0, pOutput->GetConnectivity());
	pcLayer->mcUVs.ReInit();

	fArea = 0.0;
	iWidest = 0;
	iGutters = 2 * miGutter;
	for (iChart = 0; iChart < iNumCharts; iChart++)
	{
		iDirection = aiDirections.GetValue((lsize)aiChartFaces.GetValue((lsize)aiChartStarts.GetValue((lsize)iChart)));
		iAxis = iDirection / 2;
		psMinimum = asChartMinimums.Get((lsize)iChart);
		for (i = aiChartStarts.GetValue((lsize)iChart); i < aiChartStarts.GetValue((lsize)iChart + 1); i++)
		{
			iFace = aiChartFaces.GetValue((lsize)i);
			piCorners = pMesh->GetFaceCorners(iFace);
			psUVFace = pcLayer->mcFaces.Get((lsize)iFace);
			for (k = 0; k < NUM_FACE_CORNERS; k++)
			{
				iCorner = piCorners[k];
				if (aiCornerChart.GetValue((lsize)iCorner) != iChart)
				{
					psPosition = pMesh->GetPosition(iCorner);
					afPosition[0] = psPosition->x;
					afPosition[1] = psPosition->y;
					afPosition[2] = psPosition->z;
					fU = afPosition[(iAxis + 1) % 3] * mfTexelsPerUnit;
					fV = afPosition[(iAxis + 2) % 3] * mfTexelsPerUnit;
					if ((iDirection & 1) == 0)
					{
						fU = -fU;
					}
					iUV = (int)asTexels.NumElements();
					asTexels.Add()->Init(fU, fV);
					aiUVCharts.Add(iChart);
					if ((i == aiChartStarts.GetValue((lsize)iChart)) && (k == 0))
					{
						psMinimum->Init(fU, fV);
					}
					else
					{
						psMinimum->Init((fU < psMinimum->x) ? fU : psMinimum->x, (fV < psMinimum->y) ? fV : psMinimum->y);
					}
					aiCornerChart.SetValue((lsize)iCorner, iChart);
					aiCornerUV.SetValue((lsize)iCorner, iUV);
				}
				psUVFace->aiUVs[k] = aiCornerUV.GetValue((lsize)iCorner);
			}
		}
	}

	//Each chart's size, from its minimum to its maximum, rounded up to whole texels.
	for (iChart = 0; iChart < iNumCharts; iChart++)
	{
		aiChartSizes.SetValue((lsize)iChart * 2, 1);
		aiChartSizes.SetValue(((lsize)iChart * 2) + 1, 1);
	}
	for (iUV = 0; iUV < (int)asTexels.NumElements(); iUV++)
	{
		iChart = aiUVCharts.GetValue((lsize)iUV);
		psUV = asTexels.Get((lsize)iUV);
		psMinimum = asChartMinimums.Get((lsize)iChart);
		x = (int)ceilf(psUV->x - psMinimum->x);
		y = (int)ceilf(psUV->y - psMinimum->y);
		if (x > aiChartSizes.GetValue((lsize)iChart * 2))
		{
			aiChartSizes.SetValue((lsize)iChart * 2, x);
		}
		if (y > aiChartSizes.GetValue(((lsize)iChart * 2) + 1))
		{
			aiChartSizes.SetValue(((lsize)iChart * 2) + 1, y);
		}
	}
	for (iChart = 0; iChart < iNumCharts; iChart++)
	{
		x = aiChartSizes.GetValue((lsize)iChart * 2) + iGutters;
		y = aiChartSizes.GetValue(((lsize)iChart * 2) + 1) + iGutters;
		fArea += (float64)x * y;
		iWidest = (x > iWidest) ? x : iWidest;
	}

	//The layout as wide as the square of all the charts' area, or the widest, in whole pages.
	iLayoutWidth = (int)ceil(sqrt(fArea));
	iLayoutWidth = (iWidest > iLayoutWidth) ? iWidest : iLayoutWidth;
	iLayoutWidth = ((iLayoutWidth + miPageSize - 1) / miPageSize) * miPageSize;

	//Tallest first along the skyline.
	for (iChart = 0; iChart < iNumCharts; iChart++)
	{
		aiOrder.SetValue((lsize)iChart, iChart);
	}
	for (i = 1; i < iNumCharts; i++)
	{
		iChart = aiOrder.GetValue((lsize)i);
		j = i - 1;
		while ((j >= 0) && (aiChartSizes.GetValue(((lsize)aiOrder.GetValue((lsize)j) * 2) + 1) < aiChartSizes.GetValue(((lsize)iChart * 2) + 1)))
		{
			aiOrder.SetValue((lsize)j + 1, aiOrder.GetValue((lsize)j));
			j--;
		}
		aiOrder.SetValue((lsize)(j + 1), iChart);
	}

	aiChartPlaces.Init();
	aiChartPlaces.Resize((lsize)iNumCharts * 2);
	aiSkyline.Init();
	aiSkyline.Add(0);
	aiSkyline.Add(0);
	aiSkyline.Add(iLayoutWidth);
	iLayoutHeight = 0;
	for (i = 0; i < iNumCharts; i++)
	{
		iChart = aiOrder.GetValue((lsize)i);
		MeshUnwrapPlace(&aiSkyline, iLayoutWidth, aiChartSizes.GetValue((lsize)iChart * 2) + iGutters, aiChartSizes.GetValue(((lsize)iChart * 2) + 1) + iGutters, &x, &y);
		aiChartPlaces.SetValue((lsize)iChart * 2, x);
		aiChartPlaces.SetValue(((lsize)iChart * 2) + 1, y);
		y += aiChartSizes.GetValue(((lsize)iChart * 2) + 1) + iGutters;
		iLayoutHeight = (y > iLayoutHeight) ? y : iLayoutHeight;
	}
	iLayoutHeight = ((iLayoutHeight + miPageSize - 1) / miPageSize) * miPageSize;

	//The UVs: where each is in its chart, where the chart went, across the layout.
	for (iUV = 0; iUV < (int)asTexels.NumElements(); iUV++)
	{
		iChart = aiUVCharts.GetValue((lsize)iUV);
		psUV = asTexels.Get((lsize)iUV);
		psMinimum = asChartMinimums.Get((lsize)iChart);
		fU = (psUV->x - psMinimum->x) + (float32)(aiChartPlaces.GetValue((lsize)iChart * 2) + miGutter);
		fV = (psUV->y - psMinimum->y) + (float32)(aiChartPlaces.GetValue(((lsize)iChart * 2) + 1) + miGutter);
		pcLayer->mcUVs.Add()->Init(fU / (float32)iLayoutWidth, fV / (float32)iLayoutHeight);
	}

	miLayoutWidth = iLayoutWidth;
	miLayoutHeight = iLayoutHeight;
	miNumCharts = iNumCharts;

	aiSkyline.Kill();
	aiChartPlaces.Kill();
	aiChartSizes.Kill();
	asChartMinimums.Kill();
	asTexels.Kill();
	aiUVCharts.Kill();
	aiCornerUV.Kill();
	aiCornerChart.Kill();
	aiOrder.Kill();
	aiChartFaces.Kill();
	aiChartStarts.Kill();
	aiCharts.Kill();
	aiParents.Kill();
	aiDirections.Kill();
	return pOutput;
}

