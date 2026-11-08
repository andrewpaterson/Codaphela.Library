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
#include "Mesh.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "MeshEditor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::Init(Ptr<CMesh> pMesh)
{
	PreInit();

	mpMesh = pMesh;

	mcSelections.Init();
	mcEdgeVisibility.Init();
	mcPolygons.Init();

	mcModifiers.Init();

	mbBaseKept = false;
	mpBase = NULL;
	macBaseEdges.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::Free(void)
{
	macBaseEdges.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::Class(void)
{
	M_Pointer(mpMesh);
	M_Embedded(mcModifiers);
	M_Embedded(mcSelections);
	M_Embedded(mcEdgeVisibility);
	M_Embedded(mcPolygons);
	U_Bool(mbBaseKept);
	M_Pointer(mpBase);
	U_Unknown(CArrayChar, macBaseEdges);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshEditor::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(mcSelections.Save(pcFile));
	ReturnOnFalse(mcEdgeVisibility.Save(pcFile));
	ReturnOnFalse(mcPolygons.Save(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshEditor::Load(CObjectReader* pcFile)
{
	ReturnOnFalse(mcSelections.Load(pcFile));
	ReturnOnFalse(mcEdgeVisibility.Load(pcFile));
	ReturnOnFalse(mcPolygons.Load(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::Touch(void)
{
	mcModifiers.ApplyAll(this);

	mpMesh->Touch();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::Collapse(void)
{
	Touch();
	RemoveModifiers();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::RemoveModifiers(void)
{
	mcModifiers.ReInit();
	ForgetBase();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshEditor::NumCorners(void)
{
	return mpMesh->NumCorners();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshEditor::NumFaces(void)
{
	return mpMesh->NumFaces();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshEditor::NumPolygons(void)
{
	return (int)mcPolygons.mcPolygons.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshEditor::NumEdges(void)
{
	return mpMesh->NumEdges();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshEditor::NumVisibleEdges(char cEdge)
{
	return (int)mcEdgeVisibility.NumVisibleEdges(cEdge);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::ClearConnectivity(void)
{
	ClearMesh();
	ForgetBase();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::ClearMesh(void)
{
	mpMesh->ClearConnectivity();
	
	mcSelections.Clear();
	mcEdgeVisibility.Clear();
	mcPolygons.Clear();

	mcModifiers.ClearConnectivity();
}


//////////////////////////////////////////////////////////////////////////
//
// The first time the modifiers are applied.  A mesh with what a copy
// wouldn't keep can't be gone back to, so its shapes start from nothing.
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::KeepBase(void)
{
	CMeshFace*	pcFace;
	int			i;
	int			k;
	int			iNumFaces;
	int			iEdge;

	if (mbBaseKept)
	{
		return;
	}
	mbBaseKept = true;

	if (mpMesh->NumCorners() == 0)
	{
		return;
	}
	if (!mpMesh->CanCopy())
	{
		gcLogger.Warning2(__METHOD__, " Can't keep a mesh's UVs, skin or materials to add shapes to.  It is emptied.", NULL);
		return;
	}

	mpBase = OMalloc<CMesh>();
	mpBase->CopyFrom(&mpMesh);

	iNumFaces = mpMesh->NumFaces();
	for (i = 0; i < iNumFaces; i++)
	{
		pcFace = mpMesh->GetFace(i);
		for (k = 0; k < NUM_FACE_EDGES; k++)
		{
			iEdge = pcFace->maiEdge[k];
			macBaseEdges.Add((iEdge < (int)mcEdgeVisibility.mcEdges.NumElements()) ? mcEdgeVisibility.GetEdge(iEdge) : 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::ForgetBase(void)
{
	mbBaseKept = false;
	mpBase = NULL;
	macBaseEdges.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// Through AddFace so that its polygons, which its visible edges decide, come
// back as they were.  What was selected doesn't.
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::RestoreBase(void)
{
	int*	piCorners;
	int		i;
	int		iNumCorners;
	int		iNumFaces;

	KeepBase();
	ClearMesh();

	if (mpBase.IsNotNull())
	{
		iNumCorners = mpBase->NumCorners();
		for (i = 0; i < iNumCorners; i++)
		{
			AddCorner(mpBase->GetPosition(i));
		}

		iNumFaces = mpBase->NumFaces();
		for (i = 0; i < iNumFaces; i++)
		{
			piCorners = mpBase->GetFaceCorners(i);
			AddFace(piCorners[0], piCorners[1], piCorners[2], mpBase->GetFaceName(i),
					macBaseEdges.GetValue(i * NUM_FACE_EDGES) != 0, macBaseEdges.GetValue(i * NUM_FACE_EDGES + 1) != 0, macBaseEdges.GetValue(i * NUM_FACE_EDGES + 2) != 0);
		}

		mpMesh->CopyDetailsFrom(&mpBase);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::AddCorner(float32 x, float32 y, float32 z)
{
	SFloat32Vec3		s;

	s.Init(x, y, z);

	mpMesh->AddCorner(&s);
	mcModifiers.AddPosition(&s);
	mcSelections.AddCorner();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::AddCorner(SFloat32Vec3* psPosition)
{
	mpMesh->AddCorner(psPosition);
	mcModifiers.AddPosition(psPosition);
	mcSelections.AddCorner();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMFRet CMeshEditor::AddFace(int iCorner1, int iCorner2, int iCorner3, int iName, bool bEdge12, bool bEdge23, bool bEdge31)
{
	CMFRet				rFace;
	CMeshFace*			pcFace;
	CMeshConnectivity*	pcConn;
	int					iNewEdges;

	//Selections, edge visibility and polygons need this one face's edges, not those of faces added directly to the mesh.
	pcConn = mpMesh->GetConnectivity();
	pcConn->GenerateConnectivity();

	rFace = mpMesh->AddFace(iCorner1, iCorner2, iCorner3);
	iNewEdges = pcConn->GenerateConnectivity();
	rFace.Pack(rFace.miFirstFace, 0, 1, 2, iNewEdges);
	mpMesh->SetFaceName(rFace.miFirstFace, iName);
	pcFace = mpMesh->GetFace(rFace.miFirstFace);

	mcSelections.AddFace(rFace.NewEdges());
	mcEdgeVisibility.AddFace(pcFace, bEdge12, bEdge23, bEdge31);
	mcPolygons.AddFace(pcFace, rFace.miFirstFace, iName, mpMesh->GetConnectivity(), bEdge12, bEdge23, bEdge31);

	mcModifiers.AddFace(iCorner1, iCorner2, iCorner3);

	return rFace;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMFRet CMeshEditor::AddFace(int iCorner1, int iCorner2, int iCorner3, int iName, bool bEdge12, bool bEdge23, bool bEdge31, bool bFlipFaces)
{
	CMFRet	rFace;

	if (!bFlipFaces)
	{
		return AddFace(iCorner1, iCorner2, iCorner3, iName, bEdge12, bEdge23, bEdge31);
	}
	else
	{
		rFace = AddFace(iCorner1, iCorner3, iCorner2, iName, bEdge12, bEdge23, bEdge31);
		rFace.Repack(0, 2, 1);
		return rFace;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMFRet CMeshEditor::AddQuad(int iCorner1, int iCorner2, int iCorner3, int iCorner4, int iName, bool bFlipFaces)
{
	CMFRet	rFace1;
	CMFRet	rFace2;

	if (!bFlipFaces)
	{
		rFace1 = AddFace(iCorner1, iCorner2, iCorner4, iName, true, false, true);
		rFace1.Repack(0, 1, 3);
		rFace2 = AddFace(iCorner2, iCorner3, iCorner4, iName, true, true, false);
		rFace2.Repack(1, 2, 3);
		rFace1.Pack2(rFace2);
	}
	else
	{
		rFace1 = AddFace(iCorner1, iCorner4, iCorner2, iName, true, false, true);
		rFace1.Repack(0, 3, 1);
		rFace2 = AddFace(iCorner2, iCorner4, iCorner3, iName, false, true, true);
		rFace2.Repack(1, 3, 2);
		rFace1.Pack2(rFace2);
	}
	return rFace1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMFRet CMeshEditor::AddPolygon(CArrayInt* paiCorners, int iName, bool bFlipFaces)
{
	CMFRet	r;

	if (paiCorners->NumElements() < 3)
	{
		r.PackEmpty();
		return r;
	}
	else if (paiCorners->NumElements() == 3)
	{
		return AddFace(paiCorners->GetValue(0), paiCorners->GetValue(1), paiCorners->GetValue(2), iName, true, true, true, bFlipFaces);
	}
	else if (paiCorners->NumElements() == 4)
	{
		return AddQuad(paiCorners->GetValue(0), paiCorners->GetValue(1), paiCorners->GetValue(2), paiCorners->GetValue(3), iName, bFlipFaces);
	}
	else
	{
		r.PackOverflow();
		return r;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::RemoveFace(int iFace)
{
	mpMesh->RemoveFace(iFace);
	mcModifiers.RemoveFace(iFace);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::RemoveFaces(CArrayInt* paiFaces)
{
	mpMesh->RemoveFaces(paiFaces);
	mcModifiers.RemoveFaces(paiFaces);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::RemoveCorner(int iCorner)
{
	mpMesh->RemoveCorner(iCorner);
	mcModifiers.RemoveCorner(iCorner);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::MovePosition(int iPosition, SFloat32Vec3* psPosition)
{
	mpMesh->MovePosition(iPosition, psPosition);
	mcModifiers.MovePosition(iPosition, psPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::MovePositions(CArrayInt* paiPositions, SFloat32Vec3* psPosition)
{
	mpMesh->MovePositions(paiPositions, psPosition);
	mcModifiers.MovePositions(paiPositions, psPosition);
}

