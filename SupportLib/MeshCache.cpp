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
#include "MeshCacher.h"
#include "MeshCache.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshCache::Init(void)
{
	CMeshDetail::Init();
	mcMeshVerticesArrays.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshCache::Kill(void)
{
	mcMeshVerticesArrays.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshCache::Load(CFileReader* pcFile)
{
	ReturnOnFalse(LoadMeshDetail(pcFile));
	return mcMeshVerticesArrays.Load(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshCache::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(SaveMeshDetail(pcFile));
	return mcMeshVerticesArrays.Save(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshCache::Touch(CMesh* pcMesh)
{
	CMeshCacher	cCacher;
	bool		bResult;

	cCacher.Init(pcMesh);

	bResult = cCacher.Index();
	if (bResult)
	{
		bResult = cCacher.Vertex();
	}

	cCacher.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMeshVertexArray* CMeshCache::AddVertexArray(SMeshFaceType sMeshFaceType)
{
	CMeshVertexArray*	pcVertexArray;

	pcVertexArray = mcMeshVerticesArrays.Add();
	pcVertexArray->Init(sMeshFaceType);
	return pcVertexArray;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMeshVertexArray* CMeshCache::GetVertexArray(SMeshFaceType sMeshFaceType)
{
	CMeshVertexArray*	pcVertexArray;
	size				i;

	for (i = 0; i < mcMeshVerticesArrays.NumElements(); i++)
	{
		pcVertexArray = mcMeshVerticesArrays.Get(i);
		if (pcVertexArray->Is(&sMeshFaceType))
		{
			return pcVertexArray;
		}
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMeshVertexArray* CMeshCache::GetVertexArray(int iIndex)
{
	return mcMeshVerticesArrays.Get(iIndex);
}


