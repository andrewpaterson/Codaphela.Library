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
#include "StandardLib/ClassDefines.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "MeshCacher.h"
#include "MeshCache.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshCache::Init(void)
{
	PreInit();

	CMeshDetail::Init();
	mcMeshVerticesArrays.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshCache::Free(void)
{
	mcMeshVerticesArrays.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshCache::Class(void)
{
	CMeshDetail::Class();

	M_Embedded(mcMeshVerticesArrays);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshCache::Load(CObjectReader* pcFile)
{
	ReturnOnFalse(LoadMeshDetail(pcFile));
	return mcMeshVerticesArrays.Load(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshCache::Save(CObjectWriter* pcFile)
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
Ptr<CMeshVertexArray> CMeshCache::AddVertexArray(SMeshFaceType sMeshFaceType)
{
	Ptr<CMeshVertexArray>	pcVertexArray;

	pcVertexArray = OMalloc<CMeshVertexArray>(sMeshFaceType);
	mcMeshVerticesArrays.Add(pcVertexArray);
	return pcVertexArray;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMeshVertexArray> CMeshCache::GetVertexArray(SMeshFaceType sMeshFaceType)
{
	Ptr<CMeshVertexArray>	pcVertexArray;
	size					i;
	size					uiNumElements;

	uiNumElements = mcMeshVerticesArrays.NumElements();
	for (i = 0; i < uiNumElements; i++)
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
Ptr<CMeshVertexArray> CMeshCache::GetVertexArray(int iIndex)
{
	return mcMeshVerticesArrays.Get(iIndex);
}


