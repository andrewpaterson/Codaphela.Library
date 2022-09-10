/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "MeshEditor.h"
#include "MeshEditorWrapper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditorWrapper::Init(CMesh* pcMesh)
{
	mpcEditor = NULL;
	mpcMesh = pcMesh;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditorWrapper::Init(CMeshEditor* pcEditor)
{
	mpcEditor = pcEditor;
	mpcMesh = pcEditor->mpcMesh;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditorWrapper::Kill(void)
{
	mpcMesh = NULL;
	mpcEditor = NULL;
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditorWrapper::ReInitConnectivity(void)
{
	if (mpcEditor)
	{
		mpcEditor->ReInitConnectivity();
	}
	else
	{
		mpcMesh->ReInitConnectivity();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditorWrapper::SetUVsInUse(bool bInUse)
{
	mpcMesh->mcUVs.mbInUse = bInUse;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditorWrapper::SetNormalsInUse(bool bInUse)
{
	mpcMesh->mcNormals.mbInUse = bInUse;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMFRet CMeshEditorWrapper::AddFace(int iCorner1, int iCorner2, int iCorner3, int iName, bool bEdge12, bool bEdge23, bool bEdge31)
{
	return AddFace(iCorner1, iCorner2, iCorner3, iName, bEdge12, bEdge23, bEdge31, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMFRet CMeshEditorWrapper::AddFace(int iCorner1, int iCorner2, int iCorner3, int iName, bool bEdge12, bool bEdge23, bool bEdge31, bool bFlipFaces)
{
	CMFRet	rFace;

	if (mpcEditor)
	{
		return mpcEditor->AddFace(iCorner1, iCorner2, iCorner3, iName, bEdge12, bEdge23, bEdge31, bFlipFaces);
	}
	else
	{
		if (!bFlipFaces)
		{
			return mpcMesh->AddFace(iCorner1, iCorner2, iCorner3);
		}
		else
		{
			rFace = mpcMesh->AddFace(iCorner1, iCorner3, iCorner2);
			rFace.Repack(0, 2, 1);
			return rFace;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMFRet CMeshEditorWrapper::AddQuad(int iCorner1, int iCorner2, int iCorner3, int iCorner4, int iName, bool bFlipFaces)
{
	CMFRet	rQuad;

	if (mpcEditor)
	{
		return mpcEditor->AddQuad(iCorner1, iCorner2, iCorner3, iCorner4, iName, bFlipFaces);
	}
	else
	{
		if (!bFlipFaces)
		{
			return mpcMesh->AddQuad(iCorner1, iCorner2, iCorner3, iCorner4);
		}
		else
		{
			rQuad = mpcMesh->AddQuad(iCorner1, iCorner4, iCorner3, iCorner2);

			//I have no idea if this is correct or not.
			rQuad.Repack(0, 3, 1, 3, 2, 1);
			return rQuad;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMFRet CMeshEditorWrapper::AddPolygon(CArrayInt* paiCorners, int iName, bool bFlipFaces)
{
	if (mpcEditor)
	{
		return mpcEditor->AddPolygon(paiCorners, iName, bFlipFaces);
	}
	else
	{
		return mpcMesh->AddPolygon(paiCorners);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditorWrapper::AddCorner(float x, float y, float z)
{
	if (mpcEditor)
	{
		mpcEditor->AddCorner(x, y, z);
	}
	else
	{
		mpcMesh->AddCorner(x, y, z);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditorWrapper::AddUV(float u, float v)
{
	mpcMesh->AddUV(u, v);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditorWrapper::AddNormal(float x, float y, float z)
{
	mpcMesh->AddNormal(x, y, z);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshEditorWrapper::NumFaces(void)
{
	return mpcMesh->NumFaces();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshEditorWrapper::NumCorners(void)
{
	return mpcMesh->NumCorners();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshEditorWrapper::NumNormals(void)
{
	return mpcMesh->mcNormals.mcNormals.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshEditorWrapper::NumUVs(void)
{
	return mpcMesh->mcUVs.NumUVs();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMeshNormalFace* CMeshEditorWrapper::GetFaceNormal(int iFace)
{
	return mpcMesh->mcNormals.mcFaces.Get(iFace);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMeshUVFace* CMeshEditorWrapper::GetUVFace(int iFace, int iLayer)
{
	CMeshUVLayer*	pcUVLayer;

	pcUVLayer = mpcMesh->mcUVs.mcLayers.Get(iLayer);
	return pcUVLayer->mcFaces.Get(iFace);
}

