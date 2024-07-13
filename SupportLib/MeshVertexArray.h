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
#ifndef __MESH_VERTEX_ARRAY_H__
#define __MESH_VERTEX_ARRAY_H__
#include "StandardLib/ArrayType.h"
#include "StandardLib/Array.h"
#include "VertexArray.h"
#include "MeshFaceTypes.h"
#include "MeshIndexArray.h"


//This is conceptually a single draw call.  The faces are indices into the vertex array.
class CMeshVertexArray : public CObject
{
CONSTRUCTABLE(CMeshVertexArray);
DESTRUCTABLE(CMeshVertexArray);
protected:
	SMeshFaceType			msFaceType;
	CVertexArray			mcVertexArray;
	CArrayMeshFaceIndex		mcFaceIndicies;

public:
	void					Init(SMeshFaceType sFaceType);
	void 					Free(void);
	void					Class(void);

	bool					Load(CObjectReader* pcFile);
	bool					Save(CObjectWriter* pcFile);

	bool					Is(SMeshFaceType* psFaceType);
	CArrayMeshFaceIndex*	GetFaceIndicies(void);
	CVertexArray*			GetVertexArray(void);
	SMeshFaceType*			GetFaceType(void);

private:
	void 					Change(void);
};


typedef CArray<CMeshVertexArray>	CMeshVertexArrayArray;


#endif // __MESH_VERTEX_ARRAY_H__

