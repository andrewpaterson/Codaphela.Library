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
#ifndef __MESH_MODIFIER_H__
#define __MESH_MODIFIER_H__
#include "BaseLib/GeometricTypes.h"
#include "StandardLib/Unknown.h"
#include "Mesh.h"


//A modifier leaves the mesh it is given as it was and returns the result (see CMeshModifierStack::GetOutput).
class CMeshEditor;
class CMeshModifier : public CUnknown
{
CONSTRUCTABLE(CMeshModifier);
public:
	void			Kill(void);

	virtual Ptr<CMesh>	Modify(Ptr<CMesh> pMesh);  //Returns pMesh itself if there is nothing to change.


protected:
	bool			CanRebuild(CMesh* pcMesh);  //For a Modify that builds a mesh of other faces, which only keeps positions, normals and colours.
	Ptr<CMesh>		CopyWithNormals(CMesh* pcMesh);
	Ptr<CMesh>		BuildFromFaces(CMesh* pcMesh, SFloat32Vec3* pasCorners, int iNumCorners, int* paiFaceCorners);
};


#endif // __MESH_MODIFIER_H__

