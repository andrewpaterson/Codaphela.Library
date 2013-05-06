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
#ifndef __MESH_LEAVES_H__
#define __MESH_LEAVES_H__
#include "BaseLib/AdditionalTypes.h"
#include "MeshDetail.h"
#include "ShapesIndexed.h"


#define SPHERE_RADIUS_GREATER_THAN	1.1f
#define SPHERE_RADIUS_LESS_THAN		(1.0f/1.1f)
#define SPHERE_INTERSECTION_VOLUME_SAME	0.5f


enum ERelativeSphereVolume
{
	RSV_Unknown,
	RSV_GreaterThan,
	RSV_Equal,
	RSV_LessThan
};


class CRelativeSphereVolume
{
public:
	float					fVolume;
	ERelativeSphereVolume	eRelative[3];  // This ERelativeSphereVolume other.
	int						iNewIndex;

	void					Init(float fVolume);
	BOOL					IsAllGreaterThan(void);
	BOOL					IsAllEqual(void);
	BOOL					IsGreaterThanOrEqual(int iNumGreaterThan);
	BOOL					IsEqual(int iNumEqual);
};


class CMeshLeaf
{
public:
	CSphereIndexed		mcSphere;
	CArraySimpleInt		maiTriangles;

	void	Init(void);
	void	Kill(void);
	BOOL	Save(CFileWriter* pcFile);
	BOOL 	Load(CFileReader* pcFile);
};


typedef CArrayTemplate<CMeshLeaf>	CArrayMeshLeaf;


struct STriangleSphere 
{
	CRelativeSphereVolume	cRelativeVolume;
	CSphere					cSphere;
	SFloat3					sCenter;
};


class CMeshConnectivity;
class CMeshPositions;
class CMeshTriangles;
class CMeshLeaves : public CMeshDetail
{
BASE_FUNCTIONS(CMeshLeaves);
public:
	CArrayMeshLeaf	mcLeaves;

	void 	Init(void);
	void 	Kill(void);
	BOOL	Load(CFileReader* pcFile);
	BOOL	Save(CFileWriter* pcFile);

	void	GenerateLeafSpheres(CMeshConnectivity* pcConn, CMeshPositions* pcPositions, CMeshTriangles* pcTriangles);
	void	SetSphereFromTriangle(STriangleSphere* psSphere, CTriangleIndexed* pcTriangleIndexed, CMeshPositions* pcPositions);
	void	PrivateAddLeafSphere(int iFaceNum, CMeshConnectivity* pcConn, CMeshTriangles* pcTriangles, CMeshPositions* pcPositions, STriangleSphere* pasSpheres);
	void	PrivateAddContainedLeafSpheres(int iCentralFaceNum, int iFaceNum, int iSphereIndex, CMeshConnectivity* pcConn, CMeshTriangles* pcTriangles, CMeshPositions* pcPositions, STriangleSphere* pasSpheres);
	void	MakeLeavesIntoPolygons(void);
};


#endif // __MESH_LEAVES_H__

