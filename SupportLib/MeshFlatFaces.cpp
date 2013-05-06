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
#include "BaseLib/Numbers.h"
#include "Mesh.h"
#include "MeshEditor.h"
#include "MeshFlatFaces.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshFlatFaces::Init(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshFlatFaces::Kill(void)
{
	CMeshModifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshFlatFaces::Load(CFileReader* pcFile)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshFlatFaces::Save(CFileWriter* pcFile)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshFlatFaces::Apply(CMeshEditor* pcMeshEditor)
{
	GenerateNormals(pcMeshEditor->mpcMesh);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshFlatFaces::GenerateNormals(CMesh* pcMesh)
{
	int					i;
	SFloat3				cNormal;  
	CMeshCorner*		psCorners;
	CMeshFace*			psFaces;
	CMeshFace*			psFace;
	int					iNumFaces;
	SMeshNormalFace*	psFaceNormal;
	CMeshNormals*		pcNormals;

	if (!pcMesh->mcNormals.InUse())
	{
		return;
	}

	psCorners = pcMesh->GetCorner(0);
	psFaces = pcMesh->GetFace(0);

	pcNormals = &pcMesh->mcNormals;

	iNumFaces = pcMesh->NumFaces();

	for (i = 0; i < iNumFaces; i++)
	{
		psFace = &psFaces[i];
		psFaceNormal = pcNormals->mcFaces.Get(i);

		psFaceNormal->aiCornerNormals[0] = psFaceNormal->iFaceNormal;
		psFaceNormal->aiCornerNormals[1] = psFaceNormal->iFaceNormal;
		psFaceNormal->aiCornerNormals[2] = psFaceNormal->iFaceNormal;
	}
}


