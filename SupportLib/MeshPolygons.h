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
#ifndef __MESH_POLYGONS_H__
#define __MESH_POLYGONS_H__
#include "BaseLib/ChunkFile.h"
#include "BaseLib/ArrayIntMinimal.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Float3.h"
#include "MeshDetail.h"


//A polygon is defined by a group of faces.  
//This class does not enforce that they are contiguous (but does assume it).
class CMeshPolygon
{
public:
	CArrayIntMinimal	maiFaces;  //In polygon
	int					miName;

public:
	void 		Init(int iName);
	void 		Kill(void);
	bool	 	Save(CFileWriter* pcFile);
	bool	 	Load(CFileReader* pcFile);
};

typedef CArrayTemplate<CMeshPolygon> CArrayMeshPolygon;


//Every face in a mesh should be referred to by a polygon (mcPolygons).
//maiFacesToPolygons is the reverse mapping.  Given a face one can get the polygon it's a part of.
class CMeshEdgeVisibility;
class CMeshConnectivity;
class CMeshNormals;
class CMeshPolygons : public CObject
{
CONSTRUCTABLE(CMeshPolygons);
DESTRUCTABLE(CMeshPolygons);
public:
	CArrayMeshPolygon	mcPolygons;
	CArrayInt			maiFacesToPolygons;

	void 			Init(void);
	void 			Free(void);
	void			Class(void);
	void			KillPolygons(void);

	void 			Clear(void);

	bool			Load(CObjectReader* pcFile);
	bool			Save(CObjectWriter* pcFile);

	CMeshPolygon*	Get(int iPolygonNum);
	CMeshPolygon*	Add(int iName);
	CMeshPolygon*	GrowToNumPolygons(int iNumPolygons, int iName);
	int				GetFirstFaceNotInPolygon(CMeshConnectivity* pcConn, int iStartFaceIndex);
	bool			PrivateIsPolygonContiguous(CMeshConnectivity* pcConn, CMeshPolygon* psPolygon, int* aiStack, int* aiTouched);
	bool			IsPolygonContiguous(CMeshConnectivity* pcConn, int iPolygon);
	CMeshPolygon*	GetPolygonForFace(int iFaceNum);
	void			GeneratePolygonsFromEdgeSelection(CMeshConnectivity* pcConn, CMeshEdgeVisibility* pcEdges);
	void			GeneratePolygonFromEdgeSelection(CMeshConnectivity* pcConn, int iFaceIndex, CMeshEdgeVisibility* pcEdges);
	void			AddFace(CMeshFace* pcFace, int iFace, int iName, CMeshConnectivity* pcConn, bool bEdge12, bool bEdge23, bool bEdge31);
	int				NumUniqueNames(void);
	void			GetUniqueNames(CArrayInt* paiNames);
	void			GetNormals(CArrayFloat3* pasNormals, CMeshNormals* pcNormals);
	void			MergePolygon(int iDestPolygon, int iSource1, int iSource2);
	void			MergePolygon(int iDestPolygon, int iSource);
	void			GetAdjacentPolygons(CMeshConnectivity* pcConn, int iPolygon, CArrayInt* paiAdj);
};


#endif // __MESH_POLYGONS_H__

