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
#include "BaseLib/Numbers.h"
#include "Mesh.h"
#include "MeshEditor.h"
#include "MeshSmoothFaces.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothFaces::Init(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothFaces::Kill(void)
{
	CMeshModifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshSmoothFaces::Load(CFileReader* pcFile)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshSmoothFaces::Save(CFileWriter* pcFile)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothFaces::Apply(CMeshEditor* pcMeshEditor)
{
	GenerateNormals(&pcMeshEditor->mpcMesh);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothFaces::GenerateNormals(CMesh* pcMesh)
{
	size				i, j, l;
	int*				aiFaceRef;
	SFloat3				cNormal;  
	CMeshCorner*		psCorners;
	CMeshFace*			psFaces;
	CMeshFace*			psFace;
	int					iPosition;
	int					iNewNormalIndex;
	size				iNumCorners;
	SMeshNormalFace*	psFaceNormal;
	SFloat3*			pcNormal;
	CMeshNormals*		pcNormals;

	if (!pcMesh->mcNormals.InUse())
	{
		return;
	}

	//This is bad.  Don't access the corners/faces arrays like this.
	psCorners = pcMesh->GetCorner(0);
	psFaces = pcMesh->GetFace(0);

	pcNormals = &pcMesh->mcNormals;

	iNumCorners = pcMesh->NumCorners();
	//Step through every vertex
	for (i = 0; i < iNumCorners; i++)
	{
		//Get the array of faces attached to the vert
		aiFaceRef = (int*)psCorners[i].aiFace.GetData();

		cNormal.Init(0, 0, 0);

		//Step over every face at this vertex
		for (j = 0; j < psCorners[i].aiFace.NumElements(); j++)
		{
			psFace = &psFaces[aiFaceRef[j]];
			psFaceNormal = pcNormals->mcFaces.Get(aiFaceRef[j]);

			pcNormal = pcNormals->mcNormals.Get(psFaceNormal->iFaceNormal);
			cNormal.Add(pcNormal);
		}

		//Bit redundant at the moment, not sure if I will ever need more information about the normal
		cNormal.Normalize();

		//Add the normal to the normal array the position in the array is used elements-1
		pcNormals->mcNormals.Add(&cNormal);

		//Get the normal which has just been added.
		iNewNormalIndex = pcNormals->mcNormals.NumElements()-1;

		//Step over every face at this vertex
		for (j = 0; j < psCorners[i].aiFace.NumElements(); j++)
		{
			//Then set the face point normal reference to the newly added normal
			for (l = 0; l < 3; l++)
			{
				iPosition = psFaces[aiFaceRef[j]].asCorner[l];
				if (i == iPosition)
				{
					psFaceNormal = pcNormals->mcFaces.Get(aiFaceRef[j]);
					psFaceNormal->aiCornerNormals[l] = iNewNormalIndex;
				}
			}
		}
	}
}

