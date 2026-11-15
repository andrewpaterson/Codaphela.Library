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
#ifndef __MESH_GEOSPHERE_EDITOR_H__
#define __MESH_GEOSPHERE_EDITOR_H__
#include "BaseLib/GeometricTypes.h"
#include "MeshSourceModifier.h"


#define GEOSPHERE_ICOSAHEDRON_CORNERS	12
#define GEOSPHERE_ICOSAHEDRON_EDGES		30
#define GEOSPHERE_ICOSAHEDRON_FACES		20


//A sphere of triangles nearly all the same size: an icosahedron, with a corner on each pole along psZDirection, whose
//edges are each cut into iSegments and whose corners are then pushed out to fRadius.  It has 20 * iSegments^2 faces,
//each a polygon of its own.
class CMeshGeosphereEditor : public CMeshSourceModifier
{
CONSTRUCTABLE(CMeshGeosphereEditor);
public:
	SFloat32Vec3	msZDirection;
	SFloat32Vec3	msCentre;
	float32			mfRadius;
	int				miSegments;
	bool			mbFlipFaces;
	int				miFaceName;

	void	Init(SFloat32Vec3* psZDirection, SFloat32Vec3* psCentre, float32 fRadius, int iSegments);
	void	SetFlipFaces(bool bFlipFaces);
	void	SetFaceName(int iFaceName);
	void	Kill(void);

	int		ExpectedFaceChunkSize(void);
	int		ExpectedCornerChunkSize(void);

	void	Generate(CMeshEditor* pcMeshEditor) override;

protected:
	void	IcosahedronCorners(SFloat32Vec3* asCorners);
	int		EdgeCorner(int aaiEdges[GEOSPHERE_ICOSAHEDRON_EDGES][2], int iFrom, int iTo, int iStep);
	void	AddCorner(CMeshEditor* pcMeshEditor, SFloat32Vec3* asDirections, int iCorner, SFloat32Vec3* psDirection);
	void	AddOutwardFace(CMeshEditor* pcMeshEditor, SFloat32Vec3* asDirections, int iStartPosition, int iCorner1, int iCorner2, int iCorner3);
};


#endif // __MESH_GEOSPHERE_EDITOR_H__

