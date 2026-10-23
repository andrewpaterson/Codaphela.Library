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
#ifndef __WAV_READER_H__
#define __WAV_READER_H__
#include "Sound.h"


//Reads 8 (unsigned), 16, 24 and 32 bit PCM and 32 and 64 bit float WAVE files, including WAVE_FORMAT_EXTENSIBLE.
//8 bit samples are read as uint8, 16 as int16, 24 and 32 as int32 (24 bit samples fill the top three bytes) and float as float32 or float64.
//Channels are named from the channel mask; without one, mono is SOUND_MONO and other counts use the usual speaker layouts.
//A data chunk that runs past the end of the file is cut to the whole frames that are there.
Ptr<CSound> LoadWAV(char* szFilename);
Ptr<CSound> LoadWAV(void* pvFile, size uiFileSize);


#endif // __WAV_READER_H__

