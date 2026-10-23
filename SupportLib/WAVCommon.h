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

** ------------------------------------------------------------------------ **/
#ifndef __WAV_COMMON_H__
#define __WAV_COMMON_H__
#include "BaseLib/PrimitiveTypes.h"


#define WAV_FORMAT_PCM				0x0001
#define WAV_FORMAT_IEEE_FLOAT		0x0003
#define WAV_FORMAT_EXTENSIBLE		0xFFFE

#define WAV_FMT_SIZE				16
#define WAV_FMT_EXTENSIBLE_SIZE		40
#define WAV_EXTENSIBLE_CB_SIZE		22

//The last 14 bytes of the KSDATAFORMAT_SUBTYPE_PCM and _IEEE_FLOAT GUIDs.  The first two bytes are the format.
extern uint8 gauiWAVSubFormatGUID[14];


uint32	GetWAVDefaultChannelMask(size uiNumChannels);


#endif // __WAV_COMMON_H__

