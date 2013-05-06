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
#ifndef __MESH_DETAIL_H__
#define __MESH_DETAIL_H__
#include "StandardLib/Unknown.h"


#define MESH_DETAIL_POSITION		0x01
#define MESH_DETAIL_NORMAL			0x02
#define MESH_DETAIL_UVCOORDS		0x04
#define MESH_DETAIL_MATERIALS		0x08
#define MESH_DETAIL_COLOURS			0x10
#define MESH_DETAIL_SKIN			0x20
#define MESH_DETAIL_CONNECTIVITY	0x40


class CMeshDetail : public CUnknown
{
BASE_FUNCTIONS(CMeshDetail);
public:
	BOOL	mbInUse;

	void	Init(BOOL bInUse = FALSE);
	BOOL	InUse(void);

	BOOL	LoadMeshDetail(CFileReader* pcFile);
	BOOL	SaveMeshDetail(CFileWriter* pcFile);
};


#endif // __MESH_DETAIL_H__


