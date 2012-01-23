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
#ifndef __MATERIAL_IMAGE_H__
#define __MATERIAL_IMAGE_H__
#include "BaseLib/ChunkFile.h"


enum EUVAddress
{
	UVA_Tile,
	UVA_Mirror,
	UVA_Clamp,
	UVA_Border,
};


enum EUVFilter
{
	UVF_Nearest,
	UVF_Linear,
	UVF_Anisotropic,
	UVF_Pyramidal,
	UVF_Gaussian,
};


enum EUVSource
{
	UVS_0 = 0,
	UVS_1,
	UVS_2,
	UVS_3,
	UVS_4,
	UVS_5,
	UVS_6,
	UVS_7,
	UVS_Spherical,
	UVS_Cubic,
};


class CMaterialImage
{
public:
	int					iImageID;
	EUVAddress			eAddressU;  
	EUVAddress			eAddressV;
	EUVFilter			eMagFilter;  
	EUVFilter			eMinFilter;
	EUVFilter			eMipFilter;
	EUVSource			eUVSource;

	void Init(void);
	void Kill(void);
	BOOL Save(CFileWriter* pcFile);
	BOOL Load(CFileReader* pcFile);
};


#endif // __MATERIAL_IMAGE_H__

