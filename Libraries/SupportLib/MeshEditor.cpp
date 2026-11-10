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

	mszName.Init();
	mpMesh = pMesh;
	if (pMesh.IsNotNull())
	{
		pMesh->mcPolygons.mbInUse = true;
	}

	mcSelections.Init();
	mcEdgeVisibility.Init();

	mcModifiers.Init(pMesh);


	muiStructureChanges = 0;
	muiShapeChanges = 0;
	muiOutputStructure = 0;
	muiOutputShape = 0;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::Free(void)
{
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::Class(void)
{
	U_String(mszName);
	M_Pointer(mpMesh);
	M_Embedded(mcModifiers);
	M_Embedded(mcSelections);
	M_Embedded(mcEdgeVisibility);
	U_Size(muiStructureChanges);
	U_Size(muiShapeChanges);
	U_Size(muiOutputStructure);
	U_Size(muiOutputShape);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshEditor::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(mcSelections.Save(pcFile));
	ReturnOnFalse(mcEdgeVisibility.Save(pcFile));
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
	return true;
}


//////////////////////////////////////////////////////////////////////////
// The output is made again here through AddFace, so that the editor's edge
// visibility, selections and polygons are of its faces.
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::Collapse(void)
{
	Ptr<CMesh>	pOutput;
	int*		piCorners;
	bool		abEdges[NUM_FACE_EDGES];
	int			i;
	int			iNumCorners;
	int			iNumFaces;

	pOutput = GetOutput();
	if (&pOutput != &mpMesh)
	{
		ClearMesh();
		mpMesh->mcNormals.mbInUse |= pOutput->mcNormals.InUse();
		mpMesh->mcColours.mbInUse = pOutput->mcColours.InUse();
		mpMesh->mcSmoothing.mbInUse = false;

		iNumCorners = pOutput->NumCorners();
		for (i = 0; i < iNumCorners; i++)
		{
			AddCorner(pOutput->GetPosition(i));
		}

		iNumFaces = pOutput->NumFaces();
		for (i = 0; i < iNumFaces; i++)
		{
			piCorners = pOutput->GetFaceCorners(i);
			pOutput->mcPolygons.GetFaceEdges(pOutput->GetConnectivity(), i, abEdges);
			AddFace(piCorners[0], piCorners[1], piCorners[2], pOutput->GetFaceName(i), abEdges[0], abEdges[1], abEdges[2]);
		}

		mpMesh->CopyDetailsFrom(&pOutput);
		mpMesh->mcSmoothing.Copy(&pOutput->mcSmoothing);
	}
	RemoveModifiers();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshEditor::GetOutput(void)
{
	if ((muiOutputStructure != muiStructureChanges) || (muiOutputShape != muiShapeChanges))
	{
		mcModifiers.ClearOutput();
		muiOutputStructure = muiStructureChanges;
		muiOutputShape = muiShapeChanges;
	}
	return mcModifiers.GetOutput();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::ClearOutput(void)
{
	mcModifiers.ClearOutput();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMeshEditor::NumStructureChanges(void)
{
	return muiStructureChanges;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMeshEditor::NumShapeChanges(void)
{
	return muiShapeChanges;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMeshEditor::NumModifierChanges(void)
{
	return mcModifiers.NumChanges();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::ChangedStructure(void)
{
	muiStructureChanges++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::ChangedShape(void)
{
	muiShapeChanges++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::RemoveModifiers(void)
{
	mcModifiers.ReInit();
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
	return (int)mpMesh->mcPolygons.mcPolygons.NumElements();
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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::ClearMesh(void)
{
	ChangedStructure();
	mpMesh->ClearConnectivity();
	
	mcSelections.Clear();
	mcEdgeVisibility.Clear();

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::AddCorner(float32 x, float32 y, float32 z)
{
	SFloat32Vec3		s;

	s.Init(x, y, z);

	ChangedStructure();
	mpMesh->AddCorner(&s);
	mcSelections.AddCorner();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::AddCorner(SFloat32Vec3* psPosition)
{
	ChangedStructure();
	mpMesh->AddCorner(psPosition);
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
	ChangedStructure();
	pcConn = mpMesh->GetConnectivity();
	pcConn->GenerateConnectivity();

	rFace = mpMesh->AddFace(iCorner1, iCorner2, iCorner3);
	iNewEdges = pcConn->GenerateConnectivity();
	rFace.Pack(rFace.miFirstFace, 0, 1, 2, iNewEdges);
	mpMesh->SetFaceName(rFace.miFirstFace, iName);
	pcFace = mpMesh->GetFace(rFace.miFirstFace);

	mcSelections.AddFace(rFace.NewEdges());
	mcEdgeVisibility.AddFace(pcFace, bEdge12, bEdge23, bEdge31);
	mpMesh->mcPolygons.AddFace(pcFace, rFace.miFirstFace, iName, mpMesh->GetConnectivity(), bEdge12, bEdge23, bEdge31);


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
	ChangedStructure();
	mpMesh->RemoveFace(iFace);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::RemoveFaces(CArrayInt* paiFaces)
{
	ChangedStructure();
	mpMesh->RemoveFaces(paiFaces);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::RemoveCorner(int iCorner)
{
	ChangedStructure();
	mpMesh->RemoveCorner(iCorner);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::MovePosition(int iPosition, SFloat32Vec3* psPosition)
{
	ChangedShape();
	mpMesh->MovePosition(iPosition, psPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::MovePositions(CArrayInt* paiPositions, SFloat32Vec3* psPosition)
{
	ChangedShape();
	mpMesh->MovePositions(paiPositions, psPosition);
}

