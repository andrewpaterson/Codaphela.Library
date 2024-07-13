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
#include "BaseLib/FloatHelper.h"
#include "MeshEditor.h"
#include "MeshRingEditor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshRingEditor::Init(SFloat3* psZDirection, SFloat3* psStart, float fTopRadius, float fBottomRadius, float fLength, int iWedgeSegments, int iRingSegments)
{
	Float3Assign(&msZDirection, psZDirection);
	Float3Assign(&msStart, psStart);
	mfTopRadius = fTopRadius;
	mfBottomRadius = fBottomRadius;
	mfLength = fLength;
	miWedgeSegments = iWedgeSegments;
	miRingSegments = iRingSegments;
	mbFlipFaces = false;
	miFaceName = 0;
	miZI = 0;
	miZE = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshRingEditor::SetFlipFaces(bool bFlipFaces)
{
	mbFlipFaces = bFlipFaces;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshRingEditor::SetFaceName(int iFaceName)
{
	miFaceName = iFaceName;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshRingEditor::SetZOffsets(int iZStartOffset, int iZEndOffset)
{
	miZI = iZStartOffset;
	miZE = iZEndOffset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshRingEditor::Kill(void)
{
	CMeshSourceModifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshRingEditor::ExpectedFaceChunkSize(void)
{
	int iNumFaces;

	iNumFaces = miWedgeSegments * (miRingSegments-1-miZI-miZE) * 2;
	return iNumFaces;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshRingEditor::ExpectedCornerChunkSize(void)
{
	int	iNumVerticies;

	iNumVerticies = miWedgeSegments * (miRingSegments-miZI-miZE);
	return iNumVerticies;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshRingEditor::Apply(CMeshEditor* pcMeshEditor)
{
	ClearConnectivity(pcMeshEditor);
	GenerateRing(pcMeshEditor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshRingEditor::GenerateRing(CMeshEditor* pcMeshEditor)
{
	int			i;
	int			z;
	SFloat3		sDest;
	SFloat3		sSource;
	float		fWedgeRad;
	float		fRadius;
	float		f;
	float		fZ;
	SFloat3		sRingCenter;
	int			iStartPosition;
	int			iZOffset0;
	int			iZOffset1;

	iStartPosition = pcMeshEditor->NumCorners();

	fWedgeRad = (N_PI*2.0f)/miWedgeSegments;

	for (z = miZI; z < miRingSegments-miZE; z++)
	{
		f = (float)z/(float)(miRingSegments-1);
		fRadius = InterpolateFloat(mfBottomRadius, mfTopRadius, f);
		fZ = mfLength * f;

		sSource.Init(0.0f, fRadius, 0.0f);
		for (i = 0; i < miWedgeSegments; i++)
		{
			Float3RotateAboutNormalisedAxis(&sDest, sSource, fWedgeRad * i, msZDirection);
			sDest.Add(&msStart);
			Float3Assign(&sRingCenter, &msZDirection);
			sRingCenter *= fZ;
			sDest.Add(&sRingCenter);
			pcMeshEditor->AddCorner(&sDest);
		}
	}

	for (z = miZI; z < miRingSegments-1-miZE; z++)
	{
		iZOffset0 = (z-miZI);
		iZOffset1 = iZOffset0+1;
		AddZStrip(pcMeshEditor, iStartPosition + (iZOffset0 * miWedgeSegments), iStartPosition + (iZOffset1 * miWedgeSegments), !mbFlipFaces);
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshRingEditor::AddZStrip(CMeshEditor* pcMeshEditor, int iStartPosition1, int iStartPosition2, bool bFlipFaces)
{
	int		i;

	for (i = 0; i < miWedgeSegments-1; i++)
	{
		pcMeshEditor->AddQuad(iStartPosition1 + i+1, iStartPosition1 + i, iStartPosition2 + i, iStartPosition2 + i+1, miFaceName, bFlipFaces);
	}
	pcMeshEditor->AddQuad(iStartPosition1, iStartPosition1 + i, iStartPosition2 + i, iStartPosition2, miFaceName, bFlipFaces);
}
