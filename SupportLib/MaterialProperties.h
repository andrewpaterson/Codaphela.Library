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
#ifndef __MATERIAL_PROPERTIES_H__
#define __MATERIAL_PROPERTIES_H__
#include "BaseLib/ChunkFile.h"
#include "ImageColour.h"


enum EMaterialOpacity
{
	MO_Opaque,
	MO_Translucent,
	MO_Transparent,
	MO_Unknown,
};


class CMaterialProperties
{
public:
	CImageColourARGB	sDiffuseColour;
	CImageColourARGB	sAmbientColour;
	CImageColourARGB	sEmissiveColour;
	CImageColourARGB	sSpecularColour;
	float				fShininessPower;
	float				fShininessBase;
	BOOL				bLit;
	EMaterialOpacity	meOpacity;

	void Init(void);
	void Kill(void);
	BOOL Save(CFileWriter* pcFile);
	BOOL Load(CFileReader* pcFile);
};


#endif // __MATERIAL_PROPERTIES_H__
