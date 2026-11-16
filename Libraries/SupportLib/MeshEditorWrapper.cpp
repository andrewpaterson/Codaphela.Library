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
#include "MeshEditor.h"
#include "MeshEditorWrapper.h"


//Where each corner of a quad added the other way round, 1 4 3 2, is in the quad as it was given.
static const int gaiFlippedQuadCorners[4] = { 0, 3, 2, 1 };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditorWrapper::Init(Ptr<CMesh> pMesh)
{
	mpcEditor = NULL;
	mpcMesh = &pMesh;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditorWrapper::Init(Ptr<CMeshEditor> pEditor)
{
	mpcEditor = &pEditor;
	mpcMesh = &pEditor->mpMesh;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditorWrapper::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditorWrapper::ClearConnectivity(void)
{
	if ((mpcEditor != NULL))
	{
		mpcEditor->ClearConnectivity();
	}
	else
	{
		mpcMesh->ClearConnectivity();
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

	if ((mpcEditor != NULL))
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
// A flipped quad is added to a mesh with its corners the other way round
// from its first, 1 4 3 2, which is cut where the quad would be.  What's
// returned numbers the corners as they were given.
//
//////////////////////////////////////////////////////////////////////////
CMFRet CMeshEditorWrapper::AddQuad(int iCorner1, int iCorner2, int iCorner3, int iCorner4, int iName, bool bFlipFaces)
{
	CMFRet	rQuad;
	int		aiCorners[6];

	if ((mpcEditor != NULL))
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
			rQuad.Unpack1(&aiCorners[0], &aiCorners[1], &aiCorners[2]);
			rQuad.Unpack2(&aiCorners[3], &aiCorners[4], &aiCorners[5]);
			rQuad.Repack(gaiFlippedQuadCorners[aiCorners[0]], gaiFlippedQuadCorners[aiCorners[1]], gaiFlippedQuadCorners[aiCorners[2]],
						  gaiFlippedQuadCorners[aiCorners[3]], gaiFlippedQuadCorners[aiCorners[4]], gaiFlippedQuadCorners[aiCorners[5]]);
			return rQuad;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A flipped polygon is added to a mesh with its corners the other way round
// from its first, as a flipped quad is.
//
//////////////////////////////////////////////////////////////////////////
CMFRet CMeshEditorWrapper::AddPolygon(CArrayInt* paiCorners, int iName, bool bFlipFaces)
{
	CArrayInt	aiFlipped;
	CMFRet		r;
	size		i;
	size		uiNumCorners;

	if ((mpcEditor != NULL))
	{
		return mpcEditor->AddPolygon(paiCorners, iName, bFlipFaces);
	}
	else if (!bFlipFaces || (paiCorners->NumElements() == 4))
	{
		if (bFlipFaces)
		{
			return AddQuad(paiCorners->GetValue(0), paiCorners->GetValue(1), paiCorners->GetValue(2), paiCorners->GetValue(3), iName, true);
		}
		return mpcMesh->AddPolygon(paiCorners);
	}
	else
	{
		uiNumCorners = paiCorners->NumElements();
		aiFlipped.Init();
		for (i = 0; i < uiNumCorners; i++)
		{
			aiFlipped.Add(paiCorners->GetValue((size)((uiNumCorners - i) % uiNumCorners)));
		}
		r = mpcMesh->AddPolygon(&aiFlipped);
		aiFlipped.Kill();
		return r;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditorWrapper::AddCorner(float32 x, float32 y, float32 z)
{
	if ((mpcEditor != NULL))
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
void CMeshEditorWrapper::AddUV(float32 u, float32 v)
{
	mpcMesh->AddUV(u, v);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEditorWrapper::AddNormal(float32 x, float32 y, float32 z)
{
	mpcMesh->AddNormal(x, y, z);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshEditorWrapper::NumFaces(void) const
{
	return mpcMesh->NumFaces();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshEditorWrapper::NumCorners(void) const
{
	return mpcMesh->NumCorners();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshEditorWrapper::NumNormals(void) const
{
	return (int)mpcMesh->mcNormals.mcNormals.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshEditorWrapper::NumUVs(void) const
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

