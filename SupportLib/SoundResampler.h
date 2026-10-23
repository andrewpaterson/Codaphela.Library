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
#ifndef __SOUND_RESAMPLER_H__
#define __SOUND_RESAMPLER_H__
#include "Sound.h"
#include "SoundModifier.h"


enum ESoundResample
{
	SR_NearestNeighbour,
	SR_Linear,
	SR_Cubic,		//Catmull-Rom.
	SR_Sinc,		//Kaiser windowed sinc.  Low pass filters below the new Nyquist frequency when the rate goes down.
};


#define SOUND_RESAMPLER_DEFAULT_SINC_ZERO_CROSSINGS		32


//Output frame i is taken from source time i * source rate / dest rate, so the first frames line up.  The output has
//ceil(frames * dest rate / source rate) frames and the same channels as the source.  Linear and cubic hold the end
//samples past the ends of the source; sinc treats samples past the ends as silence.  A sound already at the sample rate
//is returned as it is.
class CSoundResampler : public CSoundModifier
{
CONSTRUCTABLE(CSoundResampler);
DESTRUCTABLE(CSoundResampler);
protected:
	ESoundResample	meResample;
	uint32			muiSampleRate;
	size			muiZeroCrossings;	//Sinc only: the zero crossings of the filter on each side of the centre.
	float64			mfKaiserBeta;
	float64			mfRolloff;			//Sinc only: the filter's cut off as a fraction of the lower Nyquist frequency.

public:
	void			Init(ESoundResample eResample, uint32 uiSampleRate);
	void			Init(ESoundResample eResample, uint32 uiSampleRate, size uiZeroCrossings);
	void			Class(void) override;

	Ptr<CSound>		Modify(Ptr<CSound> pcSound) override;
	static size		GetResampledNumFrames(size uiNumFrames, uint32 uiSourceRate, uint32 uiDestRate);

protected:
	void			PrivateInit(ESoundResample eResample, uint32 uiSampleRate, size uiZeroCrossings);
	void			ResampleChannel(float64* pfSource, size uiSourceFrames, float64* pfDest, size uiDestFrames, uint32 uiSourceRate, uint32 uiDestRate, float64* pfTable, size uiNumPhases);
	float64			Nearest(float64* pfSource, size uiSourceFrames, uint64 ulliIndex, uint64 ulliPhase, uint32 uiDestRate);
	float64			Linear(float64* pfSource, size uiSourceFrames, uint64 ulliIndex, float64 fFraction);
	float64			Cubic(float64* pfSource, size uiSourceFrames, uint64 ulliIndex, float64 fFraction);
	float64			Sinc(float64* pfSource, size uiSourceFrames, uint64 ulliIndex, float64* pfTaps, size uiNumTaps);
	float64			GetCutoff(uint32 uiSourceRate, uint32 uiDestRate);
	void			CalculateSincTaps(float64 fFraction, float64 fCutoff, float64* pfTaps, size uiNumTaps);
	size			GetNumSincTaps(float64 fCutoff);
	float64			Kaiser(float64 fX);
};


#endif // __SOUND_RESAMPLER_H__

