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
#ifndef __COLOUR_RGBA32_H__
#define __COLOUR_RGBA32_H__
#include "BaseLib/PrimitiveTypes.h"


#define RGBA32_RED_SHIFT	0
#define RGBA32_GREEN_SHIFT	8
#define RGBA32_BLUE_SHIFT	16
#define RGBA32_ALPHA_SHIFT	24

#define M_RGBA32_RED_SHIFT		16
#define M_RGBA32_GREEN_SHIFT	8
#define M_RGBA32_BLUE_SHIFT		0
#define M_RGBA32_ALPHA_SHIFT	24

//See RGB32.png for a description.  Uses 'Dragon' format.  R0, G1, B2, A3 where postscript is index of colour.
int Get8BitAlphaColour(int iColour);
int Get8BitBlueColour(int iColour);
int Get8BitGreenColour(int iColour);
int Get8BitRedColour(int iColour);
int Clamp8BitColour(int iColour);
int Set8BitAlphaColour(int iAlpha);
int Set8BitBlueColour(int iBlue);
int Set8BitGreenColour(int iGreen);
int Set8BitRedColour(int iRed);
int Set8BitAlphaColour(uint8 iAlpha);
int Set8BitBlueColour(uint8 iBlue);
int Set8BitGreenColour(uint8 iGreen);
int Set8BitRedColour(uint8 iRed);
float Convert8BitColourToFloat(int iColour);
int ConvertFloatTo8BitColour(float fColour);
int Set32BitColour(int r, int g, int b, int a);
int Set32BitColour(uint8 r, uint8 g, uint8 b, uint8 a);
int Set32BitColour(float r, float g, float b, float a);
void Get32BitColour(int* r, int* g, int* b, int* a, int iColour);
void Get32BitColour(float* r, float* g, float* b, float* a, int iColour);
int Add32BitColours(int iColour1, int iColour2);
int Subtract32BitColours(int iColour1, int iColour2);
int Minimum32BitColour(int iColour1, int iColour2);
int Maximum32BitColour(int iColour1, int iColour2);
int Modulate32BitColours(int iColour1, int iColour2);
int Weight32BitColours(int iColour1, int iColour2, float fWeight1);
int BlendAlpha32BitColours(int iColour1, int iColour2);

//'M' functions are Microsoft format.  R2, G1, B0, A3.
int GetM8BitAlphaColour(int iColour);
int GetM8BitBlueColour(int iColour);
int GetM8BitGreenColour(int iColour);
int GetM8BitRedColour(int iColour);
int SetM32BitColour(uint8 r, uint8 g, uint8 b, uint8 a);
int SetM8BitAlphaColour(uint8 iAlpha);
int SetM8BitBlueColour(uint8 iBlue);
int SetM8BitGreenColour(uint8 iGreen);
int SetM8BitRedColour(uint8 iRed);

//Conversion functions see RGB32.png for details.
int ConvertToBGR(int iColour);
int ConvertToBGRA(int iColour);


#endif // !__COLOUR_RGBA32_H__

