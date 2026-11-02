#ifndef __WIN_WAVEOUT_SOUND_MIXER_H__
#define __WIN_WAVEOUT_SOUND_MIXER_H__
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
#include <windows.h>
#include <mmsystem.h>
#include "WindowLib/NativeSoundMixer.h"


#define WIN_WAVEOUT_BUFFERS				32
#define WIN_WAVEOUT_BUFFERS_A_SECOND	32		//A buffer holds at most 1/32 of a second, so they all hold a second.


//The wave mapper's output.  Each Write is queued in as many buffers as it needs: frames that find no free buffer are
//dropped.  The frames queued are those in the buffers not yet played.
class CWinWaveoutSoundMixer : public CNativeSoundMixer
{
CONSTRUCTABLE(CWinWaveoutSoundMixer);
protected:
	HWAVEOUT	mhWaveOut;
	WAVEHDR		masHeaders[WIN_WAVEOUT_BUFFERS];
	int16*		mpaiData;			//WIN_WAVEOUT_BUFFERS buffers of muiBufferFrames left/right pairs.
	lsize		muiBufferFrames;
	size		muiNext;			//The buffer the next Write starts in.

public:
	void	Init(CSoundMixer* pcMixer, CNativeSoundFactory* pcSoundFactory);
	void	Kill(void) override;

	bool	Open(uint32 uiSampleRate) override;
	void	Close(void) override;
	lsize	NumQueuedFrames(void) override;
	void	Write(int16* paiFrames, lsize uiFrames) override;

protected:
	bool	IsQueued(WAVEHDR* psHeader);
};


#endif // __WIN_WAVEOUT_SOUND_MIXER_H__

