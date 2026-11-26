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
#ifndef __MESH_SMOOTH_GROUPS_H__
#define __MESH_SMOOTH_GROUPS_H__
#include "BaseLib/ArrayUint32.h"
#include "MeshModifier.h"


enum ESmoothGenerationStyle
{
	SGS_None,
	SGS_Name,
	SGS_Angle,
};


class CMesh;


//Makes a mesh's normals from its smoothing groups (CMesh::mcSmoothing).  With SGS_None it keeps the mesh's groups, and a mesh with none has
//every face put in muiAddGroup.  SGS_Name makes them from its faces' names, one group for each name.  SGS_Angle makes them from the
//angles between faces: faces joined across edges within mfSharpAngle degrees of each other share a group, which differs from those of
//the faces they meet at sharper edges.
class CMeshSmoothGroups : public CMeshModifier
{
CONSTRUCTABLE(CMeshSmoothGroups);
DESTRUCTABLE(CMeshSmoothGroups);
public:
	uint32					muiAddGroup;
	ESmoothGenerationStyle	meGenerationStyle;
	float32					mfSharpAngle;

	void		Init(uint32 uiAddGroup = 1);
	void		Class(void) override;

	Ptr<CMesh>	Modify(Ptr<CMesh> pMesh) override;

	static void	GenerateNormals(CMesh* pcMesh);

protected:
	void		GenerateGroups(CMesh* pcMesh);
	void		GenerateGroupsFromNames(CMesh* pcMesh);
	void		GenerateGroupsFromAngles(CMesh* pcMesh);
	void		FindAngleRegions(CMesh* pcMesh, float32 fDot, CArrayLargeFloat32Vec3* pasPolygonNormals, CArrayInt* paiRegions, CArrayInt* paiOrder, CArrayInt* paiRegionStarts);
	uint32		ChooseRegionGroup(CMesh* pcMesh, int iRegion, CArrayInt* paiRegions, CArrayInt* paiOrder, CArrayInt* paiRegionStarts, CArrayUint32* paiRegionGroups);
};


#endif // __MESH_SMOOTH_GROUPS_H__

