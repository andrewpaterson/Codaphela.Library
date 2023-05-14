/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "ColourARGB.h"


int Get8BitAlphaColour(int iColour)
{
	return (iColour >> RGBA32_ALPHA_SHIFT) & 0xff;
}

int Get8BitBlueColour(int iColour)
{
	return (iColour >> RGBA32_BLUE_SHIFT) & 0xff;
}

int Get8BitGreenColour(int iColour)
{
	return (iColour >> RGBA32_GREEN_SHIFT) & 0xff;
}

int Get8BitRedColour(int iColour)
{
	return (iColour >> RGBA32_RED_SHIFT) & 0xff;
}


int Clamp8BitColour(int iColour)
{
	if (iColour > 0xff)
	{
		return 0xff;
	}
	else if (iColour < 0x00)
	{
		return 0x00;
	}
	return iColour;
}


int Set8BitAlphaColour(int iAlpha)
{
	iAlpha = Clamp8BitColour(iAlpha);
	return iAlpha << RGBA32_ALPHA_SHIFT;
}

int Set8BitBlueColour(int iBlue)
{
	iBlue = Clamp8BitColour(iBlue);
	return iBlue << RGBA32_BLUE_SHIFT;
}

int Set8BitGreenColour(int iGreen)
{
	iGreen = Clamp8BitColour(iGreen);
	return iGreen << RGBA32_GREEN_SHIFT;
}

int Set8BitRedColour(int iRed)
{
	iRed = Clamp8BitColour(iRed);
	return iRed << RGBA32_RED_SHIFT;
}

int Set8BitAlphaColour(unsigned char iAlpha)
{
	return iAlpha << RGBA32_ALPHA_SHIFT;
}

int Set8BitBlueColour(unsigned char iBlue)
{
	return iBlue << RGBA32_BLUE_SHIFT;
}

int Set8BitGreenColour(unsigned char iGreen)
{
	return iGreen << RGBA32_GREEN_SHIFT;
}

int Set8BitRedColour(unsigned char iRed)
{
	return iRed << RGBA32_RED_SHIFT;
}

float Convert8BitColourToFloat(int iColour)
{
	return (float)iColour / 255.0f;
}

int ConvertFloatTo8BitColour(float fColour)
{
	return ((int)(fColour * 255.0f));
}

int Set32BitColour(int r, int g, int b, int a)
{
	int		iColour;

	iColour = Set8BitRedColour(r);
	iColour |= Set8BitGreenColour(g);
	iColour |= Set8BitBlueColour(b);
	iColour |= Set8BitAlphaColour(a);

	return iColour;
}


int Set32BitColour(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	int		iColour;

	iColour = Set8BitRedColour(r);
	iColour |= Set8BitGreenColour(g);
	iColour |= Set8BitBlueColour(b);
	iColour |= Set8BitAlphaColour(a);

	return iColour;
}


void Get32BitColour(int* r, int* g, int* b, int* a, int iColour)
{
	*r = Get8BitRedColour(iColour);
	*g = Get8BitGreenColour(iColour);
	*b = Get8BitBlueColour(iColour);
	*a = Get8BitAlphaColour(iColour);
}


void Get32BitColour(float* r, float* g, float* b, float* a, int iColour)
{
	*r = Convert8BitColourToFloat(Get8BitRedColour(iColour));
	*g = Convert8BitColourToFloat(Get8BitGreenColour(iColour));
	*b = Convert8BitColourToFloat(Get8BitBlueColour(iColour));
	*a = Convert8BitColourToFloat(Get8BitAlphaColour(iColour));
}


int Add32BitColours(int iColour1, int iColour2)
{
	int		r1, g1, b1, a1;
	int		r2, g2, b2, a2;

	Get32BitColour(&r1, &g1, &b1, &a1, iColour1);
	Get32BitColour(&r2, &g2, &b2, &a2, iColour2);
	r1 += r2;
	g1 += g2;
	b1 += b2;
	a1 += a2;

	return Set32BitColour(r1, g1, b1, a1);
}


int Subtract32BitColours(int iColour1, int iColour2)
{
	int		r1, g1, b1, a1;
	int		r2, g2, b2, a2;

	Get32BitColour(&r1, &g1, &b1, &a1, iColour1);
	Get32BitColour(&r2, &g2, &b2, &a2, iColour2);
	r1 -= r2;
	g1 -= g2;
	b1 -= b2;
	a1 -= a2;

	return Set32BitColour(r1, g1, b1, a1);
}


int Minimum32BitColour(int iColour1, int iColour2)
{
	int		r1, g1, b1, a1;
	int		r2, g2, b2, a2;

	Get32BitColour(&r1, &g1, &b1, &a1, iColour1);
	Get32BitColour(&r2, &g2, &b2, &a2, iColour2);
	if (r1 > r2) r1 = r2;
	if (g1 > g2) g1 = g2;
	if (b1 > b2) b1 = b2;
	if (a1 > a2) a1 = a2;

	return Set32BitColour(r1, g1, b1, a1);
}


int Maximum32BitColour(int iColour1, int iColour2)
{
	int		r1, g1, b1, a1;
	int		r2, g2, b2, a2;

	Get32BitColour(&r1, &g1, &b1, &a1, iColour1);
	Get32BitColour(&r2, &g2, &b2, &a2, iColour2);
	if (r1 < r2) r1 = r2;
	if (g1 < g2) g1 = g2;
	if (b1 < b2) b1 = b2;
	if (a1 < a2) a1 = a2;

	return Set32BitColour(r1, g1, b1, a1);
}


int Modulate32BitColours(int iColour1, int iColour2)
{
	int		r1, g1, b1, a1;
	int		r2, g2, b2, a2;

	Get32BitColour(&r1, &g1, &b1, &a1, iColour1);
	Get32BitColour(&r2, &g2, &b2, &a2, iColour2);

	r1 = (r1 * r2) / 255;
	g1 = (g1 * g2) / 255;
	b1 = (b1 * b2) / 255;
	a1 = (a1 * a2) / 255;

	return Set32BitColour(r1, g1, b1, a1);
}


int Weight32BitColours(int iColour1, int iColour2, float fWeight1)
{
	int		r1, g1, b1, a1;
	int		r2, g2, b2, a2;

	Get32BitColour(&r1, &g1, &b1, &a1, iColour1);
	Get32BitColour(&r2, &g2, &b2, &a2, iColour2);

	r1 = (int)(r1 * fWeight1 + r2 * (1.0f - fWeight1));
	g1 = (int)(g1 * fWeight1 + g2 * (1.0f - fWeight1));
	b1 = (int)(b1 * fWeight1 + b2 * (1.0f - fWeight1));
	a1 = (int)(a1 * fWeight1 + a2 * (1.0f - fWeight1));

	return Set32BitColour(r1, g1, b1, a1);
}


int BlendAlpha32BitColours(int iColour1, int iColour2)
{
	int		r1, g1, b1, a1;
	int		r2, g2, b2, a2;

	Get32BitColour(&r1, &g1, &b1, &a1, iColour1);
	Get32BitColour(&r2, &g2, &b2, &a2, iColour2);

	r1 = (r1 * a1 + r2 * (255 - a1)) / 255;
	g1 = (g1 * a1 + g2 * (255 - a1)) / 255;
	b1 = (b1 * a1 + b2 * (255 - a1)) / 255;
	a1 = (a1 * a1 + a2 * (255 - a1)) / 255;

	return Set32BitColour(r1, g1, b1, a1);
}


int Set32BitColour(float r, float g, float b, float a)
{
	int	ri, gi, bi, ai;

	ri = ConvertFloatTo8BitColour(r);
	gi = ConvertFloatTo8BitColour(g);
	bi = ConvertFloatTo8BitColour(b);
	ai = ConvertFloatTo8BitColour(a);

	return Set32BitColour(ri, gi, bi, ai);
}


int ConvertToBGR(int iColour)
{
	Swap(&(((uint8*)(&iColour))[0]), &(((uint8*)(&iColour))[2]));
	return iColour;
}


int ConvertToBGRA(int iColour)
{
	Swap(&(((uint8*)(&iColour))[0]), &(((uint8*)(&iColour))[2]));
	return iColour;
}


int GetM8BitAlphaColour(int iColour)
{
	return (iColour >> M_RGBA32_ALPHA_SHIFT) & 0xff;
}

int GetM8BitBlueColour(int iColour)
{
	return (iColour >> M_RGBA32_BLUE_SHIFT) & 0xff;
}

int GetM8BitGreenColour(int iColour)
{
	return (iColour >> M_RGBA32_GREEN_SHIFT) & 0xff;
}

int GetM8BitRedColour(int iColour)
{
	return (iColour >> M_RGBA32_RED_SHIFT) & 0xff;
}

int SetM8BitAlphaColour(unsigned char iAlpha)
{
	return iAlpha << M_RGBA32_ALPHA_SHIFT;
}

int SetM8BitBlueColour(unsigned char iBlue)
{
	return iBlue << M_RGBA32_BLUE_SHIFT;
}

int SetM8BitGreenColour(unsigned char iGreen)
{
	return iGreen << M_RGBA32_GREEN_SHIFT;
}

int SetM8BitRedColour(unsigned char iRed)
{
	return iRed << M_RGBA32_RED_SHIFT;
}

int SetM32BitColour(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	int		iColour;

	iColour = SetM8BitRedColour(r);
	iColour |= SetM8BitGreenColour(g);
	iColour |= SetM8BitBlueColour(b);
	iColour |= SetM8BitAlphaColour(a);

	return iColour;
}


