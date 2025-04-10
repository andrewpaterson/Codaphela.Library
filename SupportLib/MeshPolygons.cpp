/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "StandardLib/ClassDefines.h"
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
	ReturnOnFalse(maiFaces.Write(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMeshPolygon::Load(CFileReader* pcFile)
{
	ReturnOnFalse(maiFaces.Read(pcFile));
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshPolygons::Init(void)
{
	PreInit();

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
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMeshPolygons::Save(CObjectWriter* pcFile)
{
	size			i;
	CMeshPolygon*	psPolygon;

	ReturnOnFalse(mcPolygons.WriteAllocatorAndHeader(pcFile));	

	for (i = 0; i < mcPolygons.NumElements(); i++)
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

	ReturnOnFalse(mcPolygons.ReadAllocatorAndHeader(pcFile));

	for (i = 0; i < mcPolygons.NumElements(); i++)
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
	int				iOldNum;
	int				i;
	CMeshPolygon*		psPolygon;

	iOldNum = mcPolygons.Resize(iNumPolygons);
	for (i = iOldNum; i < iNumPolygons; i++)
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
	int		i;
	int		iNumFaces;
	int		iPolygonIndex;

	if (pcConn->mcFaces.NumElements() != maiFacesToPolygons.NumElements())
	{
		return -1;
	}

	iNumFaces = pcConn->mcFaces.NumElements();
	for (i = iStartFaceIndex; i < iNumFaces; i++)
	{
		iPolygonIndex = maiFacesToPolygons.GetValue(i);
		if (iPolygonIndex == -1)
		{
			return i;
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
	int				iNumFaces;
	int				iFaceIndex;
	int				iAdjFaceIndex;
	int				iPolygonFaceIndex;
	int				iStackTop;
	int				iAdj;
	CMeshPolygon*	psPolygonForFace;
	int				i;
	SAdjFaces		sAdjFaces;

	iNumFaces = psPolygon->maiFaces.NumElements();

	memset(aiTouched, 0, sizeof(int)*iNumFaces);
	memset(aiStack, -1, sizeof(int)*iNumFaces);

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
				iPolygonFaceIndex = psPolygon->maiFaces.Find(iAdjFaceIndex);

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
		iNumFaces = psPolygon->maiFaces.NumElements();
		if (iNumFaces == 0)
		{
			return false;
		}
		aiStack = (int*)malloc(iNumFaces*sizeof(int)*2);
		aiTouched = (int*)RemapSinglePointer(aiStack, iNumFaces*sizeof(int));

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
	int				i;
	CMeshFace*		psFace;
	char*			cEdge;
	int				iEdgeIndex;
	CArrayInt		aiStack;
	int				iPolygonIndex;
	SAdjFaces		sAdjFaces;

	pcPolygon = Add(1);
	iPolygonIndex = mcPolygons.NumElements()-1;
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
			iEdgeIndex = psFace->asEdge[i];
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
	int				aiPolygon[NUM_FACE_EDGES];
	CMeshPolygon*	pcPolygon;
	bool			bPartOfExisting;
	bool			bCrush2;
	bool			bCrush3;

	maiFacesToPolygons.Add(-1);

	aiPolygon[0] = -1;
	aiPolygon[1] = -1;
	aiPolygon[2] = -1;
	bPartOfExisting = false;
	bCrush2 = false;
	bCrush3 = false;

	sAdjFaces = pcConn->GetAdjacentFaces(iFace);

	if (bEdge12 == false)
	{
		if (sAdjFaces.aiFace[0] != -1)
		{
			aiPolygon[0] = maiFacesToPolygons.GetValue(sAdjFaces.aiFace[0]);
			pcPolygon = mcPolygons.Get(aiPolygon[0]);
			pcPolygon->maiFaces.Add(iFace);
			maiFacesToPolygons.SetValue(iFace, aiPolygon[0]);
			bPartOfExisting = true;
		}
	}

	if (bEdge23 == false)
	{
		if (sAdjFaces.aiFace[1] != -1)
		{
			aiPolygon[1] = maiFacesToPolygons.GetValue(sAdjFaces.aiFace[1]);
			pcPolygon = mcPolygons.Get(aiPolygon[1]);
			if (aiPolygon[0] != aiPolygon[1])
			{
				if (bPartOfExisting)
				{
					bCrush2 = true;
				}
				else
				{
					maiFacesToPolygons.SetValue(iFace, aiPolygon[1]);
					pcPolygon->maiFaces.Add(iFace);
					bPartOfExisting = true;
				}
			}
		}
	}

	if (bEdge31 == false)
	{
		if (sAdjFaces.aiFace[2] != -1)
		{
			aiPolygon[2] = maiFacesToPolygons.GetValue(sAdjFaces.aiFace[2]);
			pcPolygon = mcPolygons.Get(aiPolygon[2]);
			if ((aiPolygon[0] != aiPolygon[2]) || (aiPolygon[1] != aiPolygon[2]))
			{
				if (bPartOfExisting)
				{
					bCrush3 = true;
				}
				else
				{
					maiFacesToPolygons.SetValue(iFace, aiPolygon[2]);
					pcPolygon->maiFaces.Add(iFace);
					bPartOfExisting = true;
				}
			}
		}
	}

	if (!bPartOfExisting)
	{
		maiFacesToPolygons.SetValue(iFace, mcPolygons.NumElements());

		pcPolygon = mcPolygons.Add();
		pcPolygon->Init(iName);
		pcPolygon->maiFaces.Add(iFace);
	}
	else
	{
		if (bCrush2 && bCrush3)
		{
			MergePolygon(aiPolygon[0], aiPolygon[1], aiPolygon[2]);
		}
		else if (bCrush2)
		{
			MergePolygon(aiPolygon[0], aiPolygon[1]);
		}
		else if (bCrush3)
		{
			if (aiPolygon[0] != -1)
			{
				MergePolygon(aiPolygon[0], aiPolygon[2]);
			}
			else
			{
				MergePolygon(aiPolygon[1], aiPolygon[2]);
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
	int				i;
	int				iNumPolygons;
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
	iUnique = cai.NumElements();
	cai.Kill();

	return iUnique;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshPolygons::GetNormals(CArrayFloat3* pasNormals, CMeshNormals* pcNormals)
{
	int				i;
	int				iNumPolygons;
	CMeshPolygon*	pcPolygon;
	int				iNumFaces;
	int				j;
	int				iFace;
	SFloat3*		psNormal;
	SFloat3*		pasNormalsData;

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
void CMeshPolygons::MergePolygon(int iDestPolygon, int iSource1, int iSource2)
{
	MergePolygon(iDestPolygon, iSource2);

	if (iSource1 != mcPolygons.NumElements())
	{
		MergePolygon(iDestPolygon, iSource1);
	}
	else
	{
		//Then this index has changed.
		//It became iSource2 after merging.
		MergePolygon(iDestPolygon, iSource2);
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

	pcDestPolygon = mcPolygons.Get(iDestPolygon);
	pcSourcePolygon = mcPolygons.Get(iSource);

	iLastPolygon = mcPolygons.NumElements()-1;

	for (i = 0; i < maiFacesToPolygons.NumElements(); i++)
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
		for (i = 0; i < maiFacesToPolygons.NumElements(); i++)
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

