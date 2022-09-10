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
#ifndef __MESH_UV_COORDS_H__
#define __MESH_UV_COORDS_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/GeometricTypes.h"
#include "MeshDetail.h"


struct SMeshUVFace
{
	int		aiUVs[NUM_FACE_CORNERS];  //If aiUVs[0] == -1 then this face is not textured.
};


typedef CArrayTemplate<SMeshUVFace>	CArrayMeshFaceUV;


class CMeshUVLayer
{
public:
	CArrayFloat2		mcUVs;
	CArrayMeshFaceUV	mcFaces;  //mcFaces.NumElements == mpcMesh->mcFaces.NumElements
	
	void	Init(void);
	void 	Kill(void);
	bool	Load(CFileReader* pcFile);
	bool	Save(CFileWriter* pcFile);
	int		AddFace(int iUV1, int iUV2, int iUV3);
};


typedef CArrayTemplate<CMeshUVLayer>	CArrayMeshUVLayer;


class CMeshConnectivity;
class CMeshUVCoords : public CMeshDetail
{
CONSTRUCTABLE(CMeshUVCoords)
public:
	CArrayMeshUVLayer	mcLayers;

	void			Init(void);
	void			ReInit(void);
	void			Kill(void);
	bool			Load(CFileReader* pcFile);
	bool			Save(CFileWriter* pcFile);

	void			AddFace(void);
	int				AddLayer(CMeshConnectivity* pcConn);
	CMeshUVLayer*	EnsureLayerExists(int iLayer, CMeshConnectivity* pcConn);

	void			IndexFaceCorners(int* paiDest, int iStride, CArrayInt* paiSourceFaces, int iNumLayers);
	int				NumUVs(void);
};


#endif // !__MESH_UV_COORDS_H__

