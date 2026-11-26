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
	PreInit();

	muiAddGroup = uiAddGroup;
	meGenerationStyle = SGS_None;
	mfSharpAngle = 22.5f;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::Class(void)
{
	CMeshModifier::Class();
	U_UInt32(muiAddGroup);
	U_Enum(meGenerationStyle);
	U_Float32(mfSharpAngle);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshSmoothGroups::Modify(Ptr<CMesh> pMesh)
{
	Ptr<CMesh>	pOutput;

	pOutput = CopyWithNormals(&pMesh);
	GenerateGroups(&pOutput);
	GenerateNormals(&pOutput);
	return pOutput;
}


//////////////////////////////////////////////////////////////////////////
// SGS_None keeps groups the mesh already has.
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::GenerateGroups(CMesh* pcMesh)
{
	switch (meGenerationStyle)
	{
	case SGS_None:
		if (!pcMesh->mcSmoothing.InUse())
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
// Faces joined across edges whose normals are within mfSharpAngle of each
// other are a region, and each region is given one smoothing group, a bit
// that no region touching it at a corner has.  So a region is smooth and
// meets its neighbours at sharp edges.
//
// A face in a polygon is compared by its polygon's normal, and the faces of
// a polygon are always in one region.  A face in none, or a mesh with no
// polygons, is compared by its own normal.
//
// Joining is from face to face, so a region can hold two faces that are
// further apart than the angle (round a cone's tip), and they're smoothed
// where they meet, as 3ds Max's Auto Smooth does.  A region whose
// neighbours already have all 32 bits takes the bit it touches least, and is
// smoothed with the neighbours that have it where they meet.
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::GenerateGroupsFromAngles(CMesh* pcMesh)
{
	int						iNumFaces;
	float32					fDot;
	CArrayLargeFloat32Vec3	asPolygonNormals;
	CArrayInt				aiRegions;
	CArrayInt				aiOrder;
	CArrayInt				aiRegionStarts;
	CArrayUint32			aiRegionGroups;
	int						iNumRegions;
	int						iRegion;
	int						i;
	int						iFace;
	uint32					uiGroup;

	iNumFaces = pcMesh->NumFaces();
	pcMesh->mcSmoothing.SetAll((size)iNumFaces, 0);
	if (iNumFaces == 0)
	{
		return;
	}

	fDot = Deg2Dot(mfSharpAngle);

	asPolygonNormals.Init();
	if (pcMesh->mcPolygons.InUse())
	{
		pcMesh->mcPolygons.GetNormals(&asPolygonNormals, &pcMesh->mcNormals);
	}

	aiRegions.Init();
	aiOrder.Init();
	aiRegionStarts.Init();
	FindAngleRegions(pcMesh, fDot, &asPolygonNormals, &aiRegions, &aiOrder, &aiRegionStarts);
	iNumRegions = (int)aiRegionStarts.NumElements() - 1;

	aiRegionGroups.Init();
	aiRegionGroups.Resize(iNumRegions);
	for (iRegion = 0; iRegion < iNumRegions; iRegion++)
	{
		uiGroup = ChooseRegionGroup(pcMesh, iRegion, &aiRegions, &aiOrder, &aiRegionStarts, &aiRegionGroups);
		aiRegionGroups.SetValue(iRegion, uiGroup);
		for (i = aiRegionStarts.GetValue(iRegion); i < aiRegionStarts.GetValue(iRegion + 1); i++)
		{
			iFace = aiOrder.GetValue(i);
			pcMesh->mcSmoothing.mauiGroups.SetValue(iFace, uiGroup);
		}
	}

	aiRegionGroups.Kill();
	aiRegionStarts.Kill();
	aiOrder.Kill();
	aiRegions.Kill();
	asPolygonNormals.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The normal a face is compared by: its polygon's, if it's in one, else its
// own.
//
//////////////////////////////////////////////////////////////////////////
static SFloat32Vec3* AngleRegionNormal(CMesh* pcMesh, CArrayLargeFloat32Vec3* pasPolygonNormals, int iFace)
{
	int		iPolygon;

	if (pasPolygonNormals->NumElements() != 0)
	{
		iPolygon = pcMesh->mcPolygons.maiFacesToPolygons.GetValue(iFace);
		if (iPolygon != -1)
		{
			return pasPolygonNormals->Get(iPolygon);
		}
	}
	return pcMesh->mcNormals.GetFaceNormal(iFace);
}


//////////////////////////////////////////////////////////////////////////
//
// Floods the faces into regions across edges that aren't sharp.  paiRegions
// gets each face's region; paiOrder the faces, a region's together and in
// order; paiRegionStarts where each region starts in paiOrder, and one more
// for where the last ends.
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothGroups::FindAngleRegions(CMesh* pcMesh, float32 fDot, CArrayLargeFloat32Vec3* pasPolygonNormals, CArrayInt* paiRegions, CArrayInt* paiOrder, CArrayInt* paiRegionStarts)
{
	int						iNumFaces;
	int						iFace;
	int						iRegion;
	int						iNext;
	int						iThis;
	int						iOther;
	size					j;
	SAdjFaces				sAdjFaces;
	SFloat32Vec3*			psThisNormal;
	SFloat32Vec3*			psOtherNormal;
	bool					bSamePolygon;
	int						iThisPolygon;
	CMeshConnectivity*		pcConn;

	iNumFaces = pcMesh->NumFaces();
	pcConn = pcMesh->GetConnectivity();

	paiRegions->Resize(iNumFaces);
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		paiRegions->SetValue(iFace, -1);
	}

	iRegion = 0;
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		if (paiRegions->GetValue(iFace) != -1)
		{
			continue;
		}

		paiRegionStarts->Add((int)paiOrder->NumElements());
		paiRegions->SetValue(iFace, iRegion);
		paiOrder->Add(iFace);

		//paiOrder is the queue: what's after iNext is still to be looked around.
		for (iNext = paiRegionStarts->GetValue(iRegion); iNext < (int)paiOrder->NumElements(); iNext++)
		{
			iThis = paiOrder->GetValue(iNext);
			psThisNormal = AngleRegionNormal(pcMesh, pasPolygonNormals, iThis);
			iThisPolygon = -1;
			if (pasPolygonNormals->NumElements() != 0)
			{
				iThisPolygon = pcMesh->mcPolygons.maiFacesToPolygons.GetValue(iThis);
			}

			sAdjFaces = pcConn->GetAdjacentFaces(iThis);
			for (j = 0; j < NUM_FACE_EDGES; j++)
			{
				iOther = sAdjFaces.aiFace[j];
				if ((iOther == -1) || (paiRegions->GetValue(iOther) != -1))
				{
					continue;
				}

				bSamePolygon = (iThisPolygon != -1) && (pcMesh->mcPolygons.maiFacesToPolygons.GetValue(iOther) == iThisPolygon);
				psOtherNormal = AngleRegionNormal(pcMesh, pasPolygonNormals, iOther);
				if (bSamePolygon || (Float32Vec3Dot(psThisNormal, psOtherNormal) >= fDot))
				{
					paiRegions->SetValue(iOther, iRegion);
					paiOrder->Add(iOther);
				}
			}
		}
		iRegion++;
	}
	paiRegionStarts->Add((int)paiOrder->NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
// The lowest bit that no region before iRegion touching it at a corner has.
// If they have all 32, the bit it touches least: counted by the faces of
// those regions at its corners.
//
//////////////////////////////////////////////////////////////////////////
uint32 CMeshSmoothGroups::ChooseRegionGroup(CMesh* pcMesh, int iRegion, CArrayInt* paiRegions, CArrayInt* paiOrder, CArrayInt* paiRegionStarts, CArrayUint32* paiRegionGroups)
{
	int				aiBitUses[MAX_UNOVERLAPPING_SMOOTHING_GROUPS];
	uint32			uiUsed;
	int				i;
	int				iFace;
	int*			aiCorners;
	size			k;
	size			l;
	size			uiNumCornerFaces;
	CMeshCorner*	pcCorner;
	int				iOtherRegion;
	uint32			uiOtherGroup;
	int				iBit;
	int				iLeastBit;

	memset(aiBitUses, 0, sizeof(aiBitUses));
	uiUsed = 0;
	for (i = paiRegionStarts->GetValue(iRegion); i < paiRegionStarts->GetValue(iRegion + 1); i++)
	{
		iFace = paiOrder->GetValue(i);
		aiCorners = pcMesh->GetFaceCorners(iFace);
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			pcCorner = pcMesh->GetCorner(aiCorners[k]);
			uiNumCornerFaces = pcCorner->maiFace.NumElements();
			for (l = 0; l < uiNumCornerFaces; l++)
			{
				iOtherRegion = paiRegions->GetValue(pcCorner->maiFace.GetValue(l));
				if (iOtherRegion < iRegion)
				{
					uiOtherGroup = paiRegionGroups->GetValue(iOtherRegion);
					uiUsed |= uiOtherGroup;
					for (iBit = 0; iBit < MAX_UNOVERLAPPING_SMOOTHING_GROUPS; iBit++)
					{
						if (uiOtherGroup == ((uint32)1 << iBit))
						{
							aiBitUses[iBit]++;
						}
					}
				}
			}
		}
	}

	iLeastBit = 0;
	for (iBit = 0; iBit < MAX_UNOVERLAPPING_SMOOTHING_GROUPS; iBit++)
	{
		if ((uiUsed & ((uint32)1 << iBit)) == 0)
		{
			return (uint32)1 << iBit;
		}
		if (aiBitUses[iBit] < aiBitUses[iLeastBit])
		{
			iLeastBit = iBit;
		}
	}
	return (uint32)1 << iLeastBit;
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
	SFloat32Vec3		cNormal;
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
	SFloat32Vec3*		pcNormal;
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


