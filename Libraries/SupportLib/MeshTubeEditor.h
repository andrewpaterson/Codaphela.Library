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
#ifndef __MESH_TUBE_EDITOR_H__
#define __MESH_TUBE_EDITOR_H__
#include "BaseLib/GeometricTypes.h"
#include "MeshSourceModifier.h"


//A pipe from psStart fLength along psZDirection, with walls from fInnerRadius out to fOuterRadius.  Like
//CMeshConeEditor's, its walls have iRingSegments rings of corners from end to end, and its ends iCircleSegments bands
//from wall to wall.  Its faces are named 1 on the top, 2 outside, 3 on the bottom and 4 inside.
class CMeshTubeEditor : public CMeshSourceModifier
{
CONSTRUCTABLE(CMeshTubeEditor);
public:
	SFloat32Vec3	msZDirection;
	SFloat32Vec3	msStart;
	float32			mfInnerRadius;
	float32			mfOuterRadius;
	float32			mfLength;
	int				miWedgeSegments;
	int				miCircleSegments;
	int				miRingSegments;
	bool			mbFlipFaces;

	void	Init(SFloat32Vec3* psZDirection, SFloat32Vec3* psStart, float32 fInnerRadius, float32 fOuterRadius, float32 fLength, int iWedgeSegments, int iCircleSegments, int iRingSegments);
	void	SetFlipFaces(bool bFlipFaces);
	void	Kill(void);

	int		ExpectedFaceChunkSize(void);
	int		ExpectedCornerChunkSize(void);

	void	Generate(CMeshEditor* pcMeshEditor) override;

protected:
	int		NumProfilePoints(void);
};


#endif // __MESH_TUBE_EDITOR_H__

