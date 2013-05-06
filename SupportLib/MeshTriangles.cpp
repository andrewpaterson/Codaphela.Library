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
#include "MeshDefines.h"
#include "MeshConnectivity.h"
#include "MeshNormals.h"
#include "MeshTriangles.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshTriangles::Init(void)
{
	CMeshDetail::Init();
	mcTriangles.Init(MESH_FACES_CHUNK_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshTriangles::Kill(void)
{
	mcTriangles.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshTriangles::Load(CFileReader* pcFile)
{
	ReturnOnFalse(LoadMeshDetail(pcFile));
	ReturnOnFalse(pcFile->ReadArrayTemplate(&mcTriangles));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshTriangles::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(SaveMeshDetail(pcFile));
	return pcFile->WriteArrayTemplate(&mcTriangles);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshTriangles::GenerateTriangles(CMeshConnectivity* pcConn, CMeshNormals* pcNormals)
{
	CTriangleIndexed*	pcTriangle;
	int					iNumFaces;
	CMeshFace*			pcFace;
	int					iNormal;
	SMeshNormalFace*	psFaceNormal;
	int					i;

	iNumFaces = pcConn->mcFaces.NumElements();

	mcTriangles.Kill();
	mcTriangles.Init(pcConn->mcFaces.ChunkSize());
	mcTriangles.SetUsedElements(iNumFaces);

	for (i = 0; i < iNumFaces; i++)
	{
		pcFace = pcConn->mcFaces.Get(i);
		psFaceNormal = pcNormals->mcFaces.Get(i);
		pcTriangle = mcTriangles.Get(i);
		iNormal = psFaceNormal->iFaceNormal;
		pcTriangle->Init(pcFace->asCorner[0], pcFace->asCorner[1], pcFace->asCorner[2], iNormal);
	}
}

