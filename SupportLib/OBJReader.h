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
#ifndef __OBJ_READER_H__
#define __OBJ_READER_H__
#include "BaseLib/Bool.h"
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "MeshEditorWrapper.h"


enum EWavefrontOBJPrefix
{
	WOBJP_v,
	WOBJP_vt,
	WOBJP_vn,
	WOBJP_g,
	WOBJP_f,
	WOBJP_s,
	WOBJP_usemtl,
	WOBJP_mtllib,
};


class CMesh;
class CMeshEditor;
class CTextParser;


BOOL LoadOBJ(CMesh* pcMesh, char* szFileName);
BOOL LoadOBJ(CMeshEditor* pcMesh, char* szFileName);


class CObjReader
{
public:
	CMeshEditorWrapper	mcWrapper;
	CChars				mszFileName;
	int					miMaterial;
	int					miSmoothingGroup;

	void Init(CMesh* pcMesh, char *szFileName);
	void Init(CMeshEditor* pcEditor, char *szFileName);
	void Kill(void);

	BOOL Read(void);
	BOOL ReadPosition(void);
	BOOL ReadNormal(void);
	BOOL ReadUVCoord(void);
	BOOL ReadFace(CTextParser* pcTextParser, int iNumCorners, int iNumNormals, int iNumUVs);
	BOOL ReadPosition(CTextParser* pcTextParser);
	BOOL ReadUVCoord(CTextParser* pcTextParser);
	BOOL ReadNormal(CTextParser* pcTextParser);
	BOOL ReadObjectName(CTextParser* pcTextParser);
	BOOL ReadMaterialLibrary(CTextParser* pcTextParser);
	BOOL ReadMaterial(CTextParser* pcTextParser);
	BOOL ReadSmoothingGroup(CTextParser* pcTextParser);
};


#endif // !__OBJ_READER_H__

