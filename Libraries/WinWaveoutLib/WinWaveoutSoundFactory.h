#ifndef __WIN_WAVEOUT_SOUND_FACTORY_H__
#define __WIN_WAVEOUT_SOUND_FACTORY_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WinWaveoutLib

Codaphela WinWaveoutLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WinWaveoutLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WinWaveoutLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "WindowLib/NativeSoundFactory.h"


//Sound mixers played through waveOut.
class CWinWaveoutSoundFactory : public CNativeSoundFactory
{
CONSTRUCTABLE(CWinWaveoutSoundFactory);
public:
	void				Init(CMallocator* pcMallocator);
	void				Kill(void);

	CNativeSoundMixer*	CreateNativeSoundMixer(CSoundMixer* pcMixer) override;
};


#endif // __WIN_WAVEOUT_SOUND_FACTORY_H__

