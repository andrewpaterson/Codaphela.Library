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
#ifndef __MESH_MODIFIER_H__
#define __MESH_MODIFIER_H__
#include "BaseLib/GeometricTypes.h"
#include "StandardLib/Unknown.h"


class CMeshEditor;
class CMeshModifier : public CUnknown
{
BASE_FUNCTIONS(CMeshModifier);
public:
	void			Kill(void);

	virtual void	Apply(CMeshEditor* pcMeshEditor) =0;

	virtual void	ReInitConnectivity(int iCornerChunkSize, int iFaceChunkSize);

	virtual void 	AddPosition(float x, float y, float z);
	virtual void	AddPosition(SFloat3* psPosition);

	virtual void	AddFace(int iCorner1, int iCorner2, int iCorner3);
	virtual void	AddFaces(CArrayInt* paiCorners);  //paiCorners size is a multiple of 3.

	virtual void	RemoveFace(int iFace);
	virtual void	RemoveFaces(CArrayInt* paiFaces);

	virtual void	RemoveCorner(int iCorner);
	virtual void	RemoveCorners(CArrayInt* paiCorners);

	virtual void	MovePosition(int iPosition, SFloat3* psPosition);
	virtual void	MovePositions(CArrayInt* paiPositions, SFloat3* psPosition);
};


#endif // __MESH_MODIFIER_H__

