/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MeshLib

Codaphela MeshLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MeshLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MeshLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __MESH_CONNECTIVITY_H__
#define __MESH_CONNECTIVITY_H__
#include "StandardLib/Object.h"
#include "MeshFaceReturn.h"
#include "MeshCorner.h"
#include "MeshEdge.h"
#include "MeshFace.h"
#include "MeshDetail.h"


typedef	CArrayTemplate<CMeshCorner>		CArrayCorner;
typedef CArrayTemplate<CMeshEdge>		CArrayEdge;
typedef CArrayTemplate<CMeshFace>		CArrayFace;


//This is an intermediate struct for generating connectivity.
struct CCornerEdgesMap
{
	//int	iSmallCornerIndex;  //Implied by position in array.
	CArrayIntMinimal		maiLargerCornerIndex;
	CArrayIntMinimal		maiEdgeIndex;  //Same size as maiLargerCornerIndex.  

	void Init(void);
	void Kill(void);
};

typedef	CArrayTemplate<CCornerEdgesMap>		CArrayCornerEdgesMap;


//Positions belongs in here, not separate.
class CMeshConnectivity : public CMeshDetail
{
CONSTRUCTABLE(CMeshConnectivity);
public:
	CArrayCorner			mcCorners;
	CArrayEdge				mcEdges;
	CArrayFace				mcFaces;

	CArrayCornerEdgesMap	mcCornerEdgesMap;

	void 			Init(void);
	void 			Kill(void);
	void 			ReInit(void);
	bool			Load(CFileReader* pcFile);
	bool			Save(CFileWriter* pcFile);

	void			InitCorners(int iNumCorners);
	void			KillCorners(void);
	void			KillCornerEdgesMap(void);
	CMeshCorner*	AddCorner(void);
	CMFRet			AddFace(int iCorner1, int iCorner2, int iCorner3);
	CMeshCorner*	GetCorner(int iElementNum);
	CMeshEdge*		GetEdge(int iElementNum);
	CMeshFace*		GetFace(int iElementNum);
	SAdjFaces		GetAdjacentFaces(int iFaceNum);

	int				GenerateEdgeConnectivity(void);
	int				GenerateEdgeConnectivity(CMeshFace* pcFace, int iFaceNum);
	int				GenerateEdgeConnectivity(CMeshFace* pcFace, int iFaceNum, int iC1, int iC2, int iE);

	void			IndexFaceCorners(int* paiDest, int iStride, CArrayInt* paiSourceFaces);

protected:
	CMeshEdge*		AddEdge(void);
	CMeshFace*		AddFace(void);
};


#endif // !__MESH_CONNECTIVITY_H__

