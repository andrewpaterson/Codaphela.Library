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
#include "MeshDetail.h"
#include "MeshCorner.h"
#include "MeshDiscEditor.h"
#include "MeshEditor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshDiscEditor::Init(SFloat3* psZDirection, SFloat3* psStart, float fInnerRadius, float fOuterRadius, int iWedgeSegments, int iCircleSegments)
{
	msZDirection.Init(psZDirection->x, psZDirection->y, psZDirection->z);
	msZDirection.Normalize();
	msStart.Init(psStart->x, psStart->y, psStart->z);
	miWedgeSegments = iWedgeSegments;
	miCircleSegments = iCircleSegments;
	mfOuterRadius = fOuterRadius;
	mfInnerRadius = fInnerRadius;
	mbFlipFaces = FALSE;
	miFaceName = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshDiscEditor::SetFlipFaces(BOOL bFlipFaces)
{
	mbFlipFaces = bFlipFaces;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshDiscEditor::SetFaceName(int iFaceName)
{
	miFaceName = iFaceName;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshDiscEditor::Kill(void)
{
	CMeshSourceModifier::Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshDiscEditor::ExpectedFaceChunkSize(void)
{
	int iNumFaces;

	iNumFaces = miWedgeSegments;
	if (miCircleSegments > 1)
	{
		iNumFaces += miWedgeSegments * (miCircleSegments-1) * 2;
	}
	return iNumFaces;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshDiscEditor::ExpectedCornerChunkSize(void)
{
	int	iNumVerticies;

	iNumVerticies = 1 + miWedgeSegments * miCircleSegments;
	return iNumVerticies;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshDiscEditor::Apply(CMeshEditor* pcMeshEditor)
{
	ReInitConnectivity(pcMeshEditor);
	GenerateDisc(pcMeshEditor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshDiscEditor::GenerateDisc(CMeshEditor* pcMeshEditor)
{
	int			i;
	int			j;
	SFloat3		sDest;
	SFloat3		sSource;
	float		fWedgeRad;
	float		fRadius;
	int			iStartPosition;

	fWedgeRad = (N_PI*2.0f)/miWedgeSegments;

	iStartPosition = pcMeshEditor->NumCorners();

	pcMeshEditor->AddCorner(&msStart);
	for (j = 0; j < miCircleSegments; j++)
	{
		fRadius = (mfOuterRadius / miCircleSegments) * (j+1);
		sSource.Init(0.0f, fRadius, 0.0f);
		for (i = 0; i < miWedgeSegments; i++)
		{
			Float3RotateAboutNormalisedAxis(&sDest, sSource, fWedgeRad * i, msZDirection);
			sDest.Add(&msStart);
			pcMeshEditor->AddCorner(&sDest);
		}
	}

	for (i = 0; i < miWedgeSegments-1; i++)
	{
		pcMeshEditor->AddFace(iStartPosition + 0, iStartPosition + i+1, iStartPosition + i+2, miFaceName, TRUE, TRUE, TRUE, mbFlipFaces);
	}
	pcMeshEditor->AddFace(iStartPosition + 0, iStartPosition + i+1, iStartPosition + 1, miFaceName, TRUE, TRUE, TRUE, mbFlipFaces);

	for (j = 0; j < miCircleSegments-1; j++)
	{
		for (i = 0; i < miWedgeSegments-1; i++)
		{
			pcMeshEditor->AddQuad(iStartPosition + (j*miWedgeSegments) + (i+2), iStartPosition + (j*miWedgeSegments) + (i+1),
				                  iStartPosition + ((j+1)*miWedgeSegments) + (i+1), iStartPosition + ((j+1)*miWedgeSegments) + (i+2), 
								  miFaceName, mbFlipFaces);
		}
		pcMeshEditor->AddQuad(iStartPosition + (j*miWedgeSegments) + (1), iStartPosition + (j*miWedgeSegments) + (i+1),
			                  iStartPosition + ((j+1)*miWedgeSegments) + (i+1), iStartPosition + ((j+1)*miWedgeSegments) + (1), 
			                  miFaceName, mbFlipFaces);
	}
}

