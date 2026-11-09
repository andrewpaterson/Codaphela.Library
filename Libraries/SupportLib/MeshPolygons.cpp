/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MeshLib

Codaphela MeshLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MeshLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MeshLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/GlobalMemory.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "MeshConnectivity.h"
#include "MeshEdgeVisibility.h"
#include "MeshNormals.h"
#include "MeshPolygons.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshPolygon::Init(int iName)
{
	maiFaces.Init();
	miName = iName;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshPolygon::Kill(void)
{
	miName = -1;
	maiFaces.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMeshPolygon::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteInt32(miName));
	ReturnOnFalse(maiFaces.Write(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMeshPolygon::Load(CFileReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadInt32(&miName));
	ReturnOnFalse(maiFaces.Read(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshPolygons::Init(void)
{
	PreInit();

	CMeshDetail::Init();
	mcPolygons.Init();
	maiFacesToPolygons.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshPolygons::KillPolygons(void)
{
	size			i;
	CMeshPolygon*	pcPolygon;
	size			uiNumElements;

	uiNumElements = mcPolygons.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcPolygon = mcPolygons.Get(i);
		pcPolygon->Kill();
	}

	mcPolygons.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshPolygons::Free(void)
{
	maiFacesToPolygons.Kill();
	KillPolygons();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshPolygons::Class(void)
{
	CMeshDetail::Class();
	U_Unknown(CArrayMeshPolygon, mcPolygons);
	U_Unknown(CArrayInt, maiFacesToPolygons);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshPolygons::Clear(void)
{
	KillPolygons();
	mcPolygons.Init();

	maiFacesToPolygons.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshPolygons::Copy(CMeshPolygons* pcSource)
{
	CMeshPolygon*	pcFrom;
	CMeshPolygon*	pcTo;
	size			i;
	size			j;

	Clear();
	mbInUse = pcSource->mbInUse;
	for (i = 0; i < pcSource->mcPolygons.NumElements(); i++)
	{
		pcFrom = pcSource->mcPolygons.Get(i);
		pcTo = Add(pcFrom->miName);
		for (j = 0; j < pcFrom->maiFaces.NumElements(); j++)
		{
			pcTo->maiFaces.Add(pcFrom->maiFaces.GetValue(j));
		}
	}
	for (i = 0; i < pcSource->maiFacesToPolygons.NumElements(); i++)
	{
		maiFacesToPolygons.Add(pcSource->maiFacesToPolygons.GetValue(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether each of a face's three edges is on the outside of its polygon,
// which is how CMeshEditor::AddFace is told to make the same polygons
// again.  An edge is hidden when the face on its other side is in the same
// polygon.
//
//////////////////////////////////////////////////////////////////////////
void CMeshPolygons::GetFaceEdges(CMeshConnectivity* pcConn, int iFace, bool* pabVisible)
{
	SAdjFaces	sAdjFaces;
	int			iPolygon;
	int			i;

	for (i = 0; i < NUM_FACE_EDGES; i++)
	{
		pabVisible[i] = true;
	}
	if (!mbInUse || (iFace >= (int)maiFacesToPolygons.NumElements()))
	{
		return;
	}
	iPolygon = maiFacesToPolygons.GetValue(iFace);
	if (iPolygon == -1)
	{
		return;
	}

	pcConn->GenerateConnectivity();
	sAdjFaces = pcConn->GetAdjacentFaces(iFace);
	for (i = 0; i < NUM_FACE_EDGES; i++)
	{
		if ((sAdjFaces.aiFace[i] != -1) && (sAdjFaces.aiFace[i] < (int)maiFacesToPolygons.NumElements()) && (maiFacesToPolygons.GetValue(sAdjFaces.aiFace[i]) == iPolygon))
		{
			pabVisible[i] = false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMeshPolygons::Save(CObjectWriter* pcFile)
{
	size			i;
	CMeshPolygon*	psPolygon;
	size			uiNumElements;

	ReturnOnFalse(mcPolygons.WriteAllocatorAndHeader(pcFile));	

	uiNumElements = mcPolygons.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psPolygon = mcPolygons.Get(i);
		ReturnOnFalse(psPolygon->Save(pcFile));
	}

	ReturnOnFalse(maiFacesToPolygons.Write(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMeshPolygons::Load(CObjectReader* pcFile)
{
	size			i;
	CMeshPolygon*	psPolygon;
	size			uiNumElements;

	ReturnOnFalse(mcPolygons.ReadAllocatorAndHeader(pcFile));

	uiNumElements = mcPolygons.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psPolygon = mcPolygons.Get(i);
		ReturnOnFalse(psPolygon->Load(pcFile));
	}

	ReturnOnFalse(maiFacesToPolygons.Read(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CMeshPolygon* CMeshPolygons::Add(int iName)
{
	CMeshPolygon*		psPolygon;

	psPolygon = mcPolygons.Add();
	psPolygon->Init(iName);
	return psPolygon;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CMeshPolygon* CMeshPolygons::Get(int iPolygonNum)
{
	return mcPolygons.Get(iPolygonNum);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CMeshPolygon* CMeshPolygons::GrowToNumPolygons(int iNumPolygons, int iName)
{
	size			iOldNum;
	size			i;
	CMeshPolygon*		psPolygon;

	iOldNum = mcPolygons.Resize(iNumPolygons);
	for (i = iOldNum; i < (size)iNumPolygons; i++)
	{
		psPolygon = mcPolygons.Get(i);
		psPolygon->Init(iName);
	}

	psPolygon = mcPolygons.Get(iOldNum);
	return psPolygon;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshPolygons::GetFirstFaceNotInPolygon(CMeshConnectivity* pcConn, int iStartFaceIndex)
{
	size	i;
	size	iNumFaces;
	int		iPolygonIndex;

	if (pcConn->mcFaces.NumElements() != maiFacesToPolygons.NumElements())
	{
		return -1;
	}

	iNumFaces = pcConn->mcFaces.NumElements();
	for (i = (size)iStartFaceIndex; i < iNumFaces; i++)
	{
		iPolygonIndex = maiFacesToPolygons.GetValue(i);
		if (iPolygonIndex == -1)
		{
			return (int)i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshPolygons::PrivateIsPolygonContiguous(CMeshConnectivity* pcConn, CMeshPolygon* psPolygon, int* aiStack, int* aiTouched)
{
	size			iNumFaces;
	int				iFaceIndex;
	int				iAdjFaceIndex;
	int				iPolygonFaceIndex;
	int				iStackTop;
	size			iAdj;
	CMeshPolygon*	psPolygonForFace;
	size			i;
	SAdjFaces		sAdjFaces;

	iNumFaces = psPolygon->maiFaces.NumElements();

	memset(aiTouched, 0, (lsize)sizeof(int)*iNumFaces);
	memset(aiStack, -1, (lsize)sizeof(int)*iNumFaces);

	iPolygonFaceIndex = 0;

	//'Push' the first item onto the stack.
	iStackTop = 0;
	aiStack[iStackTop] = iPolygonFaceIndex;

	for (;;)
	{
		//Mark this item as touched.
		aiTouched[aiStack[iStackTop]] = 1;

		//Pop the stack.
		iStackTop--;

		iFaceIndex = psPolygon->maiFaces.GetValue(iPolygonFaceIndex);
		sAdjFaces = pcConn->GetAdjacentFaces(iFaceIndex);

		for (iAdj = 0; iAdj < 3; iAdj++)
		{
			iAdjFaceIndex = sAdjFaces.aiFace[iAdj];
			psPolygonForFace = GetPolygonForFace(iAdjFaceIndex);

			if (psPolygonForFace == psPolygon)			
			{
				iPolygonFaceIndex = (int)psPolygon->maiFaces.Find(iAdjFaceIndex);

				//Make sure we haven't already dealt with this item.
				if (aiTouched[iPolygonFaceIndex] == 0)
				{
					//Push this item onto the stack.
					iStackTop++;
					aiStack[iStackTop] = iPolygonFaceIndex;
				}
			}
		}

		//If there is nothing left on the stack...
		if (iStackTop == -1)
		{
			//Then we're done.
			break;
		}
	}

	for (i = 0; i < iNumFaces; i++)
	{
		if (aiTouched[i] != 1)
		{
			return false;
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshPolygons::IsPolygonContiguous(CMeshConnectivity* pcConn, int iPolygon)
{
	CMeshPolygon*	psPolygon;
	int				iNumFaces;
	int*			aiStack;
	int*			aiTouched;
	bool			bResult;

	psPolygon = mcPolygons.SafeGet(iPolygon);
	if (psPolygon)
	{
		iNumFaces = (int)psPolygon->maiFaces.NumElements();
		if (iNumFaces == 0)
		{
			return false;
		}
		aiStack = (int*)malloc((lsize)iNumFaces*sizeof(int)*2);
		aiTouched = (int*)RemapSinglePointer(aiStack, (lsize)iNumFaces*sizeof(int));

		bResult = PrivateIsPolygonContiguous(pcConn, psPolygon, aiStack, aiTouched);

		free(aiStack);

		return bResult;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMeshPolygon* CMeshPolygons::GetPolygonForFace(int iFaceNum)
{
	int*	piPolygon;

	piPolygon = (int*)maiFacesToPolygons.SafeGet(iFaceNum);
	if (piPolygon)
	{
		return mcPolygons.Get(*piPolygon);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshPolygons::GeneratePolygonsFromEdgeSelection(CMeshConnectivity* pcConn, CMeshEdgeVisibility* pcEdges)
{
	int			iFaceIndex;

	Clear();

	iFaceIndex = 0;
	for (;;)
	{
		iFaceIndex = GetFirstFaceNotInPolygon(pcConn, iFaceIndex);
		if (iFaceIndex == -1)
		{
			break;
		}

		GeneratePolygonFromEdgeSelection(pcConn, iFaceIndex, pcEdges);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshPolygons::GeneratePolygonFromEdgeSelection(CMeshConnectivity* pcConn, int iFaceIndex, CMeshEdgeVisibility* pcEdges)
{
	int				iAdjFaceIndex;
	CMeshPolygon*	pcPolygon;
	size			i;
	CMeshFace*		psFace;
	char*			cEdge;
	int				iEdgeIndex;
	CArrayInt		aiStack;
	int				iPolygonIndex;
	SAdjFaces		sAdjFaces;

	pcPolygon = Add(1);
	iPolygonIndex = (int)mcPolygons.NumElements()-1;
	aiStack.Init();
	aiStack.Push(iFaceIndex);

	for (;;)
	{
		iFaceIndex = aiStack.Pop();
		pcPolygon->maiFaces.Add(iFaceIndex);
		maiFacesToPolygons.SetValue(iFaceIndex, iPolygonIndex);

		psFace = pcConn->GetFace(iFaceIndex);
		sAdjFaces = pcConn->GetAdjacentFaces(iFaceIndex);

		for (i = 0; i < NUM_FACE_EDGES; i++)
		{
			iEdgeIndex = psFace->maiEdge[i];
			cEdge = pcEdges->mcEdges.Get(iEdgeIndex);
			if (!cEdge)
			{
				iAdjFaceIndex = sAdjFaces.aiFace[i];
				if (iAdjFaceIndex != -1)
				{
					if (maiFacesToPolygons.GetValue(iAdjFaceIndex) == -1)
					{
						aiStack.Push(iAdjFaceIndex);
					}
				}
			}
		}

		if (aiStack.NumElements() == 0)
		{
			break;
		}
	}
	aiStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshPolygons::AddFace(CMeshFace* pcFace, int iFace, int iName, CMeshConnectivity* pcConn, bool bEdge12, bool bEdge23, bool bEdge31)
{
	SAdjFaces		sAdjFaces;
	bool			abHidden[NUM_FACE_EDGES];
	int				aiJoined[NUM_FACE_EDGES];
	int				iNumJoined;
	int				iPolygon;
	int				iLastPolygon;
	CMeshPolygon*	pcPolygon;
	bool			bFound;
	int				i;
	int				j;

	if (!mbInUse)
	{
		return;
	}

	//The mesh numbers the faces, and it may have some that were added to it directly.
	while ((int)maiFacesToPolygons.NumElements() <= iFace)
	{
		maiFacesToPolygons.Add(-1);
	}

	sAdjFaces = pcConn->GetAdjacentFaces(iFace);
	abHidden[0] = !bEdge12;
	abHidden[1] = !bEdge23;
	abHidden[2] = !bEdge31;

	//The polygons on the far side of its hidden edges, each once.  A face added directly to the mesh has none.
	iNumJoined = 0;
	for (i = 0; i < NUM_FACE_EDGES; i++)
	{
		if (abHidden[i] && (sAdjFaces.aiFace[i] != -1))
		{
			iPolygon = maiFacesToPolygons.GetValue(sAdjFaces.aiFace[i]);
			bFound = iPolygon == -1;
			for (j = 0; j < iNumJoined; j++)
			{
				bFound |= aiJoined[j] == iPolygon;
			}
			if (!bFound)
			{
				aiJoined[iNumJoined] = iPolygon;
				iNumJoined++;
			}
		}
	}

	if (iNumJoined == 0)
	{
		maiFacesToPolygons.SetValue(iFace, (int)mcPolygons.NumElements());

		pcPolygon = mcPolygons.Add();
		pcPolygon->Init(iName);
		pcPolygon->maiFaces.Add(iFace);
		return;
	}

	maiFacesToPolygons.SetValue(iFace, aiJoined[0]);
	mcPolygons.Get(aiJoined[0])->maiFaces.Add(iFace);

	for (i = iNumJoined - 1; i > 0; i--)
	{
		iLastPolygon = (int)mcPolygons.NumElements() - 1;
		MergePolygon(aiJoined[0], aiJoined[i]);

		//The last polygon has moved to where the merged one was.
		for (j = 0; j < i; j++)
		{
			if (aiJoined[j] == iLastPolygon)
			{
				aiJoined[j] = aiJoined[i];
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshPolygons::GetUniqueNames(CArrayInt* paiNames)
{
	size			i;
	size			iNumPolygons;
	CMeshPolygon*	pcPolygon;

	iNumPolygons = mcPolygons.NumElements();

	//Generate polygon normals;
	for (i = 0; i < iNumPolygons; i++)
	{
		pcPolygon = mcPolygons.Get(i);
		paiNames->AddIfUnique(pcPolygon->miName);
	}

	paiNames->QuickSort();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshPolygons::NumUniqueNames(void)
{
	CArrayInt	cai;
	int			iUnique;

	cai.Init();
	GetUniqueNames(&cai);
	iUnique = (int)cai.NumElements();
	cai.Kill();

	return iUnique;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshPolygons::GetNormals(CArrayFloat32Vec3* pasNormals, CMeshNormals* pcNormals)
{
	size			i;
	size			iNumPolygons;
	CMeshPolygon*	pcPolygon;
	size			iNumFaces;
	size			j;
	int				iFace;
	SFloat32Vec3*	psNormal;
	SFloat32Vec3*	pasNormalsData;

	iNumPolygons = mcPolygons.NumElements();
	pasNormals->Resize(iNumPolygons);
	pasNormalsData = pasNormals->GetData();

	//Generate polygon normals;
	for (i = 0; i < iNumPolygons; i++)
	{
		pcPolygon = mcPolygons.Get(i);
		iNumFaces = pcPolygon->maiFaces.NumElements();
		pasNormalsData[i].Zero();
		for (j = 0; j < iNumFaces; j++)
		{
			iFace = pcPolygon->maiFaces.GetValue(j);
			psNormal = pcNormals->GetFaceNormal(iFace);
			pasNormalsData[i].Add(psNormal);
		}

		if (iNumFaces > 1)
		{
			pasNormalsData[i].Normalize();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshPolygons::MergePolygon(int iDestPolygon, int iSource)
{
	CMeshPolygon*	pcDestPolygon;
	CMeshPolygon*	pcSourcePolygon;
	size			i;
	int*			piPolygon;
	int				iFace;
	int				iLastPolygon;
	size			uiNumElements;

	pcDestPolygon = mcPolygons.Get(iDestPolygon);
	pcSourcePolygon = mcPolygons.Get(iSource);

	iLastPolygon = (int)mcPolygons.NumElements()-1;

	uiNumElements = maiFacesToPolygons.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		piPolygon = (int*)maiFacesToPolygons.Get(i);
		if (*piPolygon == iSource)
		{
			*piPolygon = iDestPolygon;
		}
	}

	for (i = 0; i < pcSourcePolygon->maiFaces.NumElements(); i++)
	{
		iFace = pcSourcePolygon->maiFaces.GetValue(i);
		pcDestPolygon->maiFaces.Add(iFace);
	}

	pcSourcePolygon->Kill();

	mcPolygons.RemoveAt(iSource, false);

	if (iLastPolygon != iSource)
	{
		uiNumElements = maiFacesToPolygons.NumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			piPolygon = (int*)maiFacesToPolygons.Get(i);
			if (*piPolygon == iLastPolygon)
			{
				*piPolygon = iSource;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshPolygons::GetAdjacentPolygons(CMeshConnectivity* pcConn, int iPolygon, CArrayInt* paiAdj)
{
	CMeshPolygon*	pcPolygon;
	size			i;
	int				iFace;
	size			j;
	SAdjFaces		sAdjFaces;
	int				iOtherGon;

	pcPolygon = mcPolygons.Get(iPolygon);
	
	for (i = 0; i < pcPolygon->maiFaces.NumElements(); i++)
	{
		iFace = pcPolygon->maiFaces.GetValue(i);
		sAdjFaces = pcConn->GetAdjacentFaces(iFace);

		for (j = 0; j < NUM_FACE_EDGES; j++)
		{
			if (sAdjFaces.aiFace[j] != -1)
			{
				iOtherGon = maiFacesToPolygons.GetValue(sAdjFaces.aiFace[j]);
				if (iOtherGon != iPolygon)
				{
					paiAdj->AddIfUnique(iOtherGon);
				}
			}
		}
	}
}

