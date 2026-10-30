/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "BaseLib/IntegerHelper.h"
#include "ColourBlend.h"
#include "ColourARGB32.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 Get8BitAlphaColour(ARGB32 uiColour)
{
	return (uiColour >> RGBA32_ALPHA_SHIFT) & 0xff;
}

uint8 Get8BitBlueColour(ARGB32 uiColour)
{
	return (uiColour >> RGBA32_BLUE_SHIFT) & 0xff;
}

uint8 Get8BitGreenColour(ARGB32 uiColour)
{
	return (uiColour >> RGBA32_GREEN_SHIFT) & 0xff;
}

uint8 Get8BitRedColour(ARGB32 uiColour)
{
	return (uiColour >> RGBA32_RED_SHIFT) & 0xff;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 Clamp8BitColour(uint32 uiColour)
{
	if (uiColour > 0xff)
	{
		return 0xff;
	}
	return uiColour;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ARGB32 Set8BitAlphaColour(uint32 iAlpha)
{
	iAlpha = Clamp8BitColour(iAlpha);
	return iAlpha << RGBA32_ALPHA_SHIFT;
}

ARGB32 Set8BitBlueColour(uint32 iBlue)
{
	iBlue = Clamp8BitColour(iBlue);
	return iBlue << RGBA32_BLUE_SHIFT;
}

ARGB32 Set8BitGreenColour(uint32 iGreen)
{
	iGreen = Clamp8BitColour(iGreen);
	return iGreen << RGBA32_GREEN_SHIFT;
}

ARGB32 Set8BitRedColour(uint32 iRed)
{
	iRed = Clamp8BitColour(iRed);
	return iRed << RGBA32_RED_SHIFT;
}

ARGB32 Set8BitAlphaColour(uint8 iAlpha)
{
	return (ARGB32)iAlpha << RGBA32_ALPHA_SHIFT;
}

ARGB32 Set8BitBlueColour(uint8 iBlue)
{
	return (ARGB32)iBlue << RGBA32_BLUE_SHIFT;
}

ARGB32 Set8BitGreenColour(uint8 iGreen)
{
	return (ARGB32)iGreen << RGBA32_GREEN_SHIFT;
}

ARGB32 Set8BitRedColour(uint8 iRed)
{
	return (ARGB32)iRed << RGBA32_RED_SHIFT;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 Convert8BitColourToFloat(ARGB32 uiColour)
{
	return (float32)uiColour / 255.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 ConvertFloatTo8BitColour(float32 fColour)
{
	return ((uint8)(fColour * 255.0f));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ARGB32 Set32BitColour(uint32 r, uint32 g, uint32 b, uint32 a)
{
	ARGB32		uiColour;

	uiColour = Set8BitRedColour(r);
	uiColour |= Set8BitGreenColour(g);
	uiColour |= Set8BitBlueColour(b);
	uiColour |= Set8BitAlphaColour(a);

	return uiColour;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ARGB32 Set32BitColour(uint8 r, uint8 g, uint8 b, uint8 a)
{
	ARGB32		uiColour;

	uiColour = Set8BitRedColour(r);
	uiColour |= Set8BitGreenColour(g);
	uiColour |= Set8BitBlueColour(b);
	uiColour |= Set8BitAlphaColour(a);

	return uiColour;
}


void Get32BitColour(uint32* r, uint32* g, uint32* b, uint32* a, ARGB32 uiColour)
{
	*r = Get8BitRedColour(uiColour);
	*g = Get8BitGreenColour(uiColour);
	*b = Get8BitBlueColour(uiColour);
	*a = Get8BitAlphaColour(uiColour);
}


void Get32BitColour(float32* r, float32* g, float32* b, float32* a, ARGB32 uiColour)
{
	*r = Convert8BitColourToFloat(Get8BitRedColour(uiColour));
	*g = Convert8BitColourToFloat(Get8BitGreenColour(uiColour));
	*b = Convert8BitColourToFloat(Get8BitBlueColour(uiColour));
	*a = Convert8BitColourToFloat(Get8BitAlphaColour(uiColour));
}


void Get32BitColour(float32* r, float32* g, float32* b, ARGB32 uiColour)
{
	*r = Convert8BitColourToFloat(Get8BitRedColour(uiColour));
	*g = Convert8BitColourToFloat(Get8BitGreenColour(uiColour));
	*b = Convert8BitColourToFloat(Get8BitBlueColour(uiColour));
}


ARGB32 Add32BitColours(ARGB32 uiColour1, ARGB32 uiColour2)
{
	uint32		r1, g1, b1, a1;
	uint32		r2, g2, b2, a2;

	Get32BitColour(&r1, &g1, &b1, &a1, uiColour1);
	Get32BitColour(&r2, &g2, &b2, &a2, uiColour2);
	r1 += r2;
	g1 += g2;
	b1 += b2;
	a1 += a2;

	return Set32BitColour(r1, g1, b1, a1);
}


ARGB32 Subtract32BitColours(ARGB32 uiColour1, ARGB32 uiColour2)
{
	uint32		r1, g1, b1, a1;
	uint32		r2, g2, b2, a2;

	Get32BitColour(&r1, &g1, &b1, &a1, uiColour1);
	Get32BitColour(&r2, &g2, &b2, &a2, uiColour2);
	r1 -= r2;
	g1 -= g2;
	b1 -= b2;
	a1 -= a2;

	return Set32BitColour(r1, g1, b1, a1);
}


ARGB32 Minimum32BitColour(ARGB32 uiColour1, ARGB32 uiColour2)
{
	uint32		r1, g1, b1, a1;
	uint32		r2, g2, b2, a2;

	Get32BitColour(&r1, &g1, &b1, &a1, uiColour1);
	Get32BitColour(&r2, &g2, &b2, &a2, uiColour2);
	if (r1 > r2) r1 = r2;
	if (g1 > g2) g1 = g2;
	if (b1 > b2) b1 = b2;
	if (a1 > a2) a1 = a2;

	return Set32BitColour(r1, g1, b1, a1);
}


ARGB32 Maximum32BitColour(ARGB32 uiColour1, ARGB32 uiColour2)
{
	uint32		r1, g1, b1, a1;
	uint32		r2, g2, b2, a2;

	Get32BitColour(&r1, &g1, &b1, &a1, uiColour1);
	Get32BitColour(&r2, &g2, &b2, &a2, uiColour2);
	if (r1 < r2) r1 = r2;
	if (g1 < g2) g1 = g2;
	if (b1 < b2) b1 = b2;
	if (a1 < a2) a1 = a2;

	return Set32BitColour(r1, g1, b1, a1);
}


ARGB32 Modulate32BitColours(ARGB32 uiColour1, ARGB32 uiColour2)
{
	uint32		r1, g1, b1, a1;
	uint32		r2, g2, b2, a2;

	Get32BitColour(&r1, &g1, &b1, &a1, uiColour1);
	Get32BitColour(&r2, &g2, &b2, &a2, uiColour2);

	r1 = (r1 * r2) / 255;
	g1 = (g1 * g2) / 255;
	b1 = (b1 * b2) / 255;
	a1 = (a1 * a2) / 255;

	return Set32BitColour(r1, g1, b1, a1);
}


ARGB32 Weight32BitColours(ARGB32 uiColour1, ARGB32 uiColour2, float32 fWeight1)
{
	uint32		r1, g1, b1, a1;
	uint32		r2, g2, b2, a2;

	Get32BitColour(&r1, &g1, &b1, &a1, uiColour1);
	Get32BitColour(&r2, &g2, &b2, &a2, uiColour2);

	r1 = (uint32)(r1 * fWeight1 + r2 * (1.0f - fWeight1));
	g1 = (uint32)(g1 * fWeight1 + g2 * (1.0f - fWeight1));
	b1 = (uint32)(b1 * fWeight1 + b2 * (1.0f - fWeight1));
	a1 = (uint32)(a1 * fWeight1 + a2 * (1.0f - fWeight1));

	return Set32BitColour(r1, g1, b1, a1);
}


ARGB32 BlendAlpha32BitColours(ARGB32 uiColour1, ARGB32 uiColour2)
{
	uint32		r1, g1, b1, a1;
	uint32		r2, g2, b2, a2;
	uint8		auiSource[4];
	uint8		auiDest[4];

	Get32BitColour(&r1, &g1, &b1, &a1, uiColour1);
	Get32BitColour(&r2, &g2, &b2, &a2, uiColour2);

	//Colour 1 over colour 2 with straight (not premultiplied) alpha.
	auiSource[0] = (uint8)a1;
	auiSource[1] = (uint8)r1;
	auiSource[2] = (uint8)g1;
	auiSource[3] = (uint8)b1;
	auiDest[0] = (uint8)a2;
	auiDest[1] = (uint8)r2;
	auiDest[2] = (uint8)g2;
	auiDest[3] = (uint8)b2;
	BlendStraightOverPixel(auiDest, auiSource, 256);

	return Set32BitColour((uint32)auiDest[1], (uint32)auiDest[2], (uint32)auiDest[3], (uint32)auiDest[0]);
}


ARGB32 Set32BitColour(float32 r, float32 g, float32 b, float32 a)
{
	uint8	ri, gi, bi, ai;

	ri = ConvertFloatTo8BitColour(r);
	gi = ConvertFloatTo8BitColour(g);
	bi = ConvertFloatTo8BitColour(b);
	ai = ConvertFloatTo8BitColour(a);

	return Set32BitColour(ri, gi, bi, ai);
}


ARGB32 Set32BitColour(uint32 r, uint32 g, uint32 b)
{
	ARGB32		uiColour;

	uiColour = Set8BitRedColour(r);
	uiColour |= Set8BitGreenColour(g);
	uiColour |= Set8BitBlueColour(b);
	uiColour |= (ARGB32)255 << RGBA32_ALPHA_SHIFT;

	return uiColour;
}


ARGB32 Set32BitColour(uint8 r, uint8 g, uint8 b)
{
	ARGB32		uiColour;

	uiColour = Set8BitRedColour(r);
	uiColour |= Set8BitGreenColour(g);
	uiColour |= Set8BitBlueColour(b);
	uiColour |= (ARGB32)255 << RGBA32_ALPHA_SHIFT;

	return uiColour;
}


ARGB32 Set32BitColour(float32 r, float32 g, float32 b)
{
	uint8	ri, gi, bi;

	ri = ConvertFloatTo8BitColour(r);
	gi = ConvertFloatTo8BitColour(g);
	bi = ConvertFloatTo8BitColour(b);

	return Set32BitColour(ri, gi, bi);
}



uint32 ConvertToBGR(ARGB32 uiColour)
{
	SwapByte(&(((uint8*)(&uiColour))[0]), &(((uint8*)(&uiColour))[2]));
	return uiColour;
}


uint32 ConvertToBGRA(ARGB32 uiColour)
{
	SwapByte(&(((uint8*)(&uiColour))[0]), &(((uint8*)(&uiColour))[2]));
	return uiColour;
}


uint8 GetM8BitAlphaColour(ARGB32 uiColour)
{
	return (uiColour >> M_RGBA32_ALPHA_SHIFT) & 0xff;
}

uint8 GetM8BitBlueColour(ARGB32 uiColour)
{
	return (uiColour >> M_RGBA32_BLUE_SHIFT) & 0xff;
}

uint8 GetM8BitGreenColour(ARGB32 uiColour)
{
	return (uiColour >> M_RGBA32_GREEN_SHIFT) & 0xff;
}

uint8 GetM8BitRedColour(ARGB32 uiColour)
{
	return (uiColour >> M_RGBA32_RED_SHIFT) & 0xff;
}

ARGB32 SetM8BitAlphaColour(uint8 iAlpha)
{
	return (ARGB32)iAlpha << M_RGBA32_ALPHA_SHIFT;
}

ARGB32 SetM8BitBlueColour(uint8 iBlue)
{
	return (ARGB32)iBlue << M_RGBA32_BLUE_SHIFT;
}

ARGB32 SetM8BitGreenColour(uint8 iGreen)
{
	return (ARGB32)iGreen << M_RGBA32_GREEN_SHIFT;
}

ARGB32 SetM8BitRedColour(uint8 iRed)
{
	return (ARGB32)iRed << M_RGBA32_RED_SHIFT;
}

ARGB32 SetM32BitColour(uint8 r, uint8 g, uint8 b, uint8 a)
{
	ARGB32		uiColour;

	uiColour = SetM8BitRedColour(r);
	uiColour |= SetM8BitGreenColour(g);
	uiColour |= SetM8BitBlueColour(b);
	uiColour |= SetM8BitAlphaColour(a);

	return uiColour;
}


