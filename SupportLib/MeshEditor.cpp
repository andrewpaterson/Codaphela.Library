/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "StandardLib/ClassDefines.h"
#include "Mesh.h"
#include "MeshEditor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::Init(Ptr<CMesh> pcMesh)
{
	PreInit();

	mpcMesh = pcMesh;

	mcSelections.Init();
	mcEdgeVisibility.Init();
	mcPolygons.Init();

	mcModifiers.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::Free(void)
{
	mcModifiers.Kill();

	mcSelections.Kill();
	mcEdgeVisibility.Kill();
	mcPolygons.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::Class(void)
{
	M_Pointer(mpcMesh);
	M_Embedded(mcModifiers);
	M_Embedded(mcSelections);
	M_Embedded(mcEdgeVisibility);
	M_Embedded(mcPolygons);
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

	mpcMesh->Touch();
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
	mcModifiers.Kill();
	mcModifiers.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshEditor::NumCorners(void)
{
	return mpcMesh->NumCorners();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshEditor::NumFaces(void)
{
	return mpcMesh->NumFaces();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshEditor::NumPolygons(void)
{
	return mcPolygons.mcPolygons.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshEditor::NumEdges(void)
{
	return mpcMesh->NumEdges();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshEditor::NumVisibleEdges(char cEdge)
{
	return mcEdgeVisibility.NumVisibleEdges(cEdge);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::ClearConnectivity(void)
{
	mpcMesh->ClearConnectivity();
	
	mcSelections.Clear();
	mcEdgeVisibility.Clear();
	mcPolygons.Clear();

	mcModifiers.ClearConnectivity();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::AddCorner(float x, float y, float z)
{
	SFloat3		s;

	s.Init(x, y, z);

	mpcMesh->AddCorner(&s);
	mcModifiers.AddPosition(&s);
	mcSelections.AddCorner();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::AddCorner(SFloat3* psPosition)
{
	mpcMesh->AddCorner(psPosition);
	mcModifiers.AddPosition(psPosition);
	mcSelections.AddCorner();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMFRet CMeshEditor::AddFace(int iCorner1, int iCorner2, int iCorner3, int iName, bool bEdge12, bool bEdge23, bool bEdge31)
{
	CMFRet		rFace;
	CMeshFace*	pcFace;

	rFace = mpcMesh->AddFace(iCorner1, iCorner2, iCorner3);
	pcFace = mpcMesh->GetFace(rFace.miFirstFace);
	
	mcSelections.AddFace(rFace.NewEdges());
	mcEdgeVisibility.AddFace(pcFace, rFace.NewEdges(), bEdge12, bEdge23, bEdge31);
	mcPolygons.AddFace(pcFace, rFace.miFirstFace, iName, mpcMesh->GetConnectivity(), bEdge12, bEdge23, bEdge31);

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
		rFace1.Repack(1, 3, 2);
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
	mpcMesh->RemoveFace(iFace);
	mcModifiers.RemoveFace(iFace);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::RemoveFaces(CArrayInt* paiFaces)
{
	mpcMesh->RemoveFaces(paiFaces);
	mcModifiers.RemoveFaces(paiFaces);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::RemoveCorner(int iCorner)
{
	mpcMesh->RemoveCorner(iCorner);
	mcModifiers.RemoveCorner(iCorner);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::MovePosition(int iPosition, SFloat3* psPosition)
{
	mpcMesh->MovePosition(iPosition, psPosition);
	mcModifiers.MovePosition(iPosition, psPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditor::MovePositions(CArrayInt* paiPositions, SFloat3* psPosition)
{
	mpcMesh->MovePositions(paiPositions, psPosition);
	mcModifiers.MovePositions(paiPositions, psPosition);
}

