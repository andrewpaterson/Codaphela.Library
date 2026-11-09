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
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "MeshModifier.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifier::Kill(void)
{
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshModifier::IsSource(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifier::Apply(CMeshEditor* pcMeshEditor)
{
	NotImplemented(__METHOD__);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshModifier::Modify(Ptr<CMesh> pMesh)
{
	NotImplemented(__METHOD__);
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifier::ClearConnectivity(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifier::AddPosition(float32 x, float32 y, float32 z)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifier::AddPosition(SFloat32Vec3* psPosition)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifier::AddFace(int iCorner1, int iCorner2, int iCorner3)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifier::AddFaces(CArrayInt* paiCorners)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifier::RemoveFace(int iFace)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifier::RemoveFaces(CArrayInt* paiFaces)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifier::RemoveCorner(int iCorner)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifier::RemoveCorners(CArrayInt* paiCorners)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifier::MovePosition(int iPosition, SFloat32Vec3* psPosition)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifier::MovePositions(CArrayInt* paiPositions, SFloat32Vec3* psPosition)
{
}


//////////////////////////////////////////////////////////////////////////
//
// A mesh with more than that is left as it is, rather than losing the rest.
//
//////////////////////////////////////////////////////////////////////////
bool CMeshModifier::CanRebuild(CMesh* pcMesh)
{
	if (pcMesh->mcUVs.InUse() || pcMesh->mcSkin.InUse() || pcMesh->mcMaterials.InUse())
	{
		gcLogger.Warning2(__METHOD__, " Can't keep a mesh's UVs, skin or materials.  It is left as it is.", NULL);
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshModifier::CopyWithNormals(CMesh* pcMesh)
{
	Ptr<CMesh>	pOutput;

	pOutput = OMalloc<CMesh>();
	pOutput->mcNormals.mbInUse = true;
	pOutput->CopyFrom(pcMesh);
	return pOutput;
}


//////////////////////////////////////////////////////////////////////////
//
// A new mesh of pcMesh's faces with their corners changed.  paiFaceCorners has
// NUM_FACE_CORNERS indices into pasCorners for each of pcMesh's faces, the
// first of them -1 if the face goes.  Only the corners and normals that are
// used are kept.  Flat normals come first, so the corner normals are numbered
// after the faces that stay.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshModifier::BuildFromFaces(CMesh* pcMesh, SFloat32Vec3* pasCorners, int iNumCorners, int* paiFaceCorners)
{
	Ptr<CMesh>			pOutput;
	CArrayInt			aiOutputCorners;
	CArrayInt			aiOutputNormals;
	CArrayInt			aiNormalOrder;
	SMeshNormalFace*	psNormalFace;
	SMeshNormalFace*	psOutputNormalFace;
	SFloat32Vec3*		psNormal;
	int*				piFaceCorners;
	int*				piColours;
	int*				piOutputCorner;
	int*				piOutputNormal;
	int					aiCorners[NUM_FACE_CORNERS];
	int					iNumFaces;
	int					iNumKept;
	int					iOutputFace;
	int					iFace;
	int					i;
	int					iNormal;
	bool				bNormals;
	bool				bColours;
	size				u;

	bNormals = pcMesh->mcNormals.InUse();
	bColours = pcMesh->mcColours.InUse();

	pOutput = OMalloc<CMesh>();
	pOutput->mcNormals.mbInUse = bNormals;
	pOutput->mcColours.mbInUse = bColours;
	for (u = 0; u < pcMesh->mcColours.mcColours.NumElements(); u++)
	{
		pOutput->mcColours.mcColours.Add(pcMesh->mcColours.mcColours.GetValue(u));
	}

	iNumFaces = pcMesh->NumFaces();
	iNumKept = 0;
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		if (paiFaceCorners[iFace * NUM_FACE_CORNERS] >= 0)
		{
			iNumKept++;
		}
	}

	aiOutputCorners.Init();
	aiOutputCorners.Resize(iNumCorners);
	aiOutputCorners.SetArrayValues(-1);
	aiOutputNormals.Init();
	aiOutputNormals.Resize(pcMesh->mcNormals.mcNormals.NumElements());
	aiOutputNormals.SetArrayValues(-1);
	aiNormalOrder.Init();

	iOutputFace = 0;
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		piFaceCorners = &paiFaceCorners[iFace * NUM_FACE_CORNERS];
		if (piFaceCorners[0] >= 0)
		{
			for (i = 0; i < NUM_FACE_CORNERS; i++)
			{
				piOutputCorner = aiOutputCorners.Get(piFaceCorners[i]);
				if (*piOutputCorner < 0)
				{
					*piOutputCorner = pOutput->NumCorners();
					pOutput->AddCorner(&pasCorners[piFaceCorners[i]]);
				}
				aiCorners[i] = *piOutputCorner;
			}
			pOutput->AddFace(aiCorners[0], aiCorners[1], aiCorners[2]);
			pOutput->SetFaceName(iOutputFace, pcMesh->GetFaceName(iFace));

			if (bNormals)
			{
				psNormalFace = pcMesh->mcNormals.mcFaces.Get(iFace);
				psOutputNormalFace = pOutput->mcNormals.mcFaces.Get(iOutputFace);
				for (i = 0; i < NUM_FACE_CORNERS; i++)
				{
					iNormal = psNormalFace->aiCornerNormals[i];
					if (iNormal >= 0)
					{
						piOutputNormal = aiOutputNormals.Get(iNormal);
						if (*piOutputNormal < 0)
						{
							*piOutputNormal = iNumKept + (int)aiNormalOrder.NumElements();
							aiNormalOrder.Add(iNormal);
						}
						iNormal = *piOutputNormal;
					}
					psOutputNormalFace->aiCornerNormals[i] = iNormal;
				}
			}

			if (bColours)
			{
				piColours = pcMesh->mcColours.mcFaces.Get(iFace)->aiCornerColours;
				for (i = 0; i < NUM_FACE_CORNERS; i++)
				{
					pOutput->mcColours.mcFaces.Get(iOutputFace)->aiCornerColours[i] = piColours[i];
				}
			}
			iOutputFace++;
		}
	}

	for (u = 0; u < aiNormalOrder.NumElements(); u++)
	{
		psNormal = pcMesh->mcNormals.mcNormals.Get(aiNormalOrder.GetValue(u));
		pOutput->AddNormal(psNormal->x, psNormal->y, psNormal->z);
	}

	aiNormalOrder.Kill();
	aiOutputNormals.Kill();
	aiOutputCorners.Kill();
	return pOutput;
}
