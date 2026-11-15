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
#ifndef __MESH_TORUS_EDITOR_H__
#define __MESH_TORUS_EDITOR_H__
#include "BaseLib/GeometricTypes.h"
#include "MeshSourceModifier.h"


//A ring doughnut around psZDirection through psCentre.  fRadius is out to the middle of its tube and fTubeRadius is
//the tube's.  iWedgeSegments go around psZDirection and iTubeSegments around the tube.
class CMeshTorusEditor : public CMeshSourceModifier
{
CONSTRUCTABLE(CMeshTorusEditor);
DESTRUCTABLE(CMeshTorusEditor);
public:
	SFloat32Vec3	msZDirection;
	SFloat32Vec3	msCentre;
	float32			mfRadius;
	float32			mfTubeRadius;
	int				miWedgeSegments;
	int				miTubeSegments;
	bool			mbFlipFaces;
	int				miFaceName;

	void	Init(SFloat32Vec3* psZDirection, SFloat32Vec3* psCentre, float32 fRadius, float32 fTubeRadius, int iWedgeSegments, int iTubeSegments);
	void	SetFlipFaces(bool bFlipFaces);
	void	SetFaceName(int iFaceName);
	void	Class(void) override;

	int		ExpectedFaceChunkSize(void);
	int		ExpectedCornerChunkSize(void);

	void	Generate(CMeshEditor* pcMeshEditor) override;
};


#endif // __MESH_TORUS_EDITOR_H__

