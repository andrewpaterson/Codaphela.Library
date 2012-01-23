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
#ifndef __LIGHT_H__
#define __LIGHT_H__
#include "BaseLib/ChunkFile.h"
#include "BaseLib/GeometricTypes.h"
#include "StandardLib/StandardHeader.h"
#include "StandardLib/TrackerTemplate.h"
#include "ImageColour.h"


enum ELightDecayType
{
	LDT_None,
	LDT_Linear,
	LDT_Square,
};


enum ELightType
{
	LT_Omni,
	LT_Directional,
	LT_Spot
};


class CLight : public CStandardTrackerObject
{
public:
	BASE_FUNCTIONS(CLight);

	ELightType			meType;
	CImageColourARGB	msColour;
	ELightDecayType		meDecay;
	float				mfDecayStart;
	BOOL				mbCastShadows;
	float				mfIntensity;

	void 	Init(void);
	void 	Init(ELightType eType, CImageColourARGB sColour, ELightDecayType eDecay, float fDecayStart, BOOL bCastShadows, float fIntensity);
	void 	Kill(void);

	BOOL	LoadSpecific(CFileReader* pcFile, int iChunkNum);;
	BOOL	Save(CFileWriter* pcFile);
	void	Copy(CLight* pcLight);
};


typedef CTrackerTemplate<CLight> CLightTracker;


#endif // __LIGHT_H__

