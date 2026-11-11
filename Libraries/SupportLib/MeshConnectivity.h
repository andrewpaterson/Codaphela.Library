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


//Large: a mesh's corners, edges and faces can pass 64KB with a 16 bit size (W65C816).
typedef	CArrayLargeTemplate<CMeshCorner>	CArrayCorner;
typedef CArrayLargeTemplate<CMeshEdge>		CArrayEdge;
typedef CArrayLargeTemplate<CMeshFace>		CArrayFace;


//This is an intermediate struct for generating connectivity.
struct CCornerEdgesMap
{
	//int	iSmallCornerIndex;  //Implied by position in array.
	CArrayEmbeddedInt		maiLargerCornerIndex;
	CArrayEmbeddedInt		maiEdgeIndex;  //Same size as maiLargerCornerIndex.  

	void Init(void);
	void Kill(void);
};

typedef	CArrayLargeTemplate<CCornerEdgesMap>	CArrayCornerEdgesMap;


//Positions belongs in here, not separate.
//Adding a corner or a face only records it.  What touches what (a corner's faces and edges, the edges, a face's edges) is
//generated for the corners and faces added since it was last generated, when something asks for it.  A mesh that is
//only read from its faces' corners never pays for it, and removing a face is never needed: a new mesh is built instead.
class CMeshConnectivity : public CMeshDetail
{
CONSTRUCTABLE(CMeshConnectivity);
DESTRUCTABLE(CMeshConnectivity);
public:
	CArrayFace				mcFaces;  //maiCorner is always valid.  maiEdge isn't until generated, GetFace does that.

protected:
	CArrayCorner			mcCorners;
	CArrayEdge				mcEdges;
	CArrayCornerEdgesMap	mcCornerEdgesMap;

	lsize					muiNumCorners;
	lsize					muiGeneratedFaces;

public:
	void 			Init(void);
	void 			Free(void) override;
	void			Class(void) override;
	
	void 			Clear(void);
	bool			Load(CObjectReader* pcFile) override;
	bool			Save(CObjectWriter* pcFile) override;

	void			AddCorner(void);
	CMFRet			AddFace(int iCorner1, int iCorner2, int iCorner3);  //NewEdges is zero, GenerateConnectivity returns it.

	lsize			NumCorners(void);
	lsize			NumFaces(void);
	int*			GetFaceCorners(int iElementNum);
	lsize			FaceChunkSize(void);
	lsize			CornerChunkSize(void);

	//These generate what they need.
	lsize			NumEdges(void);
	CMeshCorner*	GetCorner(int iElementNum);
	CMeshEdge*		GetEdge(int iElementNum);
	CMeshFace*		GetFace(int iElementNum);
	CCornerEdgesMap*	GetCornerEdgesMap(int iElementNum);
	SAdjFaces		GetAdjacentFaces(int iFaceNum);

	int				GenerateConnectivity(void);  //Returns how many edges were new.
	bool			IsGenerated(void);

protected:
	void			KillCorners(void);
	void			KillCornerEdgesMap(void);
	int				GenerateEdgeConnectivity(CMeshFace* pcFace, int iFaceNum);
	int				GenerateEdgeConnectivity(CMeshFace* pcFace, int iFaceNum, int iC1, int iC2, int iE);
	void			GenerateCornerFaces(CMeshFace* pcFace, int iFaceNum);
	void			GenerateCorners(void);
	CMeshEdge*		AddEdge(void);
	CMeshFace*		AddFace(void);
};


#endif // __MESH_CONNECTIVITY_H__

