/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WorldLib

Codaphela WorldLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WorldLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WorldLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "BaseLib/SystemAllocator.h"
#include "MeshObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshObjectNode::Init(void)
{
	mcVerticies.Init(1);
	mcNormals.Init(1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshObjectNode::Init(int iNumVerticies, int iNumNormals)
{
	mcVerticies.Init(&gcSystemAllocator, iNumVerticies);
	mcVerticies.Resize(iNumVerticies);

	mcNormals.Init(&gcSystemAllocator, iNumNormals);
	mcNormals.Resize(iNumNormals);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshObjectNode::Kill(void)
{
	mcNormals.Kill();
	mcVerticies.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshObjectNode::SetVertex(int iVertex, SFloat3* psVec)
{
	mcVerticies.Set(iVertex, psVec);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshObjectNode::SetNormal(int iNormal, SFloat3* psVec)
{
	mcNormals.Set(iNormal, psVec);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayVector* CMeshObjectNode::GetVerticies(void)
{
	return &mcVerticies;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayVector* CMeshObjectNode::GetNormals(void)
{
	return &mcNormals;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshObject::Init(void)
{
	mcNodes.Init();
	mcVerticies.Init(1);
	mcNormals.Init(1);
	mcSkinnedVertexPtrs.Init(1);
	mcSkinnedNormalPtrs.Init(1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshObject::Init(int iNumVerticies, int iNumNormals)
{
	mcNodes.Init();
	mcSkinnedVertexPtrs.Init(1);
	mcSkinnedNormalPtrs.Init(1);

	mcVerticies.Init(iNumVerticies);
	mcVerticies.Resize(iNumVerticies);

	mcNormals.Init(iNumNormals);
	mcNormals.Resize(iNumNormals);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshObject::Kill(void)
{
	mcNormals.Kill();
	mcVerticies.Kill();
	mcNodes.Kill();
	mcSkinnedVertexPtrs.Kill();
	mcSkinnedNormalPtrs.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshObject::SetVertex(int iVertex, SFloat3* psVec)
{
	mcVerticies.Set(iVertex, psVec);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshObject::SetNormal(int iNormal, SFloat3* psVec)
{
	mcNormals.Set(iNormal, psVec);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshObject::IsSkinned(void)
{
	return (mcNodes.NumElements() != 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayVector* CMeshObject::GetVerticies(void)
{
	return &mcVerticies;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayVector* CMeshObject::GetNormals(void)
{
	return &mcNormals;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArraySkinnedLinkObjectNodes* CMeshObject::GetSkinnedVertexPtrs(void)
{
	return &mcSkinnedVertexPtrs;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArraySkinnedLinkObjectNodes* CMeshObject::GetSkinnedNormalPtrs(void)
{
	return &mcSkinnedNormalPtrs;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLinkListLinkObjectNode* CMeshObject::GetNodes(void)
{
	return &mcNodes;
}


