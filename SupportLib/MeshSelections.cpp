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
#include "BaseLib/IntegerHelper.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "MeshSelections.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelections::Init(void)
{
	PreInit();

	mcVerts.Init();
	mcEdges.Init();
	mcFaces.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelections::Free(void)
{
	mcFaces.Kill();
	mcEdges.Kill();
	mcVerts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelections::Class(void)
{
	U_Unknown(CArrayBit, mcVerts);
	U_Unknown(CArrayBit, mcEdges);
	U_Unknown(CArrayBit, mcFaces);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshSelections::Clear(void)
{
	mcVerts.ReInit();
	mcEdges.ReInit();
	mcFaces.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshSelections::Load(CObjectReader* pcFile)
{
	ReturnOnFalse(mcVerts.Read(pcFile));
	ReturnOnFalse(mcEdges.Read(pcFile));
	ReturnOnFalse(mcFaces.Read(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshSelections::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(mcVerts.Write(pcFile));
	ReturnOnFalse(mcEdges.Write(pcFile));
	ReturnOnFalse(mcFaces.Write(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshSelections::GetNumberOfSelectedFaces(bool bSelected)
{
	size	i;
	size	iCount;
	bool	bCurrent;

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
void CMeshSelections::SetFaceSelectionTo(bool bSelected)
{
	size	i;

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
	SetFaceSelectionTo(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshSelections::GetNumberOfSelectedCorners(bool bSelected)
{
	size	i;
	size	iCount;
	bool	bCurrent;

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
void CMeshSelections::SetCornerSelectionTo(bool bSelected)
{
	size	i;

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
	SetCornerSelectionTo(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshSelections::GetNumberOfSelectedEdges(bool bSelected)
{
	size	i;
	size	iCount;
	bool	bCurrent;

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
void CMeshSelections::SetEdgeSelectionTo(bool bSelected)
{
	size	i;

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
	SetEdgeSelectionTo(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelections::AddCorner(void)
{
	mcVerts.Add(false);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelections::AddFace(int iNewEdges)
{
	int		i;

	mcFaces.Add(false);

	for (i = 0; i < iNewEdges; i++)
	{
		mcEdges.Add(false);
	}
}

