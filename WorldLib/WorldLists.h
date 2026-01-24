/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WorldLib

Codaphela WorldLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WorldLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WorldLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __WORLD_LISTS_H__
#define __WORLD_LISTS_H__
#include "StandardLib/ArrayUnknownTemplate.h"
#include "CameraInstance.h"
#include "LightInstance.h"
#include "GraphicsObject.h"
#include "ParticleSystem.h"
#include "Movement.h"


typedef CArrayUnknownTemplate<CCameraInstance>	CWorldCameraList;
typedef CArrayUnknownTemplate<CLightInstance>	CWorldLightList;
typedef CArrayUnknownTemplate<CParticleSystem>	CParticleSystemList;
typedef CArrayUnknownTemplate<CMovement>			CMovementList;
typedef CArrayUnknownTemplate<CGraphicsViewport>	CWorldViewportList;


#endif //__WORLD_LISTS_H__

