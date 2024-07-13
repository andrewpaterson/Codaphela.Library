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
#ifndef __MTL_READER_H__
#define __MTL_READER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "MeshEditorWrapper.h"


enum EWavefrontMTLPrefix
{
	WMTLP_Ns,		//Glossiness
	WMTLP_Ni,		//?
	WMTLP_d, 		//Opacity
	WMTLP_Tr,		//Opacity
	WMTLP_Tf,		//Opacity
	WMTLP_illum,	//?
	WMTLP_Ka,		//Ambient RGB
	WMTLP_Kd,		//Diffuse RGB
	WMTLP_Ks,		//Specular RGB
	WMTLP_Ke,		//Emmisive RGB
	WMTLP_map_Ka,	//Ambient RGB Map
	WMTLP_map_Kd,	//Diffuse RGB Map
	WMTLP_map_Ks,	//Specular RGB Map
	WMTLP_map_Ke,	//Emmisive RGB Map
	WMTLP_map_d,	//Opacity Map
	WMTLP_map_bump, //Bump Map
	WMTLP_bump,		//Bump Map
	WMTLP_map_refl, //Environment Map
};


class CMaterial;
class CTextParser;
class CMTLReader
{
public:
	CMaterial*	mpcMaterial;
	CChars		mszFileName;

	void Init(CMaterial* pcMaterial, char *szFileName);
	void Kill(void);

	bool Read(void);
};


#endif // __MTL_READER_H__

