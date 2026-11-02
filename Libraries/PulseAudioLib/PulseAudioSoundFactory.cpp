/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela PulseAudioLib

Codaphela PulseAudioLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela PulseAudioLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela PulseAudioLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "PulseAudioSoundMixer.h"
#include "PulseAudioSoundFactory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPulseAudioSoundFactory::Init(CMallocator* pcMallocator)
{
	CNativeSoundFactory::Init(pcMallocator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPulseAudioSoundFactory::Kill(void)
{
	CNativeSoundFactory::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNativeSoundMixer* CPulseAudioSoundFactory::CreateNativeSoundMixer(CSoundMixer* pcMixer)
{
	CPulseAudioSoundMixer*	pcNativeMixer;

	pcNativeMixer = NativeMalloc<CPulseAudioSoundMixer>();
	pcNativeMixer->Init(pcMixer, this);
	return pcNativeMixer;
}

