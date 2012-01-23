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
#ifndef __MESH_RING_EDITOR_H__
#define __MESH_RING_EDITOR_H__
#include "BaseLib/GeometricTypes.h"
#include "MeshSourceModifier.h"


class CMeshRingEditor : public CMeshSourceModifier
{
BASE_FUNCTIONS(CMeshRingEditor);
public:
	SFloat3 	msZDirection;
	SFloat3 	msStart;
	float		mfTopRadius;
	float		mfBottomRadius;
	float		mfLength;
	int			miWedgeSegments;
	int			miRingSegments;
	BOOL		mbFlipFaces;
	int			miFaceName;
	int			miZI;
	int			miZE;

	void	Init(SFloat3* psZDirection, SFloat3* psStart, float fTopRadius, float fBottomRadius, float fLength, int iWedgeSegments, int iRingSegments);
	void	SetFlipFaces(BOOL bFlipFaces);
	void	SetFaceName(int iFaceName);
	void	SetZOffsets(int iZStartOffset, int iZEndOffset);
	void	Kill(void);

	int		ExpectedFaceChunkSize(void);
	int		ExpectedCornerChunkSize(void);

	void	Apply(CMeshEditor* pcMeshEditor);
	void	GenerateRing(CMeshEditor* pcMeshEditor);
	void	AddZStrip(CMeshEditor* pcMeshEditor, int iStartPosition1, int iStartPosition2, BOOL bFlipFaces);
};


#endif // __MESH_RING_EDITOR_H__

