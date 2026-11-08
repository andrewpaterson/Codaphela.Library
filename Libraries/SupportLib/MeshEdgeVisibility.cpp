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
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "MeshDefines.h"
#include "MeshEdgeVisibility.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEdgeVisibility::Init(void)
{
	PreInit();

	mcEdges.Init();

	PostInit();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEdgeVisibility::Free(void)
{
	mcEdges.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEdgeVisibility::Class(void)
{
	U_Unknown(CArrayChar, mcEdges);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEdgeVisibility::Clear(void)
{
	mcEdges.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshEdgeVisibility::Load(CObjectReader* pcFile)
{
	return mcEdges.Read(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshEdgeVisibility::Save(CObjectWriter* pcFile)
{
	return mcEdges.Write(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEdgeVisibility::AddFace(CMeshFace* pcFace, char cEdge12, char cEdge23, char cEdge31)
{
	int		i;
	char*	pcValue;

	//The mesh numbers the edges, and it may have some from faces that were added to it directly.
	for (i = 0; i < NUM_FACE_EDGES; i++)
	{
		while ((int)mcEdges.NumElements() <= pcFace->maiEdge[i])
		{
			mcEdges.Add(0);
		}
	}

	pcValue = mcEdges.Get(pcFace->maiEdge[0]);
	if (*pcValue < cEdge12)
	{
		*pcValue = cEdge12;
	}

	pcValue = mcEdges.Get(pcFace->maiEdge[1]);
	if (*pcValue < cEdge23)
	{
		*pcValue = cEdge23;
	}

	pcValue = mcEdges.Get(pcFace->maiEdge[2]);
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
size CMeshEdgeVisibility::NumVisibleEdges(char cEdge)
{
	size	i;
	size	iCount;
	size	uiNumElements;

	iCount = 0;
	uiNumElements = mcEdges.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		if (mcEdges.GetValue(i) >= cEdge)
		{
			iCount++;
		}
	}
	return iCount;
}
