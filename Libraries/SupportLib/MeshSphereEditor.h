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
#ifndef __MESH_SPHERE_EDITOR_H__
#define __MESH_SPHERE_EDITOR_H__
#include "BaseLib/GeometricTypes.h"
#include "MeshSourceModifier.h"


//A sphere of iRingSegments bands from the pole against psZDirection to the pole along it, and iWedgeSegments wedges
//around it.  The bands at the poles are triangles, the others quads.
class CMeshSphereEditor : public CMeshSourceModifier
{
CONSTRUCTABLE(CMeshSphereEditor);
public:
	SFloat32Vec3	msZDirection;
	SFloat32Vec3	msCentre;
	float32			mfRadius;
	int				miWedgeSegments;
	int				miRingSegments;
	bool			mbFlipFaces;
	int				miFaceName;

	void	Init(SFloat32Vec3* psZDirection, SFloat32Vec3* psCentre, float32 fRadius, int iWedgeSegments, int iRingSegments);
	void	SetFlipFaces(bool bFlipFaces);
	void	SetFaceName(int iFaceName);
	void	Kill(void);

	int		ExpectedFaceChunkSize(void);
	int		ExpectedCornerChunkSize(void);

	void	Generate(CMeshEditor* pcMeshEditor) override;
};


#endif // __MESH_SPHERE_EDITOR_H__

