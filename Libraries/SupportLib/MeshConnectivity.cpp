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
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/PointerFunctions.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "MeshDefines.h"
#include "MeshConnectivity.h"
#include "BaseLib/ErrorHandler.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCornerEdgesMap::Init(void)
{
	maiLargerCornerIndex.Init();
	maiEdgeIndex.Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCornerEdgesMap::Kill(void)
{
	maiLargerCornerIndex.Kill();
	maiEdgeIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshConnectivity::Init(void)
{
	PreInit();

	CMeshDetail::Init(true);

	mcCorners.Init();
	mcEdges.Init();  //This assumes a closed surface.
	mcFaces.Init();

	mcCornerEdgesMap.Init();

	muiNumCorners = 0;
	muiGeneratedFaces = 0;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshConnectivity::Free(void)
{	
	KillCornerEdgesMap();
	KillCorners();
	mcEdges.Kill();
	mcFaces.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConnectivity::Class(void)
{
	CMeshDetail::Class();

	U_Unknown(CArrayCorner, mcCorners);
	U_Unknown(CArrayEdge, mcEdges);
	U_Unknown(CArrayFace, mcFaces);
	U_Unknown(CArrayCornerEdgesMap, mcCornerEdgesMap);
	U_LSize(muiNumCorners);
	U_LSize(muiGeneratedFaces);
}


//////////////////////////////////////////////////////////////////////////
//																				//
//																				//
//////////////////////////////////////////////////////////////////////////
void CMeshConnectivity::Clear(void)
{
	KillCornerEdgesMap();
	mcCornerEdgesMap.Init();

	KillCorners();
	mcCorners.Init();

	mcEdges.ReInit();
	mcFaces.ReInit();

	muiNumCorners = 0;
	muiGeneratedFaces = 0;
}


//////////////////////////////////////////////////////////////////////////
//																				//
//																				//
//////////////////////////////////////////////////////////////////////////
void CMeshConnectivity::KillCorners(void)
{
	lsize			i;
	CMeshCorner*	psCorner;
	lsize			uiNumElements;

	uiNumElements = mcCorners.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psCorner = mcCorners.Get(i);
		psCorner->Kill();
	}

	mcCorners.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																				//
//																				//
//////////////////////////////////////////////////////////////////////////
void CMeshConnectivity::KillCornerEdgesMap(void)
{
	lsize				i;
	CCornerEdgesMap*	pcCornerEdgeMap;
	lsize				uiNumElements;

	uiNumElements = mcCornerEdgesMap.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcCornerEdgeMap = mcCornerEdgesMap.Get(i);
		pcCornerEdgeMap->Kill();
	}

	mcCornerEdgesMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMFRet CMeshConnectivity::AddFace(int iCorner1, int iCorner2, int iCorner3)
{
	CMeshFace*		pcFace;
	int				iFaceNum;
	CMFRet			rFace;

	//Calling AddFace is the only public way to alter the connectivity of the mesh.
	iFaceNum = (int)mcFaces.NumElements();

	pcFace = AddFace();
	pcFace->Init(iCorner1, iCorner2, iCorner3);

	rFace.Pack(iFaceNum, 0, 1, 2, 0);
	return rFace;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshConnectivity::AddCorner(void)
{
	//Adding a corner does not alter the connectivity until it is part of a face.
	muiNumCorners++;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CMeshEdge* CMeshConnectivity::AddEdge(void)
{
	CMeshEdge*	psEdge;

	psEdge = mcEdges.Add();
	psEdge->Init();
	return psEdge;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CMeshFace* CMeshConnectivity::AddFace(void)
{
	CMeshFace*	psFace;

	psFace = mcFaces.Add();
	psFace->Init();
	return psFace;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
lsize CMeshConnectivity::NumCorners(void)
{
	return muiNumCorners;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CMeshConnectivity::NumFaces(void)
{
	return mcFaces.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CMeshConnectivity::NumEdges(void)
{
	GenerateConnectivity();
	return mcEdges.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CMeshConnectivity::FaceChunkSize(void)
{
	return mcFaces.ChunkSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CMeshConnectivity::CornerChunkSize(void)
{
	return mcCorners.ChunkSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMeshCorner* CMeshConnectivity::GetCorner(int iElementNum)
{
	GenerateConnectivity();
	return mcCorners.SafeGet(iElementNum);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CMeshEdge* CMeshConnectivity::GetEdge(int iElementNum)
{
	GenerateConnectivity();
	return mcEdges.SafeGet(iElementNum);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CMeshFace* CMeshConnectivity::GetFace(int iElementNum)
{
	GenerateConnectivity();
	return mcFaces.SafeGet(iElementNum);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int* CMeshConnectivity::GetFaceCorners(int iElementNum)
{
	return mcFaces.Get(iElementNum)->maiCorner;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCornerEdgesMap* CMeshConnectivity::GetCornerEdgesMap(int iElementNum)
{
	GenerateConnectivity();
	return mcCornerEdgesMap.SafeGet(iElementNum);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshConnectivity::Load(CObjectReader* pcFile)
{
	lsize		i;
	lsize		uiNumFaces;
	CMeshFace*	pcFace;

	ReturnOnFalse(CMeshDetail::Load(pcFile));
	ReturnOnFalse(pcFile->ReadSize(&muiNumCorners));
	ReturnOnFalse(mcFaces.Read(pcFile));

	uiNumFaces = mcFaces.NumElements();
	for (i = 0; i < uiNumFaces; i++)
	{
		pcFace = mcFaces.Get(i);
		pcFace->Init(pcFace->maiCorner[0], pcFace->maiCorner[1], pcFace->maiCorner[2]);
	}

	mcCorners.Init();
	mcEdges.Init();
	mcCornerEdgesMap.Init();
	muiGeneratedFaces = 0;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Only what can't be generated again is written.
//
//////////////////////////////////////////////////////////////////////////
bool CMeshConnectivity::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(CMeshDetail::Save(pcFile));
	ReturnOnFalse(pcFile->WriteSize(muiNumCorners));
	ReturnOnFalse(mcFaces.Write(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAdjFaces CMeshConnectivity::GetAdjacentFaces(int iFaceNum)
{
	CMeshFace*	psFace;
	size		i;
	CMeshEdge*	psEdge;
	int			iAdjFaceNum;
	int			iEdgeIndex;
	SAdjFaces	sAdjFaces;

	GenerateConnectivity();

	psFace = mcFaces.Get(iFaceNum);

	for (i = 0; i < NUM_FACE_EDGES; i++)
	{
		iAdjFaceNum = -1;
		iEdgeIndex = psFace->maiEdge[i];
		if (iEdgeIndex != -1)
		{
			psEdge = mcEdges.Get(iEdgeIndex);
			if ((psEdge->maiFace[0] != iFaceNum) && (psEdge->maiFace[0] != -1))
			{
				iAdjFaceNum = psEdge->maiFace[0];
			}
			else if ((psEdge->maiFace[1] != iFaceNum) && (psEdge->maiFace[1] != -1))
			{
				iAdjFaceNum = psEdge->maiFace[1];
			}
		}
		sAdjFaces.aiFace[i] = iAdjFaceNum;
	}
	return sAdjFaces;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshConnectivity::GenerateConnectivity(void)
{
	lsize		uiNumFaces;
	int			iNewEdges;
	CMeshFace*	pcFace;

	GenerateCorners();

	iNewEdges = 0;
	uiNumFaces = mcFaces.NumElements();
	for (; muiGeneratedFaces < uiNumFaces; muiGeneratedFaces++)
	{
		pcFace = mcFaces.Get(muiGeneratedFaces);
		GenerateCornerFaces(pcFace, (int)muiGeneratedFaces);
		iNewEdges += GenerateEdgeConnectivity(pcFace, (int)muiGeneratedFaces);
	}
	return iNewEdges;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshConnectivity::IsGenerated(void)
{
	return (mcCorners.NumElements() == muiNumCorners) && (muiGeneratedFaces == mcFaces.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConnectivity::GenerateCorners(void)
{
	CMeshCorner*		pcCorner;
	CCornerEdgesMap*	pcCornerEdgesMap;

	while (mcCorners.NumElements() < muiNumCorners)
	{
		pcCorner = mcCorners.Add();
		pcCorner->Init();

		pcCornerEdgesMap = mcCornerEdgesMap.Add();
		pcCornerEdgesMap->Init();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConnectivity::GenerateCornerFaces(CMeshFace* pcFace, int iFaceNum)
{
	size	i;

	for (i = 0; i < NUM_FACE_CORNERS; i++)
	{
		mcCorners.Get(pcFace->maiCorner[i])->maiFace.Add(iFaceNum);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshConnectivity::GenerateEdgeConnectivity(CMeshFace* pcFace, int iFaceNum)
{
	int		iNumEdges;

	iNumEdges =  GenerateEdgeConnectivity(pcFace, iFaceNum,  0, 1,  0);
	iNumEdges += GenerateEdgeConnectivity(pcFace, iFaceNum,  1, 2,  1);
	iNumEdges += GenerateEdgeConnectivity(pcFace, iFaceNum,  2, 0,  2);

	return iNumEdges;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshConnectivity::GenerateEdgeConnectivity(CMeshFace* pcFace, int iFaceNum, int iC1, int iC2, int iE)
{
	int					iCornerSmall;
	int					iCornerLarger;
	size				uiIndex;
	CMeshEdge*			pcEdge;
	int					iEdgeIndex;
	CMeshCorner*		pcCornerSmall;
	CMeshCorner*		pcCornerLarger;
	CCornerEdgesMap*	pcCornerEdgesMap;
	int					iAddedEdges;

	iCornerSmall = pcFace->maiCorner[iC1];
	iCornerLarger = pcFace->maiCorner[iC2];
	if (iCornerSmall > iCornerLarger)
	{
		SwapInt(&iCornerSmall, &iCornerLarger);
	}

	iAddedEdges = 0;
	pcCornerEdgesMap = mcCornerEdgesMap.Get(iCornerSmall);
	uiIndex = pcCornerEdgesMap->maiLargerCornerIndex.Find(iCornerLarger);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		pcCornerEdgesMap->maiLargerCornerIndex.Add(iCornerLarger);
		iEdgeIndex = (int)mcEdges.NumElements();
		pcCornerEdgesMap->maiEdgeIndex.Add(iEdgeIndex);
		pcEdge = AddEdge();
		iAddedEdges++;
	}
	else
	{
		iEdgeIndex = pcCornerEdgesMap->maiEdgeIndex.GetValue(uiIndex);
		pcEdge = mcEdges.Get(iEdgeIndex);
	}

	pcCornerSmall = mcCorners.Get(iCornerSmall);
	pcCornerSmall->maiEdge.AddIfUnique(iEdgeIndex);

	pcCornerLarger = mcCorners.Get(iCornerLarger);
	pcCornerLarger->maiEdge.AddIfUnique(iEdgeIndex);

	pcEdge->AddFace(iFaceNum);
	pcFace->maiEdge[iE] = iEdgeIndex;

	//This will sometimes be re-assigned to the same value.
	pcEdge->maiCorner[0] = iCornerSmall;
	pcEdge->maiCorner[1] = iCornerLarger;

	return iAddedEdges;
}

