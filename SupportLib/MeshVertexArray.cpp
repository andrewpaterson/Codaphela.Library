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
#include "StandardLib/ClassDefines.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "MeshVertexArray.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshVertexArray::Init(SMeshFaceType sFaceType)
{
	PreInit();

	msFaceType = sFaceType;
	mcVertexArray.Init();
	mcFaceIndicies.Init();

	PostInit();

	Change();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshVertexArray::Free(void)
{
	mcFaceIndicies.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshVertexArray::Class(void)
{
	M_Embedded(mcVertexArray);
	U_Unknown(CArrayMeshFaceIndex, mcFaceIndicies);
	U_Unknown(SMeshFaceType, msFaceType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshVertexArray::Load(CObjectReader* pcFile)
{
	ReturnOnFalse(msFaceType.Load(pcFile));
	ReturnOnFalse(mcVertexArray.Load(pcFile));
	ReturnOnFalse(mcFaceIndicies.Read(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshVertexArray::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(msFaceType.Save(pcFile));
	ReturnOnFalse(mcVertexArray.Save(pcFile));
	ReturnOnFalse(mcFaceIndicies.Write(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshVertexArray::Change(void)
{
	bool	bNormal;
	bool	bColour; 
	int		iNumberOfTextures;
	int		iNumberOfMatrices;
	int		i;

	bNormal = msFaceType.IsNormal();
	bColour = msFaceType.IsColour();
	iNumberOfTextures = msFaceType.GetUVs();
	iNumberOfMatrices = msFaceType.GetMatricies();

	mcVertexArray.BeginChange();
	mcVertexArray.AddChannel(VERTEX_POSITION_X, VERTEX_POSITION_Y, VERTEX_POSITION_Z, PT_float32);

	if (bNormal)
	{
		mcVertexArray.AddChannel(VERTEX_NORMAL_X, VERTEX_NORMAL_Y, VERTEX_NORMAL_Z, PT_float32);
	}

	if (bColour)
	{
		mcVertexArray.AddChannel(VERTEX_DIFFUSE_RED, VERTEX_DIFFUSE_GREEN, VERTEX_DIFFUSE_BLUE, VERTEX_OPACITY, PT_uint8);
	}

	for (i = 0; i < iNumberOfTextures; i++)
	{
		mcVertexArray.AddChannel(CHANNEL(VP_UVCoord_(i), CT_U), CHANNEL(VP_UVCoord_(i), CT_U), PT_float32);
	}

	for (i = 0; i < iNumberOfMatrices; i++)
	{
		mcVertexArray.AddChannel(CHANNEL(VP_Matrix_(i), CT_Index), PT_int32);
		mcVertexArray.AddChannel(CHANNEL(VP_Weight_(i), CT_Intensity), PT_float32);
	}

	mcVertexArray.EndChange();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshVertexArray::Is(SMeshFaceType* psFaceType) { return msFaceType.Equals(psFaceType); }
CArrayMeshFaceIndex* CMeshVertexArray::GetFaceIndicies(void) { return &mcFaceIndicies; }
CVertexArray* CMeshVertexArray::GetVertexArray(void) { return &mcVertexArray; }
SMeshFaceType* CMeshVertexArray::GetFaceType(void) { return &msFaceType; }

