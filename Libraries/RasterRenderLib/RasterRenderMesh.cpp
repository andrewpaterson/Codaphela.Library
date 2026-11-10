/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RasterRenderLib

Codaphela RasterRenderLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RasterRenderLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RasterRenderLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "RasterMeshBuild.h"
#include "RasterRenderMesh.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterRenderMesh::Init(Ptr<CRasterRenderer> pRenderer, CMesh* pcMesh)
{
	PreInit();

	CRenderMesh::Init(pRenderer);
	mpRasterMesh = OMalloc<CRasterMesh>();
	BuildRasterMesh(&mpRasterMesh, pcMesh, pRenderer->GetScale());

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterRenderMesh::Class(void)
{
	CRenderMesh::Class();
	M_Pointer(mpRasterMesh);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterRenderMesh::Free(void)
{
	CRenderMesh::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CRasterMesh> CRasterRenderMesh::GetRasterMesh(void)
{
	return mpRasterMesh;
}

