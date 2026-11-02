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
#include <stdlib.h>
#include <string.h>
#include "BaseLib/Logger.h"
#include "WinWaveoutSoundMixer.h"


#pragma comment(lib, "winmm.lib")


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinWaveoutSoundMixer::Init(CSoundMixer* pcMixer, CNativeSoundFactory* pcSoundFactory)
{
	CNativeSoundMixer::Init(pcMixer, pcSoundFactory);
	mhWaveOut = NULL;
	memset(masHeaders, 0, sizeof(masHeaders));
	mpaiData = NULL;
	muiBufferFrames = 0;
	muiNext = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinWaveoutSoundMixer::Kill(void)
{
	Close();
	CNativeSoundMixer::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The wave mapper at uiSampleRate 16 bit stereo frames.
//
//////////////////////////////////////////////////////////////////////////
bool CWinWaveoutSoundMixer::Open(uint32 uiSampleRate)
{
	WAVEFORMATEX	sFormat;
	MMRESULT		eResult;

	Close();
	if (uiSampleRate == 0)
	{
		return false;
	}

	memset(&sFormat, 0, sizeof(WAVEFORMATEX));
	sFormat.wFormatTag = WAVE_FORMAT_PCM;
	sFormat.nChannels = 2;
	sFormat.nSamplesPerSec = uiSampleRate;
	sFormat.wBitsPerSample = 16;
	sFormat.nBlockAlign = 4;
	sFormat.nAvgBytesPerSec = uiSampleRate * 4;
	eResult = waveOutOpen(&mhWaveOut, WAVE_MAPPER, &sFormat, 0, 0, CALLBACK_NULL);
	if (eResult != MMSYSERR_NOERROR)
	{
		gcLogger.Error2(__METHOD__, " Could not open the wave output.", NULL);
		mhWaveOut = NULL;
		return false;
	}

	muiBufferFrames = uiSampleRate / WIN_WAVEOUT_BUFFERS_A_SECOND;
	if (muiBufferFrames < 64)
	{
		muiBufferFrames = 64;
	}
	mpaiData = (int16*)malloc((size_t)(WIN_WAVEOUT_BUFFERS * muiBufferFrames * 2 * sizeof(int16)));
	memset(masHeaders, 0, sizeof(masHeaders));
	muiNext = 0;
	muiSampleRate = uiSampleRate;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinWaveoutSoundMixer::Close(void)
{
	size	i;

	if (mhWaveOut)
	{
		waveOutReset(mhWaveOut);
		for (i = 0; i < WIN_WAVEOUT_BUFFERS; i++)
		{
			if (masHeaders[i].dwFlags & WHDR_PREPARED)
			{
				waveOutUnprepareHeader(mhWaveOut, &masHeaders[i], sizeof(WAVEHDR));
			}
		}
		waveOutClose(mhWaveOut);
		mhWaveOut = NULL;
	}
	free(mpaiData);
	mpaiData = NULL;
	memset(masHeaders, 0, sizeof(masHeaders));
	muiBufferFrames = 0;
	muiSampleRate = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The driver sets the flags from its own thread.
//
//////////////////////////////////////////////////////////////////////////
bool CWinWaveoutSoundMixer::IsQueued(WAVEHDR* psHeader)
{
	return (((volatile DWORD&)psHeader->dwFlags) & WHDR_INQUEUE) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CWinWaveoutSoundMixer::NumQueuedFrames(void)
{
	size	i;
	lsize	uiFrames;

	uiFrames = 0;
	for (i = 0; i < WIN_WAVEOUT_BUFFERS; i++)
	{
		if (IsQueued(&masHeaders[i]))
		{
			uiFrames += masHeaders[i].dwBufferLength / 4;
		}
	}
	return uiFrames;
}


//////////////////////////////////////////////////////////////////////////
//
// The frames go into the buffers after the last written, each as full as
// it can be.  A buffer is prepared again for each write as its length
// changes.
//
//////////////////////////////////////////////////////////////////////////
void CWinWaveoutSoundMixer::Write(int16* paiFrames, lsize uiFrames)
{
	WAVEHDR*	psHeader;
	lsize		uiCopy;

	if (mhWaveOut == NULL)
	{
		muiDroppedFrames += uiFrames;
		return;
	}

	while (uiFrames > 0)
	{
		psHeader = &masHeaders[muiNext];
		if (IsQueued(psHeader))
		{
			muiDroppedFrames += uiFrames;
			return;
		}
		if (psHeader->dwFlags & WHDR_PREPARED)
		{
			waveOutUnprepareHeader(mhWaveOut, psHeader, sizeof(WAVEHDR));
		}

		uiCopy = (uiFrames < muiBufferFrames) ? uiFrames : muiBufferFrames;
		memset(psHeader, 0, sizeof(WAVEHDR));
		psHeader->lpData = (LPSTR)&mpaiData[muiNext * muiBufferFrames * 2];
		psHeader->dwBufferLength = (DWORD)(uiCopy * 4);
		memcpy(psHeader->lpData, paiFrames, (size_t)(uiCopy * 4));
		waveOutPrepareHeader(mhWaveOut, psHeader, sizeof(WAVEHDR));
		waveOutWrite(mhWaveOut, psHeader, sizeof(WAVEHDR));

		paiFrames += uiCopy * 2;
		uiFrames -= uiCopy;
		muiNext = (muiNext + 1) % WIN_WAVEOUT_BUFFERS;
	}
}

