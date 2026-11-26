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
#include "StandardLib/Objects.h"
#include "MeshEditor.h"
#include "MeshSourceModifier.h"


//////////////////////////////////////////////////////////////////////////
//
// A new mesh of pMesh and the shape.  The shape's faces come after pMesh's,
// and flat normals come before corner normals, so pMesh's corner normals go
// on after the shape's flat ones and are numbered that much higher.  pMesh's
// polygons, smoothing, UVs, materials and skin stay on its own faces.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshSourceModifier::Modify(Ptr<CMesh> pMesh)
{
	Ptr<CMesh>			pOutput;
	CMeshEditor			cEditor;
	SMeshNormalFace*	psNormalFace;
	SMeshNormalFace*	psOutputNormalFace;
	SFloat32Vec3*		psNormal;
	int*				piCorners;
	int					iNumFaces;
	int					iNumNormals;
	int					iNumCorners;
	int					iNewFaces;
	int					iFace;
	int					i;
	size				u;
	bool				abEdges[NUM_FACE_EDGES];

	pOutput = OMalloc<CMesh>();
	pOutput->mcNormals.mbInUse = pMesh->mcNormals.InUse();
	pOutput->mcColours.mbInUse = pMesh->mcColours.InUse();
	pOutput->mcSmoothing.mbInUse = pMesh->mcSmoothing.InUse();
	for (u = 0; u < pMesh->mcColours.mcColours.NumElements(); u++)
	{
		pOutput->mcColours.mcColours.Add(pMesh->mcColours.mcColours.GetValue(u));
	}

	cEditor.Init(pOutput);

	iNumCorners = pMesh->NumCorners();
	for (i = 0; i < iNumCorners; i++)
	{
		cEditor.AddCorner(pMesh->GetPosition(i));
	}

	iNumFaces = pMesh->NumFaces();
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		piCorners = pMesh->GetFaceCorners(iFace);
		pMesh->mcPolygons.GetFaceEdges(pMesh->GetConnectivity(), iFace, abEdges);
		cEditor.AddFace(piCorners[0], piCorners[1], piCorners[2], pMesh->GetFaceName(iFace), abEdges[0], abEdges[1], abEdges[2]);
		if (pMesh->mcSmoothing.InUse())
		{
			pOutput->mcSmoothing.mauiGroups.SetValue(iFace, pMesh->mcSmoothing.mauiGroups.GetValue(iFace));
		}
		if (pMesh->mcColours.InUse())
		{
			for (i = 0; i < NUM_FACE_CORNERS; i++)
			{
				pOutput->mcColours.mcFaces.Get(iFace)->aiCornerColours[i] = pMesh->mcColours.mcFaces.Get(iFace)->aiCornerColours[i];
			}
		}
	}

	Generate(&cEditor);
	cEditor.Kill();

	pOutput->CopyUVsFrom(&pMesh);
	pOutput->CopyMaterialsFrom(&pMesh);
	pOutput->CopySkinFrom(&pMesh);

	if (pMesh->mcNormals.InUse())
	{
		iNewFaces = pOutput->NumFaces() - iNumFaces;
		for (iFace = 0; iFace < iNumFaces; iFace++)
		{
			psNormalFace = pMesh->mcNormals.mcFaces.Get(iFace);
			psOutputNormalFace = pOutput->mcNormals.mcFaces.Get(iFace);
			for (i = 0; i < NUM_FACE_CORNERS; i++)
			{
				psOutputNormalFace->aiCornerNormals[i] = (psNormalFace->aiCornerNormals[i] < 0) ? -1 : psNormalFace->aiCornerNormals[i] + iNewFaces;
			}
		}

		iNumNormals = (int)pMesh->mcNormals.mcNormals.NumElements();
		for (i = iNumFaces; i < iNumNormals; i++)
		{
			psNormal = pMesh->mcNormals.mcNormals.Get(i);
			pOutput->AddNormal(psNormal->x, psNormal->y, psNormal->z);
		}
	}
	return pOutput;
}


//////////////////////////////////////////////////////////////////////////
//
// The direction across psZDirection (which is normalised) that a shape's
// first wedge points in: Y with what is along psZDirection taken out, or X
// if psZDirection is along Y.  Every shape around an axis starts from it.
//
//////////////////////////////////////////////////////////////////////////
void CMeshSourceModifier::WedgeDirection(SFloat32Vec3* psDest, SFloat32Vec3* psZDirection)
{
	SFloat32Vec3	sAcross;
	SFloat32Vec3	sAlong;
	float32			fDot;

	sAcross.Init(0, 1, 0);
	fDot = Float32Vec3Dot(&sAcross, psZDirection);
	if (fabsf(fDot) > 0.99f)
	{
		sAcross.Init(1, 0, 0);
		fDot = Float32Vec3Dot(&sAcross, psZDirection);
	}
	sAlong.Init(psZDirection);
	sAlong *= fDot;
	*psDest = sAcross - sAlong;
	psDest->Normalize();
}


//////////////////////////////////////////////////////////////////////////
//
// Turns a closed loop of iNumPoints (each a radius out from psZDirection and
// a distance along it from psStart) once around psZDirection, in
// iWedgeSegments wedges.  The quad between point p and the next is named
// paiFaceNames[p].  Its faces face out where the loop goes along
// psZDirection on the outside, as CMeshRingEditor's do.
//
//////////////////////////////////////////////////////////////////////////
void CMeshSourceModifier::GenerateRevolved(CMeshEditor* pcMeshEditor, SFloat32Vec3* psZDirection, SFloat32Vec3* psStart, float32* pafRadii, float32* pafZs, int* paiFaceNames, int iNumPoints, int iWedgeSegments, bool bFlipFaces)
{
	SFloat32Vec3	sAcross;
	SFloat32Vec3	sWedge;
	SFloat32Vec3	sAlong;
	float32			fWedgeRad;
	int				iStartPosition;
	int				iThis;
	int				iNext;
	int				p;
	int				i;

	iStartPosition = pcMeshEditor->NumCorners();
	fWedgeRad = (N_PI*2.0f) / iWedgeSegments;
	WedgeDirection(&sAcross, psZDirection);

	for (p = 0; p < iNumPoints; p++)
	{
		sAlong.Init(psZDirection);
		sAlong *= pafZs[p];
		sAlong.Add(psStart);
		for (i = 0; i < iWedgeSegments; i++)
		{
			Float32Vec3RotateAboutNormalisedAxis(&sWedge, sAcross, fWedgeRad * i, *psZDirection);
			sWedge *= pafRadii[p];
			sWedge.Add(&sAlong);
			pcMeshEditor->AddCorner(&sWedge);
		}
	}

	for (p = 0; p < iNumPoints; p++)
	{
		iThis = iStartPosition + (p * iWedgeSegments);
		iNext = iStartPosition + (((p + 1) % iNumPoints) * iWedgeSegments);
		for (i = 0; i + 1 < iWedgeSegments; i++)
		{
			pcMeshEditor->AddQuad(iThis + i + 1, iThis + i, iNext + i, iNext + i + 1, paiFaceNames[p], !bFlipFaces);
		}
		pcMeshEditor->AddQuad(iThis, iThis + i, iNext + i, iNext, paiFaceNames[p], !bFlipFaces);
	}
}

