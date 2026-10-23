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
#ifndef __WAV_WRITER_H__
#define __WAV_WRITER_H__
#include "Sound.h"


enum EWAVSampleFormat
{
	WSF_Default,	//Chosen from the sound's sample type: 8 bit types as PCM 8, 16 bit as PCM 16, 32 bit integers as PCM 32 and floats as themselves.
	WSF_PCM8,
	WSF_PCM16,
	WSF_PCM24,
	WSF_PCM32,
	WSF_Float32,
	WSF_Float64,
};


//Writes speaker channels in channel mask order followed by discrete channels.  Ignored channels are not written.
//Mono and stereo PCM up to 16 bits is written as a plain PCM file; everything else is WAVE_FORMAT_EXTENSIBLE.
bool SaveWAV(Ptr<CSound> pcSound, char* szPathName, EWAVSampleFormat eFormat = WSF_Default);


#endif // __WAV_WRITER_H__

