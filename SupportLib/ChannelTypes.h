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
#ifndef __IMAGE_TYPES_H__
#define __IMAGE_TYPES_H__
#include "BaseLib/Chars.h"


//The stop 'channel'.
#define CHANNEL_ZERO  0  


enum EChannelType  //1 byte for type
{
	CT_Red = 1,
	CT_Green,
	CT_Blue,
	CT_Hue,
	CT_Saturation,
	CT_Value,
	CT_Intensity,
	CT_X,
	CT_Y,
	CT_Z,
	CT_U,
	CT_V,
	CT_Ignored,
	CT_Index,

	NUM_CHANNEL_TYPES = 256,
};


typedef int EChannel;


#define CHANNEL(purpose, type)		((EChannel)((purpose * NUM_CHANNEL_TYPES) + type + 1))
#define CHANNEL_PURPOSE(channel)	(int)(channel / NUM_CHANNEL_TYPES)
#define CHANNEL_TYPE(channel)		(EChannelType)((channel % NUM_CHANNEL_TYPES) - 1)


void PrintChannelType(EChannelType eType, CChars* psz);


#endif // __IMAGE_TYPES_H__

