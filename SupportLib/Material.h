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
#include "BaseLib/ChunkFile.h"
#include "StandardLib/NamedObject.h"
#include "MaterialImage.h"
#include "MaterialProperties.h"


class CMaterial : public CNamedObject
{
BASE_FUNCTIONS(CMaterial);
public:

	CMaterialImage		msDiffuse;
	CMaterialImage		msDiffuseLevel;
	CMaterialImage		msSpecular;
	CMaterialImage		msIllumination;
	CMaterialImage		msOpacity;
	CMaterialImage		msBump;
	CMaterialImage		msDetail;
	CMaterialImage		msDecal;
	CMaterialImage		msReflection;
	CMaterialProperties	msProperties;

	void		Init(void);
	void		KillData(void);

	BOOL		Load(CObjectDeserialiser* pcFile);
	BOOL		Save(CObjectSerialiser* pcFile);
	void		Copy(CMaterial* pcMaterial);

	void		Dump(void);
};


#endif //__MATERIAL_H__

