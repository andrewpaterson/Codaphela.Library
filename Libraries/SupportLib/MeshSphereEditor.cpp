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
#include "MeshEditor.h"
#include "MeshSphereEditor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSphereEditor::Init(SFloat32Vec3* psZDirection, SFloat32Vec3* psCentre, float32 fRadius, int iWedgeSegments, int iRingSegments)
{
	PreInit();

	msZDirection.Init(psZDirection);
	msZDirection.Normalize();
	msCentre.Init(psCentre);
	mfRadius = fRadius;
	miWedgeSegments = iWedgeSegments;
	miRingSegments = iRingSegments;
	mbFlipFaces = false;
	miFaceName = 0;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSphereEditor::Class(void)
{
	CMeshSourceModifier::Class();
	U_3Float32(msZDirection);
	U_3Float32(msCentre);
	U_Float32(mfRadius);
	U_SInt(miWedgeSegments);
	U_SInt(miRingSegments);
	U_Bool(mbFlipFaces);
	U_SInt(miFaceName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSphereEditor::SetFlipFaces(bool bFlipFaces)
{
	mbFlipFaces = bFlipFaces;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSphereEditor::SetFaceName(int iFaceName)
{
	miFaceName = iFaceName;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshSphereEditor::ExpectedFaceChunkSize(void)
{
	return miWedgeSegments * (miRingSegments - 1) * 2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshSphereEditor::ExpectedCornerChunkSize(void)
{
	return 2 + miWedgeSegments * (miRingSegments - 1);
}


//////////////////////////////////////////////////////////////////////////
//
// The pole against msZDirection, then a ring of corners at the top of each
// band but the last, then the pole along msZDirection.  The bottom pole's
// faces turn the way CMeshConeEditor's bottom disc's do, the top's the way
// its top disc's do and the bands the way its ring's do.
//
//////////////////////////////////////////////////////////////////////////
void CMeshSphereEditor::Generate(CMeshEditor* pcMeshEditor)
{
	SFloat32Vec3	sAcross;
	SFloat32Vec3	sPosition;
	SFloat32Vec3	sAlong;
	float32			fWedgeRad;
	float32			fRingRad;
	float32			fRingRadius;
	int				iBottom;
	int				iFirstRing;
	int				iTop;
	int				iLower;
	int				iUpper;
	int				k;
	int				i;

	fWedgeRad = (N_PI*2.0f) / miWedgeSegments;
	fRingRad = N_PI / miRingSegments;
	WedgeDirection(&sAcross, &msZDirection);

	iBottom = pcMeshEditor->NumCorners();
	sPosition.Init(&msZDirection);
	sPosition *= -mfRadius;
	sPosition.Add(&msCentre);
	pcMeshEditor->AddCorner(&sPosition);

	iFirstRing = pcMeshEditor->NumCorners();
	for (k = 1; k < miRingSegments; k++)
	{
		fRingRadius = sinf(fRingRad * k) * mfRadius;
		sAlong.Init(&msZDirection);
		sAlong *= -cosf(fRingRad * k) * mfRadius;
		sAlong.Add(&msCentre);
		for (i = 0; i < miWedgeSegments; i++)
		{
			Float32Vec3RotateAboutNormalisedAxis(&sPosition, sAcross, fWedgeRad * i, msZDirection);
			sPosition *= fRingRadius;
			sPosition.Add(&sAlong);
			pcMeshEditor->AddCorner(&sPosition);
		}
	}

	iTop = pcMeshEditor->NumCorners();
	sPosition.Init(&msZDirection);
	sPosition *= mfRadius;
	sPosition.Add(&msCentre);
	pcMeshEditor->AddCorner(&sPosition);

	for (i = 0; i + 1 < miWedgeSegments; i++)
	{
		pcMeshEditor->AddFace(iBottom, iFirstRing + i, iFirstRing + i + 1, miFaceName, true, true, true, !mbFlipFaces);
	}
	pcMeshEditor->AddFace(iBottom, iFirstRing + i, iFirstRing, miFaceName, true, true, true, !mbFlipFaces);

	for (k = 0; k + 2 < miRingSegments; k++)
	{
		iLower = iFirstRing + (k * miWedgeSegments);
		iUpper = iLower + miWedgeSegments;
		for (i = 0; i + 1 < miWedgeSegments; i++)
		{
			pcMeshEditor->AddQuad(iLower + i + 1, iLower + i, iUpper + i, iUpper + i + 1, miFaceName, !mbFlipFaces);
		}
		pcMeshEditor->AddQuad(iLower, iLower + i, iUpper + i, iUpper, miFaceName, !mbFlipFaces);
	}

	iUpper = iFirstRing + ((miRingSegments - 2) * miWedgeSegments);
	for (i = 0; i + 1 < miWedgeSegments; i++)
	{
		pcMeshEditor->AddFace(iTop, iUpper + i, iUpper + i + 1, miFaceName, true, true, true, mbFlipFaces);
	}
	pcMeshEditor->AddFace(iTop, iUpper + i, iUpper, miFaceName, true, true, true, mbFlipFaces);
}

