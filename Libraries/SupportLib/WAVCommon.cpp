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
#include "WAVCommon.h"


//{xxxx0000-0000-0010-8000-00AA00389B71} as stored in the file, after the two format bytes.
uint8 gauiWAVSubFormatGUID[14] = { 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 };


//////////////////////////////////////////////////////////////////////////
//
// The speakers a file without a channel mask is assumed to have.  Zero
// for mono (which is not a speaker in the mask) and unusual counts.
//
//////////////////////////////////////////////////////////////////////////
uint32 GetWAVDefaultChannelMask(size uiNumChannels)
{
	switch (uiNumChannels)
	{
	case 2:
		return 0x003;	//Front left, front right.
	case 3:
		return 0x007;	//Front left, front right, front centre.
	case 4:
		return 0x033;	//Front left, front right, back left, back right.
	case 5:
		return 0x037;	//Front left, front right, front centre, back left, back right.
	case 6:
		return 0x03F;	//5.1.
	case 7:
		return 0x70F;	//6.1: front left, front right, front centre, low frequency, back centre, side left, side right.
	case 8:
		return 0x63F;	//7.1: 5.1 with side left and side right.
	default:
		return 0;
	}
}

