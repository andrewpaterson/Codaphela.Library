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
#include "ConvexHull.h"
#include "MeshPositions.h"
#include "MeshNormals.h"
#include "ShapesIndexed.h"
#include "MeshConvexHull.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConvexHull::Init(void)
{
	CMeshDetail::Init();
	mcHull.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConvexHull::Kill(void)
{
	mcHull.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshConvexHull::Load(CFileReader* pcFile)
{
	ReturnOnFalse(LoadMeshDetail(pcFile));
	return mcHull.Load(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshConvexHull::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(SaveMeshDetail(pcFile));
	return mcHull.Save(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConvexHull::GenerateConvexHull(CMeshPositions* pcPositions, CMeshNormals* pcNormals)
{
	CConvexHull				cHull;
	CConvexHullGenerator	cGenerator;
	SFloat3*				asPoint;
	int						iFirstNormal;
	SFloat3*				asNormal;
	int						i;
	CPolygon*				pcPolygon;
	CPolygonIndexed*		pcPolygonIndexed;
	int						iNumPolygons;
	int						j;
	int						iNumVerts;
	SFloat3*				psPosition;
	uint32					iPositionIndex;
	int						iNormalIndex;

	asPoint = pcPositions->mcPositions.GetData();

	cHull.Init();
	cHull.BeginSetFromPoints(&cGenerator, asPoint, sizeof(SFloat3), pcPositions->mcPositions.NumElements(), NULL);

	iFirstNormal = pcNormals->mcNormals.AddNum(cGenerator.mcNormals.NumElements());
	asNormal = pcNormals->mcNormals.Get(iFirstNormal);
	cHull.EndSetFromPoints(asNormal, sizeof(SFloat3), &cGenerator);

	iNumPolygons = cHull.mcPolygons.NumElements();

	mcHull.Kill();
	mcHull.Init(iNumPolygons);

	for (i = 0; i < iNumPolygons; i++)
	{
		pcPolygon = cHull.mcPolygons.Get(i);
		pcPolygonIndexed = mcHull.mcPolygons.Get(i);

		iNumVerts = pcPolygon->mapsPositions.NumElements();
		pcPolygonIndexed->maiPositions.Resize(iNumVerts);
		for (j = 0; j < iNumVerts; j++)
		{
			psPosition = *((SFloat3**)pcPolygon->mapsPositions.Get(j));
			iPositionIndex = pcPositions->mcPositions.GetIndex(psPosition);
			pcPolygonIndexed->maiPositions.Set(j, &iPositionIndex);
		}
		pcPolygonIndexed->mbConvex = pcPolygon->mbConvex;
		iNormalIndex = pcNormals->mcNormals.GetIndex(pcPolygon->mpsNormal);
		pcPolygonIndexed->miNormalIndex = iNormalIndex;
	}

	cHull.Kill();
}

