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
#ifndef __RASTER_LIGHTS_H__
#define __RASTER_LIGHTS_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Int32Vec3.h"
#include "BaseLib/Int64Vec3.h"
#include "RasterMesh.h"


//Shade is 0 .. RASTER_MAX_SHADE.  What the lights add up to is clamped at it.
#define RASTER_MAX_SHADE				256

//The most lights a frame uses.  There should only ever be one or two far away.  A point light's place in the frame's list is a
//bit in a mask of those that reach an instance.
#define RASTER_MAX_DIRECTIONAL_LIGHTS	8
#define RASTER_MAX_POINT_LIGHTS			64

//The furthest a point light reaches, so that what's squared fits.
#define RASTER_MAX_REACH				(1 << 30)


//How a point light falls off past where it starts to: not at all, as the distance, or as its square.
enum ERasterLightDecay
{
	RLD_None,
	RLD_Linear,
	RLD_Square,
};


//Towards a light that's far away.  On the screen (x right, y up, z out of it) it moves with the eye, as the light the rasteriser
//starts with does.  Otherwise it's in the space the instances are put in.  sTowards is a unit.
struct SRasterDirectionalLight
{
	SRasterUnitVec3	sTowards;
	int32			iAmount;  //Of RASTER_MAX_SHADE.
	bool			bOnScreen;
};


//A light somewhere in the space the instances are put in, shining every way.  Past iStart its amount falls off as eDecay says,
//and from iReach on what it adds is less than a shade, so it adds nothing.  uiReachSquared is that squared, which is what's
//compared.
struct SRasterPointLight
{
	SInt32Vec3			sPosition;
	int32				iAmount;
	int32				iStart;
	ERasterLightDecay	eDecay;
	int32				iReach;
	uint64				uiReachSquared;
};


typedef CArrayTemplate<SRasterDirectionalLight>	CArrayRasterDirectionalLight;
typedef CArrayTemplate<SRasterPointLight>		CArrayRasterPointLight;


//What CRasteriser shades with: an ambient amount, lights far away and point lights, all grey.  Their amounts add up.  Init
//and Default give the light it has always had: 64 ambient and 192 from over the viewer's shoulder.  Clear leaves no light at
//all.
//
//A point light's range, if it has one, is as far as it reaches.  A light with more than RASTER_MAX_ lights of its kind isn't
//added, and Add returns false.
class CRasterLights
{
public:
	int32							miAmbient;
	CArrayRasterDirectionalLight	masDirectional;
	CArrayRasterPointLight			masPoint;

	void	Init(void);
	void	Kill(void);

	void	Default(void);
	void	Clear(void);
	void	SetAmbient(int32 iAmount);
	bool	AddDirectional(SRasterUnitVec3* psTowards, int32 iAmount, bool bOnScreen);
	bool	AddPoint(SInt32Vec3* psPosition, int32 iAmount, ERasterLightDecay eDecay, int32 iStart, int32 iRange);
};


uint32	RasterSquareRoot(uint64 uiValue);
int32	RasterPointShade(SRasterPointLight* psLight, SInt64Vec3* psNormal, SInt32Vec3* psPosition);


#endif // __RASTER_LIGHTS_H__

