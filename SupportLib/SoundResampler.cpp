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
#include <math.h>
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ClassDefines.h"
#include "CatmullRom.h"
#include "SoundResampler.h"


//A table of every phase's taps is only built when it is smaller than this many taps.
#define SOUND_RESAMPLER_MAX_TABLE_TAPS		0x100000

#define SOUND_RESAMPLER_PI					3.14159265358979323846


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundResampler::Init(ESoundResample eResample, uint32 uiSampleRate)
{
	PreInit();

	PrivateInit(eResample, uiSampleRate, SOUND_RESAMPLER_DEFAULT_SINC_ZERO_CROSSINGS);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundResampler::Init(ESoundResample eResample, uint32 uiSampleRate, size uiZeroCrossings)
{
	PreInit();

	PrivateInit(eResample, uiSampleRate, uiZeroCrossings);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundResampler::PrivateInit(ESoundResample eResample, uint32 uiSampleRate, size uiZeroCrossings)
{
	meResample = eResample;
	muiSampleRate = uiSampleRate;
	muiZeroCrossings = uiZeroCrossings < 1 ? 1 : uiZeroCrossings;
	mfKaiserBeta = 9.0;
	mfRolloff = 0.95;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundResampler::Class(void)
{
	CSoundModifier::Class();
	U_Enum(meResample);
	U_UInt32(muiSampleRate);
	U_Size(muiZeroCrossings);
	U_Float64(mfKaiserBeta);
	U_Float64(mfRolloff);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CSoundResampler::GetResampledNumFrames(size uiNumFrames, uint32 uiSourceRate, uint32 uiDestRate)
{
	uint64	ulli;

	if (uiSourceRate == 0)
	{
		return 0;
	}
	ulli = ((uint64)uiNumFrames * (uint64)uiDestRate + (uint64)uiSourceRate - 1) / (uint64)uiSourceRate;
	if (ulli > SIZE_MAX)
	{
		return 0;
	}
	return (size)ulli;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 SoundResamplerGreatestCommonDivisor(uint32 uiA, uint32 uiB)
{
	uint32	uiRemainder;

	while (uiB != 0)
	{
		uiRemainder = uiA % uiB;
		uiA = uiB;
		uiB = uiRemainder;
	}
	return uiA;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSound> CSoundResampler::Modify(Ptr<CSound> pcSource)
{
	Ptr<CSound>		pcDest;
	uint32			uiSourceRate;
	size			uiSourceFrames;
	size			uiDestFrames;
	size			uiChannel;
	size			uiNumChannels;
	float64*		pfSource;
	float64*		pfDest;
	float64*		pfTable;
	size			uiNumPhases;
	size			uiNumTaps;
	size			uiPhase;
	float64			fCutoff;

	if (pcSource.IsNull() || !pcSource->CanAccessSamples() || (pcSource->GetSampleRate() == 0) || (muiSampleRate == 0))
	{
		gcLogger.Error2(__METHOD__, " Cannot resample sound.", NULL);
		return NULL;
	}

	uiSourceRate = pcSource->GetSampleRate();
	uiSourceFrames = pcSource->GetNumFrames();
	if (uiSourceRate == muiSampleRate)
	{
		return pcSource;
	}

	uiDestFrames = GetResampledNumFrames(uiSourceFrames, uiSourceRate, muiSampleRate);
	pcDest = OMalloc<CSound>(muiSampleRate, uiDestFrames, pcSource);
	if ((uiDestFrames > 0) && (pcDest->GetData() == NULL))
	{
		gcLogger.Error2(__METHOD__, " Could not allocate resampled sound of [", SizeToString(uiDestFrames), "] frames.", NULL);
		return NULL;
	}

	//Every output frame is at one of uiNumPhases fractions between source frames.
	uiNumPhases = muiSampleRate / SoundResamplerGreatestCommonDivisor(uiSourceRate, muiSampleRate);

	pfTable = NULL;
	if (meResample == SR_Sinc)
	{
		fCutoff = GetCutoff(uiSourceRate, muiSampleRate);
		uiNumTaps = GetNumSincTaps(fCutoff);
		if ((uint64)uiNumPhases * (uint64)uiNumTaps <= SOUND_RESAMPLER_MAX_TABLE_TAPS)
		{
			pfTable = (float64*)malloc(sizeof(float64) * uiNumPhases * uiNumTaps);
			for (uiPhase = 0; uiPhase < uiNumPhases; uiPhase++)
			{
				CalculateSincTaps((float64)uiPhase / (float64)uiNumPhases, fCutoff, &pfTable[uiPhase * uiNumTaps], uiNumTaps);
			}
		}
	}

	pfSource = (float64*)malloc(sizeof(float64) * (uiSourceFrames + 1));
	pfDest = (float64*)malloc(sizeof(float64) * (uiDestFrames + 1));
	uiNumChannels = pcSource->NumChannels();
	for (uiChannel = 0; uiChannel < uiNumChannels; uiChannel++)
	{
		pcSource->GetSamples(uiChannel, 0, uiSourceFrames, pfSource);
		ResampleChannel(pfSource, uiSourceFrames, pfDest, uiDestFrames, uiSourceRate, muiSampleRate, pfTable, uiNumPhases);
		pcDest->SetSamples(uiChannel, 0, uiDestFrames, pfDest);
	}
	free(pfDest);
	free(pfSource);
	SafeFree(pfTable);

	return pcDest;
}


//////////////////////////////////////////////////////////////////////////
//
// The sinc filter's cut off as a fraction of the source Nyquist frequency.
//
//////////////////////////////////////////////////////////////////////////
float64 CSoundResampler::GetCutoff(uint32 uiSourceRate, uint32 uiDestRate)
{
	if (uiDestRate < uiSourceRate)
	{
		return mfRolloff * (float64)uiDestRate / (float64)uiSourceRate;
	}
	return mfRolloff;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundResampler::ResampleChannel(float64* pfSource, size uiSourceFrames, float64* pfDest, size uiDestFrames, uint32 uiSourceRate, uint32 uiDestRate, float64* pfTable, size uiNumPhases)
{
	size		uiFrame;
	uint64		ulliPosition;
	uint64		ulliIndex;
	uint64		ulliPhase;
	float64		fFraction;
	float64		fCutoff;
	size		uiNumTaps;
	float64*	pfTaps;
	size		uiPhaseIndex;

	fCutoff = 0.0;
	uiNumTaps = 0;
	pfTaps = NULL;
	if ((meResample == SR_Sinc) && (pfTable == NULL))
	{
		fCutoff = GetCutoff(uiSourceRate, uiDestRate);
		uiNumTaps = GetNumSincTaps(fCutoff);
		pfTaps = (float64*)malloc(sizeof(float64) * uiNumTaps);
	}
	else if (meResample == SR_Sinc)
	{
		uiNumTaps = GetNumSincTaps(GetCutoff(uiSourceRate, uiDestRate));
	}

	for (uiFrame = 0; uiFrame < uiDestFrames; uiFrame++)
	{
		ulliPosition = (uint64)uiFrame * (uint64)uiSourceRate;
		ulliIndex = ulliPosition / uiDestRate;
		ulliPhase = ulliPosition % uiDestRate;
		fFraction = (float64)ulliPhase / (float64)uiDestRate;

		switch (meResample)
		{
		case SR_NearestNeighbour:
			pfDest[uiFrame] = Nearest(pfSource, uiSourceFrames, ulliIndex, ulliPhase, uiDestRate);
			break;
		case SR_Linear:
			pfDest[uiFrame] = Linear(pfSource, uiSourceFrames, ulliIndex, fFraction);
			break;
		case SR_Cubic:
			pfDest[uiFrame] = Cubic(pfSource, uiSourceFrames, ulliIndex, fFraction);
			break;
		case SR_Sinc:
			if (pfTable)
			{
				//The phase is always a multiple of dest rate / number of phases.
				uiPhaseIndex = (size)(ulliPhase / (uiDestRate / uiNumPhases));
				pfDest[uiFrame] = Sinc(pfSource, uiSourceFrames, ulliIndex, &pfTable[uiPhaseIndex * uiNumTaps], uiNumTaps);
			}
			else
			{
				CalculateSincTaps(fFraction, fCutoff, pfTaps, uiNumTaps);
				pfDest[uiFrame] = Sinc(pfSource, uiSourceFrames, ulliIndex, pfTaps, uiNumTaps);
			}
			break;
		}
	}
	SafeFree(pfTaps);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CSoundResampler::Nearest(float64* pfSource, size uiSourceFrames, uint64 ulliIndex, uint64 ulliPhase, uint32 uiDestRate)
{
	if (ulliPhase * 2 >= uiDestRate)
	{
		ulliIndex++;
	}
	if (ulliIndex >= uiSourceFrames)
	{
		ulliIndex = uiSourceFrames - 1;
	}
	return pfSource[ulliIndex];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CSoundResampler::Linear(float64* pfSource, size uiSourceFrames, uint64 ulliIndex, float64 fFraction)
{
	float64	f0;
	float64	f1;

	f0 = pfSource[ulliIndex < uiSourceFrames ? ulliIndex : uiSourceFrames - 1];
	f1 = pfSource[ulliIndex + 1 < uiSourceFrames ? ulliIndex + 1 : uiSourceFrames - 1];
	return f0 + (f1 - f0) * fFraction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CSoundResampler::Cubic(float64* pfSource, size uiSourceFrames, uint64 ulliIndex, float64 fFraction)
{
	float64	fm1;
	float64	f0;
	float64	f1;
	float64	f2;
	uint64	ulliLast;

	ulliLast = uiSourceFrames - 1;
	fm1 = pfSource[ulliIndex == 0 ? 0 : (ulliIndex - 1 <= ulliLast ? ulliIndex - 1 : ulliLast)];
	f0 = pfSource[ulliIndex <= ulliLast ? ulliIndex : ulliLast];
	f1 = pfSource[ulliIndex + 1 <= ulliLast ? ulliIndex + 1 : ulliLast];
	f2 = pfSource[ulliIndex + 2 <= ulliLast ? ulliIndex + 2 : ulliLast];
	return CatmullRom(fm1, f0, f1, f2, fFraction);
}


//////////////////////////////////////////////////////////////////////////
//
// Tap m multiplies source frame ulliIndex - (uiNumTaps / 2) + 1 + m.
//
//////////////////////////////////////////////////////////////////////////
float64 CSoundResampler::Sinc(float64* pfSource, size uiSourceFrames, uint64 ulliIndex, float64* pfTaps, size uiNumTaps)
{
	int64	lliFirst;
	int64	lliFrame;
	size	uiTap;
	size	uiLastTap;
	float64	fSum;

	lliFirst = (int64)ulliIndex - (int64)(uiNumTaps / 2) + 1;
	uiTap = 0;
	if (lliFirst < 0)
	{
		uiTap = (size)(-lliFirst);
	}
	uiLastTap = uiNumTaps;
	if (lliFirst + (int64)uiNumTaps > (int64)uiSourceFrames)
	{
		uiLastTap = (size)((int64)uiSourceFrames - lliFirst);
	}

	fSum = 0.0;
	for (; uiTap < uiLastTap; uiTap++)
	{
		lliFrame = lliFirst + (int64)uiTap;
		fSum += pfSource[lliFrame] * pfTaps[uiTap];
	}
	return fSum;
}


//////////////////////////////////////////////////////////////////////////
//
// The taps for an output fFraction of a frame after the centre source
// frame.  They are scaled to sum to one so silence and constant sound
// keep their level.
//
//////////////////////////////////////////////////////////////////////////
void CSoundResampler::CalculateSincTaps(float64 fFraction, float64 fCutoff, float64* pfTaps, size uiNumTaps)
{
	size	uiTap;
	size	uiHalf;
	float64	fT;
	float64	fX;
	float64	fHalfWidth;
	float64	fSum;
	float64	fTap;

	uiHalf = uiNumTaps / 2;
	fHalfWidth = (float64)muiZeroCrossings / fCutoff;
	fSum = 0.0;
	for (uiTap = 0; uiTap < uiNumTaps; uiTap++)
	{
		//The distance in source frames from the output position to the tap's frame.
		fT = fFraction + (float64)uiHalf - 1.0 - (float64)uiTap;
		if (fabs(fT) >= fHalfWidth)
		{
			fTap = 0.0;
		}
		else
		{
			fX = SOUND_RESAMPLER_PI * fCutoff * fT;
			if (fX == 0.0)
			{
				fTap = fCutoff;
			}
			else
			{
				fTap = fCutoff * sin(fX) / fX;
			}
			fTap *= Kaiser(fT / fHalfWidth);
		}
		pfTaps[uiTap] = fTap;
		fSum += fTap;
	}

	if (fSum != 0.0)
	{
		for (uiTap = 0; uiTap < uiNumTaps; uiTap++)
		{
			pfTaps[uiTap] /= fSum;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Enough taps to cover the filter's half width on each side.
//
//////////////////////////////////////////////////////////////////////////
size CSoundResampler::GetNumSincTaps(float64 fCutoff)
{
	return 2 * (size)ceil((float64)muiZeroCrossings / fCutoff);
}


//////////////////////////////////////////////////////////////////////////
//
// I0 is the zeroth order modified Bessel function of the first kind.
//
//////////////////////////////////////////////////////////////////////////
float64 BesselI0(float64 fX)
{
	float64	fSum;
	float64	fTerm;
	float64	fHalfX;
	int		k;

	fSum = 1.0;
	fTerm = 1.0;
	fHalfX = fX / 2.0;
	for (k = 1; k < 64; k++)
	{
		fTerm *= (fHalfX / k) * (fHalfX / k);
		fSum += fTerm;
		if (fTerm < fSum * 1e-17)
		{
			break;
		}
	}
	return fSum;
}


//////////////////////////////////////////////////////////////////////////
//
// The Kaiser window, fX from -1 to 1.
//
//////////////////////////////////////////////////////////////////////////
float64 CSoundResampler::Kaiser(float64 fX)
{
	float64	f;

	f = 1.0 - fX * fX;
	if (f <= 0.0)
	{
		return 0.0;
	}
	return BesselI0(mfKaiserBeta * sqrt(f)) / BesselI0(mfKaiserBeta);
}

