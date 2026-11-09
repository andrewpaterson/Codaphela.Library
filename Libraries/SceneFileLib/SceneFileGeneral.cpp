/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SceneFileLib

Codaphela SceneFileLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SceneFileLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SceneFileLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "SceneFileGeneral.h"


const char* const gaszSceneFileMaterialColours[SFMC_NumColours] =
{
	"Diffuse",
	"Ambient",
	"Emissive",
	"Specular",
};


const char* const gaszSceneFileTextures[SFT_NumTextures] =
{
	"DiffuseTexture",
	"DiffuseLevelTexture",
	"SpecularTexture",
	"IlluminationTexture",
	"OpacityTexture",
	"BumpTexture",
	"DetailTexture",
	"DecalTexture",
	"ReflectionTexture",
};

