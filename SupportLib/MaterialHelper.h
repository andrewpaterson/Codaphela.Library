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
#ifndef __MATERIAL_HELPER_H__
#define __MATERIAL_HELPER_H__
#include "Image.h"
#include "Material.h"


class CMaterialHelper
{
public:
	CMaterial*	mpcMaterial;

	void Init(CMaterial* pcMaterial);
	void Kill(void);
	void Touch(CImageTracker* pcImageTracker);
};


#endif // __MATERIAL_HELPER_H__
