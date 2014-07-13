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
#include "MeshVertexArray.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshVertexArray::Init(SMeshFaceType sFaceType)
{
	msFaceType = sFaceType;
	mcVertexArray.Init();
	mcFaceIndicies.Init(24);
	Change();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshVertexArray::Kill(void)
{
	mcVertexArray.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshVertexArray::Load(CFileReader* pcFile)
{
	ReturnOnFalse(msFaceType.Load(pcFile));
	ReturnOnFalse(mcVertexArray.Load(pcFile));
	ReturnOnFalse(mcFaceIndicies.Read(pcFile));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshVertexArray::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(msFaceType.Save(pcFile));
	ReturnOnFalse(mcVertexArray.Save(pcFile));
	ReturnOnFalse(mcFaceIndicies.Write(pcFile));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshVertexArray::Change(void)
{
	BOOL				bNormal;
	BOOL				bColour; 
	int					iNumberOfTextures;
	int					iNumberOfMatrices;
	int					i;

	bNormal = msFaceType.IsNormal();
	bColour = msFaceType.IsColour();
	iNumberOfTextures = msFaceType.GetUVs();
	iNumberOfMatrices = msFaceType.GetMatricies();

	mcVertexArray.BeginChange();
	mcVertexArray.AddChannel(VERTEX_POSITION_X, VERTEX_POSITION_Y, VERTEX_POSITION_Z, PT_float);

	if (bNormal)
	{
		mcVertexArray.AddChannel(VERTEX_NORMAL_X, VERTEX_NORMAL_Y, VERTEX_NORMAL_Z, PT_float);
	}

	if (bColour)
	{
		mcVertexArray.AddChannel(VERTEX_DIFFUSE_RED, VERTEX_DIFFUSE_GREEN, VERTEX_DIFFUSE_BLUE, VERTEX_OPACITY, PT_uchar);
	}

	for (i = 0; i < iNumberOfTextures; i++)
	{
		mcVertexArray.AddChannel(CHANNEL(VP_UVCoord_(i), CT_U), CHANNEL(VP_UVCoord_(i), CT_U), PT_float);
	}

	for (i = 0; i < iNumberOfMatrices; i++)
	{
		mcVertexArray.AddChannel(CHANNEL(VP_Matrix_(i), CT_Index), PT_int);
		mcVertexArray.AddChannel(CHANNEL(VP_Weight_(i), CT_Intensity), PT_float);
	}

	mcVertexArray.EndChange();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshVertexArray::Is(SMeshFaceType* psFaceType) { return msFaceType.Equals(psFaceType); }
CArrayMeshFaceIndex* CMeshVertexArray::GetFaceIndicies(void) { return &mcFaceIndicies; }
CVertexArray* CMeshVertexArray::GetVertexArray(void) { return &mcVertexArray; }
SMeshFaceType* CMeshVertexArray::GetFaceType(void) { return &msFaceType; }
