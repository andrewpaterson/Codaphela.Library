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

** ------------------------------------------------------------------------ **/
#ifndef __MESH_SMOOTHING_H__
#define __MESH_SMOOTHING_H__
#include "BaseLib/ArrayUint32.h"
#include "MeshDetail.h"


//A mesh's smoothing groups: the bits of the groups each face is in.  Faces that share a group share their normals where they meet, and a face
//in none is flat.  A face added while it's in use is in none.  Normals are made from them by CMeshSmoothGroups.
class CMeshSmoothing : public CMeshDetail
{
CONSTRUCTABLE(CMeshSmoothing);
DESTRUCTABLE(CMeshSmoothing);
public:
	CArrayUint32	mauiGroups;  //mauiGroups.NumElements == the mesh's faces, when it's in use.

	void 	Init(void);
	void	Class(void) override;
	void 	Free(void) override;

	void	Clear(void);
	void	Copy(CMeshSmoothing* pcSource);

	bool	Load(CObjectReader* pcFile) override;
	bool	Save(CObjectWriter* pcFile) override;

	void	AddFace(void);
	void	SetAll(size uiNumFaces, uint32 uiGroups);
};


#endif // __MESH_SMOOTHING_H__

