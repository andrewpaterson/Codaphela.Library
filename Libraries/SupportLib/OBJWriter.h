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
#ifndef __OBJ_WRITER_H__
#define __OBJ_WRITER_H__
#include "Mesh.h"


class CMeshEditor;


//A mesh on its own is written as triangles.  An editor knows its mesh's polygons, and writes each as one face where that
//can say what its triangles do: it is one loop with no corners inside it, and its triangles agree on each corner's UV and
//normal.  Any other is written as its triangles.
bool SaveOBJ(CMesh* pcMesh, const char* szFilename);
bool SaveOBJ(CMeshEditor* pcEditor, const char* szFilename);


#endif // __OBJ_WRITER_H__

