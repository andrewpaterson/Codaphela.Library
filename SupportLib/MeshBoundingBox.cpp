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
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "StandardLib/ClassDefines.h"
#include "Box.h"
#include "MeshPositions.h"
#include "MeshNormals.h"
#include "MeshBoundingBox.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshBoundingBox::Init(void)
{
	PreInit();

	CMeshDetail::Init();
	mcBox.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshBoundingBox::Free(void)
{
	mcBox.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshBoundingBox::Class(void)
{
	CMeshDetail::Class();

	U_Unknown(CBoxIndexed, mcBox);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshBoundingBox::Load(CObjectReader* pcFile)
{
	ReturnOnFalse(LoadMeshDetail(pcFile));
	return mcBox.Load(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshBoundingBox::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(SaveMeshDetail(pcFile));
	return mcBox.Save(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshBoundingBox::GenerateBoundingBox(CMeshPositions* pcPositions, CMeshNormals* pcNormals)
{
	SFloat3*			psPoints;
	CBox				cBox;
	SFloat3*			psCenter;
	SFloat3*			psXAxis;
	SFloat3*			psYAxis;
	SFloat3*			psZAxis;
	int					iCenterIndex;
	int					iLongAxisIndex;

	psPoints = pcPositions->mcPositions.GetData();

	iCenterIndex = pcPositions->mcPositions.NumElements();
	psCenter = pcPositions->mcPositions.Add();

	iLongAxisIndex = pcNormals->mcNormals.NumElements();
	psXAxis = pcNormals->mcNormals.Add();
	psYAxis = pcNormals->mcNormals.Add();
	psZAxis = pcNormals->mcNormals.Add();

	psCenter->Init(0,0,0);
	cBox.Init(psCenter, psXAxis, psYAxis, psZAxis);

	cBox.SetFromPointsUsingBestFit(psPoints, sizeof(SFloat3), pcPositions->mcPositions.NumElements(), false);

	mcBox.Init(iCenterIndex, iLongAxisIndex, cBox.mfLongLength, iLongAxisIndex+1, cBox.mfMiddleLength, iLongAxisIndex+2, cBox.mfShortLength);
}

