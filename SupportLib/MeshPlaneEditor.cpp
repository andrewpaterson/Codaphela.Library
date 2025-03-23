/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "MeshEditor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshPlaneEditor::Init(SFloat3* psXDirection, SFloat3* psYDirection, SFloat3* psStart, int iXSegments, int iYSegments)
{
	msXDirection.Init(psXDirection->x, psXDirection->y, psXDirection->z);
	msYDirection.Init(psYDirection->x, psYDirection->y, psYDirection->z);
	msStart.Init(psStart->x, psStart->y, psStart->z);
	miXSegments = iXSegments;
	miYSegments = iYSegments;
	miXI = 0;
	miXE = 0;
	miYI = 0;
	miYE = 0;
	mbFlipFaces = false;
	miFaceName = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshPlaneEditor::SetXOffsets(int iXStartOffset, int iXEndOffset)
{
	miXI = iXStartOffset;
	miXE = iXEndOffset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshPlaneEditor::SetYOffsets(int iYStartOffset, int iYEndOffset)
{
	miYI = iYStartOffset;
	miYE = iYEndOffset;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshPlaneEditor::SetFlipFaces(bool bFlipFaces)
{
	mbFlipFaces = bFlipFaces;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshPlaneEditor::SetFaceName(int iFaceName)
{
	miFaceName = iFaceName;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshPlaneEditor::Kill(void)
{
	CMeshSourceModifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshPlaneEditor::ExpectedFaceChunkSize(void)
{
	int iNumFaces;

	iNumFaces = miXSegments * miYSegments * 2;
	return iNumFaces;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshPlaneEditor::ExpectedCornerChunkSize(void)
{
	int	iNumVerticies;

	iNumVerticies = (miXSegments+1) * (miYSegments+1);
	return iNumVerticies;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshPlaneEditor::Apply(CMeshEditor* pcMeshEditor)
{
	ClearConnectivity(pcMeshEditor);
	GeneratePlane(pcMeshEditor);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshPlaneEditor::GeneratePlane(CMeshEditor* pcMeshEditor)
{
	int				x;
	int				y;
	int				iNumVerticies;
	int				iNumFaces;
	SFloat3			sXFraction;
	SFloat3			sYFraction;
	int				iBottomLeftCorner;
	int				iBottomRightCorner;
	int				iTopLeftCorner;
	int				iTopRightCorner;
	SFloat3			sPosition;
	int				iStartPosition;

	iNumVerticies = (miXSegments+1 - (miXI+miXE)) * (miYSegments+1 - (miYI+miYE));
	iNumFaces = (miXSegments - (miXI+miXE)) * (miYSegments - (miYI+miYE)) * 2;

	sXFraction = msXDirection;
	sXFraction /= (float)miXSegments;
	sYFraction = msYDirection;
	sYFraction /= (float)miYSegments;

	iStartPosition = pcMeshEditor->NumCorners();
	for (y = miYI; y < miYSegments-miYE+1; y++)
	{
		for (x = miXI; x < miXSegments-miXE+1; x++)
		{
			sPosition = msStart + sXFraction * (float)x + sYFraction * (float)y;
			pcMeshEditor->AddCorner(sPosition.x, sPosition.y, sPosition.z);
		}
	}

	for (y = miYI; y < miYSegments-miYE; y++)
	{
		for (x = miXI; x < miXSegments-miXE; x++)
		{
			iBottomLeftCorner = (x-miXI) + (y-miYI) * (miXSegments-miXI-miXE+1);
			iBottomRightCorner = (x-miXI+1) + (y-miYI) * (miXSegments-miXI-miXE+1);
			iTopLeftCorner = (x-miXI) + (y-miYI+1) * (miXSegments-miXI-miXE+1);
			iTopRightCorner = (x-miXI+1) + (y-miYI+1) * (miXSegments-miXI-miXE+1);

			pcMeshEditor->AddQuad(iStartPosition + iBottomLeftCorner, 
									iStartPosition + iBottomRightCorner, 
									iStartPosition + iTopRightCorner, 
									iStartPosition + iTopLeftCorner, 
									miFaceName,
									mbFlipFaces);
		}
	}
}

