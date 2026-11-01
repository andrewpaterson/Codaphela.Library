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
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "SoundCel.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundCel::Init(Ptr<CSound> pcSourceSound)
{
	Init(pcSourceSound, 0, pcSourceSound->GetNumFrames(), SOUND_CEL_NO_LOOP);
}


//////////////////////////////////////////////////////////////////////////
//
// A loop frame that isn't inside the cel means no loop.
//
//////////////////////////////////////////////////////////////////////////
void CSoundCel::Init(Ptr<CSound> pcSourceSound, lsize uiFirstFrame, lsize uiNumFrames, lsize uiLoopFrame)
{
	PreInit();

	mpSourceSound = pcSourceSound;
	muiFirstFrame = uiFirstFrame;
	muiNumFrames = uiNumFrames;
	SetLoopFrame(uiLoopFrame);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundCel::Class(void)
{
	M_Pointer(mpSourceSound);
	U_LSize(muiFirstFrame);
	U_LSize(muiNumFrames);
	U_LSize(muiLoopFrame);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundCel::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSoundCel::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSoundCel::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundCel::Copy(CSoundCel* pcSource)
{
	mpSourceSound = pcSource->GetSourceSound();
	muiFirstFrame = pcSource->muiFirstFrame;
	muiNumFrames = pcSource->muiNumFrames;
	muiLoopFrame = pcSource->muiLoopFrame;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the cel's frames are all in the source sound.
//
//////////////////////////////////////////////////////////////////////////
bool CSoundCel::IsValid(void)
{
	if (mpSourceSound.IsNull())
	{
		return false;
	}
	return (muiFirstFrame <= mpSourceSound->GetNumFrames()) && (muiNumFrames <= mpSourceSound->GetNumFrames() - muiFirstFrame);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSound> CSoundCel::GetSourceSound(void)
{
	return mpSourceSound;
}


//////////////////////////////////////////////////////////////////////////
//
// The cel's first frame in the source sound.
//
//////////////////////////////////////////////////////////////////////////
lsize CSoundCel::GetFirstFrame(void)
{
	return muiFirstFrame;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CSoundCel::GetNumFrames(void)
{
	return muiNumFrames;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSoundCel::HasLoop(void)
{
	return muiLoopFrame != SOUND_CEL_NO_LOOP;
}


//////////////////////////////////////////////////////////////////////////
//
// The frame (from the start of the cel) the cel goes back to when it
// reaches its end.  SOUND_CEL_NO_LOOP if it doesn't loop.
//
//////////////////////////////////////////////////////////////////////////
lsize CSoundCel::GetLoopFrame(void)
{
	return muiLoopFrame;
}


//////////////////////////////////////////////////////////////////////////
//
// The frames from the loop frame to the end of the cel, zero if it
// doesn't loop.
//
//////////////////////////////////////////////////////////////////////////
lsize CSoundCel::GetLoopNumFrames(void)
{
	if (!HasLoop())
	{
		return 0;
	}
	return muiNumFrames - muiLoopFrame;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundCel::SetLoopFrame(lsize uiLoopFrame)
{
	if (uiLoopFrame < muiNumFrames)
	{
		muiLoopFrame = uiLoopFrame;
	}
	else
	{
		muiLoopFrame = SOUND_CEL_NO_LOOP;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// uiFrame is counted from the start of the cel.
//
//////////////////////////////////////////////////////////////////////////
float64 CSoundCel::GetSample(lsize uiFrame, size uiChannelIndex)
{
	float64	fSample;

	if (!GetSamples(uiChannelIndex, uiFrame, 1, &fSample))
	{
		return 0.0;
	}
	return fSample;
}


//////////////////////////////////////////////////////////////////////////
//
// uiFirstFrame is counted from the start of the cel.
//
//////////////////////////////////////////////////////////////////////////
bool CSoundCel::GetSamples(size uiChannelIndex, lsize uiFirstFrame, lsize uiNumFrames, float64* pfDest)
{
	if ((uiFirstFrame > muiNumFrames) || (uiNumFrames > muiNumFrames - uiFirstFrame))
	{
		gcLogger.Error2(__METHOD__, " Frames [", LongToString((uint64)uiFirstFrame), "] to [", LongToString((uint64)(uiFirstFrame + uiNumFrames)), "] are past the end of the cel [", LongToString((uint64)muiNumFrames), "].", NULL);
		return false;
	}
	if (!IsValid())
	{
		gcLogger.Error2(__METHOD__, " Cel is not in its source sound.", NULL);
		return false;
	}
	return mpSourceSound->GetSamples(uiChannelIndex, muiFirstFrame + uiFirstFrame, uiNumFrames, pfDest);
}


//////////////////////////////////////////////////////////////////////////
//
// Gets the samples played from uiPosition frames after the cel started,
// going back to the loop frame each time the end is reached as the SNES
// does.  A cel without a loop is silent after its end.  Returns the
// frames that came from the cel; the rest of pfDest is silence.
//
//////////////////////////////////////////////////////////////////////////
lsize CSoundCel::Play(size uiChannelIndex, lsize uiPosition, lsize uiNumFrames, float64* pfDest)
{
	lsize	uiWritten;
	lsize	uiFrame;
	lsize	uiCount;
	lsize	i;

	uiWritten = 0;
	while (uiWritten < uiNumFrames)
	{
		uiFrame = GetLoopedFrame(uiPosition + uiWritten, muiNumFrames, muiLoopFrame);
		if (uiFrame == SOUND_CEL_NO_LOOP)
		{
			break;
		}

		uiCount = muiNumFrames - uiFrame;
		if (uiCount > uiNumFrames - uiWritten)
		{
			uiCount = uiNumFrames - uiWritten;
		}
		if (!GetSamples(uiChannelIndex, uiFrame, uiCount, &pfDest[uiWritten]))
		{
			break;
		}
		uiWritten += uiCount;
	}

	for (i = uiWritten; i < uiNumFrames; i++)
	{
		pfDest[i] = 0.0;
	}
	return uiWritten;
}


//////////////////////////////////////////////////////////////////////////
//
// The frame played uiFrame frames after a cel of uiNumFrames started:
// past the end it goes round the loop.  SOUND_CEL_NO_LOOP if the cel
// has ended.
//
//////////////////////////////////////////////////////////////////////////
lsize CSoundCel::GetLoopedFrame(lsize uiFrame, lsize uiNumFrames, lsize uiLoopFrame)
{
	if (uiFrame < uiNumFrames)
	{
		return uiFrame;
	}
	if ((uiLoopFrame == SOUND_CEL_NO_LOOP) || (uiLoopFrame >= uiNumFrames))
	{
		return SOUND_CEL_NO_LOOP;
	}
	return uiLoopFrame + (uiFrame - uiLoopFrame) % (uiNumFrames - uiLoopFrame);
}


//////////////////////////////////////////////////////////////////////////
//
// GetLoopedFrame for a fractional position, which is moved back into the
// loop.  Returns false if the cel has ended.
//
//////////////////////////////////////////////////////////////////////////
bool CSoundCel::WrapPosition(float64* pfPosition, lsize uiNumFrames, lsize uiLoopFrame)
{
	float64	fLoopFrame;

	if (*pfPosition < (float64)uiNumFrames)
	{
		return true;
	}
	if ((uiLoopFrame == SOUND_CEL_NO_LOOP) || (uiLoopFrame >= uiNumFrames))
	{
		return false;
	}
	fLoopFrame = (float64)uiLoopFrame;
	*pfPosition = fLoopFrame + fmod(*pfPosition - fLoopFrame, (float64)(uiNumFrames - uiLoopFrame));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A new sound with just the cel's frames, in the source's format.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSound> CSoundCel::ToSound(void)
{
	Ptr<CSound>	pcSound;
	float64*	pfSamples;
	size		uiChannel;

	if (!IsValid())
	{
		gcLogger.Error2(__METHOD__, " Cel is not in its source sound.", NULL);
		return NULL;
	}

	pcSound = OMalloc<CSound>(mpSourceSound->GetSampleRate(), muiNumFrames, mpSourceSound);
	pfSamples = (float64*)malloc((lsize)sizeof(float64) * (muiNumFrames + 1));
	for (uiChannel = 0; uiChannel < mpSourceSound->NumChannels(); uiChannel++)
	{
		GetSamples(uiChannel, 0, muiNumFrames, pfSamples);
		pcSound->SetSamples(uiChannel, 0, muiNumFrames, pfSamples);
	}
	free(pfSamples);
	return pcSound;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundCel::Print(CChars* psz)
{
	psz->Append("Sound Cel [");
	psz->Append(muiFirstFrame);
	psz->Append(", ");
	psz->Append(muiNumFrames);
	psz->Append(" frames");
	if (HasLoop())
	{
		psz->Append(", loop ");
		psz->Append(muiLoopFrame);
	}
	psz->Append("]");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundCel::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.AppendNewLine();
	sz.DumpKill();
}

