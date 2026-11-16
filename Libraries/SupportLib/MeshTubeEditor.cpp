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
#include <stdlib.h>
#include "BaseLib/Numbers.h"
#include "MeshEditor.h"
#include "MeshTubeEditor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshTubeEditor::Init(SFloat32Vec3* psZDirection, SFloat32Vec3* psStart, float32 fInnerRadius, float32 fOuterRadius, float32 fLength, int iWedgeSegments, int iCircleSegments, int iRingSegments)
{
	PreInit();

	msZDirection.Init(psZDirection);
	msZDirection.Normalize();
	msStart.Init(psStart);
	mfInnerRadius = fInnerRadius;
	mfOuterRadius = fOuterRadius;
	mfLength = fLength;
	miWedgeSegments = iWedgeSegments;
	miCircleSegments = iCircleSegments;
	miRingSegments = iRingSegments;
	mbFlipFaces = false;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshTubeEditor::Class(void)
{
	CMeshSourceModifier::Class();
	U_3Float32(msZDirection);
	U_3Float32(msStart);
	U_Float32(mfInnerRadius);
	U_Float32(mfOuterRadius);
	U_Float32(mfLength);
	U_SInt(miWedgeSegments);
	U_SInt(miCircleSegments);
	U_SInt(miRingSegments);
	U_Bool(mbFlipFaces);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshTubeEditor::SetFlipFaces(bool bFlipFaces)
{
	mbFlipFaces = bFlipFaces;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshTubeEditor::NumProfilePoints(void) const
{
	return ((miRingSegments - 1) + miCircleSegments) * 2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshTubeEditor::ExpectedFaceChunkSize(void)
{
	return miWedgeSegments * NumProfilePoints() * 2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshTubeEditor::ExpectedCornerChunkSize(void)
{
	return miWedgeSegments * NumProfilePoints();
}


//////////////////////////////////////////////////////////////////////////
//
// Up the outside, in across the top, down the inside and out across the
// bottom.
//
//////////////////////////////////////////////////////////////////////////
void CMeshTubeEditor::Generate(CMeshEditor* pcMeshEditor)
{
	float32*	pafRadii;
	float32*	pafZs;
	int*		paiFaceNames;
	int			iNumPoints;
	int			iWallBands;
	int			p;
	int			j;

	iNumPoints = NumProfilePoints();
	iWallBands = miRingSegments - 1;
	pafRadii = (float32*)malloc(sizeof(float32) * iNumPoints);
	pafZs = (float32*)malloc(sizeof(float32) * iNumPoints);
	paiFaceNames = (int*)malloc(sizeof(int) * iNumPoints);

	p = 0;
	for (j = 0; j < iWallBands; j++, p++)
	{
		pafRadii[p] = mfOuterRadius;
		pafZs[p] = (mfLength * j) / iWallBands;
		paiFaceNames[p] = 2;
	}
	for (j = 0; j < miCircleSegments; j++, p++)
	{
		pafRadii[p] = mfOuterRadius + (((mfInnerRadius - mfOuterRadius) * j) / miCircleSegments);
		pafZs[p] = mfLength;
		paiFaceNames[p] = 1;
	}
	for (j = 0; j < iWallBands; j++, p++)
	{
		pafRadii[p] = mfInnerRadius;
		pafZs[p] = mfLength - ((mfLength * j) / iWallBands);
		paiFaceNames[p] = 4;
	}
	for (j = 0; j < miCircleSegments; j++, p++)
	{
		pafRadii[p] = mfInnerRadius + (((mfOuterRadius - mfInnerRadius) * j) / miCircleSegments);
		pafZs[p] = 0.0f;
		paiFaceNames[p] = 3;
	}

	GenerateRevolved(pcMeshEditor, &msZDirection, &msStart, pafRadii, pafZs, paiFaceNames, iNumPoints, miWedgeSegments, mbFlipFaces);

	free(paiFaceNames);
	free(pafZs);
	free(pafRadii);
}

