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


typedef uint32 ARGB32;


//See RGB32.png for a description.  Uses 'Dragon' format.  R0, G1, B2, A3 where postscript is index of colour.
uint8	Get8BitAlphaColour(ARGB32 uiColour);
uint8	Get8BitBlueColour(ARGB32 uiColour);
uint8	Get8BitGreenColour(ARGB32 uiColour);
uint8	Get8BitRedColour(ARGB32 uiColour);
uint8	Clamp8BitColour(uint32 uiColour);
ARGB32	Set8BitAlphaColour(uint32 iAlpha);
ARGB32	Set8BitBlueColour(uint32 iBlue);
ARGB32	Set8BitGreenColour(uint32 iGreen);
ARGB32	Set8BitRedColour(uint32 iRed);
ARGB32	Set8BitAlphaColour(uint8 iAlpha);
ARGB32	Set8BitBlueColour(uint8 iBlue);
ARGB32	Set8BitGreenColour(uint8 iGreen);
ARGB32	Set8BitRedColour(uint8 iRed);
float	Convert8BitColourToFloat(ARGB32 uiColour);
uint8	ConvertFloatTo8BitColour(float fColour);
ARGB32	Set32BitColour(uint32 r, uint32 g, uint32 b, uint32 a);
ARGB32	Set32BitColour(uint8 r, uint8 g, uint8 b, uint8 a);
ARGB32	Set32BitColour(float r, float g, float b, float a);
ARGB32	Set32BitColour(uint32 r, uint32 g, uint32 b);
ARGB32	Set32BitColour(uint8 r, uint8 g, uint8 b);
ARGB32	Set32BitColour(float r, float g, float b);
void	Get32BitColour(uint32* r, uint32* g, uint32* b, uint32* a, ARGB32 uiColour);
void	Get32BitColour(float* r, float* g, float* b, float* a, ARGB32 uiColour);
ARGB32	Add32BitColours(ARGB32 uiColour1, ARGB32 uiColour2);
ARGB32	Subtract32BitColours(ARGB32 uiColour1, ARGB32 uiColour2);
ARGB32	Minimum32BitColour(ARGB32 uiColour1, ARGB32 uiColour2);
ARGB32	Maximum32BitColour(ARGB32 uiColour1, ARGB32 uiColour2);
ARGB32	Modulate32BitColours(ARGB32 uiColour1, ARGB32 uiColour2);
ARGB32	Weight32BitColours(ARGB32 uiColour1, ARGB32 uiColour2, float fWeight1);
ARGB32	BlendAlpha32BitColours(ARGB32 uiColour1, ARGB32 uiColour2);

//'M' functions are Microsoft format.  R2, G1, B0, A3.
uint8	GetM8BitAlphaColour(ARGB32 uiColour);
uint8	GetM8BitBlueColour(ARGB32 uiColour);
uint8	GetM8BitGreenColour(ARGB32 uiColour);
uint8	GetM8BitRedColour(ARGB32 uiColour);
ARGB32	SetM32BitColour(uint8 r, uint8 g, uint8 b, uint8 a);
ARGB32	SetM8BitAlphaColour(uint8 iAlpha);
ARGB32	SetM8BitBlueColour(uint8 iBlue);
ARGB32	SetM8BitGreenColour(uint8 iGreen);
ARGB32	SetM8BitRedColour(uint8 iRed);

//Conversion functions see RGB32.png for details.
uint32	ConvertToBGR(ARGB32 uiColour);
uint32	ConvertToBGRA(ARGB32 uiColour);


#endif // __COLOUR_RGBA32_H__

