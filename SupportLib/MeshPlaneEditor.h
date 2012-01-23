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
#ifndef __MESH_PLANE_EDITOR_H__
#define __MESH_PLANE_EDITOR_H__
#include "BaseLib/GeometricTypes.h"
#include "MeshSourceModifier.h"


class CMeshPlaneEditor : public CMeshSourceModifier
{
BASE_FUNCTIONS(CMeshPlaneEditor);
public:
	SFloat3 	msXDirection;
	SFloat3 	msYDirection; 
	SFloat3 	msStart;
	int			miXSegments;
	int			miYSegments;
	int			miXI;
	int			miXE;
	int			miYI;
	int			miYE;
	BOOL		mbFlipFaces;
	int			miFaceName;

	void	Init(SFloat3* psXDirection, SFloat3* psYDirection, SFloat3* psStart, int iXSegments, int iYSegments);
	void	SetXOffsets(int iXStartOffset, int iXEndOffset);
	void	SetYOffsets(int iYStartOffset, int iYEndOffset);
	void	SetFlipFaces(BOOL bFlipFaces);
	void	SetFaceName(int iFaceName);
	void	Kill(void);

	int		ExpectedFaceChunkSize(void);
	int		ExpectedCornerChunkSize(void);

	void	Apply(CMeshEditor* pcMeshEditor);
	void	GeneratePlane(CMeshEditor* pcMeshEditor);
};


#endif // __MESH_PLANE_EDITOR_H__

