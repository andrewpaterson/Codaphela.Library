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
#include "BaseLib/Numbers.h"
#include "Mesh.h"
#include "MeshEditor.h"
#include "MeshSmoothGroups.h"
#include "BaseLib/ErrorHandler.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::Init(uint32 uiAddGroup)
{
	muiAddGroup = uiAddGroup;
	meGenerationStyle = SGS_None;
	mfSharpAngle = 22.5f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::Kill(void)
{
	CMeshModifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::Apply(CMeshEditor* pcMeshEditor)
{
	GenerateGroups(&pcMeshEditor->mpMesh, false);
	GenerateNormals(&pcMeshEditor->mpMesh);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshSmoothGroups::Modify(Ptr<CMesh> pMesh)
{
	Ptr<CMesh>	pOutput;

	if (!CanRebuild(&pMesh))
	{
		return pMesh;
	}

	pOutput = CopyWithNormals(&pMesh);
	GenerateGroups(&pOutput, true);
	GenerateNormals(&pOutput);
	return pOutput;
}


//////////////////////////////////////////////////////////////////////////
// bKeep keeps groups the mesh already has, with SGS_None.
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::GenerateGroups(CMesh* pcMesh, bool bKeep)
{
	switch (meGenerationStyle)
	{
	case SGS_None:
		if (!bKeep || !pcMesh->mcSmoothing.InUse())
		{
			pcMesh->mcSmoothing.SetAll((size)pcMesh->NumFaces(), muiAddGroup);
		}
		break;
	case SGS_Name:
		GenerateGroupsFromNames(pcMesh);
		break;
	case SGS_Angle:
		GenerateGroupsFromAngles(pcMesh);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::GenerateGroupsFromNames(CMesh* pcMesh)
{
	CArrayInt	aiUniqueNames;
	int			i;
	int			iNumFaces;
	int			iName;

	iNumFaces = pcMesh->NumFaces();

	aiUniqueNames.Init();
	for (i = 0; i < iNumFaces; i++)
	{
		aiUniqueNames.AddIfUnique(pcMesh->GetFaceName(i));
	}

	if (aiUniqueNames.NumElements() >= 32)
	{
		//Too many face names to deal with.
		pcMesh->mcSmoothing.SetAll((size)iNumFaces, 1);
	}
	else
	{
		pcMesh->mcSmoothing.SetAll((size)iNumFaces, 0);
		for (i = 0; i < iNumFaces; i++)
		{
			iName = pcMesh->GetFaceName(i);
			pcMesh->mcSmoothing.mauiGroups.SetValue(i, (iName != 0) ? ((uint32)1 << (iName - 1)) : 0);
		}
	}

	aiUniqueNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::GenerateGroupsFromAngles(CMesh* pcMesh)
{
	size			i;
	size			iNumPolygons;
	CArrayFloat32Vec3	asNormals;
	CArrayInt		aiAdjPolys;
	SFloat32Vec3*		psThisNormal;
	SFloat32Vec3*		psOtherNormal;
	size			j;
	int				iOtherGon;
	float32			fDot;
	float32			fResult;
	size				uiNumElements;

	fDot = Deg2Dot(mfSharpAngle);

	asNormals.Init();
	pcMesh->mcPolygons.GetNormals(&asNormals, &pcMesh->mcNormals);

	pcMesh->mcSmoothing.SetAll((size)pcMesh->NumFaces(), 0);

	iNumPolygons = pcMesh->mcPolygons.mcPolygons.NumElements();
	for (i = 0; i < iNumPolygons; i++)
	{
		psThisNormal = asNormals.Get(i);
		aiAdjPolys.Init();
		pcMesh->mcPolygons.GetAdjacentPolygons(pcMesh->GetConnectivity(), (int)i, &aiAdjPolys);

		uiNumElements = aiAdjPolys.NumElements();
		for (j = 0; j < uiNumElements; j++)
		{
			iOtherGon = aiAdjPolys.GetValue(j);
			psOtherNormal = asNormals.Get(iOtherGon);

			fResult = Float32Vec3Dot(psThisNormal, psOtherNormal);
			if (fResult >= fDot)
			{
				//Smooth!
			}
		}

		aiAdjPolys.Kill();
	}

	asNormals.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::GenerateNormals(CMesh* pcMesh)
{
	//! Remeber to check out the maxsdk doc: Computing Face and Vertex Normals, for Weighting by Face Angle.

	size				i, j, k, l;
	int*				aiFaceRef;
	SFloat32Vec3				cNormal;  
	uint32				dwSmooth[MAX_UNOVERLAPPING_SMOOTHING_GROUPS];
	size				iNumNormals;
	CMeshCorner*		psCorners;
	CMeshFace*			psFaces;
	int					iPosition;
	bool				bSmoothingOverlap;
	uint32*				puiSmoothing;
	uint32				uiSmoothing;
	int					iNewNormalIndex;
	size				iNumCorners;
	SMeshNormalFace*	psFaceNormal;
	SFloat32Vec3*			pcNormal;
	CMeshNormals*		pcNormals;
	size				uiNumFaces;

	if (!pcMesh->mcNormals.InUse() || !pcMesh->mcSmoothing.InUse() || (pcMesh->NumFaces() == 0))
	{
		return;
	}

	//This is bad.  Don't access the corners/faces arrays like this.
	psCorners = pcMesh->GetCorner(0);
	psFaces = pcMesh->GetFace(0);

	puiSmoothing = pcMesh->mcSmoothing.mauiGroups.Get(0);

	pcNormals = &pcMesh->mcNormals;
	pcNormals->RemoveCornerNormals(pcMesh->NumFaces());

	iNumCorners = pcMesh->NumCorners();
	//Step through every vertex
	for (i = 0; i < iNumCorners; i++)
	{
		iNumNormals = 0;

		//Get the array of faces attached to the vert
		aiFaceRef = (int*)psCorners[i].maiFace.GetData();

		//Zero the smoothing group
		dwSmooth[0] = 0;

		//First work out all the overlapping smoothing groups as this vertex
		//Step through every face attached to that vertex
		uiNumFaces = psCorners[i].maiFace.NumElements();
		for (j = 0; j < uiNumFaces; j++)
		{
			uiSmoothing = puiSmoothing[aiFaceRef[j]];
			//Ignore faces with zero smoothing groups
			if (uiSmoothing != 0)
			{
				bSmoothingOverlap = false;

				for (k = 0; k < iNumNormals; k++)
				{
					//If the smoothing groups overlap then add this face normal to the temp normal
					if (dwSmooth[k] & uiSmoothing)
					{
						dwSmooth[k] |= uiSmoothing;
						bSmoothingOverlap = true;
						break;
					}
				}
				if (!bSmoothingOverlap)
				{
					//Just in case
					if (iNumNormals < MAX_UNOVERLAPPING_SMOOTHING_GROUPS)
					{
						dwSmooth[iNumNormals] = uiSmoothing;
						iNumNormals++;
					}
				}
			}
		}

		//Still working out overlapping smoothing groups
		//Remove redundant groups
		for (j = 0; j < iNumNormals; j++)
		{
			for (k = 0; k < iNumNormals; k++)
			{
				//Check if smoothing groups overlap and its not comparing with itself
				if ((dwSmooth[j] & dwSmooth[k]) && (j != k))
				{	
					dwSmooth[j] |= dwSmooth[k];

					//Delete the group which overlaps with another
					iNumNormals--;

					//Move the counter back one so it processes the new normal
					if (k > 0)
					{
						k--;
					}
					dwSmooth[k] = dwSmooth[iNumNormals];
				}
			}
		}  

		//Now have number of unique normals at this vertex
		for (k = 0; k < iNumNormals; k++)
		{
			cNormal.Init(0, 0, 0);

			//Step over every face at this vertex
			uiNumFaces = psCorners[i].maiFace.NumElements();
			for (j = 0; j < uiNumFaces; j++)
			{
				uiSmoothing = puiSmoothing[aiFaceRef[j]];

				//If smoothing group on this normal overlaps smoothing group on face
				if (uiSmoothing & dwSmooth[k])
				{
					psFaceNormal = pcNormals->mcFaces.Get(aiFaceRef[j]);

					pcNormal = pcNormals->mcNormals.Get(psFaceNormal->iFaceNormal);
					cNormal.Add(pcNormal);
				}
			}

			//Bit redundant at the moment, not sure if I will ever need more information about the normal
			cNormal.Normalize();

			//Add the normal to the normal array the position in the array is used elements-1
			pcNormals->mcNormals.Add(&cNormal);

			//Get the normal which has just been added.
			iNewNormalIndex = (int)pcNormals->mcNormals.NumElements()-1;

			//Step over every face at this vertex
			uiNumFaces = psCorners[i].maiFace.NumElements();
			for (j = 0; j < uiNumFaces; j++)
			{
				uiSmoothing = puiSmoothing[aiFaceRef[j]];

				//If the smoothing group on this normal overlaps the smoothing group on face
				if (uiSmoothing & dwSmooth[k])
				{
					//Then set the face point normal reference to the newly added normal
					for (l = 0; l < 3; l++)
					{
						iPosition = psFaces[aiFaceRef[j]].maiCorner[l];
						if ((int)i == iPosition)
						{
							psFaceNormal = pcNormals->mcFaces.Get(aiFaceRef[j]);
							psFaceNormal->aiCornerNormals[l] = iNewNormalIndex;
						}
					}
				}
			}
		}

		//Handle faces with zero smoothing groups as special cases
		//Step over every face at this vertex
		uiNumFaces = psCorners[i].maiFace.NumElements();
		for (j = 0; j < uiNumFaces; j++)
		{
			uiSmoothing = puiSmoothing[aiFaceRef[j]];

			if (uiSmoothing == 0)
			{
				psFaceNormal = pcNormals->mcFaces.Get(aiFaceRef[j]);
				if (psFaceNormal->iFaceNormal == -1)
				{
					gcUserError.Set("Couldn't find normal for face.");
				}

				psFaceNormal->aiCornerNormals[0] = psFaceNormal->iFaceNormal;
				psFaceNormal->aiCornerNormals[1] = psFaceNormal->iFaceNormal;
				psFaceNormal->aiCornerNormals[2] = psFaceNormal->iFaceNormal;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// What faces are put in when they're given groups.
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::SetAllGroups(uint32 uiGroups)
{
	muiAddGroup = uiGroups;
}

