/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __VERTEX_CHANNEL_H__
#define __VERTEX_CHANNEL_H__
#include "ChannelTypes.h"


enum EVertexPurpose
{
	//1 byte for purpose.
	VP_Unknown,
	VP_Position,
	VP_Diffuse,
	VP_Specular,
	VP_Opacity,
	VP_Normal,
	VP_UVCoord,
	VP_Weight,
	VP_Matrix,

	NUM_VERTEX_PURPOSE = 256,
};


#define VP_UVCoord_(p)	((EVertexPurpose)((int)VP_UVCoord + p * 256))
#define VP_Weight_(p)	((EVertexPurpose)((int)VP_Weight + p * 256))
#define VP_Matrix_(p)	((EVertexPurpose)((int)VP_Matrix + p * 256))


#define IMAGE_PURPOSE(iChannel) ((EImagePurpose)((iChannel - 1) / NUM_CHANNEL_TYPES))


#define VERTEX_IGNORED			CHANNEL(VP_Unknown, CT_Ignored)
#define VERTEX_DIFFUSE_RED		CHANNEL(VP_Diffuse, CT_Red)
#define VERTEX_DIFFUSE_GREEN	CHANNEL(VP_Diffuse, CT_Green)
#define VERTEX_DIFFUSE_BLUE		CHANNEL(VP_Diffuse, CT_Blue)
#define VERTEX_OPACITY			CHANNEL(VP_Opacity, CT_Intensity)
#define VERTEX_NORMAL_X			CHANNEL(VP_Normal, CT_X)
#define VERTEX_NORMAL_Y			CHANNEL(VP_Normal, CT_Y)
#define VERTEX_NORMAL_Z			CHANNEL(VP_Normal, CT_Z)
#define VERTEX_POSITION_X		CHANNEL(VP_Position, CT_X)
#define VERTEX_POSITION_Y		CHANNEL(VP_Position, CT_Y)
#define VERTEX_POSITION_Z		CHANNEL(VP_Position, CT_Z)
#define VERTEX_UVCOORD_U_0		CHANNEL(VP_UVCoord_(0), CT_U)
#define VERTEX_UVCOORD_V_0		CHANNEL(VP_UVCoord_(0), CT_V)
#define VERTEX_UVCOORD_U_1		CHANNEL(VP_UVCoord_(1), CT_U)
#define VERTEX_UVCOORD_V_1		CHANNEL(VP_UVCoord_(1), CT_V)
#define VERTEX_UVCOORD_U_2		CHANNEL(VP_UVCoord_(2), CT_U)
#define VERTEX_UVCOORD_V_2		CHANNEL(VP_UVCoord_(2), CT_V)
#define VERTEX_WEIGHT_0			CHANNEL(VP_Weight_(0), CT_Intensity)
#define VERTEX_WEIGHT_1			CHANNEL(VP_Weight_(1), CT_Intensity)
#define VERTEX_WEIGHT_2			CHANNEL(VP_Weight_(2), CT_Intensity)
#define VERTEX_WEIGHT_3			CHANNEL(VP_Weight_(3), CT_Intensity)
#define VERTEX_MATRIX_0			CHANNEL(VP_Matrix_(0), CT_Index)
#define VERTEX_MATRIX_1			CHANNEL(VP_Matrix_(1), CT_Index)
#define VERTEX_MATRIX_2			CHANNEL(VP_Matrix_(2), CT_Index)
#define VERTEX_MATRIX_3			CHANNEL(VP_Matrix_(3), CT_Index)


#endif // __VERTEX_CHANNEL_H__

