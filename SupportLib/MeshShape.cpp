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
#include "StandardLib/ClassDefines.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "Mesh.h"
#include "MeshShape.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshShape::Init(Ptr<CMesh> pcMesh)
{
	PreInit();

	mpcMesh = pcMesh;
	mcTriangles.Init();
	mcLeaves.Init();
	mcBoundingBox.Init();
	mcHull.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshShape::Free(void)
{
	mcTriangles.Kill();
	mcLeaves.Kill();
	mcBoundingBox.Kill();
	mcHull.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshShape::Class(void)
{
	M_Pointer(mpcMesh);
	M_Embedded(mcTriangles);
	M_Embedded(mcLeaves);
	M_Embedded(mcBoundingBox);
	M_Embedded(mcHull);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshShape::Load(CObjectReader* pcFile)
{
	ReturnOnFalse(mcTriangles.Load(pcFile));
	ReturnOnFalse(mcLeaves.Load(pcFile));
	ReturnOnFalse(mcBoundingBox.Load(pcFile));
	ReturnOnFalse(mcHull.Load(pcFile));
	return true;
}


//////////////////////////////////1////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshShape::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(mcTriangles.Save(pcFile));
	ReturnOnFalse(mcLeaves.Save(pcFile));
	ReturnOnFalse(mcBoundingBox.Save(pcFile));
	ReturnOnFalse(mcHull.Save(pcFile));
	return true;
}

