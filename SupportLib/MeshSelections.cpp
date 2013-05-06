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
#include "MeshSelections.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelections::Init(void)
{
	mcVerts.Init();
	mcEdges.Init();
	mcFaces.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelections::Kill(void)
{
	mcFaces.Kill();
	mcEdges.Kill();
	mcVerts.Kill();
	CUnknown::Kill();
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshSelections::ReInit(void)
{
	mcVerts.ReInit();
	mcEdges.ReInit();
	mcFaces.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshSelections::Load(CFileReader* pcFile)
{
	ReturnOnFalse(mcVerts.Load(pcFile));
	ReturnOnFalse(mcEdges.Load(pcFile));
	ReturnOnFalse(mcFaces.Load(pcFile));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshSelections::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(mcVerts.Save(pcFile));
	ReturnOnFalse(mcEdges.Save(pcFile));
	ReturnOnFalse(mcFaces.Save(pcFile));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshSelections::GetNumberOfSelectedFaces(BOOL bSelected)
{
	int		i;
	int		iCount;
	BOOL	bCurrent;

	iCount = 0;
	for (i = 0; i < mcFaces.NumElements(); i++)
	{
		bCurrent = FixBool(mcFaces.Get(i));
		if (bCurrent == bSelected)
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
void CMeshSelections::SetFaceSelectionTo(BOOL bSelected)
{
	int		i;

	for (i = 0; i < mcFaces.NumElements(); i++)
	{
		mcFaces.Set(i, bSelected);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelections::ClearFaceSelection(void)
{
	SetFaceSelectionTo(FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshSelections::GetNumberOfSelectedCorners(BOOL bSelected)
{
	int		i;
	int		iCount;
	BOOL	bCurrent;

	iCount = 0;
	for (i = 0; i < mcVerts.NumElements(); i++)
	{
		bCurrent = FixBool(mcVerts.Get(i));
		if (bCurrent == bSelected)
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
void CMeshSelections::SetCornerSelectionTo(BOOL bSelected)
{
	int			i;

	for (i = 0; i < mcVerts.NumElements(); i++)
	{
		mcVerts.Set(i, bSelected);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelections::ClearCornerSelection(void)
{
	SetCornerSelectionTo(FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshSelections::GetNumberOfSelectedEdges(BOOL bSelected)
{
	int		i;
	int		iCount;
	BOOL	bCurrent;

	iCount = 0;
	for (i = 0; i < mcEdges.NumElements(); i++)
	{
		bCurrent = FixBool(mcEdges.Get(i));
		if (bCurrent == bSelected)
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
void CMeshSelections::SetEdgeSelectionTo(BOOL bSelected)
{
	int			i;

	for (i = 0; i < mcEdges.NumElements(); i++)
	{
		mcEdges.Set(i, bSelected);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelections::ClearEdgeSelection(void)
{
	SetEdgeSelectionTo(FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelections::AddCorner(void)
{
	mcVerts.Add(FALSE);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelections::AddFace(int iNewEdges)
{
	int		i;

	mcFaces.Add(FALSE);

	for (i = 0; i < iNewEdges; i++)
	{
		mcEdges.Add(FALSE);
	}
}

