/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ProceduralLib

Codaphela ProceduralLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ProceduralLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ProceduralLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "BaseLib/ArrayInt.h"
#include "StandardLib/Objects.h"
#include "MeshShapeNormals.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshShapeNormals::Init(Ptr<CShape> pShape, float32 fCreaseAngle)
{
	PreInit();
	CMeshModifier::Init();
	mpShape = pShape;
	mfCreaseAngle = fCreaseAngle;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshShapeNormals::Class(void)
{
	CMeshModifier::Class();
	M_Pointer(mpShape);
	U_Float32(mfCreaseAngle);
}


//////////////////////////////////////////////////////////////////////////
//
// A new mesh of the same corners and faces, its flat normals first (as
// AddFace makes them with normals in use), then the normals its face
// corners use.  Each face corner first finds the gradient a tenth of the
// way in toward its face's middle: its own side of any crease.  A corner
// where those of its faces are all within the crease angle of each other
// is smooth, and its faces share the gradient at the corner itself (which
// at a crease is only one side's, or a blend).  At a crease each face
// corner has its own side's.  A face corner whose normal would be further
// than the crease angle from its face keeps its flat normal (-1).
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshShapeNormals::Modify(Ptr<CMesh> pMesh)
{
	Ptr<CMesh>					pOutput;
	CArrayLargeInt				aiCornerNormals;  //Each corner's shared normal: -1 not added yet, -2 none (the gradient has no direction).
	CArrayLargeInt				aiCreased;  //Each corner: 0 no face seen, 1 smooth so far, 2 creased.
	CArrayLargeFloat32Vec3		asFirstInside;  //Each corner's first face's own side's gradient.
	CArrayLargeFloat32Vec3		asInside;  //Each face corner's own side's gradient, 0 if it has none or its face no area.
	SMeshNormalFace*			psNormalFace;
	SFloat32Vec3				sFaceNormal;
	SFloat32Vec3				sSide1;
	SFloat32Vec3				sSide2;
	SFloat32Vec3				sMiddle;
	SFloat32Vec3				sPoint;
	SFloat32Vec3				sNormal;
	SFloat32Vec3*				psInside;
	int*						piCorners;
	int*						piColours;
	int							iNumFaces;
	int							iNumNormals;
	int							iFace;
	int							iCorner;
	int							iNormal;
	int							k;
	float32						fMinCosine;
	float32						fLength;
	size						u;

	if (mpShape.IsNull() || (pMesh->NumFaces() == 0) || !CanRebuild(&pMesh))
	{
		return pMesh;
	}

	pOutput = OMalloc<CMesh>();
	pOutput->mcNormals.mbInUse = true;
	pOutput->mcColours.mbInUse = pMesh->mcColours.InUse();
	for (u = 0; u < pMesh->mcColours.mcColours.NumElements(); u++)
	{
		pOutput->mcColours.mcColours.Add(pMesh->mcColours.mcColours.GetValue(u));
	}
	for (iCorner = 0; iCorner < pMesh->NumCorners(); iCorner++)
	{
		pOutput->AddCorner(pMesh->GetPosition(iCorner));
	}

	iNumFaces = pMesh->NumFaces();
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		piCorners = pMesh->GetFaceCorners(iFace);
		pOutput->AddFace(piCorners[0], piCorners[1], piCorners[2]);
		pOutput->SetFaceName(iFace, pMesh->GetFaceName(iFace));
		if (pMesh->mcColours.InUse())
		{
			piColours = pMesh->mcColours.mcFaces.Get(iFace)->aiCornerColours;
			for (k = 0; k < NUM_FACE_CORNERS; k++)
			{
				pOutput->mcColours.mcFaces.Get(iFace)->aiCornerColours[k] = piColours[k];
			}
		}
	}

	fMinCosine = cosf(mfCreaseAngle);
	aiCornerNormals.Init();
	aiCornerNormals.Resize((lsize)pMesh->NumCorners());
	aiCornerNormals.SetArrayValues(-1);
	aiCreased.Init();
	aiCreased.Resize((lsize)pMesh->NumCorners());
	aiCreased.SetArrayValues(0);
	asFirstInside.Init();
	asFirstInside.Resize((lsize)pMesh->NumCorners());
	asInside.Init();
	asInside.Resize((lsize)(iNumFaces * NUM_FACE_CORNERS));

	//Each face corner's own side's gradient, and whether each corner's faces' agree.
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		piCorners = pMesh->GetFaceCorners(iFace);
		sMiddle = (*pMesh->GetPosition(piCorners[0]) + *pMesh->GetPosition(piCorners[1]) + *pMesh->GetPosition(piCorners[2])) / 3.0f;
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			iCorner = piCorners[k];
			psInside = asInside.Get((lsize)((iFace * NUM_FACE_CORNERS) + k));
			sPoint = *pMesh->GetPosition(iCorner) + ((sMiddle - *pMesh->GetPosition(iCorner)) * MESH_SHAPE_NORMALS_INSIDE);
			mpShape->Gradient(psInside, &sPoint);
			fLength = (float32)psInside->Magnitude();
			if (fLength <= 0.0f)
			{
				psInside->Init(0, 0, 0);
				aiCreased.SetValue((lsize)iCorner, 2);
				continue;
			}
			*psInside /= fLength;

			if (aiCreased.GetValue((lsize)iCorner) == 0)
			{
				*asFirstInside.Get((lsize)iCorner) = *psInside;
				aiCreased.SetValue((lsize)iCorner, 1);
			}
			else if (Float32Vec3Dot(psInside, asFirstInside.Get((lsize)iCorner)) < fMinCosine)
			{
				aiCreased.SetValue((lsize)iCorner, 2);
			}
		}
	}

	iNumNormals = iNumFaces;
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		piCorners = pMesh->GetFaceCorners(iFace);
		psNormalFace = pOutput->mcNormals.mcFaces.Get(iFace);
		sSide1 = *pMesh->GetPosition(piCorners[1]) - *pMesh->GetPosition(piCorners[0]);
		sSide2 = *pMesh->GetPosition(piCorners[2]) - *pMesh->GetPosition(piCorners[0]);
		Float32Vec3Cross(&sFaceNormal, &sSide1, &sSide2);
		fLength = (float32)sFaceNormal.Magnitude();
		if (fLength > 0.0f)
		{
			sFaceNormal /= fLength;
		}

		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			iCorner = piCorners[k];
			psNormalFace->aiCornerNormals[k] = -1;
			if (fLength <= 0.0f)
			{
				continue;
			}

			if (aiCreased.GetValue((lsize)iCorner) == 1)
			{
				iNormal = aiCornerNormals.GetValue((lsize)iCorner);
				if (iNormal == -1)
				{
					iNormal = -2;
					mpShape->Gradient(&sNormal, pMesh->GetPosition(iCorner));
					fLength = (float32)sNormal.Magnitude();
					if (fLength > 0.0f)
					{
						sNormal /= fLength;
						pOutput->AddNormal(sNormal.x, sNormal.y, sNormal.z);
						iNormal = iNumNormals;
						iNumNormals++;
					}
					aiCornerNormals.SetValue((lsize)iCorner, iNormal);
					fLength = 1.0f;
				}
				if ((iNormal >= 0) && (Float32Vec3Dot(pOutput->mcNormals.mcNormals.Get((lsize)iNormal), &sFaceNormal) >= fMinCosine))
				{
					psNormalFace->aiCornerNormals[k] = iNormal;
					continue;
				}
			}

			psInside = asInside.Get((lsize)((iFace * NUM_FACE_CORNERS) + k));
			if (Float32Vec3Dot(psInside, &sFaceNormal) >= fMinCosine)
			{
				pOutput->AddNormal(psInside->x, psInside->y, psInside->z);
				psNormalFace->aiCornerNormals[k] = iNumNormals;
				iNumNormals++;
			}
		}
	}

	asInside.Kill();
	asFirstInside.Kill();
	aiCreased.Kill();
	aiCornerNormals.Kill();
	return pOutput;
}

