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
#include "MeshTorusEditor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshTorusEditor::Init(SFloat32Vec3* psZDirection, SFloat32Vec3* psCentre, float32 fRadius, float32 fTubeRadius, int iWedgeSegments, int iTubeSegments)
{
	msZDirection.Init(psZDirection);
	msZDirection.Normalize();
	msCentre.Init(psCentre);
	mfRadius = fRadius;
	mfTubeRadius = fTubeRadius;
	miWedgeSegments = iWedgeSegments;
	miTubeSegments = iTubeSegments;
	mbFlipFaces = false;
	miFaceName = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshTorusEditor::SetFlipFaces(bool bFlipFaces)
{
	mbFlipFaces = bFlipFaces;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshTorusEditor::SetFaceName(int iFaceName)
{
	miFaceName = iFaceName;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshTorusEditor::Kill(void)
{
	CMeshSourceModifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshTorusEditor::ExpectedFaceChunkSize(void)
{
	return miWedgeSegments * miTubeSegments * 2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshTorusEditor::ExpectedCornerChunkSize(void)
{
	return miWedgeSegments * miTubeSegments;
}


//////////////////////////////////////////////////////////////////////////
//
// The tube's circle starts on the outside and goes up over the top first.
//
//////////////////////////////////////////////////////////////////////////
void CMeshTorusEditor::Generate(CMeshEditor* pcMeshEditor)
{
	float32*	pafRadii;
	float32*	pafZs;
	int*		paiFaceNames;
	float32		fTubeRad;
	int			k;

	pafRadii = (float32*)malloc(sizeof(float32) * miTubeSegments);
	pafZs = (float32*)malloc(sizeof(float32) * miTubeSegments);
	paiFaceNames = (int*)malloc(sizeof(int) * miTubeSegments);

	fTubeRad = (N_PI*2.0f)/miTubeSegments;
	for (k = 0; k < miTubeSegments; k++)
	{
		pafRadii[k] = mfRadius + cosf(fTubeRad * k) * mfTubeRadius;
		pafZs[k] = sinf(fTubeRad * k) * mfTubeRadius;
		paiFaceNames[k] = miFaceName;
	}

	GenerateRevolved(pcMeshEditor, &msZDirection, &msCentre, pafRadii, pafZs, paiFaceNames, miTubeSegments, miWedgeSegments, mbFlipFaces);

	free(paiFaceNames);
	free(pafZs);
	free(pafRadii);
}

