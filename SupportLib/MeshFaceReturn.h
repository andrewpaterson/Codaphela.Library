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
#ifndef __MESH_FACE_RETURN_H__
#define __MESH_FACE_RETURN_H__
#include "BaseLib/PrimitiveTypes.h"


class CMFRet
{
public:
	int				miFirstFace;
	uint8	manCorners[4];

	void	PackEmpty(void);
	void	Pack(int iFirstFace, int iCorner1, int iCorner2, int iCorner3, int iNewEdges);
	void	Pack(int iFirstFace, int iCorner1, int iCorner2, int iCorner3, int iCorner4, int iCorner5, int iCorner6, int iNewEdges);
	void	Pack2(CMFRet r2);
	void	Repack(int iCorner1, int iCorner2, int iCorner3);
	void	Repack(int iCorner1, int iCorner2, int iCorner3, int iCorner4, int iCorner5, int iCorner6);
	void	PackOverflow(void);

	int		NumFaces(void);  //A value of 3 implies that this structure has overflowed.  Result is elsewhere.
	int		NewEdges(void);

	void	Unpack1(int* piCorner1, int* piCorner2, int* piCorner3);
	void	Unpack2(int* piCorner4, int* piCorner5, int* piCorner6);
};


#endif // __MESH_FACE_RETURN_H__

