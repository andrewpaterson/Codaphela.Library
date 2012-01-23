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
#include "MeshDiscEditor.h"
#include "MeshRingEditor.h"
#include "MeshConeEditor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConeEditor::Init(SFloat3* psZDirection, SFloat3* psStart, float fTopRadius, float fBottomRadius, float fLength, int iWedgeSegments, int iTopCircleSegments, int iBottomCircleSegments, int iRingSegments)
{
	Float3Assign(&msZDirection, psZDirection);
	Float3Assign(&msStart, psStart);
	mfTopRadius = fTopRadius;
	mfBottomRadius = fBottomRadius;
	mfLength = fLength;
	miWedgeSegments = iWedgeSegments;
	miRingSegments = iRingSegments;
	miTopCircleSegments	= iTopCircleSegments;
	miBottomCircleSegments = iBottomCircleSegments;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConeEditor::Kill(void)
{
	CMeshSourceModifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshConeEditor::ExpectedFaceChunkSize(void)
{
	int iNumFaces;

	iNumFaces = mcTopDisc.ExpectedFaceChunkSize();
	if (miRingSegments > 2)
	{
		iNumFaces += mcRing.ExpectedFaceChunkSize();
		iNumFaces += miWedgeSegments * 2 * 2;
	}
	else
	{
		iNumFaces += miWedgeSegments * 2;
	}
	iNumFaces += mcBottomDisc.ExpectedFaceChunkSize();

	return iNumFaces;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshConeEditor::ExpectedCornerChunkSize(void)
{
	int	iNumVerticies;

	iNumVerticies = mcTopDisc.ExpectedCornerChunkSize();
	iNumVerticies += mcRing.ExpectedCornerChunkSize();
	iNumVerticies += mcBottomDisc.ExpectedCornerChunkSize();

	return iNumVerticies;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConeEditor::Apply(CMeshEditor* pcMeshEditor)
{
	SFloat3				sTopCenter;
	int					iFirstTop;
	int					iFirstRing;
	int					iFirstBottom;
	int					iLastRing;
	int					iLastTop;
	int					iLastBottom;

	Float3Assign(&sTopCenter, &msZDirection);
	sTopCenter *= mfLength;
	sTopCenter.Add(&msStart);

	mcTopDisc.Init(&msZDirection, &sTopCenter, 0.0f, mfTopRadius, miWedgeSegments, miTopCircleSegments);
	mcTopDisc.SetFaceName(1);

	mcRing.Init(&msZDirection, &msStart, mfBottomRadius, mfTopRadius, mfLength, miWedgeSegments, miRingSegments);
	mcRing.SetZOffsets(1, 1);
	mcRing.SetFaceName(2);

	mcBottomDisc.Init(&msZDirection, &msStart, 0.0f, mfBottomRadius, miWedgeSegments, miBottomCircleSegments);
	mcBottomDisc.SetFlipFaces(TRUE);
	mcBottomDisc.SetFaceName(3);
	
	ReInitConnectivity(pcMeshEditor);

	iFirstTop = 0;
	mcTopDisc.GenerateDisc(pcMeshEditor);
	iLastTop = pcMeshEditor->NumCorners() - miWedgeSegments;

	iFirstBottom = pcMeshEditor->NumCorners();
	mcBottomDisc.GenerateDisc(pcMeshEditor);
	iLastBottom = pcMeshEditor->NumCorners() - miWedgeSegments;

	if (miRingSegments > 2)
	{
		iFirstRing = pcMeshEditor->NumCorners();
		mcRing.GenerateRing(pcMeshEditor);
		iLastRing = pcMeshEditor->NumCorners() - miWedgeSegments;

		mcRing.AddZStrip(pcMeshEditor, iLastRing, iLastTop, TRUE);
		mcRing.AddZStrip(pcMeshEditor, iLastBottom, iFirstRing, TRUE);
	}
	else
	{
		mcRing.AddZStrip(pcMeshEditor, iLastBottom, iLastTop, TRUE);
	}

	mcTopDisc.Kill();
	mcRing.Kill();
	mcBottomDisc.Kill();
}

