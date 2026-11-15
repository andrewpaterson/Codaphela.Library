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
#include "BaseLib/FloatHelper.h"
#include "MeshEditor.h"
#include "MeshDiscEditor.h"
#include "MeshRingEditor.h"
#include "MeshConeEditor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConeEditor::Init(SFloat32Vec3* psZDirection, SFloat32Vec3* psStart, float32 fTopRadius, float32 fBottomRadius, float32 fLength, int iWedgeSegments, int iTopCircleSegments, int iBottomCircleSegments, int iRingSegments)
{
	PreInit();

	msZDirection.Init(psZDirection);
	msStart.Init(psStart);
	mfTopRadius = fTopRadius;
	mfBottomRadius = fBottomRadius;
	mfLength = fLength;
	miWedgeSegments = iWedgeSegments;
	miRingSegments = iRingSegments;
	miTopCircleSegments	= iTopCircleSegments;
	miBottomCircleSegments = iBottomCircleSegments;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConeEditor::Class(void)
{
	CMeshSourceModifier::Class();
	U_3Float32(msZDirection);
	U_3Float32(msStart);
	U_Float32(mfTopRadius);
	U_Float32(mfBottomRadius);
	U_Float32(mfLength);
	U_SInt(miWedgeSegments);
	U_SInt(miTopCircleSegments);
	U_SInt(miBottomCircleSegments);
	U_SInt(miRingSegments);
}


//////////////////////////////////////////////////////////////////////////
//
// The editors of its top, its side and its bottom, briefly, for working out
// its sizes and generating it.
//
//////////////////////////////////////////////////////////////////////////
void CMeshConeEditor::InitParts(CMeshDiscEditor* pcTopDisc, CMeshRingEditor* pcRing, CMeshDiscEditor* pcBottomDisc)
{
	SFloat32Vec3	sTopCenter;

	sTopCenter.Init(&msZDirection);
	sTopCenter *= mfLength;
	sTopCenter.Add(&msStart);

	pcTopDisc->Init(&msZDirection, &sTopCenter, 0.0f, mfTopRadius, miWedgeSegments, miTopCircleSegments);
	pcTopDisc->SetFaceName(1);

	pcRing->Init(&msZDirection, &msStart, mfTopRadius, mfBottomRadius, mfLength, miWedgeSegments, miRingSegments);
	pcRing->SetZOffsets(1, 1);
	pcRing->SetFaceName(2);

	pcBottomDisc->Init(&msZDirection, &msStart, 0.0f, mfBottomRadius, miWedgeSegments, miBottomCircleSegments);
	pcBottomDisc->SetFlipFaces(true);
	pcBottomDisc->SetFaceName(3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConeEditor::KillParts(CMeshDiscEditor* pcTopDisc, CMeshRingEditor* pcRing, CMeshDiscEditor* pcBottomDisc)
{
	pcBottomDisc->Kill();
	pcRing->Kill();
	pcTopDisc->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshConeEditor::ExpectedFaceChunkSize(void)
{
	CMeshDiscEditor		cTopDisc;
	CMeshRingEditor		cRing;
	CMeshDiscEditor		cBottomDisc;
	int					iNumFaces;

	InitParts(&cTopDisc, &cRing, &cBottomDisc);
	iNumFaces = cTopDisc.ExpectedFaceChunkSize();
	if (miRingSegments > 2)
	{
		iNumFaces += cRing.ExpectedFaceChunkSize();
		iNumFaces += miWedgeSegments * 2 * 2;
	}
	else
	{
		iNumFaces += miWedgeSegments * 2;
	}
	iNumFaces += cBottomDisc.ExpectedFaceChunkSize();
	KillParts(&cTopDisc, &cRing, &cBottomDisc);
	return iNumFaces;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshConeEditor::ExpectedCornerChunkSize(void)
{
	CMeshDiscEditor		cTopDisc;
	CMeshRingEditor		cRing;
	CMeshDiscEditor		cBottomDisc;
	int					iNumCorners;

	InitParts(&cTopDisc, &cRing, &cBottomDisc);
	iNumCorners = cTopDisc.ExpectedCornerChunkSize();
	iNumCorners += cRing.ExpectedCornerChunkSize();
	iNumCorners += cBottomDisc.ExpectedCornerChunkSize();
	KillParts(&cTopDisc, &cRing, &cBottomDisc);
	return iNumCorners;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConeEditor::Generate(CMeshEditor* pcMeshEditor)
{
	CMeshDiscEditor		cTopDisc;
	CMeshRingEditor		cRing;
	CMeshDiscEditor		cBottomDisc;
	int					iFirstRing;
	int					iLastRing;
	int					iLastTop;
	int					iLastBottom;

	InitParts(&cTopDisc, &cRing, &cBottomDisc);

	cTopDisc.Generate(pcMeshEditor);
	iLastTop = pcMeshEditor->NumCorners() - miWedgeSegments;

	cBottomDisc.Generate(pcMeshEditor);
	iLastBottom = pcMeshEditor->NumCorners() - miWedgeSegments;

	if (miRingSegments > 2)
	{
		iFirstRing = pcMeshEditor->NumCorners();
		cRing.Generate(pcMeshEditor);
		iLastRing = pcMeshEditor->NumCorners() - miWedgeSegments;
		cRing.AddZStrip(pcMeshEditor, iLastRing, iLastTop, true);
		cRing.AddZStrip(pcMeshEditor, iLastBottom, iFirstRing, true);
	}
	else
	{
		cRing.AddZStrip(pcMeshEditor, iLastBottom, iLastTop, true);
	}

	KillParts(&cTopDisc, &cRing, &cBottomDisc);
}
