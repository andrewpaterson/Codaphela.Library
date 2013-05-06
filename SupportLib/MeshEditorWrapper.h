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
#ifndef __MESH_EDITOR_WRAPPER_H__
#define __MESH_EDITOR_WRAPPER_H__
#include "BaseLib/ArrayInt.h"
#include "StandardLib/Unknown.h"
#include "MeshFaceReturn.h"


class CMesh;
class CMeshEditor;
struct SMeshNormalFace;
struct SMeshUVFace;
class CMeshEditorWrapper : public CUnknown
{
BASE_FUNCTIONS(CUnknown);
public:

	//Either/Or.
	CMeshEditor*	mpcEditor;
	CMesh*			mpcMesh;

	void 				Init(CMesh* pcMesh);
	void 				Init(CMeshEditor* pcMesh);
	void				Kill(void);

	void				ReInitConnectivity(int iCornerChunkSize = 0, int iFaceChunkSize = 0);

	void				SetUVsInUse(BOOL bInUse);
	void				SetNormalsInUse(BOOL bInUse);

	CMFRet				AddFace(int iCorner1, int iCorner2, int iCorner3, int iName = 0, BOOL bEdge12 = TRUE, BOOL bEdge23 = TRUE, BOOL bEdge31 = TRUE);
	CMFRet				AddFace(int iCorner1, int iCorner2, int iCorner3, int iName, BOOL bEdge12, BOOL bEdge23, BOOL bEdge31, BOOL bFlipFaces);
	CMFRet				AddQuad(int iCorner1, int iCorner2, int iCorner3, int iCorner4, int iName = 0, BOOL bFlipFaces = FALSE);  //Corners clock-wise.
	CMFRet				AddPolygon(CArrayInt* paiCorners, int iName = 0, BOOL bFlipFaces = FALSE);  //Corners clock-wise.

	void				AddCorner(float x, float y, float z);
	void				AddUV(float u, float v);
	void				AddNormal(float x, float y, float z);

	int					NumFaces(void);
	int					NumCorners(void);
	int					NumNormals(void);
	int					NumUVs(void);

	SMeshNormalFace*	GetFaceNormal(int iFace);
	SMeshUVFace*		GetUVFace(int iFace, int iLayer);
};


#endif // __MESH_EDITOR_WRAPPER_H__

