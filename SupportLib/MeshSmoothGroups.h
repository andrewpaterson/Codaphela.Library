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
#ifndef __MESH_SMOOTH_GROUPS_H__
#define __MESH_SMOOTH_GROUPS_H__
#include "BaseLib/ArrayInt.h"
#include "MeshModifier.h"


enum ESmoothGenerationStyle
{
	SGS_None,
	SGS_Name,
	SGS_Angle,
};


class CMesh;
class CMeshSmoothGroups : public CMeshModifier
{
BASE_FUNCTIONS(CMeshSmoothGroups);
public:
	CArrayInt				mcSmoothingGroups;  //mcSmoothingGroups.NumElements == mpcMesh->mcFaces.NumElements
	int						miAddGroup;
	ESmoothGenerationStyle	meGenerationStyle;
	float					mfSharpAngle;

	void 	Init(int iAddGroup = 1);
	void 	Kill(void);
	BOOL	Load(CFileReader* pcFile);
	BOOL	Save(CFileWriter* pcFile);

	void	GenerateNormals(CMesh* pcMesh);
	void	GenerateSmoothing(CMeshEditor* pcMeshEditor);
	void	GenerateSmoothingFromNames(CMeshEditor* pcMeshEditor);
	void	GenerateSmoothingFromAngles(CMeshEditor* pcMeshEditor);

	void	Apply(CMeshEditor* pcMeshEditor);

	void	ReInitConnectivity(int iCornerChunkSize, int iFaceChunkSize);

	void	AddFace(int iCorner1, int iCorner2, int iCorner3);

	void	RemoveFace(int iFace);
	void	RemoveFaces(CArrayInt* paiFaces);

	void	SetAllGroups(int iGroup);
};


#endif // __MESH_SMOOTH_GROUPS_H__

