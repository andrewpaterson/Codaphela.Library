/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MeshLib

Codaphela MeshLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MeshLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MeshLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __MATERIAL_H__
#define __MATERIAL_H__
#include "BaseLib/Chars.h"
#include "StandardLib/Array.h"
#include "StandardLib/Object.h"
#include "MaterialImage.h"
#include "MaterialProperties.h"


//In the order a scene file numbers them.
enum EMaterialImage
{
	MI_Diffuse,
	MI_DiffuseLevel,
	MI_Specular,
	MI_Illumination,
	MI_Opacity,
	MI_Bump,
	MI_Detail,
	MI_Decal,
	MI_Reflection,

	MI_NumImages,
};


class CMaterial : public CObject
{
CONSTRUCTABLE(CMaterial);
DESTRUCTABLE(CMaterial);
public:
	CChars				mszName;
	CMaterialProperties	msProperties;

protected:
	CMaterialImage		msDiffuse;
	CMaterialImage		msDiffuseLevel;
	CMaterialImage		msSpecular;
	CMaterialImage		msIllumination;
	CMaterialImage		msOpacity;
	CMaterialImage		msBump;
	CMaterialImage		msDetail;
	CMaterialImage		msDecal;
	CMaterialImage		msReflection;

public:
	void			Init(void);
	void			Free(void) override;
	void			Class(void) override;

	CMaterialImage*	GetImage(EMaterialImage eImage);
};


typedef CArray<CMaterial>	CArrayMaterial;


#endif //__MATERIAL_H__

