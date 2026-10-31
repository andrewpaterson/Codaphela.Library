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
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshConnectivity::Clear(void)
{
	KillCornerEdgesMap();
	mcCornerEdgesMap.Init();

	KillCorners();
	mcCorners.Init();

	mcCorners.ReInit();
	mcEdges.ReInit();
	mcFaces.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshConnectivity::KillCorners(void)
{
	size			i;
	CMeshCorner*	psCorner;
	size			uiNumElements;

	uiNumElements = mcCorners.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psCorner = mcCorners.Get(i);
		psCorner->Kill();
	}

	mcCorners.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshConnectivity::KillCornerEdgesMap(void)
{
	size				i;
	CCornerEdgesMap*	pcCornerEdgeMap;
	size				uiNumElements;

	uiNumElements = mcCornerEdgesMap.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcCornerEdgeMap = mcCornerEdgesMap.Get(i);
		pcCornerEdgeMap->Kill();
	}

	mcCornerEdgesMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshConnectivity::InitCorners(size iNumCorners)
{
	size			i;
	CMeshCorner*	psCorner;

	mcCorners.Init();
	mcCorners.Resize(iNumCorners);

	for (i = 0; i < iNumCorners; i++)
	{
		psCorner = mcCorners.Get(i);
		psCorner->Init();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMFRet CMeshConnectivity::AddFace(int iCorner1, int iCorner2, int iCorner3)
{
	CMeshFace*		pcFace;
	CMeshCorner*	pcCorner;
	int				iFaceNum;
	int				iNewEdges;
	CMFRet			rFace;

	//Calling AddFace/RemoveFace is the only public way to alter the connectivity of the mesh.
	iFaceNum = (int)mcFaces.NumElements();

	pcFace = AddFace();
	pcFace->Init(iCorner1, iCorner2, iCorner3);

	pcCorner = mcCorners.Get(pcFace->asCorner[0]);
	pcCorner->aiFace.Add(iFaceNum);

	pcCorner = mcCorners.Get(pcFace->asCorner[1]);
	pcCorner->aiFace.Add(iFaceNum);

	pcCorner = mcCorners.Get(pcFace->asCorner[2]);
	pcCorner->aiFace.Add(iFaceNum);

	iNewEdges = GenerateEdgeConnectivity(pcFace, iFaceNum);

	rFace.Pack(iFaceNum, 0, 1, 2,iNewEdges);
	return rFace;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CMeshCorner* CMeshConnectivity::AddCorner(void)
{
	CMeshCorner*		psCorner;
	CCornerEdgesMap*	pcCornerEdgesMap;

	//Adding a corner does not alter the connectivity until it is part of a face.
	psCorner = mcCorners.Add();
	psCorner->Init();

	pcCornerEdgesMap = mcCornerEdgesMap.Add();
	pcCornerEdgesMap->Init();

	return psCorner;
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
CMeshCorner* CMeshConnectivity::GetCorner(int iElementNum)
{
	return mcCorners.SafeGet(iElementNum);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CMeshEdge* CMeshConnectivity::GetEdge(int iElementNum)
{
	return mcEdges.SafeGet(iElementNum);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CMeshFace* CMeshConnectivity::GetFace(int iElementNum)
{
	return mcFaces.SafeGet(iElementNum);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshConnectivity::Load(CObjectReader* pcFile)
{
	size			i;
	CMeshCorner*	pcCorner;
	size			uiNumElements;

	ReturnOnFalse(CMeshDetail::Load(pcFile));

	if (mcCorners.ReadAllocatorAndHeader(pcFile))
	{
		gcUserError.Set("Could not read corners.");
		return false;
	}

	//Read in each corner... they contain arrays themselves.
	uiNumElements = mcCorners.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcCorner = mcCorners.Get(i);
		if (!pcCorner->Load(pcFile))
		{
			gcUserError.Set("Could not read corner.");
			return false;
		}
	}

	//Read in the edges array.
	if (mcEdges.Read(pcFile))
	{
		gcUserError.Set("Could not read edges array.");
		return false;
	}


	//Read in the faces array.  
	if (mcFaces.Read(pcFile))
	{
		gcUserError.Set("Could not read faces array.");
		return false;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshConnectivity::Save(CObjectWriter* pcFile)
{
	size			i;
	CMeshCorner*	pcCorner;
	size			uiNumElements;

	ReturnOnFalse(CMeshDetail::Save(pcFile));

	ReturnOnFalse(mcCorners.WriteAllocatorAndHeader(pcFile));

	uiNumElements = mcCorners.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcCorner = mcCorners.Get(i);
		if (!pcCorner->Save(pcFile))
		{
			gcUserError.Set("Could not read corner.");
			return false;
		}
	}

	ReturnOnFalse(mcEdges.Write(pcFile));
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

	psFace = mcFaces.Get(iFaceNum);

	for (i = 0; i < NUM_FACE_EDGES; i++)
	{
		iAdjFaceNum = -1;
		iEdgeIndex = psFace->asEdge[i];
		if (iEdgeIndex != -1)
		{
			psEdge = mcEdges.Get(iEdgeIndex);
			if ((psEdge->aiFace[0] != iFaceNum) && (psEdge->aiFace[0] != -1))
			{
				iAdjFaceNum = psEdge->aiFace[0];
			}
			else if ((psEdge->aiFace[1] != iFaceNum) && (psEdge->aiFace[1] != -1))
			{
				iAdjFaceNum = psEdge->aiFace[1];
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
int CMeshConnectivity::GenerateEdgeConnectivity(void)
{
	size				iCornerNum;
	size				iFaceNum;
	size				iNumCorners;
	CCornerEdgesMap*	pcCornerEdgesMap;
	CMeshFace*			pcFace;
	int					iNumEdges;
	size				iNumFaces;

	mcEdges.Kill();
	mcEdges.Init();

	KillCornerEdgesMap();
	mcCornerEdgesMap.Init();

	iNumCorners = mcCorners.NumElements();

	for (iCornerNum = 0; iCornerNum < iNumCorners; iCornerNum++)
	{
		pcCornerEdgesMap = mcCornerEdgesMap.Add();
		pcCornerEdgesMap->Init();
	}

	iNumEdges = 0;
	iNumFaces = mcFaces.NumElements();
	for (iFaceNum = 0; iFaceNum < iNumFaces; iFaceNum++)
	{
		pcFace = mcFaces.Get(iFaceNum);
		iNumEdges += GenerateEdgeConnectivity(pcFace, (int)iFaceNum);
	}
	return iNumEdges;
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

	iCornerSmall = pcFace->asCorner[iC1];
	iCornerLarger = pcFace->asCorner[iC2];
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
		pcEdge = GetEdge(iEdgeIndex);
	}

	pcCornerSmall = mcCorners.Get(iCornerSmall);
	pcCornerSmall->aiEdge.AddIfUnique(iEdgeIndex);

	pcCornerLarger = mcCorners.Get(iCornerLarger);
	pcCornerLarger->aiEdge.AddIfUnique(iEdgeIndex);

	pcEdge->AddFace(iFaceNum);
	pcFace->asEdge[iE] = iEdgeIndex;

	//This will sometimes be re-assigned to the same value.
	pcEdge->aiCorner[0] = iCornerSmall;
	pcEdge->aiCorner[1] = iCornerLarger;

	return iAddedEdges;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConnectivity::IndexFaceCorners(int* paiDest, int iStride, CArrayInt* paiSourceFaces)
{
	size		i;
	int			iFaceIndex;
	CMeshFace*	pcFace;
	size		uiNumElements;

	uiNumElements = paiSourceFaces->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		iFaceIndex = paiSourceFaces->GetValue(i);
		pcFace = mcFaces.Get(iFaceIndex);

		*paiDest = pcFace->asCorner[0];
		paiDest = (int*)RemapSinglePointer(paiDest, iStride);

		*paiDest = pcFace->asCorner[1];
		paiDest = (int*)RemapSinglePointer(paiDest, iStride);

		*paiDest = pcFace->asCorner[2];
		paiDest = (int*)RemapSinglePointer(paiDest, iStride);
	}
}

