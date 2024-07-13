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


bool LoadOBJ(Ptr<CMesh> pcMesh, char* szFileName);
bool LoadOBJ(Ptr<CMeshEditor> pcMesh, char* szFileName);


class CObjReader
{
public:
	CMeshEditorWrapper	mcWrapper;
	CChars				mszFileName;
	int					miMaterial;
	int					miSmoothingGroup;

	void Init(Ptr<CMesh> pcMesh, char *szFileName);
	void Init(Ptr<CMeshEditor> pcEditor, char *szFileName);
	void Kill(void);

	bool Read(void);
	bool ReadPosition(void);
	bool ReadNormal(void);
	bool ReadUVCoord(void);
	bool ReadFace(CTextParser* pcTextParser, int iNumCorners, int iNumNormals, int iNumUVs);
	bool ReadPosition(CTextParser* pcTextParser);
	bool ReadUVCoord(CTextParser* pcTextParser);
	bool ReadNormal(CTextParser* pcTextParser);
	bool ReadObjectName(CTextParser* pcTextParser);
	bool ReadMaterialLibrary(CTextParser* pcTextParser);
	bool ReadMaterial(CTextParser* pcTextParser);
	bool ReadSmoothingGroup(CTextParser* pcTextParser);
};


#endif // __OBJ_READER_H__

