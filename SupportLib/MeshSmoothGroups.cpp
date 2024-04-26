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
#include "MeshSmoothGroups.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::Init(int iAddGroup)
{
	mcSmoothingGroups.Init();
	miAddGroup = iAddGroup;
	meGenerationStyle = SGS_None;
	mfSharpAngle = 22.5f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::Kill(void)
{
	mcSmoothingGroups.Kill();
	CMeshModifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshSmoothGroups::Load(CFileReader* pcFile)
{
	ReturnOnFalse(mcSmoothingGroups.Read(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshSmoothGroups::Save(CFileWriter* pcFile)
{
	return mcSmoothingGroups.Write(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::Apply(CMeshEditor* pcMeshEditor)
{
	GenerateSmoothing(pcMeshEditor);
	GenerateNormals(pcMeshEditor->mpcMesh);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::GenerateSmoothing(CMeshEditor* pcMeshEditor)
{
	switch (meGenerationStyle)
	{
	case SGS_None:
		break;
	case SGS_Name:
		GenerateSmoothingFromNames(pcMeshEditor);
		break;
	case SGS_Angle:
		GenerateSmoothingFromAngles(pcMeshEditor);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::GenerateSmoothingFromNames(CMeshEditor* pcMeshEditor)
{
	int				i;
	int				iNumPolygons;
	CMeshPolygon*	pcPolygon;
	int				iNumFaces;
	int				j;
	int				iFace;
	CArrayInt		aiUniqueNames;
	uint32	uiFaceName;
	uint32	uiSmoothingGroup;

	iNumPolygons = pcMeshEditor->mcPolygons.mcPolygons.NumElements();

	aiUniqueNames.Init();
	pcMeshEditor->mcPolygons.GetUniqueNames(&aiUniqueNames);

	if (aiUniqueNames.NumElements() >= 32)
	{
		//Too many face names to deal with.
		mcSmoothingGroups.SetArrayValues(1);
	}
	else
	{
		for (i = 0; i < iNumPolygons; i++)
		{
			pcPolygon = pcMeshEditor->mcPolygons.Get(i);
			iNumFaces = pcPolygon->maiFaces.NumElements();
			uiFaceName = pcPolygon->miName;
			if (uiFaceName != 0)
			{
				uiSmoothingGroup = 1 << (uiFaceName-1);
			}
			else
			{
				uiSmoothingGroup = 0;
			}

			for (j = 0; j < iNumFaces; j++)
			{
				iFace = pcPolygon->maiFaces.GetValue(j);
				mcSmoothingGroups.SetValue(iFace, uiSmoothingGroup);
			}
		}
	}

	aiUniqueNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::GenerateSmoothingFromAngles(CMeshEditor* pcMeshEditor)
{
	int				i;
	int				iNumPolygons;
	CMeshPolygon*	pcPolygon;
	CArrayFloat3	asNormals;
	CArrayInt		aiAdjPolys;
	SFloat3*		psThisNormal;
	SFloat3*		psOtherNormal;
	int				j;
	int				iOtherGon;
	float			fDot;
	float			fResult;

	fDot = Deg2Dot(mfSharpAngle);

	asNormals.Init();
	pcMeshEditor->mcPolygons.GetNormals(&asNormals, &pcMeshEditor->mpcMesh->mcNormals);

	mcSmoothingGroups.SetArrayValues(0);

	iNumPolygons = pcMeshEditor->mcPolygons.mcPolygons.NumElements();
	for (i = 0; i < iNumPolygons; i++)
	{
		psThisNormal = asNormals.Get(i);
		pcPolygon = pcMeshEditor->mcPolygons.Get(i);
		aiAdjPolys.Init();
		pcMeshEditor->mcPolygons.GetAdjacentPolygons(pcMeshEditor->mpcMesh->GetConnectivity(), i, &aiAdjPolys);

		for (j = 0; j < aiAdjPolys.NumElements(); j++)
		{
			iOtherGon = aiAdjPolys.GetValue(j);
			psOtherNormal = asNormals.Get(iOtherGon);

			fResult = Float3Dot(psThisNormal, psOtherNormal);
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

	int					i, j, k, l;
	int*				aiFaceRef;
	SFloat3				cNormal;  
	uint32		dwSmooth[MAX_UNOVERLAPPING_SMOOTHING_GROUPS];
	int					iNumNormals;
	CMeshCorner*		psCorners;
	CMeshFace*			psFaces;
	CMeshFace*			psFace;
	int					iPosition;
	bool				bSmoothingOverlap;
	int*				piSmoothing;
	int					iSmoothing;
	int					iNewNormalIndex;
	int					iNumCorners;
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

	piSmoothing = mcSmoothingGroups.Get(0);

	pcNormals = &pcMesh->mcNormals;

	iNumCorners = pcMesh->NumCorners();
	//Step through every vertex
	for (i = 0; i < iNumCorners; i++)
	{
		iNumNormals = 0;

		//Get the array of faces attached to the vert
		aiFaceRef = psCorners[i].aiFace.GetData();

		//Zero the smoothing group
		dwSmooth[0] = 0;

		//First work out all the overlapping smoothing groups as this vertex
		//Step through every face attached to that vertex
		for (j = 0; j < psCorners[i].aiFace.NumElements(); j++)
		{
			iSmoothing = piSmoothing[aiFaceRef[j]];
			//Ignore faces with zero smoothing groups
			if (iSmoothing != 0)
			{
				bSmoothingOverlap = false;

				for (k = 0; k < iNumNormals; k++)
				{
					//If the smoothing groups overlap then add this face normal to the temp normal
					if (dwSmooth[k] & iSmoothing)
					{
						dwSmooth[k] |= iSmoothing;
						bSmoothingOverlap = true;
						break;
					}
				}
				if (!bSmoothingOverlap)
				{
					//Just in case
					if (iNumNormals < MAX_UNOVERLAPPING_SMOOTHING_GROUPS)
					{
						dwSmooth[iNumNormals] = iSmoothing;
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
					if (iNumNormals < 0)
					{
						iNumNormals = 0;
					}

					//Move the counter back one so it processes the new normal
					k--;
					if (k < 0)
					{
						k = 0;
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
			for (j = 0; j < (int)psCorners[i].aiFace.NumElements(); j++)
			{
				iSmoothing = piSmoothing[aiFaceRef[j]];

				//If smoothing group on this normal overlaps smoothing group on face
				if (iSmoothing & dwSmooth[k])
				{
					psFace = &psFaces[aiFaceRef[j]];
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
			iNewNormalIndex = pcNormals->mcNormals.NumElements()-1;

			//Step over every face at this vertex
			for (j = 0; j < psCorners[i].aiFace.NumElements(); j++)
			{
				iSmoothing = piSmoothing[aiFaceRef[j]];

				//If the smoothing group on this normal overlaps the smoothing group on face
				if (iSmoothing & dwSmooth[k])
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

		//Handle faces with zero smoothing groups as special cases
		//Step over every face at this vertex
		for (j = 0; j < psCorners[i].aiFace.NumElements(); j++)
		{
			iSmoothing = piSmoothing[aiFaceRef[j]];

			if (iSmoothing == 0)
			{
				psFace = &psFaces[aiFaceRef[j]];
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
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::ReInitConnectivity(void)
{
	mcSmoothingGroups.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::AddFace(int iCorner1, int iCorner2, int iCorner3)
{
	mcSmoothingGroups.Add(miAddGroup);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::RemoveFace(int iFace)
{
	mcSmoothingGroups.RemoveAt(iFace, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::RemoveFaces(CArrayInt* paiFaces)
{
	mcSmoothingGroups.RemoveAt(paiFaces->GetData(), paiFaces->NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::SetAllGroups(int iGroup)
{
	mcSmoothingGroups.SetArrayValues(iGroup);
	miAddGroup = iGroup;
}

