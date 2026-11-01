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
#include "BaseLib/IntegerHelper.h"
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
	lsize	i;
	size	iCount;
	bool	bCurrent;
	lsize	uiNumElements;

	iCount = 0;
	uiNumElements = mcFaces.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		bCurrent = FixBool(mcFaces.Get(i));
		if (bCurrent == bSelected)
		{
			iCount++;
		}
	}
	return (int)iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelections::SetFaceSelectionTo(bool bSelected)
{
	lsize	i;
	lsize	uiNumElements;

	uiNumElements = mcFaces.NumElements();
	for (i = 0; i < uiNumElements; i++)
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
	lsize	i;
	size	iCount;
	bool	bCurrent;
	lsize	uiNumElements;

	iCount = 0;
	uiNumElements = mcVerts.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		bCurrent = FixBool(mcVerts.Get(i));
		if (bCurrent == bSelected)
		{
			iCount++;
		}
	}
	return (int)iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelections::SetCornerSelectionTo(bool bSelected)
{
	lsize	i;
	lsize	uiNumElements;

	uiNumElements = mcVerts.NumElements();
	for (i = 0; i < uiNumElements; i++)
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
	lsize	i;
	size	iCount;
	bool	bCurrent;
	lsize	uiNumElements;

	iCount = 0;
	uiNumElements = mcEdges.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		bCurrent = FixBool(mcEdges.Get(i));
		if (bCurrent == bSelected)
		{
			iCount++;
		}
	}
	return (int)iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSelections::SetEdgeSelectionTo(bool bSelected)
{
	lsize	i;
	lsize	uiNumElements;

	uiNumElements = mcEdges.NumElements();
	for (i = 0; i < uiNumElements; i++)
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
	size	i;

	mcFaces.Add(false);

	for (i = 0; i < (size)iNewEdges; i++)
	{
		mcEdges.Add(false);
	}
}

