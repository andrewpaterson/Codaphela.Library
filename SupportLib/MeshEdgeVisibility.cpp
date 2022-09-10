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
#include "MeshDefines.h"
#include "MeshConnectivity.h"
#include "MeshPolygons.h"
#include "MeshEdgeVisibility.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEdgeVisibility::Init(void)
{
	mcEdges.Init();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEdgeVisibility::Kill(void)
{
	mcEdges.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEdgeVisibility::ReInit(void)
{
	mcEdges.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshEdgeVisibility::Load(CFileReader* pcFile)
{
	ReturnOnFalse(mcEdges.Read(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshEdgeVisibility::Save(CFileWriter* pcFile)
{
	return mcEdges.Write(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEdgeVisibility::AddFace(CMeshFace* pcFace, int iNewEdges, char cEdge12, char cEdge23, char cEdge31)
{
	int		i;
	char*	pcValue;

	for (i = 0; i < iNewEdges; i++)
	{
		mcEdges.Add(0);
	}
	
	pcValue = mcEdges.Get(pcFace->asEdge[0]);
	if (*pcValue < cEdge12)
	{
		*pcValue = cEdge12;
	}

	pcValue = mcEdges.Get(pcFace->asEdge[1]);
	if (*pcValue < cEdge23)
	{
		*pcValue = cEdge23;
	}

	pcValue = mcEdges.Get(pcFace->asEdge[2]);
	if (*pcValue < cEdge31)
	{
		*pcValue = cEdge31;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CMeshEdgeVisibility::GetEdge(int iEdge)
{
	return *mcEdges.Get(iEdge);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshEdgeVisibility::NumVisibleEdges(char cEdge)
{
	int		i;
	int		iCount;

	iCount = 0;
	for (i = 0; i < mcEdges.NumElements(); i++)
	{
		if (mcEdges.GetValue(i) >= cEdge)
		{
			iCount++;
		}
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEdgeVisibility::GenerateEdgeSelectionFromPolygons(CMeshConnectivity* pcConn, CMeshPolygons* pcPolygons)
{
	int				i;
	int				iPolygon;
	CMeshPolygon*	psPolygon;
	int				iFaceIndex;
	int				j;
	CMeshFace*		psFace;
	int				iEdgeIndex;
	int				iAdjFaceIndex;
	int				iAdjPolygon;
	char*			pcEdge;
	SAdjFaces		sAdjFaces;

	pcConn->mcEdges.Zero();

	//I'm not sure how this used to work.
	pcEdge = NULL;

	for (iPolygon = 0; iPolygon < pcPolygons->mcPolygons.NumElements(); iPolygon++)
	{
		psPolygon = pcPolygons->mcPolygons.Get(iPolygon);
		for (i = 0; i < psPolygon->maiFaces.NumElements(); i++)
		{
			iFaceIndex = psPolygon->maiFaces.GetValue(i);
			psFace = pcConn->GetFace(iFaceIndex);
			sAdjFaces = pcConn->GetAdjacentFaces(iFaceIndex);
			for (j = 0; j < NUM_FACE_EDGES; j++)
			{
				iAdjFaceIndex = sAdjFaces.aiFace[j];
				if (iAdjFaceIndex == -1)
				{
					*pcEdge = 1;
				}
				else
				{
					iEdgeIndex = psFace->asEdge[j];
					pcEdge = mcEdges.Get(iEdgeIndex);

					iAdjPolygon = pcPolygons->maiFacesToPolygons.GetValue(iAdjFaceIndex);
					if (iAdjPolygon != iPolygon)
					{
						*pcEdge = 1;
					}
				}
			}
		}
	}
}

