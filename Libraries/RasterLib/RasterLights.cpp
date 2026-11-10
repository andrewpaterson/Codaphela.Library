/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RasterLib

Codaphela RasterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RasterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RasterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "RasterLights.h"


//The light the rasteriser has always had.
#define RASTER_DEFAULT_AMBIENT	64
#define RASTER_DEFAULT_DIFFUSE	192


//////////////////////////////////////////////////////////////////////////
//
// From over the viewer's shoulder.
//
//////////////////////////////////////////////////////////////////////////
void CRasterLights::Init(void)
{
	SRasterUnitVec3	sTowards;

	masDirectional.Init();
	masPoint.Init();
	miAmbient = RASTER_DEFAULT_AMBIENT;
	sTowards.Init(SRasterUnit::FromRaw(4096), SRasterUnit::FromRaw(6144), SRasterUnit::FromRaw(14336));
	AddDirectional(&sTowards, RASTER_DEFAULT_DIFFUSE, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterLights::Kill(void)
{
	masPoint.Kill();
	masDirectional.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterLights::Clear(void)
{
	miAmbient = 0;
	masDirectional.ReInit();
	masPoint.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterLights::SetAmbient(int32 iAmount)
{
	miAmbient = iAmount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRasterLights::AddDirectional(SRasterUnitVec3* psTowards, int32 iAmount, bool bOnScreen)
{
	SRasterDirectionalLight*	psLight;

	if (masDirectional.NumElements() >= RASTER_MAX_DIRECTIONAL_LIGHTS)
	{
		return false;
	}
	psLight = masDirectional.Add();
	psLight->sTowards = *psTowards;
	psLight->iAmount = iAmount;
	psLight->bOnScreen = bOnScreen;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Where what the light adds is less than a shade even facing it.  Past
// iStart it's iAmount * iStart / distance falling off as the distance, and
// iAmount * (iStart / distance) squared as its square.  A start of less than
// one is one, as nothing falls off from nothing.
//
//////////////////////////////////////////////////////////////////////////
static int64 RasterReach(int32 iAmount, ERasterLightDecay eDecay, int32 iStart, int32 iRange)
{
	int64	iReach;

	if (iAmount <= 0)
	{
		return 0;
	}
	if (iStart < 1)
	{
		iStart = 1;
	}

	if (eDecay == RLD_Linear)
	{
		iReach = (int64)iAmount * iStart + 1;
	}
	else if (eDecay == RLD_Square)
	{
		iReach = (int64)iStart * (RasterSquareRoot((uint64)iAmount) + 1);
	}
	else
	{
		iReach = RASTER_MAX_REACH;
	}

	if ((iRange > 0) && (iRange < iReach))
	{
		iReach = iRange;
	}
	return (iReach < RASTER_MAX_REACH) ? iReach : RASTER_MAX_REACH;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRasterLights::AddPoint(SInt32Vec3* psPosition, int32 iAmount, ERasterLightDecay eDecay, int32 iStart, int32 iRange)
{
	SRasterPointLight*	psLight;

	if (masPoint.NumElements() >= RASTER_MAX_POINT_LIGHTS)
	{
		return false;
	}
	psLight = masPoint.Add();
	psLight->sPosition = *psPosition;
	psLight->iAmount = iAmount;
	psLight->iStart = (iStart < 1) ? 1 : iStart;
	psLight->eDecay = eDecay;
	psLight->iReach = (int32)RasterReach(iAmount, eDecay, iStart, iRange);
	psLight->uiReachSquared = (uint64)psLight->iReach * (uint64)psLight->iReach;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The whole part of the square root, a bit at a time.
//
//////////////////////////////////////////////////////////////////////////
uint32 RasterSquareRoot(uint64 uiValue)
{
	uint64	uiResult;
	uint64	uiBit;

	uiResult = 0;
	uiBit = (uint64)1 << 62;
	while (uiBit > uiValue)
	{
		uiBit >>= 2;
	}
	while (uiBit != 0)
	{
		if (uiValue >= uiResult + uiBit)
		{
			uiValue -= uiResult + uiBit;
			uiResult = (uiResult >> 1) + uiBit;
		}
		else
		{
			uiResult >>= 1;
		}
		uiBit >>= 2;
	}
	return (uint32)uiResult;
}


//////////////////////////////////////////////////////////////////////////
//
// What a point light adds to the shade at psPosition, with psNormal the
// normal there (raw SRasterUnits), both in the space the instances are put
// in.  Nothing from its reach on.
//
//////////////////////////////////////////////////////////////////////////
int32 RasterPointShade(SRasterPointLight* psLight, SInt64Vec3* psNormal, SInt32Vec3* psPosition)
{
	int64	iX;
	int64	iY;
	int64	iZ;
	int64	iReach;
	int64	iDot;
	int64	iLength;
	int64	iShade;
	uint64	uiSquared;

	iReach = psLight->iReach;
	iX = (int64)psLight->sPosition.x - psPosition->x;
	iY = (int64)psLight->sPosition.y - psPosition->y;
	iZ = (int64)psLight->sPosition.z - psPosition->z;
	if ((iX >= iReach) || (iX <= -iReach) || (iY >= iReach) || (iY <= -iReach) || (iZ >= iReach) || (iZ <= -iReach))
	{
		return 0;
	}
	uiSquared = (uint64)(iX * iX) + (uint64)(iY * iY) + (uint64)(iZ * iZ);
	if (uiSquared >= psLight->uiReachSquared)
	{
		return 0;
	}

	iDot = psNormal->x * iX + psNormal->y * iY + psNormal->z * iZ;
	if (iDot <= 0)
	{
		return 0;
	}
	iLength = RasterSquareRoot(uiSquared);
	iShade = ((iDot / iLength) * psLight->iAmount) >> SRasterUnit::Bits;
	if (iLength > psLight->iStart)
	{
		if (psLight->eDecay == RLD_Linear)
		{
			iShade = iShade * psLight->iStart / iLength;
		}
		else if (psLight->eDecay == RLD_Square)
		{
			iShade = iShade * psLight->iStart / iLength;
			iShade = iShade * psLight->iStart / iLength;
		}
	}
	return (int32)iShade;
}

