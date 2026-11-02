/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "NativeSoundMixer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNativeSoundMixer::Init(CSoundMixer* pcMixer, CNativeSoundFactory* pcSoundFactory)
{
	mpcMixer = pcMixer;
	mpcSoundFactory = pcSoundFactory;
	muiSampleRate = 0;
	muiDroppedFrames = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Subclasses close their output before calling this.
//
//////////////////////////////////////////////////////////////////////////
void CNativeSoundMixer::Kill(void)
{
	mpcMixer = NULL;
	mpcSoundFactory = NULL;
	muiSampleRate = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNativeSoundMixer::IsOpen(void) { return muiSampleRate != 0; }
uint32 CNativeSoundMixer::GetSampleRate(void) { return muiSampleRate; }
uint64 CNativeSoundMixer::NumDroppedFrames(void) { return muiDroppedFrames; }
CSoundMixer* CNativeSoundMixer::GetMixer(void) { return mpcMixer; }
CNativeSoundFactory* CNativeSoundMixer::GetFactory(void) { return mpcSoundFactory; }

