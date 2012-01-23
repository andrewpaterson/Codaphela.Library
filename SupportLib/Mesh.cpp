/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "BaseLib/Numbers.h"
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/PointerFunctions.h"
#include "Mesh.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMesh::Init(void)
{
	CStandardTrackerObject::Init();
	PrivateInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMesh::Init(int iUniqueID, char* szName)
{
	CStandardTrackerObject::Init(iUniqueID, szName);
	PrivateInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMesh::PrivateInit(void)
{
	mcPositions.Init();
	mcConnectivity.Init();
	mcNormals.Init();
	mcUVs.Init();
	mcSkin.Init();
	mcMaterials.Init();
	mcColours.Init();
	mcFaceTypes.Init();
	mcCache.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMesh::Kill(void)
{
	mcNormals.Kill();
	mcUVs.Kill();
	mcSkin.Kill();
	mcMaterials.Kill();
	mcColours.Kill();
	mcConnectivity.Kill();
	mcPositions.Kill();
	mcFaceTypes.Kill();
	mcCache.Kill();
	CStandardTrackerObject::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMesh::Load(CFileReader* pcFile)
{
	//Fixes: ambiguous access of 'Load'
	return CStandardTrackerObject::Load(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMesh::LoadSpecific(CFileReader* pcFile, int iChunkNum)
{
	ReturnOnFalse(BeginLoadStandardTrackerObject(pcFile, iChunkNum));

	ReturnOnFalse(mcPositions.Load(pcFile));
	ReturnOnFalse(mcConnectivity.Load(pcFile));
	ReturnOnFalse(mcNormals.Load(pcFile));
	ReturnOnFalse(mcUVs.Load(pcFile));
	ReturnOnFalse(mcSkin.Load(pcFile));
	ReturnOnFalse(mcMaterials.Load(pcFile));
	ReturnOnFalse(mcColours.Load(pcFile));
	ReturnOnFalse(mcFaceTypes.Load(pcFile));
	ReturnOnFalse(mcCache.Load(pcFile));

	return EndLoadStandardTrackerObject(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMesh::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(BeginSaveStandardTrackerObject(pcFile));
	
	ReturnOnFalse(mcPositions.Save(pcFile));
	ReturnOnFalse(mcConnectivity.Save(pcFile));
	ReturnOnFalse(mcNormals.Save(pcFile));
	ReturnOnFalse(mcUVs.Save(pcFile));
	ReturnOnFalse(mcSkin.Save(pcFile));
	ReturnOnFalse(mcMaterials.Save(pcFile));
	ReturnOnFalse(mcColours.Save(pcFile));
	ReturnOnFalse(mcFaceTypes.Save(pcFile));
	ReturnOnFalse(mcCache.Save(pcFile));

	return EndSaveStandardTrackerObject(pcFile);	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMesh::Copy(CMesh* pcMesh)
{
	//Er, yeah... so...

	//should probably write this.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMesh::Touch(void)
{
	mcSkin.Touch();

	mcFaceTypes.Touch(this);
	return mcCache.Touch(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMesh::NumCorners(void)
{
	return mcConnectivity.mcCorners.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMesh::NumFaces(void)
{
	return mcConnectivity.mcFaces.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMesh::NumEdges(void)
{
	return mcConnectivity.mcEdges.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMesh::TestNumPositions(void)
{
	return mcPositions.mcPositions.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMesh::FaceChunkSize(void)
{
	return mcConnectivity.mcFaces.ChunkSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMesh::CornerChunkSize(void)
{
	return mcConnectivity.mcCorners.ChunkSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMesh::TestPositionChunkSize(void)
{
	return mcPositions.mcPositions.ChunkSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat3* CMesh::GetPosition(int iIndex)
{
	return mcPositions.mcPositions.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMeshCorner* CMesh::GetCorner(int iIndex)
{
	return mcConnectivity.mcCorners.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMeshEdge* CMesh::GetEdge(int iIndex)
{
	return mcConnectivity.mcEdges.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMeshFace* CMesh::GetFace(int iIndex)
{
	return mcConnectivity.mcFaces.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMesh::ReInitConnectivity(int iCornerChunkSize, int iFaceChunkSize)
{
	mcConnectivity.ReInit(iCornerChunkSize, iFaceChunkSize);
	mcPositions.ReInit(iCornerChunkSize);

	mcNormals.ReInit(iCornerChunkSize, iFaceChunkSize);
	mcUVs.ReInit(iCornerChunkSize, iFaceChunkSize);
	mcSkin.ReInit(iCornerChunkSize);
	mcMaterials.ReInit(iFaceChunkSize);
	mcColours.ReInit(iCornerChunkSize, iFaceChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMesh::AddCorner(float x, float y, float z)
{
	mcPositions.AddPosition(x, y, z);
	mcConnectivity.AddCorner();
	mcSkin.AddCorner();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMesh::AddCorner(SFloat3* psPosition)
{
	mcPositions.AddPosition(psPosition);
	mcConnectivity.AddCorner();
	mcSkin.AddCorner();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMFRet CMesh::AddFace(int iCorner1, int iCorner2, int iCorner3)
{
	CMFRet	r;

	r = mcConnectivity.AddFace(iCorner1, iCorner2, iCorner3);
	mcNormals.AddFace(r.miFirstFace, &mcPositions, &mcConnectivity);
	mcUVs.AddFace();
	mcMaterials.AddFace();
	mcColours.AddFace();
	return r;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMFRet CMesh::AddQuad(int iCorner1, int iCorner2, int iCorner3, int iCorner4)
{
	CMFRet	r1;
	CMFRet	r2;

	r1 = AddFace(iCorner1, iCorner2, iCorner4);
	r1.Repack(0, 1, 3);
	r2 = AddFace(iCorner2, iCorner3, iCorner4);
	r2.Repack(1, 2, 3);

	r1.Pack2(r2);
	return r1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMFRet CMesh::AddPolygon(CArrayInt* paiCorners)
{
	CMFRet	r;

	if (paiCorners->NumElements() < 3)
	{
		r.PackEmpty();
		return r;
	}
	else if (paiCorners->NumElements() == 3)
	{
		return AddFace(paiCorners->GetValue(0), paiCorners->GetValue(1), paiCorners->GetValue(2));
	}
	else if (paiCorners->NumElements() == 4)
	{
		return AddQuad(paiCorners->GetValue(0), paiCorners->GetValue(1), paiCorners->GetValue(2), paiCorners->GetValue(3));
	}
	else
	{
		//Remember to return the new edge count also.
		r.PackOverflow();
		return r;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMesh::RemoveFace(int iFace)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMesh::RemoveFaces(CArrayInt* paiFaces)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMesh::RemoveCorner(int iCorner)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMesh::MovePosition(int iPosition, SFloat3* psPosition)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMesh::MovePositions(CArrayInt* paiPositions, SFloat3* psPosition)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMesh::AddUV(float u, float v, int iLayer)
{
	CMeshUVLayer*	pcUVLayer;
	SFloat2*		psUV;

	pcUVLayer = mcUVs.EnsureLayerExists(iLayer, &mcConnectivity);
	psUV = pcUVLayer->mcUVs.Add();
	psUV->Init(u, v);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMesh::AddNormal(float x, float y, float z)
{
	SFloat3*	psNormal;

	psNormal = mcNormals.mcNormals.Add();
	psNormal->Init(x, y, z);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMeshConnectivity* CMesh::GetConnectivity(void) { return &mcConnectivity; }