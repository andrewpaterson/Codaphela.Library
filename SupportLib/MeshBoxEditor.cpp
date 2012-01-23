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
#include "Mesh.h"
#include "MeshDetail.h"
#include "MeshCorner.h"
#include "MeshPlaneEditor.h"
#include "MeshBoxEditor.h"
#include "MeshEditor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshBoxEditor::Init(SFloat3* psXDirection, SFloat3* psYDirection, SFloat3* psZDirection, SFloat3* psStart, int iXSegments, int iYSegments, int iZSegments)
{
	msXDirection.Init(psXDirection->x, psXDirection->y, psXDirection->z);
	msYDirection.Init(psYDirection->x, psYDirection->y, psYDirection->z);
	msZDirection.Init(psZDirection->x, psZDirection->y, psZDirection->z);
	msStart.Init(psStart->x, psStart->y, psStart->z);
	miXSegments = iXSegments;
	miYSegments = iYSegments;
	miZSegments = iZSegments;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshBoxEditor::Kill(void)
{
	CMeshSourceModifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshBoxEditor::ExpectedFaceChunkSize(void)
{
	int iNumFaces;

	//Top
	iNumFaces = miXSegments * miYSegments * 2;

	//Bottom
	iNumFaces *= 2;

	//Left
	iNumFaces += miXSegments * miZSegments * 2;

	//Right
	iNumFaces += miXSegments * miZSegments * 2;

	//Front
	iNumFaces += miYSegments * miZSegments * 2;

	//Back
	iNumFaces += miYSegments * miZSegments * 2;
	
	return iNumFaces;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshBoxEditor::ExpectedCornerChunkSize(void)
{
	int	iNumVerticies;

	//Top
	iNumVerticies = (miXSegments+1) * (miYSegments+1);

	//Bottom
	iNumVerticies *= 2;

	//Left
	iNumVerticies += (miXSegments+1) * (miZSegments-1);

	//Right
	iNumVerticies += (miXSegments+1) * (miZSegments-1);

	//Front
	iNumVerticies += (miYSegments-1) * (miZSegments-1);

	//Back
	iNumVerticies += (miYSegments-1) * (miZSegments-1);

	return iNumVerticies;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshBoxEditor::Apply(CMeshEditor* pcMeshEditor)
{
	CMeshPlaneEditor	cPlaneEditor;
	SFloat3				sBottom;
	SFloat3				sRight;
	SFloat3				sBack;

	int					iFirstTop;
	int					iFirstBottom;
	int					iFirstLeft;
	int					iFirstRight;
	int					iFirstFront;
	int					iFirstBack;

	int					iIndex1;
	int					iIndex2;
	int					iIndex3;
	int					iIndex4;
	int					iIndex5;
	int					iIndex6;
	int					iIndex7;
	int					iIndex8;
	int					iIndex9;
	int					iIndex10;
	int					iIndex11;
	int					iIndex12;
	CMesh*				pcMesh;

	int					iNameTop = 1;
	int					iNameBottom = 2;
	int					iNameLeft = 3;
	int					iNameRight = 4;
	int					iNameFront = 5;
	int					iNameBack = 6;

	ReInitConnectivity(pcMeshEditor);

	pcMesh = pcMeshEditor->mpcMesh;

	//Top
	iFirstTop = 0;
	cPlaneEditor.Init(&msXDirection, &msYDirection, &msStart, miXSegments, miYSegments);
	cPlaneEditor.SetFlipFaces(TRUE);
	cPlaneEditor.SetFaceName(iNameTop);
	cPlaneEditor.GeneratePlane(pcMeshEditor);
	cPlaneEditor.Kill();

	//Bottom
	iFirstBottom = pcMeshEditor->NumCorners();
	sBottom = msStart;
	sBottom.Add(&msZDirection);

	cPlaneEditor.Init(&msXDirection, &msYDirection, &sBottom, miXSegments, miYSegments);
	cPlaneEditor.SetFaceName(iNameBottom);
	cPlaneEditor.GeneratePlane(pcMeshEditor);
	cPlaneEditor.Kill();

	if (miZSegments > 1)
	{
		//Left
		iFirstLeft = pcMeshEditor->NumCorners();
		cPlaneEditor.Init(&msXDirection, &msZDirection, &msStart, miXSegments, miZSegments);
		cPlaneEditor.SetFaceName(iNameLeft);
		cPlaneEditor.SetYOffsets(1, 1);
		cPlaneEditor.GeneratePlane(pcMeshEditor);
		cPlaneEditor.Kill();

		//Right
		iFirstRight = pcMeshEditor->NumCorners();
		sRight = msStart;
		sRight.Add(&msYDirection);

		cPlaneEditor.Init(&msXDirection, &msZDirection, &sRight, miXSegments, miZSegments);
		cPlaneEditor.SetFaceName(iNameRight);
		cPlaneEditor.SetYOffsets(1, 1);
		cPlaneEditor.SetFlipFaces(TRUE);
		cPlaneEditor.GeneratePlane(pcMeshEditor);
		cPlaneEditor.Kill();
	}
	if ((miYSegments > 1) && (miZSegments > 1))
	{
		//Front
		iFirstFront = pcMeshEditor->NumCorners();
		cPlaneEditor.Init(&msYDirection, &msZDirection, &msStart, miYSegments, miZSegments);
		cPlaneEditor.SetFaceName(iNameFront);
		cPlaneEditor.SetFlipFaces(TRUE);
		cPlaneEditor.SetYOffsets(1, 1);
		cPlaneEditor.SetXOffsets(1, 1);
		cPlaneEditor.GeneratePlane(pcMeshEditor);
		cPlaneEditor.Kill();

		//Back
		iFirstBack = pcMeshEditor->NumCorners();
		sBack = msStart;
		sBack.Add(&msXDirection);

		cPlaneEditor.Init(&msYDirection, &msZDirection, &sBack, miYSegments, miZSegments);
		cPlaneEditor.SetFaceName(iNameBack);
		cPlaneEditor.SetYOffsets(1, 1);
		cPlaneEditor.SetXOffsets(1, 1);
		cPlaneEditor.GeneratePlane(pcMeshEditor);
		cPlaneEditor.Kill();
	}
	iIndex1 = iFirstTop + ((miXSegments+1) * (miYSegments));
	iIndex2 = iFirstBottom + ((miXSegments+1) * (miYSegments));
	if (miZSegments > 1)
	{
		iIndex7 = iFirstLeft + ((miXSegments+1) * (miZSegments-2));
		iIndex10 = iFirstRight + ((miXSegments+1) * (miZSegments-2));

		AddXStrip(pcMeshEditor, iFirstTop, iFirstLeft, iFirstBottom, iIndex7, FALSE, iNameLeft);
		AddXStrip(pcMeshEditor, iIndex1, iFirstRight, iIndex2, iIndex10, TRUE, iNameRight);
	}
	iIndex3 = iFirstTop + miXSegments;
	iIndex4 = iFirstBottom + miXSegments;
	if ((miYSegments > 1) && (miZSegments > 1))
	{
		iIndex5 = iFirstLeft + miXSegments;
		iIndex6 = iFirstRight + miXSegments;
		iIndex8 = iFirstFront + ((miYSegments-1) * (miZSegments-2));
		iIndex11 = iFirstBack + ((miYSegments-1) * (miZSegments-2));

		AddYStrip(pcMeshEditor, iFirstTop, iFirstFront, iFirstBottom, iIndex8, FALSE, iNameFront);
		AddYStrip(pcMeshEditor, iIndex3, iFirstBack, iIndex4, iIndex11, TRUE, iNameBack);

		iIndex9 = iFirstFront + miYSegments-2;
		iIndex12 = iFirstBack + miYSegments-2;

		AddZStrip(pcMeshEditor, iFirstLeft, iFirstFront, iFirstRight, iIndex9, FALSE, iNameFront);
		AddZStrip(pcMeshEditor, iIndex5, iFirstBack, iIndex6, iIndex12, TRUE, iNameBack);

		pcMeshEditor->AddQuad(iFirstTop, iFirstLeft, iFirstFront, iFirstTop + miXSegments+1, iNameFront);
		pcMeshEditor->AddQuad(iFirstRight, iIndex1, iFirstTop + (miXSegments+1) * (miYSegments-1), iFirstFront + miYSegments-2, iNameFront);
		pcMeshEditor->AddQuad(iIndex7, iFirstBottom, iFirstBottom + miXSegments+1, iIndex8, iNameFront);
		pcMeshEditor->AddQuad(iIndex2, iIndex10, iFirstFront + ((miYSegments-1) * (miZSegments-1))-1, iFirstBottom + ((miXSegments+1) * (miYSegments-1)), iNameFront);

		pcMeshEditor->AddQuad(iFirstTop + miXSegments, iFirstLeft + miXSegments, iFirstBack, iFirstTop + miXSegments+1 + miXSegments, iNameBack, TRUE);
		pcMeshEditor->AddQuad(iFirstRight + miXSegments, iIndex1 + miXSegments, iFirstTop + (miXSegments+1) * (miYSegments-1) + miXSegments, iFirstBack + miYSegments-2, iNameBack, TRUE);
		pcMeshEditor->AddQuad(iIndex7 + miXSegments, iFirstBottom + miXSegments, iFirstBottom + miXSegments+1 + miXSegments, iIndex11, iNameBack, TRUE);
		pcMeshEditor->AddQuad(iIndex2 + miXSegments, iIndex10 + miXSegments, iFirstBack + ((miYSegments-1) * (miZSegments-1))-1, iFirstBottom + ((miXSegments+1) * (miYSegments-1)) + miXSegments, iNameBack, TRUE);
	}
	if ((miZSegments > 1) && (miYSegments == 1))
	{
		iIndex5 = iFirstLeft + miXSegments;
		iIndex6 = iFirstRight + miXSegments;

		AddZStrip(pcMeshEditor, iFirstLeft, iFirstRight, FALSE, iNameFront);
		AddZStrip(pcMeshEditor, iIndex5, iIndex6, TRUE, iNameBack);

		iIndex7 = iFirstLeft + ((miXSegments+1) * (miZSegments-2));
		iIndex10 = iFirstRight + ((miXSegments+1) * (miZSegments-2));

		pcMeshEditor->AddQuad(iFirstLeft, iFirstRight, iFirstTop + miXSegments+1, iFirstTop, iNameFront);
		pcMeshEditor->AddQuad(iFirstBottom, iFirstBottom + miXSegments+1, iIndex10, iIndex7, iNameFront);

		pcMeshEditor->AddQuad(iFirstLeft + miXSegments, iFirstRight + miXSegments, iFirstTop + miXSegments+1 + miXSegments, iFirstTop + miXSegments, iNameBack, TRUE);
		pcMeshEditor->AddQuad(iFirstBottom + miXSegments, iFirstBottom + miXSegments+1 + miXSegments, iIndex10 + miXSegments, iIndex7 + miXSegments, iNameBack, TRUE);
	}
	if (miZSegments == 1)
	{
		AddYStrip(pcMeshEditor, iFirstTop, iFirstBottom, TRUE, iNameFront);
		AddYStrip(pcMeshEditor, iIndex3, iIndex4, FALSE, iNameBack);

		AddXStrip(pcMeshEditor, iFirstTop, iFirstBottom, FALSE, iNameLeft);
		AddXStrip(pcMeshEditor, iIndex1, iIndex2, TRUE, iNameRight);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshBoxEditor::AddXStrip(CMeshEditor* pcMeshEditor, int iIndex1, int iIndex2, int iIndex3, int iIndex4, BOOL bFlipFaces, int iName)
{
	int	i;

	for (i = 0; i < miXSegments; i++)
	{
		pcMeshEditor->AddQuad(iIndex1 + i, iIndex1+1 + i, iIndex2+1 + i, iIndex2 + i, iName, bFlipFaces);
	}

	for (i = 0; i < miXSegments; i++)
	{
		pcMeshEditor->AddQuad(iIndex4 + i, iIndex4+1 + i, iIndex3+1 + i, iIndex3 + i, iName, bFlipFaces);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshBoxEditor::AddXStrip(CMeshEditor* pcMeshEditor, int iIndex1, int iIndex3, BOOL bFlipFaces, int iName)
{
	int	i;

	for (i = 0; i < miXSegments; i++)
	{
		pcMeshEditor->AddQuad(iIndex1 + i, iIndex1+1 + i, iIndex3+1 + i, iIndex3 + i, iName, bFlipFaces);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshBoxEditor::AddYStrip(CMeshEditor* pcMeshEditor, int iIndex1, int iIndex2, int iIndex3, int iIndex4, BOOL bFlipFaces, int iName)
{
	int	i;

	for (i = 1; i < miYSegments-1; i++)
	{
		pcMeshEditor->AddQuad(iIndex1 + (i) * (miXSegments+1), iIndex2 + i-1, iIndex2 + i, iIndex1 + (i + 1) * (miXSegments+1), iName, bFlipFaces);
	}

	for (i = 1; i < miYSegments-1; i++)
	{
		pcMeshEditor->AddQuad(iIndex4 + i-1, iIndex3 + (i) * (miXSegments+1), iIndex3 + (i + 1) * (miXSegments+1), iIndex4 + i, iName, bFlipFaces);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshBoxEditor::AddYStrip(CMeshEditor* pcMeshEditor, int iIndex1, int iIndex3, BOOL bFlipFaces, int iName)
{
	int	i;

	for (i = 0; i < miYSegments; i++)
	{
		pcMeshEditor->AddQuad(iIndex1 + (i) * (miXSegments+1), iIndex1 + (i + 1) * (miXSegments+1), iIndex3 + (i + 1) * (miXSegments+1), iIndex3 + (i) * (miXSegments+1), iName, bFlipFaces);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshBoxEditor::AddZStrip(CMeshEditor* pcMeshEditor, int iIndex1, int iIndex2, int iIndex3, int iIndex4, BOOL bFlipFaces, int iName)
{
	int	i;

	for (i = 1; i < miZSegments-1; i++)
	{
		pcMeshEditor->AddQuad(iIndex1 + (i-1) * (miXSegments+1), iIndex1 + (i) * (miXSegments+1), iIndex2 + (i) * (miYSegments-1), iIndex2 + (i-1) * (miYSegments-1), iName, bFlipFaces);
	}

	for (i = 1; i < miZSegments-1; i++)
	{
		pcMeshEditor->AddQuad(iIndex3 + (i) * (miXSegments+1), iIndex3 + (i-1) * (miXSegments+1), iIndex4 + (i-1) * (miYSegments-1), iIndex4 + i * (miYSegments-1), iName, bFlipFaces);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshBoxEditor::AddZStrip(CMeshEditor* pcMeshEditor, int iIndex1, int iIndex3, BOOL bFlipFaces, int iName)
{
	int	i;

	for (i = 1; i < miZSegments-1; i++)
	{
		pcMeshEditor->AddQuad(iIndex1 + (i-1) * (miXSegments+1), iIndex1 + (i) * (miXSegments+1), iIndex3 + (i) * (miXSegments+1), iIndex3 + (i-1) * (miXSegments+1), iName, bFlipFaces);
	}
}
