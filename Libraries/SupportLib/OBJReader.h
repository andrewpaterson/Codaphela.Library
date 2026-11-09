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
#ifndef __OBJ_READER_H__
#define __OBJ_READER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "MeshCornerBuilder.h"
#include "MeshEditorWrapper.h"


enum EWavefrontOBJPrefix
{
	WOBJP_v,
	WOBJP_vt,
	WOBJP_vn,
	WOBJP_f,
};


class CMesh;
class CMeshEditor;
class CTextParser;


bool LoadOBJ(Ptr<CMesh> pMesh, const char* szFilename);
bool LoadOBJ(Ptr<CMeshEditor> pEditor, const char* szFilename);


class CObjReader
{
protected:
	CMeshEditorWrapper	mcWrapper;
	CChars				mszFileName;
	int					miLine;

public:
	void	Init(Ptr<CMesh> pMesh, const char* szFilename);
	void	Init(Ptr<CMeshEditor> pEditor, const char* szFilename);
	void	Kill(void);

	bool	Read(void);

protected:
	bool	ReadLine(CTextParser* pcTextParser);
	bool	ReadPosition(CTextParser* pcTextParser);
	bool	ReadUVCoord(CTextParser* pcTextParser);
	bool	ReadNormal(CTextParser* pcTextParser);
	bool	ReadFace(CTextParser* pcTextParser);
	void	ReadFaceCorners(CTextParser* pcTextParser, CArrayMeshCornerBuilder* pasCorners);
	bool	ResolveFaceCorners(CArrayMeshCornerBuilder* pasCorners);
	void	AddPolygonFaces(CArrayMeshCornerBuilder* pasCorners);
	void	AddPolygonFace(int* aiPolygonCorners, CArrayMeshCornerBuilder* pasCorners);
	void	SetFaceDetails(int iFace, int* aiPolygonCorners, CArrayMeshCornerBuilder* pasCorners);
	bool	Error(const char* szMessage);
};


#endif // __OBJ_READER_H__

